#pragma once

#include "heap.h"

#include "../lib/typedefs.h"
#include "../lib/screen.h"

void intialiseHeap(void* base, void* limit);
void *malloc(uint32_t size);
void free(void* addr);