#include "../include/file_io.h"
#include <stdio.h>
#include <direct.h>

int ensure_directory(const char *path) {
    if (path == 0 || path[0] == '\0') {
        return 0;
    }
    if (_mkdir(path) == 0) {
        return 1;
    }
    return 1;
}

int append_line(const char *path, const char *line) {
    FILE *fp = fopen(path, "a");
    if (fp == 0) {
        return 0;
    }
    fputs(line, fp);
    fputs("\n", fp);
    fclose(fp);
    return 1;
}
