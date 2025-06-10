
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

__attribute__((interrupt)) void divide_by_zero(struct interrupt_frame *frame);
__attribute__((interrupt)) void debug(struct interrupt_frame *frame);
__attribute__((interrupt)) void
non_maskable_interrupt(struct interrupt_frame *frame);
__attribute__((interrupt)) void breakpoint(struct interrupt_frame *frame);
__attribute__((interrupt)) void overflow(struct interrupt_frame *frame);
__attribute__((interrupt)) void bound_range(struct interrupt_frame *frame);
__attribute__((interrupt)) void invalid_opcode(struct interrupt_frame *frame);
__attribute__((interrupt)) void
device_not_available(struct interrupt_frame *frame);
__attribute__((interrupt)) void double_fault(struct interrupt_frame *frame,
                                             uint32_t error_code);
//__attribute__ ((interrupt)) not used in modern systems
// void coprocessor_segment_overrun(struct interrupt_frame *frame);
__attribute__((interrupt)) void invalid_tss(struct interrupt_frame *frame,
                                            uint32_t error_code);
__attribute__((interrupt)) void
segment_not_present(struct interrupt_frame *frame, uint32_t error_code);
__attribute__((interrupt)) void stack(struct interrupt_frame *frame,
                                      uint32_t error_code);
__attribute__((interrupt)) void
general_protection(struct interrupt_frame *frame, uint32_t error_code);
__attribute__((interrupt)) void page_fault(struct interrupt_frame *frame,
                                           uint32_t error_code);
//
// 15 reserved
void x87_floating_point(struct interrupt_frame *frame);
__attribute__((interrupt)) void alignment_check(struct interrupt_frame *frame,
                                                uint32_t error_code);
__attribute__((interrupt)) void machine_check(struct interrupt_frame *frame);
__attribute__((interrupt)) void
SIMD_floating_point(struct interrupt_frame *frame);
//
// 20 reserved
__attribute__((interrupt)) void
control_protection(struct interrupt_frame *frame, uint32_t error_code);
//
// 22 - 27 reserved
//
//
__attribute__((interrupt)) void
hypervisor_injection(struct interrupt_frame *frame);
__attribute__((interrupt)) void
VMM_communication(struct interrupt_frame *frame);
__attribute__((interrupt)) void
security_exception(struct interrupt_frame *frame);
//
// 31 reserved
