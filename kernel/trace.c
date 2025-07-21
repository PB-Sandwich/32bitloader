
#include "trace.h"
#include <print.h>
#include <stdint.h>

extern void kernel_entry(void*);

void run_stack_trace()
{
    uint32_t* ebp;
    __asm__ volatile("movl %%ebp, %0\n\t" : "=r"(ebp));

    printf("Running stack trace\n");

    while (ebp < (uint32_t*)0x90000) {
        uint32_t return_address = ebp[1];
        printf("[0x%x]\n", return_address);

        ebp = (uint32_t*)ebp[0];
    }
}
