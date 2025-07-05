#include <stdint.h>
#include <stdbool.h>
#include <endian.h>
#include <string.h>
#include <kernel.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>

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

int prev_random = 1;

int rand()
{
    prev_random = (75 * prev_random) % 65537;
    return prev_random;
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
    ECOT_Pow
};

union CalcExpressionValue
{
    fixed_t constant_value;
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
    case '^':
        expr.value.operation = ECOT_Pow;
        break;
    default:
        *success = false;
        return expr;
    }
    *success = true;
    return expr;
}

CalcExpression make_expr_const(fixed_t val)
{
    CalcExpression expr;
    expr.type = ECET_Constant;
    expr.value.constant_value = val;
    return expr;
}

/// @brief Get precedence for the given operator
/// @param op
/// @return
int32_t get_operator_precedence(char op)
{
    switch (op)
    {
    case '^':
        return 3;
    case '/':
    case '*':
        return 2;
    case '+':
    case '-':
        return 1;
    default:
        return -1;
    }
}

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
        return 2;
    case ECOT_Pow:
        return 3;
    default:
        return -1;
    }
}

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
    case ECOT_Pow:
        return '^';
    default:
        return '?';
    }
}

void run_value_string_test(struct KernelExports *kernel_exports)
{
    char buffer[33];
    kernel_exports->printf("input values to add:\n");
    char *line = kernel_exports->get_line();
    fixed_t f1 = str_to_fixed(line);
    line = kernel_exports->get_line();
    fixed_t f2 = str_to_fixed(line);
    fixed_t f3 = f1 + f2;
    char buffer_f1[33];
    char buffer_f2[33];
    char buffer_f3[33];

    kernel_exports->printf("%s + %s = %s", fixed_to_str(f1, buffer_f1), fixed_to_str(f2, buffer_f2), fixed_to_str(f3, buffer_f3));
    kernel_exports->wait_for_keypress();

    fixed_t val1 = -DECIMAL(0, 1);
    fixed_t val2 = DECIMAL(0, 3);
    fixed_t res = val1 + val2;
    kernel_exports->clear();
    kernel_exports->printf(fixed_to_str(DECIMAL(3545, 1234), buffer));
    kernel_exports->printf("\n");
    kernel_exports->printf(fixed_to_str(DECIMAL(0, 0), buffer));
    kernel_exports->printf("\n");
    kernel_exports->printf(fixed_to_str(DECIMAL(0, 9) + DECIMAL(0, 1), buffer));
    kernel_exports->printf("\n");
    kernel_exports->printf(fixed_to_str(DECIMAL(65534, 65535) + DECIMAL(0, 0001), buffer));
    kernel_exports->printf("\n");
    kernel_exports->printf(fixed_to_str(-DECIMAL(6, 6), buffer));
    kernel_exports->printf("\n");
    kernel_exports->printf(fixed_to_str(res, buffer));
    kernel_exports->printf("\n");
    kernel_exports->printf(fixed_to_str(DECIMAL(0, 9999) + DECIMAL(0, 1), buffer));
    kernel_exports->printf("\n");

    kernel_exports->wait_for_keypress();

    fixed_t test_val1 = DECIMAL(0, 3);
    fixed_t test_val2 = -test_val1;

    kernel_exports->printf("pos: %s\n", itoa(test_val1, buffer, 2));
    kernel_exports->printf("neg: %s\n", itoa(test_val2, buffer, 2));
    kernel_exports->wait_for_keypress();

    for (int j = 0; j < 20; j++)
    {
        kernel_exports->clear();
        int val = rand() * ((j % 2) ? 1 : -1);
        for (int i = 2; i <= 16; i++)
        {
            kernel_exports->printf("Number(%d) in base %d = %s\n", val, i, itoa(val, buffer, i));
        }
        kernel_exports->wait_for_keypress();
    }
}

bool parse_input(const char *input_buffer, int32_t input_buffer_len, char **error_message, CalcExpression *result, int32_t *result_len)
{
    // buffer used for parsing numbers
    char number_buffer[50];
    CalcExpression expression_stack[100];

    int32_t result_array_offset = 0;
    int32_t expression_stack_offset = -1;

    int32_t input_buffer_pos = 0;
    for (; input_buffer_pos < input_buffer_len && input_buffer[input_buffer_pos] != '\0'; input_buffer_pos++)
    {
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

int main(struct KernelExports *kernel_exports)
{

    // buffer for the entire input string
    char input_buffer[255] = "2.2 * (9 - 8) + 89.98 ^ (2 * 9)";
    char *error_message = 0;
    char fixed_str_buffer[33];

    CalcExpression operation_array[100];
    int32_t operation_array_len = 0;
    if (!parse_input(input_buffer, sizeof(input_buffer), &error_message, operation_array, &operation_array_len))
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
                kernel_exports->printf("%s ", fixed_to_str(operation_array[i].value.constant_value, fixed_str_buffer));
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
    }
    kernel_exports->wait_for_keypress();

    return 0;
}