#include "ipc_test.h"

#include "../processes/process.h"
#include "../processes/ipc.h"


#include "../lib/screen.h"
#include "../lib/math.h"


lock test_lock;
condition test_cond;


uint8_t shm_mark = 0;
uint8_t  mq_mark = 0;


char *blocka = "blocka";
char *blockb = "blockb";
char *blockc = "blockc";


/* Primary function for IPC test process */
void IPC_test(){
    lock_init(&test_lock);
    cond_init(&test_cond);
    create_proc("PROD",producer,NULL,PC_NFLAG);
    create_proc("CONS",consumer,NULL,PC_NFLAG);


	/* Wait for consumer to finish testing */
    cond_wait(&test_cond,&test_lock);

    test_report();
}


/* Producer process */
void producer(){
    lock_acquire(&test_lock);
    
    shm_init();
    mq_init();

    shared_desc_t *desc_a = shm_open(blocka,O_CREAT);
    shared_desc_t *desc_b = shm_open(blockb,O_CREAT);
    shared_desc_t *desc_c = shm_open(blockc,O_CREAT);

    void *ptr_a = mmap(desc_a);
    void *ptr_b = mmbp(desc_b);
    void *ptr_c = mmcp(desc_c);

    write(ptr_a,blocka,7);

    write(ptr_b,blockb,7);
    ptr_b+=7;
    write(ptr_b,blockb,7);

    
    write(ptr_c,blockc,7);

    lock_release(&test_lock);
}


/* Consumer process */
void consumer(){
    lock_acquire(&test_lock);

	/* Shared Memory Tests */

    shared_desc_t *desc_a = shm_open(blocka,O_OPEN);
    shared_desc_t *desc_b = shm_open(blockb,O_OPEN);
    shared_desc_t *desc_c = shm_open(blockc,O_OPEN);

	/* Test 1: Open descriptors */
	if(!desc_a || !desc_b || !desc_c){
		mq_mark = shm_mark=0;
		cond_signal(&test_cond,&test_lock);
		lock_release(&test_lock);
		return;
	}
	shm_mark=shm_mark | 1<<0;

	void *buffer = malloc(256);

	/* Test 2: Can Read from 1 descriptor */

	void *ptr_a = mmap(desc_a);
	read(buffer,ptr_a,7);
	if(strcmp(buffer,blocka)==0) shm_mark = shm_mark | 1<<1;


	

	/* Message Queue Tests */

	/* Test 1: Creation */

	/* Test 2: Basic Send */ 

	/* Test 3: Test buffer wrap around */

    cond_signal(&test_cond,&test_lock);
    lock_release(&test_lock);
}


/* Calls performTest() and displays results */
void test_report(){
	print_attempt("Testing Shared Memory");

	if(shm_mark==pow(2,NUM_TESTS)-1){
		print_ok();
	}
	else{
		print_fail();
		int i;
		for(i=0;i<NUM_TESTS;i++){
			if(!(shm_mark&(1<<i))){
				println("Failed test: ");
				print(itoa(i+1,str,BASE_DEC));	
			}
		}
	}

    print_attempt("Testing Message Queue");

	if(mq_mark==pow(2,NUM_TESTS)-1){
		print_ok();
	}
	else{
		print_fail();
		int i;
		for(i=0;i<NUM_TESTS;i++){
			if(!(mq_mark&(1<<i))){
				println("Failed test: ");
				print(itoa(i+1,str,BASE_DEC));	
			}
		}
	}

}

int perform_IPC_test(){

}