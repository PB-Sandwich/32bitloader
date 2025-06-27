#include <kernel.h>

extern int main(struct KernelExports *kernel_exports);

int app_main(struct KernelExports *kernel_exports)
{  
    // in the future this can be used to perform initialisation and such
    // init();
    main(kernel_exports);
    // finish();
    return 0;
}