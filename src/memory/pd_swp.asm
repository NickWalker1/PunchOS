
; void update_pd(uint32_t* pd)
[global update_pd]
update_pd:
    ; Get the pd argument and move into eax
    mov eax, [esp+4]

    ; Move the pointer into CR3 to be used as the pd.
    mov cr3, eax

    ret

[global tlb_flush]
tlb_flush:
    ; As using i386 architecture unable to use
    ; invlpg to invalidate tlb entry hence must force
    ; a full tlb flush by moving cr3 in and out.

    mov eax,cr3
    mov cr3,eax

    ret