#include "error_handlers.h"
#include <print.h>
#include <terminal/tty.h>

void divide_by_zero(struct interrupt_frame* frame)
{
    printf("divide_by_zero error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void debug(struct interrupt_frame* frame)
{
    return;
}
void non_maskable_interrupt(struct interrupt_frame* frame)
{
    printf("non_maskable_interrupt error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void breakpoint(struct interrupt_frame* frame)
{
    printf("breakpoint error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void overflow(struct interrupt_frame* frame)
{
    printf("overflow error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void bound_range(struct interrupt_frame* frame)
{

    printf("bound_range error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void invalid_opcode(struct interrupt_frame* frame)
{

    printf("invalid_opcode error @ %x @ ", frame->eip);
    while (1)
        ;
    return;
}
void device_not_available(struct interrupt_frame* frame)
{

    printf("device_not_available error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void double_fault(struct interrupt_frame* frame, uint32_t error_code)
{

    printf("double_fault error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
//__attribute__ ((interrupt)) not used in modern systems
// void coprocessor_segment_overrun(struct interrupt_frame *frame){ return; }
void invalid_tss(struct interrupt_frame* frame, uint32_t error_code)
{

    printf("invalid_tss error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void segment_not_present(struct interrupt_frame* frame, uint32_t error_code)
{

    printf("segment_not_present error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void stack(struct interrupt_frame* frame, uint32_t error_code)
{

    printf("stack error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void general_protection(struct interrupt_frame* frame, uint32_t error_code)
{

    printf("general_protection error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void page_fault(struct interrupt_frame* frame, uint32_t error_code)
{

    printf("page_fault error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
//
// 15 reserved
void x87_floating_point(struct interrupt_frame* frame)
{

    printf("x87_floating_point error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void alignment_check(struct interrupt_frame* frame, uint32_t error_code)
{

    printf("alignment_check error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void machine_check(struct interrupt_frame* frame)
{

    printf("machine_check error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void SIMD_floating_point(struct interrupt_frame* frame)
{

    printf("SIMD_floating_point error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
//
// 20 reserved
void control_protection(struct interrupt_frame* frame, uint32_t error_code)
{

    printf("control_protection error @ %x\n", frame->eip);
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

    printf("hypervisor_injection error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void VMM_communication(struct interrupt_frame* frame)
{

    printf("VMM_communication error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
void security_exception(struct interrupt_frame* frame)
{

    printf("security_exception error @ %x\n", frame->eip);
    while (1)
        ;
    return;
}
//
// 31 reserved
