#include "tcb.h"

#include "../memory/page.h"


/* Rounds down address to nearest 4k alligned number */
uint32_t* get_base_page(uint32_t* addr){
    uint32_t remainder = (uint32_t)addr % PGSIZE;
    uint32_t base = (uint32_t)addr;

    return (uint32_t*)(base-remainder);
}


/* Returns current esp value */
void* get_esp(){
    void* esp;

    asm("mov %%esp, %0" : "=g" (esp));
    return esp;
}


/* Checks if the thread is not corrupted by checking magic value */
bool is_thread(TCB_t* p){
    return p->magic==THR_MAGIC;
}


/* Returns pointer to current TCB */
TCB_t* current_thread(){
    TCB_t* t= (TCB_t*)get_base_page((uint32_t*)get_esp());

    ASSERT(is_thread(t),"Attempted to retrieve non-thread page or thread corrupted");
    return t;
}


/* Gets the contents of the current thread and prints it */
void thread_dump(TCB_t* p){
    if(!is_thread(p)) PANIC("Cannot dump non-thread");
    

    println("ID: ");
    print(itoa(p->tid,str,BASE_DEC));
    println("Name: ");
    print(p->name);
    println("Status: ");
    print(itoa(p->status,str,BASE_DEC));
    println("Stack: ");
    print(itoa((uint32_t)p->stack,str,BASE_HEX));
    println("Priority: ");
    print(itoa(p->priority,str,BASE_HEX));
}