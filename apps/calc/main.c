#include <stdint.h>
#include <stdbool.h>
#include <endian.h>
#include <string.h>
#include <estros.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <format.h>
#include <errno.h>
#include <stdio.h>

struct KernelExports *exports;

// This way it can be swapped out with any type once logic for that type is added
typedef int32_t number_t;

number_t parse_number(char *buffer)
{
    return atoi(buffer);
}

enum CalcExprType
{
    ECET_Constant,
    ECET_Operator,
    ECET_BracketOpen,
    ECET_BracketClose
};

enum CalcOperationType
{
    ECOT_Add,
    ECOT_Sub,
    ECOT_Mul,
    ECOT_Div,
    ECOT_Mod
};

union CalcExpressionValue
{
    number_t constant;
    enum CalcOperationType operation;
};

struct CalcExpression
{
    enum CalcExprType type;
    union CalcExpressionValue value;
};

typedef struct CalcExpression CalcExpression;

CalcExpression make_expr_bracket_open()
{
    CalcExpression expr;
    expr.type = ECET_BracketOpen;
    return expr;
}

CalcExpression make_expr_bracket_close()
{
    CalcExpression expr;
    expr.type = ECET_BracketClose;
    return expr;
}

CalcExpression make_expr_op(enum CalcOperationType op)
{
    CalcExpression expr;
    expr.type = ECET_Operator;
    expr.value.operation = op;
    return expr;
}

/// @brief Create an operator expression from a character
/// @param c Operator character
/// @param success Value to write the result to, if character does no map to a valid operator this will contain "false"
/// @return Expression
CalcExpression make_expr_op_from_char(char c, bool *success)
{
    CalcExpression expr;
    expr.type = ECET_Operator;
    switch (c)
    {
    case '+':
        expr.value.operation = ECOT_Add;
        break;
    case '-':
        expr.value.operation = ECOT_Sub;
        break;
    case '*':
        expr.value.operation = ECOT_Mul;
        break;
    case '/':
        expr.value.operation = ECOT_Div;
        break;
    case '%':
        expr.value.operation = ECOT_Mod;
        break;
    default:
        *success = false;
        return expr;
    }
    *success = true;
    return expr;
}

/// @brief Create an expression from a constant vavlue
/// @param val Value
/// @return Expression
CalcExpression make_expr_const(number_t val)
{
    CalcExpression expr;
    expr.type = ECET_Constant;
    expr.value.constant = val;
    return expr;
}

/// @brief Get precedence for the given operator
/// @param op Character representation of the operator
/// @return Precedence
int32_t get_operator_precedence(char op)
{
    switch (op)
    {
    case '^':
        return 3;
    case '/':
    case '*':
    case '%':
        return 2;
    case '+':
    case '-':

        return 1;
    default:
        return -1;
    }
}

/// @brief Get operator precedence for an expression. Returns -1 if expression is not an operator
/// @param expr Expression
/// @return Precedence
int32_t calc_expr_get_operator_precedence(CalcExpression *expr)
{
    if (expr->type != ECET_Operator)
    {
        return -1;
    }
    switch (expr->value.operation)
    {
    case ECOT_Add:
    case ECOT_Sub:
        return 1;
    case ECOT_Mul:
    case ECOT_Div:
    case ECOT_Mod:
        return 2;

    default:
        return -1;
    }
}

/// @brief Convert operator to character representation
/// @param  op Operation
/// @return
char operator_to_char(enum CalcOperationType op)
{
    switch (op)
    {
    case ECOT_Add:
        return '+';
    case ECOT_Sub:
        return '-';
    case ECOT_Mul:
        return '*';
    case ECOT_Div:
        return '/';
    case ECOT_Mod:
        return '%';
    default:
        return '?';
    }
}

struct NumberStack
{
    number_t stack[50];
    int32_t offset;
};

typedef struct NumberStack NumberStack;

