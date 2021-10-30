#pragma once

#include "multiboot.h"

#include "../lib/typedefs.h"
#include "../gdt/gdt.h"
#include "../lib/screen.h"
#include "../lib/panic.h"
#include "../interrupt/idt.h"
#include "../paging/paging.h"


int kernel_main(uint32_t, uint32_t addr);
bool setup(uint32_t magic, uint32_t addr);
bool validMemory(uint32_t addr);