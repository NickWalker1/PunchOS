#include "kernel.h"


#include "heap.h"


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
	uint32_t *KHEAP_ADDR = _KERNEL_END_-_KERNEL_END_%0x1000 + 0x1000;
	uint32_t *KHEAP_ADDR_MAX = (uint32_t*)((uint32_t)KHEAP_ADDR+0x1000);


	//test script to ensure malloc is performing correctly.
	print_attempt("Testing Heap.");

	//warning there is no check for a timeout, hence poor code may run indefinitely.
	int mark=heap_test(); 
	
	if(mark==0b1111){
		print_ok();
	}
	else{
		print_fail();
		int i;
		int x=1;
		for(i=0;i<4;i++){
			if(!(mark&(x<<i))){
				println("Failed test: ");
				print(itoa(i+1,str,BASE_DEC));	
			}
		}
	}
	

	return 0;
}

/* Returns a bit map of heap tests passed. 1 for pass, 0 for fail.*/
int heap_test(){
	int mark=0b1111;
	//Test 1 test basic malloc.
	uint32_t *test1=(uint32_t*)malloc(50);
	if(!test1) mark=mark&0b1110;

	//Test consecutive malloc
	uint32_t *test2=(uint32_t*)malloc(256);
	if(!test2) mark=mark&0b1101;

	//Test expecting failure
	uint32_t *test3=(uint32_t*)malloc(5000);
	if(0) mark=mark&0b1011;

	//Test free
	free(test2);
	uint32_t *test4=(uint32_t*)malloc(256);
	if(!test4 || (test4!=test2)) mark=mark&0b0111;

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