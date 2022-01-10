#pragma once

#include "../lib/typedefs.h"

typedef struct MemorySegmentHeader MemorySegmentHeader_t;

#include "../sync/sync.h"



#define SHR_HEAP_SIZE 8 /* Number of pages to allocated to each heap */

extern lock shared_heap_lock;

struct MemorySegmentHeader{
    bool free;
    uint32_t size;
    MemorySegmentHeader_t* next;
    MemorySegmentHeader_t* previous;
};

MemorySegmentHeader_t *intialise_heap(void* base, void* limit);
void *malloc(uint32_t size);
void *shr_malloc(uint32_t size);
void  *alloc(uint32_t size);
void free(void *addr);
void shr_free(void *addr);
uint32_t heap_usage(MemorySegmentHeader_t *s);
uint32_t get_shared_heap_usage();
void clear_heap(void* addr, int pg_count);