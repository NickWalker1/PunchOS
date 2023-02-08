#include "screen.h"
#include "va_args.h"


#include "printf_nick.h"


int va_test(int count,...){
    va_list args;

    va_start(args, count);

    for(int i=0;i<count;i++){
        println(itoa(va_arg(args,int),str,BASE_DEC));
    }
    
    
}

int printf(char *fmt, ...){
    return 0;
}
