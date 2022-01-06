#include "debug.h"

void PANIC(char* msg){
    draw_panic_screen();

    println("PANIC");
    println("System Error Occured");
    println(msg);

    halt();
}


void draw_panic_screen(){
    int row,column;
    for(column=0;column<80;column++){
        for(row=0;row<25;row++){
            print_char_loc(' ',column,row,WHITE_ON_BLUE);
        }
    }
    set_cursor(get_screen_offset(0,0));
    
}

void halt(){
    __asm__ volatile("cli;hlt");
}