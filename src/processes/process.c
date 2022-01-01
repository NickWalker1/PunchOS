#include "process.h"

//TODO  proper address space shit between processes



//setup data stuff
static PCB_t* init_proc; /* Main kernel and initial PCB_t stored at K_THREAD_BASE */
static PCB_t* idle_proc; /* Just spins */

//Declare some useful data structures 
static list*   all_procs;
static list* ready_procs;
static list* sleeper_list;




static int total_tick;

static int cur_tick_count;

static int num_procs=0;

/* Changes the current running code into the main kernel process.
    and creates idle process.*/
void processes_init(){
    int_disable(); //should already be disabled but yeah
    
    //counts total number of ticks for performance reports
    total_tick=0;

    int esp  = (int) get_esp();
    esp=esp-(esp%PGSIZE)-PGSIZE;
    init_proc = (PCB_t*)esp;

    // if(init_proc!=current_proc()) PANIC("stack in wrong place");


    init_proc->magic=PROC_MAGIC;
    init_proc->id=create_id();
    strcpy(init_proc->name,"Kernel Main");
    init_proc->page_directory=(page_directory_entry_t*) get_pd();
    init_proc->pool=(void*) &K_virt_pool;
    init_proc->priority=1;
    init_proc->status=P_RUNNING;
    init_proc->stack=get_esp(); //Goes to top of page and works downwards.
    

    all_procs=list_init_with(init_proc);
    ready_procs=list_init();
    sleeper_list=list_init();


    semaphore init_started;
    sema_init(&init_started,0);
    
    create_proc("Idle",(proc_func*) idle,&init_started);

    int_enable();
    sema_down(&init_started); //when sema_down is called the process will
    //block and schedule the next process. This next process will be
    // the idle process as the last thing create_proc does is unblock
    // the process by changing the state and adding it to the ready queue
}



/* Allocates a page in kernel space for the PCB and sets
 * some basic info in PCB_t struct and returns pointer to it.
 */
PCB_t* create_proc(char* name, proc_func* func, void* aux){
    PCB_t* new= (PCB_t*) palloc_kern(1,F_ASSERT | F_ZERO );
    new->id=create_id();
    new->magic=PROC_MAGIC;
    strcpy(new->name,name); 
    new->page_directory=get_pd();
    new->pool=(void*) &K_virt_pool; //TODO UPDATE
    new->priority=1;
    new->stack=(void*) ((uint32_t)new)+PGSIZE; /* initialise to top of page */
    new->status=P_BLOCKED;

    int int_level = int_disable();

    //Context for these next few stack pushes:
    //On each function call a few things are pushed to the stack.
    //Firstly: the address of the line to return to when the ret instruction
    //is called, this is how nested calls work.
    //Thus each stack struct starts with the eip value of the function of which
    //to 'return' to, but infact it has never been there before. Ha sneaky.
    //Secondly: each of the function arguments
    //Finally: Some default values to pretend it has just come from an interrupt

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

    
    int_set(int_level);

    //Add to all PCB_ts list
    append(all_procs,new);

    /* add to ready queue */
    proc_unblock(new);

    return new;
}

/* called by PIT interrupt handler */
void proc_tick(){
    //report OK to PIT so it can send the next one.
    outportb(PIC1_COMMAND, PIC_EOI);

    //TODO get proc to do some analytics n shit


    timer_tick();
    sleep_tick();

    total_tick++;

    //Preemption
    if(++cur_tick_count>= TIME_SLICE){
        proc_yield();
    }
}

/* Reschedules a process by adding it
 *  to the appropriate queue.
 * Must be called with interrupts disabled.*/
void proc_reschedule(PCB_t *p){

    //appending to ready processes if not idle process
    if(p!=idle_proc)
        append(ready_procs,p);
    
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
    //TODO update cr3 if required.
    //TODO if PCB_t is dying kill it 
    if(prev->status==P_DYING) proc_kill(prev);

}


