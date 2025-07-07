#include <stdint.h>
#include <stdbool.h>
#include <endian.h>
#include <string.h>
#include <kernel.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <format.h>
#include <errno.h>
#include <stdio.h>
// #define BIT_FIXED_DECIMAL
#ifdef BIT_FIXED_DECIMAL
/// @brief simple alternative to floats until they will be available. Upper 16 bits are whole part and lower 16 bits are decimal part
typedef int32_t fixed_t;
#define FIXED_DECIMAL_UPPER_MASK 0xffff0000
#define FIXED_DECIMAL_LOWER_MASK 0x0000ffff

#define DECIMAL(whole, decimal) (((uint32_t)whole << 16) | (uint32_t)decimal)

/// @brief Convert a fixed decimal type value into string
/// @param val Value to convert
/// @param buffer Buffer where the value will be stored. Must be able to contain whole number
/// @return Pointer to the buffer
char *fixed_point_to_string(fixed_t val, char *buffer)
{
    // max number count for whole part, max number count for decimal part, byte for point, byte for sign, byte for null terminator
    char temp[5];
    // where do we currently have to write in the buffer
    uint32_t buffer_pos = 0;

    if (val < 0)
    {
        buffer[buffer_pos++] = '-';
        val = -val;
    }
    uint32_t current_num_len = 0;
    uint16_t upper = (val & FIXED_DECIMAL_UPPER_MASK) >> 16;
    uint16_t lower = val & FIXED_DECIMAL_LOWER_MASK;

    int_to_str(upper, buffer + buffer_pos, 10, &current_num_len);
    buffer_pos += current_num_len;
    buffer[buffer_pos++] = '.';

    for (int32_t i = sizeof(temp) - 1; i >= 0; lower /= 10)
    {
        temp[i--] = lower % 10 + '0';
    }
    for (int32_t i = 0; i < sizeof(temp); i++)
    {
        buffer[buffer_pos + i] = temp[i];
    }
    buffer[buffer_pos + sizeof(temp)] = '\0';
    return buffer;
}
#else
// this is the implementation which is meant for base 10
// this does mean that it can be awkward to use

/// @brief simple alternative to floats until they will be available. Upper 16 bits are whole part and lower 16 bits are decimal part
typedef int32_t fixed_t;
// Which position does the whole number start from
#define FIXED_WHOLE_POSITION 10000

#define DECIMAL(whole, decimal) (((fixed_t)whole * FIXED_WHOLE_POSITION) + (fixed_t)decimal)

/// @brief Convert a fixed decimal type value into string
/// @param val Value to convert
/// @param buffer Buffer where the value will be stored. Must be able to contain whole number
/// @return Pointer to the buffer
char *fixed_to_str(fixed_t val, char *buffer)
{
    // max number count for whole part, max number count for decimal part, byte for point, byte for sign, byte for null terminator
    char temp[4];
    // where do we currently have to write in the buffer
    uint32_t buffer_pos = 0;

    if (val < 0)
    {
        buffer[buffer_pos++] = '-';
        val = -val;
    }
    uint32_t current_num_len = 0;
    uint16_t upper = val / FIXED_WHOLE_POSITION;
    uint16_t lower = val % FIXED_WHOLE_POSITION;

    int_to_str(upper, buffer + buffer_pos, 10, &current_num_len);
    buffer_pos += current_num_len;
    buffer[buffer_pos++] = '.';

    for (int32_t i = sizeof(temp) - 1; i >= 0; lower /= 10)
    {
        temp[i--] = lower % 10 + '0';
    }
    for (int32_t i = 0; i < sizeof(temp); i++)
    {
        buffer[buffer_pos + i] = temp[i];
    }
    buffer[buffer_pos + sizeof(temp)] = '\0';
    return buffer;
}

fixed_t fixed_mul(fixed_t a, fixed_t b)
{
    double ta = (a / FIXED_WHOLE_POSITION) + (a % FIXED_WHOLE_POSITION) / FIXED_WHOLE_POSITION;
    double tb = (b / FIXED_WHOLE_POSITION) + (b % FIXED_WHOLE_POSITION) / FIXED_WHOLE_POSITION;
    double tc = ta * tb;
    return DECIMAL((fixed_t)tc, (tc - (double)((fixed_t)tc) * FIXED_WHOLE_POSITION));
}

fixed_t fixed_div(fixed_t a, fixed_t b)
{
    return (((a * FIXED_WHOLE_POSITION) + (b / 2)) / b);
}

fixed_t fixed_mod(fixed_t a, fixed_t b)
{
    return a - ((a / b) * a);
}

