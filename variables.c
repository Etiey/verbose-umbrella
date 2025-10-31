#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minimake.h"

/*Head of the variables linked list*/
static variable_t *vars_head = NULL;

/*Initialize the variable system (reset to empty)*/
void variable_init(void)
{
    vars_head = NULL;
}

/*Set a variable value (creates new or updates existing)*/
void variable_set(const char *name, const char *value)
{
    /*Expand variable name (for cases like $(VAR)=value)*/
    char *exp_name = variable_expand(name);
    /*Check if variable already exists*/
    for (variable_t *v = vars_head; v; v = v->next)
    {
        if (strcmp(v->name, exp_name) == 0)
        {
            /*Update existing variable*/
            free(v->value);
            v->value = string_duplicate(value);
            free(exp_name);
            return;
        }
    }
    /*Create new variable*/
    variable_t *new_var = malloc(sizeof(variable_t));
    if (!new_var)
    {
        error_exit("Memory allocation failed");
    }
    new_var->name = exp_name;
    new_var->value = string_duplicate(value);
    new_var->next = vars_head;
    vars_head = new_var;
}

/*Get variable value (with environment variable fallback)*/
const char *variable_get(const char *name)
{
    /*Search in our variables*/
    for (variable_t *v = vars_head; v; v = v->next)
    {
        if (strcmp(v->name, name) == 0)
        {
            return v->value;
        }
    }
    /*Fallback to environment variables*/
    return getenv(name);
}

/*Extract variable name from $(VAR), ${VAR}, or $V syntax*/
static char *extract_var_name(const char *str, size_t *len)
{
    /*Handle $(VAR) syntax*/
    if (str[1] == '(')
    {
        const char *end = strchr(str + 2, ')');
        if (!end)
        {
            error_exit("Unterminated variable reference");
        }
        *len = end - str + 1;
        char *name = malloc(end - str - 1);
        strncpy(name, str + 2, end - str - 2);
        name[end - str - 2] = '\0';
        return name;
    }
    /*Handle ${VAR} syntax*/
    if (str[1] == '{')
    {
        const char *end = strchr(str + 2, '}');
        if (!end)
        {
            error_exit("Unterminated variable reference");
        }
        *len = end - str + 1;
        char *name = malloc(end - str - 1);
        strncpy(name, str + 2, end - str - 2);
        name[end - str - 2] = '\0';
        return name;
    }
    /*Handle $V syntax (single character)*/
    *len = 2;
    char *name = malloc(2);
    name[0] = str[1];
    name[1] = '\0';
    return name;
}

/*Recursively expand all variables in a string*/
char *variable_expand(const char *str)
{
    if (!str)
    {
        return NULL;
    }
    /*Allocate result buffer*/
    size_t cap = 4096;
    char *result = malloc(cap);
    if (!result)
    {
        error_exit("Memory allocation failed");
    }
    size_t pos = 0;
    const char *src = str;
    /*Process string character by character*/
    while (*src)
    {
        if (*src == '$')
        {
            /*Handle $$ -> $ escape sequence*/
            if (src[1] == '$')
            {
                if (pos + 1 >= cap)
                {
                    cap *= 2;
                    result = realloc(result, cap);
                }
                result[pos++] = '$';
                src += 2;
                continue;
            }
            /*Extract and expand variable name*/
            size_t var_len;
            char *name = extract_var_name(src, &var_len);
            char *exp_name = variable_expand(name);
            const char *value = variable_get(exp_name);
            /*Insert variable value if found*/
            if (value)
            {
                size_t val_len = strlen(value);
                while (pos + val_len >= cap)
                {
                    cap *= 2;
                    result = realloc(result, cap);
                }
                strcpy(result + pos, value);
                pos += val_len;
            }
            free(name);
            free(exp_name);
            src += var_len;
        }
        else
        {
            /*Copy regular character*/
            if (pos + 1 >= cap)
            {
                cap *= 2;
                result = realloc(result, cap);
            }
            result[pos++] = *src++;
        }
    }
    result[pos] = '\0';
    return result;
}

/*Free all variables and cleanup memory*/
void variable_free(void)
{
    while (vars_head)
    {
        variable_t *tmp = vars_head;
        vars_head = vars_head->next;
        free(tmp->name);
        free(tmp->value);
        free(tmp);
    }
}
