#pragma once

#include "multiboot.h"

#include "../lib/typedefs.h"
#include "../gdt/gdt.h"
#include "../lib/screen.h"
#include "../lib/debug.h"
#include "../interrupt/idt.h"
#include "../paging/paging.h"
#include "../processes/process.h"


int kernel_main(uint32_t, uint32_t addr);
bool setup(uint32_t magic, uint32_t addr);
bool validate_memory(uint32_t addr);
void test_func();
void spin(int position);