#ifndef ESTROS_APP_DATA_H
#define ESTROS_APP_DATA_H

#include <stdint.h>

typedef struct {
    void* heap_space;
    void* stdout;
    void* stdin;
    void* stderr;
} EstrosAppData;

static inline EstrosAppData get_app_data()
{
    EstrosAppData app_data;
    __asm__ volatile("int $0x40" : "=a"(app_data.heap_space), "=b"(app_data.stdout), "=c"(app_data.stdin), "=d"(app_data.stderr) : "a"(1));
    return app_data;
}

#endif
