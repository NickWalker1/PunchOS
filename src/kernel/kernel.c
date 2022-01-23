#include "kernel.h"

#include "../lib/math.h"

#include "heap.h"

int NUMTESTS=6;

uint32_t* KHEAP_ADDR;
uint32_t* KHEAP_ADDR_MAX;

/* Main entry point into the OS */
int kernel_main(uint32_t magic, uint32_t addr){
	print("Entering Kernel Code.");

	print_attempt("Boot process");
	if(!setup(magic, addr)){
		print_fail();
		halt();
	}
	print_ok();

	
	/* ------------- Lab 1 Code ------------- */

	//_KERNEL_END_ is a variable provided by the linker marking the end of all loaded data,
	// hence all data since is free for us to allocate as we wish.

	//Rounding up the end of the kernel loaded code to the nearest 4KiB boundary.
	KHEAP_ADDR=&_KERNEL_END_;
	KHEAP_ADDR=(uint32_t*)((uint32_t)KHEAP_ADDR-((uint32_t)KHEAP_ADDR%0x1000)+0x1000);
	KHEAP_ADDR_MAX = (uint32_t*)((uint32_t)KHEAP_ADDR+0x1000);

	println("HEAP START: ");
	print(itoa((uint32_t)KHEAP_ADDR,str,BASE_HEX));

	println("HEAP END:   ");
	print(itoa((uint32_t)KHEAP_ADDR_MAX,str,BASE_HEX));



	//call heap Init function
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);


	//Feel free to test your implementations here




	//test script to ensure malloc is performing correctly.
	//warning there is no check for a timeout, hence poor code may run indefinitely.
	testReport();



	println("Kernel End Reached. Halting."); 

	//will return to boot.asm where it will disable interrupts and halt.
	return 0;
}


/* Calls performTest() and displays results */
void testReport(){
	print_attempt("Testing Heap.");
	
	int mark=performTest(KHEAP_ADDR,KHEAP_ADDR_MAX); 

	if(mark==pow(2,NUMTESTS)-1){
		print_ok();
	}
	else{
		print_fail();
		int i;
		for(i=0;i<NUMTESTS;i++){
			if(!(mark&(1<<i))){
				println("Failed test: ");
				print(itoa(i+1,str,BASE_DEC));	
			}
		}
	}
}

/* Returns a bit map of heap tests passed. 1 for pass, 0 for fail.
 * WARNING: Tests not entirely exhaustive.
 */
int performTest(){
	int mark=0b111111;
	uint32_t test1,test2,test3,test4,test5;
	uint32_t MSH = sizeof(MemorySegmentHeader_t);



	//TEST 1: basic malloc

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test1 = (uint32_t) malloc(0x50);
	if(test1!=(uint32_t)KHEAP_ADDR+MSH) return 0; //fail all tests by default



	//TEST 2: Consecutive malloc

	clear_heap(KHEAP_ADDR);
	intialiseHeap(KHEAP_ADDR,KHEAP_ADDR_MAX);

	test1 = (uint32_t) malloc(0x50);
	test2 = (uint32_t) malloc(0x70);
	if(test2!=(test1+0x50+MSH)) mark=mark ^ 1<<1;



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
	
	heap_dump();


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
	free(test5);

	heap_dump();

	return mark;
}



/* Function to ensure multiboot header and memory loaded properly */
bool setup(uint32_t magic, uint32_t addr){
	if(magic!=MULTIBOOT_BOOTLOADER_MAGIC){
		println("Invalid multiboot header.");
		return false;
	}

	if(!validMemory(addr)){
		println("Memory unable to meet assumptions.");
		return false;
	}
	return true;
}

/* Ensures the memory map provided by multiboot meets the assumptions
 * made further on in the code.
 */
bool validMemory(uint32_t addr){
	multiboot_info_t *mbi = (multiboot_info_t *) addr;

	int count=0;

	bool okay=false;

	if(mbi->flags & 6){
		multiboot_memory_map_t *mmap;
		for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
           	(uint32_t) mmap < mbi->mmap_addr + mbi->mmap_length;
           	mmap = (multiboot_memory_map_t *) ((uint32_t) mmap
                                    + mmap->size + sizeof (mmap->size)))
		{
			count++;
			//check the memory meets some assumptions I have made later on in the OS
			//Assume: memory is free, it's larger than 1MiB, and starts at 1MiB
			if(mmap->type==1 && mmap->len>0x100000 && mmap->addr==0x100000) okay=true;
		}
	}
	return okay;
}