#include "system_calls.h"
#include "../ata.h"
#include "../tty.h"
#include "irq_handlers.h"
#include <stdint.h>

void syscall(struct interrupt_frame* frame)
{
    uint32_t eax_val;
    uint32_t ebx_val;
    uint32_t ecx_val;
    uint32_t edx_val;
    __asm__ __volatile__("mov %%eax, %0" : "=r"(eax_val));
    __asm__ __volatile__("mov %%ebx, %0" : "=r"(ebx_val));
    __asm__ __volatile__("mov %%ecx, %0" : "=r"(ecx_val));
    __asm__ __volatile__("mov %%edx, %0" : "=r"(edx_val));

    switch (eax_val) {
    case 0x00:
        break;
    case 0x01:
        clear();
        break;
    case 0x02:
        print_string((uint8_t*)ebx_val);
        break;
    case 0x03:
        ebx_val = (uint32_t)text_buffer;
        ecx_val = VGA_WIDTH;
        edx_val = VGA_HEIGHT;
        break;
    case 0x04:
        break;
    case 0x05:
        key_pressed = 0;
        while (!key_pressed)
            ;
        ebx_val = scancode;
        break;
    case 0x06:
        keyboard_function = (void*)ebx_val;
        break;
    case 0x07:
        for (int i = 0; i < ecx_val; i++) {
            ata_read_sector(ebx_val + i, (uint8_t*)(edx_val + (i * 512)));
        }
        break;
    case 0x08:
        for (int i = 0; i < ecx_val; i++) {
            ata_write_sector(ebx_val + i, (uint8_t*)(edx_val + (i * 512)));
        }
        break;
    default:
        break;
    }
}
