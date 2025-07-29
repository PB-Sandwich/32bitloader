#include <estros.h>
#include <stdlib.h>
#include <estros/syscall.h>
#include <estros/process.h>
extern int main();

int app_main()
{

    // in the future this can be used to perform initialisation and such
    // init();
    Process *p = get_current_process();
    estros_stdin = p->stdin;
    estros_stdout = p->stdout;
    estros_stderr = p->stderr;
    init_heap((uint8_t *)0x500000, 0x100000);
    int res = main();
    exit(res);
    // finish();
    // return 0;
}

int errno = 0;

File *estros_stdin;
File *estros_stdout;
File *estros_stderr;

int *__errno_location(void)
{
    return &errno;
}