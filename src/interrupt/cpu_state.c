#include "cpu_state.h"

void exception_state_dump(exception_state *state){
    char str[128];
    println("---CORE DUMP---");
    println("IDTR:");
    print(itoa(state->idtr,str,BASE_HEX));
    println("GDTR:");
    print(itoa(state->gdtr,str,BASE_HEX));
    println("GS:");
    print(itoa(state->gs,str,BASE_HEX));
    println("FS:");
    print(itoa(state->fs,str,BASE_HEX));
    println("ES:");
    print(itoa(state->es,str,BASE_HEX));
    println("CR4:");
    print(itoa(state->cr4,str,BASE_BIN));
    println("CR3:");
    print(itoa(state->cr3,str,BASE_HEX));
    println("CR2:");
    print(itoa(state->cr2,str,BASE_HEX));
    println("CR0:");
    print(itoa(state->cr0,str,BASE_BIN));
    println("DS:");
    print(itoa(state->ds,str,BASE_HEX));
    println("INT_NUM:");
    print(itoa(state->interrupt_number,str,BASE_DEC));
    println("ERROR_CODE:");
    print(itoa(state->error_code,str,BASE_BIN));
    println("EIP:");
    print(itoa(state->eip,str,BASE_HEX));
    println("CS:");
    print(itoa(state->cs,str,BASE_HEX));
    println("EFLAGS:");
    print(itoa(state->eflags,str,BASE_HEX));
    println("IGNORE ESP/SS if in kernel mode");
    println("ESP:");
    print(itoa(state->esp,str,BASE_HEX));
    println("SS:");
    print(itoa(state->ss,str,BASE_HEX));

    halt();
}

void interrupt_state_dump(interrupt_state* state){
    println("INTERRUPT CORE DUMP");
    println("INT_NUM:");
    print(itoa(state->interrupt_number,str,BASE_DEC));
    println("EIP:");
    print(itoa(state->eip,str,BASE_HEX));
    println("CS:");
    print(itoa(state->cs,str,BASE_HEX));
    println("EFLAGS:");
    print(itoa(state->eflags,str,BASE_BIN));
    println("ESP:");
    print(itoa(state->esp,str,BASE_HEX));
    println("SS:");
    print(itoa(state->ss,str,BASE_HEX));
}
