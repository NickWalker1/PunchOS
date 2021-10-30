#pragma once

#include "multiboot.h"

#include "../lib/typedefs.h"
#include "../gdt/gdt.h"
#include "../lib/screen.h"
#include "../lib/panic.h"
#include "../interrupt/idt.h"


extern uint32_t _KERNEL_END_;

int kernel_main(uint32_t, uint32_t addr);
bool setup(uint32_t magic, uint32_t addr);
bool validMemory(uint32_t addr);