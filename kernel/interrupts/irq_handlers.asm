[bits 32]
global irq0_timer
extern irq0_timer_c
extern get_current_process

irq0_timer:
    cli
    pusha

    mov eax, esp

    mov eax, esp
    push eax

    call irq0_timer_c

    add esp, 4

    mov esp, eax

    mov eax, cr3
    and eax, 0xfff
    or eax, ebx,
    mov cr3, eax

    popa
    sti
    iret

