#include <filesystem/filesystem.h>
#include <harddrive/ata.h>
#include <heap.h>
#include <idt.h>
#include <inboutb.h>
#include <interrupts/error_handlers.h>
#include <interrupts/irq_handlers.h>
#include <interrupts/system_calls.h>
#include <keyboard/input.h>
#include <memutils.h>
#include <pic.h>
#include <print.h>
#include <stdint.h>
#include <terminal/tty.h>
#include <tss.h>

struct GDT {
    uint32_t low;
    uint32_t high;
} __attribute__((packed));

struct GDT_descriptor {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct App {
    uint32_t size;
    uint32_t entry;
};

struct {
    void* printf;
    void* print_char;
    void* print_string;
    void* set_color;
    void* newline;
    void* clear;
    void* get_cursor_pos;
    void* set_cursor_pos;
    void* clear_key_pressed;
    void* key_pressed;
    void* scancode;
    void* set_keyboard_function;
    void* wait_for_keypress;
    void* get_line;
    void* keycode_to_ascii;
    void* scancode_to_keycode;
    void* ata_read_sector;
    void* ata_write_sector;
    void* make_idt_entry;
    void* memcpy;
} __attribute__((packed)) kernel_exports = {
    .printf = printf,

    .print_char = print_char,
    .print_string = print_string,
    .set_color = set_color,
    .newline = newline,
    .clear = clear,
    .get_cursor_pos = get_cursor_pos,
    .set_cursor_pos = set_cursor_pos,

    .clear_key_pressed = clear_key_pressed,
    .key_pressed = key_pressed,
    .scancode = scancode,
    .set_keyboard_function = set_keyboard_function,

    .wait_for_keypress = wait_for_keypress,
    .get_line = get_line,
    .keycode_to_ascii = keycode_to_ascii,
    .scancode_to_keycode = scancode_to_keycode,

    .ata_read_sector = ata_read_sector,
    .ata_write_sector = ata_write_sector,

    .make_idt_entry = make_idt_entry,

    .memcpy = memcpy
};

int main();

void kernel_entry(struct GDT* gdt)
{
    // interrupt init
    printf("setting interrupt descriptor table\n");
    struct IDTPointer idt_pointer;
    idt_pointer.limit = 0xffff;
    idt_pointer.base = IDT_BASE;
    __asm__ volatile("lidt (%0)" : : "r"(&idt_pointer));

    printf("setting exception handlers\n");
    struct IDTEntry* idt_entries = (struct IDTEntry*)IDT_BASE;
    idt_entries[0] = make_idt_entry((uint32_t*)divide_by_zero, 0x8, 0x0E);
    idt_entries[1] = make_idt_entry((uint32_t*)debug, 0x8, 0xE);
    idt_entries[2] = make_idt_entry((uint32_t*)non_maskable_interrupt, 0x8, 0xE);
    idt_entries[3] = make_idt_entry((uint32_t*)breakpoint, 0x8, 0xE);
    idt_entries[4] = make_idt_entry((uint32_t*)overflow, 0x8, 0xE);
    idt_entries[5] = make_idt_entry((uint32_t*)bound_range, 0x8, 0xE);
    idt_entries[6] = make_idt_entry((uint32_t*)invalid_opcode, 0x8, 0xE);
    idt_entries[7] = make_idt_entry((uint32_t*)device_not_available, 0x8, 0xE);
    idt_entries[8] = make_idt_entry((uint32_t*)double_fault, 0x8, 0xE);
    // 9 Coprocessor-Segment-Overrun (not used in modern systems)
    idt_entries[10] = make_idt_entry((uint32_t*)invalid_tss, 0x8, 0xE);
    idt_entries[11] = make_idt_entry((uint32_t*)segment_not_present, 0x8, 0xE);
    idt_entries[12] = make_idt_entry((uint32_t*)stack, 0x8, 0xE);
    idt_entries[13] = make_idt_entry((uint32_t*)general_protection, 0x8, 0xE);
    idt_entries[14] = make_idt_entry((uint32_t*)page_fault, 0x8, 0xE);
    // 15 reserved
    idt_entries[16] = make_idt_entry((uint32_t*)x87_floating_point, 0x8, 0xE);
    idt_entries[17] = make_idt_entry((uint32_t*)alignment_check, 0x8, 0xE);
    idt_entries[18] = make_idt_entry((uint32_t*)machine_check, 0x8, 0xE);
    idt_entries[19] = make_idt_entry((uint32_t*)SIMD_floating_point, 0x8, 0xE);
    // 20 reserved
    idt_entries[21] = make_idt_entry((uint32_t*)control_protection, 0x8, 0xE);
    // 22 - 27 reserved
    idt_entries[28] = make_idt_entry((uint32_t*)hypervisor_injection, 0x8, 0xE);
    idt_entries[29] = make_idt_entry((uint32_t*)VMM_communication, 0x8, 0xE);
    idt_entries[30] = make_idt_entry((uint32_t*)security_exception, 0x8, 0xE);
    // 31 reserved

    // tss init
    printf("setting task segment\n");
    static struct TSS tss = { 0 };
    tss.esp0 = 0x80000; // set the esp for privilege lvel zero switches
    tss.ss0 = 0x10; // stack segment for privilege level zero switches
    tss.io_bitmap_base = sizeof(struct TSS); // no io bit map so set to end of tss

    struct TSS_descriptor tss_descriptor = {
        .limit_low16 = sizeof(struct TSS) - 1,
        .base_low16 = (uint32_t)&tss & 0xFFFF,
        .base_mid8 = ((uint32_t)&tss >> 16) & 0xFF,
        .type_attr = 0b10001001, // present, ring 0, free TSS
        .limit_high4_attr = ((sizeof(struct TSS) >> 16) & 0xF),
        .base_high8 = ((uint32_t)&tss >> 24) & 0xFF
    };

    memcpy((void*)&gdt[3], (void*)&tss_descriptor, sizeof(struct TSS_descriptor)); // load it into GDT index 3

    struct GDT_descriptor gdt_descriptor = {
        .limit = sizeof(struct GDT) * 4 - 1, // 4 entries in the GDT
        .base = (uint32_t)gdt
    };

    printf("reloading global descriptor table and task register\n");
    __asm__ volatile("lgdt (%0)" : : "r"(&gdt_descriptor)); // load the new GDT descriptor
    __asm__ volatile("ltr %%ax" : : "a"((3 << 3) | 0x0)); // load the TSS into the task register

    // initializing pic
    // https://wiki.osdev.org/8259_PIC
    printf("initializing programmable interrupt controller\n");
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, 32); // set pic1 offset to interrupt 32
    io_wait();
    outb(PIC2_DATA, 40); // set pic2 offset to interrupt 40
    io_wait();
    // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC1_DATA, 4);
    io_wait();
    // ICW3: tell Slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 2);
    io_wait();
    // ICW4: have the PICs use 8086 mode (and not 8080 mode)
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // mask everything but the keyboard irq (1) and spurious irq (7, 15)
    // because thats all we care about for now
    outb(PIC1_DATA, 0b01111101);
    outb(PIC2_DATA, 0b01111111);

