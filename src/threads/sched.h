#pragma once

#include "tcb.h"

#define PRIO_MIN 2
#define PRIO_MAX 0

bool scheduling_init();

void thread_reschedule(TCB_t *t);
TCB_t *get_next_thread();
TCB_t *peek_next_thread();
bool deschedule(TCB_t *t);
void queue_dump();