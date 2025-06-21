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

.halt:
    hlt
    jmp .halt

key_pressed:
    push ebp
    mov ebp, esp

    mov eax, 0x02
    mov ebx, string3
    int 0x40

    mov ecx, [ebp + 8]
    and ecx, 0b01111111
    mov edx, [scancode_to_ascii + ecx]
    mov [string2], dl
    mov ebx, string2
    int 0x40

    pop ebp
    ret

string:
    db "welcome!", 0xA, 0
string2:
    db "0", 0xA, 0
string3:
    db "key pressed: ", 0

scancode_to_ascii:
    db 27,  "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", 8
    ;      ESC                              BACKSPACE
    db 9,   "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", 13
    ;      TAB                                     ENTER
    db 0,   "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`"
    ;     (LCTRL or CAPS)
    db 0,   "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", 0
    ;    (LSHIFT and RSHIFT)
    db 0,   " "
    ;    (LALT)   SPACE

