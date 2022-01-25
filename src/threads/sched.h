#pragma once

#include "tcb.h"

bool scheduling_init();

void thread_reschedule(TCB_t *t);
TCB_t *get_next_thread();