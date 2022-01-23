#pragma once

#include "../lib/typedefs.h"

typedef struct MemorySegmentHeader MemorySegmentHeader_t;

#include "../sync/sync.h"



#define SHR_HEAP_SIZE 8 /* Number of pages to allocated to each heap */

extern lock shared_heap_lock;

#define segment_magic 0x87654321

struct MemorySegmentHeader{
    bool free;
    uint32_t size;
    MemorySegmentHeader_t* next;
    MemorySegmentHeader_t* previous;
    uint32_t magic;
};

MemorySegmentHeader_t *intialise_heap(void* base, void* limit);
void *malloc(uint32_t size);
void *shr_malloc(uint32_t size);
void  *alloc(uint32_t size, MemorySegmentHeader_t *start_seg);
void free(void *addr);
void shr_free(void *addr);
uint32_t heap_usage(MemorySegmentHeader_t *s);
uint32_t get_shared_heap_usage();
void clear_heap(void* addr, int pg_count);
void shared_heap_dump();