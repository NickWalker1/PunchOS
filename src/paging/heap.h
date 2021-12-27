#pragma once

#include "heap.h"

#include "../lib/typedefs.h"
#include "../lib/screen.h"


#define HEAP_SIZE 2 /* Number of pages to allocated to each heap */

typedef struct MemorySegmentHeader MemorySegmentHeader_t;

struct MemorySegmentHeader{
    //implement here
    bool free;
    uint32_t size;
    MemorySegmentHeader_t* next;
    MemorySegmentHeader_t* previous;
};

void intialiseHeap(void* base, void* limit);
void *malloc(uint32_t size);
void free(void* addr);
void clear_heap(void* addr);