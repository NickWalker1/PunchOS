#pragma once

#include "multiboot.h"

#include "../lib/typedefs.h"
#include "../gdt/gdt.h"
#include "../lib/screen.h"
#include "../lib/debug.h"
#include "../interrupt/idt.h"
#include "../memory/paging.h"
#include "../processes/process.h"
#include "../shell/shell.h"
#include "../threads/thread.h"
#include "../tests/sched_test.h"


void kernel_entry(uint32_t, uint32_t addr);
void main();
bool setup(uint32_t magic, uint32_t addr);
bool validate_memory(uint32_t addr);
void test_func();
void spin(int position);
void scheduling_test();
void draw_logo(bool reset_screen);