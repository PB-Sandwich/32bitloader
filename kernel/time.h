
#ifndef TIME_H
#define TIME_H

#include <stdint.h>

struct time {
    uint32_t seconds;
    uint32_t millisecond;
};

extern volatile struct time time;

#endif
