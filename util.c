#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

void *emalloc(size_t size) {
    void *buf = calloc(1, size);

    if(buf == NULL) {
        err(EXIT_FAILURE, "emalloc");
    }

    return buf;
}
