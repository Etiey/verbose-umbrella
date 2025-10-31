#ifndef MINIMAKE_H
#define MINIMAKE_H

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

int parse_makefile(const char *filename);
void pretty_print(void);
int execute_recipe(rule_t *rule);
typedef struct rule
{
    char *target;
    char **dependencies;
    size_t dep_count;
    char **recipe;
    size_t recipe_count;
    int is_pattern;
    int is_phony;
    struct rule *next;
} rule_t;

void rules_init(void);
rule_t *rule_create(const char *target);
void rule_add(rule_t *rule);
rule_t *rule_find(const char *target);
rule_t *rule_get_default(void);
int build_target(const char *target);
void rules_free(void);

typedef struct variable
{
    char *name;
    char *value;
    struct variable *next;
} variable_t;

void variable_init(void);
void variable_set(const char *name, const char *value);
const char *variable_get(const char *name);
char *variable_expand(const char *str);
void variable_free(void);

#endif /*MINIMAKE_H*/
