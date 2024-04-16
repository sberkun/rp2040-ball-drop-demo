#ifndef ACTOR_QUEUE_H
#define ACTOR_QUEUE_H


#include <stdlib.h>
#include "pico/sync.h"
#include "pico/stdlib.h"


/**
 * Some terminology:
 *  - actor:  Like an actor in the actor model. Similar to a reactor in LF, only
 *            one action may be running in an actor at a specific time. Actions
 *            are identified with a size_t id.
 *  - action: A function that runs in an actor. Similar to a reaction in LF.
 *  - event:  An invocation of an action at a specific time. These are used like
 *            messages in the actor model. An event also has an 8-bit priority,
 *            and may optionally carry a small amount of data.
 *  - release time: the time an event is meant to be invoked. In practice,
 *            the actual time an event is invoked happens slightly after the
 *            release time; this may be called the "start time" of the event.
 *  - urgency: "more urgent" means an event will be invoked earlier. "less urgent"
 *            means an event will be invoked later. The urgency is a combination
 *            of the event's time and priority.
 * 
 * The event queue is a sorted queue of events. It's a circular queue, with O(N)
 * worst case times. It's optimized for low contention (few events happening
 * simultaneously) and events being scheduled mostly in-order. 
 * 
 */


typedef void (*action)(uint64_t release_time_us, size_t actor_id, void* arg);

typedef struct event {
    uint64_t time_prio; // first 56 bits: time. last 8 bits: priority
    size_t actor_id;    // which actor this event is for
    action fn;       // the action to invoke
    void* arg;          // an optional argument to the function
} event_t;


typedef struct event_queue {
    
    bool* actors_busy; // an array, one for each actor. 
    size_t num_actors;

    event_t* events; // a circular array, starting at start_index
    size_t start_index;
    size_t size;
    size_t; capacity;

    critical_section_t crit_sec;
    semaphore_t notifs[NUM_CORES];
} event_queue_t;


/**
 * Initialize an event_queue. 
 */
void event_queue_init(event_queue_t* queue, size_t max_size, size_t num_actors);

/**
 * De-Initialize an event_queue.
*/
void event_queue_deinit(event_queue_t* queue);

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
int schedule_event(event_queue_t* queue, size_t actor_id, action fn, void* arg, uint64_t release_time_us, uint8_t priority);


/**
 * Waits for events to release, then invokes them in an infinite loop. 
 */
void work(event_queue_t* queue);




#endif // ACTOR_QUEUE_H