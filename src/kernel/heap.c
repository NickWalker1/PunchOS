#include "heap.h"

/* pointer to the first head of the linked-list */
MemorySegmentHeader_t *firstSegment;

void intialiseHeap(void* base, void* limit){
    /* To be implemented. */

}

void *malloc(uint32_t size){
    /* To be implemented. */

    return 0;
}


void free(void* addr){
    /* To be implemented. */
}

/* Used to wipe heap space clean for testing.*/
void clear_heap(void* base_heap){
    memset(base_heap,0,0x1000);
}


void heap_dump(){
    MemorySegmentHeader_t *cur_seg=firstSegment;
    int used, total,count;
    used=total=count=0;
    println("---------------\n");
    while(cur_seg!=NULL){
        count++;
        print("|");
        print(itoa(cur_seg->size,str,BASE_DEC));
        if(!cur_seg->free){
            used+=cur_seg->size;
            print(" U");
        } 
        else{
            print(" F");
        }
        print("|");
        total+=cur_seg->size+sizeof(MemorySegmentHeader_t);
        cur_seg=cur_seg->next;
    }
    println("Count: ");
    print(itoa(count,str,BASE_DEC));
    println("Used: ");
    print(itoa(used,str,BASE_DEC));
    println("Available: ");
    print(itoa(total-used,str,BASE_DEC));
    println("Total: ");
    print(itoa(total,str,BASE_DEC));
}