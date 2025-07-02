[org 0x2FFFFC]
dd 0x300000

mov eax, 0x03
mov ecx, 4
mov edx, str1
int 0x40

mov eax, 0x04
mov ecx, 4
mov edx, str1
int 0x40

hlt
hlt
hlt

ret

str1:
    db "test"
