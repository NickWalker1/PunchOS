#include "sched_test.h"

#include "../threads/thread.h"

void test_func_a(){
    for(int i=0;i<10;i++){
        thread_sleep(5,UNIT_TICK);
        malloc(50);
    }
    println("A completed.");
}

/* Really hard *work* */
void test_func_b(){
    uint32_t a =0;
    int i,j,k;
    for(i=0;i<10000;i++){
        for(j=0;j<10000;j++){
            for(k=0;k<100;k++){
                a+=5;
            }
        }
    }
    println("B completed.");
}

/* Hard work with sleeps */
void test_func_c(){
    uint32_t a =0;
    int i,j,k;
    for(i=0;i<1000;i++){
        thread_sleep(2,UNIT_TICK);
        for(j=0;j<10000;j++){
            for(k=0;k<1000;k++){
                a+=5;
            }
        }
    }
    println("C completed.");
}

void test_func_d(){

}


/* For test validity please call from init thread with no extra threads running */
void scheduling_test(){

}