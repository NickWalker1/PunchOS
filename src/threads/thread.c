#include "thread.h"


#include "sched.h"


#include "../lib/list.h"

#include "../processes/pcb.h"

TCB_t *idle_thread;


list *sleeper_threads;

int total_ticks;
int block_ticks;
int cur_tick_count;

//TODO gonna need some externs for the scheduling stuff?

extern MemorySegmentHeader_t *proc_heap_init();

thread_diagnostics_t thread_tracker[256];


bool multi_threading_init(){
    if(!scheduling_init()) return false;


    /* Create dummy thread that can be scheduled out of */
    int esp  = (int) get_esp();
    esp=esp-(esp%PGSIZE)-PGSIZE;
    TCB_t *dummy_thread = (TCB_t*)esp; /* Page allocated before main jump so this page is free */
    dummy_thread->owner_pid=0;
    dummy_thread->status=T_DYING;
    dummy_thread->magic=THR_MAGIC;
    dummy_thread->tid=0;
    thread_tracker[0].present=true; //TODO check
    dummy_thread->priority=1;


    sleeper_threads=list_init_shared();
    if(!sleeper_threads) return false;

    //counts total number of ticks for performance reports
    total_ticks=0;
    cur_tick_count=0;


    /* Create the idle thread */
    idle_thread=thread_create("idle",idle,NULL,1,THR_IDLE);



    return true;

}


TCB_t *thread_create(char *name, thread_func *func, void *aux,uint32_t owner_pid, uint8_t flags){
    int int_level = int_disable();


    t_id tid = get_new_tid();
    if(tid==-1) return NULL;
    TCB_t *new= (TCB_t*) palloc_kern(1,F_ASSERT);
    if(!new) return NULL;

    if(flags & THR_MAIN){
        new->is_main=true;
    }

    new->tid =tid;
    new->owner_pid=owner_pid;

    new->magic=THR_MAGIC;
    
    strcpy(new->name,name); 

    thread_diagnostics_insert(tid,new);

    /* Default setup values */
    new->stack=(void*) ((uint32_t)new)+PGSIZE; /* initialise to top of page */
    new->status=T_BLOCKED;


    //TODO add 1 to thread_diagnostics count n shit and the threadesses info


    
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


    int_set(int_level);


    /* add to ready queue */
    thread_unblock(new);

    return new;
}


/* called by PIT interrupt handler */
void thread_tick(){

    //TODO get thread to do some analytics n shit
    //add to each thread in the ready queue their current latency
    //when actualy being scheduled add that latency to the total wait
    //and update average latency using a "num times scheduled count" avg_latency=(avg_latency*n-1 + latency)/n

    total_ticks++;

    TCB_t *thread = current_thread();

    thread_tracker[thread->tid].running_ticks++;


    int i=1;
    while(thread_tracker[i].present){
        TCB_t *t = thread_tracker[i].thread;
        if(t->status==T_READY){
            thread_tracker[i].wait_ticks++;
        }
        i++;
    }

    sleep_tick();

    

    /* Preemption */
    if(++cur_tick_count>= TIME_SLICE){
        thread_yield();
    }
}


/* Yields current threades by updating status, adding to ready threades
 * and scheduling a new one */
void thread_yield(){
    TCB_t* t = current_thread();

    int int_level=int_disable();

    thread_tracker[t->tid].mem_usage=heap_usage(get_proc(t->owner_pid)->heap_start_segment);

    if(t!=idle_thread)
        thread_reschedule(t);

    schedule();

    //renable intterupts to previous level
    int_set(int_level);
}


/* Primary context switching function
 * Must be called with interrupts off */
void schedule(){
    TCB_t* curr = current_thread();
    TCB_t* next = get_next_thread();
    TCB_t* prev = curr; //in case of no switch
    if(int_get_level()) PANIC("SCHEDULING WITH INTERUPTS ENABLED");
    if(curr->status==T_RUNNING) PANIC("Current threadess is still running...");


    /* Update statistics */
    thread_diagnostics_t *td = &thread_tracker[next->tid];
    td->average_latency=((td->average_latency*td->scheduled_count)+td->wait_ticks)/(td->scheduled_count+1);
    td->scheduled_count++;


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

    //schedule the old thread backinto ready queue
    //and update the new thread details 
    switch_complete(prev);

}


/* Final stage of context switch.
 * Updates current threades status.
 * Kills previous thread if it's dying.
 */
