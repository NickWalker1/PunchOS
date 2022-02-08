#include "pg_swap.h"

extern uint32_t *phys_page_pool_end;

/* Need some form of a tracker for RAM and HDD 
 * All pages must always be in HDD as well as RAM as if not modified can just be overwritten without copy
 * Page stored must also have a virtual adress associated with it and the owning process for lookup purposes
 *      As must be able to check valid Vaddr for the process and not just complete garbage. 
 * 
 * when swapping out mustn't forget to set that page's table entry present bit 0 otherwise will be faulty read/writes
 *      Must have proper check for this in the test environment.
 */

/* Must allocate some pages for simulated RAM environment */
void virt_RAM_init(){
    /*Some pages between:
     *  phys_page_pool_end and phys_page_pool_end + virt_RAM_size * PGSIZE
     */

}


/* Must allocate some pages for simulated HDD environment */
void virt_HDD_init(){
    /*Some pages between:
     *  phys_page_pool_end + virt_RAM_size*PGSIZE and phys_page_pool_end + (virt_RAM_size+virt_HDD_size) * PGSIZE
     */     

}

/* Sets the accesssed state bits to 0 on all pages */
void access_reset(){
    //For simplicitity can just set every present one to 0 rather than one ones that are releveant to us. 
}


/* This function is called directly from the generic exception handler in idt.c.
 */
void page_fault_handler(exception_state *state){

    /* To be implemented */
    char msg[128];
    strcpy(msg, "Page fault on virtual address: ");
    itoa(state->cr2,msg+strlen(msg),BASE_HEX);

    
    PANIC(msg);


}