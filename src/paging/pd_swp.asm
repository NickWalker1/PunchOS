[global update_pd]

; void update_pd(uint32_t* pd)
update_pd:
    ; Get the pd argument and move into eax
    mov eax, [esp+4]

    ; Move the pointer into CR3 to be used as the pd.
    mov cr3, eax

    ; Force TLB flush
    ;invlpg [0]

    ret