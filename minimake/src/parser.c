#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "outils.h"

/*Helper to expand variables in a string*/
char *expand_variables(struct makefile *mf, const char *str)
{
    return str_dup(str);
}

static int is_comment_line(const char *line)
{
    const char *p = line;
    while (isblank(*p))
    {
        p++;
    }
    return *p == '#' || *p == '\0' || *p == '\n';
}

static int is_recipe_line(const char *line)
{
    return line[0] == '\t';
}

static int find_first_unquoted(const char *line, char c)
{
    int i = 0;
    while (line[i])
    {
        if (line[i] == c)
        {
            return i;
        }
        i++;
    }
    return -1;
}

int parse_makefile(struct makefile *mf, const char *filename, int is_first)
{
    FILE *fp = fopen(filename, "r");
    char *line = NULL;
    size_t len = 0;
    size_t nread;
    struct rule *current_rule = NULL;
    if (!fp)
    {
        if (!is_first)
        {
            fprintf(stderr, "minimake: %s: No such file or directory\n",
                    filename);
            /*Treat as target*/
            struct rule *r = rule_create();
            if (r)
            {
                r->target = str_dup(filename);
                makefile_add_rule(mf, r);
            }
        }
        else
        {
            fprintf(stderr, "minimake: %s: No such file or directory\n",
                    filename);
            return 2;
        }
        return 0;
    }
    while ((nread = getline(&line, &len, fp)) != -1)
    {
        /*Remove newline*/
        if (nread > 0 && line[nread - 1] == '\n')
        {
            line[nread - 1] = '\0';
        }
        /*Skip empty lines and comments*/
        if (!current_rule && is_comment_line(line))
        {
            continue;
        }
        /*Check if this is a recipe line*/
        if (is_recipe_line(line))
        {
            if (!current_rule)
            {
                fprintf(stderr,
                        "minimake: *** command outside of rule. Stop.\n");
                free(line);
                fclose(fp);
                return 2;
            }
            /*Parse command*/
            char *cmd = line + 1;
            int silent = 0;
            /*Trim leading spaces after tab*/
            while (isblank(*cmd))
            {
                cmd++;
            }
            /*Check for @ prefix*/
            if (*cmd == '@')
            {
                silent = 1;
                cmd++;
            }
            if (rule_add_command(current_rule, cmd, silent) != 0)
            {
                fprintf(stderr, "minimake: *** Memory error. Stop.\n");
                free(line);
                fclose(fp);
                return 2;
            }
            continue;
        }
        /*Not a recipe line, so finish current rule*/
        if (current_rule)
        {
            makefile_add_rule(mf, current_rule);
            current_rule = NULL;
        }
        /*Skip comments and empty lines*/
        if (is_comment_line(line))
        {
            continue;
        }
        /*Check for variable or rule*/
        int colon_pos = find_first_unquoted(line, ':');
        int equals_pos = find_first_unquoted(line, '=');
        if (equals_pos != -1 && (colon_pos == -1 || equals_pos < colon_pos))
        {
            /*Variable definition*/
            line[equals_pos] = '\0';
            char *name = str_trim(line);
            char *value = str_trim(line + equals_pos + 1);
            if (strlen(name) > 0)
            {
                if (makefile_add_variable(mf, name, value) != 0)
                {
                    fprintf(stderr, "minimake: *** Memory error. Stop.\n");
                    free(line);
                    fclose(fp);
                    return 2;
                }
            }
        }
        else if (colon_pos != -1)
        {
            /*Rule definition*/
            line[colon_pos] = '\0';
            char *target_str = str_trim(line);
            char *deps_str = str_trim(line + colon_pos + 1);
            if (strlen(target_str) == 0)
            {
                /*Ignore if empty target*/
                continue;
            }
            current_rule = rule_create();
            if (!current_rule)
            {
                fprintf(stderr, "minimake: *** Memory error. Stop.\n");
                free(line);
                fclose(fp);
                return 2;
            }
            current_rule->target = str_dup(target_str);
            current_rule->is_pattern = strchr(target_str, '%') != NULL;
            /*Parse dependencies*/
            char *dep = strtok(deps_str, " \t");
            while (dep)
            {
                if (strlen(dep) > 0)
                {
                    rule_add_dependency(current_rule, dep);
                }
                dep = strtok(NULL, " \t");
            }
        }
    }
    /*Add last rule if any*/
    if (current_rule)
    {
        makefile_add_rule(mf, current_rule);
    }
    free(line);
    fclose(fp);
    return 0;
}

void pretty_print_makefile(struct makefile *mf)
{
    int i;
    int j;
    /* Print variables */
    if (mf->var_count > 0)
    {
        printf("#variables\n");
        for (i = 0; i < mf->var_count; i++)
        {
            printf("'%s' = '%s'\n", mf->variables[i]->name,
                   mf->variables[i]->value);
        }
    }
    /* Print rules */
    if (mf->rule_count > 0)
    {
        printf("#rules\n");
        for (i = 0; i < mf->rule_count; i++)
        {
            struct rule *r = mf->rules[i];
            printf("(%s):", r->target);
            for (j = 0; j < r->dep_count; j++)
            {
                printf(" [%s]", r->dependencies[j]);
            }
            printf("\n");
            for (j = 0; j < r->cmd_count; j++)
            {
                if (r->commands[j]->silent)
                {
                    printf("'@%s'\n", r->commands[j]->text);
                }
                else
                {
                    printf("'%s'\n", r->commands[j]->text);
                }
            }
        }
    }
}
