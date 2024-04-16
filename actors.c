#include "actors.h"


/**
 * Initialize an event_queue. 
 */
void event_queue_init(event_queue_t* queue, size_t max_size, size_t num_actors) {
    queue->actors_busy = (bool*) malloc(sizeof(bool) * num_actors);
    queue->num_actors = num_actors;

    queue->actions = (action_t*) malloc(sizeof(action_t) * max_size);
    queue->capacity = max_size;
    queue->size = 0;
    queue->start_index = 0;

    critical_section_init(&queue->crit_sec);
    for (int i = 0; i < NUM_CORES; i++) {
        sem_init(&queue->notifs[i], 0, 1);
    }
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


/**
 * Adds an event to the queue.
 * This function is safe to call concurrently, including from interrupts.
 * \param queue The queue to add the event to
 * \param actor_id Which actor this event is for
 * \param fn the action to call when this event releases
 * \param arg an extra function arguments for the action, or NULL if not needed
 * \param release_time_us the earliest time that this event may be invoked
 * \param priority 0 for most urgent / highest priority, 255 for least urgent / lowest priority
 * \return 0 on success, 1 on error (i.e. not enough capacity).
 */
int schedule_event(event_queue_t* queue, size_t actor_id, action fn, void* arg, uint64_t release_time_us, uint8_t priority) {
    critical_section_enter_blocking(&queue->crit_sec);

    // check if queue has space for another action
    if (queue->size >= queue->capacity) {
        critical_section_exit(&queue->crit_sec);
        return 1;
    }

    // check if actor_id is valid
    if (actor_id >= queue->num_actors) {
        critical_section_exit(&queue->crit_sec);
        return 1;
    }

    // TODO combine priority and time

    // queue should be sorted by time. For every action that is
    // less urgent (has greater time), slide it over 1 to make
    // room for this new action. O(N).
    size_t N = queue->capacity;
    size_t dest_ind = mod1(queue->start_index + queue->size, N);
    while (dest_ind != queue->start_index && queue->events[pred(dest_ind, N)].time > us_since_boot) {
        queue->events[dest_ind] = queue->events[pred(dest_ind, N)];
        dest_ind = pred(dest_ind, N);
    }
    queue->events[dest_ind].fn = fn;
    queue->events[dest_ind].time = us_since_boot;
    // TODO: all the other things
    queue->size += 1;

    // main difference from action_queue_v2:
    // some number of events at the beginning of the queue are blocked
    // heuristic: check if the event before this one is blocked (its actor is busy)
    bool most_urgent_action_maybe_changed = (dest_ind == queue->start_index)
        || queue->actors_busy[queue->events[pred(dest_ind, N)].actor_id];

    critical_section_exit(&queue->crit_sec);

    // If most urgent action changes,
    // notify all threads to get most urgent action.
    if (most_urgent_action_maybe_changed) {
        for (int i = 0; i < NUM_CORES; i++) {
            sem_reset(&queue->notifs[i], 1);
        }
    }
    return 0;
}


/**
 * Waits for events to release, then invokes them in an infinite loop. 
 */
void work(event_queue_t* queue) {
    action_t next_action;
    semaphore_t* mailbox = &(queue->notifs[get_core_num()]);

    // TODO all the logic related to thingys being blocked
    // namely, once grabbing an action, acquire its actors "busy"

    while (1) {
        // minimize spurious wakeups
        sem_reset(mailbox, 0);

        // "claim" an action and put it in next_action
        while (1) {
            critical_section_enter_blocking(&queue->crit_sec);
            if (&queue->size == 0) {
                // queue is empty; wait for it to get something
                critical_section_exit(&queue->crit_sec);
                sem_acquire_blocking(mailbox);
            } else {
                // pop action off queue
                next_action = queue->actions[queue->start_index];
                queue->start_index = mod1(queue->start_index + 1, queue->capacity);
                queue->size -= 1;
                critical_section_exit(&queue->crit_sec);
                break;
            }
        }

        // wait for action to expire
        // in best case, action expires without any notifications
        bool notified = sem_acquire_block_until(mailbox, from_us_since_boot(next_action.time));
        while (notified) {
            // something was put on the queue; 
            // all threads check if it's more urgent than the one they claimed
            critical_section_enter_blocking(&queue->crit_sec);
            if (&queue->size > 0 && queue->actions[queue->start_index].time < next_action.time) {
                // switcheroo.
                action_t h = next_action;
                next_action = queue->actions[queue->start_index];

                // bubble up; multiple urgent actions may have been pushed
                size_t N = queue->capacity;
                size_t end = mod1(queue->start_index + queue->size, N);
                size_t curr = queue->start_index;
                size_t next = succ(curr, N);
                while (next != end && queue->actions[next].time < h.time) {
                    queue->actions[curr] = queue->actions[next];
                    curr = next;
                    next = succ(next, N);
                }
                queue->actions[curr] = h;
            }
            critical_section_exit(&queue->crit_sec);
            notified = sem_acquire_block_until(mailbox, from_us_since_boot(next_action.time));
        }

        // call the action function
        next_action.fn(next_action.time);

        // release the actors "busy"
        // TODO: fold this into the "grab a thing off the queue" logic?
        critical_section_enter_blocking(&queue->crit_sec);
        
        critical_section_exit(&queue->crit_sec);
    }
}

