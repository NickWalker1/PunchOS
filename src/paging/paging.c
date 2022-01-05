#include "paging.h"

#include "heap.h"

#include "../lib/screen.h"
#include "../lib/debug.h"
#include "../processes/pcb.h"

extern uint32_t helper_variable;

extern uint32_t _KERNEL_END;

extern MemorySegmentHeader_t *shared_first_seg;
extern MemorySegmentHeader_t *first_segment;

/* pointer to Kernel page directory.
 * NOTE IN VIRTUAL ADDRESS SPACE
 * MUST BE CONVERTED TO PHYS ADDR WHEN UPDATING CR3
 */
// page_directory_entry_t *kernel_pd;

/* Kernel virtual address of a page from which to copy the base required mappings for new proceses */
page_directory_entry_t *base_pd;

/* To page pool with only 128 available pages.
 * The actual available memory space is far larger than this. */

pool_t phys_page_pool;

/* Virtual page pool for kernel. Each process will need one of these also
 * As each process needs to know which virtual addresses
 * it has already assgigned so to not override them. */
pool_t K_virt_pool;


void *base_PCB_block;

/* Converts physical address to kernel virtual address */
void *Kptov(void* phys){
    return (void*) (phys+(uint32_t)KERN_BASE);
}
/* Converts virtual address to kernel physical address */
void *Kvtop(void* virt){
    return (void*) (virt-(uint32_t)KERN_BASE);
}


/* Intialises Paging by creating new Kernel Page Directory
 * without the identity maps. Then sets up available page pools
 * and the kernel heap.
 */
void paging_init(){
    int i;
    //First step to work out where the free pages start from.
    //Must ensure is after kernel end.

    
    /* Physical memory location where kernel finished loading */
    uint32_t kernel_end = (uint32_t)&_KERNEL_END;
    //Align to next 4k boundary (4096 = 0x1000)
    kernel_end= (uint32_t)Kvtop((void*)(kernel_end-kernel_end%0x1000 + 0x1000));


    void *end_addr =(void*)kernel_end;
    base_PCB_block=Kptov(end_addr);

    // //The overarching table of process info
    // perform_map(end_addr,Kptov(end_addr),kernel_pd,F_ASSERT);
    // end_addr+=PGSIZE;

    end_addr+=MAX_PROCS*PGSIZE;

    //Initialise physical page pool.
    phys_page_pool.first_free_idx=0;

    for(i=0;i<PG_COUNT;i++){
        phys_page_pool.pages[i].base_addr=(void*)(end_addr+(i*PGSIZE));
        phys_page_pool.pages[i].type=M_FREE;
    }

    //Initialise default pool for the first process
    init_vpool(&K_virt_pool);


    //Create new kernel page directory.
    // kernel_pd=Kptov(get_next_free_phys_page(1,F_ZERO | F_ASSERT));
    
    //acquire another page for the base pd for new processes.
    base_pd = Kptov(get_next_free_phys_page(1,F_ASSERT));

    //Map the kernel and base pd //TODO don't ever use kernel_pd
    // perform_map(Kvtop(kernel_pd),(void*)kernel_pd,kernel_pd ,F_ASSERT);
    perform_map(Kvtop(base_pd),(void*)base_pd,base_pd,F_ASSERT);

    //Map kernel code pages.
    int kernel_pages_count = kernel_end/PGSIZE; //Includes all pages beneath kernel also

    for(i=0;i<kernel_pages_count;i++){
        perform_map((void*)(i*PGSIZE),Kptov((void*)(i*PGSIZE)),base_pd,F_ASSERT);
    }

    //add PCB block mappings to the 64 page space saved earlier.
    void *addr = base_PCB_block;
    for(i=0;i<MAX_PROCS;i++){
        perform_map(Kvtop(addr),addr,base_pd,F_ASSERT);
        addr+=PGSIZE;
    }

    
    //Allocate 8 pages as a shared heap space for all processes.
    //Must request and allocate manually as cannot use palloc_kern yet due to processes
    //not being initialised yet.
	void *heap_addr=Kptov(get_next_free_phys_page(1,F_ASSERT));
    perform_map(Kvtop(heap_addr),heap_addr,base_pd,F_ASSERT);
    for(i=1;i<8;i++){
        void* addr=get_next_free_phys_page(1,F_ASSERT);
        perform_map(addr,Kptov(addr),base_pd,F_ASSERT);
    }

    //palloc_kern(8,F_ASSERT);
	shared_first_seg = intialise_heap(heap_addr,heap_addr+(HEAP_SIZE*PGSIZE));

    //by default before processes
    first_segment=shared_first_seg;

    //switch to using kernel_pd rather than temporary setup one.
    update_pd(Kvtop(base_pd));
}


