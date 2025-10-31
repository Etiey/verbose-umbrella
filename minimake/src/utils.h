#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <time.h>

void error_exit(const char *msg);
void error_msg(const char *msg);
int file_exists(const char *path);
time_t get_modification_time(const char *path);
int is_older(const char *file1, const char *file2);
char *string_duplicate(const char *str);
char **split_whitespace(const char *str, size_t *count);
void free_string_array(char **arr);
char *trim_whitespace(char *str);

#endif /*UTILS_H*/
