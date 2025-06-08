#include <stdint.h>

struct IDTPointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct IDTEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

#define IDT_BASE 0x200000

struct IDTEntry make_idt_entry(uint32_t *offset, uint16_t selector, uint8_t type_attr);