void number_stack_init(NumberStack *stack)
{
    stack->offset = 0;
    memset(stack->stack, 0, sizeof(int32_t) * 50);
}
number_t number_stack_pop(NumberStack *stack, bool *success)
{
    if (stack->offset == 0)
    {
        *success = false;
        return 0;
    }
    *success = true;
    return stack->stack[--stack->offset];
}

void number_stack_push(NumberStack *stack, number_t val)
{
    stack->stack[stack->offset++] = val;
}

/// @brief Parse math expression input into an array of operators that can then be executed
/// @param input_buffer Buffer containing the expression text
/// @param input_buffer_len Length of the expression text buffer
/// @param error_message Pointer which will store the error message, if any error occurs
/// @param result Array containing the resulting set of operators. Has to be big enough to contain all values
/// @param result_buffer_max_len Max size of the array of operations
/// @param result_len The amount of operators actually stored in the array
/// @return True if no error has occurred, false otherwise
bool parse_input(const char *input_buffer, int32_t input_buffer_len, char **error_message, CalcExpression *result, int32_t result_buffer_max_len, int32_t *result_len)
{
    // buffer used for parsing numbers
    char number_buffer[50];
    CalcExpression expression_stack[100];

    int32_t result_array_offset = 0;
    int32_t expression_stack_offset = -1;

    int32_t input_buffer_pos = 0;
    for (; input_buffer_pos < input_buffer_len && input_buffer[input_buffer_pos] != '\0'; input_buffer_pos++)
    {
        if (result_array_offset >= result_buffer_max_len)
        {
            *error_message = "Result buffer is not large enough";
            return false;
        }
        if (expression_stack_offset >= 100)
        {
            *error_message = "Expression stack is not large enough";
            return false;
        }
        char ch = input_buffer[input_buffer_pos];
        if (ch == ' ')
        {
            continue;
        }
        if (isdigit(ch))
        {
            bool had_decimal_point = false;
            memset(number_buffer, 0, sizeof(number_buffer));
            for (int32_t number_buffer_offset = 0;
                 input_buffer_pos < input_buffer_len &&
                 input_buffer[input_buffer_pos] != '\0' &&
                 (isdigit(input_buffer[input_buffer_pos]) || input_buffer[input_buffer_pos] == '.');
                 input_buffer_pos++)
            {
                if (input_buffer[input_buffer_pos] == '.')
                {
                    // temporary until floats are added
                    if (had_decimal_point || true)
                    {
                        *error_message = "Invalid number";
                        return false;
                    }
                    number_buffer[number_buffer_offset++] = '.';
                    had_decimal_point = true;
                }
                else
                {
                    number_buffer[number_buffer_offset++] = input_buffer[input_buffer_pos];
                }
            }
            result[result_array_offset++] = make_expr_const(parse_number(number_buffer));
            input_buffer_pos--;
        }
        else if (ch == '(')
        {
            expression_stack[++expression_stack_offset] = make_expr_bracket_open();
        }
        else if (ch == ')')
        {
            while (expression_stack_offset >= 0 && expression_stack[expression_stack_offset].type != ECET_BracketOpen)
            {
                result[result_array_offset++] = expression_stack[expression_stack_offset--];
            }
            // remove bracket from the stack
            expression_stack_offset = expression_stack_offset - 1;
        }
        else
        {
            while (expression_stack_offset >= 0 &&
                   (get_operator_precedence(ch) <= calc_expr_get_operator_precedence(&expression_stack[expression_stack_offset])))
            {
                result[result_array_offset++] = expression_stack[expression_stack_offset--];
            }
            bool valid_expr = false;
            expression_stack[++expression_stack_offset] = make_expr_op_from_char(ch, &valid_expr);
            if (!valid_expr)
            {
                *error_message = "Invalid character encountered";
                return false;
            }
        }
    }
    while (expression_stack_offset >= 0)
    {
        result[result_array_offset++] = expression_stack[expression_stack_offset--];
    }
    *result_len = result_array_offset;
    return true;
}

