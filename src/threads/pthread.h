#pragma once

#include "../lib/typedefs.h"

#include "../sync/sync.h"

#define PTHREAD_MAX_THREADS 32

typedef uint32_t pthread_t; /* Is NOT the same as t_id. Unique to pthreads */

/* Unused attribute struct for simplicity */
typedef struct pthread_attr{
    int unused;
} pthread_attr_t;


/* Status struct, will be updated on creation and death */
typedef enum pthread_status{
    PTHR_RUN,
    PTHR_EXIT
} pthread_status_t;


/* This will perist after a thread may be killed and removed from any trace systems.
 * Thus status will can used for pthread_join */
typedef struct pthread_desc{
    pthread_t thread; /* Unique pthread identifier */
    pthread_status_t status; /* Current status */
    lock finished_lock; /* condition requires a lock */
    condition finished_cond; /*Used to avoid active wait */
} pthread_desc_t;


int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(start_routine)(void *), void *arg);
int pthread_join(pthread_t thread, void **retval);
void pthread_exit(void *retval); 


/* Non standard function for support */
void pthread_run(void *(start_routine)(void *), void *arg);
