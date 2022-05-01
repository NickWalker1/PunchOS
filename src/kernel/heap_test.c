#include "heap_test.h"


#include "heap.h"

#include "../lib/math.h"


int NUMTESTS=6;


extern uint32_t* KHEAP_ADDR;
extern uint32_t* KHEAP_ADDR_MAX;




int mark=0;

/* Test descriptions */
char test_descriptions[][32]={
	"Basic malloc",
    "Consecutive malloc",
    "Failure on size",
    "free inside 2 segments",
    "free combines segments",
    "free 2 electric boogaloo"
};








/* Returns a bit map of heap tests passed. 1 for pass, 0 for fail.
 * WARNING: Tests not entirely exhaustive.
 */
void perform_test(){
	uint32_t test1,test2,test3,test4;
	uint32_t MSH = sizeof(MemorySegmentHeader_t);



	//TEST 1: basic malloc

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test1 = (uint32_t) malloc(0x50);
	if(test1!=(uint32_t)KHEAP_ADDR+MSH) return; //fail all tests by default
	
	mark = 1;


	//TEST 2: Consecutive malloc

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test1 = (uint32_t) malloc(0x50);
	test2 = (uint32_t) malloc(0x70);
	if(test2==(test1+ 0x50 + MSH)) mark=mark | 1<<1;


	//TEST 3: Failure on size

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test3 = (uint32_t) malloc(0x1001);
	if(!test3) mark=mark | 1<<2;



	//TEST 4: Free inside two segments

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test1= (uint32_t) malloc(0x50);
	test2= (uint32_t) malloc(0x70);
	malloc(0x50);
	free((void*)test2);
	test3 = (uint32_t) malloc(0x40);
	if(test3==(test1+0x50+MSH)) mark=mark | 1<<3;



	//TEST 5: Free combines consecutive segments

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test1=(uint32_t)malloc(0x50);
	test2=(uint32_t)malloc(0x50);
	test3=(uint32_t)malloc(0x50);
	malloc(0x50);
	free((void*)test2);
	free((void*)test3);
	test4= (uint32_t) malloc(0x60);
	if(test4==(test1+0x50+MSH)) mark=mark | 1<<4;
	

	//TEST 6: Free 2 electric boogaloo

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);
	test1=(uint32_t)malloc(0x50);
	test2=(uint32_t)malloc(0x50);
	test3=(uint32_t)malloc(0x50);
	malloc(0x50);
	free((void*)test3);
	free((void*)test2);
	test4= (uint32_t) malloc(0x70);
	if(test4==(test1+0x50+MSH)) mark=mark | 1<<4;
	

}


/* Calls performTest() and displays results */
void test_report(){
	print("\n");
	print_attempt("Testing Heap...");
	
	if(mark==pow(2,NUMTESTS)-1){
		print_pass();
	}
	else{
		print_fail();
	}
	int i;
	for(i=0;i<NUMTESTS;i++){
		if(!(mark&(1<<i))){
			println("");
			print_fail_generic();
			print(" test: ");
			print(itoa(i+1,str,BASE_DEC));
			print(" ");
			print(test_descriptions[i]);
		}
		else{
			println("");
			print_pass_generic();
			print(" test: ");
			print(itoa(i+1,str,BASE_DEC));
			print(" ");
			print(test_descriptions[i]);
		}
	}
	
}


/* Perform and report on the outcome of the tests */
void heap_tests(){
    perform_test(KHEAP_ADDR,KHEAP_ADDR_MAX);
    test_report();
}