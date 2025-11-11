#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commands.h"

#define BUFFER_SIZE 256

typedef int (*handler)(const char *arg);

struct cmd
{
    handler handle;
    const char *command_name;
};

static struct cmd commands[] = {
    { .handle = help, .command_name = "help" },
    { .handle = hello, .command_name = "hello" },
    { .handle = print, .command_name = "print" },
    { .handle = myexit, .command_name = "exit" },
    { .handle = cat, .command_name = "cat" },
};

static char *readline(void)
{
    printf("cmd$ ");
    fflush(stdout);

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    nread = getline(&line, &len, stdin);
    if (nread == -1)
    {
        free(line);
        return NULL;
    }

    // Remove trailing newline if present
    if (nread > 0 && line[nread - 1] == '\n')
        line[nread - 1] = '\0';

    return line;
}

static int call_command(const char *cmd, const char *arg)
{
    size_t nb_command = sizeof(commands) / sizeof(struct cmd);

    for (size_t i = 0; i < nb_command; ++i)
        if (!strcmp(cmd, commands[i].command_name))
            return commands[i].handle(arg);

    warnx("unknown command");
    return -1;
}

static int parse_and_execute(char *cmd)
{
    char *arg = NULL;
    char *command_name = strtok(cmd, " ");

    if (command_name == NULL)
        command_name = cmd;
    else
    {
        arg = strtok(NULL, " ");
        if (strtok(NULL, " "))
        {
            warnx("A command can take only one argument");
            return -1;
        }
    }
    return call_command(command_name, arg);
}

int main(void)
{
    while (1)
    {
        char *line = readline();
        if (line == NULL)
            break;
        int status = parse_and_execute(line);
        free(line);
        if (status == -1)
        {
            break;
        }
    }
    return 0;
}