/* Initialises a virtual pool from 0xb0000000 and sets all pages to free */
void init_vpool(pool_t *pool){
    
    uint32_t addr = 0xb0000000;
    pool->first_free_idx=0;
    int i;
    for(i=0;i<PG_COUNT;i++){
        pool->pages[i].base_addr=(void*)addr;
        pool->pages[i].type=M_FREE;
        addr+=0x1000;
    }
}


/* Adds pd, pt mappings in the given page directory virtual address*/
void perform_map(void *paddr, void *vaddr, page_directory_entry_t* pd, uint8_t flags){
    if((uint32_t)vaddr%PGSIZE || (uint32_t)paddr%4096) PANIC("VADDR NOT 4k ALIGNED"); 
        
    size_t pd_idx, pt_idx;
    page_table_entry_t* pt;

    pd_idx=pd_no(vaddr);
    pt_idx=pt_no(vaddr);

    //if the page table page does not exist, create one and fill out the entry
    //in the PD.
    if(pd[pd_idx].present==0){
        void* pt_addr = get_next_free_phys_page(1,F_ASSERT); //TODO fix bug to readd F_ZERO
        pd[pd_idx].page_table_base_addr=((uint32_t)pt_addr >> PGBITS); //Only most significant 20bits
        pd[pd_idx].present=1;
        pd[pd_idx].read_write=1;
        perform_map(pt_addr,Kptov(pt_addr),pd,flags); /* so that you can write to this address in kernel address space */
    }
    pt=(page_table_entry_t*) Kptov((void*)(pd[pd_idx].page_table_base_addr<<PGBITS)); //Push back to correct address
    pt[pt_idx].page_base_addr=(uint32_t) paddr>>PGBITS; //Only 20 most significant bits
    pt[pt_idx].present=1;
    pt[pt_idx].read_write=1;
    pt[pt_idx].user_supervisor=1; //As always in ring 0
        
        
    if(flags & F_VERBOSE){
        println("Mapped Page (P:V) ");
        print(itoa((int)paddr,str,BASE_HEX));
        print(":");
        print(itoa((int)vaddr,str,BASE_HEX));
    }
}


/* Adds pd, pt mappings in current process' virtual
 * address space for a new page given a virtual and physical address 
 * NOTE: Must not be called before processes initialised. */
