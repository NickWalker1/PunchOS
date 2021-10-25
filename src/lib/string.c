#include "string.h"

/* sets n bytes at str to c */
void* memset(void* str, int c, size_t n){
    uint32_t* ptr = (uint32_t*)str;
    for(size_t i=0;i<n;i++){
        *ptr=c;
        ptr++;
    }
    return str;
}

/* copies n bytes from src to dest. */
void* memcpy(void* dest, void* src, size_t n){
    for(size_t i=0;i<n;i++){
        *(uint32_t*)(dest+i)=*(uint32_t*)(src+i);
    }
    return dest;
}