#include <exit.h>
#include <filesystem/estros-fs.h>
#include <filesystem/virtual-filesystem.h>
#include <harddrive/ata.h>
#include <harddrive/hdd.h>
#include <heap.h>
#include <idt.h>
#include <inboutb.h>
#include <interrupts/error_handlers.h>
#include <interrupts/irq_handlers.h>
#include <interrupts/system_calls.h>
#include <keyboard/input.h>
#include <keyboard/keyboard.h>
#include <memutils.h>
#include <pager.h>
#include <pic.h>
#include <pit.h>
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

void main();

__attribute__((section(".text.start"))) void kernel_entry(struct GDT* gdt)
{
    // enable sse
    // https://wiki.osdev.org/SSE
    __asm__ volatile("mov %cr0, %eax\n\t"
                     "and $0xfffb, %ax\n\t"
                     "or  $0x2, %ax\n\t"
                     "mov %eax, %cr0\n\t"
                     "mov %cr4, %eax\n\t"
                     "or  $(3 << 9), %ax\n\t"
                     "mov %eax, %cr4\n\t");

    // interrupt init
    struct IDTPointer idt_pointer;
    idt_pointer.limit = 0xffff;
    idt_pointer.base = IDT_BASE;
    __asm__ volatile("lidt (%0)" : : "r"(&idt_pointer));

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

    __asm__ volatile("lgdt (%0)" : : "r"(&gdt_descriptor)); // load the new GDT descriptor
    __asm__ volatile("ltr %%ax" : : "a"((3 << 3) | 0x0)); // load the TSS into the task register

    // initializing pic
    // https://wiki.osdev.org/8259_PIC
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

    // mask everything but timer irq (0) the keyboard irq (1) and spurious irq (7, 15)
    // because thats all we care about for now
    outb(PIC1_DATA, 0b01111100);
    outb(PIC2_DATA, 0b01111111);

    idt_entries[32] = make_idt_entry((uint32_t*)irq0_timer, 0x8, 0xE);
    idt_entries[33] = make_idt_entry((uint32_t*)irq1_keyboard, 0x8, 0xE);
    idt_entries[39] = make_idt_entry((uint32_t*)irq7_15_spurious, 0x8, 0xE);
    idt_entries[47] = make_idt_entry((uint32_t*)irq7_15_spurious, 0x8, 0xE);

    idt_entries[0x40] = make_idt_entry((uint32_t*)syscall, 0x8, 0xE);

    // enable the pit timer for 10ms
    uint16_t divisor = TIMER_DIVISOR;
    // Send the command byte
    outb(0x43, 0x36); // Channel 0, LSB then MSB, mode 3 (square wave)
    // Send divisor low byte first
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));

    __asm__ volatile("sti"); // reenable maskable interrupts

    __asm__ volatile(
        "mov %0, %%esp\n\t"
        :
        : "r"(0x90000)
        : "esp");

    main();

    exit_kernel();
}

void main()
{
    init_heap((uint8_t*)0x210000, 0x90000);

    vfs_init();

    vfs_create_device_file_no_checks("/dev/hdd", get_hdd_file_operations(), VFS_BLOCK_DEVICE);

    fs_set_harddrive("/dev/hdd");

    vfs_set_driver(get_fs_driver_operations());

    vfs_create_device_file("/dev/tty", get_tty_file_operations(), VFS_CHARACTER_DEVICE);

    set_print_output("/dev/tty");

    VFSFile* log_file = vfs_open_file("/sys/kernel.log", VFS_WRITE);
    if (log_file == NULL) {
        if (vfs_create_regular_file("/sys/kernel.log") != 0) {
            printf("Unable to create log file /sys/kernel.log\n");
        } else {
            log_file = vfs_open_file("/sys/kernel.log", VFS_WRITE);
            if (log_file == NULL) {
                printf("Unable to open log file /sys/kernel.log\n");
            }
        }
    }
    if (log_file == NULL) {
        return;
    }
    vfs_close_file(log_file);

    // set_print_output("/sys/kernel.log");

    vfs_create_device_file("/dev/kdb", get_keyboard_file_operations(), VFS_BLOCK_DEVICE);

    set_input_kdb_dev("/dev/kdb");

    init_pager();

    PDETable* kernel_table = (PDETable*)create_new_table();

    while (new_page(PAGER_ERROR, kernel_table, PAGE_GLOBAL) < (void*)0x400000 - PAGE_SIZE)
        ;

    load_page_table(kernel_table);

    PDETable* test = (PDETable*)soft_copy_table((PageTable*)kernel_table, 1);
    new_page(PAGER_ERROR, test, 0);
    new_page(PAGER_ERROR, kernel_table, 0);

    load_page_table(test);
    //load_page_table(kernel_table);

    while (new_page(PAGER_ERROR, test, PAGE_GLOBAL) < (void*)0x1000000 - PAGE_SIZE)
        ;

    VFSFile* file = vfs_open_file("/apps/new_test.bin", VFS_READ);
    if (file == NULL) {
        printf("Unable to open file\n");
        return;
    }

    uint8_t* buf = (uint8_t*)(0x400000);
    vfs_seek(file, 4, VFS_BEG);
    while (vfs_read(file, buf, 1024)) {
        buf += 1024;
    }

    uint32_t entry_point;
    vfs_seek(file, 0, VFS_BEG);
    vfs_read(file, &entry_point, 4);

    vfs_close_file(file);

    void (*entry_function)() = (void*)entry_point;
    entry_function();

    // VFSDirectory* dir = vfs_open_directory("/");
    // if (dir == NULL) {
    //     printf("Unable to open directory\n");
    //     return;
    // }
    // printf("Opened dir\n");
    // for (int i = 0; i < dir->entries_length; i++) {
    //     printf("Entry: %s\n", dir->entries[i].path);
    // }

    while (1)
        ;
}
