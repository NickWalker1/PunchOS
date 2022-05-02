#include "sched.h"

/* Colleciton of functions relating to the scheduling of threads */

#include "thread.h"

#include "../lib/list.h"


/* Used for a naive round robin approach. You may choose to remove this. */
list *ready_queue;


/* The number of ticks the current process is allowed to operate for before a context switch */
int time_quantum;

/* Is true if the last schedule was due to a preemption on time basis */
bool timeout;


extern TCB_t *idle_thread;

extern thread_diagnostics_t thread_tracker[MAX_THREADS+1];


bool naive_scheduling=true;

/* MLFQ queues here: */
list *prio_0;
list *prio_1;
list *prio_2;

int prio_0_TQ = 2;
int prio_1_TQ = 4;
int prio_2_TQ = 8;

/* Initialises schedulising by initialising any lists or structures needed for scheduling.
 * Returns true on success. */
bool scheduling_init(){
    if(naive_scheduling){
        /* Default setup value */
        time_quantum=4;

        ready_queue=list_init_shared();
        if(!ready_queue) return false;


        return true;
    }
    /* MLFQ Scheduling */

    /* To be implemented. */


    return false;
}


/* Reschedules a thread by adding itto the appropriate queue.
 * Must be called with interrupts disabled.*/
void thread_reschedule(TCB_t *t){
    /* Reset waiting ticks counter to 0 */
    thread_tracker[t->tid].wait_ticks=0;

    t->status=T_READY;

    if(naive_scheduling){
        append_shared(ready_queue,t);
        return;
    }

    /* MLFQ to be implemented. */


    
}


/* Returns the next process to be scheduled.
 * Currently round robin approach */
TCB_t* get_next_thread(){
    if(naive_scheduling){
        if(is_empty(ready_queue)){
            return idle_thread;
        }

        return (TCB_t*)(pop_shared(ready_queue));
    }

    /* MLFQ to be implemented. */
    
    return NULL;
}


/* Used to remove any thread from any queues in the case of preemptive kill */
bool deschedule(TCB_t *t){
    if(naive_scheduling){
        return remove_shared(ready_queue,t);
    }

    /* MLFQ to be implemented. */

    
    return false;
}


/* Returns what would be the next thread thread to execute without removing anything */  
TCB_t *peek_next_thread(){
    if(naive_scheduling){
        return peek(ready_queue);
    }

    /* MLFQ to be implemented. */

    return NULL;
}


void queue_dump(){
    println("Queue Dump:");
    if(naive_scheduling){
        list_dump(ready_queue);
        return;
    }

    /* MLFQ to be implemented. */

}