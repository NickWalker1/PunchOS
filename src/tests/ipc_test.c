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

	void *buffer=malloc(256);

    shared_desc_t *desc_a = shm_open(blocka,O_CREAT);
    shared_desc_t *desc_b = shm_open(blockb,O_CREAT);
    shared_desc_t *desc_c = shm_open(blockc,O_CREAT);

    void *ptr_a = mmap(desc_a);
    void *ptr_b = mmap(desc_b);
    void *ptr_c = mmap(desc_c);

    write(ptr_a,blocka,7);

    write(ptr_b,blockb,7);
    ptr_b+=7;
    write(ptr_b,blockb,7);

	/* Message Queue Tests */

	/* Test 1: Creation */

	/* Test default attributes */
	mqd_t *mqdes_1 = mq_open("MQ1",NULL,O_CREAT);
	if(!mqdes_1) return;


	mq_attr_t *attr= shr_malloc(sizeof(mq_attr_t));
	attr->mq_curmsgs=0;;
	attr->mq_msgsize=128;
	attr->mq_maxmsg=8;
	attr->mq_flags=0;

	/* Test using attributes */
	mqd_t *mqdes_2 = mq_open("MQ2",attr,O_CREAT);
	mqdes_2->attr->mq_msgsize=128;

	/* Test failure */

	mqd_t *mqdes_3 = mq_open("MQ2",NULL, O_CREAT);
	if(mqdes_1 && mqdes_2 && !mqdes_3) mq_mark = mq_mark | 1<<0;


	/* Test 2: Basic Send/Recieve */ 

	mq_send(mqdes_2,blocka,7);



	/* Test 3: Repeated send */

	mq_send(mqdes_3,blocka,7);
	mq_send(mqdes_3,blockb,7);
	mq_send(mqdes_3,blockc,7);

	/* Test 4: Buffer wrap around */

	mq_recieve(mqdes_3,buffer,7);
	mq_recieve(mqdes_3,buffer,7);

	int status =1;
	for(int i=0;i<7;i++){
		status = status & mq_send(mqdes_3,blocka,7);
	}
	
	/* Test 5: Test Full */

	mq_send(mqdes_2,blockb,7);
	mq_send(mqdes_2,blockb,7);
    
    write(ptr_c,blockc,7);

	/* Test failure on size */
	status = mq_send(mqdes_3,blockc,2000000);
	if(!status) mq_mark = mq_mark | 1<<2;

    lock_release(&test_lock);
}


/* Consumer process */
void consumer(){
    lock_acquire(&test_lock);

	void *buffer = malloc(256);
	void *buffer2= malloc(256);

	/*-------- Shared Memory Tests --------*/

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


	/* Test 2: Can Read from 1 descriptor */

	void *ptr_a = mmap(desc_a);
	read(buffer,ptr_a,7);
	if(strcmp(buffer,blocka)==0) shm_mark = shm_mark | 1<<1;


	/* Test 3: Ensure virtual address space independence */
	void *ptr_b = mmap(desc_b);
	void *ptr_c = mmap(desc_c);

	read(buffer,ptr_b,7);
	read(buffer,ptr_c,7);

	if(strcmp(buffer,buffer2)==0) shm_mark = shm_mark | 1<<2;




	/*-------- Message Queue Tests --------*/

	/* Test 2: Basic Send/Recieve */


	/* Test 3: Repeated send */
	mqd_t *mqdes_3 = mq_open("MQ2",NULL,O_OPEN);

	int status = mq_recieve(mqdes_3,buffer,7);
	if(strcmp(buffer,blockc)==0) mq_mark = mq_mark | 1<<1;


	/* Test 4: Buffer wrap around */

	/* Test 5: Test Full */



    cond_signal(&test_cond,&test_lock);
    lock_release(&test_lock);
}


/* Calls performTest() and displays results */
void test_report(){
	print_attempt("Testing Shared Memory");

	if(shm_mark==pow(2,SHM_NUM_TESTS)-1){
		print_ok();
	}
	else{
		print_fail();
		int i;
		for(i=0;i<SHM_NUM_TESTS;i++){
			if(!(shm_mark&(1<<i))){
				println("Failed test: ");
				print(itoa(i+1,str,BASE_DEC));	
			}
		}
	}

    print_attempt("Testing Message Queue");

	if(mq_mark==pow(2,MQ_NUM_TESTS)-1){
		print_ok();
	}
	else{
		print_fail();
		int i;
		for(i=0;i<MQ_NUM_TESTS;i++){
			if(!(mq_mark&(1<<i))){
				println("Failed test: ");
				print(itoa(i+1,str,BASE_DEC));	
			}
		}
	}

}
