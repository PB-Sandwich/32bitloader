[org 0x300000]
[bits 32]
start:
mov [0xb8000], word 0xffff
hlt
