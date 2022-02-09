#include "pg_swap.h"


#include "paging.h"

#include "../processes/pcb.h"


extern uint32_t *phys_page_pool_end;

/* Need some form of a tracker for RAM and HDD 
 * All pages must always be in HDD as well as RAM as if not modified can just be overwritten without copy
 * Page stored must also have a virtual adress associated with it and the owning process for lookup purposes
 *      As must be able to check valid Vaddr for the process and not just complete garbage. 
 * 
 * when swapping out mustn't forget to set that page's table entry present bit 0 otherwise will be faulty read/writes
 *      Must have proper check for this in the test environment.
 */



void *virt_HDD_start;
void *virt_RAM_start;

typedef struct swap_page{
    void *vaddr;
    p_id owner_pid;
    void *swap_paddr; /* This shouldn't change throughout the lifetime of the page */
} swap_page_t; 




bool HDD_status[virt_HDD_size];
bool RAM_status[virt_RAM_size];

swap_page_t page_swap_tracker[virt_HDD_size];

/* Copies the given page from the source to the destination given the PHYSICAL addresses */
void phys_page_copy(void *dest, void *src){
    memcpy(Kptov(dest),Kptov(src),PGSIZE);
}


/* Allocates a page in virtual HDD memory, and notably does NOT add anything to RAM.
 * Pages are only added to the virt_RAM on pagefault (demand paging) */
void *palloc_HDD(){
    int i=0;
    while(HDD_status[i]&& i<virt_HDD_size) i++;
    if(i==virt_HDD_size) {
        KERN_WARN("HDD full");
        return NULL;
    }
    HDD_status[i]=true;
    page_swap_tracker[i].owner_pid=current_proc()->pid;
    page_swap_tracker[i].vaddr=get_next_free_virt_page(1,F_ASSERT);
    page_swap_tracker[i].swap_paddr=virt_HDD_start+i*PGSIZE;

    return page_swap_tracker[i].vaddr;

}

/* Must allocate some pages for simulated RAM environment */
void virt_RAM_init(){
    /*Some pages between:
     *  phys_page_pool_end and phys_page_pool_end + virt_RAM_size * PGSIZE
     */
    virt_RAM_start=phys_page_pool_end;

}


/* Must allocate some pages for simulated HDD environment */
void virt_HDD_init(){
    /*Some pages between:
     *  phys_page_pool_end + virt_RAM_size*PGSIZE and phys_page_pool_end + (virt_RAM_size+virt_HDD_size) * PGSIZE
     */     
    virt_HDD_start=virt_RAM_size+virt_RAM_size*PGSIZE;

}

/* Sets the accesssed state bits to 0 on all pages */
void access_reset(){
    //For simplicitity can just set every present one to 0 rather than one ones that are releveant to us. 
    /* To be implemented. */
}


/* This function is called directly from the generic exception handler in idt.c.
 */
void page_fault_handler(exception_state *state){

    /* To be implemented */

    /* Replace this */
    char msg[128];
    strcpy(msg, "Page fault on virtual address: ");
    itoa(state->cr2,msg+strlen(msg),BASE_HEX);

    
    PANIC(msg);


}