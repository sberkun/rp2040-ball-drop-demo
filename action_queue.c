

#include "action_queue.h"



/**
 * Initialize an action_queue. 
 */
void action_queue_init(action_queue_t* queue, size_t max_size) {
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
 * De-Initialize an action_queue.
*/
void action_queue_deinit(action_queue_t* queue) {
    free(queue->actions);
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
 * Adds an action to the queue. fn will be called at the time us_since_boot.
 * Returns 0 on success, 1 on error (i.e. not enough capacity).
 * 
 * This function is safe to call concurrently, including from interrupts.
 */
int schedule_action(action_queue_t* queue, action_fn_ptr fn, uint64_t us_since_boot) {
    critical_section_enter_blocking(&queue->crit_sec);

    // check if queue has space for another action
    if (queue->size >= queue->capacity) {
        critical_section_exit(&queue->crit_sec);
        return 1;
    }

    // queue should be sorted by time. For every action that is
    // less urgent (has greater time), slide it over 1 to make
    // room for this new action. O(N).
    size_t N = queue->capacity;
    size_t dest_ind = mod1(queue->start_index + queue->size, N);
    while (dest_ind != queue->start_index && queue->actions[pred(dest_ind, N)].time > us_since_boot) {
        queue->actions[dest_ind] = queue->actions[pred(dest_ind, N)];
        dest_ind = pred(dest_ind, N);
    }
    queue->actions[dest_ind].fn = fn;
    queue->actions[dest_ind].time = us_since_boot;
    queue->size += 1;

    // check if the action that was just pushed is the new most urgent action
    bool most_urgent_action_changed = dest_ind == queue->start_index;

    critical_section_exit(&queue->crit_sec);

    // if most urgent action changes,
    // notify all threads to get most urgent action 
    if (most_urgent_action_changed) {
        for (int i = 0; i < NUM_CORES; i++) {
            sem_reset(&queue->notifs[i], 1);
        }
    }
    return 0;
}

/**
 * Waits for the time of the next action, then calls the function with the
 * requested time as an argument (since actual and requested times may differ slightly)
 */
void do_next_action(action_queue_t* queue) {
    action_t next_action;
    semaphore_t* mailbox = &(queue->notifs[get_core_num()]);
    
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
}



