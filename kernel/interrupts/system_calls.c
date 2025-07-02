#include "system_calls.h"
#include "irq_handlers.h"
#include <harddrive/ata.h>
#include <heap.h>
#include <keyboard/input.h>
#include <stdint.h>
#include <string.h>
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
        break;
    case 0x02:
        break;
    case 0x03:
        break;
    case 0x04:
        break;
    case 0x05:
        break;
    case 0x06:
        break;
    case 0x07:
        break;
    case 0x08:
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
