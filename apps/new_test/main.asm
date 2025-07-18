[org 0x300000 - 4]
[bits 32]
dd 0x300000

mov eax, 0x01
int 0x40

mov eax, 0x05
mov ecx, 4
mov edx, str1
int 0x40

mov eax, 0x04
mov ecx, 1000
mov edx, str1
int 0x40

hlt
hlt
hlt

ret

str1:
    db "test"

stdout:
    dd 0
