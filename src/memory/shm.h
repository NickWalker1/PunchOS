#pragma once

#include "heap.h"

#include "../lib/list.h"
#include "../lib/typedefs.h"
#include "../lib/string.h"

#define O_CREATE  1<<0
#define O_OPEN    1<<1
#define O_RD      1<<2
#define O_RDWR    1<<3


typedef struct shared_block{
    char name[12]; /* Block name, must be unique */
    void *base; /* Base physical address */
    void *rw_hdr_off; /* Read write header offset */
} shared_block_t;