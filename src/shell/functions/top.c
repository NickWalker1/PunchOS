#include  "top.h"

#include "../../lib/screen.h"
#include "../../threads/thread.h"


extern thread_diagnostics_t thread_tracker[257];

extern int total_ticks;


void top(){
    clear_screen();
    print_from("Thread Info",TOP_LEFT);

    /* Header line */
    int offset=TOP_LEFT+LINE_OFF;
    print_from("Name",offset);
    offset+=12*CHAR_OFF;
    print_from("TID",offset);
    offset+=4*CHAR_OFF;
    print_from("PID",offset);
    offset+=4*CHAR_OFF;
    print_from("CPU",offset);
    offset+=4*CHAR_OFF;
    print_from("MEM",offset);
    offset+=4*CHAR_OFF;
    print_from("LAT",offset);
    offset+=4*CHAR_OFF;
    print_from("PRIO",offset);

    int i;

    while(1){
        i=1;
        // clear_screen();

        while(thread_tracker[i].present){
            clear_line(i+1);

            offset=LINE(i+1);
            thread_diagnostics_t *pd=&thread_tracker[i];
            TCB_t *t=pd->thread;

            /* Name */
            print_from(t->name,offset);

            /* TID */
            offset+=12*CHAR_OFF;
            print_from(itoa(t->tid,str,BASE_DEC),offset);

            /* PID */
            offset+=4*CHAR_OFF;
            print_from(itoa(t->owner_pid,str,BASE_DEC),offset);

            /* CPU */
            offset+=4*CHAR_OFF;
            print_from(itoa(pd->running_ticks*100/total_ticks,str,BASE_DEC),offset);
            
            /* MEM */
            offset+=4*CHAR_OFF;
            print_from(itoa(pd->mem_usage,str,BASE_DEC),offset);

            /* LATENCY */
            offset+=4*CHAR_OFF;
            print_from(itoa(pd->average_latency,str,BASE_DEC),offset);

            /* PRIORITY */
            offset+=4*CHAR_OFF;
            print_from(itoa(t->priority,str,BASE_DEC),offset);

            i++;
            set_cursor(BOTTOM_LEFT);
        }
        thread_sleep(1,UNIT_SEC);
    }







}