#include <stdint.h>
#include <stdbool.h>
#include <endian.h>
#include <string.h>
#include <estros.h>
#include <stdlib.h>
#include <ctype.h>

#include <estros/keyboard.h>
#include <estros/file.h>

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
    ECRR_Unfinished,
    ECRR_Success,
    ECRR_MemoryOutOfBounds,
    ECRR_UnmatchedPair,
    ECRR_ForcedExit,
    ECRR_Invalid,
    ECRR_ExpectInput
};

enum EditorMode
{
    EEM_Editing,
    EEM_Command,
    EEM_Running,
    EEM_RunningWaitingForInput,
    EEM_RunningFinished,
    EEM_ViewingHelp,
    EEM_ErrorMessage,
    EEM_Exit
};

typedef struct
{
    enum Keycode keycode;
    bool shift_pressed;
} Input;

typedef struct
{
    uint8_t memory[300];
    uint32_t mem_pos;
    char *code;
    int code_len;
    int current_code_offset;
} CodeExecutionData;

void init_exec_data(CodeExecutionData *exec, char *code, int code_len)
{
    memset(exec, 0, sizeof(exec->memory));
    exec->code_len = code_len;
    exec->code = code;
    exec->current_code_offset = 0;
    exec->mem_pos = 0;
}

typedef struct
{
    File *kdb;
    enum EditorMode current_mode;
    char code_buffer[512];
    char command_buffer[50];
    uint16_t *text_buffer;
    uint16_t *command_text_buffer;
    int32_t code_len;
    int32_t code_text_offset;
    int32_t command_text_offset;
    int32_t window_width;
    int32_t window_height;
    int32_t cursor_pos;
    int32_t prev_cursor_pos;
    uint32_t command_display_offset;
    bool shift_state;
    CodeExecutionData exec_data;
} AppState;

enum CodeRunResult step_code(CodeExecutionData *data, bool has_input, char input)
{
    if (data == NULL)
    {
        return ECRR_Invalid;
    }
    if (data->current_code_offset < 0)
    {
        return ECRR_UnmatchedPair;
    }
    if (data->current_code_offset >= data->code_len)
    {
        return ECRR_Success;
    }
    if (data->mem_pos > sizeof(data->memory))
    {
        return ECRR_MemoryOutOfBounds;
    }
    switch (data->code[data->current_code_offset])
    {
    case '+':
        data->memory[data->mem_pos]++;
        break;
    case '-':
        data->memory[data->mem_pos]--;
        break;
    case '>':
        data->mem_pos++;
        break;
    case '<':
        data->mem_pos--;
        break;
    case '.':
        sys_print(&data->memory[data->mem_pos], 1);
        break;
    case ',':
        if (!has_input)
        {
            return ECRR_ExpectInput;
        }
        data->memory[data->mem_pos] = input; // keyboard_wait_for_ascii_press();
        break;
    case '[':
        if (data->memory[data->mem_pos] == 0)
        {
            int counter = 0;
            for (data->current_code_offset++;
                 !(data->code[data->current_code_offset] == ']' && counter <= 0) && data->current_code_offset < data->code_len;
                 data->current_code_offset++)
            {
                if (data->code[data->current_code_offset] >= data->code_len)
                {
                    return ECRR_UnmatchedPair;
                }
                if (data->code[data->current_code_offset] == '[')
                {
                    counter++;
                }
                else if (data->code[data->current_code_offset] == ']')
                {
                    counter--;
                }
            }
        }
        break;
    case ']':
        if (data->memory[data->mem_pos] != 0)
        {
            int counter = 0;
            for (data->current_code_offset--;
                 !(data->code[data->current_code_offset] == '[' && counter <= 0) && data->current_code_offset >= 0;
                 data->current_code_offset--)
            {
                if (data->code[data->current_code_offset] < 0)
                {
                    return ECRR_UnmatchedPair;
                }
                if (data->code[data->current_code_offset] == ']')
                {
                    counter++;
                }
                else if (data->code[data->current_code_offset] == '[')
                {
                    counter--;
                }
            }
        }
        break;
    }
    data->current_code_offset++;
    return ECRR_Unfinished;
}

void write_text_to_status_bar(const char *text, uint8_t color, uint16_t *status_bar_start)
{
    while (*text != '\0')
    {
        *status_bar_start = (color << 8) | *text;
        status_bar_start++;
        text++;
    }
}

void reset_display(const char *code)
{
    //  kernel_exports->set_cursor_pos(0, 0);
    sys_set_cursor(0, 0);
    sys_clear();
    sys_print(code, strlen(code));
}

void prepare_for_exec()
{
    sys_set_cursor(0, 0);
    sys_clear();
}

