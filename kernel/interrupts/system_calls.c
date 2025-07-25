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
        PageTable* page_table;

    case 0x00:
        break;

    case 0x01:
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
        if (regs->edx == (uint32_t)PAGER_ERROR) {
            page_table = get_current_process()->page_table;
        } else {
            page_table = (PageTable*)regs->edx;
        }
        regs->ebx = (uint32_t)new_page((void*)regs->ebx, &page_table->pde, 0);
        break;

    case 0x11:
        if (regs->edx == (uint32_t)PAGER_ERROR) {
            page_table = get_current_process()->page_table;
        } else {
            page_table = (PageTable*)regs->edx;
        }
        free_page((void*)regs->ebx, &page_table->pde);
        break;

    case 0x12:
        regs->ebx = (uint32_t)soft_copy_table(kernel_table, 1);
        break;

    case 0x13:
        break;
        // nops
    case 0x1f:
        break;

    case 0x20:
        regs->ebx = (uint32_t)get_current_process();
        break;

    case 0x21:
        pd = (void*)regs->ebx;
        regs->ebx = (uint32_t)create_process(pd->name, pd->initial_state, pd->entry_point, pd->stack_base_current_table, pd->stack_base_apps_table, pd->page_table, pd->stdout, pd->stdin, pd->stderr);
        break;

    case 0x22:
        process = get_current_process();
        process->state = PROCESS_TERMINATED;
        break;
    }
    //__asm__ volatile("nop\n\t"); // needed for gcc as it made the wrong jump address
    return;
}
