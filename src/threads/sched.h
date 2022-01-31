#pragma once

#include "tcb.h"

#define PRIO_MIN 2
#define PRIO_MAX 0

bool scheduling_init();

void thread_reschedule(TCB_t *t);
TCB_t *get_next_thread();