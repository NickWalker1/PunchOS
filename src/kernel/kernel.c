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
	

	//Create a test process
	create_proc("A",proc_test_A,NULL);
	

	println("Welcome to...");
	println("");
	println(" |  __ \\                | |    / __ \\ / ____|");
	println(" | |__) |   _ _ __   ___| |__ | |  | | (___  ");
	println(" |  ___/ | | | '_ \\ / __| '_ \\| |  | |\\___ \\ ");
	println(" | |   | |_| | | | | (__| | | | |__| |____) |");
	println(" |_|    \\__,_|_| |_|\\___|_| |_|\\____/|_____/ ");
	println("");
                      

	/* Active loop to keep interrupts going. */
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


	void *heap_addr=palloc_kern(4,F_ASSERT);
	intialiseHeap(heap_addr,heap_addr+(HEAP_SIZE*PGSIZE));


	processes_init();


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
