#pragma once


#include "pcb.h"

#include "../lib/typedefs.h"

#define TIME_SLICE 4

#define PC_NFLAG 0 /* No flags */
#define PC_INIT 1<<0 /* Is the init proc */
#define PC_IDLE 1<<1 /* Is the idle proc */
#define PC_ADDR_DUP 1<<2


typedef void proc_func(void* aux);



#include "../memory/paging.h"



PCB_t *create_proc(char *name, proc_func *func, void *aux);

void processes_init();
MemorySegmentHeader_t *proc_heap_init();
void proc_diagnostics_init(int pid, PCB_t *p);
PCB_t *proc_create(char *name, proc_func *func, void *aux,uint8_t flags);
void proc_echo();
void proc_kill(PCB_t *p);
void sleep_tick();

void multi_proc_start();
void *get_pd();

p_id get_new_pid();



void proc_test_A();
void proc_test_hardwork();
void proc_heap_display();