bool execute(CalcExpression *ops, int32_t ops_len, number_t *result, char **error_message)
{
    NumberStack stack;
    number_stack_init(&stack);
    for (int i = 0; i < ops_len; i++)
    {
        switch (ops[i].type)
        {
        case ECET_Constant:
            number_stack_push(&stack, ops[i].value.constant);
            break;
        case ECET_Operator:
        {
            bool value_grab_result = false;

            number_t b = number_stack_pop(&stack, &value_grab_result);
            if (!value_grab_result)
            {
                *error_message = "Not enough values on stack";
                return false;
            }
            number_t a = number_stack_pop(&stack, &value_grab_result);
            if (!value_grab_result)
            {
                *error_message = "Not enough values on stack";
                return false;
            }
            switch (ops[i].value.operation)
            {
            case ECOT_Add:
                number_stack_push(&stack, a + b);
                break;
            case ECOT_Sub:
                number_stack_push(&stack, a - b);
                break;
            case ECOT_Mul:
                number_stack_push(&stack, a * b);
                break;
            case ECOT_Div:
                number_stack_push(&stack, a / b);
                break;
            case ECOT_Mod:
                number_stack_push(&stack, a % b);
                break;
            }
        }
        break;
        }
    }
    bool success = false;
    *result = number_stack_pop(&stack, &success);
    return success;
}

typedef struct
{
    bool shift_pressed;
    bool alt_pressed;
    bool ctrl_pressed;
    bool pressed;
    KernelKeycode keycode;
} InputEventData;

typedef struct
{
    bool shift_state;
    bool alt_state;
    bool ctrl_state;
} InputMapState;

typedef struct
{
    uint16_t *text_buffer;
    int32_t width;
    int32_t height;
} ScreenData;

/// @brief Clear screen with a given color
/// @param screen Screen data
/// @param clear_color What color to clean the screen to
void gob_term_clear(ScreenData *screen, KernelTerminalColors clear_color)
{
    for (int32_t i = 0; i < screen->width * screen->height; i++)
    {
        screen->text_buffer[i] = ((clear_color << 4) | clear_color) << 8;
    }
}

/// @brief Draw a line of characters from given coors to the end of the screen going right or until the end of width
/// @param screen Screen data
/// @param x
/// @param y
/// @param width Width of the line
/// @param ch Character to write
/// @param bg_color Color of the character
/// @param fg_color Color of the background
void gob_term_line(ScreenData *screen, int32_t x, int32_t y, int32_t width, char ch, KernelTerminalColors bg_color, KernelTerminalColors fg_color)
{
    uint16_t *start = screen->text_buffer + x + y * screen->width;
    for (int32_t i = 0; i + x < screen->width && i < width; i++)
    {
        start[i] = (((bg_color << 4) | fg_color) << 8) | ch;
    }
}

void gob_term_text(ScreenData *screen, int32_t x, int32_t y, const char *text, size_t len, KernelTerminalColors bg_color, KernelTerminalColors fg_color)
{
    uint16_t *start = screen->text_buffer + x + y * screen->width;
    for (int32_t i = 0; i < len && x + i < screen->width; i++)
    {
        start[i] = (((bg_color << 4) | fg_color) << 8) | text[i];
    }
}

/// @brief Display text at x,y and pad or cutoff text if it doesn't fit the size. Will not go past the screen
/// @param screen
/// @param x
/// @param y
/// @param text Text to display
/// @param text_len Length of the text
/// @param len Fixed length of the text strip
/// @param padding_character Character to put at the end if text is too short
/// @param bg_color
/// @param fg_color
void gob_term_text_fixed(ScreenData *screen,
                         int32_t x,
                         int32_t y,
                         const char *text,
                         size_t text_len,
                         size_t len,
                         char padding_character,
                         KernelTerminalColors bg_color,
                         KernelTerminalColors fg_color)
{
    uint16_t *start = screen->text_buffer + x + y * screen->width;
    int32_t i = 0;
    for (; i < text_len && x + i < screen->width && i < len; i++)
    {
        start[i] = (((bg_color << 4) | fg_color) << 8) | text[i];
    }
    for (; i < len && x + i < screen->width; i++)
    {
        start[i] = (((bg_color << 4) | fg_color) << 8) | padding_character;
    }
}

