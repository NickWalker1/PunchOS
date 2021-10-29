#pragma once
#include "../lib/typedefs.h"

#define GDT_SIZE 6

typedef struct gdt_info
{
    uint16_t size;
    uint32_t offset;
} __attribute__((packed)) gdt_info;

typedef struct gdt_entry
{
    uint16_t limit_0_15;
    uint16_t base_0_15;
    uint8_t base_16_23;

    uint8_t accessed : 1;
    uint8_t read_write : 1;
    uint8_t direction : 1;
    uint8_t executable : 1;
    uint8_t reserved_1 : 1;
    uint8_t privilege_level : 2;
    uint8_t present : 1;

    uint8_t limit_16_19 : 4;
    uint8_t reserved_2 : 2;
    uint8_t size : 1;
    uint8_t granularity : 1;

    uint8_t base_24_31;

} __attribute__((packed)) gdt_entry;

void gdt_init();
void gdt_fill_entry(int index, bool executable, uint8_t privilege_level);
void gdt_add_tss(int index, bool executable, uint8_t privilege_level);