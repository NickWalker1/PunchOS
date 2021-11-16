#include "paging.h"



extern uint32_t _KERNEL_END;

/* pointer to Kernel page directory.
 * NOTE IN VIRTUAL ADDRESS SPACE
 * MUST BE CONVERTED TO PHYS ADDR WHEN UPDATING CR3
 */
page_directory_entry_t* kernel_pd;

/* To page pool with only 128 available pages.
 * The actual available memory space is far larger than this.
 * However  */    

pool_t phys_page_pool;

/* Virtual page pool for kernel. Each process will need one of these also
 * As each process needs to know which virtual addresses
 * it has already assgigned so to not override them. */
pool_t K_virt_pool;

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

    
    /* Physical memory location where kernel finished loading */
    uint32_t kernel_end = &_KERNEL_END;
    //Align to next 4k boundary (4096 = 0x1000)
    kernel_end= Kvtop(kernel_end-kernel_end%0x1000 + 0x1000);

    //Initialise physical page pool.
    int i;
    for(i=0;i<MAX_PHYS_PAGE;i++){
        phys_page_pool.pages[i].base_addr=(void*)(kernel_end+(i*PGSIZE));
        phys_page_pool.pages[i].type=M_FREE;
    }

    //Create new kernel page directory.
    kernel_pd=Kptov(get_next_free_phys_page(1,F_ZERO));

    map_page(Kvtop(kernel_pd),kernel_pd,F_KERN | F_ASSERT | F_VERBOSE);
    println("kernel pd mapping");

    //map kernel code pages.
    int kernel_pages_count = kernel_end/PGSIZE;
    for(i=0;i<kernel_pages_count;i++){
        map_page(i*PGSIZE,(i*PGSIZE)+KERN_BASE,F_ASSERT | F_KERN);
    }

    //switch to using kernel_pd rather than temporary setup one.
    update_pd(Kvtop(kernel_pd));
    
}

extern uint8_t helper_variable;
/* Adds pd, pt mappings for a new page given a virtual and physical address
 * Currently only maps in the kernel page directory*/
