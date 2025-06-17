[org 0x300008]
[bits 32]
start:
    mov eax, 0x01
    int 0x40

    mov eax, 0x02
    mov ebx, string
    int 0x40

    mov eax, 0x05
    int 0x40

    add [string2], bl

    mov eax, 0x02
    mov ebx, string2
    int 0x40

    mov eax, 0x06
    mov ebx, key_pressed
    int 0x40

    hlt

key_pressed:
    push ebp
    mov ebp, esp

    mov eax, 0x02
    mov ebx, string3
    int 0x40

    pop ebp
    ret

string:
    db "welcome!", 0xA, 0
string2:
    db "0", 0xA, 0
string3:
    db "key pressed!", 0xA, 0
