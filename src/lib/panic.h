#pragma once

#include "screen.h"
#include "typedefs.h"
#include "../interrupt/cpu_state.h"
#include "../interrupt/handlers.h"

void PANIC(char* msg);
void PANIC_EXC(char* msg, exception_state* state);

void draw_panic_screen();
void halt();