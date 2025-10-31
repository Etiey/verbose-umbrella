#ifndef RULES_H
#define RULES_H

#include <stddef.h>

typedef struct rule {
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

#endif /*RULES_H*/