void switch_complete(TCB_t* prev){
    TCB_t* curr = current_thread();
    curr->status=T_RUNNING;

    cur_tick_count=0;



    //Update cr3 if required.
    if(curr->owner_pid!=prev->owner_pid){
        update_pd(Kvtop(get_proc(curr->owner_pid)->page_directory));
    }


    if(prev->status==T_DYING) thread_kill(prev);
}


/* function run by idle process*/
void idle(){
    for(;;){
        /* Let someone else run. */
        int_disable();
        thread_block();

        //Re-enable interrupts and wait for the next one 
        asm volatile ("sti; hlt");
    }
}



/* Blocks the current thread and schedules a new one
 * Must be called with interrupts disabled */
void thread_block(){
    if(int_get_level()) PANIC("Cannot block without interrupts off");

    TCB_t *t=current_thread();
    t->status=T_BLOCKED; 

    //TODO update
    thread_tracker[t->tid].mem_usage=heap_usage(get_proc(t->owner_pid)->heap_start_segment);

    //Force an early context switch
    schedule();
}


/* Unblocks the current thread and adds it to the ready queue. */
void thread_unblock(TCB_t* t){
    if(!is_thread(t)) PANIC("Cannot unblock non-thread");
    if(t->status!=T_BLOCKED) PANIC("Cannot unblock non-blocked thread");

    int level=int_disable();
    thread_reschedule(t);

    int_set(level);
}

/* Kills the given thread and frees the associated memory */
void thread_kill(TCB_t *t){
    ASSERT(is_thread(t),"Cannot kill non-thread");
    ASSERT(t!=current_thread(),"Cannot kill own thread");


    //TODO implement
}


/* Wrapper function for threads running the given function.
 * Will kill the thread when function returns */
void run(thread_func *function, void *aux){
    ASSERT(function!=NULL,"Cannot run NULL function");
    
    /* Finalise the setup of process if required. */
    TCB_t *curr = current_thread();
    if(curr->is_main){
        get_proc(curr->owner_pid)->heap_start_segment=proc_heap_init();
    }


    int_enable();

    //do the work
    function(aux);
    
    curr->status=P_DYING;

    int_disable();
    schedule();
}


/* On each tick, decrements process wait counters.
 * If counter now 0, wakeup that process */
void sleep_tick(){
    uint32_t i;
    for(i=0;i<sleeper_threads->size;i++){
        sleeper* s=list_get(sleeper_threads,i);
        s->tick_remaining--;
        if(s->tick_remaining==0){
            int level= int_disable();


            remove_shared(sleeper_threads,s);
            
            thread_unblock(s->waiting);

            shr_free(s);
            
            int_set(level);
        }
    }
}


/* Will sleep the current thread.
 * Format var either UNIT_TICK or UNIT_SEC.
 * 18 ticks per second by default. 
 * NOTE: When woken up, it will be rescheduled
 * rather than switched to hence delay not exact.
 * For exact timings implement proc_alarm instead.*/
void thread_sleep(uint32_t time, uint8_t format){
    if(time==0) return;

    sleeper* s= shr_malloc(sizeof(sleeper));
    if(!s) PANIC("sleeper alloc failed");

    s->waiting=current_thread();

    
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

    if(!append_shared(sleeper_threads,s))
        PANIC("Reschedule fail.");

    thread_block();

    //thread now awake
    //resets interrupt state to before it slept.
    int_set(level);
}


//----------------Helpers-------------

/* Used to push data to a pcb stack.
 * ONLY use when initialising the process. */
void* push_stack(TCB_t* t, uint32_t size){
    if(!is_thread(t)) PANIC("pushing stack to non-process");
    t->stack-=size;
    return t->stack;
}

/* Returns a new unique thread identifier based on the thread tracker */
t_id get_new_tid(){
    int tid=1;
    /* 0 is the dummy thread hence MAX_THREADS+1 */
    while(thread_tracker[tid].present && tid<MAX_THREADS+1) tid++;

    if(tid==MAX_THREADS+1) return -1;

    return tid;
}

/* Inserts a new thread into the thread tracker */
void thread_diagnostics_insert(t_id tid, TCB_t *t){
    thread_diagnostics_t *td=&thread_tracker[tid];

    td->present=true;
    
    td->average_latency=0;
    td->mem_usage=0;
    td->running_ticks=0;
    td->scheduled_count=0;
    td->wait_ticks=0;

    td->thread=t;
}

/* Removes a thread from the thread tracker */
void thread_diagnostics_delete(t_id tid){
    thread_tracker[tid].present=false;
}