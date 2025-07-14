#include <estros.h>
#include <stdlib.h>
extern int main(struct KernelExports *kernel_exports);

int errno = 0;

int *__errno_location(void)
{
    return &errno;
}

int app_main(struct KernelExports *kernel_exports)
{

    // in the future this can be used to perform initialisation and such
    // init();
    init_heap((uint8_t *)0x400000, 0x100000);
    main(kernel_exports);
    // finish();
    return 0;
}