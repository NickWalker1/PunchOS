#pragma once

#include "../lib/typedefs.h"
#include "../lib/list.h"

/* A counting semaphore. */
typedef struct semaphore{
    uint32_t value;             /* Current value. */
    list* waiters;        /* List of waiting processes */
} semaphore;

void sema_init(semaphore* s, unsigned value);
void sema_down(semaphore* s);
bool sema_try_down(semaphore* s);
void sema_up(semaphore* s);
void sema_self_test();



typedef struct lock lock;

void lock_init(lock* l);
void lock_acquire(lock* l);
bool lock_try_acquire(lock* l);
void lock_release(lock* l);

typedef struct condition condition;

/* Condition variable. */
struct condition {
    list waiters;        /* List of waiting processes. */
};

void cond_init(condition* c);
void cond_wait(condition* c, lock* l);
void cond_signal(condition* c, lock* l);
void cond_broadcast(condition* c, lock* l);
bool int_context();


#include "../processes/pcb.h"

/* Lock. */
struct lock {
    PCB_t *holder;      /* Process holding lock(for debugging). */
    semaphore semaphore; /* Binary semaphore controlling access. */
};
