#include "string.h"

extern uint32_t helper_variable;
/* sets n bytes at str to c */
void *memset(void* str, int c, size_t n){
    char *ptr = (char*)str;
    size_t i;
    for(i=0;i<n;i++){
        *ptr=c;
        ptr++;
    }
    return str;
}

/* Copies n bytes from src to dest. */
void *memcpy(void* dest, void* src, size_t n){ 
    char *d=dest;
    char *s=src;
    //TODO improve performance with using larger blocks than char eg long if allignment matches
    for(size_t i=0;i<n;i++){
        d[i]=s[i];
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

/* Returns the length of the string up to STR_MAX_LEN. len = num chars + 1*/
int strlen(char* str){
    size_t i=0;
    while(str[i]!=0 && i<STR_MAX_LEN){
        i++;
    }
    return i;
}

/* Returns 0 on equal strings */
int strcmp(char *a, char *b){
    int i=0;
    while(a[i]==b[i]){
        if(a[i]==0 && b[i]==0) return 0;
        i++;
    }
    
    return a[i]-b[i];
}

/* Returns 0 on equal strings n chars in */
int strncmp(char *a, char *b, size_t n){
    int i=0;
    while(a[i]==b[i]){
        if(i==n || ( a[i]==0 && b[i]==0 )) return 0;
        i++;
    }
    
    return a[i]-b[i];
}