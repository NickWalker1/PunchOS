#include "kernel.h"

/* Main entry point into the OS */
int kernel_main(uint32_t magic, uint32_t addr){
	print("Entering Kernel Code.");

	print_attempt("Boot process");
	if(!setup(magic, addr)){
		println("Magic recieved: ");
		print(itoa(magic,str,BASE_HEX));
		print_fail();
		halt();
	}
	print_ok();

	tlb_flush();
	//get_next_free_phys_page(2,0);

	//println(itoa(get_next_free_phys_page(1, F_ASSERT),str,BASE_HEX));
	//halt();
	int *x = 0x00006000;
	//println(itoa(pd_no(x),str,BASE_DEC));
	//halt();
	map_page((void*)x,(void*)x,F_VERBOSE);
	*x=50;
	println(itoa(*x,str,BASE_DEC));
	// halt();
	// // unmap_page(x);
	// // tlb_flush();
	// *x=69;
	// println(itoa(*x,str,BASE_DEC));
	halt();

	/* Active loop to keep interrupts going. */
	println("Main loop.");
	while(1);


	return 0;
}

/* Function to ensure multiboot header and memory loaded properly */
bool setup(uint32_t magic, uint32_t addr){
	if(magic!=MULTIBOOT_BOOTLOADER_MAGIC){
		println("Invalid multiboot header.");
		return false;
	}
	
	gdt_init();

	if(!validMemory(addr)){
		println("Memory unable to meet assumptions.");
		return false;
	}

	idt_init();

	paging_init();

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
