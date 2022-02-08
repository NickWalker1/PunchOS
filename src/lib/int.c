#include "typedefs.h"

/* Reverses a given string of known length */
void reverse(char* str, int length){
    int start=0;
    int end=length-1;
    char tmp;
    while(start<end){
        tmp=str[start];
        str[start]=str[end];
        str[end]=tmp;
        start++;
        end--;
    }
}

/* Writes the ascii representation of num with base to str buffer*/
char* itoa(int num, char* str, int base){
    int i=0;
    bool isNegative=false;

    if(num==0){
        str[i++]='0';
        str[i]='\0';
        return str;
    }

    //if negative and base 10, assume signed
    if(num<0 && base==10)
    {
        isNegative= true;
        num=-num;
        while(num!=0){
            int rem=num%base;
            str[i++]= (rem>9)? (rem-10)+'a' : rem+ '0';
            num=num/base;
        }
    }
    //otherwise use unsigned.
    else{
        uint32_t unum=num;
        while(unum!=0){
            int rem=unum%base;
            str[i++]= (rem>9)? (rem-10)+'a' : rem+ '0';
            unum=unum/base;
        }
    }

    if(isNegative){
        str[i++]='-';
    }
    
    if(base==BASE_HEX){
        str[i++]='x';
        str[i++]='0';
    }
    if(base==BASE_BIN){
        str[i++]='b';
        str[i++]='0';
    }
    
    str[i]='\0';

    reverse(str,i);

    return str;
}
    
