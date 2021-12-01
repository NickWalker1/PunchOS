#include "heap.h"

/* pointer to the first head of the linked-list */
MemorySegmentHeader_t *firstSegment;

void intialiseHeap(void* base, void* limit){
    //To be implemented.
}

void *malloc(uint32_t size){
    //To be implemented.
    return NULL;
}


void free(void* addr){
    //To be implemented.
}

/* Used to wipe heap space clean for testing.*/
void clear_heap(void* base_heap){
    memset(base_heap,0,0x1000);
}