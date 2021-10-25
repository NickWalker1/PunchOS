#include "kernel.h"

/* Main entry point into the OS */
void kernel_main(uint32_t magic, uint32_t addr){
	print("Entering Kernel Code.");

	print_attempt("Boot process");
	if(magic!=MULTIBOOT_BOOTLOADER_MAGIC){
		println("Invalid multiboot header.");
		//TODO end execution
	}

	if(!validMemory(addr)){
		println("Memory unable to meet assumptions.");
		//TODO end exection
	}
	print_ok();


}

/* Ensures the memory map provided by multiboot meets the assumptions
 * made further on in the code.
 */
bool validMemory(uint32_t addr){
	multiboot_info_t *mbi = (multiboot_info_t *) addr;
	/* Print out the flags. */
  	// println ("flags =");
	// print(itoa((unsigned) mbi->flags,str,BASE_BIN));

	//check memory map
	int count=0;

	if(mbi->flags & 6){
		// println("Mmap addr: ");
		// print(itoa(mbi->mmap_addr,str,BASE_HEX));
		// println("Mmap length: ");
		// print(itoa(mbi->mmap_length,str,BASE_HEX));


		multiboot_memory_map_t *mmap;
		for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
           	(uint32_t) mmap < mbi->mmap_addr + mbi->mmap_length;
           	mmap = (multiboot_memory_map_t *) ((uint32_t) mmap
                                    + mmap->size + sizeof (mmap->size)))
		{
			count++;
			// println("Memory Entry: ");
			// print(itoa(count,str,BASE_DEC));

			// println("Type: ");
			// print(itoa(mmap->type,str,BASE_DEC));

			// println("Addr: ");
			// print(itoa(mmap->addr,str,BASE_HEX));

			// println("Length: ");
			// print(itoa(mmap->len,str,BASE_HEX));

			//check the memory meets some assumptions I have made later on in the OS
			//Assume: memory is free, it's larger than 1MiB, and starts at 1MiB
			if(mmap->type==1 && mmap->len>0x100000 && mmap->addr==0x100000) return true;
		}
	}
	return false;
}