/// @brief Doubly linked list of all TUI elements in the application
struct GobElement
{
    struct GobElement *prev;
    struct GobElement *next;
    struct GobApp *app;
    /// @brief Element specific data used for the specific logic. Using void* is awkward but i can't be bothered to implement c++
    void *internal;

    bool is_focused;
    bool can_be_focused;
    int32_t x;
    int32_t y;
    bool has_shadow;
    /// @brief Function used to render this element
    void (*render)(struct GobElement *);
    /// @brief function for handling input by the element
    void (*handle_input)(struct GobElement *, InputEventData input);
    /// @brief Called when element receives or looses focus using the focus control keys
    void (*focus_changed)(struct GobElement *, bool focus);
    /// @brief Used to free all the data allocated specifically by this element
    void (*free)(struct GobElement *);
};

struct GobApp
{
    struct GobElement *root;
    struct GobElement *current_focus;
    ScreenData screen;
    KernelTerminalColors default_background_color;
    KernelTerminalColors status_bar_background_color;
    KernelTerminalColors status_bar_foreground_color;
    char status_text[80];
    bool running;

    InputMapState input_map_state;
};

struct GobLabelData
{
    char text[255];

    KernelTerminalColors text_color;
    KernelTerminalColors background_color;
};

struct GobInputBoxData
{
    /// @brief Currently inputted text
    char input_buffer[255];
    KernelTerminalColors text_color;
    KernelTerminalColors background_color;
    /// @brief Where is the editor cursor currently located
    int32_t cursor_position;
    /// @brief If true this will not override symbols when adding more
    bool is_insert_mode;
    int32_t width;
    int32_t offset;
    int32_t buffer_pos;
    KernelTerminalColors unfocused_text_color;
    KernelTerminalColors unfocused_background_color
};

typedef struct GobElement GobElement;
typedef struct GobInputBoxData GobInputBoxData;
typedef struct GobLabelData GobLabelData;

void gob_render_element(GobElement *elem)
{
    if (elem->render != NULL)
    {
        elem->render(elem);
    }
}

/// @brief Init the base element object
/// @param elem Object to init
/// @param x Horizontal position in screen coords
/// @param y Vertical position in screen coords
/// @param can_be_focused
void gob_init_element(GobElement *elem, int32_t x, int32_t y, bool can_be_focused)
{
    elem->x = x;
    elem->y = y;
    elem->can_be_focused = can_be_focused;
    elem->render = NULL;
    elem->focus_changed = NULL;
    elem->handle_input = NULL;
    elem->free = NULL;
    elem->next = NULL;
    elem->prev = NULL;
}

void gob_elem_free(GobElement *elem)
{
    elem->free(elem);
    free(elem);
}

void gob_label_free(GobElement *elem)
{
    GobLabelData *data = (GobLabelData *)(elem->internal);
    free(data);
}

void gob_label_render(GobElement *elem)
{
    GobLabelData *data = (GobLabelData *)(elem->internal);
    gob_term_text(&elem->app->screen, elem->x, elem->y, data->text, strlen(data->text), data->background_color, data->text_color);
}

void gob_label_set_text(GobElement *elem, const char *text)
{
    GobLabelData *data = (GobLabelData *)(elem->internal);
    strncpy(data->text, text, 255);
    gob_term_text(&elem->app->screen, elem->x, elem->y, data->text, strlen(data->text), data->background_color, data->text_color);
}

void gob_init_label(GobElement *elem, const char *text, int32_t x, int32_t y, KernelTerminalColors text_color, KernelTerminalColors background_color)
{
    GobLabelData *data = (GobLabelData *)malloc(sizeof(GobLabelData));
    elem->internal = data;
    gob_init_element(elem, x, y, false);
    elem->free = gob_label_free;
    elem->render = gob_label_render;
    data->text_color = text_color;
    data->background_color = background_color;
    strncpy(data->text, text, 255);
}