void display_help()
{
    sys_clear();
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
        * ',' - accept one byte of input\n\
        * '.' - print value of current memory cell as ASII\n\
        There are 300 cells each storing value in range of 0 to 255 \n\
        Press ESC - to exit help\n\
        ";
    sys_print(help_text, sizeof(help_text));
}

void editor_mode(AppState *app, Input input)
{
    enum Keycode last_key_keycode = input.keycode;
    switch (last_key_keycode)
    {
    case 0x4b:
        app->text_buffer[app->code_text_offset] = (app->text_buffer[app->code_text_offset] & 0x00FF) | (((EC_Black << 4) | EC_White) << 8);
        app->code_text_offset = MAX(0, app->code_text_offset - 1);
        app->text_buffer[app->code_text_offset] = (app->text_buffer[app->code_text_offset] & 0x00FF) | (((EC_Blue << 4) | EC_White) << 8);

        break;
    case 0x4d:
        app->text_buffer[app->code_text_offset] = (app->text_buffer[app->code_text_offset] & 0x00FF) | (((EC_Black << 4) | EC_White) << 8);
        app->code_text_offset = MIN(app->code_len, app->code_text_offset + 1);
        app->text_buffer[app->code_text_offset] = (app->text_buffer[app->code_text_offset] & 0x00FF) | (((EC_Blue << 4) | EC_White) << 8);
        if (app->text_buffer[app->code_text_offset] & 0xFF00 == 0)
        {
            app->text_buffer[app->code_text_offset] = ' ' | (((EC_Blue << 4) | EC_White) << 8);
        }

        break;
    case KC_BACKSPACE:

        if (app->code_text_offset >= 0 && app->code_len > 0)
        {
            app->text_buffer[app->code_text_offset] = (app->text_buffer[app->code_text_offset] & 0x00FF) | (((EC_Black << 4) | EC_White) << 8);
            app->code_len--;
            app->code_text_offset--;
            if (app->code_text_offset < app->code_len)
            {
                app->code_buffer[app->code_len] = 0;
                app->text_buffer[app->code_len] = 0;
                for (int i = app->code_text_offset + 1; i < app->code_len; i++)
                {
                    app->code_buffer[i] = app->code_buffer[i + 1];
                    app->text_buffer[i] = app->text_buffer[i + 1];
                }
            }
            else
            {
                app->code_buffer[app->code_text_offset] = 0;
                app->text_buffer[app->code_text_offset] = 0;
            }
            app->code_text_offset = MAX(0, app->code_text_offset);
            app->text_buffer[app->code_text_offset] = (app->text_buffer[app->code_text_offset] & 0x00FF) | (((EC_Blue << 4) | EC_White) << 8);
        }

        break;
    case KC_ESC:
        app->current_mode = EEM_Command;
        app->command_text_buffer[-1] = (((EC_Black << 4) | EC_Green) << 8) | ':';
        return;
    default:
    {
        uint8_t ch = keycode_to_ascii(last_key_keycode, app->shift_state);
        if (!((isalnum(ch) || isvalidinput(ch) || ch == ' ') && app->code_len < sizeof(app->code_buffer)))
        {
            break;
        }

        if (app->code_text_offset == app->code_len)
        {
            app->code_buffer[app->code_text_offset] = ch;
            app->text_buffer[app->code_text_offset] = (((EC_Black << 4) | EC_White) << 8) | ch;
            app->code_text_offset++;
            app->code_len++;
            app->text_buffer[app->code_text_offset] = (((EC_Black << 4) | EC_White) << 8) | ' ';
            break;
        }
        for (int i = app->code_len; i > app->code_text_offset && i > 0; i--)
        {
            app->code_buffer[i] = app->code_buffer[i - 1];
            app->text_buffer[i] = app->text_buffer[i - 1];
        }
        app->code_len++;
        app->code_buffer[app->code_text_offset] = ch;
        app->text_buffer[app->code_text_offset] = (((EC_Black << 4) | EC_White) << 8) | ch;
        app->code_text_offset++;
    }
    }
}

