#include "irq_handlers.h"
#include <inboutb.h>
#include <pic.h>
#include <print.h>
#include <stdint.h>
#include <keyboard/keyboard.h>

// void PIC_sendEOI(uint8_t irq)
// {
// 	if(irq >= 8)
// 		outb(PIC2_COMMAND,PIC_EOI);
//
// 	outb(PIC1_COMMAND,PIC_EOI);
// }

void irq1_keyboard(struct interrupt_frame* frame)
{
    uint8_t scancode = inb(0x60);

    on_event(scancode);

    outb(PIC1_CMD, PIC_EOI);
    return;
}

void irq7_15_spurious(struct interrupt_frame* frame)
{
    outb(PIC1_CMD, PIC_EOI);
    outb(PIC2_CMD, PIC_EOI);
    return;
}
