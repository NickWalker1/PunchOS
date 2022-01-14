#pragma once

#include "../lib/typedefs.h"

#define O_CREATE    1<<0
#define O_OPEN      1<<1

typedef struct shared_desc{
    char name[12];
    void *paddr;
} shared_desc_t;

void shm_init();
shared_desc_t *shm_contains(char *name);
shared_desc_t *shm_open(char *name, uint8_t flags);


void *write(void *,void*,size_t);
void *read(void *,void*,size_t);

void shm_A();
void shm_B();