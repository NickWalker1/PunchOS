#include "sched.h"

/* Colleciton of functions relating to the scheduling of threads */


#include "../lib/list.h"



list *ready_threads;


extern TCB_t *idle_thread;


#include "thread.h"


/* Initialises schedulising by initialising any lists or structures needed for scheduling */
bool scheduling_init(){
    ready_threads=list_init();
    if(!ready_threads) return false;

    /* To be implemented. */


    return true;
}



/* Reschedules a threadess by adding it
 *  to the appropriate queue.
 * Must be called with interrupts disabled.*/
void thread_reschedule(TCB_t *t){
    /* Reset waiting ticks counter to 0 */

    //TODO fix
    // thread_tracker[p->pid-1].wait_ticks=0;
    

    /* Appending to ready threads if not idle threads. */
    append_shared(ready_threads,t);
    
    t->status=T_READY;
}

/* Returns the next process to be scheduled.
 * Currently round robin approach */
TCB_t* get_next_thread(){

    //round robin approach
    if(is_empty(ready_threads)){
        return idle_thread;
    }

    return (TCB_t*)(pop_shared(ready_threads));
}
