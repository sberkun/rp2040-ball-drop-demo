


#ifndef ACTION_QUEUE_H
#define ACTION_QUEUE_H

#include <stdlib.h>
#include "pico/sync.h"
#include "pico/stdlib.h"

/**
 * This is a sorted queue of "actions"; each action is a function to
 * be called at a specific time.
 * 
 * "more urgent" means an action will be invoked at a smaller time;
 * "less urgent" means a greater time.
 * 
 * The queue is circular, and sorted from start_index. Pushing to
 * the queue is O(1) best case (if new action is least urgent), or
 * O(N) worst case. Popping from the queue is always O(1).
 * 
 * A critical section is used instead of a mutex because events can be
 * scheduled from interrupts.
 * 
 * The semaphore is the sleeping mechanism, and is used to "wake up"
 * a thread when the queue changes. Essentially, the semaphore is used
 * like a condition variable's notify_one. Only one thread is notified
 * at a time (no thundering herd), which should scale to an arbitrary
 * number of threads. 
 */


typedef void (*action_fn_ptr)(uint64_t);

typedef struct action {
    action_fn_ptr fn;
    uint64_t time;
} action_t;

typedef struct action_queue {
    action_t* actions;
    size_t start_index;
    size_t size;
    size_t capacity;
    critical_section_t crit_sec;
    semaphore_t sema;
} action_queue_t;



/**
 * Initialize an action_queue. 
 */
void action_queue_init(action_queue_t* queue, size_t max_size);

/**
 * De-Initialize an action_queue.
*/
void action_queue_deinit(action_queue_t* queue);

/**
 * Adds an action to the queue. fn will be called at the time us_since_boot.
 * Returns 0 on success, 1 on error (i.e. not enough capacity).
 * 
 * This function is safe to call concurrently, including from interrupts.
 */
int schedule_action(action_queue_t* queue, action_fn_ptr fn, uint64_t us_since_boot);


/**
 * Waits for the time of the next action, then calls the function with the
 * requested time as an argument (since actual and requested times may differ slightly)
 */
void do_next_action(action_queue_t* queue);

#endif