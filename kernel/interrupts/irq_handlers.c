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
    outb(PIC1_CMD, PIC_EOI);
    return;
}

__attribute__((interrupt)) void irq6_floppy(struct interrupt_frame* frame)
{
    printf("floppy\n");
    outb(PIC1_CMD, PIC_EOI);
    return;
}

void irq7_15_spurious(struct interrupt_frame* frame)
{
    outb(PIC1_CMD, PIC_EOI);
    outb(PIC2_CMD, PIC_EOI);
    return;
}
