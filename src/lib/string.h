#pragma once
#include "typedefs.h"

void* memset(void* str, int c, size_t n);
void* memcpy(void* dest, void* source, size_t n);
void* strcpy(void* dest, void* source); /* NOT IMPLEMENTED */
bool  strcmp(void* a, void* b); /* NOT IMPLEMENTED */