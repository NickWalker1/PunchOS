#include "process.h"

#include "../lib/list.h"
#include "../lib/string.h"

#include "../sync/sync.h"

#include "../memory/heap.h"
#include "../memory/paging.h"


#include "../threads/thread.h"

extern void main();
extern page_directory_entry_t *base_pd;


// static PCB_t* idle_proc; /* Just spins */

bool multi_processing_enabled = false;

//Declare some useful data structures 
static list*   all_procs;
// static list* ready_procs;
// static list* sleeper_procs;
extern list *ready_threads;
extern TCB_t *idle_thread;

// True at index i-1 if a process is using pid i. Is declared in pcb.c.
extern proc_diagnostics_t proc_tracker[MAX_PROCS];



/* Begins mutliprocessing. THIS FUNCTION SHOULD NEVER RETURN */
void multi_proc_start(){

    //TODO instead call external mutli-threading go function?
    ASSERT(multi_threading_init(),"Multi-threading init fail");

    //Allow PIT interrupts
    block_PIT=0;


    int_disable();
    
    multi_processing_enabled=true;
    //Switch to init process
    schedule();
}




/* Creates new init and idle processes.
 * This thread of execution will not be returned to on successful context switching. */
void processes_init(){
    int_disable(); //Sanity check
    
    

    /*Create a dummy process so it can be scheduled out of and killed.
    It must exist long enough to finish the setup process however, then the final section
    setup is to start multiprocesing. */ 
    int esp  = (int) get_esp();
    esp=esp-(esp%PGSIZE)-PGSIZE;
    PCB_t *dummy_proc = (PCB_t*)esp;
    dummy_proc->dummy=true;
    dummy_proc->magic=PROC_MAGIC;
    dummy_proc->pid=get_new_pid(); //TODO check this??
    dummy_proc->status=P_DYING;
    dummy_proc->page_directory=base_pd;

    lock_init(&shared_heap_lock);

    memset(proc_tracker,0,sizeof(proc_tracker));

    all_procs=list_init_shared();



    proc_create("init", main,NULL, PC_INIT); 

    int_enable();

    // sema_down(&init_started); //when sema_down is called the process will
    //block and schedule the next process. This next process will be
    // the idle process as the last thing proc_create does is unblock
    // the process by changing the state and adding it to the ready queue
}

/* Create a new process with name running func with arguments aux */
PCB_t *create_proc(char *name, proc_func *func, void *aux){
    return proc_create(name,func,aux,PC_NFLAG);
}



/* Allocates a page in kernel space for the PCB and sets
 * some basic info in PCB_t struct and returns pointer to it.
 */
PCB_t* proc_create(char *name, proc_func *func, void *aux, uint8_t flags){
    int int_level = int_disable();

    p_id pid = get_new_pid();
    if(pid==-1) return NULL;
    PCB_t *new= (PCB_t*) palloc_kern(1,F_ASSERT);
    if(!new) return NULL;


    /* Update pid and ppid */
    new->pid=pid;
    if(flags & PC_INIT){
        new->ppid=0;
    }
    else{
        if(flags & PC_IDLE){
            new->ppid=1; //Set to init proc
        }
        else{
            new->ppid=current_proc()->pid;
        }
    }

    new->magic=PROC_MAGIC;
    
    strcpy(new->name,name); 

    //TODO create the heap now can actually do that by modifying heap init function.

    //TODO do the bit that actually creates the new thread.

    // new->page_directory=Kptov(get_pd());
    if(flags&PC_ADDR_DUP){
        new->page_directory=virt_addr_space_duplication(current_proc()->page_directory);
    }
    else{
        new->page_directory=virt_addr_space_duplication(base_pd);
    }



    /* Initialses the processes individual virtual page pool */
    init_vpool(&new->virt_pool); 


    /* Default setup values */
    new->stack=(void*) ((uint32_t)new)+PGSIZE; /* initialise to top of page */
    // lock_init(&new->heap_lock);


    /* Initialise process diagnostics struct */
    proc_diagnostics_init(pid,new);




    
    /* Add to all procs list */
    append_shared(all_procs,new);


    int_set(int_level);


    /* add to ready queue */
    proc_unblock(new);

    return new;
}

/* Initialses the current processes' inidivual heap space.
 * NOTE: Can only be called from inside the process */
MemorySegmentHeader_t *proc_heap_init(){
    void *phys = get_next_free_phys_page(HEAP_SIZE,F_ASSERT);


    void *base = get_next_free_virt_page(HEAP_SIZE,F_ASSERT);

    int i;
    for(i=0;i<HEAP_SIZE;i++){
        map_page(phys+i*PGSIZE,base+i*PGSIZE,F_ASSERT);
    }

    
    return intialise_heap(base,base+HEAP_SIZE*PGSIZE);
}


/* Intialise process diagnostics struct */
void proc_diagnostics_init(int pid, PCB_t *p){
    proc_tracker[pid-1].present=true;
    proc_tracker[pid-1].process=p;
    proc_tracker[pid-1].running_ticks=0;
    proc_tracker[pid-1].wait_ticks=0;
    proc_tracker[pid-1].average_latency=0;
    proc_tracker[pid-1].scheduled_count=0;

}





/* Kills the given process and frees all associated memory*/
void proc_kill(PCB_t* p){
    ASSERT(is_proc(p),"Cannot kill non-process");
    ASSERT(p!=current_proc(),"Cannot kill running process.");

    if(p->dummy) return; /* Nothing to do if dummy process */


    // println("KILLING PROCESS: ");
    // print(itoa(p->pid,str,BASE_DEC));
    
    //TODO kill all threads too

    proc_tracker[p->pid-1].present=false;
    
    remove_shared(all_procs,p);



    //TODO clear all memory stuff not just return...
    return;

    // remove_shared(ready_procs,p);

    //TODO Free PID 
    
    //TODO update free all related memory
    free_virt_page(p->page_directory,1);
    
    free_virt_page(p,1);
}


/* Test Function */
void proc_echo(){
    while(1){
        int a=0;
        for(int i=0;i<1000000000;i++) a = a+1;
    }
}


/* Test Function */
void proc_test_A(){
    proc_sleep(1,UNIT_SEC);
    while(1){
        // println("proc "); print(current_proc()->name);
        proc_sleep(1,UNIT_SEC);
        malloc(256);
        
    }
}

void proc_test_hardwork(){
    int a=0;
    while(1){
        for(int i=0;i<1000000;i++)a++;
    }
}

/* Test Function */
void proc_heap_display(){
    while(1){
        proc_sleep(2,UNIT_SEC);
        
        print_to(itoa(get_shared_heap_usage(),str,BASE_DEC),BOTTOM_RIGHT-4);
        print_to("%",BOTTOM_RIGHT-2);
    }
}


//-----------------------HELPERS--------------------------------





/* Returns address stored in cr3 register */
void* get_pd(){
    void* pd;
    asm("mov %%cr3, %0" : "=g"(pd));
    return (void*) pd;
}


/* Returns an unused PID, and sets status to True in PCB_PID_TRACKER.
 * Returns -1 on failure. */
p_id get_new_pid(){
    int i=1;
    while(proc_tracker[i-1].present && i<=MAX_PROCS) i++;

    if(i==MAX_PROCS) return -1;

    proc_tracker[i-1].present=true;
    return i;
}
