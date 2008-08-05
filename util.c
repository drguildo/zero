/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

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
