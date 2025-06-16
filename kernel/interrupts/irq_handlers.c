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
    scancode = inb(0x60);
    key_pressed = 1;
    if (keyboard_function != 0) {
        keyboard_function(scancode);
    }
    outb(PIC1_CMD, PIC_EOI);
    return;
}

void irq7_15_spurious(struct interrupt_frame* frame)
{
    outb(PIC1_CMD, PIC_EOI);
    outb(PIC2_CMD, PIC_EOI);
    return;
}
