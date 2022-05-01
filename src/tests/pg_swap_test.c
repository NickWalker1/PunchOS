#include "pg_swap_test.h"


#include "../lib/math.h"
#include "../lib/debug.h"



#include "../memory/pg_swap.h"
/* Fill out some virtHDD with data, with palloc_HDD, then randomly request it, forcing swaps out, sometimes writing sometimes just reading */
/* Performance analysis would be good too. */

void perform_pg_swap_test();
void test_report();

void *vaddrs[virt_HDD_size];

int pg_swap_mark = 0b000;

/* Test descriptions */
char test_descriptions[][32]={
    "Read/write",
    "Write back",
    "You had fun!"
};




void pg_swap_test(){
    if(virt_RAM_size!=16)
        PANIC("pg swap test requires 16 virt RAM pages...");


    perform_pg_swap_test();

    test_report();
}


void perform_pg_swap_test(){
    /* Populate the virt HDD */
    for(int i=0;i<virt_HDD_size;i++){
        vaddrs[i]=palloc_HDD();
    }

    /* Test read/write */
    *(char*)vaddrs[15]='x';
    if(*(char*)vaddrs[15]!='x'){
        return;
    }

    //If not broken, must be okay...
    pg_swap_mark = pg_swap_mark | 1;
    



    /* Test for write back */


    /* Linear writes - will force capacity swap out */
    for(int i=0;i<virt_HDD_size;i++){
        *(int*)vaddrs[i]=i; //Write the HDD index to each page.
    }
   
    /* Check linear writes - reload pages and check contents */
    bool okay = true;
    for(int i=0;(i<virt_HDD_size && okay);i++){
        if(*(int*)vaddrs[i]!=i){
            okay=false;
        }
    }
    if(okay) pg_swap_mark = pg_swap_mark | 1<<1;



    /* Pseudo random queries */ 

    



    /* Final Joke */
    if(pg_swap_mark & 0b11)
        pg_swap_mark = pg_swap_mark | 1<<2;
}


/* Displays results of tests. */
void test_report(){

	if(pg_swap_mark==(size_t)pow(2,NUM_TESTS)-1){
		print_pass();
	}
	else{
		print_fail();
	}
		int i;
		for(i=0;i<NUM_TESTS;i++){
			if(!(pg_swap_mark&(1<<i))){
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