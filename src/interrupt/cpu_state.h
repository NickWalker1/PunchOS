#pragma once

#include "../lib/typedefs.h"
#include "../lib/screen.h"


typedef struct fpu_state
{
    uint32_t control_word;
    uint32_t status_word;
    uint32_t tag_word;
    uint32_t instruction_pointer_offset;
    uint16_t instruction_pointer_selector;
    uint16_t opcode;
    uint32_t operand_pointer_offset;
    uint32_t operand_pointer_selector;
    uint8_t registers[80];
} fpu_state;

typedef struct registers_state
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_unused;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} registers_state;

/* worth noting that both exception_state and interrupt_state 
 * both include ss and esp at the end and these values
 * will only be filled if the interrupt occurs when in 
 * userspace. Which is determined by where in gdt or something?
 */
typedef struct exception_state
{
    uint64_t idtr;
    uint64_t gdtr;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t cr4;
    uint32_t cr3;
    uint32_t cr2;
    uint32_t cr0;
    uint32_t ds;

    registers_state registers;
    fpu_state fpu_state;

    uint32_t interrupt_number;

    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;

    //These are only present when pushed from usermode (not currently setup)
    uint32_t esp;
    uint32_t ss;
} exception_state;

typedef struct interrupt_state
{
    registers_state registers;
    fpu_state fpu_state;

    uint32_t interrupt_number;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
} interrupt_state;


void exception_state_dump(exception_state* state);
void interrupt_state_dump(interrupt_state* state);