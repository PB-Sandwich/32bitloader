[org 0x7c00]


TEXT_BUFFER equ 0xb8000

TEMP_KERNEL_LOCATION equ 0x7f00
BOOT_DISK: db 0
NUMBER_OF_SECTORS: db 0
    mov [BOOT_DISK], dl
    mov [NUMBER_OF_SECTORS], byte 50

    mov ax, 0
    mov es, ax ; extra segment
    mov ds, ax 
    mov bp, 0xf000 ; setting stack
    mov sp, bp

    mov bx, TEMP_KERNEL_LOCATION ; segment offset
    mov al, [NUMBER_OF_SECTORS] ; number of sectors to read

    mov ah, 2 ; bios code

    mov ch, 0 ; cylinder number
    mov dh, 0 ; head number
    mov cl, 2 ; sector number (starts at 1 instead of 0)

    mov dl, [BOOT_DISK]
    int 0x13

    jc failed_to_read_disk
    cmp al, [NUMBER_OF_SECTORS]
    jne failed_to_read_disk

    ; set vidoe mode to 80x25 text mode
    mov ah, 0x0
    mov al, 0x3
    int 0x10

    jmp enter_protected_mode

    hlt

failed_to_read_disk:
    hlt

; Entering protected mode
GDT_Start:
    null_descriptor:
        dd 0
        dd 0
    code_descriptor:
        dw 0xffff ; first 16 bits of the limit
        dw 0 ; first 24 bits for the base
        db 0
        db 0b10011010 ; pres, priv, type, type flags
        db 0b11001111 ; other flags + last 4 bits of limit
        db 0 ; last 8 bits of the base
    data_descriptor:
        dw 0xffff ; first 16 bits of the limit
        dw 0 ; first 24 bits for the base
        db 0
        db 0b10010010 ; pres, priv, type, type flags
        db 0b11001111 ; other flags + last 4 bits of limit
        db 0 ; last 8 bits of the base
GDT_End:

GDT_Descriptor:
    dw GDT_End - GDT_Start - 1 ; size
    dd GDT_Start ; Start pointer

CODE_SEG equ code_descriptor - GDT_Start
DATA_SEG equ data_descriptor - GDT_Start

enter_protected_mode:
    cli
    lgdt [GDT_Descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:start_protected_mode

[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp

    cld
    mov ecx, 50 * 512 ; 30 cd sectors
    mov esi, TEMP_KERNEL_LOCATION
    mov edi, 0x100000
    rep movsb

    push GDT_Start
    call 0x100000

    hlt

times 510-($-$$) db 0
dw 0xaa55
