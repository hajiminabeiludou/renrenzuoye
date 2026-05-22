#ifndef FILE_IO_H
#define FILE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

int ensure_directory(const char *path);
int append_line(const char *path, const char *line);

#ifdef __cplusplus
}
#endif

#endif
