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

; 3 Pages beneath 1MiB mark where kernel is loaded

PD_BASE equ 0x00FFD000
PT_I_BASE equ 0x00FFE000
PT_K_BASE equ 0x00FFF000

PG_SIZE equ 0x1000

KERNEL_BASE equ 0x00100000 ; 1MiB address
KERNEL_VIRT_BASE equ 0xC0000000 ; 3GiB address

; 1 4k page table is able to cover 4MiB of virtual addresses 
; as 1024 entries each pointing to a different 4k page.

clear_tables:
    pusha
    mov eax,0

    .clear_loop:
    mov [PD_BASE+eax], byte 0
    inc eax

    cmp eax, PG_SIZE * 3
    jl .clear_loop
    
    popa
    ret

create_page_directory:
    pusha 
    ;add the identity table entry
    mov eax, PT_I_BASE
    or dword eax, 3
    mov [PD_BASE], eax

    ;add kernel mapping
    mov eax, PT_K_BASE
    or dword eax, 3
    
    ;get PT_K_BASE offset in ebx
    mov ebx, KERNEL_VIRT_BASE
    shr ebx, 22

    mov [PD_BASE+ ebx*4], eax
    popa
    ret

create_identity_page_table:
    pusha
    ; index
    mov eax, 0

    ; phys address start
    mov ebx, 0

    .indentity_loop
    mov ecx, ebx
    or dword ecx, 3

    ;move ecx into that memory location
    mov [PT_I_BASE+eax*4], ecx

    add ebx, 0x1000 ;point to next physical page
    inc eax

    cmp eax, 0x400 ; 1024 entries
    jl .indentity_loop

    popa
    ret

create_kernel_page_table:
    pusha
    ; index
    mov eax, 0

    ; phys address
    mov ebx, 0

    .kernel_loop
    mov ecx, ebx
    or dword ecx, 3

    mov [PT_K_BASE+eax*4], ecx

    add ebx, 0x1000
    inc eax

    cmp eax, 0x400
    jl .kernel_loop

    popa
    ret

enable_paging:
    pusha
    ; Set address of the directory table
    mov eax, PD_BASE
    mov cr3, eax

    ; Enable paging
    mov eax, cr0
    or eax, 0x80000020
    mov cr0, eax

    jmp .branch
    nop
    nop
    nop
    nop
    nop
    .branch:

    popa
    ret


clear_identity_map:
    pusha
    mov eax, 0

    .identity_clear_loop:
    mov [PD_BASE+eax], byte 0
    inc eax

    cmp eax, 4
    jl .identity_clear_loop

    popa
    ret

global _start:function (_start.end - _start)
_start:

	mov esp, stack_top - KERNEL_VIRT_BASE
	
    ; clear EFLAGS
    push 0
    popf
     
    finit

    cli

    ; Pushing items to the stack is the way a function is provided arguments
    ; As defined in Multiboot2 specification can push these values to the stack from eax,ebx.
    ; Push multiboot2 header pointer
    push ebx

    ; Push multiboot2 magic value
    push eax

	lgdt [gdt_descriptor]

    call clear_tables

    call create_page_directory
    
    call create_identity_page_table

    call create_kernel_page_table

    call enable_paging

    ; Update the stack pointer to be in Kernel Virtual address range
    mov eax, esp
    add eax, KERNEL_VIRT_BASE
    mov esp, eax


    ; Jump to addresses based in kernel range (0xC0000000+)
	jmp higher
.end:


section .text
higher:

    ; Jump to kernel main function in C.     
    extern kernel_main
    call kernel_main

    ; Incase kernel main ever returns, just loop.
	cli
.hang:	hlt
	jmp .hang