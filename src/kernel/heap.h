#pragma once

#include "heap.h"

#include "../lib/typedefs.h"
#include "../lib/screen.h"

typedef struct MemorySegmentHeader MemorySegmentHeader_t;

#define segment_magic 0x87654321

struct MemorySegmentHeader{
    bool free;
    uint32_t size;
    MemorySegmentHeader_t* next;
    MemorySegmentHeader_t* previous;
    uint32_t magic;
};

void intialiseHeap(void* base, void* limit);
void *malloc(uint32_t size);
void free(void* addr);
void clear_heap(void* addr);
void heap_dump();