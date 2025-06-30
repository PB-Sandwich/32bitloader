#include <stdint.h>
#include <stdbool.h>
#include <endian.h>
#include <string.h>
#include <kernel.h>
#include <stdlib.h>
#include <ctype.h>

int prev_random = 1;

int rand()
{
    prev_random = (75 * prev_random) % 65537;
    return prev_random;
}
int main(struct KernelExports *kernel_exports)
{
    char buffer[33];
    kernel_exports->clear();
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

    return 0;
}