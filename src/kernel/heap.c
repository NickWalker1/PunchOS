#include "heap.h"

/* pointer to the first head of the linked-list */
MemorySegmentHeader_t *firstSegment;

void intialiseHeap(void* base, void* limit){
    MemorySegmentHeader_t *s=(MemorySegmentHeader_t*) base;
    s->free=true;
    s->size=limit - base - sizeof(MemorySegmentHeader_t);
    s->next=0;
    s->previous=0;

    firstSegment=s;

}

void *malloc(uint32_t size){
    if(1) return 0;
    MemorySegmentHeader_t *currSeg=firstSegment;

    //traverse linked list to find one that meets conditions
    //if at end return NULL
    while(!currSeg->free || size > currSeg->size){
        currSeg=currSeg->next;
        if(!currSeg) return NULL;
    }
    /* Update the segment info */
    uint32_t init_size =currSeg->size;
    currSeg->size=size;
    currSeg->free=false;

    /* Quick check for heap corruption, as this should never exist that two free states are next to each other */
    // if(currSeg->next && currSeg->next->free)PANIC("Heap corruption. (Likely erronous writes)");


    /* Check if we can fit a new header in + some bytes to ensure it's actually useful. */
    if(init_size>sizeof(MemorySegmentHeader_t)+64){
        MemorySegmentHeader_t* newSegment = (MemorySegmentHeader_t*) ((uint32_t)currSeg + size+ sizeof(MemorySegmentHeader_t));
        newSegment->free=true;
        newSegment->magic=segment_magic;
        newSegment->previous=currSeg;
        newSegment->next=currSeg->next;
        newSegment->size=init_size-sizeof(MemorySegmentHeader_t)-size;

        if(currSeg->next) currSeg->next->previous=newSegment;
        currSeg->next=newSegment;

        //Make sure to return the start of the free memory space, not the space containing
        //the header information.
        return ++currSeg;
    }


    //cannot fit a new segmentHeader in.

    //adjust currSeg to be initial size
    currSeg->size=init_size;
    
    //Make sure to return the start of the free memory space, not the space containing
    //the header information.
    return ++currSeg;
}


void free(void* addr){
    MemorySegmentHeader_t *currSeg = (MemorySegmentHeader_t*) (addr - sizeof(MemorySegmentHeader_t));
    

    //sanity check
    if(currSeg->free) return;


    currSeg->free=true;

    //There can be at most one free block on either side. 
    //Eg: |Free|this block|Free|another block...
    // the case |Free|this block|Free|Free|... should never exist 

    //if the previous segment exists and is free
    if(currSeg->previous && currSeg->previous->free){
        MemorySegmentHeader_t* prevSeg = currSeg->previous;

        prevSeg->size=prevSeg->size + currSeg->size + sizeof(MemorySegmentHeader_t);        


        //update pointers to remove currSeg from linked list.
        prevSeg->next=currSeg->next;
        if(currSeg->next)
            currSeg->next->previous=prevSeg;

        //for the second if
        currSeg=prevSeg;
    }

    //if the next segment exists and is free
    if(currSeg->next && currSeg->next->free){
        currSeg->size=currSeg->size+currSeg->next->size + sizeof(MemorySegmentHeader_t);

        //update pointers to remove the next segment

        //if there is a following one
        if(currSeg->next->next){
            currSeg->next=currSeg->next->next;
            currSeg->next->previous=currSeg;
        }
        else{
            currSeg->next=0;
        }
        
    }

    return;
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