/* must be called with interrupts off */
void schedule(){
    PCB_t* curr = current_proc();
    PCB_t* next = get_next_process();
    PCB_t* prev = curr; //in case of no switch

    if(int_get_level()) PANIC("SCHEDULING WITH INTERUPTS ENABLED");
    if(curr->status==P_RUNNING) PANIC("Current process is still running...");

    // println("curr:");
    // print(itoa(curr->id,str,BASE_DEC));
    // print(" next: ");
    // print(itoa(next->id,str,BASE_DEC));


    if(curr!=next){
        println("switching from: ");
        // print(itoa((uint32_t)curr,str,BASE_HEX));
        print(curr->name);
        print(" to ");
        // print(itoa((uint32_t)next,str,BASE_HEX));
        print(next->name);

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
    PCB_t* p = (PCB_t*)pop(ready_procs);
    return p;
}


/* function run by idle process*/
void idle(semaphore* idle_started){
    idle_proc=current_proc();
    sema_up(idle_started);
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

    schedule();
    /* this is fine as when something is scheduled 
     * it is popped from the ready queue, 
     * so simply marking it as blocked and scheduling
     * something else will in effect block the process
     */
}


/* Unblocks the current process and adds it to the ready queue. */
void proc_unblock(PCB_t* p){
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


    println("KILLING PROCESS: ");
    print(itoa(p->id,str,BASE_DEC));

    remove(all_procs,p);
    remove(ready_procs,p);
    
    //TODO FREE ALL RELATED MEMORY

    free_virt_page(p,1);
}


/* Wrapper function for processes running the given function.
 * Will kill the process when function returns */
void run(proc_func* function, void* aux){
    ASSERT(function!=NULL,"Cannot run NULL function");

    int_enable();

    //do the work
    function(aux);
    
    PCB_t* p= current_proc();
    p->status=P_DYING;

    int_disable();
    schedule();
}


/* On each tick, decrements process wait counters.
 * If counter now 0, wakeup that process */
void sleep_tick(){
    uint32_t i;
    for(i=0;i<sleeper_list->size;i++){
        sleeper* s=list_get(sleeper_list,i);
        s->tick_remaining--;
        if(s->tick_remaining==0){
            int level= int_disable();
            println("Waking: ");print(s->waiting->name);

            remove(sleeper_list,s);
            proc_unblock(s->waiting);

            int_set(level);
        }
    }
}


/* Will sleep the current process.
 * Format var either UNIT_TICK or UNIT_SEC.
 * 18 ticks per second by default. */
void proc_sleep(uint32_t time, uint8_t format){
    if(time==0) return;

    sleeper* s= malloc(sizeof(sleeper));
    s->waiting=current_proc();
    
    if(format&UNIT_TICK){
        s->tick_remaining=time;
    }
    if(format&UNIT_SEC){
        s->tick_remaining=time*18; //TODO do proper checks for bit overflow
    }
    else{//default to tick
        s->tick_remaining=time;
    }

    int level=int_disable();
    append(sleeper_list,s);

    proc_block();

    //process now awake
    //resets interrupt state to before it slept.
    int_set(level);
}


/* Test Function */
void proc_echo(){
    while(1){
        int a=0;
        for(int i=0;i<2000000000;i++) a = a+1;
    }
}


/* Test Function */
void proc_test_A(){
    proc_sleep(3,UNIT_SEC);
    while(1){
        println("proc A");
        proc_sleep(1,UNIT_SEC);
    }
}


//-----------------------HELPERS--------------------------------


void* push_stack(PCB_t* p, uint32_t size){
    if(!is_proc(p)) PANIC("pushing stack to non-process");
    p->stack-=size;
    return p->stack;
}

/* Checks if the process is not corrupted by checking magic value */
bool is_proc(PCB_t* p){
    return p->magic==PROC_MAGIC;
}


/* Returns address stored in cr3 register */
void* get_pd(){
    void* pd;
    asm("mov %%cr3, %0" : "=g"(pd));
    return (void*) pd;
}


/* Returns current esp value */
void* get_esp(){
    void* esp;

    asm("mov %%esp, %0" : "=g" (esp));
    return esp;
}


/* Returns pointer to current PCB_t */
PCB_t* current_proc(){
    PCB_t* p= (PCB_t*) get_base_page((uint32_t*)get_esp());

    if(p->magic != PROC_MAGIC) PANIC("Attempted to retrieve non-process page or process corrupted");
    return p;
}


p_id create_id(){
    //TODO improve
    return ++num_procs;
}

/* Rounds down address to nearest 4k alligned number */
uint32_t* get_base_page(uint32_t* addr){
    //TODO this is so fucking ugly surely it must be fixable
    uint32_t remainder = (uint32_t)addr % PGSIZE;
    uint32_t base = (uint32_t)addr;

    return (uint32_t*)(base-remainder);
}

/* Gets the contents of the current process and prints it */
void process_dump(PCB_t* p){
    if(!is_proc(p)) PANIC("Cannot p-dump non-process");
    

    println("ID: ");
    print(itoa(p->id,str,BASE_DEC));
    println("Name: ");
    print(p->name);
    println("Status: ");
    print(itoa(p->status,str,BASE_DEC));
    println("Stack: ");
    print(itoa((uint32_t)p->stack,str,BASE_HEX));
    println("PD: ");
    print(itoa((uint32_t)p->page_directory,str,BASE_HEX));
    println("Pool: ");
    print(itoa((uint32_t)p->pool,str,BASE_HEX));
    println("Priority: ");
    print(itoa(p->priority,str,BASE_HEX));
    println("Magic: ");
    print(itoa(p->magic,str,BASE_HEX));
}

void ready_dump(){
    list_dump(ready_procs);
}