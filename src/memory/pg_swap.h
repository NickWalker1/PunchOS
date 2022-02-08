#pragma once

#include "../lib/typedefs.h"
#include "../lib/debug.h"

#include "../interrupt/cpu_state.h"



void page_fault_handler(exception_state *state);