[bits 32] 
global syscall
extern syscall_c

syscall:
    cli
    pusha

    mov eax, esp
    push eax
    call syscall_c
    add esp, 4

    popa
    sti
    iret
