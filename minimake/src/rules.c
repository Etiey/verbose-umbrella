#include "rules.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "executor.h"
#include "utils.h"
#include "variables.h"

/*Global variables for rule management*/
static rule_t *rules_head = NULL; /*Head of rules list*/
static rule_t *phony_rule = NULL; /*Special .PHONY rule*/
static char **built_targets = NULL; /*Targets already built*/
static size_t built_count = 0; /*Number of built targets*/

/*Initialize the rules system*/
void rules_init(void)
{
    rules_head = NULL;
    phony_rule = NULL;
    built_targets = NULL;
    built_count = 0;
}

/*Create a new rule structure*/
rule_t *rule_create(const char *target)
{
    rule_t *r = calloc(1, sizeof(rule_t));
    if (!r)
    {
        error_exit("Memory allocation failed");
    }
    r->target = string_duplicate(target);
    r->dependencies = NULL;
    r->dep_count = 0;
    r->recipe = NULL;
    r->recipe_count = 0;
    r->is_pattern = (strchr(target, '%') != NULL);
    r->is_phony = 0;
    r->next = NULL;
    return r;
}

/*Add a rule to the rules list*/
void rule_add(rule_t *rule)
{
    /*.PHONY is a special rule*/
    if (strcmp(rule->target, ".PHONY") == 0)
    {
        phony_rule = rule;
        return;
    }

    /*Add to front of list*/
    rule->next = rules_head;
    rules_head = rule;
}

/*Find a non-pattern rule by target name*/
rule_t *rule_find(const char *target)
{
    for (rule_t *r = rules_head; r; r = r->next)
    {
        if (!r->is_pattern && strcmp(r->target, target) == 0)
        {
            return r;
        }
    }
    return NULL;
}

/*Get the first non-pattern rule (default target)*/
rule_t *rule_get_default(void)
{
    for (rule_t *r = rules_head; r; r = r->next)
    {
        if (!r->is_pattern)
        {
            return r;
        }
    }
    return NULL;
}

