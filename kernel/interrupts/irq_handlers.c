#include "irq_handlers.h"
#include <inboutb.h>
#include <keyboard/keyboard.h>
#include <memutils.h>
#include <pic.h>
#include <print.h>
#include <process.h>
#include <stdint.h>
#include <time.h>
#include <x86_64_structures.h>

// void PIC_sendEOI(uint8_t irq)
// {
// 	if(irq >= 8)
// 		outb(PIC2_COMMAND,PIC_EOI);
//
// 	outb(PIC1_COMMAND,PIC_EOI);
// }

struct time time = { 0 };

struct time get_time()
{
    return time;
}

void irq0_timer_c(struct registers* regs, struct interrupt_frame* frame)
{
    time.millisecond += 10;
    if (time.millisecond > 1000) {
        time.seconds += 1;
        time.millisecond -= 1000;
    }

    struct process* current = get_current_process();
    uint32_t esp = current->process_data.frame.esp;
    current->process_data.frame = *frame;
    current->process_data.frame.esp = esp;
    current->process_data.registers = *regs;

    struct process* next = get_next_process();
    uint32_t* stack = (uint32_t*)next->process_data.frame.esp;
    stack--;
    *stack = next->process_data.frame.eflags;
    stack--;
    *stack = next->process_data.frame.cs;
    stack--;
    *stack = next->process_data.frame.eip;
    next->process_data.frame.esp = (uint32_t)stack;
    *frame = next->process_data.frame;
    *regs = next->process_data.registers;

    outb(PIC1_CMD, PIC_EOI);
    return;
}

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
