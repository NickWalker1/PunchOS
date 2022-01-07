#include "ps.h"

#include "../../processes/process.h"
#include "../../paging/heap.h"

extern PCB_t *proc_tracker[MAX_PROCS];
extern uint32_t total_ticks;

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
        while(proc_tracker[i]!=NULL){
            clear_line(i+2);
            offset=LINE(i+2);
            PCB_t *p=proc_tracker[i];
            print_from(p->name,offset);
            offset+=12*CHAR_OFF;
            print_from(itoa(p->pid,str,BASE_DEC),offset);
            offset+=4*CHAR_OFF;
            print_from(itoa(p->ppid,str,BASE_DEC),offset);
            offset+=5*CHAR_OFF;
            print_from(itoa(p->running_ticks*100/total_ticks,str,BASE_DEC),offset);
            offset+=4*CHAR_OFF;
            print_from(itoa(heap_usage(p->first_segment),str,BASE_DEC),offset);
            offset+=4*CHAR_OFF;
            print_from(itoa(p->average_latency,str,BASE_DEC),offset);


            i++;
            set_cursor(LINE(i+2));
        }
        proc_sleep(1,UNIT_SEC);
    }







}