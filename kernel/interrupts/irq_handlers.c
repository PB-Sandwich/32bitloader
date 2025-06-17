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

volatile uint8_t key_pressed_var = 0;
volatile uint8_t scancode_var = 0;
volatile void (*keyboard_function)(uint8_t scancode) = 0;

void clear_key_pressed()
{
    key_pressed_var = 0;
}
uint8_t key_pressed()
{
    return key_pressed_var;
}
uint8_t scancode()
{
    return scancode_var;
}
void set_keyboard_function(void (*keyboard_function_)(uint8_t scancode))
{
    keyboard_function = (volatile void*)keyboard_function_;
    return;
}

void irq1_keyboard(struct interrupt_frame* frame)
{
    scancode_var = inb(0x60);
    key_pressed_var = 1;
    if (keyboard_function != 0) {
        keyboard_function(scancode_var);
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
