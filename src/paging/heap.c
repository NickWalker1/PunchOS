#include "heap.h"

/* pointer to the first head of the linked-list */
MemorySegmentHeader_t *firstSegment;

void intialiseHeap(void* base, void* limit){
    //To be implemented.
    firstSegment=(MemorySegmentHeader_t*) base;
    firstSegment->free=true;
    firstSegment->size=limit - base - sizeof(MemorySegmentHeader_t);
    firstSegment->next=0;
    firstSegment->previous=0;
}

void *malloc(uint32_t size){
    //To be implemented.
    MemorySegmentHeader_t *currSeg = firstSegment;

    //traverse linked list to find one that meets conditions
    //if at end return NULL
    while(!currSeg->free || size > currSeg->size){
        currSeg=currSeg->next;
        if(!currSeg) return NULL;
    }

    //update the segment info
    uint32_t init_size =currSeg->size;
    currSeg->size=size;
    currSeg->free=false;
    

    //first check if there isn't a next segment, or the next segment is not free
    if(!currSeg->next || !currSeg->next->free){
        //now need to create a new segment

        //check if we can fit a new header in + some bytes to ensure it's actually useful.
        if(init_size>sizeof(MemorySegmentHeader_t)+64){
            MemorySegmentHeader_t* newSegment = (MemorySegmentHeader_t*) ((uint32_t)currSeg + size+ sizeof(MemorySegmentHeader_t));
            newSegment->free=true;
            newSegment->previous=currSeg;
            newSegment->next=currSeg->next;
            newSegment->size=init_size-sizeof(MemorySegmentHeader_t);

            currSeg->next->previous=newSegment;
            currSeg->next=newSegment;

            //Make sure to return the start of the free memory space, not the space containing
            //the header information.
            return (uint32_t*) ((uint32_t)currSeg+sizeof(MemorySegmentHeader_t));
        }
  

        //cannot fit a new segmentHeader in.

        //adjust currSeg to be initial size
        currSeg->size=init_size;
        
        //Make sure to return the start of the free memory space, not the space containing
        //the header information.
        return (uint32_t*) ((uint32_t)currSeg+sizeof(MemorySegmentHeader_t));
    }


    //next segment must therefore be free!
    
    MemorySegmentHeader_t *nextSeg=currSeg->next;

    //logic to "move" next header over.
    MemorySegmentHeader_t *newNextSeg = (MemorySegmentHeader_t*) (uint32_t)currSeg+ size+sizeof(MemorySegmentHeader_t);
    newNextSeg->size=nextSeg->size + init_size; //Do not need to adjust for sizeof(MSH) as is only moved
    newNextSeg->free=true;
    //copy pointers over as these are still valid.
    newNextSeg->next=nextSeg->next;
    newNextSeg->previous=newNextSeg->previous;

    //can set the nextSeg struct to 0 to ensure no horrid bugs later
    nextSeg->free=0;
    nextSeg->next=0;
    nextSeg->previous=0;
    nextSeg->size=0;

    //Make sure to return the start of the free memory space, not the space containing
    //the header information.
    return (uint32_t*) ((uint32_t)currSeg+sizeof(MemorySegmentHeader_t));
}


void free(void* addr){
    //Assumption made that addr is base of the free space
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

//used to wipe heap space clean for testing
//only call once heap initialised.
void clear_heap(void* base_heap){
    memset(base_heap,0,0x1000);
}