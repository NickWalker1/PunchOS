#include "pcb.h"


/* Rounds down address to nearest 4k alligned number */
uint32_t* get_base_page(uint32_t* addr){
    //TODO this is so fucking ugly surely it must be fixable
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


/* Checks if the process is not corrupted by checking magic value */
bool is_proc(PCB_t* p){
    return p->magic==PROC_MAGIC;
}


/* Returns pointer to current PCB_t */
PCB_t* current_proc(){
    PCB_t* p= (PCB_t*) get_base_page((uint32_t*)get_esp());

    if(p->magic != PROC_MAGIC) PANIC("Attempted to retrieve non-process page or process corrupted");
    return p;
}


/* Gets the contents of the current process and prints it */
void process_dump(PCB_t* p){
    if(!is_proc(p)) PANIC("Cannot p-dump non-process");
    

    println("ID: ");
    print(itoa(p->pid,str,BASE_DEC));
    println("Name: ");
    print(p->name);
    println("Status: ");
    print(itoa(p->status,str,BASE_DEC));
    println("Stack: ");
    print(itoa((uint32_t)p->stack,str,BASE_HEX));
    println("PD: ");
    print(itoa((uint32_t)p->page_directory,str,BASE_HEX));
    println("Pool: ");
    print(itoa((uint32_t)p->virt_pool,str,BASE_HEX));
    println("Priority: ");
    print(itoa(p->priority,str,BASE_HEX));
    println("Magic: ");
    print(itoa(p->magic,str,BASE_HEX));
}