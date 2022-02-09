#pragma once



#include "../lib/typedefs.h"
#include "../lib/debug.h"

#include "../interrupt/cpu_state.h"



#define virt_RAM_size 64
#define virt_HDD_size 256




void page_fault_handler(exception_state *state);