#pragma once

#include "multiboot.h"

#include "../lib/typedefs.h"
#include "../lib/screen.h"

void kernel_main(uint32_t, uint32_t addr);
bool validMemory(uint32_t addr);