void command_mode(AppState *app, Input input)
{

    for (int i = 0; i < sizeof(app->command_buffer); i++)
    {
        app->command_text_buffer[i] = (((EC_Black << 4) | EC_Green) << 8) | app->command_buffer[i];
    }
    enum Keycode last_key_keycode = input.keycode;
    switch (last_key_keycode)
    {
    case KC_ENTER:
        if (strncmp(app->command_buffer, "quit", 50) == 0 || strncmp(app->command_buffer, "q", 50) == 0)
        {
            app->current_mode = EEM_Exit;
            return;
        }
        else if (strncmp(app->command_buffer, "new", 50) == 0 || strncmp(app->command_buffer, "n", 50) == 0)
        {
            memset(app->code_buffer, 0, sizeof(app->code_buffer));
            memset(app->text_buffer, 0, 160 * 50);
            memset(app->command_buffer, 0, sizeof(app->command_buffer));
            app->command_display_offset = 0;
            app->code_text_offset = 0;
            app->command_text_offset = 0;
            app->current_mode = EEM_Editing;
            return;
        }
        else if (strncmp(app->command_buffer, "help", 50) == 0 || strncmp(app->command_buffer, "h", 50) == 0)
        {
            app->current_mode = EEM_ViewingHelp;
            return;
        }
        else if (strncmp(app->command_buffer, "run", 50) == 0 || strncmp(app->command_buffer, "r", 50) == 0)
        {
            app->current_mode = EEM_Running;
            prepare_for_exec();
            init_exec_data(&app->exec_data, app->code_buffer, sizeof(app->code_buffer));
            return;
        }
        // check if first 2 characters are write operation command
        else if (strncmp(app->command_buffer, "w ", 2) == 0)
        {
            const char *command = app->command_buffer + 1;
            // skip whitespace
            for (; *command == ' ' && command - app->command_buffer < 50; command++)
                ;
            File *dest_file = open_file(command, ESTROS_WRITE);
            if (dest_file == NULL)
            {
                if (create_file(command) != 0)
                {
                    write_text_to_status_bar("Failed to open file! ESC to continue", ((EC_Red) << 4 | EC_Black), app->command_text_buffer);
                    app->current_mode = EEM_ErrorMessage;
                    return;
                }
                else
                {
                    dest_file = open_file(command, ESTROS_WRITE);
                }
            }
            write_file(dest_file, app->code_buffer, strnlen(app->code_buffer, sizeof(app->code_buffer)));
            write_text_to_status_bar("Saved code! ESC to continue", ((EC_Green) << 4 | EC_Black), app->command_text_buffer);
            app->current_mode = EEM_ErrorMessage;
        }
        else if (strncmp(app->command_buffer, "l ", 2) == 0)
        {
            const char *command = app->command_buffer + 1;
            // skip whitespace
            for (; *command == ' ' && command - app->command_buffer < 50; command++)
                ;
            File *dest_file = open_file(command, ESTROS_READ);
            if (dest_file == NULL)
            {
                write_text_to_status_bar("Failed to read file! ESC to continue", ((EC_Red) << 4 | EC_Black), app->command_text_buffer);
                app->current_mode = EEM_ErrorMessage;
                return;
            }

            memset(app->code_buffer, 0, sizeof(app->code_buffer));
            memset(app->command_buffer, 0, sizeof(app->command_buffer));
            app->command_display_offset = 0;
            app->command_text_offset = 0;
            app->current_mode = EEM_Editing;

            read_file(dest_file, app->code_buffer, sizeof(app->code_buffer));
            app->code_text_offset = strlen(app->code_buffer);
            app->code_len = strlen(app->code_buffer);
            reset_display(app->code_buffer);
            return;
        }
        else
        {
            write_text_to_status_bar("Unknown command! ESC to continue", ((EC_Red) << 4 | EC_Black), app->command_text_buffer);
            app->current_mode = EEM_ErrorMessage;
            return;
        }
        break;
    case KC_BACKSPACE:

        if (app->command_text_offset > 0)
        {
            app->command_display_offset--;
            app->command_text_offset--;
            app->command_buffer[app->command_text_offset] = 0;
            app->command_text_buffer[app->command_display_offset] = (((EC_Black << 4) | EC_Green) << 8);
        }

        break;
    case KC_TAB:
        app->current_mode = EEM_Editing;
        app->command_text_buffer[-1] = (((EC_Black << 4) | EC_Green) << 8) | 0;
        return;
    default:
    {

        uint8_t ch = keycode_to_ascii(last_key_keycode, input.shift_pressed); // kernel_exports->keycode_to_ascii(last_key_keycode);
        if ((isalnum(ch) || ch == '\\' || ch == '/' || ch == ' ' || ch == '.') && app->command_text_offset < sizeof(app->command_buffer))
        {
            app->command_buffer[app->command_text_offset] = ch;
            app->command_text_buffer[app->command_display_offset] = (((EC_Black << 4) | EC_Green) << 8) | ch;
            app->command_display_offset++;
            app->command_text_offset++;
        }
    }
    }
}

