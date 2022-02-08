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



/* Initialises schedulising by initialising any lists or structures needed for scheduling */
bool scheduling_init(){
    if(naive_scheduling){
        /* Default setup value */
        time_quantum=4;

        ready_queue=list_init_shared();
        if(!ready_queue) return false;


        return true;
    }

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

    return idle_thread;
}


/* Used to remove any thread from any queues in the case of preemptive kill */
bool deschedule(TCB_t *t){
    if(naive_scheduling){
        return remove_shared(ready_queue,t);
    }

    return false;
}


/* Returns what would be the next thread thread to execute without removing anything */  
TCB_t *peek_next_thread(){
    if(naive_scheduling){
        return peek(ready_queue);
    }

    return idle_thread;
}


void queue_dump(){
    println("Queue Dump:");
    if(naive_scheduling){
        list_dump(ready_queue);
        return;
    }
}