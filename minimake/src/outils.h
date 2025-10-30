#ifndef OUTILS_H
#define OUTILS_H

#define MAX_MAKEFILES 128
#define MAX_TARGETS 128
#define MAX_RULES 1024
#define MAX_VARS 1024
#define MAX_DEPS 128
#define MAX_CMDS 256
#define MAX_LINE 4096

/* Configuration structure */
struct config
{
    char *makefiles[MAX_MAKEFILES];
    int makefile_count;
    char *targets[MAX_TARGETS];
    int target_count;
    int show_help;
    int pretty_print;
};

/* Variable structure */
struct variable
{
    char *name;
    char *value;
};

/* Command structure */
struct command
{
    char *text;
    int silent;
};

/* Rule structure */
struct rule
{
    char *target;
    char **dependencies;
    int dep_count;
    struct command **commands;
    int cmd_count;
    int is_pattern;
    int is_phony;
};

/* Makefile structure */
struct makefile
{
    struct variable **variables;
    int var_count;
    struct rule **rules;
    int rule_count;
};

/* String utilities */
char *str_trim(char *str);
char *str_dup(const char *s);
int str_contains_char(const char *str, char c);
char *expand_variables(struct makefile *mf, const char *str);

/* Makefile management */
struct makefile *makefile_create(void);
void makefile_destroy(struct makefile *mf);
int makefile_add_variable(struct makefile *mf, const char *name,
                          const char *value);
int makefile_add_rule(struct makefile *mf, struct rule *rule);
struct variable *makefile_find_variable(struct makefile *mf, const char *name);
struct rule *makefile_find_rule(struct makefile *mf, const char *target);
const char *makefile_get_first_target(struct makefile *mf);

/* Rule utilities */
struct rule *rule_create(void);
void rule_destroy(struct rule *rule);
int rule_add_dependency(struct rule *rule, const char *dep);
int rule_add_command(struct rule *rule, const char *cmd, int silent);

#endif /* OUTILS_H */
