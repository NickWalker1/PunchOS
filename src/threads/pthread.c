#include "pthread.h"

#include "thread.h"



/* Pthread logic vs PunchOS threads.
 * Each pthread is of course a punchOS thread, however it is an abstraction around it for added functionality and cleanliness.
 * Each Pthread must have a *permanent* struct allocated to it on creation or boot. (Boot preferable of data array of size 64?)
 * this is because it's error code and state must be saved for any pthread_join call that may be called on it's ID.
 * A lack of a pthread_join causes what is called a zombie process that still maintains some system resources. 
 * In this implementation, the thread will die and all PunchOS related elements will be freed. However the Pthread elements remain.
 * These are the elements that must be cleared on a pthread_join.
 * 
 * When a pthread_join call is performed, the thread will access the pthread_tracker and proceed to wait on the finished_cond. Meaning
 * in the wrapper function pthread_run, when finished it will signal any join call to wakeup. This then allows it to cleanup the pthread_tracker
 * and ensure the pthread_has finished execution. 
 * 
 * The pthread_run function will then return into the main run function in thread.c. This will then finally kill off the thread on the next context
 * switch. 
 * 
 * Pthread_exit. Must give them some code for updating thread status form run.
 * 
 * Pthread-exit is preemptive so won't fall through normal run or pthread_run closures end cleanup method. Therefore must do it manually and schedule another process.
 * 
 */
pthread_desc_t pthread_tracker[PTHREAD_MAX_THREADS];


/* Unique identifer... just add 1 each time pthread_create is called */
pthread_t id_tracker=0;

/* Creates a new pthread. */
int pthread_create(UNUSED pthread_t *thread,UNUSED pthread_attr_t *attr, UNUSED void *(star_routine)(void*),UNUSED void *arg){

    /* Get a new PTHREAD_T and allocate a space in memory for me */

    /* Create the actual PunchOS thread that will do the work using create_thread() and passing whatever arguments */
    return 0;
}

/* Waits for the given thread to exit. If the thread has already exited then will return immediately */
int pthread_join(UNUSED pthread_t thread, UNUSED void **retval){

    return 0;
}

/* Exits the calling thread and returns value via retval.
 * Sets thread status to T_DYING so can be killed on next switch_complete. */
void pthread_exit(UNUSED void *retval){


    //Must update status to T_DYING and schedule a new process 
}




/*Both pthread_finished and pthread_run are non-standard pthread function but they are
 * required for interoperability with PunchOS threads. */
void pthread_finished(){
    //lookup pthread_tracker

    //signal I'm done

}


/* Helper function used as a wrapper around standard run function defined in thread.h.
 * Used to setup pthread descriptor to allow for wait on exit etc */
void pthread_run( void *(start_routine)(void*), void *arg){
    
    int_enable();

    start_routine(arg);

    int_disable();

    pthread_finished();


}