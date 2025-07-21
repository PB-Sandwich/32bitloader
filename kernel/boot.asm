[org 0x7c00]
[bits 16]

TEXT_BUFFER equ 0xb8000
TEMP_KERNEL_LOCATION equ 0x7f00

BOOT_DISK: db 0
NUMBER_OF_SECTORS: db 0
    mov [BOOT_DISK], dl
    mov [NUMBER_OF_SECTORS], byte 80

    mov ax, 0
    mov es, ax ; extra segment
    mov ds, ax 
    mov bp, 0xf000 ; setting stack
    mov sp, bp

    mov ah, 0x42
    mov dl, [BOOT_DISK]
    mov si, DAP
    int 0x13

    ; set vidoe mode to 80x25 text mode
    mov ah, 0x0
    mov al, 0x3
    int 0x10

    mov ah, 0x01
    mov ch, 0x3F
    int 0x10

    jmp enter_protected_mode

    hlt

failed_to_read_disk:
    hlt

DAP:
    db 0x10          ; size of packet
    db 0             ; reserved
    dw 100           ; number of sectors to read
    dw TEMP_KERNEL_LOCATION
    dw 0             ; segment (we’re in real mode still)
    dq 1             ; starting LBA (starts at 0, so LBA=1 = second sector)


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
    jmp enable_paging

enable_paging:
    mov eax, cr0
    and eax, 0b01111111111111111111111111111111
    mov cr0, eax ; disable paging

    ; mov eax, cr4 ; setting Page Address Extensions
    ; or eax, 0b00000000000000000000000000100000
    ; mov cr4, eax

PDE_addr equ 0x400000
PTE_addr equ 0x401000
PDE_flags equ 0b000000001011
PTE_flags equ 0b000000001011

    mov eax, PDE_addr
    mov dword [eax], PTE_addr | PDE_flags

    mov eax, PTE_addr

    mov ecx, eax
    mov ebx, 0x0000
.loop_start:
    cmp ecx, PTE_addr + 0xA000
    jge .loop_end
    mov [ecx], ebx
    or [ecx], dword PTE_flags
    add ecx, 4
    add ebx, 0x1000
    jmp .loop_start
.loop_end:

    mov eax, PDE_addr
    or eax, 0b0001000
    mov cr3, eax

    mov eax, cr0
    or eax, 0b10000000000000000000000000000000
    mov cr0, eax ; enable paging

    mov eax, cr4
    or eax, 1 << 7 ; enable global pages
    mov cr4, eax

    jmp start_kernel

start_kernel:
    cld
    mov ecx, 100 * 512
    mov esi, TEMP_KERNEL_LOCATION
    mov edi, 0x100000
    rep movsb

    push GDT_Start
    call 0x100000

    hlt


times 440-($-$$) db 0
dd 0 ; unique disk id
dw 0 ; reserved
partition1:
    db 0x80            ; Bootable (0x80 = bootable, 0x00 = not)
    db 0x00            ; Starting CHS - head
    db 0x01            ; Starting CHS - sector (bits 0-5) + cylinder (bits 6-7)
    db 0x00            ; Starting CHS - cylinder (low 8 bits)
    db 0x0C            ; Partition type (0x0B = FAT32 CHS, 0x0C = FAT32 LBA, 0x83 = Linux)
    db 0x00            ; Ending CHS - head
    db 0x2048          ; Ending CHS - sector
    db 0x00            ; Ending CHS - cylinder
    dd 1               ; Starting LBA (start at sector 1)
    dd 2048            ; Number of sectors in partition
partition2:
    dq 0 ; unused
    dq 0 ; unused
partition3:
    dq 0 ; unused
    dq 0 ; unused
partition4:
    dq 0 ; unused
    dq 0 ; unused
dw 0xaa55
