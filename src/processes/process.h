#pragma once


#include "pcb.h"

#include "../lib/typedefs.h"

#define TIME_SLICE 4

#define PC_NFLAG 0 /* No flags */
#define PC_INIT 1<<0 /* Is the init proc */
#define PC_IDLE 1<<1 /* Is the idle proc */
#define PC_ADDR_DUP 1<<2


typedef void proc_func(void* aux);

extern MemorySegmentHeader_t *first_segment;


//assembly functions
PCB_t *context_switch(PCB_t *cur, PCB_t *next);
void first_switch();



#include "../paging/paging.h"

extern phys_pool_t K_virt_pool;


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
MemorySegmentHeader_t *proc_heap_init();
PCB_t *create_proc(char* name, proc_func* func, void* aux,uint8_t flags);
void proc_tick();
void proc_yield();
void proc_reschedule(PCB_t *p);
void switch_complete(PCB_t *prev);
void schedule();
PCB_t *get_next_process();
void idle();
void proc_block();
void proc_unblock(PCB_t *p);
void proc_kill(PCB_t* p);
void run(proc_func *function, void *aux);
void proc_echo();
void sleep_tick();
void proc_sleep(uint32_t time, uint8_t format);

void multi_proc_start();
void *push_stack(PCB_t *p, uint32_t size);
void *get_pd();

p_id get_new_pid();
void ready_dump();

#define UNIT_TICK 1<<1
#define UNIT_SEC 1<<2

typedef struct sleeper{
    uint32_t tick_remaining;
    PCB_t *waiting;
} sleeper;

void proc_test_A();
void proc_heap_display();

