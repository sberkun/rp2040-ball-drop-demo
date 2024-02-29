

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
    sem_init(&queue->sema, 0, 1);
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

    critical_section_exit(&queue->crit_sec);

    // if most urgent action changes,
    // notify one waiting thread to get most urgent action 
    if (dest_ind == queue->start_index) {
        sem_reset(&queue->sema, 1); 
    }
    return 0;
}

/**
 * Waits for the time of the next action, then calls the function with the
 * requested time as an argument (since actual and requested times may differ slightly)
 */
void do_next_action(action_queue_t* queue) {
    action_t next_action;
    uint64_t current_time = to_us_since_boot(get_absolute_time());
    
    // minimize spurious wakeups. Spurious wakeups may still occur,
    // but this handles the common case of do_next_action waking 
    // itself up on the same thread
    sem_reset(&queue->sema, 0);

    while (true) {
        critical_section_enter_blocking(&queue->crit_sec);
        next_action = queue->actions[queue->start_index];
        if (&queue->size == 0) {
            // queue is empty; wait for it to get something
            critical_section_exit(&queue->crit_sec);
            sem_acquire_blocking(&queue->sema);
        } else if (next_action.time <= current_time) {
            // it's time: pop action off queue!
            queue->start_index = mod1(queue->start_index + 1, queue->capacity);
            queue->size -= 1;
            critical_section_exit(&queue->crit_sec);
            break;
        } else {
            // wait for next action
            critical_section_exit(&queue->crit_sec);
            bool notified = sem_acquire_block_until(&queue->sema, from_us_since_boot(next_action.time));
            if (!notified) { // timed out
                current_time = next_action.time;
            }
        }
    }

    // notify another thread that queue has changed; 
    // other threads may be waiting on an action that is not 2nd most recent
    sem_reset(&queue->sema, 1);

    // call the action function
    next_action.fn(next_action.time);
}



