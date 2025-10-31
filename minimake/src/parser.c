#define _POSIX_C_SOURCE 200809L

#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rules.h"
#include "utils.h"
#include "variables.h"

/*Store parsed rules for pretty-print*/
static rule_t **parsed_rules = NULL;
static size_t parsed_count = 0;

/*Remove comments from a line (everything after #)*/
static char *remove_comment(char *line)
{
    char *hash = strchr(line, '#');
    if (hash)
    {
        *hash = '\0';
    }
    return line;
}

/*Check if line is a rule (contains : before =)*/
static int is_rule_line(const char *line)
{
    const char *colon = strchr(line, ':');
    const char *equals = strchr(line, '=');
    if (!colon)
    {
        return 0; /*No colon = not a rule*/
    }
    if (!equals)
    {
        return 1; /*Colon but no = = rule*/
    }
    return colon < equals; /*: before = = rule*/
}

/*Parse a variable definition (VAR = value)*/
static void parse_variable_def(char *line)
{
    char *equals = strchr(line, '=');
    if (!equals)
    {
        return;
    }
    /*Split at = sign*/
    *equals = '\0';
    char *name = trim_whitespace(line);
    char *value = trim_whitespace(equals + 1);
    /*Store variable*/
    variable_set(name, value);
}

/*Add a command line to a rule's recipe*/
static void add_recipe_line(rule_t *rule, const char *cmd)
{
    rule->recipe =
        realloc(rule->recipe, sizeof(char *) * (rule->recipe_count + 2));
    rule->recipe[rule->recipe_count] = string_duplicate(cmd);
    rule->recipe[rule->recipe_count + 1] = NULL;
    rule->recipe_count++;
}

/*Parse recipe lines (commands starting with tab)*/
static void parse_recipe(FILE *f, rule_t *rule)
{
    char *line = NULL;
    size_t len = 0;
    long pos = ftell(f); /*Save position before reading*/
    while (getline(&line, &len, f) != -1)
    {
        /*Recipe lines must start with tab*/
        if (line[0] != '\t')
        {
            fseek(f, pos, SEEK_SET); /*Rewind*/
            break;
        }
        /*Remove comments but keep the line*/
        char *cleaned = remove_comment(line);
        if (cleaned[0] == '\t')
        {
            add_recipe_line(rule, cleaned);
        }
        pos = ftell(f);
    }
    free(line);
}

/*Parse a rule line (target: dependencies)*/
static void parse_rule_line(char *line, FILE *f)
{
    char *colon = strchr(line, ':');
    *colon = '\0';
    /*Extract and expand target*/
    char *target = variable_expand(trim_whitespace(line));
    /*Ignore rules with empty target*/
    if (strlen(target) == 0)
    {
        free(target);
        return;
    }
    /*Create rule*/
    rule_t *rule = rule_create(target);
    free(target);
    /*Parse dependencies*/
    char *deps_str = trim_whitespace(colon + 1);
    char *exp_deps = variable_expand(deps_str);
    /*Split dependencies by whitespace*/
    rule->dependencies = split_whitespace(exp_deps, &rule->dep_count);
    free(exp_deps);
    /*Parse recipe commands*/
    parse_recipe(f, rule);
    /*Add rule to list*/
    rule_add(rule);
    /*Store for pretty-print*/
    parsed_rules = realloc(parsed_rules, sizeof(rule_t *) * (parsed_count + 1));
    parsed_rules[parsed_count++] = rule;
}

/*Main parsing function - read and parse makefile*/
int parse_makefile(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s: No such file or directory", filename);
        error_msg(msg);
        return 2;
    }
    char *line = NULL;
    size_t len = 0;
    /*Read line by line*/
    while (getline(&line, &len, f) != -1)
    {
        /*Remove comments*/
        char *cleaned = remove_comment(line);
        char *trimmed = trim_whitespace(cleaned);
        /*Skip empty lines*/
        if (strlen(trimmed) == 0)
        {
            continue;
        }
        /*Parse rule or variable*/
        if (is_rule_line(trimmed))
        {
            parse_rule_line(trimmed, f);
        }
        else if (strchr(trimmed, '='))
        {
            parse_variable_def(trimmed);
        }
    }
    free(line);
    fclose(f);
    return 0;
}

/*Pretty-print the parsed makefile (for -p option)*/
void pretty_print(void)
{
    printf("# variables\n");
    printf("\n# rules\n");
    for (size_t i = 0; i < parsed_count; i++)
    {
        rule_t *r = parsed_rules[i];
        /*Print target and dependencies*/
        printf("(%s):", r->target);
        for (size_t j = 0; j < r->dep_count; j++)
        {
            printf(" [%s]", r->dependencies[j]);
        }
        printf("\n");
        /*Print recipe commands*/
        for (size_t j = 0; j < r->recipe_count; j++)
        {
            printf("'%s'\n", r->recipe[j]);
        }
    }
}
