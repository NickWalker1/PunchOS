#include "math.h"

/* Warning cnnot compute negative powers */
int pow(int base,int exp){
    if(exp<0) return 0;
    int result=1;
    for(int i=0;i<exp;i++)
        result*=base;
    return result;