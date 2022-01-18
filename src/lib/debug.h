#pragma once

#include "screen.h"
#include "typedefs.h"
#include "../interrupt/cpu_state.h"
#include "../interrupt/handlers.h"

extern uint32_t helper_variable;

#define WARN_CHAR_MAX 18
#define WARN_LINE_MAX 22


void ASSERT(bool cond, char *msg);
void PANIC(char* msg);
void PANIC_EXC(char* msg, exception_state* state);

void WARN_DUMP();
void KERN_WARN(char *msg);

void breakpoint();

void draw_panic_screen();
void halt();

