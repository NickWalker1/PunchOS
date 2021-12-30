#pragma once
#include "typedefs.h"

void *memset(void* str, int c, size_t n);
void *memcpy(void* dest, void* source, size_t n);
char *strcpy(char* dest, char* src); 
bool  strcmp(char* a, char* b); /* NOT IMPLEMENTED */