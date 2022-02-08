#pragma once
#include "typedefs.h"

#define STR_MAX_LEN 128

void *memset(void* str, int c, size_t n);
void *memcpy(void* dest, void* source, size_t n);
char *strcpy(char* dest, char* src); 
int strlen(char* str);
int strcmp(char* a, char* b); 
char *strcat(char *a, char *b);