#include "outils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Duplicate a string*/
char str_dup(const char *s)
{
    if (!s)
    {
        return NULL;
    }
    char dup = malloc(strlen(s) + 1);
    if (dup)
    {
        strcpy(dup, s);
    }
    return dup;
}

/*Remove leading and trailing whitespace*/
char str_trim(char *str)
{
    char end;
    while (isblank(str))
    {
        str++;
    }
    if (str == 0)
    {
        return str;
    }
    end = str + strlen(str) - 1;
    while (end > str && isblank(end))
    {
        end--;
    }
    (end + 1) = 0;
    return str;
}

/*Check if string contains the character*/
int str_contains_char(const char str, char c)
{
    return strchr(str, c) != NULL;
}

/*Create a new makefile structure*/
struct makefile makefile_create(void)
{
    struct makefile mf = calloc(1, sizeof(struct makefile));
    if (!mf)
    {
        return NULL;
    }
    mf->variables = calloc(MAX_VARS, sizeof(struct variable));
    mf->rules = calloc(MAX_RULES, sizeof(struct rule));
    if (!mf->variables || !mf->rules)
    {
        free(mf->variables);
        free(mf->rules);
        free(mf);
        return NULL;
    }
    return mf;
}

/*Clear the makefile and all the content*/
void makefile_destroy(struct makefile mf)
{
    int i;
    int j;
    if (!mf)
    {
        return;
    }
    for (i = 0; i < mf->var_count; i++)
    {
        free(mf->variables[i]->name);
        free(mf->variables[i]->value);
        free(mf->variables[i]);
    }
    free(mf->variables);
    for (i = 0; i < mf->rule_count; i++)
    {
        struct rule r = mf->rules[i];
        free(r->target);
        for (j = 0; j < r->dep_count; j++)
        {
            free(r->dependencies[j]);
        }
        free(r->dependencies);
        for (j = 0; j < r->cmd_count; j++)
        {
            free(r->commands[j]->text);
            free(r->commands[j]);
        }
        free(r->commands);
        free(r);
    }
    free(mf->rules);
    free(mf);
}

/*Add variable to the makefile*/
int makefile_add_variable(struct makefile mf, const char *name,
                          const char value)
{
    if (mf->var_count >= MAX_VARS)
    {
        return -1;
    }
    struct variable v = malloc(sizeof(struct variable));
    if (!v)
    {
        return -1;
    }
    v->name = str_dup(name);
    v->value = str_dup(value);
    if (!v->name || !v->value)
    {
        free(v->name);
        free(v->value);
        free(v);
        return -1;
    }
    mf->variables[mf->var_count++] = v;
    return 0;
}

/*Add rule to the makefile*/
int makefile_add_rule(struct makefile mf, struct rule *rule)
{
    if (mf->rule_count >= MAX_RULES)
    {
        return -1;
    }
    mf->rules[mf->rule_count++] = rule;
    return 0;
}

/*Find variable by name*/
struct variable makefile_find_variable(struct makefile *mf, const char *name)
{
    for (int i = mf->var_count - 1; i >= 0; i--)
    {
        if (strcmp(mf->variables[i]->name, name) == 0)
        {
            return mf->variables[i];
        }
    }
    return NULL;
}

/*Find non-pattern rule by target name*/
struct rule makefile_find_rule(struct makefile *mf, const char *target)
{
    for (int i = 0; i < mf->rule_count; i++)
    {
        if (!mf->rules[i]->is_pattern &&

            strcmp(mf->rules[i]->target, target) == 0)
        {
            return mf->rules[i];
        }
    }
    return NULL;
}

/*Get the first standard target*/
const char makefile_get_first_target(struct makefile *mf)
{
    for (int i = 0; i < mf->rule_count; i++)
    {
        if (!mf->rules[i]->is_pattern)
            return mf->rules[i]->target;
    }
    return NULL;
}

/*Create a new rule structure*/
struct rule rule_create(void)
{
    struct rule r = calloc(1, sizeof(struct rule));
    if (!r)
    {
        return NULL;
    }
    r->dependencies = calloc(MAX_DEPS, sizeof(char));
    r->commands = calloc(MAX_CMDS, sizeof(struct command));
    if (!r->dependencies || !r->commands)
    {
        free(r->dependencies);
        free(r->commands);
        free(r);
        return NULL;
    }
    return r;
}

/*Free rule and all the content*/
void rule_destroy(struct rule rule)
{
    int i;
    if (!rule)
    {
        return;
    }
    free(rule->target);
    for (i = 0; i < rule->dep_count; i++)
    {
        free(rule->dependencies[i]);
    }
    free(rule->dependencies);
    for (i = 0; i < rule->cmd_count; i++)
    {
        free(rule->commands[i]->text);
        free(rule->commands[i]);
    }
    free(rule->commands);
    free(rule);
}

/*Add dependency to rule*/
int rule_add_dependency(struct rule rule, const char *dep)
{
    if (rule->dep_count >= MAX_DEPS)
    {
        return -1;
    }
    rule->dependencies[rule->dep_count] = str_dup(dep);
    if (!rule->dependencies[rule->dep_count])
    {
        return -1;
    }
    rule->dep_count++;
    return 0;
}

/*Add command to rule*/
int rule_add_command(struct rule rule, const char *cmd, int silent)
{
    if (rule->cmd_count >= MAX_CMDS)
    {
        return -1;
    }
    struct command c = malloc(sizeof(struct command));
    if (!c)
    {
        return -1;
    }
    c->text = str_dup(cmd);
    c->silent = silent;
    if (!c->text)
    {
        free(c);
        return -1;
    }
    rule->commands[rule->cmd_count++] = c;
    return 0;
}
