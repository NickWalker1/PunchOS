#include "process.h"


//setup data stuff
static PCB_t* init_proc; /* Main kernel and initial PCB_t stored at K_THREAD_BASE */
static PCB_t* idle_proc; /* Just spins */

//Declare some useful data structures 
//TODO  proper address space shit between processes
static list*   all_procs;
static list* ready_procs;
static list* sleeper_list;


static int tick_count;

static int num_PCB_ts=0;

/* Changes the current running code into the main kernel PCB_t.
    and creates idle PCB_t.*/
void processes_init(){
    int_disable(); //should already be disabled but yeah
    //TODO change pit interrupt handler to PCB_t tick handler from default
    // but currently just tick handler.
    int esp  = get_esp();
    esp=esp-(esp%PGSIZE)-PGSIZE;
    init_proc = (PCB_t*)esp;

    // if(init_proc!=current_proc()) PANIC("stack in wrong place");


    init_proc->magic=PROC_MAGIC;
    init_proc->id=create_id();
    // init_proc->name="KERNEL";
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
    PCB_t* tmp=create_proc("Idle",idle,&init_started);

    int_enable();
    println(itoa(&init_started,str,BASE_HEX));
    println(itoa(init_started.waiters,str,BASE_HEX));
    sema_down(&init_started); //when sema_down is called the PCB_t will
    //block and schedule the next PCB_t. This next PCB_t will be
    // the idle PCB_t as the last thing PCB_t_create does is unblocks
    // the PCB_t by changing the state and adding it to the ready queue


}

/* allocates a page in kernel space for this PCB_t and sets
 * some basic info in PCB_t struct and returns 
 */
PCB_t* create_proc(char* name, proc_func* func, void* aux){
    PCB_t* new= (PCB_t*) palloc_kern(1,F_ASSERT | F_ZERO |F_VERBOSE);
    new->id=create_id();
    new->magic=PROC_MAGIC;
    // strcpy(new->name,name);  //TODO implement
    new->page_directory=get_pd();
    new->pool=(void*) &K_virt_pool; //TODO UPDATE
    new->priority=1;
    new->stack=((uint32_t)new)+PGSIZE; /* initialise to top of page */
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
    //TODO get PCB_t to do some analytics n shit
    timer_tick();
    // sleep_tick();

    //Preemption
    if(++tick_count >= TIME_SLICE){
        proc_yield();
    }
}

void proc_yield(){
    PCB_t* p = current_proc();

    int int_level=int_disable();
    p->status=P_READY;

    //appending to ready processes if not idle process
    if(p!=idle_proc)
        append(ready_procs,p);

    schedule();

    //renable intterupts to previous level
    int_set(int_level);
}


void switch_complete(PCB_t* prev){
    PCB_t* curr = current_proc();
    curr->status=P_RUNNING;
    tick_count=0;
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
        // println("switching from: ");
        // print(itoa((uint32_t)curr,str,BASE_HEX));
        // print(" to ");
        // print(itoa((uint32_t)next,str,BASE_HEX));

        prev=context_switch(curr,next);
        
    }

    //schedule the old proc backinto ready queue
    //and update the new proc details 
    switch_complete(prev);
}

PCB_t* get_next_process(){
    //aquire ready queue lock
    //super basic round robin approach
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

        /* Re-enable interrupts and wait for the next one.

            The `sti' instruction disables interrupts until the
            completion of the next instruction, so these two
            instructions are executed atomically.  This atomicity is
            important; otherwise, an interrupt could be handled
            between re-enabling interrupts and waiting for the next
            one to occur, wasting as much as one clock tick worth of
            time.

            See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
            7.11.1 "HLT Instruction". */
        asm volatile ("sti; hlt" : : : "memory");
    }
}

void proc_block(){
    if(int_get_level()) PANIC("Cannot block without interrupts off");

    current_proc()->status=P_BLOCKED; 

    schedule();
    /* this is fine as when something is scheduled 
     * it is popped from the ready queue, 
     * so simply marking it as blocked and scheduling
     * something else will in effect block the PCB_t
     */
}

void proc_unblock(PCB_t* p){
    if(!is_proc(p)) PANIC("NOT THREAD");
    if(p->status!=P_BLOCKED) PANIC("UNBLOCKING NON-BLOCKED Thread");

    int level=int_disable();
    
    append(ready_procs,p);
    p->status=P_READY;

    int_set(level);

}

void proc_kill(PCB_t* p){
    println("KILLING PROCESS: ");
    print(itoa(p->id,str,BASE_DEC));
    remove(all_procs,p);
    remove(ready_procs,p);
    //TODO FREE PAGE

}

static void run(proc_func* function, void* aux){
    if(function==NULL) PANIC("NULL FUNCTION");
    int_enable();

    //do the work
    function(aux);
    
    PCB_t* p= current_proc();
    p->status=P_DYING;

    int_disable();
    schedule();
}

/*
void sleep_tick(){
    int i;
    for(i=0;i<sleeper_list->size;i++){
        sleeper* s=list_get(sleeper_list,i);
        s->tick_remaining--;
        if(s->tick_remaining==0){
            //free(s);
            int level= int_disable();
            remove(sleeper_list,s);
            proc_unblock(s->t);
            int_set(level);
        }
    }
}
*/
/*
void PCB_t_sleep(PCB_t* t, uint32_t ticks, uint8_t flags){
    if(ticks==0) return;
    if(!is_proc(t)) PANIC("ATTEMPTED TO SLEEP NON-THREAD");

    sleeper* s= malloc(sizeof(sleeper));
    s->t=t;
    
    if(flags&UNIT_TICK){
        s->tick_remaining=ticks;
    }
    if(flags&UNIT_SEC){
        s->tick_remaining=ticks*18; //TODO do proper checks for bit overflow
    }
    else{
        //default to tick
        s->tick_remaining=ticks;
    }

    int level=int_disable();
    append(sleeper_list,s);
    t->status=P_BLOCKED;
    remove(ready_procs,t);
    int_set(level);

}
*/
void proc_echo(){
    while(1){
        int a=0;
        for(int i=0;i<3000000000;i++) a = a+1;
        // list_dump(ready_procs);
        println("Curproc:");print(itoa(current_proc(),str,BASE_HEX));
        println("Completed itteration.");
    }
}

//-----------------------HELPERS--------------------------------


static void* push_stack(PCB_t* p, uint32_t size){
    if(!is_proc(p)) PANIC("pushing stack to non-PCB_t");
    p->stack-=size;
    return p->stack;
}


bool is_proc(PCB_t* t){
    return t->magic==PROC_MAGIC;
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

    if(p->magic != PROC_MAGIC) PANIC("TRIED TO RETRIEVE NON-THREAD PAGE");
    return p;
}


p_id create_id(){
    return ++num_PCB_ts;
}

/* Rounds down address to nearest 4k alligned number */
uint32_t* get_base_page(uint32_t* addr){
    //TODO this is so fucking ugly surely it must be fixable
    uint32_t remainder = (uint32_t)addr % PGSIZE;
    uint32_t base = (uint32_t)addr;

    return (uint32_t*)(base-remainder);
}

/* Gets the contents of the current PCB_t and prints it */
void process_dump(PCB_t* p){
    if(p==0) PANIC("NO CURRENT THREAD");
    

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