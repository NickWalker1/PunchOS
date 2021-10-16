[bits 32]

; NOTE: This will always print in the top left and just overwrite
; whatever else was there before. Can be fixed by using variable
; video memory variable but cba.

; Defining useful constants
VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

printString32:
    pusha
    mov edx, VIDEO_MEMORY

print_string_loop:
    mov al, [ebx]
    mov ah, WHITE_ON_BLACK
    
    cmp al,0        ; check for null terminating
    je print_string_done

    mov [edx], ax   ; store the character and its attributes
                    ; at the current character cell

    inc ebx         ; Incremement to the next char in the string
    add edx,2       ; Move to the next cell in video memory 

    jmp print_string_loop

print_string_done:
    popa
    ret
   