void map_page(void* paddr, void* vaddr, uint8_t flags){
    if((uint32_t)vaddr%PGSIZE || (uint32_t)paddr%4096) PANIC("VADDR NOT 4k ALIGNED"); 
        
    size_t pd_idx, pt_idx;
    page_table_entry_t* pt;

    pd_idx=pd_no(vaddr);
    pt_idx=pt_no(vaddr);

    //TODO load pd from PCB instead of just using kernel.

    //if the page table page does not exist, create one and fill out the entry
    //in the PD.
    helper_variable=0;
    if(kernel_pd[pd_idx].present==0){
        void* pt_addr = get_next_free_phys_page(1,F_ASSERT  | F_VERBOSE); //TODO fix bug to readd F_ZERO

        kernel_pd[pd_idx].page_table_base_addr=((uint32_t)pt_addr >> PGBITS); //Only most significant 20bits
        kernel_pd[pd_idx].present=1;
        kernel_pd[pd_idx].read_write=1;
        map_page(pt_addr,Kptov(pt_addr),F_KERN | F_VERBOSE ); /* so that you can write to this address in kernel address space */
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

/* Returns the base address of the next n contiguous free physical pages
 * Will PANIC if no more pages are available and F_ASSERT flag present,
 * returns NULL otherwise. */
void *get_next_free_phys_page(size_t n, uint8_t flags){
    pool_t* pool = &phys_page_pool;
    if(pool->first_free_idx==-1){
        if(flags & F_ASSERT)
            PANIC("NO PHYS PAGES AVAILABLE");
        return NULL;
    }
    
    void* base_addr= pool->pages[pool->first_free_idx].base_addr;
    //pool.pages[pool.first_free_idx].type=M_ALLOCATED;
    int idx=pool->first_free_idx;
    int start=idx;
    int i;
    for(i=1;i<n;i++){
        idx++;
        if(pool->pages[idx].type!=M_FREE){
            //start again as slot not big enough.
            i=1;
            //jump to next free page.
            while(pool->pages[idx].type!=M_FREE){
                idx++;
                if(idx==MAX_PHYS_PAGE){
                    if(flags & F_ASSERT)
                        PANIC("INSUFFICIENT PHYSICAL PAGES AVAILABLE");
                    return NULL;
                }
            }
            start=idx;
            base_addr=pool->pages[idx].base_addr;
        }
    }
    //mark those pages as allocated.
    for(i=start;i<start+n;i++){
        pool->pages[i].type=M_ALLOCATED;
    }

    //update next free pointer.
    idx++;
    while(pool->pages[idx].type!=M_FREE){
        if(idx==MAX_PHYS_PAGE){
            pool->first_free_idx=-1;
            return base_addr;
        }
        idx++;
    }

    pool->first_free_idx=idx;
    
    if(flags&F_ZERO) memset(base_addr,0,PGSIZE);
    pool->first_free_idx=idx;
    
    return base_addr;
}

/* Returns the base address of the next n free contiguous virtual pages 
 * Flag options:
 *  - F_ASSERT will PANIC if not possible*/ 
void *get_next_free_virt_page(size_t n,uint8_t flags){
    //TODO retrTieve associated virtual page pool for the process.
    //currently implemented.
    pool_t pool = K_virt_pool;

    if(pool.first_free_idx==-1) PANIC("NO VIRT PAGES AVAILABLE");

    void* base_addr = (void*) pool.pages[pool.first_free_idx].base_addr;
    int idx=pool.first_free_idx+1;

    int start= idx;
    int i;
    for(i=1;i<n;i++){
        idx++;
        if(pool.pages[idx].type!=M_FREE){
            //if next page not available, restart
            i=1;
            while(pool.pages[idx].type!=M_FREE){
                idx++;
                if(idx==MAX_PHYS_PAGE){
                    pool.first_free_idx=-1;
                    if(flags & F_ASSERT)
                        PANIC("INSUFFICIENT VIRTUAL PAGES AVAILABLE");
                    return NULL;
                }
            }
            start=idx;
            base_addr=pool.pages[start].base_addr;
        }
    }
    for(i=start;i<start+n;i++) pool.pages[i].type=M_ALLOCATED;

    //update next free pointer.
    idx++;
    while(pool.pages[idx].type!=M_FREE){
        idx++;
        if(idx==MAX_PHYS_PAGE){
            pool.first_free_idx=-1;
            return base_addr;
        }
    }
    
    pool.first_free_idx=idx;

    return base_addr;
}

void unmap_page(void* vaddr){
    if((uint32_t)vaddr%4096) PANIC("Attempted to unmap non 4k aligned address");

    //TODO retrieve associated PD from PCB
    page_directory_entry_t* pd = kernel_pd;

    uint32_t pd_idx, pt_idx;
    pd_idx=pd_no(vaddr);
    pt_idx=pt_no(vaddr);

    page_table_entry_t* pt=(page_table_entry_t*) Kptov(kernel_pd[pd_idx].page_table_base_addr<<PGBITS); //Push back to correct address
    pt[pt_idx].present=0;

    //flushing tlb is entirely overkill to unmap a page. 
    //tlb will automatically be flushed on context switch.
    //uncomment for testing purposes.
    tlb_flush();
}

/* Will add page back into virtual page pool,
 * add page back into physical page pool,
 * set present bit in page directory to 0,
 * flush tlb with cr3 refresh.
 */
bool free_virt_page(void* vaddr){
    //Todo retrive pool from PCB
    pool_t *virt_pool = &K_virt_pool;
    pool_t *phys_pool = &phys_page_pool;

    //Highly inefficient approach
    int i;
    for(i=0;i<MAX_PHYS_PAGE;i++){
        if(virt_pool->pages[i].base_addr==vaddr){
            //found the page

            //TODO update to PCB pd.
            page_directory_entry_t* pd = kernel_pd;

            uint32_t pd_idx, pt_idx,paddr;
            pd_idx=pd_no(vaddr);
            pt_idx=pt_no(vaddr);

            page_table_entry_t* pt=(page_table_entry_t*) Kptov(kernel_pd[pd_idx].page_table_base_addr<<PGBITS); //Push back to correct address
            paddr=pt[pt_idx].page_base_addr<<PGBITS;

            //find the physical entry and make it as free

            //TODO again another highly inefficent approach.
            //could be improved by knowing where phys pages start then
            //using that to calculate the index. 
            int j;
            for(j=0;j<MAX_PHYS_PAGE;j++){
                if(phys_pool->pages[j].base_addr==paddr){
                    phys_pool->pages[j].type=M_FREE;
                    if(j<phys_pool->first_free_idx) phys_pool->first_free_idx=j;


                    //unmap the page in the pd
                    pt[pt_idx].present=0;

                    //optional tlbflush
                    tlb_flush();

                    return true;
                }
            }
            //if address faulty
            PANIC("COULDN'T FIND ASSOCIATED PADDR TO VADDR FOR UNMAP");
        }
    }
    //unable to find the page.
    return false;

}