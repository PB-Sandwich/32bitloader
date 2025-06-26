#include <stdint.h>
#include <stdbool.h>
#include <endian.h>
#include <string.h>

enum Keycode
{
    EKC_NONE = 0,
    EKC_ESC,   // 0x01
    EKC_KEY_1, // 0x02
    EKC_KEY_2,
    EKC_KEY_3,
    EKC_KEY_4,
    EKC_KEY_5,
    EKC_KEY_6,
    EKC_KEY_7,
    EKC_KEY_8,
    EKC_KEY_9,
    EKC_KEY_0, // 0x0B
    EKC_MINUS, // 0x0C
    EKC_EQUAL,
    EKC_BACKSPACE, // 0x0E
    EKC_TAB,       // 0x0F
    EKC_Q,         // 0x10
    EKC_W,
    EKC_E,
    EKC_R,
    EKC_T,
    EKC_Y,
    EKC_U,
    EKC_I,
    EKC_O,
    EKC_P,            // 0x19
    EKC_LEFT_BRACKET, // 0x1A
    EKC_RIGHT_BRACKET,
    EKC_ENTER,     // 0x1C
    EKC_LEFT_CTRL, // 0x1D
    EKC_A,         // 0x1E
    EKC_S,
    EKC_D,
    EKC_F,
    EKC_G,
    EKC_H,
    EKC_J,
    EKC_K,
    EKC_L,
    EKC_SEMICOLON, // 0x27
    EKC_APOSTROPHE,
    EKC_GRAVE,      // 0x29 `~
    EKC_LEFT_SHIFT, // 0x2A
    EKC_BACKSLASH,  // 0x2B
    EKC_Z,
    EKC_X,
    EKC_C,
    EKC_V,
    EKC_B,
    EKC_N,
    EKC_M,
    EKC_COMMA,
    EKC_DOT,
    EKC_SLASH, // 0x35
    EKC_RIGHT_SHIFT,
    EKC_KP_ASTERISK, // Numpad *
    EKC_LEFT_ALT,
    EKC_SPACE,    // 0x39
    EKC_CAPSLOCK, // 0x3A

    // You can continue with F1–F12 and navigation keys:
    EKC_F1,
    EKC_F2,
    EKC_F3,
    EKC_F4,
    EKC_F5,
    EKC_F6,
    EKC_F7,
    EKC_F8,
    EKC_F9,
    EKC_F10,
    EKC_F11,
    EKC_F12,
    EKC_SCROLLLOCK,
    EKC_NUMLOCK,
    EKC_KP_7,
    EKC_KP_8,
    EKC_KP_9,
    EKC_KP_MINUS,
    EKC_KP_4,
    EKC_KP_5,
    EKC_KP_6,
    EKC_KP_PLUS,
    EKC_KP_1,
    EKC_KP_2,
    EKC_KP_3,
    EKC_KP_0,
    EKC_KP_DOT,

    // Extended scancodes (0xE0 prefix) can be added as needed
    EKC_RIGHT_CTRL,
    EKC_RIGHT_ALT,
    EKC_ARROW_UP,
    EKC_ARROW_DOWN,
    EKC_ARROW_LEFT,
    EKC_ARROW_RIGHT,
    EKC_INSERT,
    EKC_DELETE,
    EKC_HOME,
    EKC_END,
    EKC_PAGE_UP,
    EKC_PAGE_DOWN,

    // Meta/GUI
    EKC_LEFT_GUI,
    EKC_RIGHT_GUI,
};

enum Colors
{
    EC_Black,
    EC_Blue,
    EC_Green,
    EC_Cyan,
    EC_Red,
    EC_Magenta,
    EC_Brown,
    EC_LightGray,
    EC_DarkGray,
    EC_LightBlue,
    EC_LightGreen,
    EC_LightCyan,
    EC_LightRed,
    EC_Pink,
    EC_Yellow,
    EC_White
};

struct KernelExports
{
    void (*printf)(const char *fmt, ...);

    void (*print_char)(uint8_t chr);
    void (*print_string)(uint8_t *str);
    void (*set_color)(uint8_t fg, uint8_t bg);
    void (*newline)();
    void (*clear)();
    void (*get_cursor_pos)(uint8_t *x, uint8_t *y);
    void (*set_cursor_pos)(uint8_t x, uint8_t y);

    void (*clear_key_pressed)(); // clears the key pressed flag
    uint8_t (*key_pressed)();    // returns 1 if a key is pressed
    uint8_t (*scancode)();       // gets the last scancode
    void (*set_keyboard_function)(void (*(*keyboard_function_))(uint8_t scancode));

    uint8_t (*wait_for_keypress)();
    uint8_t *(*get_line)(); // returns the pointer to input buffer (*)(which will get overwritten on next call)
    uint8_t (*keycode_to_ascii)(enum Keycode kc);
    uint8_t (*scancode_to_keycode)(uint8_t sc);

