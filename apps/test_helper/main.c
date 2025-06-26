#include <stdint.h>

struct KernelExports {
    void (*printf)(const char* fmt, ...);

    void (*print_char)(uint8_t chr);
    void (*print_string)(uint8_t* str);
    void (*set_color)(uint8_t fg, uint8_t bg);
    void (*newline)();
    void (*clear)();
    void (*get_cursor_pos)(uint8_t* x, uint8_t* y);
    void (*set_cursor_pos)(uint8_t x, uint8_t y);

    void (*clear_key_pressed)(); // clears the key pressed flag
    uint8_t (*key_pressed)(); // returns 1 if a key is pressed
    uint8_t (*scancode)(); // gets the last scancode
    void (*set_keyboard_function)(void (*(*keyboard_function_))(uint8_t scancode));

    uint8_t (*wait_for_keypress)();
    uint8_t* (*get_line)(); // returns the pointer to input buffer (*)(which will get overwritten on next call)
    uint8_t (*keycode_to_ascii)(enum Keycode kc);
    uint8_t (*scancode_to_keycode)(uint8_t sc);

    void (*ata_read_sector)(uint32_t lba, uint8_t* buffer);
    void (*ata_write_sector)(uint32_t lba, uint8_t* buffer);

    struct IDTEntry (*make_idt_entry)(uint32_t* offset, uint16_t selector, uint8_t type_attr);

    void (*memcpy)(void* dest, void* source, uint32_t size);
};

void test_helper(void (*printf)(const char*, ...))
{
    printf("Helper\n");
}

int app_main(struct KernelExports* kernel_exports)
{
    kernel_exports->printf("Test\n");
    test_helper(kernel_exports->printf);
    kernel_exports->wait_for_keypress();
    return 0;
}
