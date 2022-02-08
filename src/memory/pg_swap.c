#include "pg_swap.h"

/* Sets the accesssed state bits to 0 on all pages */
void access_reset(){

}


/* This function is called directly from the generic exception handler in idt.c.
 */
void page_fault_handler(exception_state *state){

    /* To be implemented */
    char msg[128];
    strcpy(msg, "Page fault on virtual address: ");
    itoa(state->cr2,msg+strlen(msg),BASE_HEX);

    
    // PANIC(msg);


}