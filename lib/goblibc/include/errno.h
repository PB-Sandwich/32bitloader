#pragma once

/* The error code set by various library functions.  */
extern int *__errno_location (void);
#define errno (*__errno_location())

#define EINVAL          22      /* Invalid argument */
#define ERANGE          34      /* Result too large */
#define EOVERFLOW       79      /* Value too large to be stored in data type */
#define EILSEQ          86      /* Illegal byte sequence */