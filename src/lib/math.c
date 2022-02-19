#include "math.h"

#include "debug.h"

uint32_t rseed =0;

/* Warning cnnot compute negative powers */
int pow(int base,int exp){
    if(exp<0){
        KERN_WARN("Negative exponent in pow");
        return 0;
    } 

    int result=1;
    for(int i=0;i<exp;i++)
        result*=base;
    return result;
}

#define RAND_MAX_32 ((1U << 31) - 1)

void set_rseed(int x){
    rseed=x;
}

/* Returns a random 32 bit unsigned integer between 0 and 2^15-1 */
uint32_t rand(){
    return (rseed = (rseed * 214013 + 2531011) & RAND_MAX_32) >> 16;
}