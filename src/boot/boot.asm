; Declare constants for the multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; checksum of above, to prove we are multiboot
 

section .multiboot.data
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM
 

section .bss
align 16
stack_bottom:
resb 4096 ; 16 KiB
stack_top:


section .multiboot.text
%include "src/boot/gdt.asm"

global _start:function (_start.end - _start)
_start:

	mov esp, stack_top 
	
    ; Reset EFLAGS 
    push 0
    popf

    ; Pushing items to the stack is the way a function is provided arguments
    ; As defined in Multiboot2 specification can push these values to the stack from eax,ebx.
    ; Push multiboot2 header pointer
    push ebx

    ; Push multiboot2 magic value
    push eax

	lgdt [gdt_descriptor]

    jmp higher
.end:


section .text
higher:

    ;jump to kernel code 
    extern kernel_main
    call kernel_main

	cli
.hang:	hlt
	jmp .hang