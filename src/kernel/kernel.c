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


	//Splash screen
	// clear_screen();

	println("\nWelcome to...");
	println("");
	println("  _____                  _      ____   _____");
	println(" |  __ \\                | |    / __ \\ / ____|");
	println(" | |__) |   _ _ __   ___| |__ | |  | | (___  ");
	println(" |  ___/ | | | '_ \\ / __| '_ \\| |  | |\\___ \\ ");
	println(" | |   | |_| | | | | (__| | | | |__| |____) |");
	println(" |_|    \\__,_|_| |_|\\___|_| |_|\\____/ \\____/ ");
	println("");


	println("\nYou are currently in the PunchOS main branch.");

	println("\nThe 4 labs are available in git branches Lab1, Lab2, Lab3 and Lab4...");

	println("\nuse `git checkout' to move to a lab!");

	println("\nEnjoy!");


	println("\n\n\nKernel End Reached. Halting."); 

	//will return to boot.asm where it will disable interrupts and halt.
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