#include "kernel.h"


/* Entry point into the OS */
void kernel_entry(uint32_t magic, uint32_t addr){
	print("Entering Kernel Code.");

	print_attempt("Boot process");
	if(!setup(magic, addr)){
		println("Magic recieved: ");
		print(itoa(magic,str,BASE_HEX));
		print_fail();
		halt();
	}
	print_ok();



	/* Final job of setup is to start multiprocesing.
	Must be final as this function will not return */
	multi_proc_start();

	/* UNREACHABLE */
	PANIC("Reached unreachable position.");
}


/* Main function run by init process */
void main(){

	
	//Sleep to display bootscreen.
	proc_sleep(1,UNIT_SEC);

	
	//Splash screen
	clear_screen();

	print("Welcome to...");
	draw_PUNCHOS();


	proc_sleep(1, UNIT_SEC);

	// set_rseed(50);

	// while(true){
	// 	println(itoa(rand(),str,BASE_DEC));
	// 	proc_sleep(1,UNIT_SEC);
	// }

	/* Perform the message queue tests */
	MQ_test();
	// spinning_bars();


	spin(TOP_RIGHT);

}

#define SCREENSIZE (80*24*2)

void spinning_bars(){
	clear_screen();

	set_rseed(38); //Some random number to start the seed

	/* To be implemented. */


}


/* Little function that causes a bar to spin indefinitely...
 * NOTE: Highly inefficient as requires lots of context switches */
void spin(int offset){
	char spinBars[] = {'|','/','-','\\'};
	if(offset%2)offset--;

	uint8_t i=0;
	while(1){
		print_char_offset(spinBars[i++%4],WHITE_ON_BLACK,offset);
		proc_sleep(4,UNIT_TICK);
	}
		
}


/* Prints the logo to the screen with a series of printlns */
void draw_PUNCHOS(){
	println("");
	println("  _____                  _      ____   _____");
	println(" |  __ \\                | |    / __ \\ / ____|");
	println(" | |__) |   _ _ __   ___| |__ | |  | | (___  ");
	println(" |  ___/ | | | '_ \\ / __| '_ \\| |  | |\\___ \\ ");
	println(" | |   | |_| | | | | (__| | | | |__| |____) |");
	println(" |_|    \\__,_|_| |_|\\___|_| |_|\\____/ \\____/ ");
	println("");

}


/* Function to ensure multiboot header and memory loaded properly */
bool setup(uint32_t magic, uint32_t addr){
	if(magic!=MULTIBOOT_BOOTLOADER_MAGIC){
		println("Invalid multiboot header.");
		return false;
	}
	
	print_attempt("GDT init.");
	gdt_init();
	print_ok();


	if(!validate_memory(addr)){
		println("Memory unable to meet assumptions.");
		return false;
	}


	print_attempt("IDT init.");
	idt_init();
	print_ok();


	print_attempt("Paging init.");
	paging_init();
	print_ok();


	print_attempt("Processes init.");
	processes_init();
	print_ok();



	return true;
}


/* Ensures the memory map provided by multiboot meets the assumptions
 * made further on in the code.
 */
bool validate_memory(uint32_t addr){
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



/*
	for(int i=0;i<5;i++){
		create_proc(itoa(i,str,BASE_DEC),spin,(void *) (rand()%SCREENSIZE));
	}
*/