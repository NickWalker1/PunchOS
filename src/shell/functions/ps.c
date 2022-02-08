#include "ps.h"

#include "../../processes/process.h"
#include "../../threads/thread.h"
#include "../../memory/heap.h"

extern proc_diagnostics_t   proc_tracker[MAX_PROCS+1];
extern thread_diagnostics_t thread_tracker[256];
extern uint32_t total_ticks;


/* COMPLETELY BROKEN */
void ps(){
    clear_screen();
    print_from("Process Status",TOP_LEFT);

    /* Header line */
    int offset=TOP_LEFT+LINE_OFF;
    print_from("Name",offset);
    offset+=12*CHAR_OFF;
    print_from("PID",offset);
    offset+=4*CHAR_OFF;
    print_from("PPID",offset);
    offset+=5*CHAR_OFF;
    print_from("CPU",offset);
    offset+=4*CHAR_OFF;
    print_from("MEM",offset);
    offset+=4*CHAR_OFF;
    print_from("LAT",offset);

    int i;
    while(1){
        i=0;
        while(proc_tracker[i].present){
            clear_line(i+2);

            offset=LINE(i+2);
            proc_diagnostics_t *pd=&proc_tracker[i];
            PCB_t *p=pd->process;
            /* Name */
            print_from(p->name,offset);

            /* PID */
            offset+=12*CHAR_OFF;
            print_from(itoa(p->pid,str,BASE_DEC),offset);

            /* PPID */
            offset+=4*CHAR_OFF;
            print_from(itoa(p->ppid,str,BASE_DEC),offset);

            /* Summations over threads */
            uint32_t j;
            int CPU,MEM,LAT;
            CPU=MEM=LAT=0;
            for(j=0;j<p->thread_count;j++){
                int tid=p->threads[j]->tid;
                CPU+= thread_tracker[tid].running_ticks;
                MEM+= thread_tracker[tid].mem_usage;
                LAT+= thread_tracker[tid].average_latency;
            }
            

            /* CPU */
            
            // offset+=5*CHAR_OFF;
            // print_from(itoa(pd->running_ticks*100/total_ticks,str,BASE_DEC),offset);
            
            // /* MEM */
            // offset+=4*CHAR_OFF;
            // print_from(itoa(pd->mem_usage,str,BASE_DEC),offset);

            // /* LATENCY */
            // offset+=4*CHAR_OFF;
            // print_from(itoa(pd->average_latency,str,BASE_DEC),offset);


            i++;
            set_cursor(LINE(i+5));
        }
        thread_sleep(1,UNIT_SEC);
    }







}