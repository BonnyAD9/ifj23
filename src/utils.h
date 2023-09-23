#include <stdio.h>
#include <stdlib.h>

#include "errors.h"

#define TODO(...) do { \
    EPRINTF("todo %s\n", "" __VA_ARGS__); \
    exit(ERR_OTHER); \
} while (0);

#define EPRINTF(...) fprintf(stderr, __VA_ARGS__)
