[bits 32]
global irq0_timer
extern irq0_timer_c

irq0_timer:
    cli
    pusha

    mov eax, esp

    ; Compute pointer to interrupt frame:
    ; The CPU pushed 5 words after pusha if ring 0 → [eip, cs, eflags, esp, ss]
    ; So it’s located at (esp + 8*4)
    mov eax, esp
    push eax

    call irq0_timer_c

    add esp, 4

    mov esp, eax

    popa
    sti
    iret

