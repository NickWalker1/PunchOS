#include "pg_swap_test.h"


/* Test for different variations of sizes ??? */

void pg_swap_test(){
    void *paddr1=palloc_HDD();
    // strcpy(paddr1,"This is vaddr1");

    println(itoa(paddr1,str,BASE_HEX));

    


}