#include "heap.h"

#include "../lib/screen.h"
#include "../processes/pcb.h"

MemorySegmentHeader_t *shared_first_seg;

lock shared_heap_lock;


extern bool multi_processing_enabled;


/* Initialise shared kernel heap space */
MemorySegmentHeader_t *intialise_heap(void *base, void *limit){
    MemorySegmentHeader_t *s=(MemorySegmentHeader_t*) base;
    s->free=true;
    s->size=limit - base - sizeof(MemorySegmentHeader_t);
    s->next=0;
    s->previous=0;

    return s;
}


/* Returns pointer to the start of size many bytes in process' dynamic memory space, returns NULL on failure */
void *malloc(uint32_t size){
    ASSERT(multi_processing_enabled,"Must enable multiprocesing before using malloc");
    void *addr=alloc(size,current_proc()->heap_start_segment);
    if(!addr) KERN_WARN("malloc failed");
    return addr;
}


/* Returns pointer to the start of size many bytes in shared kernel dynamic memory space, returns NULL on failure.
 * May block so must not be called inside interrupt handler. */
void *shr_malloc(uint32_t size){

    int level= int_disable();

    void* addr=alloc(size,shared_first_seg);

    if(!addr)
        KERN_WARN("shr_malloc failed");
    

    int_set(level);
    
    return addr;
}


/* Should not be called by the programmer. Use malloc or shr_malloc instead */
void *alloc(uint32_t size, MemorySegmentHeader_t *start_seg){
    MemorySegmentHeader_t *currSeg=start_seg;

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

    PANIC("FUCKED IT");

    //next segment must therefore be free
    
    MemorySegmentHeader_t *nextSeg=currSeg->next;

    //logic to "move" next header over.
    MemorySegmentHeader_t *newNextSeg = (MemorySegmentHeader_t*) (uint32_t)currSeg+ size+sizeof(MemorySegmentHeader_t);
    newNextSeg->size=nextSeg->size + init_size; //Do not need to adjust for sizeof(MSH) as is only moved
    newNextSeg->free=true;
    newNextSeg->magic=segment_magic;

    //copy pointers over as these are still valid.
    newNextSeg->next=nextSeg->next;
    newNextSeg->previous=newNextSeg->previous;

    //can set the nextSeg struct to 0 to ensure no horrid bugs later
    nextSeg->free=0;
    nextSeg->next=0;
    nextSeg->previous=0;
    nextSeg->size=0;
    nextSeg->magic=0;

    //Make sure to return the start of the free memory space, not the space containing
    //the header information.
    return ++currSeg;
}


/* Free the associated memory segment with addr */
void free(void* addr){

    //Assumption made that addr is base of the free space
    MemorySegmentHeader_t *currSeg = (MemorySegmentHeader_t*) (addr - sizeof(MemorySegmentHeader_t));
    
    if(currSeg->magic!=segment_magic){
        KERN_WARN("Attempted to free non-base address");
        return;
    }

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


/* Frees memory from shared kernel space.
 * May block so must not be called inside interrupt handler */ 
void shr_free(void *addr){
    int level = int_disable(); 
    free(addr);
    int_set(level);
}


/* Returns integer between 0 and 100 of the percentage heap usage given a start pointer */
uint32_t heap_usage(MemorySegmentHeader_t *s){
    uint32_t used,all,size;
 
    used=0;
    all=0;
    while(s!=NULL){
        size=s->size+sizeof(MemorySegmentHeader_t);//TODO think about this
        all+=size;
        if(!s->free)
            used+=size;
        
        s=s->next;
    }

    return used*100/all;
}

/* Returns the % usage of the shared heap space */
uint32_t get_shared_heap_usage(){
    lock_acquire(&shared_heap_lock);
    uint32_t usage=heap_usage(shared_first_seg);
    lock_release(&shared_heap_lock);
    return usage;
}


/* Self explanatory */
void clear_heap(void* base_heap, int pg_count){
    memset(base_heap,0,PGSIZE*pg_count);
}


/* Prints current cursor contents of heap + summary at end */
void shared_heap_dump(){
    MemorySegmentHeader_t *cur_seg=shared_first_seg;
    int used, total,count;
    used=total=count=0;
    println("----------\n");
    while(cur_seg!=NULL){
        count++;
        print("|");
        print(itoa(cur_seg->size,str,BASE_DEC));
        if(!cur_seg->free){
            print(" U");
            used+=cur_seg->size;
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
