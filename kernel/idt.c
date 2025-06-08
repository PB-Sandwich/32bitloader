#include "idt.h"
#include <stdint.h>

struct IDTEntry make_idt_entry(uint32_t *offset, uint16_t selector, uint8_t type_attr) {
    struct IDTEntry entry = {
        .offset_low = ((uint16_t)offset) & 0xffff,
        .selector = selector,
        .zero = 0,
        .type_attr = 0b10000000 | type_attr,
        .offset_high = (uint16_t)((uint32_t)offset >> 16),
    };
    return entry;
}
