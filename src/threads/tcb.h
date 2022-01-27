#pragma once


#include "../lib/typedefs.h"
#include "../lib/debug.h"
#include "../lib/int.h"
#include "../lib/screen.h"
#include "../lib/string.h"

#define THR_MAGIC 0x18273645

typedef int32_t t_id;
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

typedef struct thread_diagnostics{
    bool present;

    TCB_t *thread;

    uint32_t mem_usage;

    /* Timing information */
    uint32_t running_ticks; /* Count of ticks when has been running process */
    uint32_t wait_ticks; /* Current wait time before rescheduling */
    uint32_t average_latency; /* Average number of ticks between being scheduled and being run */
    uint32_t scheduled_count; /* Number of times this process has been scheduled */

} thread_diagnostics_t;


void *get_esp();
uint32_t *get_base_page(uint32_t *addr);
bool is_thread(TCB_t *t);
TCB_t *current_thread();
void thread_dump();
