#include "time.h"

extern int total_ticks;

/* Returns the number of ticks since process start */
int get_time(){
    return total_ticks;
}

/* Returns number of whole seconds that tick count represents */
int calc_time(int ticks){
    return ticks/18;
}