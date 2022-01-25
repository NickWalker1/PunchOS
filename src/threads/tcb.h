#pragma once


#include "../lib/typedefs.h"
#include "../lib/debug.h"
#include "../lib/int.h"
#include "../lib/screen.h"
#include "../lib/string.h"

#define THR_MAGIC 0x18273645

typedef uint32_t t_id;
typedef struct TCB TCB_t;

/* Thread Control Block 
        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
             |              stack*             |
        0 kB +---------------------------------+

*/

typedef enum thread_status
{
    T_READY,
    T_RUNNING,
    T_BLOCKED,
    T_ZOMBIE,
    T_DYING
} thread_status;


/* Process Control Block Struct */
struct  TCB{
    void *stack; /* DO NOT MOVE */

    t_id tid; /* Unique thread ID */

    uint32_t owner_pid; /* Owning Process ID */

    char name[16]; 

    thread_status status; 

    int priority; /* To be implemented */


    /* Magic value to check for stack growth induced corruption and process validation */
    uint32_t magic;
};


void *get_esp();
uint32_t *get_base_page(uint32_t *addr);
bool is_thread(TCB_t *t);
TCB_t *current_thread();
void thread_dump();
