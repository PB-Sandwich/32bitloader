#include <kernel.h>

extern "C" int main(KernelExports*);

int main(KernelExports* kernel_exports)
{
    kernel_exports->clear();
    kernel_exports->printf("hello world from c++!\n");
    kernel_exports->wait_for_keypress();
    return 0;
}