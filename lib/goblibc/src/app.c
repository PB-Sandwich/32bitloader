#include <estros.h>
#include <stdlib.h>
extern int main();

int app_main()
{

    // in the future this can be used to perform initialisation and such
    // init();
    init_heap((uint8_t *)0x400000, 0x100000);
    main();
    // finish();
    return 0;
}

int errno = 0;

int *__errno_location(void)
{
    return &errno;
}