void run_mode(AppState *app, bool has_input, Input input)
{
    if (app->current_mode != EEM_Running && app->current_mode != EEM_RunningWaitingForInput)
    {
        return;
    }
    if (has_input && input.keycode == KC_ESC)
    {
        write_text_to_status_bar("Execution interrupted! TAB to edit code", ((EC_Yellow) << 4 | EC_Black), app->command_text_buffer);
        app->current_mode = EEM_RunningFinished;
        return;
    }
    // sys_clear();
    enum CodeRunResult res = step_code(&app->exec_data, has_input, keycode_to_ascii(input.keycode, input.shift_pressed));
    switch (res)
    {
    case ECRR_Unfinished:
        app->current_mode = EEM_Running;
        break;
    case ECRR_ExpectInput:
        app->current_mode = EEM_RunningWaitingForInput;
        break;
    case ECRR_Success:
        write_text_to_status_bar("Finished! TAB to edit code", ((EC_Green) << 4 | EC_White), app->command_text_buffer);
        app->current_mode = EEM_RunningFinished;
        break;
    case ECRR_MemoryOutOfBounds:
        write_text_to_status_bar("Pointer out of bounds! TAB to edit code", ((EC_Red) << 4 | EC_White), app->command_text_buffer);
        app->current_mode = EEM_RunningFinished;
        break;
    case ECRR_UnmatchedPair:
        write_text_to_status_bar("Unmatched brackets! TAB to edit code", ((EC_Red) << 4 | EC_White), app->command_text_buffer);
        app->current_mode = EEM_RunningFinished;
        break;
    case ECRR_Invalid:
        write_text_to_status_bar("Invalid app situation occurred! PANIC! TAB to edit code", ((EC_Yellow) << 4 | EC_Black), app->command_text_buffer);
        app->current_mode = EEM_RunningFinished;
        break;
    }
}

void help_mode(AppState *app, Input input)
{
    display_help();
    if (input.keycode == KC_ESC)
    {
        app->current_mode = EEM_Command;
        reset_display(app->code_buffer);
        app->command_text_buffer[-1] = (((EC_Black << 4) | EC_Green) << 8) | ':';
    }
}

int main()
{
    AppState app;
    app.kdb = open_file("/dev/kdb", ESTROS_READ);
    memset(app.code_buffer, 0, sizeof(app.code_buffer));
    memset(app.command_buffer, 0, sizeof(app.command_buffer));
    app.text_buffer = get_text_buffer_address();
    app.command_text_buffer = &app.text_buffer[80 * 24 + 1];
    app.current_mode = EEM_Command;

    app.code_len = 0;
    app.code_text_offset = 0;
    app.command_text_offset = 0;

    app.window_width = 80;
    app.window_height = 25;

    app.cursor_pos = 0;
    app.prev_cursor_pos = 0;

    app.command_display_offset = 0;
    app.shift_state = false;

    sys_clear();
    app.command_text_buffer[-1] = (((EC_Black << 4) | EC_Green) << 8) | ':';
    while (app.current_mode != EEM_Exit)
    {
        if (app.current_mode == EEM_Running)
        {
            KeyboardEvent event;
            Input input;
            uint32_t in_len = read_file(app.kdb, &event, sizeof(KeyboardEvent));
            if (in_len > 0)
            {
                input.keycode = scancode_to_keycode(event.scancode);
                input.shift_pressed = app.shift_state;
            }
            run_mode(&app, in_len > 0 && event.type == KEY_PRESSED, input);
        }
        else
        {
            if (app.current_mode == EEM_ViewingHelp)
            {
                display_help();
            }
            else if (app.current_mode == EEM_Command)
            {
                for (int i = 0; i < sizeof(app.command_buffer); i++)
                {
                    app.command_text_buffer[i] = (((EC_Black << 4) | EC_Green) << 8) | app.command_buffer[i];
                }
            }

            KeyboardEvent event;
            while (!read_file(app.kdb, &event, sizeof(KeyboardEvent)))
                ;
            enum Keycode code = scancode_to_keycode(event.scancode);
            if (code == KC_LEFT_SHIFT || code == KC_RIGHT_SHIFT)
            {
                app.shift_state = event.type == KEY_PRESSED;
            }
            if (event.type == KEY_RELEASED)
            {
                continue;
            }
            Input input;
            input.keycode = code;
            input.shift_pressed = app.shift_state;
            switch (app.current_mode)
            {
            case EEM_ViewingHelp:
                help_mode(&app, input);
                break;
            case EEM_ErrorMessage:
                if (input.keycode == KC_ESC)
                {
                    app.current_mode = EEM_Command;
                    reset_display(app.code_buffer);
                }
                break;
            case EEM_RunningFinished:
                if (input.keycode == KC_TAB)
                {
                    app.current_mode = EEM_Editing;
                    reset_display(app.code_buffer);
                }
                break;
            case EEM_RunningWaitingForInput:
                run_mode(&app, true, input);
                break;
            case EEM_Editing:
                editor_mode(&app, input);
                break;
            case EEM_Command:
                command_mode(&app, input);
                break;
            }
        }
    }
    return 0;
}
