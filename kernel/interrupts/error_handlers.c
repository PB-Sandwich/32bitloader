#include "error_handlers.h"
#include "../print.h"
#include "../tty.h"

void divide_by_zero(struct interrupt_frame* frame)
{
    clear();
    printf("divide_by_zero error\n");
    while (1)
        ;
    return;
}
void debug(struct interrupt_frame* frame)
{
    clear();
    printf("debug error\n");
    while (1)
        ;
    return;
}
void non_maskable_interrupt(struct interrupt_frame* frame)
{
    clear();
    printf("non_maskable_interrupt error\n");
    while (1)
        ;
    return;
}
void breakpoint(struct interrupt_frame* frame)
{
    clear();
    printf("breakpoint error\n");
    while (1)
        ;
    return;
}
void overflow(struct interrupt_frame* frame)
{
    clear();
    printf("overflow error\n");
    while (1)
        ;
    return;
}
void bound_range(struct interrupt_frame* frame)
{
    clear();
    printf("bound_range error\n");
    while (1)
        ;
    return;
}
void invalid_opcode(struct interrupt_frame* frame)
{
    clear();
    printf("invalid_opcode error\n");
    while (1)
        ;
    return;
}
void device_not_available(struct interrupt_frame* frame)
{
    clear();
    printf("device_not_available error\n");
    while (1)
        ;
    return;
}
void double_fault(struct interrupt_frame* frame, uint32_t error_code)
{
    clear();
    printf("double_fault error\n");
    while (1)
        ;
    return;
}
//__attribute__ ((interrupt)) not used in modern systems
// void coprocessor_segment_overrun(struct interrupt_frame *frame){ return; }
void invalid_tss(struct interrupt_frame* frame, uint32_t error_code)
{
    clear();
    printf("invalid_tss error\n");
    while (1)
        ;
    return;
}
void segment_not_present(struct interrupt_frame* frame, uint32_t error_code)
{
    clear();
    printf("segment_not_present error\n");
    while (1)
        ;
    return;
}
void stack(struct interrupt_frame* frame, uint32_t error_code)
{
    clear();
    printf("stack error\n");
    while (1)
        ;
    return;
}
void general_protection(struct interrupt_frame* frame, uint32_t error_code)
{
    clear();
    printf("general_protection error\n");
    while (1)
        ;
    return;
}
void page_fault(struct interrupt_frame* frame, uint32_t error_code)
{
    clear();
    printf("page_fault error\n");
    while (1)
        ;
    return;
}
//
// 15 reserved
void x87_floating_point(struct interrupt_frame* frame)
{
    clear();
    printf("x87_floating_point error\n");
    while (1)
        ;
    return;
}
void alignment_check(struct interrupt_frame* frame, uint32_t error_code)
{
    clear();
    printf("alignment_check error\n");
    while (1)
        ;
    return;
}
void machine_check(struct interrupt_frame* frame)
{
    clear();
    printf("machine_check error\n");
    while (1)
        ;
    return;
}
void SIMD_floating_point(struct interrupt_frame* frame)
{
    clear();
    printf("SIMD_floating_point error\n");
    while (1)
        ;
    return;
}
//
// 20 reserved
void control_protection(struct interrupt_frame* frame, uint32_t error_code)
{
    clear();
    printf("control_protection error\n");
    while (1)
        ;
    return;
}
//
// 22 - 27 reserved
//
//
void hypervisor_injection(struct interrupt_frame* frame)
{
    clear();
    printf("hypervisor_injection error\n");
    while (1)
        ;
    return;
}
void VMM_communication(struct interrupt_frame* frame)
{
    clear();
    printf("VMM_communication error\n");
    while (1)
        ;
    return;
}
void security_exception(struct interrupt_frame* frame)
{
    clear();
    printf("security_exception error\n");
    while (1)
        ;
    return;
}
//
// 31 reserved
