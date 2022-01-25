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



#include "../memory/paging.h"



PCB_t *create_proc(char *name, proc_func *func, void *aux);

void processes_init();
MemorySegmentHeader_t *proc_heap_init();
void proc_diagnostics_init(int pid, PCB_t *p);
PCB_t *proc_create(char *name, proc_func *func, void *aux,uint8_t flags);
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
void proc_test_hardwork();
void proc_heap_display();
