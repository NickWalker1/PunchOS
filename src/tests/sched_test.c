#include "sched_test.h"

#include "../threads/thread.h"

#include "../lib/time.h"
#include "../lib/screen.h"

uint8_t completedA=0;
uint8_t completedB=0;
uint8_t completedC=0;
uint8_t completedD=0;

condition start_cond;
lock start_lock;

void test_func_a(){
    /* Wait for start signal */
    lock_acquire(&start_lock);
    cond_wait(&start_cond,&start_lock);
    lock_release(&start_lock);


    for(int i=0;i<10;i++){
        thread_sleep(8,UNIT_TICK);
        malloc(50);
    }
    completedA++;
    println("A completed.");
}


/* Really hard *work* */
void test_func_b(){
    /* Wait for start signal */
    lock_acquire(&start_lock);
    cond_wait(&start_cond,&start_lock);
    lock_release(&start_lock);

    uint32_t a =0;
    int i,j,k;
    for(i=0;i<500;i++){
        for(j=0;j<1000;j++){
            for(k=0;k<500;k++){
                a+=5;
            }
        }
    }
    completedB++;
    println("B completed.");
}


/* Hard work with sleeps */
void test_func_c(){
    /* Wait for start signal */
    lock_acquire(&start_lock);
    cond_wait(&start_cond,&start_lock);
    lock_release(&start_lock);


    uint32_t a =0;
    int i,j,k;
    for(i=0;i<50;i++){
        thread_sleep(1,UNIT_TICK);
        for(j=0;j<10000;j++){
            for(k=0;k<1000;k++){
                a+=5;
            }
        }
    }
    completedC++;
    println("C completed.");
}


void test_func_d(){
    /* Wait for start signal */
    lock_acquire(&start_lock);
    cond_wait(&start_cond,&start_lock);
    lock_release(&start_lock);


    uint32_t a =0;
    int i,j,k;
    for(i=0;i<20;i++){
        thread_sleep(2,UNIT_TICK);
        for(j=0;j<10000;j++){
            for(k=0;k<1000;k++){
                a+=5;
            }
        }
    }
    completedD++;
    println("D completed.");
}

/* Returns true if all threads have finished execution. */
bool end_cond(){
    return completedA==4 && completedB==2 && completedC==1 && completedD==2;
}



/* Test function for validating performance improvements of MLFQ. 
 * For test validity please crate a new thread to run this and ensure no extra threads running 
 * other than the init thread. */
void scheduling_test_func(){
    print_attempt("Scheduling test setup.");
    bool timeout=false;


    lock_init(&start_lock);
    cond_init(&start_cond);    

    TCB_t *a1 = thread_create("A1",test_func_a,NULL,1,0);
    TCB_t *a2 = thread_create("A2",test_func_a,NULL,1,0);
    TCB_t *a3 = thread_create("A3",test_func_a,NULL,1,0);
    TCB_t *a4 = thread_create("A4",test_func_a,NULL,1,0);

    TCB_t *b1 = thread_create("B1",test_func_b,NULL,1,0);
    TCB_t *b2 = thread_create("B2",test_func_b,NULL,1,0);

    TCB_t *c = thread_create("C",test_func_c,NULL,1,0);

    TCB_t *d1 = thread_create("D1",test_func_d,NULL,1,0);
    TCB_t *d2 = thread_create("D2",test_func_d,NULL,1,0);

    if(!a1 || !a2 || !a3 || !a4 || !b1 || !b2 || !c || !d1 || !d2){
        print_fail();
        println("Failed to create all threads.");
        
        thread_attempt_kill(a1);
        thread_attempt_kill(a2);
        thread_attempt_kill(a3);
        thread_attempt_kill(a4);
        thread_attempt_kill(b1);
        thread_attempt_kill(b2);
        thread_attempt_kill(c);
        thread_attempt_kill(d1);
        thread_attempt_kill(d2);
        return;
    }

    print_ok();

    /* Wait for all threads to be waiting on a start signal */
    while(get_size(start_cond.waiters)!=9) 
        thread_sleep(2,UNIT_TICK);
    

    /* All threads are ready */
    print_attempt("Running tests...");

    /* Best chance at getting them to start as close as they can to each other. As none can start until all are ready. */
    lock_acquire(&start_lock);
    cond_broadcast(&start_cond,&start_lock);
    lock_release(&start_lock);

    int t1= get_time(); /* Start clock */

    while(!end_cond()){
        thread_sleep(2,UNIT_TICK);
        if((get_time()-t1)>20*TICK_PS){
            timeout=true;
            break;
        }
    }

    int t2=get_time();
    if(timeout){
        print_fail();
        println("Timeout failure. Check for thread starvation.");

        thread_attempt_kill(a1);
        thread_attempt_kill(a2);
        thread_attempt_kill(a3);
        thread_attempt_kill(a4);
        thread_attempt_kill(b1);
        thread_attempt_kill(b2);
        thread_attempt_kill(c);
        thread_attempt_kill(d1);
        thread_attempt_kill(d2);

        return;
    }


    println("Time taken: ");
    int time =calc_time(t2-t1);
    print(itoa(time,str,BASE_DEC));
    print(" seconds.");

    if(time<20){ 
        print_pass();
    }
    else{
        print_fail();
    }

    println("\nNaive scheduling time is 20 seconds.\n");
}