void map_page(void* paddr, void* vaddr, uint8_t flags){
    perform_map(paddr,vaddr,current_proc()->page_directory,flags);
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
    size_t i;
    for(i=1;i<n;i++){
        idx++;
        if(pool->pages[idx].type!=M_FREE){
            //start again as slot not big enough.
            i=1;
            //jump to next free page.
            while(pool->pages[idx].type!=M_FREE){
                idx++;
                if(idx==PG_COUNT){
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
        if(idx==PG_COUNT){
            pool->first_free_idx=-1;
            return base_addr;
        }
        idx++;
    }

    pool->first_free_idx=idx;
    
    if(flags&F_VERBOSE){ println("Got phys page: "); print(itoa((int)base_addr,str,BASE_HEX));}

    //TODO FIX AS currently broken.
    //if(flags&F_ZERO){
    if(0){
        //Because this page may or may not be mapped to an unkown virtual address.
        //must create temporary virtual kernel map and use that to reference this page

        map_page(base_addr,Kptov(base_addr),0);
        memset(Kptov(base_addr),0,n*PGSIZE);
        unmap_page(Kptov(base_addr),0);
    }

    pool->first_free_idx=idx;
    
    return base_addr;
}


/* Returns the base address of the next n free contiguous virtual pages 
 * Flag options:
 *  - F_ASSERT will PANIC if not possible
 */ 
void *get_next_free_virt_page(size_t n,uint8_t flags){
    //TODO retrtieve associated virtual page pool for the process.
    //currently implemented.
    pool_t *pool = &K_virt_pool;

    if(pool->first_free_idx==-1) PANIC("NO VIRT PAGES AVAILABLE");

    int idx=pool->first_free_idx;

    int start= idx;

    void* base_addr = (void*) pool->pages[start].base_addr;

    size_t i;
    for(i=1;i<n;i++){
        idx++;
        if(pool->pages[idx].type!=M_FREE){
            //if next page not available, restart
            i=1;
            while(pool->pages[idx].type!=M_FREE){
                idx++;
                if(idx==PG_COUNT){
                    pool->first_free_idx=-1;
                    if(flags & F_ASSERT)
                        PANIC("INSUFFICIENT VIRTUAL PAGES AVAILABLE");
                    return NULL;
                }
            }
            start=idx;
            base_addr=pool->pages[start].base_addr;
        }
    }
    for(i=start;i<start+n;i++) pool->pages[i].type=M_ALLOCATED;

    //update next free pointer->
    while(pool->pages[idx].type!=M_FREE){
        idx++;
        if(idx==PG_COUNT){
            pool->first_free_idx=-1;
            return base_addr;
        }
    }
    
    pool->first_free_idx=idx;
    return base_addr;
}



/* Unmaps the physical page associated with the given
 * virtual address, and flushes TLB.
 * Returns phys addr of unmaped page
 * Vaddr must be 4k aligned.
 */
void* unmap_page(void* vaddr,uint8_t flags){ 
    if((uint32_t)vaddr%4096) PANIC("Attempted to unmap non 4k aligned address");

    //TODO retrieve associated PD from PCB
    page_directory_entry_t* pd = current_proc()->page_directory;

    uint32_t pd_idx, pt_idx;
    pd_idx=pd_no(vaddr);
    pt_idx=pt_no(vaddr);

    page_table_entry_t* pt=(page_table_entry_t*) Kptov((void*)(pd[pd_idx].page_table_base_addr<<PTBITS)); //Push back to correct address
    void* paddr= (void*)(pt[pt_idx].page_base_addr<<PTSHIFT);
    pt[pt_idx].present=0;

    //Flushing TLB is an expensive operation, and is done automatically on context switch
    //only use for high priority memory protection or testing pursposes.
    if(flags & F_FLUSH)
        tlb_flush();
    
    if(flags & F_VERBOSE){
        println("Unmapped page: ");
        print(itoa((int)vaddr,str,BASE_HEX));
    }

    return paddr;
}


/* Returns mapped base physical address of vaddr
 * Vaddr must be PGSIZE alligned*/ 
void *lookup_phys(void* vaddr){
    if(!(uint32_t) vaddr%4096) PANIC("Vaddr not PGSIZE alligned on lookup.");

    //TODO update to PCB pd.
    page_directory_entry_t* pd = current_proc()->page_directory;
    
    uint32_t pd_idx, pt_idx;
    pd_idx=pd_no(vaddr);
    pt_idx=pt_no(vaddr);

    page_table_entry_t* pt=(page_table_entry_t*) Kptov((void*)(pd[pd_idx].page_table_base_addr<<PGBITS)); //Push back to correct address
    
    return pt[pt_idx].present ? (void*)(pt[pt_idx].page_base_addr<<PGBITS) : (void*)0;
}


/* Frees the physical page from the general page pool */
bool free_phys_page(void* paddr, size_t n){
    if(n<=0) return false;
    if((uint32_t)paddr%4096) PANIC("Cannot free unaligned paddr");

    //TODO update pool from PCB
    pool_t *pool=&phys_page_pool;

    //Highly inefficient approach
    int i,j;
    for(i=0;i<PG_COUNT;i++){
        if(pool->pages[i].base_addr==paddr){
            //found the page
            pool->pages[i].type=M_FREE;
            if(pool->first_free_idx>i)
                pool->first_free_idx=i;
            

            for(j=i+1;j<i+(int)n;j++){
                pool->pages[j].type=M_FREE;
            }
            return true;
        }
    }
    return false;
}


/* Frees the n virtual pages in the process' virtual pool */
bool free_virt_page(void* vaddr, size_t n){
    if(n==0) return false;
    if((uint32_t)vaddr%4096) PANIC("Cannot free unaligned vaddr");

    //TODO update pool from PCB
    pool_t *pool=current_proc()->virt_pool;
    //Highly inefficient approach
    int i;
    for(i=0;i<PG_COUNT;i++){
        if(pool->pages[i].base_addr==vaddr){
            //found the page
            pool->pages[i].type=M_FREE;
            if(pool->first_free_idx>i)
                pool->first_free_idx=i;
            
            size_t j;
            for(j=i+1;j<i+n;j++){
                pool->pages[j].type=M_FREE;
            }
            return true;
        }
    }
    return false;
}


/* Given a virtual address this will unmap both the virtual and 
 * physical address from the pools and unamp the virtal address also.
 * Returns true if successfull.
 */
bool free_virt_phys_page(void* vaddr){
    if(!free_virt_page(vaddr,1)) return false;
    
    void* paddr= unmap_page(vaddr,0);
    if(!paddr) return false;

    if(!free_phys_page(paddr,1)) return false;

    return true;
}

/* Returns pointer to a free page in the  MAX_PROCS*PGSIZE sized Kernel PCB Block of memory. */
void *palloc_pcb(int pid){
    //needs process ID to index the block
    void *addr=base_PCB_block+(pid-1)*PGSIZE;
    memset(addr,0,PGSIZE);
    return addr;
}


/* Returns virtual address pointer to the start of an n-page free
 * address in kernel address space 
 * TODO check this function. Not sure it makes sense anymore. */
void *palloc_kern(size_t n, page_directory_entry_t *pd, uint8_t flags){
    void *paddr= get_next_free_phys_page(n,flags);
    if(!paddr) return NULL;

    //TODO surely should use processes own vpool???
    for(size_t i=0;i<n;i++){
        perform_map(paddr+i*PGSIZE,Kptov(paddr+i*PGSIZE),pd,F_ASSERT);
    }

    return Kptov(paddr);
}


/* Duplicates the given virtual address space by creating new PD and associated PTs
 * Returns PHYSICAL address of new page directory.
 * NOTE: pd arg must be current process pd or base pd otherwise will cause pgfault */
void *virt_addr_space_duplication(page_directory_entry_t *pd){
    ASSERT(pd==base_pd || pd==current_proc()->page_directory ,"Cannot duplicate inaccessible address space.");
    void* new_pd =get_next_free_phys_page(1,F_ASSERT);

    //temporary map
    map_page(new_pd,Kptov(new_pd),F_ASSERT);

    //map itself 
    perform_map(new_pd,Kptov(new_pd),Kptov(new_pd),F_ASSERT);

    page_directory_entry_t pde;
    void *pt_addr;
    /* Iterate through the pd and create new PT's as required */
    int i;
    for(i=0;i<1024;i++){
        pde=pd[i];
        if(pde.present){
            pt_addr=get_next_free_phys_page(1,F_ASSERT);
            /* Map page temporarily so we can write to it. */
            map_page(pt_addr,Kptov(pt_addr),F_ASSERT);

            /* Copy the page table over into the new pt */
            memcpy(Kptov(pt_addr),Kptov((void*)(pde.page_table_base_addr<<PTSHIFT)),PGSIZE);

            /* Unmap again */
            unmap_page(Kptov(pt_addr),F_ASSERT);

            //TODO add the new mapping into new_pd
        }
    }
    

    return pd;
}