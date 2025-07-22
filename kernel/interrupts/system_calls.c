#include "system_calls.h"
#include <filesystem/virtual-filesystem.h>
#include <harddrive/ata.h>
#include <heap.h>
#include <keyboard/input.h>
#include <pager.h>
#include <print.h>
#include <process.h>
#include <stdint.h>
#include <terminal/tty.h>
#include <x86_64_structures.h>

void syscall_c(struct registers* regs)
{
    switch (regs->eax) {
        struct process* process;
        struct process_init_data* pd;

    case 0x00:
        break;

    case 0x01:
        process = get_current_process();
        regs->ebx = (uint32_t)process->stdout;
        regs->ecx = (uint32_t)process->stdin;
        regs->edx = (uint32_t)process->stderr;
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
        regs->eax = vfs_create_regular_file((char*)regs->ebx);
        break;

    case 0x0a:
        break;
        // nops
    case 0x0f:
        break;

    case 0x10:
        process = get_current_process();
        regs->ebx = (uint32_t)new_page((void*)regs->ebx, &process->page_table->pde, 0);
        break;

    case 0x11:
        process = get_current_process();
        free_page((void*)regs->ebx, &process->page_table->pde);
        break;

    case 0x12:
        process = get_current_process();
        regs->ebx = virt_to_phys(regs->ebx, &process->page_table->pde);
        break;

    case 0x13:
        regs->ebx = virt_to_phys(regs->ebx, (PDETable*)regs->edx);
        break;

    case 0x14:
        break;
        // nops
    case 0x1f:
        break;

    case 0x20:
        pd = (void*)regs->ebx;
        regs->ebx = (uint32_t)create_process(pd->name, pd->initial_state, pd->entry_point, pd->stack_base, pd->page_table, pd->stdout, pd->stdin, pd->stderr);
        break;
    }
    //__asm__ volatile("nop\n\t"); // needed for gcc as it made the wrong jump address
    return;
}