/// @brief Convert a null terminated string into a fixed decimal value
/// @param buffer String buffer
/// @return parsed value
fixed_t str_to_fixed(char *buffer)
{
    int had_sign = false;
    int had_point = false;
    fixed_t upper = 0;
    fixed_t lower = 0;
    fixed_t sign = 1;
    int32_t decimal_part_mult = FIXED_WHOLE_POSITION / 10;
    for (; *buffer != '\0'; buffer++)
    {
        if (*buffer == '-' && !had_sign)
        {
            sign = -1;
            had_sign = 1;
        }
        else if (*buffer == '.')
        {
            had_point = 1;
        }
        else if (isdigit(*buffer))
        {
            had_sign = 1;
            if (!had_point)
            {
                upper += *buffer - '0';
                upper *= 10;
            }
            else
            {
                lower += (*buffer - '0') * decimal_part_mult;
                decimal_part_mult /= 10;
            }
        }
    }
    return DECIMAL(upper / 10, lower) * sign;
}
#endif

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
    fixed_t constant;
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
CalcExpression make_expr_const(fixed_t val)
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
    fixed_t stack[50];
    int32_t offset;
};

typedef struct NumberStack NumberStack;

void number_stack_init(NumberStack *stack)
{
    stack->offset = 0;
    memset(stack->stack, 0, sizeof(int32_t) * 50);
}
fixed_t number_stack_pop(NumberStack *stack, bool *success)
{
    if (stack->offset == 0)
    {
        *success = false;
        return 0;
    }
    *success = true;
    return stack->stack[--stack->offset];
}

void number_stack_push(NumberStack *stack, fixed_t val)
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
                    if (had_decimal_point)
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
            result[result_array_offset++] = make_expr_const(str_to_fixed(number_buffer));
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

bool execute(CalcExpression *ops, int32_t ops_len, fixed_t *result, char **error_message)
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

            fixed_t b = number_stack_pop(&stack, &value_grab_result);
            if (!value_grab_result)
            {
                *error_message = "Not enough values on stack";
                return false;
            }
            fixed_t a = number_stack_pop(&stack, &value_grab_result);
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
                number_stack_push(&stack, fixed_mul(a, b));
                break;
            case ECOT_Div:
                number_stack_push(&stack, fixed_div(a, b));
                break;
            case ECOT_Mod:
                number_stack_push(&stack, fixed_mod(a, b));
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

int main(struct KernelExports *kernel_exports)
{

    // buffer for the entire input string
    // char input_buffer[255] = "(5.35 / 2) * (24.8 / 8) - 232 * (9 + 2)";
    char input_buffer[255] = "232 * 11";
    char *error_message = 0;
    char fixed_str_buffer[33];

    char output_buffer[255];
    gob_sprintf(output_buffer, "dec: %u, oct: %o,hex: %x. Also a float: %f but %s\n", 1234, 1234, 1234, 69.4201f, input_buffer);
    kernel_exports->printf(output_buffer);
    kernel_exports->wait_for_keypress();

    double mul_val3;
    int mul_val1;
    int mul_val2;
    int match_count = gob_sscanf("23.4", "%f", &mul_val3);
    memset(output_buffer, 0, sizeof(input_buffer));
    gob_sprintf(output_buffer, "\nx = %x;f = %f;\n", mul_val3, mul_val3);
    kernel_exports->printf(output_buffer);
    kernel_exports->printf(strerror(errno));
    kernel_exports->wait_for_keypress();

    kernel_exports->printf("%s\n", fixed_to_str(fixed_mul(DECIMAL(232, 0), DECIMAL(11, 0)), fixed_str_buffer));

    CalcExpression operation_array[100];
    int32_t operation_array_len = 0;
    if (!parse_input(input_buffer, sizeof(input_buffer), &error_message, operation_array, 100, &operation_array_len))
    {
        kernel_exports->printf(error_message);
    }
    else
    {
        kernel_exports->printf("parsed correctly\n");
        kernel_exports->printf("len: %s\n", itoa(operation_array_len, fixed_str_buffer, 10));
        for (int32_t i = 0; i < operation_array_len; i++)
        {
            switch (operation_array[i].type)
            {
            case ECET_Constant:
                kernel_exports->printf("%s ", fixed_to_str(operation_array[i].value.constant, fixed_str_buffer));
                break;
            case ECET_Operator:
                kernel_exports->printf("%c ", operator_to_char(operation_array[i].value.operation));
                break;
            case ECET_BracketOpen:
                kernel_exports->printf("?(? ");
                break;
            default:
                kernel_exports->printf("??? ");
            }
        }
        fixed_t res;
        if (!execute(operation_array, operation_array_len, &res, &error_message))
        {
            kernel_exports->printf(error_message);
        }
        else
        {
            kernel_exports->printf("\n%s = %s\n", input_buffer, fixed_to_str(res, fixed_str_buffer));
        }
    }
    kernel_exports->wait_for_keypress();

    return 0;
}