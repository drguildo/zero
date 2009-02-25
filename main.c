/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include <sys/stat.h>

#include <err.h>
#define __USE_XOPEN_EXTENDED
#include <ftw.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

int verbose = 0;

static char *randstr(size_t len) {
    const char *chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    char *name;
    size_t i;

    name = emalloc(len+1);

    for (i = 0; i < len; i++) {
        name[i] = chars[rand() % strlen(chars)];
    }
    name[len+1] = '\0';

    return name;
}

static char *randren(const char *path) {
    unsigned long newpathlen = 0;
    unsigned int error = 0;

    char *dname, *rname, *pathcopy, *newpath;

    /* glibc dirname modifies its argument */
    pathcopy = strdup(path);

    dname = dirname(pathcopy);

    /* Calling pathconf directly on a symlink will return the value for
     * the file that it links to or, if it's a dangling symlink, -1
     * (failure) so we use the directory it's stored in instead. */
    rname = randstr(pathconf(dname, _PC_NAME_MAX));

    newpathlen += strlen(dname);
    newpathlen += strlen(rname);
    newpathlen += 2; /* '/' & '\0' */

    newpath = emalloc(newpathlen);

    strcpy(newpath, dname);
    strcpy(newpath + strlen(newpath), "/");
    strcpy(newpath + strlen(newpath), rname);
    strcpy(newpath + strlen(newpath), "\0");

    /* check whether the file exists */
    if (access(newpath, F_OK) == 0) {
        warnx("randren: File exists");
        error = 1;
    } else {
        if (rename(path, newpath) == -1) {
            warn("randren");
            error = 1;
        }
    }

    free(pathcopy);
    free(rname);

    if (error == 1) {
        free(newpath);
        return NULL;
    } else {
        return newpath;
    }
}

static int fzero(const char *path) {
    ssize_t written = 0;

    int fd;
    void *buf;
    struct stat stbuf;

    if (stat(path, &stbuf) == -1) {
        warn("fzero");
        return -1;
    }

    buf = emalloc(stbuf.st_blksize);

    if ((fd = open(path, O_WRONLY)) == -1) {
        warn("fzero");
        return -1;
    }

    while (written < stbuf.st_size) {
        written += write(fd, buf, stbuf.st_blksize);
        if (verbose) {
            putchar('.');
        }
    }
    if (verbose) {
        putchar('\n');
    }

    fdatasync(fd);

    close(fd);
    free(buf);

    return 0;
}

int can_write(const struct stat *sb) {
    uid_t id;

    id = geteuid();
    if (sb->st_uid == id) {
        return 1;
    } else {
        return 0;
    }
}

int fn(const char *fpath, const struct stat *sb, int typeflag,
       struct FTW *ftwbuf) {
    char *path;

    if (!can_write(sb)) {
        return 0;
    }

    path = randren(fpath);
    if (path == NULL) {
        return 1;
    }

    switch (typeflag) {
        case FTW_SL:
            unlink(path);
            break;
        case FTW_F:
            if (fzero(path) != -1) {
                unlink(path);
            }
            break;
        case FTW_DP:
            rmdir(path);
            break;
    }
    free(path);

    return 0;
}

int main(int argc, char *argv[]) {
    int i, c;

    while ((c = getopt(argc, argv, "+v")) != -1) {
        switch (c) {
            case 'v':
                verbose = 1;
                break;
        }
    }

    if (optind == argc) {
        fprintf(stderr, "usage: %s [-rv] [file ...]\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    for (i = optind; i < argc; i++) {
        nftw(argv[i], fn, 10, FTW_DEPTH | FTW_PHYS);
    }

    return 0;
}
