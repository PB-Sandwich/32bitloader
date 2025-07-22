#include <trace.h>
#include <exit.h>
#include <filesystem/virtual-filesystem.h>
#include <memutils.h>
#include <print.h>
#include <stdint.h>
#include <time.h>

void emergency_print(char* str)
{
    memcpy((void*)0xb8000, str, strlen(str));
}

void exit_kernel()
{
    VFSFile* tty = vfs_open_file("/dev/tty", VFS_WRITE);
    if (tty == NULL) {
        emergency_print("Could not open tty at /dev/tty");
        while (1)
            ;
    }

    VFSFile* log = vfs_open_file("/sys/kernel.log", VFS_READ);

    if (log == NULL) {
        emergency_print("Could not open log file at /sys/kernel.log");
        while (1)
            ;
    }

    set_print_output("/dev/tty");

    uint8_t buffer[1024];
    uint32_t length = 0;
    while ((length = vfs_read(log, buffer, 1024)) > 0) {
        vfs_write(tty, buffer, length);
    }
    vfs_write(tty, "\ntail of /sys/kernel.log\nexited kernel\n", strlen("\ntail of /sys/kernel.log\nexited kernel\n"));
    vfs_write(tty, "running stack trace in\n", strlen("running stack trace in\n"));

    uint64_t start_time = time.seconds;
    int i = 5;
    while (1) {
        if (i == 0) {
            break;
        }
        uint64_t current_time = time.seconds;
        if (current_time > start_time + 1) {
            start_time = time.seconds;
            printf("%d...", i);
            i--;
        }
    }
    printf("\n");
    run_stack_trace();

    while (1)
        ;
}
