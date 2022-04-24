#pragma once



#include "../lib/typedefs.h"
#include "../lib/debug.h"

#include "../interrupt/cpu_state.h"
#include "../processes/pcb.h"

typedef struct swap_page{
    void *vaddr;     /* Associated vaddr */
    p_id owner_pid;  /* Owning process PID */
    void *HDD_paddr; /* This shouldn't change throughout the lifetime of the page */
    void *RAM_paddr; /* NULL if swapped out */
} swap_page_t; 


#define virt_RAM_size 16 /* Can be no bigger than 32 or may run out of virt_pages */
#define virt_HDD_size 32

void page_fault_handler(exception_state *state);
void *palloc_HDD();


void phys_page_copy(void *dest, void *src);
void *palloc_HDD();
void virt_RAM_init();
void virt_HDD_init();

void access_reset();
bool invalidate_RAM_page(void *RAM_paddr);
swap_page_t *page_swap_lookup(void *vaddr);
void page_fault_panic(void *vaddr);