    void (*ata_read_sector)(uint32_t lba, uint8_t *buffer);
    void (*ata_write_sector)(uint32_t lba, uint8_t *buffer);

    struct IDTEntry (*make_idt_entry)(uint32_t *offset, uint16_t selector, uint8_t type_attr);

    void (*memcpy)(void *dest, void *source, uint32_t size);
};

bool isalphanum(int c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
}

bool isvalidinput(int c)
{
    return c >= '!' && c <= '~';
}

bool isbf(int c)
{
    const char valid_bf[] = "[]+-,.<>";
    for (int i = 0; i < sizeof(valid_bf); i++)
    {
        if (valid_bf[i] == c)
        {
            return true;
        }
    }
    return false;
}

enum CodeRunResult
{
    ECRR_Success,
    ECRR_MemoryOutOfBounds,
    ECRR_UnmatchedPair
};

enum CodeRunResult run_code(char *code, int len, struct KernelExports *kernel_exports)
{
    // for the sake of simplicity
    uint8_t memory[300];
    memset(memory, 0, sizeof(memory));
    uint32_t mem_pos = 0;
    int i = 0;
    for (; i < len && i >= 0; i++)
    {
        if (mem_pos > sizeof(memory))
        {
            return ECRR_MemoryOutOfBounds;
        }
        switch (code[i])
        {
        case '+':
            memory[mem_pos]++;
            break;
        case '-':
            memory[mem_pos]--;
            break;
        case '>':
            mem_pos++;
            break;
        case '<':
            mem_pos--;
            break;
        case '.':
            kernel_exports->print_char(memory[mem_pos]);
            // kernel_exports->print_char('a');
            break;
        case ',':
            kernel_exports->wait_for_keypress();
            memory[mem_pos] = kernel_exports->keycode_to_ascii((enum Keycode)kernel_exports->scancode_to_keycode(kernel_exports->scancode()));
            break;
        case '[':
            if (memory[mem_pos] == 0)
            {
                int counter = 0;
                for (i++; !(code[i] == ']' && counter <= 0) && i < len; i++)
                {
                    if (code[i] == '[')
                    {
                        counter++;
                    }
                    else if (code[i] == ']')
                    {
                        counter--;
                    }
                }
            }
            break;
        case ']':
            if (memory[mem_pos] != 0)
            {
                int counter = 0;
                for (i--; !(code[i] == '[' && counter <= 0) && i >= 0; i--)
                {
                    if (code[i] == ']')
                    {
                        counter++;
                    }
                    else if (code[i] == '[')
                    {
                        counter--;
                    }
                }
            }
            break;
        }
    }
    if (i < 0)
    {
        return ECRR_UnmatchedPair;
    }
    else
    {
        return ECRR_Success;
    }
}

enum EditorMode
{
    EEM_Editing,
    EEM_Command,
    EEM_Running,
    EEM_ViewingHelp,
    EEM_Exit
};

void write_text_to_status_bar(const char *text, uint8_t color, uint16_t *status_bar_start)
{
    while (*text != '\0')
    {
        *status_bar_start = (color << 8) | *text;
        status_bar_start++;
        text++;
    }
}

void reset_display(const char *code, struct KernelExports *kernel_exports)
{
    kernel_exports->set_cursor_pos(0, 0);
    kernel_exports->clear();
    kernel_exports->printf(code);
}

void display_help(struct KernelExports *kernel_exports)
{
    kernel_exports->clear();
    const char help_text[] =
        "  Simple brainfuck interpreter\n\
            by Sofia \"MetalPizzaCat\"\n\
        Modes:\n\
        * Editor - accessed by pressing TAB. - Allows user to edit code\n\
        * Commands - accessed by pressing ESC. - Allows user to enter commands\n\
        Commands: \n\
        * run   | r - run current code\n\
        * help  | h - display help\n\
        * new   | n - clear inputted code\n\
        * quit  | q - exit\n\
        Brainfuck commands: \n\
        * '+' - increase current cell value by one\n\
        * '-' - decrease current cell value by one\n\
        * '>' - move cell pointer right\n\
        * '<' - move cell pointer left\n\
        * '[' - if current cell is 0, move to matching ']'\n\
        * ']' - if current cell is NOT 0, move to matching '['\n\
        There are 300 cells each storing value in range of 0 to 255 \n\
        Press ESC - to exit help\n\
        ";
    kernel_exports->printf(help_text);
}

