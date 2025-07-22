
#pragma once

#include <stdint.h>

struct interrupt_frame {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
    uint32_t es;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
};

struct registers {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

struct sse_registers {
    uint32_t register_part[512 / 4];
};
