#include "sync.h"

void sema_init(semaphore* s, uint32_t value){
    if(s==0) PANIC("Semaphore NULL");

    s->value=value;
    s->waiters=list_init();
}

/* Waits for semaphore to become positive then automatically
 * decrements it.
 * It may sleep so must not be called within interrupt handler.
 * It can be called with interrupts disabled.
 */
void sema_down(semaphore* s){
    int itr_level;

    if(s==NULL) PANIC("NULL semahore");

    //to ensure no raceconditions as cannot context switch
    itr_level=int_disable();
    while(s->value==0){
        append(s->waiters,current_proc());
        proc_block();
    }
    s->value--;
    int_set(itr_level);
}


/* Incremenets semaphores value, and wakes up any of the
 * waiting threads if they exist.
 * This may be called from an interrupt handler
 */
void sema_up(semaphore* s){
    int itr_level;

    if(s==NULL) PANIC("NULL semaphore");

    //to ensure no raceconditions as cannot context switch
    itr_level=int_disable();
    if(!is_empty(s->waiters)){
        proc_unblock(pop(s->waiters));
    }
    s->value++;
    int_set(itr_level);
}


/* Initialises a lock using a semaphore.
 * Differences being that the person who locks
 * it must also be the same person to unlock
 * and it has a value of 1.
 */
void lock_init(lock* l){
    if(l==NULL) PANIC("NULL lock");

    l->holder=NULL;
    sema_init(&l->semaphore,1);
}

/* Accquires the lock using sema down
 * this function may sleep so cannot 
 * be called within interrupt handler
 */
void lock_accquire(lock* l){
    if(l==NULL) PANIC("NULL lock");

    if(l->holder==current_proc()) PANIC("Lock already held by this thread");

    sema_down(&l->semaphore);
    l->holder=current_proc();
}

/* Releases lock using sema_up
 * and sets holder to NULL
 */
void lock_release(lock* l){
    if(l==NULL) PANIC("NULL lock");
    if(l->holder!=current_proc()) PANIC("Attempted release of lock not held by current thread");

    l->holder=NULL;
    sema_up(&l->semaphore);
}

/* Initialises condition variable
 * Allows one thread to send signals 
 * to one or all threads to wake up when some 
 * condition is met
 */
void cond_init(condition* cond){
    if(cond==NULL) PANIC("NULL condition");

    list_init(&cond->waiters);
}

//TODO Implement cond wait
/*
void cond_wait(condition* c, lock* l){}

void cond_signal(condition* c, lock* l){}

void cond_broadcast(condition* c, lock* l){}
*/