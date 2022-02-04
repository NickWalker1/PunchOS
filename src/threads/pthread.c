#include "pthread.h"

#include "thread.h"

pthread_desc_t pthread_tracker[PTHREAD_MAX_THREADS];


/* Unique identifer... just add 1 each time pthread_create is called */
pthread_t id_tracker=0;

/* Creates a new pthread. */
int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(star_routine)(void*),void *arg){

}

/* Waits for the given thread to exit. If the thread has already exited then will return immediately */
int pthread_join(pthread_t thread, void **retval){

}

/* Exits the calling thread and returns value via retval.
 * Sets thread status to T_DYING so can be killed on next switch_complete. */
void pthread_exit(void *retval){

}


/* Helper function used as a wrapper around standard run function defined in thread.h.
 * Used to setup pthread descriptor to allow for wait on exit etc */
void pthread_run(void *(start_routine)(void*),void *arg){
    
}