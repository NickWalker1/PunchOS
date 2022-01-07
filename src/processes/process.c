#include "process.h"

#include "../lib/list.h"
#include "../lib/string.h"

#include "../sync/sync.h"

#include "../paging/heap.h"
#include "../paging/paging.h"

extern void main();
extern page_directory_entry_t *base_pd;


static PCB_t* idle_proc; /* Just spins */

bool multi_processing_enabled = false;

//Declare some useful data structures 
static list*   all_procs;
static list* ready_procs;
static list* sleeper_procs;

// True at index i-1 if a process is using pid i.
PCB_t *proc_tracker[MAX_PROCS];

int total_ticks;

static int cur_tick_count;


/* Begins mutliprocessing. THIS FUNCTION SHOULD NEVER RETURN */
void multi_proc_start(){
    ASSERT(is_proc(idle_proc),"idle process not created on multiproc start");
    ASSERT(!is_empty(ready_procs),"ready queue empty on mutliproc start");

    //Allow PIT interrupts
    block_PIT=0;

    total_ticks=0;

    int_disable();
    
    multi_processing_enabled=true;
    //Switch to init process
    schedule();
}




/* Creates new init and idle processes.
 * This thread of execution will not be returned to on successful context switching. */
void processes_init(){
    int_disable(); //Sanity check
    
    
    //counts total number of ticks for performance reports
    total_ticks=0;

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

    
    all_procs=list_init_shared();
    ready_procs=list_init_shared();
    sleeper_procs=list_init_shared();



    create_proc("init", main,NULL, PC_INIT); 
    idle_proc=create_proc("idle",idle,NULL,PC_IDLE);

    int_enable();

    // sema_down(&init_started); //when sema_down is called the process will
    //block and schedule the next process. This next process will be
    // the idle process as the last thing create_proc does is unblock
    // the process by changing the state and adding it to the ready queue
}

/* Allocates a page in kernel space for the PCB and sets
 * some basic info in PCB_t struct and returns pointer to it.
 */
