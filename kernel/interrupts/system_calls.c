#include "system_calls.h"
#include "filesystem/virtual-filesystem.h"
#include "irq_handlers.h"
#include <harddrive/ata.h>
#include <heap.h>
#include <keyboard/input.h>
#include <print.h>
#include <stdint.h>
#include <string.h>
#include <terminal/tty.h>

struct syscall_regs {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

void syscall_c(struct syscall_regs* regs)
{
    printf("Syscall: eax = %d\n", regs->eax);
    switch (regs->eax) {
    case 0x00:
        break;
    case 0x01:
        regs->eax = 0x500000;
        regs->ebx = (uint32_t)vfs_open_file("/dev/tty", VFS_READ | VFS_WRITE);
        regs->ecx = regs->ebx;
        regs->edx = regs->ebx;
        break;
    case 0x02:
        regs->ebx = (uint32_t)vfs_open_file((char*)regs->ebx, regs->ecx);
        break;
    case 0x03:
        vfs_close_file((void*)regs->ebx);
        break;
    case 0x04:
        __asm__("sti\n");
        regs->eax = vfs_read((void*)regs->ebx, (void*)regs->edx, regs->ecx);
        __asm__("cli\n");
        break;
    case 0x05:
        regs->eax = vfs_write((void*)regs->ebx, (void*)regs->edx, regs->ecx);
        break;
    case 0x06:
        vfs_ioctl((void*)regs->ebx, (uint32_t*)regs->ecx, (uint32_t*)regs->edx);
        break;
    case 0x07:
        vfs_seek((void*)regs->ebx, regs->ecx, regs->edx);
        break;
    case 0x08:
        regs->ecx = vfs_tell((void*)regs->ebx);
        break;
    case 0x09:
        break;
    default:
        break;
    }
}
