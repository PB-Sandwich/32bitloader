#include "irq_handlers.h"
#include "filesystem/virtual-filesystem.h"
#include "pager.h"
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

volatile struct time time = { 0 };

uint32_t irq0_timer_c(uint32_t* esp)
{
    time.millisecond += 10;
    if (time.millisecond > 1000) {
        time.seconds += 1;
        time.millisecond -= 1000;
    }

    struct process* current = get_current_process();
    current->esp = (uint32_t)esp;

    struct process* next = get_next_process();
    while (next->state != PROCESS_RUNNING) {
        switch (next->state) {
        case PROCESS_RUNNING:
            break;
        case PROCESS_SLEEPING:
            if (time.seconds >= next->wake_time.seconds && time.millisecond >= next->wake_time.millisecond) {
                next->state = PROCESS_RUNNING;
                break;
            } else {
                next = get_next_process();
            }
        case PROCESS_TERMINATED:
            free_pde_table(&next->page_table->pde);
            vfs_close_file(next->stdout);
            vfs_close_file(next->stdin);
            vfs_close_file(next->stderr);
            uint32_t id = next->id;
            next = get_next_process();
            remove_process(id);
            break;
        default:
            next = get_next_process();
            break;
        }
    }

    outb(PIC1_CMD, PIC_EOI);
    __asm__ volatile("mov %0, %%ebx\n\t" ::"r"(&next->page_table->pde));
    return next->esp;
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
