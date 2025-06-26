#include "system_calls.h"
#include "irq_handlers.h"
#include <harddrive/ata.h>
#include <stdint.h>
#include <terminal/tty.h>

struct syscall_regs {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

void syscall_c(struct syscall_regs* regs)
{
    switch (regs->eax) {
    case 0x00:
        break;
    case 0x01:
        clear();
        break;
    case 0x02:
        print_string((uint8_t*)regs->ebx);
        break;
    case 0x03:
        regs->ebx = (uint32_t)text_buffer;
        regs->ecx = VGA_WIDTH;
        regs->edx = VGA_HEIGHT;
        break;
    case 0x04:
        set_cursor_pos(regs->ebx, regs->ecx);
        break;
    case 0x05:
        clear_key_pressed();
        __asm__ volatile("sti");
        while (key_pressed() == 0)
            ;
        __asm__ volatile("cli");
        regs->ebx = scancode();
        break;
    case 0x06:
        set_keyboard_function((void*)regs->ebx);
        break;
    case 0x07:
        for (int i = 0; i < regs->ecx; i++) {
            ata_read_sector(regs->ebx + i, (uint8_t*)(regs->edx + (i * 512)));
        }
        break;
    case 0x08:
        for (int i = 0; i < regs->ecx; i++) {
            ata_write_sector(regs->ebx + i, (uint8_t*)(regs->edx + (i * 512)));
        }
        break;
    default:
        break;
    }
}

void syscall(struct interrupt_frame* frame)
{
    __asm__ __volatile__(
        "pushf\n\t"
        "pusha\n\t"
        "mov %%esp, %%eax\n\t" // Move pointer to pushed registers into eax
        "push %%eax\n\t" // Push as argument
        "call syscall_c\n\t"
        "add $4, %%esp\n\t"
        "popa\n\t"
        "popf\n\t"
        "iret\n\t"
        :
        :
        : "eax");
}
