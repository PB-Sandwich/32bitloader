
#pragma once

#include <stdint.h>

struct TSS_descriptor {
    uint16_t limit_low16;
    uint16_t base_low16;
    uint8_t base_mid8;
    uint8_t type_attr;
    uint8_t limit_high4_attr;
    uint8_t base_high8;
} __attribute__((packed));

struct TSS {
    uint32_t link; // last tss selector
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;

    uint32_t cr3;

    uint32_t eip;
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;

    uint32_t esp;
    uint32_t ebp;

    uint32_t esi;
    uint32_t edi;

    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;

    uint32_t ldt_selector;
    uint32_t io_bitmap_base;

    uint32_t ssp;
};
