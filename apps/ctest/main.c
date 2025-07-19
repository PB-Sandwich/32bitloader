#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <format.h>
#include <stdio.h>
#include <estros.h>

int main()
{
    char sprintf_test_buffer_1[255];
    char user_math_buffer[255];
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    sys_print("Hello world in c! Here are some test numbers to test if stuff works! \n", 71);
    gob_sprintf(sprintf_test_buffer_1, "int: %d; float: %f; hex: %u\n", 69420, 69.420f, 4096);
    sys_print(sprintf_test_buffer_1, strlen(sprintf_test_buffer_1));

    sys_print("Type a *single* float value to do some math: ", 45);
    uint32_t bytes_read = sys_read(buffer, 10);
    float user_value = 0;
    gob_sscanf(buffer, "%f", &user_value);
    sys_print(buffer, 10);
    sys_print("\n", 1);
    gob_sprintf(user_math_buffer, "%.3f + %.4f = %.2f\n", 69.420f, user_value, user_value + 69.420f);
    sys_print(user_math_buffer, strlen(user_math_buffer));
    return 0;
}