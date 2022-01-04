; Offsets inside the stack
curr_offset equ 20
next_offset equ 24


global context_switch
; PCB_t *context_switch(PCB_t *cur, PCB_t *next);
context_switch:
    ; Save old callee-preserved registers
    ; No need to store eax-edx as they are handled 
    ; by the interrupt  handler
    push  ebp
    push  ebx
    push  esi
    push  edi


    ; Store the current stack pointer in the PCB struct    
    mov edx, 0 ; currently just have stack* at offset 0 in struct but this can change
    mov eax, [curr_offset+esp]
    mov [eax+edx*1],esp


    ; Load the stack pointer of the next Thread struct
    mov ecx, [next_offset+esp]
    mov esp, [ecx+edx*1]

    ; Load new callee-preserved registers
    pop  edi
    pop  esi
    pop  ebx
    pop  ebp

    ret


; void first_switch()
global first_switch
first_switch: 
    add esp, 8 ; get rid of switch process arguments


    push eax    ; push returned value from previous function which was
                ; context_switch which returns the previous process.

    extern switch_complete 
    call switch_complete 

    add esp,4   ; skip the push just done to be in line for the eip value
                ; for ret
    


    ;start process properly
    ret


