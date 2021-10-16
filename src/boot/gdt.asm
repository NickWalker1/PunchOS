; GDT
; For simplictity I will define two segments, one data and
; one code, they will overlap and there will be no memory protection
; because I'm cool like that. 
; Byte reference table page 34 of bham pdf

gdt_start:

gdt_null:           ; Mandatory null descriptor
    dd 0x0
    dd 0x0          ; all 8 bytes 0s

gdt_code:           ; the code segment descriptor
    ; base= 0x0, limit=0xffff
    ; 1st  flags: (present )1 (privilege )00 (descriptor  type)1 -> 1001b
    ; type  flags: (code)1 (conforming )0 (readable )1 (accessed )0 -> 1010b
    ; 2nd  flags: (granularity )1 (32-bit  default )1 (64-bit  seg)0 (AVL)0 -> 1100b
    dw 0xffff       ; Limit (bits 0-15)
    dw 0x0          ; base
    db 0x0          ; base
    db 10011010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, Limit (bits 16-19)
    db 0x0          ; Base (bits 24-31)

gdt_data:
    ; Same as code  segment  except  for  the  type  flags:
    ; type  flags: (code)0 (expand  down)0 (writable )1 (accessed )0 -> 0010b
    dw 0xffff       ; Limit (bits  0-15)
    dw 0x0          ; Base (bits 0-15)
    db 0x0          ; Base (bits 16-23)
    db 10010010b    ; 1st flags, type flags
    db 11001111b    ; 2nd flags, Limit (bits 16-19)
    db 0x0          ; Base (bits 24-31)

gdt_end:


gdt_descriptor:
    dw gdt_end-gdt_start-1

    dd gdt_start

CODE_SEQ equ gdt_code - gdt_start
DATA_SEQ equ gdt_data - gdt_start
