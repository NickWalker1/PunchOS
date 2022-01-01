#pragma once

#include "../lib/typedefs.h"
#include "../paging/page.h"
#include "../paging/heap.h"

typedef uint32_t p_id;


typedef struct PCB PCB_t;


typedef enum proc_status
{
    P_READY,
    P_RUNNING,
    P_BLOCKED,
    P_ZOMBIE,
    P_DYING
} proc_status;

/* Process Control Block 
        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
             |              stack*             |
        0 kB +---------------------------------+

*/


/* Process Control Block Struct */
struct  PCB{
    void *stack; /* DO NOT MOVE */


    p_id id;
    char name[16];

    proc_status status;

    page_directory_entry_t* page_directory;
    void *pool; //was heap before

    int priority; /* 1 is highest priority, 5 is lowest NOT CURRENTLY USED WITH ROUND ROBIN AND SUBJECT TO CHANGE */

    uint32_t cpu_usage;

    //uint32_t sleep_deadline;

    //Each process has it's own heap, this points to the start of it.
    MemorySegmentHeader_t *firstSegment;    

    uint32_t magic;
}__attribute__((packed));