void gob_input_focus_changed(GobElement *elem, bool focus)
{
    if (elem->is_focused == focus)
    {
        return;
    }
    GobInputBoxData *data = (GobInputBoxData *)(elem->internal);
    elem->is_focused = focus;
    if (focus)
    {
        gob_term_text_fixed(&elem->app->screen,
                            elem->x,
                            elem->y,
                            data->input_buffer + data->offset,
                            strlen(data->input_buffer),
                            data->width,
                            ' ',
                            data->background_color,
                            data->text_color);
    }
    else
    {
        gob_term_text_fixed(&elem->app->screen,
                            elem->x,
                            elem->y,
                            data->input_buffer + data->offset,
                            strlen(data->input_buffer),
                            data->width,
                            ' ',
                            data->unfocused_background_color,
                            data->unfocused_text_color);
    }
}

void gob_input_render(GobElement *elem)
{
    GobInputBoxData *data = (GobInputBoxData *)(elem->internal);
    if (elem->is_focused)
    {
        gob_term_text_fixed(&elem->app->screen,
                            elem->x,
                            elem->y,
                            data->input_buffer + data->offset,
                            strlen(data->input_buffer),
                            data->width,
                            ' ',
                            data->background_color,
                            data->text_color);
    }
    else
    {
        gob_term_text_fixed(&elem->app->screen,
                            elem->x,
                            elem->y,
                            data->input_buffer + data->offset,
                            strlen(data->input_buffer),
                            data->width,
                            ' ',
                            data->unfocused_background_color,
                            data->unfocused_text_color);
    }
}

void gob_input_handle_input(GobElement *elem, InputEventData input)
{
    // placeholder
    char ch = exports->keycode_to_ascii(input.keycode);
    GobInputBoxData *data = (GobInputBoxData *)(elem->internal);
    data->input_buffer[data->buffer_pos++] = ch;
    gob_term_text_fixed(&elem->app->screen,
                        elem->x,
                        elem->y,
                        data->input_buffer + data->offset,
                        strlen(data->input_buffer),
                        data->width,
                        ' ',
                        data->background_color,
                        data->text_color);
}

void gob_init_input(GobElement *elem,
                    int32_t x,
                    int32_t y,
                    int32_t width,
                    KernelTerminalColors text_color,
                    KernelTerminalColors background_color,
                    KernelTerminalColors unfocused_text_color,
                    KernelTerminalColors unfocused_background_color)
{
    GobInputBoxData *data = (GobInputBoxData *)malloc(sizeof(GobInputBoxData));
    elem->internal = data;
    gob_init_element(elem, x, y, false);
    elem->render = gob_input_render;
    elem->focus_changed = gob_input_focus_changed;
    elem->handle_input = gob_input_handle_input;
    elem->can_be_focused = true;
    data->background_color = background_color;
    data->text_color = text_color;
    data->unfocused_background_color = unfocused_background_color;
    data->unfocused_text_color = unfocused_text_color;
    data->width = width;
}

typedef struct GobApp GobApp;

void gob_init_app(GobApp *app, uint16_t *text_buffer, int32_t width, int32_t height, KernelTerminalColors background_color)
{
    app->screen.text_buffer = text_buffer;
    app->screen.width = width;
    app->screen.height = height;
    app->default_background_color = background_color;
    app->root = NULL;
    app->current_focus = NULL;
    app->input_map_state.alt_state = false;
    app->input_map_state.ctrl_state = false;
    app->input_map_state.shift_state = false;
}
/// @brief Perform full redraw of the entire screen
/// @param app
void gob_app_render(GobApp *app)
{
    // app has full control over text buffer
    // so we can clear it as we want
    gob_term_clear(&app->screen, app->default_background_color);
    GobElement *root = app->root;
    while (root != NULL)
    {
        gob_render_element(root);
        root = root->next;
    }
    // draw the status bar
    gob_term_line(&app->screen, 0, app->screen.height - 1, app->screen.width, 'c', EC_Magenta, EC_Black);
    gob_term_text(&app->screen, 0, app->screen.height - 1, app->status_text, strnlen(app->status_text, 80), EC_Magenta, EC_Yellow);
}

