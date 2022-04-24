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

/* Array of bools indicating if a page is present at that index */
bool HDD_status[virt_HDD_size];
bool RAM_status[virt_RAM_size];


/* Tracks the status of HDD pages */
swap_page_t page_swap_tracker[virt_HDD_size];








/* This function is called directly from the generic exception handler in idt.c.
 */
void page_fault_handler(exception_state *state){
    void *vaddr=state->cr2; /* The virtual adress that caused the exception is stored in the cr2 register */

    /* To be implemented */


    

    /* Lookup if the vaddr is a valid page stored in the tracker for that process using page_swap_lookup() */
    swap_page_t *swp_page = page_swap_lookup(vaddr);
    if(!swp_page)
        page_fault_panic(vaddr);


    /* If it is, see if there is space in virt_RAM using the virt_RAM status array */
    

    /* If there is space, copy page into memory, update the page mapping using map_page function */

    /* If there is not space, determine a page to swap out using NRU */

    /* Depending on the status of that page either copy it back or simply overwrite it.
     * Do not forget to mark that page in the PD as not present, to do this simply call my function.*/
}



/* Finds an appropriate page using preference heirarchy,
 *  updates associated swap_page struct
 *  and sets the status of that virt_RAM page to free */
void swap_out_page(){

}


/* Sets the accesssed state bits to 0 on all pages */
void access_reset(){
    //For simplicitity, you can just set every present one to 0 rather than only ones that are releveant to us. 

    /* Get the page directory */
    page_directory_entry_t *pd = current_proc()->page_directory;

    /* To be implemented. */
}



//---------------------------------------------------
//--------------------HELPER CODE--------------------
//---------------------------------------------------


/* Will return the swap_page_t * relating to the vaddr and the current process if it exists.
 * Otherwise return NULL which indicates a faulty VADDR and should PANIC. */
swap_page_t *page_swap_lookup(void *vaddr){
    int i=0;
    p_id pid = current_proc()->pid;
    
    while(i<virt_HDD_size && (page_swap_tracker[i].vaddr!=vaddr || page_swap_tracker[i].owner_pid!=pid)) i++;
    
    if(i==virt_HDD_size) return NULL;

    return &page_swap_tracker[i];
}


/* Copies the given page from the source to the destination given the PHYSICAL addresses */
void phys_page_copy(void *dest, void *src){
    memcpy(Kptov(dest),Kptov(src),PGSIZE);
}


/* Allocates a page in virt_HDD memory, and notably does NOT add anything to virt_RAM.
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
    page_swap_tracker[i].vaddr=get_next_free_virt_page(1,F_ASSERT); //TODO update??? Have a space at 0xa0000000??
    page_swap_tracker[i].HDD_paddr=virt_HDD_start+i*PGSIZE;
    page_swap_tracker[i].RAM_paddr=NULL; /* As not been moved into RAM yet */

    return page_swap_tracker[i].vaddr;

}



/* Used to invalidate a RAM_paddr in that process' page directory */
bool invalidate_RAM_page(void *RAM_paddr){
    int i=0;
    while(page_swap_tracker[i].RAM_paddr!=RAM_paddr && i<virt_RAM_size) i++;
    if(i==virt_RAM_size) return false;

    p_id pid=page_swap_tracker[i].owner_pid;

    return invalidate_entry(page_swap_tracker[i].vaddr, get_proc(pid)->page_directory);
}


void page_fault_panic(void *vaddr){
    char msg[128];
    strcpy(msg, "Page fault on virtual address: ");
    itoa((int)vaddr,msg+strlen(msg),BASE_HEX);

    
    PANIC(msg);
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
    virt_HDD_start=virt_RAM_start+virt_RAM_size*PGSIZE;

}