#ifndef PARSER_H
#define PARSER_H

#include "outils.h"

/*Helper to expand variables in a string*/
char *expand_variables(struct makefile *mf, const char *str);

static int is_comment_line(const char *line);

static int is_recipe_line(const char *line);

static int find_first_unquoted(const char *line, char c);

/*Parse a makefile*/
int parse_makefile(struct makefile *mf, const char *filename, int is_first);

/*Pretty print the makefile*/
void pretty_print_makefile(struct makefile *mf);

#endif /*PARSER_H*/