int app_main(struct KernelExports *kernel_exports)
{
    kernel_exports->clear();

    uint16_t *text_buffer = (uint16_t *)0xb8000;
    uint16_t *command_text_buffer = &text_buffer[80 * 24 + 1];

    char code_buffer[512];
    char command_buffer[50];

    memset(code_buffer, 0, sizeof(code_buffer));
    memset(command_buffer, 0, sizeof(command_buffer));

    int32_t code_text_offset = 0;
    int32_t command_text_offset = 0;

    int window_width = 80;
    int window_height = 25;

    uint32_t text_display_offset = 0;
    uint32_t command_display_offset = 0;

    enum EditorMode current_mode = EEM_Command;
    command_text_buffer[-1] = (((EC_Black << 4) | EC_Green) << 8) | ':';
    do
    {
        kernel_exports->wait_for_keypress();
        enum Keycode last_key_keycode = kernel_exports->scancode_to_keycode(kernel_exports->scancode());
        if (current_mode == EEM_ViewingHelp)
        {
            if (last_key_keycode == EKC_ESC)
            {
                current_mode = EEM_Command;
                reset_display(code_buffer, kernel_exports);
                command_text_buffer[-1] = (((EC_Black << 4) | EC_Green) << 8) | ':';
            }
            continue;
        }
        switch (last_key_keycode)
        {
        case EKC_ENTER:
            // can't be bothered dealing with line returns so all bf code is meant to be written in one line
            if (current_mode == EEM_Command)
            {
                if (strncmp(command_buffer, "quit", 50) == 0 || strncmp(command_buffer, "q", 50) == 0)
                {
                    return 0;
                }
                else if (strncmp(command_buffer, "new", 50) == 0 || strncmp(command_buffer, "n", 50) == 0)
                {
                    memset(code_buffer, 0, sizeof(code_buffer));
                    memset(text_buffer, 0, 160 * 50);
                    memset(command_buffer, 0, sizeof(command_buffer));
                    text_display_offset = 0;
                    command_display_offset = 0;
                    code_text_offset = 0;
                    command_text_offset = 0;
                    current_mode = EEM_Editing;
                }
                else if (strncmp(command_buffer, "help", 50) == 0 || strncmp(command_buffer, "h", 50) == 0)
                {
                    current_mode = EEM_ViewingHelp;
                    display_help(kernel_exports);
                }
                else if (strncmp(command_buffer, "run", 50) == 0 || strncmp(command_buffer, "r", 50) == 0)
                {
                    kernel_exports->clear();
                    current_mode = EEM_Running;
                    enum CodeRunResult res = run_code(code_buffer, sizeof(code_buffer), kernel_exports);
                    switch (res)
                    {
                    case ECRR_Success:
                        write_text_to_status_bar("Finished! TAB to edit code", ((EC_Green) << 4 | EC_White), command_text_buffer);
                        break;
                    case ECRR_MemoryOutOfBounds:
                        write_text_to_status_bar("Pointer out of bounds! TAB to edit code", ((EC_Red) << 4 | EC_White), command_text_buffer);
                        break;
                    case ECRR_UnmatchedPair:
                        write_text_to_status_bar("Unmatched brackets! TAB to edit code", ((EC_Red) << 4 | EC_White), command_text_buffer);
                        break;
                    }
                }
            }
            break;
        case EKC_BACKSPACE:

            if (current_mode == EEM_Editing)
            {

                if (code_text_offset > 0)
                {
                    text_display_offset--;
                    code_text_offset--;
                    code_buffer[code_text_offset] = 0;
                    text_buffer[text_display_offset] = 0;
                }
            }
            else if (current_mode == EEM_Command)
            {
                if (command_text_offset > 0)
                {
                    command_display_offset--;
                    command_text_offset--;
                    command_buffer[command_text_offset] = 0;
                    command_text_buffer[command_display_offset] = (((EC_Black << 4) | EC_Green) << 8);
                }
            }
            break;
        case EKC_TAB:
            if (current_mode == EEM_Running)
            {
                reset_display(code_buffer, kernel_exports);
            }
            current_mode = EEM_Editing;
            command_text_buffer[-1] = (((EC_Black << 4) | EC_Green) << 8) | 0;
            break;
        case EKC_ESC:
            if (current_mode == EEM_Running)
            {
                reset_display(code_buffer, kernel_exports);
            }
            current_mode = EEM_Command;
            command_text_buffer[-1] = (((EC_Black << 4) | EC_Green) << 8) | ':';
            break;
        default:
        {
            if (current_mode == EEM_Editing)
            {
                uint8_t ch = kernel_exports->keycode_to_ascii(last_key_keycode);
                if ((isalphanum(ch) || isvalidinput(ch) || ch == ' ') && code_text_offset < sizeof(code_buffer))
                {
                    code_buffer[code_text_offset] = ch;
                    text_buffer[text_display_offset] = (((EC_Black << 4) | EC_White) << 8) | ch;
                    text_display_offset++;
                    code_text_offset++;
                }
            }
            else if (current_mode == EEM_Command)
            {
                uint8_t ch = kernel_exports->keycode_to_ascii(last_key_keycode);
                if (isalphanum(ch) && command_text_offset < sizeof(command_buffer))
                {
                    command_buffer[command_text_offset] = ch;
                    command_text_buffer[command_display_offset] = (((EC_Black << 4) | EC_Green) << 8) | ch;
                    command_display_offset++;
                    command_text_offset++;
                }
            }
        }
        }

    } while (current_mode != EEM_Exit);
    return 0;
}