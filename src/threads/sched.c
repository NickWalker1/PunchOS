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


/* MLFQ queues here: */
list *prio_0;
list *prio_1;
list *prio_2;

int prio_0_TQ = 2;
int prio_1_TQ = 4;
int prio_2_TQ = 8;

/* Initialises schedulising by initialising any lists or structures needed for scheduling */
bool scheduling_init(){
    // ready_queue=list_init_shared();
    // if(!ready_queue) return false;

    /* Default setup value */
    time_quantum=4;

    /* To be implemented. */
    prio_0=list_init_shared();
    prio_1=list_init_shared();
    prio_2=list_init_shared();
    if(!prio_0 || !prio_1 ||  !prio_2) return false;



    return true;
}


/* Reschedules a thread by adding itto the appropriate queue.
 * Must be called with interrupts disabled.*/
void thread_reschedule(TCB_t *t){
    /* Appending to ready threads if not idle threads. */
    // append_shared(ready_queue,t);

    /* Reset waiting ticks counter to 0 */
    thread_tracker[t->tid].wait_ticks=0;

    t->status=T_READY;

    // return;

    /* If thread has been preempted then reduce it's priority 0 is highest, 2 is lowest*/
    if(timeout && t->priority<PRIO_MIN) ++(t->priority);
    timeout=false;
    
    switch(t->priority){
    case 0:
        append_shared(prio_0,t);
        break;
    case 1:
        append_shared(prio_1,t);
        break;
    default:
        append_shared(prio_2,t);
    }

}


/* Returns the next process to be scheduled.
 * Currently round robin approach */
TCB_t* get_next_thread(){

    /* Round robin approach */
    // if(is_empty(ready_queue)){
    //     return idle_thread;
    // }

    // return (TCB_t*)(pop_shared(ready_queue));

    
    if(!is_empty(prio_0)){
        time_quantum=prio_0_TQ;
        return pop_shared(prio_0);

    }

    if(!is_empty(prio_1)) {
        time_quantum=prio_1_TQ;
        return pop_shared(prio_1);
    }

    if(!is_empty(prio_2)) {
        time_quantum=prio_2_TQ;
        return pop_shared(prio_2);
    }

    return idle_thread;
}


/* Used to remove any thread from any queues in the case of preemptive kill */
bool deschedule(TCB_t *t){
    return remove_shared(prio_0,t) || remove_shared(prio_1,t) || remove_shared(prio_2,t);
}

/* Returns what would be the next thread thread to execute without removing anything */  
TCB_t *peek_next_thread(){
    TCB_t *next;
    next=peek(prio_0);
    if(next) return next;
    next=peek(prio_1);

    if(next) return next;
    next=peek(prio_2);

    if(next) return next;

    return idle_thread;
}


void queue_dump(){
    println("Queue Dump:");
    list_dump(prio_0);
    list_dump(prio_1);
    list_dump(prio_2);
}