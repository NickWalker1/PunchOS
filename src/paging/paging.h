#pragma once

#include "page.h"

#include "../lib/typedefs.h"
#include "../lib/screen.h"


#define KERN_BASE           0xC0000000

#define F_KERN 0x1 /* 1 for kernel, 0 for user */
#define F_ZERO 0x2 /* 1 for set all bytes to 0, 0 don't bother */
#define F_ASSERT 0x4 /* 1 for PANIC if cannot be completed */
#define F_VERBOSE 0x80 /* 1 for print, 0 for not */



void *Kvtop(void* virt);
void *Kptov(void* phys);

void paging_init();
void map_page(void* paddr, void* vaddr, uint8_t flags);
void setup_page_pool();
void setup_kernel_heap();
void* get_next_free_phys_page();