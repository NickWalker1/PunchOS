#include "math.h"

#include "debug.h"

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