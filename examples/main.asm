[org 0x300008]
[bits 32]
start:
    mov eax, 0x01
    int 0x40
    mov eax, 0x02
    mov ebx, string
    int 0x40
    hlt

string:
    db "test print string", 0xA, 0
