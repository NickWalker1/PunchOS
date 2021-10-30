#pragma once

#define MAX_PHYS_PAGE       128
#define PGBITS              12
#define PDBITS              10
#define	PTSHIFT             PGBITS/* First page table bit. */
#define PTBITS              10/* Number of page table bits. */
#define PDSHIFT             (PTSHIFT + PTBITS)/* First page directory bit. */
#define PGSIZE              4096

/*   Virtual addresses structure:

    31                  22 21                  12 11                   0
   +----------------------+----------------------+----------------------+
   | Page Directory Index |   Page Table Index   |    Page Offset       |
   +----------------------+----------------------+----------------------+
*/

/* Obtains page table index from a virtual address. */
static inline unsigned pt_no (const void *va) {
  // return (size_t) va & PTMASK) >> PTSHIFT;
  return ((size_t) va >> PGBITS) & 0x3ff;
}

/* Obtains page directory index from a virtual address. */
static inline size_t pd_no (const void *va) {
  return (size_t) va >> PDSHIFT;
}

typedef enum memory_type{
    M_FREE=1,/* Not allocated to anyone */
    M_ALLOCATED=2, /* Allocated to someone */
    M_RESERVED=3, /* Cannot be allocated */
}memory_type_t;

typedef struct page_entry{
    memory_type_t type;
    void *base_addr;
}page_entry_t;


typedef struct page_directory_entry
{
    uint8_t present : 1;
    uint8_t read_write : 1;
    uint8_t user_supervisor : 1;
    uint8_t write_through : 1;
    uint8_t cache_disabled : 1;
    uint8_t accessed : 1;
    uint8_t reserved : 1;
    uint8_t page_size : 1;
    uint8_t global : 1;

    uint8_t available : 3;
    uint32_t page_table_base_addr : 20;
} __attribute__((packed)) page_directory_entry_t;

typedef struct page_table_entry
{
    uint8_t present : 1; /* If this entry is present in the table */
    uint8_t read_write : 1; /* 1= R/W , 0=RO */
    uint8_t user_supervisor : 1; /* 1=supervisor , 0=nonsupervisor */
    uint8_t write_through : 1; /* 1=write through enabled , 0=not enabled */
    uint8_t cache_disabled : 1;
    uint8_t accessed : 1;
    uint8_t dirty : 1;
    uint8_t page_table_attribute_index: 1;
    uint8_t global : 1;

    uint8_t available : 3;
    uint32_t page_base_addr : 20; /* Base physical address of page */
} __attribute__((packed)) page_table_entry_t;

