#pragma once

#include "page.h"


#include "../lib/typedefs.h"

#define KERN_BASE           0xC0000000

#define F_VERBOSE 1<<1 /* 1 for print, 0 for not */
#define F_ZERO 1<<2 /* 1 for set all bytes to 0, 0 don't bother */
#define F_ASSERT 1<<3 /* 1 for PANIC if cannot be completed */
#define F_FLUSH   1<<4 /* Can be used to peform TLB flush if appropriate */

/* used to store an array of pages (can be virtual or physical) and
 * an index to the next free one */
typedef struct pool{
    int first_free_idx;
    page_entry_t pages[PG_COUNT];
} pool_t;


void *Kvtop(void* virt);
void *Kptov(void* phys);

void paging_init();
void init_vpool(pool_t *pool);
void perform_map(void *paddr, void *vaddr,page_directory_entry_t *pd, uint8_t flags);
void map_page(void *paddr, void *vaddr, uint8_t flags);
void *unmap_page(void *vaddr,uint8_t flags);
void setup_page_pool();
void setup_kernel_heap();
void *get_next_free_phys_page(size_t n, uint8_t flags);
void *get_next_free_virt_page(size_t n, uint8_t flags);
void *lookup_phys(void *vaddr);
bool free_virt_page(void *vaddr,size_t n);
bool free_phys_page(void *paddr, size_t n);
bool free_virt_phys_page(void *vaddr);
void *palloc_kern(size_t n,page_directory_entry_t *pd, uint8_t flags);
void *palloc_pcb(int pid);
void *new_pd();



extern void update_pd(uint32_t* pd); /* Note cr3 register takes physical address of pd */
extern void tlb_flush(); /* Flushes TLB cache */ 