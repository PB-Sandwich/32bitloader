#include <stdint.h>
#include <stdbool.h>
#include <endian.h>
#include <string.h>
#include <estros.h>
#include <stdlib.h>

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
    ECRR_UnmatchedPair,
    ECRR_ForcedExit
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
        if (kernel_exports->key_pressed() && kernel_exports->scancode_to_keycode(kernel_exports->scancode()) == EKC_ESC)
        {
            return ECRR_ForcedExit;
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

int main(struct KernelExports *kernel_exports)
{
    kernel_exports->clear();

    uint16_t *text_buffer = (uint16_t *)0xb8000;
    uint16_t *command_text_buffer = &text_buffer[80 * 24 + 1];

    char code_buffer[512];
    char command_buffer[50];

    memset(code_buffer, 0, sizeof(code_buffer));
    memset(command_buffer, 0, sizeof(command_buffer));

    int32_t code_len = 0;
    int32_t code_text_offset = 0;
    int32_t command_text_offset = 0;

    int32_t window_width = 80;
    int32_t window_height = 25;

    int32_t cursor_pos = 0;
    int32_t prev_cursor_pos = 0;

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
        case 0x4b:
            if (current_mode == EEM_Editing)
            {
                text_buffer[code_text_offset] = (text_buffer[code_text_offset] & 0x00FF) | (((EC_Black << 4) | EC_White) << 8);
                code_text_offset = MAX(0, code_text_offset - 1);
                text_buffer[code_text_offset] = (text_buffer[code_text_offset] & 0x00FF) | (((EC_Blue << 4) | EC_White) << 8);
            }
            break;
        case 0x4d:
            if (current_mode == EEM_Editing)
            {
                text_buffer[code_text_offset] = (text_buffer[code_text_offset] & 0x00FF) | (((EC_Black << 4) | EC_White) << 8);
                code_text_offset = MIN(code_len, code_text_offset + 1);
                text_buffer[code_text_offset] = (text_buffer[code_text_offset] & 0x00FF) | (((EC_Blue << 4) | EC_White) << 8);
                if(text_buffer[code_text_offset] & 0xFF00 == 0)
                {
                    text_buffer[code_text_offset] = ' ' | (((EC_Blue << 4) | EC_White) << 8);
                }
            }
            break;
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
                    case ECRR_ForcedExit:
                        write_text_to_status_bar("Execution interrupted! TAB to edit code", ((EC_Yellow) << 4 | EC_Black), command_text_buffer);
                        break;
                    }
                }
            }
            break;
        case EKC_BACKSPACE:

            if (current_mode == EEM_Editing)
            {

                if (code_text_offset >= 0 && code_len > 0)
                {
                    text_buffer[code_text_offset] = (text_buffer[code_text_offset] & 0x00FF) | (((EC_Black << 4) | EC_White) << 8);
                    code_len--;
                    code_text_offset--;
                    if (code_text_offset < code_len)
                    {
                        code_buffer[code_len] = 0;
                        text_buffer[code_len] = 0;
                        for (int i = code_text_offset + 1; i < code_len; i++)
                        {
                            code_buffer[i] = code_buffer[i + 1];
                            text_buffer[i] = text_buffer[i + 1];
                        }
                    }
                    else
                    {
                        code_buffer[code_text_offset] = 0;
                        text_buffer[code_text_offset] = 0;
                    }
                    code_text_offset = MAX(0, code_text_offset);
                    text_buffer[code_text_offset] = (text_buffer[code_text_offset] & 0x00FF) | (((EC_Blue << 4) | EC_White) << 8);
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
                if (!((isalphanum(ch) || isvalidinput(ch) || ch == ' ') && code_len < sizeof(code_buffer)))
                {
                    break;
                }

                if (code_text_offset == code_len)
                {
                    code_buffer[code_text_offset] = ch;
                    text_buffer[code_text_offset] = (((EC_Black << 4) | EC_White) << 8) | ch;
                    code_text_offset++;
                    code_len++;
                    text_buffer[code_text_offset] = (((EC_Black << 4) | EC_White) << 8) | ' ';
                    break;
                }
                for (int i = code_len; i > code_text_offset && i > 0; i--)
                {
                    code_buffer[i] = code_buffer[i - 1];
                    text_buffer[i] = text_buffer[i - 1];
                }
                code_len++;
                code_buffer[code_text_offset] = ch;
                text_buffer[code_text_offset] = (((EC_Black << 4) | EC_White) << 8) | ch;
                code_text_offset++;
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