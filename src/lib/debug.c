#include "debug.h"

uint32_t helper_variable;

#include "../processes/pcb.h"

/* Character buffer for warnings  */
char WARN[WARN_CHAR_MAX*WARN_LINE_MAX];

int WARN_LINE =0;

/* If cond fails, will PANIC with msg */
void ASSERT(bool cond, char *msg){
    if(!cond) PANIC(msg);
}

/* Displays PANIC screen and writes message */
void PANIC(char* msg){
    int_disable();

    draw_panic_screen();

    println("PANIC");
    println("System Error Occured");
    println(msg);
    println("helper: ");
    print(itoa(helper_variable,str,BASE_HEX));

    WARN_DUMP();

    // print_to(itoa(get_shared_heap_usage(),str,BASE_DEC),BOTTOM_RIGHT-4);
    // print_to("%",BOTTOM_RIGHT-2);

    halt();
}

/* Sets screen to blue
 * Displays error msg
 * Dumps exception state
 */
void PANIC_EXC(char* msg, exception_state* state){
    /* Disable interrupts first so cannot be interrupted for robustness */
    int_disable();

    draw_panic_screen();

    println("PANIC");
    println("Unhandled Exception: ");
    print(msg);
    println("Helper: ");
    print(itoa(helper_variable,str,BASE_HEX));
    exception_state_dump(state);

    WARN_DUMP();

    print_to(itoa(get_shared_heap_usage(),str,BASE_DEC),BOTTOM_RIGHT-6);
    print_to("\%",BOTTOM_RIGHT-2);

    halt();
}

/* Prints the warning data to the PANIC screen */
void WARN_DUMP(){
    int line_off =LINE_OFF;
    print_from("| Kernel Warnings:",WARN_BASE_OFF);
    int l =0;
    while(WARN[l*WARN_CHAR_MAX]!=0 && l<WARN_LINE_MAX){
        print_from("| ",WARN_BASE_OFF+line_off);
        print_from(&WARN[l*WARN_CHAR_MAX],WARN_BASE_OFF+line_off+4);
        line_off+=LINE_OFF;
        l++;
    }

}


/* Kernel Warning function for debugging help. */
void KERN_WARN(char *msg){
    /* Handle buffer overflow */
    if(WARN_LINE==WARN_LINE_MAX){
        int i;
        for(i=1;i<WARN_LINE_MAX;i++){
            strcpy(&WARN[(i-1)*WARN_CHAR_MAX],&WARN[i*WARN_CHAR_MAX]);
        }
    }
    /* Copy warning message in */
    strcpy(&WARN[WARN_LINE*WARN_CHAR_MAX],msg);
    WARN_LINE=WARN_LINE+(strlen(msg)/WARN_LINE_MAX) + 1;

    /* Removed as too verbose */
    // println("Thread: ");
    // print(current_thread()->name);
    // print(" KERN WARN: ");
    // print(msg);
}


/* Sets entirety of the screen to blue */
void draw_panic_screen(){
    int row,column;
    for(column=0;column<80;column++){
        for(row=0;row<25;row++){
            print_char_loc(' ',column,row,WHITE_ON_BLUE);
        }
    }
    set_cursor(get_screen_offset(0,0));
    
}

/* Used to completely halt the system by disabling interrupts and halting execution */
void halt(){
    __asm__ volatile("cli;hlt");
}

/* Used to create a breakpoint exception */
void breakpoint(){
    __asm__ volatile("int $3");
}