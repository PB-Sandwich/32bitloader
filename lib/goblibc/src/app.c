#include <kernel.h>

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
    main(kernel_exports);
    // finish();
    return 0;
}