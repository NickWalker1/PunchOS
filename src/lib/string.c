#include "string.h"

/* sets n bytes at str to c */
void *memset(void* str, int c, size_t n){
    uint32_t* ptr = (uint32_t*)str;
    size_t i;
    for(i=0;i<n;i++){
        *ptr=c;
        ptr++;
    }
    return str;
}

/* copies n bytes from src to dest. */
void *memcpy(void* dest, void* src, size_t n){
    for(size_t i=0;i<n;i++){
        *(uint32_t*)(dest+i)=*(uint32_t*)(src+i);
    }
    return dest;
}
/*Copies the C string pointed by source into the array pointed by destination, 
 * including the terminating null character (and stopping at that point).
 * This can overflow as no check for size of dest space.
 * Will run indefinitely if src not NULL terminated.*/
char *strcpy(char *dest, char *src){
    int i=0;
    while(src[i]!=0){
        dest[i]=src[i];
        i++;
    }
    return dest;
}