void gob_app_set_status(GobApp *app, const char *text)
{
    strncpy(app->status_text, text, 80);
    // draw the status bar
    gob_term_line(&app->screen, 0, app->screen.height - 1, app->screen.width, ' ', EC_Magenta, EC_Black);
    gob_term_text(&app->screen, 0, app->screen.height - 1, app->status_text, strnlen(app->status_text, 80), EC_Magenta, EC_Yellow);
}

void gob_app_add_element(GobApp *app, GobElement *elem)
{
    if (elem == NULL)
    {
        return;
    }
    elem->app = app;
    if (app->root == NULL)
    {
        app->root = elem;
        return;
    }
    GobElement *prev = app->root;
    GobElement *start = prev->next;
    while (start != NULL)
    {
        prev = start;
        start = start->next;
    }
    prev->next = elem;
    elem->prev = prev;
    elem->next = NULL;
    if (app->current_focus == NULL && elem->can_be_focused)
    {
        app->current_focus = elem;
    }
}

/// @brief Get the last gui element in the app
/// @param app
/// @return Pointer to the element or NULL if no element is found
GobElement *gob_app_get_last_elem(GobApp *app)
{
    GobElement *result = NULL;
    GobElement *elem = app->root;
    while (elem != NULL)
    {
        result = elem;
        elem = elem->next;
    }
    return result;
}

/// @brief Advance focus to the next focusable object
/// @param app Application
/// @param start Start element from which to keep seeking. Passing NULL will start from the top of the list
void gob_app_advance_focus_forwards(GobApp *app, GobElement *start)
{
    GobElement *curr = start;
    if (start == NULL)
    {
        curr = app->root;
    }
    // else
    // {
    //     curr = start->next;
    // }
    if (curr == NULL)
    {
        return;
    }

    // go to the end of the list of all elements
    while (curr != NULL)
    {
        if (curr->can_be_focused)
        {
            if (app->current_focus != NULL)
            {
                app->current_focus->focus_changed(app->current_focus, false);
            }
            app->current_focus = curr;
            curr->focus_changed(curr, true);
            return;
        }
        curr = curr->next;
    }
    app->current_focus = NULL;
    // we reached the end of the list
    // check if we reached it starting from nothing, and if not do it again
    // this is to prevent infinite recursion
    if (start != NULL)
    {
        gob_app_advance_focus_forwards(app, NULL);
    }
}

void gob_app_advance_focus_backwards(GobApp *app, GobElement *start)
{
    GobElement *curr = start;
    if (start == NULL)
    {
        curr = gob_app_get_last_elem(app);
    }
    if (curr == NULL)
    {
        return;
    }
    while (curr != NULL)
    {
        if (curr->can_be_focused)
        {
            if (app->current_focus != NULL)
            {
                app->current_focus->focus_changed(app->current_focus, false);
            }
            app->current_focus = curr;
            curr->focus_changed(curr, true);
            return;
        }
        curr = curr->prev;
    }
    if (start != NULL)
    {
        gob_app_advance_focus_backwards(app, NULL);
    }
}

