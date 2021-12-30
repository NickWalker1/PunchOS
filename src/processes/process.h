#pragma once


#include "../lib/typedefs.h"

typedef struct PCB PCB_t;
#define PROC_MAGIC 0x12345678
#define MAX_THREADS 64
#define TIME_SLICE 4

typedef void proc_func(void* aux);


//assembly functions
PCB_t *context_switch(PCB_t *cur, PCB_t *next);
void first_switch();


typedef uint32_t p_id;


typedef enum proc_status
{
    P_READY,
    P_RUNNING,
    P_BLOCKED,
    P_ZOMBIE,
    P_DYING
} proc_status;


#include "../lib/list.h"
#include "../lib/string.h"

#include "../sync/sync.h"

#include "../paging/heap.h"
#include "../paging/paging.h"

extern pool_t K_virt_pool;


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


typedef struct runframe
{
    void* eip;              //Return addr
    proc_func* function;  //Function to run
    void* aux;              //function arguments
} __attribute__((packed, aligned(4))) runframe;

typedef struct context_switch_stack{
    uint32_t edi;               /*  0: Saved edi. */
    uint32_t esi;               /*  4: Saved esi. */
    uint32_t ebp;               /*  8: Saved ebp. */
    uint32_t ebx;               /* 12: Saved ebx. */
    void (*eip) (void);         /* 16: Return address. */
    PCB_t *cur;         /* 20: context_switch()'s CUR argument. */
    PCB_t *next;        /* 24: context_switch()'s NEXT argument. */
} __attribute__((packed, aligned(4))) context_switch_stack;


/* Stack frame for switch_entry(). */
typedef struct switch_entry_stack{
    void (*eip) (void);
}__attribute__((packed, aligned(4))) switch_entry_stack;

void processes_init();
PCB_t* create_proc(char* name, proc_func* func, void* aux);
void proc_tick();
void proc_yield();
void switch_complete(PCB_t* prev);
void schedule();
PCB_t* get_next_process();
void idle(semaphore* idle_started);
void proc_block();
void proc_unblock(PCB_t* p);
void proc_kill(PCB_t* p);
void run(proc_func *function, void *aux);
void proc_echo();

void *push_stack(PCB_t* p, uint32_t size);
bool is_proc(PCB_t *p);
void *get_pd();
void *get_esp();

PCB_t* current_proc();
p_id create_id();
uint32_t* get_base_page(uint32_t *addr);
void process_dump(PCB_t *p);
