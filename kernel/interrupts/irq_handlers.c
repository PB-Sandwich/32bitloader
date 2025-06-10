#include "irq_handlers.h"
#include "../inboutb.h"
#include "../pic.h"
#include "../print.h"

// void PIC_sendEOI(uint8_t irq)
// {
// 	if(irq >= 8)
// 		outb(PIC2_COMMAND,PIC_EOI);
//
// 	outb(PIC1_COMMAND,PIC_EOI);
// }

void irq1_keyboard(struct interrupt_frame* frame)
{
    int key = inb(0x60);
    printf("keyboard pressed: %x\n", key);
    printf("eip: %x\n", frame->eip);
    printf("cs: %x\n", frame->cs);
    printf("eflags: %x\n", frame->eflags);
    printf("esp: %x\n", frame->esp);
    printf("ss: %x\n", frame->ss);
    printf("es: %x\n", frame->es);
    printf("ds: %x\n", frame->ds);
    printf("fs: %x\n", frame->fs);
    printf("gs: %x\n", frame->gs);
    outb(PIC1_CMD, PIC_EOI);
}

void irq7_15_spurious(struct interrupt_frame* frame)
{
    return;
}