    printf("setting interrupt request handlers\n");
    idt_entries[33] = make_idt_entry((uint32_t*)irq1_keyboard, 0x8, 0xE);
    idt_entries[39] = make_idt_entry((uint32_t*)irq7_15_spurious, 0x8, 0xE);
    idt_entries[47] = make_idt_entry((uint32_t*)irq7_15_spurious, 0x8, 0xE);

    printf("enableing maskable interrupts\n");
    __asm__ volatile("sti"); // reenable maskable interrupts

    printf("setting syscall handler\n");
    idt_entries[0x40] = make_idt_entry((uint32_t*)syscall, 0x8, 0xE);

    // set stack
    __asm__ volatile(
        "mov %0, %%esp\n\t"
        :
        : "r"(0x90000)
        : "esp");

    main();

    while (1)
        ;
}

void launch_app(FS_RAMFileDescriptor* file)
{
    printf("%s\n", file->name);
    fs_read_file((void*)0x300000 - 4, file->hdd_file_descriptor.file_size_bytes, file);
    fs_close_file(file);
    uint32_t* entry_point = (uint32_t*)(0x300000 - 4);
    void (*entry_function)(void*) = (void*)*entry_point;
    entry_function(&kernel_exports);
    clear();
}

void display_file_info(FS_RAMFileDescriptor* file)
{
    clear();
    printf("Name: %s\n", file->name);
    printf("Type: ");
    switch (file->hdd_file_descriptor.type) {
    case FS_FILE:
        printf("File");
        break;
    case FS_DIRECTORY:
        printf("Directory");
        break;
    case FS_BLOCK_DEVICE:
        printf("Block Device");
        break;
    }
    printf("\n");
    printf("Path: %s\n", file->path);
    printf("Last accessed by: %s\n", file->last_accessed_path);
    printf("Last modified by: %s\n", file->last_modified_path);
    printf("Size: %d bytes\n", file->hdd_file_descriptor.file_size_bytes);
    printf("Size on disk: %d bytes\n", ((file->hdd_file_descriptor.file_size_bytes + 511) / 512) * 512);
    printf("Attributes: %x\n", file->hdd_file_descriptor.attributes);

    set_cursor_pos(0, VGA_HEIGHT - 1);
    printf("Press any key to leave...");

    wait_for_keypress();
    clear();
};

void display_list(FS_RAMFileDescriptor* list, uint32_t list_len, uint32_t selection)
{
    set_cursor_pos(0, 0);
    printf("j/k for up and down; i for info; enter to luanch\n");
    for (uint32_t i = 0; i < list_len; i++) {
        if (selection == i) {
            set_color(Black, LightGray);
        }
        printf("%s\n", list[i].name);
        set_color(White, Black);
    }
}

int main()
{
    clear();

    init_heap((uint8_t*)0x210000, 0x10000);

    fs_init_filesystem(0x10000 / SECTOR_SIZE);

    FS_RAMFileDescriptor* file = fs_open_file("/");
    FS_RAMFileDescriptor* entries = fs_read_directory(file);
    uint32_t dir_len = file->hdd_file_descriptor.file_size_bytes / 4;

    display_list(entries, dir_len, 0);

    uint32_t selection = 0;
    while (1) {
        enum Keycode kc = wait_for_keypress();
        switch (kc) {
        case J:
            if (selection < dir_len - 1) {
                selection++;
            }
            break;
        case K:
            if (selection > 0) {
                selection--;
            }
            break;
        case ENTER:
            launch_app(&entries[selection]);
            break;
        case I:
            display_file_info(&entries[selection]);
            break;
        }
        display_list(entries, dir_len, selection);
    }

    fs_free_directory(entries, dir_len);

    dump_heap();

    return 0;
}
