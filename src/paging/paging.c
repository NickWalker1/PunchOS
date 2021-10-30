#include "paging.h"



extern uint32_t _KERNEL_END;

/* pointer to Kernel page directory. */
page_directory_entry_t* kernel_pd;

/* To page pool with only 128 available pages.
 * The actual available memory space is far larger than this.
 * However  */    
page_entry_t phys_page_pool[MAX_PHYS_PAGE];
uint16_t first_free_phys_idx;

/* Virtual page pool for kernel. Each process will need one of these also
 * As each process needs to know which virtual addresses
 * it has already assgigned so to not override them. */
page_entry_t K_virt_heap_pool[MAX_PHYS_PAGE];
uint16_t K_first_free_virt_idx;

/* Converts physical address to kernel virtual address */
void *Kptov(void* phys){
    return (void*) (phys+(uint32_t)KERN_BASE);
}
/* Converts virtual address to kernel physical address */
void *Kvtop(void* virt){
    return (void*) virt-(uint32_t)KERN_BASE;
}


/* Intialises Paging by creating new Kernel Page Directory
 * without the identity maps. Then sets up available page pool
 * and the kernel heap.
 */
void paging_init(){
    //First step to work out where the free pages start from.
    //Must ensure is after kernel end.

    //Align to next 4k boundary (4096 = 0x1000)
    uint32_t kernel_end = _KERNEL_END-_KERNEL_END%0x1000 + 0x1000;

    //Initialise physical page pool.
    int i;
    for(i=0;i<MAX_PHYS_PAGE;i++){
        phys_page_pool[i].base_addr=(void*)(kernel_end+i*PGSIZE);
        phys_page_pool[i].type=M_FREE;
    }

    //Create new kernel page directory.
    kernel_pd=Kptov(get_next_free_phys_page());
    map_page(Kvtop(kernel_pd),kernel_pd,F_KERN | F_ASSERT);

    //map kernel code pages.
    int kernel_pages_count = (kernel_end-KERN_BASE)/PGSIZE;
    for(i=0;i<kernel_pages_count;i++){
        map_page(i*PGSIZE,(i*PGSIZE)+KERN_BASE,F_ASSERT | F_KERN | F_VERBOSE);
    }

    //switch to using kernel_pd rather than temporary setup one.
    
}

/* Adds pd, pt mappings for a new page given a virtual and physical address
 * Currently only maps in the kernel page directory*/
void map_page(void* paddr, void* vaddr, uint8_t flags){
    if((uint32_t)vaddr%PGSIZE || (uint32_t)paddr%4096) PANIC("VADDR NOT 4k ALIGNED"); 
        

    size_t pd_idx, pt_idx;
    page_table_entry_t* pt;

    pd_idx=pd_no(vaddr);
    pt_idx=pt_no(vaddr);

    //TODO load pd form thread_control_block instead of just using kernel.

    //if the page table page does not exist, create one and fill out the entry
    //in the PD.
    if(kernel_pd[pd_idx].present==0){
        void* pt_addr = get_next_free_physical_page();
        map_page(pt_addr,Kptov(pt_addr),F_KERN); /* so that you can write to this address in kernel address space */

        kernel_pd[pd_idx].page_table_base_addr=((uint32_t)pt_addr >> PGBITS); //Only most significant 20bits
        kernel_pd[pd_idx].present=1;
        kernel_pd[pd_idx].read_write=1;
    }
    pt=(page_table_entry_t*) Kptov(kernel_pd[pd_idx].page_table_base_addr<<PGBITS); //Push back to correct address
    pt[pt_idx].page_base_addr=(uint32_t) paddr>>PGBITS; //Only 20 most significant bits
    pt[pt_idx].present=1;
    pt[pt_idx].read_write=1;

    if(flags & F_KERN) pt[pt_idx].user_supervisor=1;
        
        
    if(flags & F_VERBOSE){
        println("Mapped Page (P:V) ");
        print(itoa((int)paddr,str,BASE_HEX));
        print(":");
        print(itoa((int)vaddr,str,BASE_HEX));
    }

}

/* Returns the base address of the next free physical page.
 * Will PANIC if no more pages are available */
void *get_next_free_phys_page(){
    if(first_free_phys_idx==-1) PANIC("NO AVAILABLE PAGES REMAINING");
    
    void* addr= phys_page_pool[first_free_phys_idx].base_addr;
    phys_page_pool[first_free_phys_idx].type=M_ALLOCATED;

    uint16_t idx=first_free_phys_idx+1;
    while(phys_page_pool[idx].type!=M_FREE){
        idx++;
        if(idx==MAX_PHYS_PAGE){
            first_free_phys_idx=-1;
            return addr;
        }
    }
    first_free_phys_idx=idx;
    return addr;
}