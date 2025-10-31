#include "executor.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"
#include "variables.h"

/* Remove leading whitespace and tabs from command */
static char *strip_leading_ws(const char *cmd)
{
    char *result = string_duplicate(cmd);
    char *ptr = result;
    /* Skip all leading whitespace */
    while (*ptr && (isblank(*ptr) || *ptr == '\t'))
    {
        ptr++;
    }
    /* Move string to beginning */
    memmove(result, ptr, strlen(ptr) + 1);
    return result;
}

/* Check if command should be logged (doesn't start with @) */
static int should_log(const char *cmd)
{
    const char *ptr = cmd;
    /* Skip whitespace to find first real character */
    while (*ptr && (isblank(*ptr) || *ptr == '\t'))
    {
        ptr++;
    }
    /* Don't log if starts with @ */
    return *ptr != '@';
}

/* Remove @ sign from beginning of command */
static char *remove_at_sign(const char *cmd)
{
    char *result = string_duplicate(cmd);
    char *ptr = result;
    /* Find first non-whitespace character */
    while (*ptr && (isblank(*ptr) || *ptr == '\t'))
    {
        ptr++;
    }
    /* Remove @ if present */
    if (*ptr == '@')
    {
        memmove(ptr, ptr + 1, strlen(ptr));
    }
    return result;
}

/* Helper to append string to dynamic buffer */
static void expand_special_var(char **dst, size_t *pos, size_t *cap,
                               const char *value)
{
    size_t val_len = strlen(value);
    /* Expand buffer if needed */
    while (*pos + val_len >= *cap)
    {
        *cap *= 2;
        *dst = realloc(*dst, *cap);
        if (!*dst)
        {
            error_exit("Memory allocation failed");
        }
    }
    /* Append value */
    strcpy(*dst + *pos, value);
    *pos += val_len;
}

/* Expand special variables ($@, $<, $^) in command */
static char *expand_special(const char *cmd, rule_t *rule)
{
    size_t cap = 4096;
    char *result = malloc(cap);
    if (!result)
    {
        error_exit("Memory allocation failed");
    }
    const char *src = cmd;
    size_t pos = 0;

    while (*src)
    {
        /* $@ = target name */
        if (*src == '$' && src[1] == '@')
        {
            expand_special_var(&result, &pos, &cap, rule->target);
            src += 2;
        }
        /* $< = first dependency */
        else if (*src == '$' && src[1] == '<')
        {
            if (rule->dep_count > 0)
            {
                char *exp = variable_expand(rule->dependencies[0]);
                expand_special_var(&result, &pos, &cap, exp);
                free(exp);
            }
            src += 2;
        }
        /* $^ = all dependencies */
        else if (*src == '$' && src[1] == '^')
        {
            for (size_t i = 0; i < rule->dep_count; i++)
            {
                if (i > 0)
                {
                    result[pos++] = ' ';
                    result[pos] = '\0';
                }
                char *exp = variable_expand(rule->dependencies[i]);
                expand_special_var(&result, &pos, &cap, exp);
                free(exp);
            }
            src += 2;
        }
        /* Regular character */
        else
        {
            if (pos + 1 >= cap)
            {
                cap *= 2;
                result = realloc(result, cap);
                if (!result)
                {
                    error_exit("Memory allocation failed");
                }
            }
            result[pos++] = *src++;
            result[pos] = '\0';
        }
    }
    return result;
}

/* Execute a single command using /bin/sh -c */
static int exec_command(const char *cmd)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        error_msg("Fork failed");
        return 2;
    }
    /* Child process: execute command */
    if (pid == 0)
    {
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        perror("execl");
        exit(127);
    }
    /* Parent process: wait for child */
    int status;
    waitpid(pid, &status, 0);
    /* Check exit status */
    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0)
        {
            return 2; /* Command failed */
        }
    }
    else if (WIFSIGNALED(status))
    {
        return 2; /* Killed by signal */
    }
    return 0;
}

/* Execute all commands in a rule's recipe */
int execute_recipe(rule_t *rule)
{
    for (size_t i = 0; i < rule->recipe_count; i++)
    {
        /* Expand special variables ($@, $<, $^) */
        char *special = expand_special(rule->recipe[i], rule);
        /* Expand regular variables */
        char *expanded = variable_expand(special);
        /* Remove leading whitespace */
        char *cleaned = strip_leading_ws(expanded);
        /* Log command if not silent */
        if (should_log(rule->recipe[i]))
        {
            printf("%s\n", cleaned);
            fflush(stdout); /* Flush before execution */
        }
        /* Remove @ sign and execute */
        char *exec_cmd = remove_at_sign(cleaned);
        char *final_cmd = strip_leading_ws(exec_cmd);
        int ret = exec_command(final_cmd);
        /* Cleanup */
        free(special);
        free(expanded);
        free(cleaned);
        free(exec_cmd);
        free(final_cmd);
        /* Stop on first error */
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}