void gob_app_handle_input(GobApp *app, KernelKeycode keycode)
{
    if (keycode == EKC_TAB)
    {
        if (app->input_map_state.shift_state)
        {
            gob_app_advance_focus_backwards(app, app->current_focus->prev);
        }
        else
        {
            gob_app_advance_focus_forwards(app, app->current_focus->next);
        }
        return;
    }
    KernelKeycode key = keycode & 0b01111111;

    if (key == EKC_LEFT_SHIFT || key == EKC_RIGHT_SHIFT)
    {
        app->input_map_state.shift_state = (keycode & 0b10000000) > 0;
    }
    if (key == EKC_LEFT_CTRL || key == EKC_RIGHT_CTRL)
    {
        app->input_map_state.shift_state = (keycode & 0b10000000) > 0;
    }
    if (key == EKC_LEFT_ALT || key == EKC_RIGHT_ALT)
    {
        app->input_map_state.shift_state = (keycode & 0b10000000) > 0;
    }
    if (app->current_focus != NULL && app->current_focus->handle_input != NULL)
    {
        InputEventData event;
        event.shift_pressed = app->input_map_state.shift_state;
        event.alt_pressed = app->input_map_state.alt_state;
        event.ctrl_pressed = app->input_map_state.ctrl_state;
        event.pressed = (keycode & 0b10000000) == 0;
        event.keycode = key;
        app->current_focus->handle_input(app->current_focus, event);
    }
}

/// @brief Wait for input and return keycode of the key
/// @return
KernelKeycode get_input_keycode()
{
    return exports->scancode_to_keycode(exports->wait_for_keypress());
}

int main(struct KernelExports *kernel_exports)
{
    exports = kernel_exports;
    GobApp app;
    app.running = true;
    GobElement input_temp_label;
    GobElement input_box;
    GobElement input_box2;
    gob_init_label(&input_temp_label, "(5.5 / 2) * (24 / 8) - 232 * (9 + 2)", 20, 5, EC_Blue, EC_White);
    gob_init_input(&input_box, 20, 20, 20, EC_Black, EC_LightGray, EC_White, EC_DarkGray);
    gob_init_input(&input_box2, 20, 10, 20, EC_Black, EC_LightGray, EC_White, EC_DarkGray);
    gob_init_app(&app, get_text_buffer_address(), 80, 25, EC_Green);
    gob_app_add_element(&app, &input_temp_label);
    gob_app_add_element(&app, &input_box);
    gob_app_add_element(&app, &input_box2);
    gob_app_set_status(&app, "hello looser!");
    gob_app_render(&app);

    while (app.running)
    {
        gob_app_handle_input(&app, exports->scancode_to_keycode(exports->wait_for_keypress()));
    }
    return 0;

    // buffer for the entire input string
    // char input_buffer[255] = "(5.35 / 2) * (24.8 / 8) - 232 * (9 + 2)";
    char input_buffer[255] = "(5.5 / 2) * (24 / 8) - 232 * (9 + 2)";
    char *error_message = 0;
    char fixed_str_buffer[33];

    char output_buffer[255];
    CalcExpression operation_array[100];
    int32_t operation_array_len = 0;
    if (!parse_input(input_buffer, sizeof(input_buffer), &error_message, operation_array, 100, &operation_array_len))
    {
        gob_app_set_status(&app, error_message);
    }
    else
    {
        gob_app_set_status(&app, "parsed correctly");
        kernel_exports->printf("len: %s\n", itoa(operation_array_len, fixed_str_buffer, 10));
        for (int32_t i = 0; i < operation_array_len; i++)
        {
            switch (operation_array[i].type)
            {
            case ECET_Constant:
                memset(output_buffer, 0, sizeof(output_buffer));
                gob_sprintf(output_buffer, "%i ", operation_array[i].value.constant);
                break;
            case ECET_Operator:
                gob_sprintf(output_buffer, "%c ", operator_to_char(operation_array[i].value.operation));
                break;
            case ECET_BracketOpen:
                gob_sprintf(output_buffer, "( ");
                break;
            default:
                gob_sprintf(output_buffer, "???");
            }
            kernel_exports->printf(output_buffer);
        }
        number_t res;
        if (!execute(operation_array, operation_array_len, &res, &error_message))
        {
            kernel_exports->printf(error_message);
        }
        else
        {
            memset(output_buffer, 0, sizeof(output_buffer));
            gob_sprintf(output_buffer, "\n%s = %d", input_buffer, res);
            kernel_exports->printf(output_buffer);
        }
    }
    kernel_exports->wait_for_keypress();

    return 0;
}