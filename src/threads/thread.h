#pragma once


#include "tcb.h"


#define TIME_SLICE 4

typedef void thread_func(void *aux);

//assembly functions
TCB_t *context_switch(TCB_t *cur, TCB_t *next);
void first_switch();



#include "../memory/paging.h"

extern phys_pool_t K_virt_pool;


typedef struct runframe
{
    void* eip;              //Return addr
    thread_func* function;  //Function to run
    void* aux;              //function arguments
} __attribute__((packed, aligned(4))) runframe;

typedef struct context_switch_stack{
    uint32_t edi;               /*  0: Saved edi. */
    uint32_t esi;               /*  4: Saved esi. */
    uint32_t ebp;               /*  8: Saved ebp. */
    uint32_t ebx;               /* 12: Saved ebx. */
    void (*eip) (void);         /* 16: Return address. */
    TCB_t *cur;         /* 20: context_switch()'s CUR argument. */
    TCB_t *next;        /* 24: context_switch()'s NEXT argument. */
} __attribute__((packed, aligned(4))) context_switch_stack;


/* Stack frame for switch_entry(). */
typedef struct switch_entry_stack{
    void (*eip) (void);
}__attribute__((packed, aligned(4))) switch_entry_stack;


#define UNIT_TICK 1<<1
#define UNIT_SEC 1<<2

/* Struct to contain sleeping thread info */
typedef struct sleeper{
    uint32_t tick_remaining;
    PCB_t *waiting;
} sleeper;

bool mutli_threading_init();
void thread_create(char *name, thread_func *func, void *aux, uint32_t owner_pid, uint8_t flags);
void thread_tick();
void thread_yield();
void schedule();
void thread_block();
