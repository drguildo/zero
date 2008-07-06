#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "util.h"

static char *randstr(size_t len) {
    const char *chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    char *name;
    size_t i;

    name = emalloc(len+1);

    for (i = 0; i < len; i++) {
        name[i] = chars[rand() % strlen(chars)];
    }
    name[len] = '\0';

    return name;
}

static char *randren(const char *path) {
    unsigned long newpathlen = 0;
    unsigned int error = 0;

    char *dname, *rname, *pathcopy, *newpath;

    /* glibc dirname modifies its argument */
    pathcopy = strdup(path);

    dname = dirname(pathcopy);

    rname = randstr(pathconf(path, _PC_NAME_MAX));

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
    }

    fdatasync(fd);

    close(fd);
    free(buf);

    return 0;
}

int main(int argc, char *argv[]) {
    int i;
    char *path;

    if (argc < 2) {
        fprintf(stderr, "usage: %s [file ...]\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    for (i = 1; i < argc; i++) {
        path = randren(argv[i]);

        if (path != NULL) {
            if (fzero(path) != -1) {
                unlink(path);
            }
            free(path);
        }
    }

    return 0;
}
