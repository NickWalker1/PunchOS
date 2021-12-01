#pragma once

#include "heap.h"

#include "../lib/typedefs.h"
#include "../lib/screen.h"

typedef struct MemorySegmentHeader MemorySegmentHeader_t;

struct MemorySegmentHeader{
    //implement here
};

void intialiseHeap(void* base, void* limit);
void *malloc(uint32_t size);
void free(void* addr);
void clear_heap(void* addr);