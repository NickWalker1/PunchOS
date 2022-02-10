#include "pg_swap_test.h"


/* Test for different variations of sizes ??? */

void pg_swap_test(){
    void *vaddr1=palloc_HDD();
    strcpy(vaddr1,"This is vaddr1");

    println(vaddr1);

    


}