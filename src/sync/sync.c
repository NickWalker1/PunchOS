#include "sync.h"

#include "../processes/process.h"


/* NOTE:
 * ALL SYNCHRONISATION PRIMITIVES MAKE USE OF THE SHARED KERNEL SPACE 
 */

void sema_init(semaphore* s, uint32_t value){
    if(s==0) PANIC("Semaphore must be allocated before init");

    s->value=value;
    s->waiters=list_init_shared();
}


/* Waits for semaphore to become positive then automatically
 * decrements it.
 * It may sleep so must not be called within interrupt handler.
 * It can be called with interrupts disabled.
 */
void sema_down(semaphore* s){
    int itr_level;

    if(s==NULL) PANIC("NULL semaphore in sema_down");

    //to ensure no raceconditions as cannot context switch
    itr_level=int_disable();
    while(s->value==0){
        append_shared(s->waiters,current_proc());
        proc_block();
    }
    s->value--;
    int_set(itr_level);
}


/* Incremenets semaphores value, and wakes one of the
 * waiting threads if they exist.
 * This may be called from an interrupt handler
 */
void sema_up(semaphore* s){
    int itr_level;

    if(s==NULL) PANIC("NULL semaphore in sema_up");

    //to ensure no raceconditions as cannot context switch
    itr_level=int_disable();
    
    if(!is_empty(s->waiters)){
        proc_unblock(pop_shared(s->waiters));
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
    ASSERT(l!=NULL,"Must allocate lock before init");

    l->holder=NULL;
    sema_init(&l->semaphore,1);
}


/* Accquires the lock using sema down
 * this function may sleep so cannot 
 * be called within interrupt handler
 */
void lock_acquire(lock* l){
    if(l==NULL) PANIC("NULL lock in acquire");

    if(l->holder==current_proc()) PANIC("Lock already held by this thread");

    sema_down(&l->semaphore);
    l->holder=current_proc();
}


/* Releases lock using sema_up
 * and sets holder to NULL
 */
void lock_release(lock* l){
    if(l==NULL) PANIC("NULL lock in release");
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
    if(cond==NULL) PANIC("NULL cond in init");

    cond->waiters=list_init_shared();
}


/* Releases lock then waits until has been signaled by another process to wake up.
 * will reaquire lock on wakeup so may block
 */
void cond_wait(condition* c, lock* l){

    if(c==NULL) PANIC("NULL cond in wait");
    if(l==NULL) PANIC("NULL lock in wait");
    // ASSERT(!in_external_int(),"In external interrupt");

    PCB_t *p = current_proc();
    ASSERT(l->holder ==p,"Lock not held by current thread in cond wait.");

    //Reason using a semaphore here instead of just a pointer to the PCB
    // because incase the cond is waiting on multiple conditions, we do not
    // want it to wake up to the wrong one.
    semaphore s;; 

    sema_init(&s,0);
    
    //Add to list of waiters on the condition
    append_shared(c->waiters,&s);

    //Release lock
    lock_release(l);

    //Wait for s to become positive when signalled
    sema_down(&s);


    //require lock and return
    lock_acquire(l);
}


/* Signals to wakeup a thread waiting on c
 * Process must have ownership of lock l */
void cond_signal(condition* c, lock* l){
    ASSERT(c!=NULL, "Null cond in signal.");
    ASSERT(l!=NULL, "Null lock in signal.");
    // ASSERT(!in_external_int(),"In external interrupt");
    ASSERT(l->holder==current_proc(),"Lock not held by current thread in signal.");

    //wakeup the first waiter on cond c
    if(get_size(c->waiters))
        sema_up((semaphore*) pop_shared(c->waiters));
}


/* Signals all processes waiting on cond c to wakeup */
void cond_broadcast(condition* c, lock* l){
    ASSERT(c!=NULL, "Null cond in broadcast");
    ASSERT(l!=NULL, "Null lock in broadcast");

    while(get_size(c->waiters)){
        cond_signal(c,l);
    }
}
