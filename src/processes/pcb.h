#pragma once

#include "../lib/typedefs.h"

#define MAX_PROCS 64

typedef int32_t p_id;
typedef struct PCB PCB_t;

#include "../lib/debug.h"
#include "../memory/paging.h"
#include "../memory/heap.h"


#define PROC_MAGIC 0x12345678



typedef enum proc_status
{
    P_READY,
    P_RUNNING,
    P_BLOCKED,
    P_ZOMBIE,
    P_DYING
} proc_status;



/* Process Control Block Struct */
struct  PCB{
    void *stack; /* DO NOT MOVE */


    p_id pid; /* Process ID */
    p_id ppid; /* Parent process ID */
    char name[16]; 

    proc_status status; //TODO check if required?

    bool dummy; /* Dummy value to know if it was the intial boot process before processing initialised */

    uint32_t thread_count;
    //list *threads
    TCB_t *threads[8];

    page_directory_entry_t* page_directory; /* NOTE: is virtual address not physical */

    /* Each process has it's own heap, this points to the start of it. */
    MemorySegmentHeader_t *heap_start_segment;

    //TODO implement heap_lock
    //lock heap_lock; /* To lock the heap between mutlithreaded processes */



    /* Virtual pool to store virtual page tracking info */
    virt_pool_t virt_pool;


    /* Magic value for validity check */
    uint32_t magic;
};


typedef struct proc_diagnostics{
    bool present;

    PCB_t *process;
    
    // uint32_t mem_usage; /* % Local Memory usage calculated using heap_usage() yield or block. */

    // /* Timing information */
    // uint32_t running_ticks; /* Count of ticks when has been running process */
    // uint32_t wait_ticks; /* Current wait time before rescheduling */
    // uint32_t average_latency; /* Average number of ticks between being scheduled and being run */
    // uint32_t scheduled_count; /* Number of times this process has been scheduled */

} proc_diagnostics_t;


bool is_proc(PCB_t *p);
PCB_t *get_proc(p_id pid);
PCB_t *current_proc();
void process_dump(PCB_t *p);
