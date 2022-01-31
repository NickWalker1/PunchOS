#include "sched.h"

/* Colleciton of functions relating to the scheduling of threads */

#include "thread.h"

#include "../lib/list.h"


/* Used for a naive round robin approach. You may choose to remove this. */
list *ready_queue;


/* The number of ticks the current process is allowed to operate for before a context switch */
int time_quantum;

extern TCB_t *idle_thread;

extern thread_diagnostics_t thread_tracker[MAX_THREADS+1];


/* MLFQ queues here: */
list *prio_0;
list *prio_1;
list *prio_2;

int prio_0_TQ = 4;
int prio_1_TQ = 8;
int prio_2_TQ = 16;

/* Initialises schedulising by initialising any lists or structures needed for scheduling */
bool scheduling_init(){
    ready_queue=list_init_shared();
    if(!ready_queue) return false;

    /* Default setup value */
    time_quantum=4;

    /* To be implemented. */
    prio_0=list_init_shared();
    prio_1=list_init_shared();
    prio_2=list_init_shared();
    if(!prio_0 || !prio_1 ||  !prio_2) return false;



    return true;
}


/* If a new thread has been rescheduled that is of a higher priority than the current process. Switch to that process */
void check_interrupt(TCB_t *new){
    if(current_thread()->priority < new->priority) {
        /* Add to front of queue */
        schedule();
    }

}

/* Reschedules a threadess by adding it
 *  to the appropriate queue.
 * Must be called with interrupts disabled.*/
void thread_reschedule(TCB_t *t){
    /* Appending to ready threads if not idle threads. */
    append_shared(ready_queue,t);

    /* Reset waiting ticks counter to 0 */
    thread_tracker[t->tid].wait_ticks=0;

    t->status=T_READY;

    return;

    /* If thread has been preempted then reduce it's priority  */
    // if(timeout && t->priority<PRIO_MIN) --(t->priority);
    
    // switch(t->priority){
    // case 0:
    //     append(prio_0,t);
    //     break;
    // case 1:
    //     append(prio_1,t);
    //     break;
    // default:
    //     append(prio_2,t);
    // }

    /* Check if current process needs to be interrupted */

}

/* Returns the next process to be scheduled.
 * Currently round robin approach */
TCB_t* get_next_thread(){

    //round robin approach
    if(is_empty(ready_queue)){
        return idle_thread;
    }

    return (TCB_t*)(pop_shared(ready_queue));
}
