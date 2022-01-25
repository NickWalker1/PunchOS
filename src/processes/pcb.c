#include "pcb.h"

proc_diagnostics_t proc_tracker[MAX_PROCS];





/* Checks if the process is not corrupted by checking magic value */
bool is_proc(PCB_t* p){
    return p->magic==PROC_MAGIC;
}

PCB_t *get_proc(p_id pid){
    return proc_tracker[pid-1].process;
}

/* Returns pointer to current PCB_t */
PCB_t* current_proc(){
    TCB_t *t= (TCB_t*) get_base_page((uint32_t*)get_esp());
    ASSERT(is_thread(t),"Attempted to retreive non-thread page. Thread likely corrupted");
    PCB_t *p=get_proc((p_id)t->owner_pid);
    ASSERT(is_proc(p),"Thread has no owner process...");
    return p;
}



/* Gets the contents of the current process and prints it */
void process_dump(PCB_t* p){
    if(!is_proc(p)) PANIC("Cannot dump non-process");
    

    println("PID: ");
    print(itoa(p->pid,str,BASE_DEC));
    println("PPID: ");
    println(itoa(p->ppid,str,BASE_DEC));
    println("Name: ");
    print(p->name);
    // println("Status: ");
    // print(itoa(p->status,str,BASE_DEC));
    println("PD: ");
    print(itoa((uint32_t)p->page_directory,str,BASE_HEX));
    println("VPool FFIDX: ");
    print(itoa((uint32_t)p->virt_pool.first_free_idx,str,BASE_DEC));
    println("Thread count: ");
    print(itoa(p->thread_count,str,BASE_DEC));

}