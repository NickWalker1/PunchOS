#include "heap_test.h"

#include "../lib/math.h"

int NUMTESTS=6;

char test_descriptions[][32]={
	"Waa",
	"Woop",
	"Yaa",
	"yeet",
	"yote",
	"yope"
};



extern uint32_t* KHEAP_ADDR;
extern uint32_t* KHEAP_ADDR_MAX;

/* Calls performTest() and displays results */
void test_report(){
	print_attempt("Testing Heap.");
	
	int mark=perform_test(KHEAP_ADDR,KHEAP_ADDR_MAX); 

	if(mark==pow(2,NUMTESTS)-1){
		print_ok();
	}
	else{
		print_fail();
	}
	int i;
	for(i=0;i<NUMTESTS;i++){
        print("\n");
		if(!(mark&(1<<i))){
			print_fail_generic();
		}
		else{
			print_pass_generic();
		}
		print(itoa(i+1,str,BASE_DEC));
		print(" - ");
		print(test_descriptions[i]);
	}
}

/* Returns a bit map of heap tests passed. 1 for pass, 0 for fail.
 * WARNING: Tests not entirely exhaustive.
 */
int perform_test(){
	int mark=0b111111;
	uint32_t test1,test2,test3,test4,test5;
	uint32_t MSH = sizeof(MemorySegmentHeader_t);



	//TEST 1: basic malloc

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);
	// heap_dump();

	test1 = (uint32_t) malloc(0x50);
	if(test1!=(uint32_t)KHEAP_ADDR+MSH) return 0; //fail all tests by default
	// heap_dump();



	//TEST 2: Consecutive malloc

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test1 = (uint32_t) malloc(0x50);
	test2 = (uint32_t) malloc(0x70);
	if(test2!=(test1+0x50+MSH)) mark=mark ^ 1<<1;
	// heap_dump();


	//TEST 3: Failure on size

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test3 = (uint32_t) malloc(0x1001);
	if(test3) mark=mark ^ 1<<2;



	//TEST 4: Free inside two segments

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test1= (uint32_t) malloc(0x50);
	test2= (uint32_t) malloc(0x70);
	malloc(0x50);
	free((void*)test2);
	test3 = (uint32_t) malloc(0x40);
	if(test3!=(test1+0x50+MSH)) mark=mark ^1<<3;



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
	if(test4!=(test1+0x50+MSH)) mark=mark ^1<<4;
	

	//TEST 6: Free again

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);
	test1=(uint32_t)malloc(0x50);
	test2=(uint32_t)malloc(0x50);
	test3=(uint32_t)malloc(0x50);
	malloc(0x50);
	free((void*)test3);
	free((void*)test2);
	test4= (uint32_t) malloc(0x70);
	if(test4!=(test1+0x50+MSH)) mark=mark ^1<<4;
	
	// heap_dump();


	//TEST 7: Free again

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test1=(uint32_t)malloc(0x50);
	test2=(uint32_t)malloc(0x50);
	test3=(uint32_t)malloc(0x50);
	test4=(uint32_t)malloc(0x50);
	malloc(0x50);
	free((void*)test2);
	free((void*)test4);
	free((void*)test3);
	test5= (uint32_t) malloc(0x150+2*MSH);
	free((void*)test5);

	// heap_dump();

	return mark;
}