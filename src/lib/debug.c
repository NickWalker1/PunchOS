#include "debug.h"

uint32_t helper_variable;

/* If cond fails, will PANIC with msg */
void ASSERT(bool cond, char *msg){
    if(!cond) PANIC(msg);
}

/* Displays PANIC screen and writes message */
void PANIC(char* msg){
    draw_panic_screen();

    println("PANIC");
    println("System Error Occured");
    println(msg);
    println("helper: ");
    print(itoa(helper_variable,str,BASE_HEX));

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

    halt();
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