#include "ipc_test.h"

#include "../processes/process.h"
#include "../processes/ipc.h"


#include "../lib/screen.h"
#include "../lib/math.h"


lock test_lock;
condition test_cond;


size_t shm_mark = 0;
size_t mq_mark = 0;


char *blocka = "blocka";
char *blockb = "blockb";
char *blockc = "blockc";


/* Primary function for IPC test process.
 * WARNING: Tests are not exhaustive */
void IPC_test(){
    lock_init(&test_lock);
    cond_init(&test_cond);

	lock_acquire(&test_lock);

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

	/*-------- Shared Memory Tests --------*/
    shared_desc_t *desc_a = shm_open(blocka,O_CREAT);
    shared_desc_t *desc_b = shm_open(blockb,O_CREAT);
    shared_desc_t *desc_c = shm_open(blockc,O_CREAT);

	/* Test 1: Basic Creation */

	if(desc_a && desc_b && desc_c) shm_mark = shm_mark | 1<<0;



    void *ptr_a = mmap(desc_a);
    void *ptr_b = mmap(desc_b);
    void *ptr_c = mmap(desc_c);

	/* Test 2: Basic Write */	
    write(ptr_a,blocka,7);

	/* Test 3: Consecutive Write */

    write(ptr_b,blockb,7);
    ptr_b+=7;
    write(ptr_b,blockb,7);

	write(ptr_c, blockc,7);




	/*--------- Message Queue Tests --------*/



	/* Test 1: Creation */

	/* Test default attributes */
	mqd_t *mqdes_1 = mq_open("MQ1",NULL,O_CREAT);
	if(mqdes_1) mq_mark=mq_mark | 1<<0;

	/* Test 2 using attributes */

	mq_attr_t *attr= shr_malloc(sizeof(mq_attr_t));
	attr->mq_curmsgs=0;;
	attr->mq_msgsize=128;
	attr->mq_maxmsg=8;
	attr->mq_flags=0;

	mqd_t *mqdes_2 = mq_open("MQ2",attr,O_CREAT);
	if(mqdes_2->attr->mq_msgsize==128) mq_mark= mq_mark | 1<<1;

	/* Test 3: Generic Failure Cases */

	if(!mq_open("MQ1",NULL, O_CREAT) && !mq_open("A",NULL,0)) mq_mark = mq_mark | 1<<2;


	/* Test 4: Basic Send/Recieve */ 

	if(mq_send(mqdes_2,blockb,7)==7) mq_mark = mq_mark | 1<<3;



	/* Test 5: Repeated Send */

	mqd_t *mqdes_3 = mq_open("MQ3",attr,O_CREAT);

	int status=1;

	status = status & (mq_send(mqdes_3,blockc,7)==7);
	status = status & (mq_send(mqdes_3,blockb,7)==7);
	status = status & (mq_send(mqdes_3,blocka,7)==7);
	if(status) mq_mark = mq_mark | 1<<4;


	/* Test 6: Buffer wrap around */

	mqd_t *mqdes_4 = mq_open("MQ4",attr,O_CREAT);
	mq_send(mqdes_4,blocka,7);
	mq_send(mqdes_4,blocka,7);
	mq_send(mqdes_4,blocka,7);

	mq_recieve(mqdes_4,buffer,256);
	mq_recieve(mqdes_4,buffer,256);

	if(mqdes_4->write_hdr==3 && mqdes_4->read_hdr==2) mq_mark = mq_mark | 1<<5;


	
	/* Test 7: Test Full Queue*/
	status=1;
	for(int i=3;i<8;i++)status=status & mq_send(mqdes_3,blocka,7);
	if(status && !mq_send(mqdes_3,blockb,7)) mq_mark = mq_mark | 1<<6;

	/* Test 8: Failure on message size */
	status = mq_send(mqdes_3,blockc,2000000);
	if(!status) mq_mark = mq_mark | 1<<7;


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

	/* Test 4: Open descriptors */
	if(desc_a && desc_b && desc_c) shm_mark=shm_mark | 1<<3;


	/* Test 5: Can Read from 1 descriptor 
			   Also checks Test 2 validity.*/

	void *ptr_a = mmap(desc_a);
	read(buffer,ptr_a,7);
	if(strcmp(buffer,blocka)==0) shm_mark = shm_mark | 1<<4 | 1<<1;


	/* Test 6: Ensure independence.
			   Also checks Test 3 validity. */

	void *ptr_b = mmap(desc_b);
	void *ptr_c = mmap(desc_c);

	read(buffer,ptr_b,7);
	read(buffer2,ptr_c,7);

	if(strcmp(buffer,blockb)==0 && strcmp(buffer2,blockc)==0) shm_mark = shm_mark | 1<<5 | 1<<2;




	/*-------- Message Queue Tests --------*/

	/* Test 9: Basic Send/Recieve (Based on test 4 send)*/
	mqd_t *mqdes_2 = mq_open("MQ2",NULL,O_OPEN);

	mq_recieve(mqdes_2,buffer,256);
	if(strcmp(buffer,blockb)==0) mq_mark = mq_mark | 1<<8;


	/* Test 10: Repeated Recieve (Based on test 5 send) */

	mqd_t *mqdes_3 = mq_open("MQ3",NULL,O_OPEN);

	int status=1;
	int check=1;
	status = status & mq_recieve(mqdes_3,buffer,256);
	check  = check  & (strcmp(buffer,blockc)==0);
	status = status & mq_recieve(mqdes_3,buffer,256);
	check  = check  & (strcmp(buffer,blockb)==0);
	status = status & mq_recieve(mqdes_3,buffer,256);
	check  = check  & (strcmp(buffer,blocka)==0);
	if(check) mq_mark = mq_mark | 1<< 9;



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
