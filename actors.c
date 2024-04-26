#include "actors.h"


/**
 * Initialize an event_queue. 
 */
int event_queue_init(event_queue_t* queue, size_t max_size, size_t num_actors) {
    queue->actors_busy = (bool*) malloc(sizeof(bool) * num_actors);
    queue->num_actors = num_actors;
    queue->events = (event_t*) malloc(sizeof(event_t) * max_size);
    queue->capacity = max_size;
    queue->size = 0;
    queue->start_index = 0;

    if (queue->actors_busy == NULL || queue->events == NULL) {
        return 1;
    }

    for (int a = 0; a < num_actors; a++) {
        queue->actors_busy[a] = false;
    }
    critical_section_init(&queue->crit_sec);
    for (int i = 0; i < NUM_CORES; i++) {
        sem_init(&queue->notifs[i], 0, 1);
    }
    return 0;
}

/**
 * De-Initialize an event_queue.
*/
void event_queue_deinit(event_queue_t* queue) {
    free(queue->actors_busy);
    free(queue->events);
    critical_section_deinit(&queue->crit_sec);
}


static inline size_t mod1(size_t a, size_t N) {
    return a - (a >= N ? N : 0);
}

static inline size_t succ(size_t a, size_t N) {
    return (a + 1 == N) ? 0 : (a + 1);
}

static inline size_t pred(size_t a, size_t N) {
    return (a == 0 ? N : a) - 1;
}




int schedule_event(event_queue_t* queue, uint64_t release_time_us, size_t actor_id, action fn, void* arg) {
    critical_section_enter_blocking(&queue->crit_sec);

    
    #ifdef SAFE_QUEUE
        // check if queue has space for another event
        if (queue->size >= queue->capacity) {
            critical_section_exit(&queue->crit_sec);
            return 1;
        }

        // check if actor_id is valid
        if (actor_id >= queue->num_actors) {
            critical_section_exit(&queue->crit_sec);
            return 1;
        }
    #endif


    // queue should be sorted by time. For every event that is
    // less urgent (has greater time), slide it over 1 to make
    // room for this new event. O(N).
    size_t N = queue->capacity;
    size_t dest_ind = mod1(queue->start_index + queue->size, N);
    while (dest_ind != queue->start_index) {
        size_t prev = pred(dest_ind, N);
        if (queue->events[prev].time <= release_time_us) {
            break;
        }
        queue->events[dest_ind] = queue->events[prev];
        dest_ind = prev;
    }
    event_t new_event = {
        .time = release_time_us,
        .actor_id = actor_id,
        .fn = fn,
        .arg = arg
    };
    queue->events[dest_ind] = new_event;
    queue->size += 1;

    critical_section_exit(&queue->crit_sec);

    // notify all threads to get most urgent event.
    for (int i = 0; i < NUM_CORES; i++) {
        sem_reset(&queue->notifs[i], 1);
    }
    return 0;
}


// returns -1 if not found
#define NOT_FOUND ((size_t) (-1))
static inline size_t find_most_urgent(event_queue_t* queue) {
    size_t N = queue->capacity;
    size_t end = mod1(queue->start_index + queue->size, N);
    for (size_t i = queue->start_index; i != end; i = succ(i, N)) {
        size_t actor_id = queue->events[i].actor_id;
        if (!queue->actors_busy[actor_id]) {
            return i;
        }
    }
    return NOT_FOUND;
}

// removes a event_t from the ind
static inline void remove_front(event_queue_t* queue, size_t ind_to_remove) {
    size_t N = queue->capacity;
    while (ind_to_remove != queue->start_index) {
        size_t prev = pred(ind_to_remove, N);
        queue->events[ind_to_remove] = queue->events[prev];
        ind_to_remove = prev;
    }
    queue->start_index = succ(queue->start_index, N);
    queue->size -= 1;
}

// given an empty position ind, and an item that goes in >= ind, finds right place for
// item and puts it there
static inline void bubble_up(event_queue_t* queue, size_t ind_to_bubble, event_t item_to_bubble) {
    size_t N = queue->capacity;
    size_t end = mod1(queue->start_index + queue->size, N);
    while (ind_to_bubble != end) {
        size_t next = succ(ind_to_bubble, N);
        if (item_to_bubble.time <= queue->events[next].time) {
            queue->events[ind_to_bubble] = item_to_bubble;
            return;
        }
        queue->events[ind_to_bubble] = queue->events[next];
        ind_to_bubble = next;
    }
}


void work(event_queue_t* queue) {
    event_t next_event;
    semaphore_t* mailbox = &(queue->notifs[get_core_num()]);
    next_event.actor_id = NOT_FOUND;

    while (1) {
        // minimize spurious wakeups
        sem_reset(mailbox, 0);

        // "claim" an event and put it in next_event
        while (1) {
            critical_section_enter_blocking(&queue->crit_sec);

            // release the actor whose event we just ran
            if (next_event.actor_id != NOT_FOUND) {
                queue->actors_busy[next_event.actor_id] = false;
            }

            size_t N = queue->capacity;
            size_t most_urgent = find_most_urgent(queue);
            if (most_urgent == NOT_FOUND) {
                // queue is empty; wait for it to get something
                critical_section_exit(&queue->crit_sec);
                sem_acquire_blocking(mailbox);
            } else {
                next_event = queue->events[most_urgent];
                remove_front(queue, most_urgent);
                queue->actors_busy[next_event.actor_id] = true;
                critical_section_exit(&queue->crit_sec);
                break;
            }
        }

        // wait for event to expire
        // in best case, event expires without any notifications
        bool notified = sem_acquire_block_until(mailbox, from_us_since_boot(next_event.time));
        while (notified) {
            // something was put on the queue; 
            // all threads check if it's more urgent than the one they claimed
            critical_section_enter_blocking(&queue->crit_sec);

            size_t N = queue->capacity;
            size_t most_urgent = find_most_urgent(queue);
            if (most_urgent != NOT_FOUND && queue->events[most_urgent].time < next_event.time) {
                queue->actors_busy[next_event.actor_id] = false;
                event_t new_best = queue->events[most_urgent];
                bubble_up(queue, most_urgent, next_event);
                next_event = new_best;
                queue->actors_busy[next_event.actor_id] = true;
            }
            
            critical_section_exit(&queue->crit_sec);
            notified = sem_acquire_block_until(mailbox, from_us_since_boot(next_event.time));
        }

        // call the action function
        next_event.fn(next_event.time, next_event.actor_id, next_event.arg);
    }
}

