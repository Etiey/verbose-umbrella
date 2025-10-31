#ifndef VARIABLES_H
#define VARIABLES_H

typedef struct variable {
    char *name;
    char *value;
    struct variable *next;
} variable_t;

void variable_init(void);
void variable_set(const char *name, const char *value);
const char *variable_get(const char *name);
char *variable_expand(const char *str);
void variable_free(void);

#endif /*VARUABLES_H*/
