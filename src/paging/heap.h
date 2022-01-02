#pragma once

#include "../lib/typedefs.h"

typedef struct MemorySegmentHeader MemorySegmentHeader_t;

#include "../sync/sync.h"



#define HEAP_SIZE 2 /* Number of pages to allocated to each heap */

extern lock kernel_heap_lock;

struct MemorySegmentHeader{
    bool free;
    uint32_t size;
    MemorySegmentHeader_t* next;
    MemorySegmentHeader_t* previous;
};

MemorySegmentHeader_t *intialise_heap(void* base, void* limit);
void *malloc(uint32_t size);
void *kalloc(uint32_t size);
void  *alloc(uint32_t size);
void free(void *addr);
void kfree(void *addr);
void clear_heap(void* addr, int pg_count);