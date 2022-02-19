#pragma once

#include "multiboot.h"

#include "../lib/typedefs.h"
#include "../gdt/gdt.h"
#include "../lib/screen.h"
#include "../lib/debug.h"
#include "../lib/math.h"
#include "../interrupt/idt.h"
#include "../memory/paging.h"
#include "../processes/process.h"
#include "../shell/shell.h"
#include "../processes/mq.h"
#include "../tests/mq_test.h"


void kernel_entry(uint32_t, uint32_t addr);
void main();
void spinning_bars();
bool setup(uint32_t magic, uint32_t addr);
bool validate_memory(uint32_t addr);
void test_func();
void draw_PUNCHOS();
void spin(int position);