PCB_t* create_proc(char* name, proc_func* func, void* aux, uint8_t flags){
    int int_level = int_disable();

    p_id pid = get_new_pid();
    if(pid==-1) return NULL;
    PCB_t *new= (PCB_t*) palloc_kern(1,F_ASSERT);
    if(!new) return NULL;

    proc_tracker[pid-1]=new;
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
    new->priority=1;
    new->stack=(void*) ((uint32_t)new)+PGSIZE; /* initialise to top of page */
    new->status=P_BLOCKED;
    new->running_ticks=0;
    new->wait_ticks=0;
    new->average_latency=0;
    new->scheduled_count=0;


    /*

        Context for these next few stack pushes:
    On each function call a few things are pushed to the stack.
    Firstly: the address of the line to return to when the ret instruction
    is called, this is how nested calls work.
    Thus each stack struct starts with the eip value of the function of which
    to 'return' to, but infact it has never been there before. Ha sneaky.
    Secondly: each of the function arguments
    Finally: Some default values to pretend it has just come from an interrupt

    */

    //run stack frame
    runframe* run_stack=(runframe*) push_stack(new,sizeof(runframe));/*sizeof(runframe)=24*/
    run_stack->eip=NULL;
    run_stack->function=func;
    run_stack->aux=aux;

    //stack for first_switch function. All it does is push the eip of run to the stack so that when ret is
    //called at the end of first_switch it will jump to the start of the run function.
    switch_entry_stack* switch_stack=(switch_entry_stack*) push_stack(new,sizeof(switch_entry_stack));/*sizeof(switch_entry_stack)=8*/
    switch_stack->eip=(void (*) (void)) run;

    //stack contents for context_switch function
    context_switch_stack* context_stack=(context_switch_stack*) push_stack(new,sizeof(context_switch_stack));/*sizeof(context_switch_stack)=40*/
    context_stack->eip = first_switch;
    context_stack->ebp = 0;

    
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


/* called by PIT interrupt handler */
void proc_tick(){
    //TODO REMOVE
    //report OK to PIT so it can send the next one.
    outportb(PIC1_COMMAND, PIC_EOI);

    //TODO get proc to do some analytics n shit
    //add to each proc in the ready queue their current latency
    //when actualy being scheduled add that latency to the total wait
    //and update average latency using a "num times scheduled count" avg_latency=(avg_latency*n-1 + latency)/n
    PCB_t *proc = current_proc();
    proc->running_ticks++;

    int i=0;
    while(proc_tracker[i]!=NULL){
        if(proc_tracker[i]==proc){
            i++;
            continue;
        }
        proc_tracker[i]->wait_ticks++;
        i++;
    }

    sleep_tick();

    total_ticks++;

    //Preemption
    if(++cur_tick_count>= TIME_SLICE){
        proc_yield();
    }
}

/* Reschedules a process by adding it
 *  to the appropriate queue.
 * Must be called with interrupts disabled.*/
void proc_reschedule(PCB_t *p){
    /* Reset waiting ticks counter to 0 */
    p->wait_ticks=0;

    /* Appending to ready processes if not idle process. */
    if(p!=idle_proc){
        append_shared(ready_procs,p);
    }
    
    p->status=P_READY;
}


/* Yeilds current proces by updating status, adding to ready processes
 * and scheduling a new one */
void proc_yield(){
    PCB_t* p = current_proc();

    int int_level=int_disable();
    

    proc_reschedule(p);

    schedule();

    //renable intterupts to previous level
    int_set(int_level);
}


/* Final stage of context switch.
 * Updates current processes status.
 * Kills previous process if it's dying.
 */
void switch_complete(PCB_t* prev){
    PCB_t* curr = current_proc();
    curr->status=P_RUNNING;

    cur_tick_count=0;

    //Update cr3 if required.
    update_pd(Kvtop(curr->page_directory));

    //update base heap pointer
    first_segment=curr->first_segment;

    if(prev->status==P_DYING) proc_kill(prev);
}



/* Primary context switching function
 * Must be called with interrupts off */
void schedule(){
    PCB_t* curr = current_proc();
    PCB_t* next = get_next_process();
    PCB_t* prev = curr; //in case of no switch
    if(int_get_level()) PANIC("SCHEDULING WITH INTERUPTS ENABLED");
    if(curr->status==P_RUNNING) PANIC("Current process is still running...");

    // println("curr:");
    // print(itoa(curr->pid,str,BASE_DEC));
    // print(" next: ");
    // print(itoa(next->pid,str,BASE_DEC));

    /* Update statistics */
    next->average_latency=((next->average_latency*next->scheduled_count)+next->wait_ticks)/(next->scheduled_count+1);
    next->scheduled_count++;
    
    if(curr!=next){
        if(0){
            println("switching from: ");
            // print(itoa((uint32_t)curr,str,BASE_HEX));
            print(curr->name);
            print(" to ");
            // print(itoa((uint32_t)next,str,BASE_HEX));
            print(next->name);
        }

        //perform the context switch function from pswap.asm
        prev=context_switch(curr,next);
        
    }

    //schedule the old proc backinto ready queue
    //and update the new proc details 
    switch_complete(prev);

}


/* Returns the next process to be scheduled.
 * Currently round robin approach */
PCB_t* get_next_process(){

    //round robin approach
    if(is_empty(ready_procs)){
        return idle_proc;
    }
    PCB_t* p = (PCB_t*)(pop_shared(ready_procs));
    return p;
}


/* function run by idle process*/
void idle(){
    // idle_proc=current_proc();
    // sema_up(idle_started);
    for(;;){
        /* Let someone else run. */
        int_disable ();
        proc_block ();

        //Re-enable interrupts and wait for the next one 
        asm volatile ("sti; hlt");
    }
}


/* Blocks the current process and schedules a new one
 * Must be called with interrupts disabled */
void proc_block(){
    if(int_get_level()) PANIC("Cannot block without interrupts off");

    current_proc()->status=P_BLOCKED; 

    //Force an early context switch
    schedule();
}


/* Unblocks the current process and adds it to the ready queue. */
void proc_unblock(PCB_t* p){
    helper_variable=0;
    if(!is_proc(p)) PANIC("NOT THREAD");
    if(p->status!=P_BLOCKED) PANIC("UNBLOCKING NON-BLOCKED Thread");

    int level=int_disable();
    proc_reschedule(p);

    int_set(level);

}

/* Kills the given process and frees all associated memory*/
void proc_kill(PCB_t* p){
    ASSERT(is_proc(p),"Cannot kill non-process");
    ASSERT(p!=current_proc(),"Cannot kill running process.");

    if(p->dummy) return; /* Nothing to do if dummy process */


    println("KILLING PROCESS: ");
    print(itoa(p->pid,str,BASE_DEC));
    
    remove_shared(all_procs,p);

    remove_shared(ready_procs,p);

    //TODO FIX THIS
    return;

    //TODO Free PID 
    
    //TODO update free all related memory
    free_virt_page(p->page_directory,1);
    
    free_virt_page(p,1);
}


/* Wrapper function for processes running the given function.
 * Will kill the process when function returns */
void run(proc_func* function, void* aux){
    ASSERT(function!=NULL,"Cannot run NULL function");
    
    PCB_t* p= current_proc();
    p->first_segment=proc_heap_init();


    int_enable();

    //do the work
    function(aux);
    
    p->status=P_DYING;

    int_disable();
    schedule();
}


/* On each tick, decrements process wait counters.
 * If counter now 0, wakeup that process */
void sleep_tick(){
    uint32_t i;
    for(i=0;i<sleeper_procs->size;i++){
        sleeper* s=list_get(sleeper_procs,i);
        s->tick_remaining--;
        if(s->tick_remaining==0){
            int level= int_disable();


            remove_shared(sleeper_procs,s);
            
            proc_unblock(s->waiting);

            shr_free(s);
            
            int_set(level);
        }
    }
}


/* Will sleep the current process.
 * Format var either UNIT_TICK or UNIT_SEC.
 * 18 ticks per second by default. 
 * NOTE: When woken up, it will be rescheduled
 * rather than switched to hence delay not exact.
 * For exact timings implement proc_alarm instead.*/
void proc_sleep(uint32_t time, uint8_t format){
    if(time==0) return;

    sleeper* s= shr_malloc(sizeof(sleeper));
    s->waiting=current_proc();
    
    if(format&UNIT_TICK){
        s->tick_remaining=time;
    }
    else if(format&UNIT_SEC){
        s->tick_remaining=time*18; //TODO do proper checks for bit overflow
    }
    else{//default to tick
        s->tick_remaining=time;
    }

    int level=int_disable();

    append_shared(sleeper_procs,s);

    proc_block();

    //process now awake
    //resets interrupt state to before it slept.
    int_set(level);
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
    }
}

void proc_test_hardwork(){
    int a=0;
    while(1){
        for(int i=0;i<1000000;i++)a++;
        print_from(itoa(a,str,BASE_HEX),BOTTOM_LEFT);
    }
}

/* Test Function */
void proc_heap_display(){
    while(1){
        proc_sleep(2,UNIT_SEC);
        // print_to(itoa(get_shared_heap_usage(),str,BASE_HEX),BOTTOM_RIGHT);
        get_shared_heap_usage();
        
        print_from(itoa(get_shared_heap_usage(),str,BASE_DEC),BOTTOM_LEFT);
        print_from("\%",BOTTOM_LEFT+2);
    }
}


//-----------------------HELPERS--------------------------------


/* Used to push data to a pcb stack.
 * ONLY use when initialising the process. */
void* push_stack(PCB_t* p, uint32_t size){
    if(!is_proc(p)) PANIC("pushing stack to non-process");
    p->stack-=size;
    return p->stack;
}


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
    while(proc_tracker[i-1]!=NULL && i<=MAX_PROCS) i++;

    if(i==MAX_PROCS) return -1;

    return i;
}


void ready_dump(){
    list_dump(ready_procs);
}