/*Check if target is declared as phony*/
static int is_phony_target(const char *target)
{
    if (!phony_rule)
    {
        return 0;
    }
    /*Check if target is in .PHONY dependencies*/
    for (size_t i = 0; i < phony_rule->dep_count; i++)
    {
        if (strcmp(phony_rule->dependencies[i], target) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/*Check if target was already built this run*/
static int was_built(const char *target)
{
    for (size_t i = 0; i < built_count; i++)
    {
        if (strcmp(built_targets[i], target) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/*Mark target as built (for deduplication)*/
static void mark_built(const char *target)
{
    built_targets = realloc(built_targets, sizeof(char *) * (built_count + 1));
    if (!built_targets)
    {
        error_exit("Memory allocation failed");
    }
    built_targets[built_count++] = string_duplicate(target);
}

/*Forward declarations for mutual recursion*/
static int is_nothing_done(rule_t *rule);
static int is_up_to_date(rule_t *rule);

/*Check if all dependencies are nothing-to-be-done or up-to-date*/
static int check_deps_status(rule_t *rule)
{
    for (size_t i = 0; i < rule->dep_count; i++)
    {
        char *dep = variable_expand(rule->dependencies[i]);
        rule_t *dep_rule = rule_find(dep);
        /*Check if dependency is nothing-to-be-done or up-to-date*/
        int ntbd = dep_rule ? is_nothing_done(dep_rule) : 0;
        int utd = dep_rule ? is_up_to_date(dep_rule) : file_exists(dep);
        free(dep);
        if (!ntbd && !utd)
        {
            return 0; /*At least one dep needs building*/
        }
    }
    return 1; /*All deps are OK*/
}

/*Check if rule is "nothing to be done"*/
static int is_nothing_done(rule_t *rule)
{
    /*Must have empty recipe*/
    if (rule->recipe_count > 0)
    {
        return 0;
    }
    /*All dependencies must be nothing-to-be-done or up-to-date*/
    return check_deps_status(rule);
}

/*Check if rule is "up to date"*/
static int is_up_to_date(rule_t *rule)
{
    /*Must have a recipe*/
    if (rule->recipe_count == 0)
    {
        return 0;
    }
    /*Target file must exist*/
    if (!file_exists(rule->target))
    {
        return 0;
    }
    /*All dependencies must be OK*/
    if (!check_deps_status(rule))
    {
        return 0;
    }
    /*Target must be newer than all file dependencies*/
    for (size_t i = 0; i < rule->dep_count; i++)
    {
        char *dep = variable_expand(rule->dependencies[i]);
        if (file_exists(dep) && is_older(rule->target, dep))
        {
            free(dep);
            return 0; /*Dependency is newer*/
        }
        free(dep);
    }
    return 1; /*Target is up to date*/
}

/*Build all dependencies of a rule*/
static int build_dependencies(rule_t *rule)
{
    for (size_t i = 0; i < rule->dep_count; i++)
    {
        char *dep = variable_expand(rule->dependencies[i]);
        /*Check if dependency rule exists or file exists*/
        rule_t *dep_rule = rule_find(dep);
        if (!dep_rule && !file_exists(dep))
        {
            char msg[512];
            snprintf(msg, sizeof(msg),
                     "No rule to make target '%s', needed by '%s'", dep,
                     rule->target);
            error_exit(msg);
            free(dep);
            return 2;
        }
        /*Recursively build dependency*/
        if (dep_rule)
        {
            int ret = build_target(dep);
            if (ret != 0)
            {
                free(dep);
                return ret;
            }
        }
        free(dep);
    }
    return 0;
}

/*Build a target (main build logic)*/
int build_target(const char *target)
{
    char *exp_target = variable_expand(target);
    /*Check if already built (deduplication)*/
    if (was_built(exp_target))
    {
        rule_t *r = rule_find(exp_target);
        if (r && r->is_phony)
        {
            printf("minimake: Nothing to be done for '%s'.\n", exp_target);
        }
        else
        {
            printf("minimake: '%s' is up to date.\n", exp_target);
        }
        free(exp_target);
        return 0;
    }
    /*Find the rule*/
    rule_t *rule = rule_find(exp_target);
    if (!rule)
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "No rule to make target '%s'", exp_target);
        error_exit(msg);
        free(exp_target);
        return 2;
    }
    /*Check if target is phony*/
    rule->is_phony = is_phony_target(exp_target);
    /*Build all dependencies first*/
    if (build_dependencies(rule) != 0)
    {
        free(exp_target);
        return 2;
    }
    /*Mark as built for deduplication*/
    mark_built(exp_target);
    /*Check if nothing to be done*/
    if (is_nothing_done(rule))
    {
        printf("minimake: Nothing to be done for '%s'.\n", exp_target);
        free(exp_target);
        return 0;
    }
    /*Check if up to date*/
    if (is_up_to_date(rule))
    {
        printf("minimake: '%s' is up to date.\n", exp_target);
        free(exp_target);
        return 0;
    }
    /*Execute the recipe*/
    int ret = execute_recipe(rule);
    free(exp_target);
    return ret;
}

/*Free all rules and cleanup*/
void rules_free(void)
{
    /*Free regular rules*/
    while (rules_head)
    {
        rule_t *tmp = rules_head;
        rules_head = rules_head->next;
        free(tmp->target);
        for (size_t i = 0; i < tmp->dep_count; i++)
        {
            free(tmp->dependencies[i]);
        }
        free(tmp->dependencies);
        for (size_t i = 0; i < tmp->recipe_count; i++)
        {
            free(tmp->recipe[i]);
        }
        free(tmp->recipe);
        free(tmp);
    }
    /*Free .PHONY rule*/
    if (phony_rule)
    {
        free(phony_rule->target);
        for (size_t i = 0; i < phony_rule->dep_count; i++)
        {
            free(phony_rule->dependencies[i]);
        }
        free(phony_rule->dependencies);
        free(phony_rule);
    }
    /*Free built targets list*/
    for (size_t i = 0; i < built_count; i++)
    {
        free(built_targets[i]);
    }
    free(built_targets);
}
