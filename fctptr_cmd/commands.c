#include "commands.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int help(const char *arg)
{
    if (arg != NULL)
    {
        fprintf(stderr, "fctptr_cmd: help takes no arguments\n");
        return 1;
    }
    printf("The available commands are:\n");
    printf("help\n");
    printf("hello\n");
    printf("print string\n");
    printf("exit\n");
    printf("cat file\n");
    return 0;
}

int hello(const char *arg)
{
    if (arg != NULL)
    {
        fprintf(stderr, "fctptr_cmd: hello takes no arguments\n");
        return 1;
    }
    printf("hello\n");
    return 0;
}

int print(const char *arg)
{
    if (arg == NULL)
    {
        fprintf(stderr, "fctptr_cmd: print requires one argument\n");
        return 1;
    }
    printf("%s\n", arg);
    return 0;
}

int myexit(const char *arg)
{
    if (arg != NULL)
    {
        fprintf(stderr, "fctptr_cmd: exit takes no arguments\n");
        return 1;
    }
    return -1;
}

int cat(const char *arg)
{
    if (arg == NULL)
    {
        fprintf(stderr, "fctptr_cmd: cat requires one argument\n");
        return 1;
    }
    int fd = open(arg, O_RDONLY);
    if (fd == -1)
    {
        fprintf(stderr, "fctptr_cmd: cannot open file '%s'\n", arg);
        return 1;
    }
    char buf[4096];
    ssize_t r;
    while ((r = read(fd, buf, sizeof(buf))) > 0)
    {
        write(1, buf, r);
    }
    close(fd);
    if (r == -1)
    {
        fprintf(stderr, "fctptr_cmd: error reading file\n");
        return 1;
    }
    return 0;
}
