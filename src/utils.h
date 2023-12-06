#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdio.h>  // fprintf, stderr
#include <stdlib.h> // exit

#include "errors.h"

#define TODO(...) do { \
    EPRINTF("todo %s\n", "" __VA_ARGS__); \
    exit(ERR_OTHER); \
} while (0)

#define EPRINTF(...) fprintf(stderr, __VA_ARGS__)

#ifdef NDEBUG

#define DPRINTF(...)

#else

#define DPRINTF(fmt, ...) EPRINTF("debug: " fmt "\n", ##__VA_ARGS__)

#define WPRINTF(fmt, ...) fprintf(stdout, "warning: " fmt "\n", ##__VA_ARGS__)

#endif // NDEBUG

#endif // UTILS_H_INCLUDED
