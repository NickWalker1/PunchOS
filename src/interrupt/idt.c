#include "idt.h"

static bool in_ext_int;

bool block_PIT=1;

__attribute__((aligned(0x10)))
static idt_entry idt[256];

static idtr_t idtr;

void idt_set_descriptor(uint8_t vector, uint32_t (*handler)(interrupt_state *state), bool user_interrupt){
    idt_entry* descriptor = &idt[vector];
    uint32_t handler_addr = (uint32_t)handler;

    descriptor->isr_low         =handler_addr & 0xFFFF;
    descriptor->kernel_cs       =8; //change this to where kernel offset is if I set that up later or something
    descriptor->isr_high        =(uint32_t)handler_addr>>16;
    descriptor->reserved        =0;
    descriptor->present         =1;
    descriptor->privilege_level = user_interrupt ? 3 : 1;
    descriptor->type            = 15; //32bit Trap, change to enum struct thingy later
}

void idt_init(){
    idtr.base=(uint32_t)&idt;
    idtr.limit=(uint16_t)sizeof(idt_entry) * IDT_MAX_DESCRIPTORS -1;

    // Exceptions
    idt_set_descriptor(0, idt_exc0, false);
    idt_set_descriptor(1, idt_exc1, false);
    idt_set_descriptor(2, idt_exc2, false);
    idt_set_descriptor(3, idt_exc3, false);
    idt_set_descriptor(4, idt_exc4, false);
    idt_set_descriptor(5, idt_exc5, false);
    idt_set_descriptor(6, idt_exc6, false);
    idt_set_descriptor(7, idt_exc7, false);
    idt_set_descriptor(8, idt_exc8, false);
    idt_set_descriptor(9, idt_exc9, false);
    idt_set_descriptor(10, idt_exc10, false);
    idt_set_descriptor(11, idt_exc11, false);
    idt_set_descriptor(12, idt_exc12, false);
    idt_set_descriptor(13, idt_exc13, false);
    idt_set_descriptor(14, idt_exc14, false);
    idt_set_descriptor(15, idt_exc15, false);
    idt_set_descriptor(16, idt_exc16, false);
    idt_set_descriptor(17, idt_exc17, false);
    idt_set_descriptor(18, idt_exc18, false);
    idt_set_descriptor(19, idt_exc19, false);
    idt_set_descriptor(20, idt_exc20, false);
    idt_set_descriptor(21, idt_exc21, false);
    idt_set_descriptor(22, idt_exc22, false);
    idt_set_descriptor(23, idt_exc23, false);
    idt_set_descriptor(24, idt_exc24, false);
    idt_set_descriptor(25, idt_exc25, false);
    idt_set_descriptor(26, idt_exc26, false);
    idt_set_descriptor(27, idt_exc27, false);
    idt_set_descriptor(28, idt_exc28, false);
    idt_set_descriptor(29, idt_exc29, false);
    idt_set_descriptor(30, idt_exc30, false);
    idt_set_descriptor(31, idt_exc31, false);

    // Hardware interrupts (IRQ)
    idt_set_descriptor(32, idt_int32, false); // Programmable Interrupt Timer
    idt_set_descriptor(33, idt_int33, false); // Keyboard
    idt_set_descriptor(34, idt_int34, false); // Cascade
    idt_set_descriptor(35, idt_int35, false); // COM2
    idt_set_descriptor(36, idt_int36, false); // COM1
    idt_set_descriptor(37, idt_int37, false); // LPT2
    idt_set_descriptor(38, idt_int38, false); // Floppy
    idt_set_descriptor(39, idt_int39, false); // LPT1
    idt_set_descriptor(40, idt_int40, false); // CMOS
    idt_set_descriptor(41, idt_int41, false); // Free
    idt_set_descriptor(42, idt_int42, false); // Free
    idt_set_descriptor(43, idt_int43, false); // Free
    idt_set_descriptor(44, idt_int44, false); // Mouse
    idt_set_descriptor(45, idt_int45, false); // FPU
    idt_set_descriptor(46, idt_int46, false); // Primary ATA Hard Disk
    idt_set_descriptor(47, idt_int47, false); // Secondary ATA Hard Disk

    // Software interrupts (AKA: syscall)
    idt_set_descriptor(50, idt_int50, true);

    //Otherwise pic interrupt will be erronously recorded as a doublefault.
    irq_remap();

    in_ext_int=false;

    __asm__ volatile("lidt %0" : : "memory"(idtr)); //load the new IDT
    __asm__ volatile("sti"); //Set interrupt flag

}


/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
*  is a problem in protected mode, because IDT entry 8 is a
*  Double Fault! Without remapping, every time IRQ0 fires,
*  you get a Double Fault Exception, which is NOT actually
*  what's happening. We send commands to the Programmable
*  Interrupt Controller (PICs - also called the 8259's) in
*  order to make IRQ0 to 15 be remapped to IDT entries 32 to
*  47 */
void irq_remap(void)
{
    outportb(PIC1_COMMAND, 0x11);
    outportb(PIC2_COMMAND, 0x11);
    outportb(PIC1_DATA, 0x20);
    outportb(PIC2_DATA, 0x28);
    outportb(PIC1_DATA, 0x04);
    outportb(PIC2_DATA, 0x02);
    outportb(PIC1_DATA, 0x01);
    outportb(PIC2_DATA, 0x01);
    outportb(PIC1_DATA, 0x0);
    outportb(PIC2_DATA, 0x0);
}


void int_set(int level){
    if(level)int_enable();
}

int int_get_level(){
    uint32_t flags;

    __asm__ volatile("pushfl; pop %0" : "=g" (flags));
    return flags & 0x200 ? 1 : 0;
}

/* enables interrupts and returns previous status */
int int_enable(){
    int level = int_get_level();
    __asm__ volatile("sti");
    return level;
}

/* disables interrupts and returns previous status */
int int_disable(){
    int level = int_get_level();
    __asm__ volatile("cli");
    return level;
}

void idt_global_int_wrapper(interrupt_state *state){

    in_ext_int=true;

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (state->interrupt_number >= 40)
    {
        outportb(PIC2_COMMAND, PIC_EOI);
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outportb(PIC1_COMMAND, PIC_EOI);
    

    if(state->interrupt_number==32 && !block_PIT) thread_tick();

    in_ext_int=false;
}

void idt_global_exc_wrapper(exception_state *state){
    default_exception_handler(state);
}

uint32_t ticks=0; 
void timer_tick(){
    ticks++;
    if(ticks%18==0) println("1 second");
}

/* Returns true if currently in an external interrupt handler */
bool in_external_int(){
    return in_ext_int;
}
