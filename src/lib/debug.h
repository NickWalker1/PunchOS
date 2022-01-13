#pragma once

#include "screen.h"
#include "typedefs.h"
#include "../interrupt/cpu_state.h"
#include "../interrupt/handlers.h"

extern uint32_t helper_variable;


void ASSERT(bool cond, char *msg);
void PANIC(char* msg);
void PANIC_EXC(char* msg, exception_state* state);

void breakpoint();

void draw_panic_screen();
void halt();

