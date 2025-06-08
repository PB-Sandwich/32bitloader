#include "error_handlers.h"
#include "../print.h"

void divide_by_zero(struct interrupt_frame *frame) { return; }

void debug(struct interrupt_frame *frame) { return; }
void non_maskable_interrupt(struct interrupt_frame *frame) { return; }
void breakpoint(struct interrupt_frame *frame) { return; }
void overflow(struct interrupt_frame *frame) { return; }
void bound_range(struct interrupt_frame *frame) { return; }
void invalid_opcode(struct interrupt_frame *frame) { return; }
void device_not_available(struct interrupt_frame *frame) { return; }
void double_fault(struct interrupt_frame *frame, uint32_t error_code) {
  return;
}
//__attribute__ ((interrupt)) not used in modern systems
// void coprocessor_segment_overrun(struct interrupt_frame *frame){ return; }
void invalid_tss(struct interrupt_frame *frame, uint32_t error_code) { return; }
void segment_not_present(struct interrupt_frame *frame, uint32_t error_code) {
  return;
}
void stack(struct interrupt_frame *frame, uint32_t error_code) { return; }
void general_protection(struct interrupt_frame *frame, uint32_t error_code) {
  return;
}
void page_fault(struct interrupt_frame *frame, uint32_t error_code) { return; }

//
// 15 reserved
void x87_floating_point(struct interrupt_frame *frame) { return; }
void alignment_check(struct interrupt_frame *frame, uint32_t error_code) {
  return;
}
void machine_check(struct interrupt_frame *frame) { return; }
void SIMD_floating_point(struct interrupt_frame *frame) { return; }
//
// 20 reserved
void control_protection(struct interrupt_frame *frame, uint32_t error_code) {
  return;
}
//
// 22 - 27 reserved
//
//
void hypervisor_injection(struct interrupt_frame *frame) { return; }
void VMM_communication(struct interrupt_frame *frame) { return; }
void security_exception(struct interrupt_frame *frame) { return; }
//
// 31 reserved
