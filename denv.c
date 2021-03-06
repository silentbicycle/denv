/* 
 * Copyright (c) 2010-2012 Scott Vokes <vokes.s@gmail.com>
 *  
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <err.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

#define BUFSZ (64 * 1024)

extern char **environ;

static void usage() {
    printf("Exec a command with env vars set to the contents of files in a dir.\n"
        "Usage: denv DIR COMMAND [ARGS...]\n");
}

static void read_var_file(const char *dir, const char *fname) {
    static char buf[BUFSZ];
    struct stat s;
    char path[PATH_MAX];
    FILE *fd = NULL;
    bool is_script = false;
    size_t sz;
    if (PATH_MAX <= snprintf(path, PATH_MAX, "%s/%s", dir, fname)) {
        errx(1, "snprintf error\n");
    }
    
    if (stat(path, &s) < 0) err(1, "%s", path);
    if (!S_ISREG(s.st_mode)) return; /* not regular file, skip */
    if (s.st_size == 0) {            /* empty file -> clear var */
        if (unsetenv(fname) < 0) err(1, "couldn't clear: %s", fname);
        return;
    }

    is_script = (s.st_mode & 0111) != 0;

    if (is_script) {            /* executable -> open with popen */
        if ((fd = popen(path, "r")) == NULL) err(1, "popen: %s", path);
    } else {                    /* normal file */
        if ((fd = fopen(path, "r")) == NULL) err(1, "couldn't open: %s", path);
    }
    
    if (fgets(buf, sizeof(buf), fd) != NULL) {
        sz = strlen(buf);
        if (buf[sz - 1] == '\n') buf[sz - 1] = '\0';
        if (setenv(fname, buf, 1) < 0) err(1, "setenv: %s", fname);
    }

    if (is_script) {
        if (pclose(fd) == EOF) err(1, "%s", path);
    } else {
        if (fclose(fd) == EOF) err(1, "%s", path);
    }
}

static void walk(char dir[]) {
    struct stat s;
    DIR *d = NULL;
    struct dirent *de = NULL;
    char path[PATH_MAX];
    
    stat(dir, &s);
    if (!S_ISDIR(s.st_mode)) err(1, "not a dir: %s", dir);
    
    d = opendir(dir);
    while ((de = readdir(d)) != NULL) {
        if (de->d_name[0] != '.') {
            if (PATH_MAX <= snprintf(path, PATH_MAX, "%s/%s", dir, de->d_name)) {
                errx(1, "snprintf error\n");
            }
            read_var_file(dir, de->d_name);
        }
    }
    closedir(d);
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        walk(argv[1]);
        if (execve(argv[2], argv + 2, environ) == -1) err(1, "bad cmd: %s", argv[2]);
        return 0;
    } else {
        usage();
        return 1;
    }
}
