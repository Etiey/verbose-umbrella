#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "minimake.h"

/*Print error message to stderr and exit with code 2*/
void error_exit(const char *msg)
{
    fprintf(stderr, "minimake: *** %s. Stop.\n", msg);
    exit(2);
}

/*Print error message to stderr without exiting*/
void error_msg(const char *msg)
{
    fprintf(stderr, "minimake: %s\n", msg);
}

/* Check if a file exists using access() */
int file_exists(const char *path)
{
    return access(path, F_OK) == 0;
}

/* Get file modification time using stat() */
time_t get_modification_time(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
    {
        return 0;
    }
    return st.st_mtime;
}

/* Compare modification times of two files */
int is_older(const char *file1, const char *file2)
{
    time_t t1 = get_modification_time(file1);
    time_t t2 = get_modification_time(file2);
    if (t1 == 0 || t2 == 0)
    {
        return 0;
    }
    return t1 < t2;
}

/* Duplicate a string with error checking */
char *string_duplicate(const char *str)
{
    if (!str)
    {
        return NULL;
    }
    char *dup = strdup(str);
    if (!dup)
    {
        error_exit("Memory allocation failed");
    }
    return dup;
}

/* Split string by blank characters (space/tab) */
char **split_whitespace(const char *str, size_t *count)
{
    if (!str)
    {
        *count = 0;
        return NULL;
    }
    /* Allocate initial array */
    size_t cap = 16;
    char **result = malloc(sizeof(char *) * cap);
    *count = 0;
    /* Work on a copy to avoid modifying original */
    char *copy = string_duplicate(str);
    char *ptr = copy;
    while (*ptr)
    {
        /* Skip leading whitespace */
        while (isblank(*ptr))
        {
            ptr++;
        }
        if (*ptr == '\0')
        {
            break;
        }
        /* Find end of current token */
        char *start = ptr;
        while (*ptr && !isblank(*ptr))
        {
            ptr++;
        }
        /* Expand array if needed */
        if (*count >= cap)
        {
            cap *= 2;
            result = realloc(result, sizeof(char *) * cap);
        }
        /* Extract token */
        char old = *ptr;
        *ptr = '\0';
        result[(*count)++] = string_duplicate(start);
        if (old)
        {
            *ptr++ = old;
        }
    }
    free(copy);
    return result;
}

/* Free array of strings (NULL-terminated or with known size) */
void free_string_array(char **arr)
{
    if (!arr)
    {
        return;
    }
    for (size_t i = 0; arr[i]; i++)
    {
        free(arr[i]);
    }
    free(arr);
}

/* Remove leading and trailing whitespace from string */
char *trim_whitespace(char *str)
{
    /* Skip leading whitespace */
    while (isblank(*str))
    {
        str++;
    }
    /* Remove trailing whitespace */
    char *end = str + strlen(str) - 1;
    while (end > str && isblank(*end))
    {
        *end-- = '\0';
    }
    return str;
}
