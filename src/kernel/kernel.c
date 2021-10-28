#include "kernel.h"

/* Main entry point into the OS */
int kernel_main(uint32_t magic, uint32_t addr){
	print("Entering Kernel Code.");

	print_attempt("Boot process");
	if(!setup(magic, addr)){
		println(itoa(magic,str,BASE_HEX));
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

	return 0;
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