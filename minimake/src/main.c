#include "parser.h"
#include "rules.h"
#include "variables.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Structure to hold command-line options */
typedef struct {
    char *makefile;         /* -f option: makefile name */
    int pretty;             /* -p option: pretty-print mode */
    int help;               /* -h option: show help */
    char **targets;         /* List of targets to build */
    size_t target_count;    /* Number of targets */
} options_t;

/* Print help message */
static void print_help(void) {
    printf("Usage: minimake [options] [targets]\n");
    printf("Options:\n");
    printf("  -f FILE    Use FILE as makefile\n");
    printf("  -p         Pretty-print the makefile\n");
    printf("  -h         Display this help\n");
}

/* Check for empty string arguments (forbidden) */
static int check_empty_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strlen(argv[i]) == 0) {
            error_exit("empty string invalid as argument");
            return 2;
        }
    }
    return 0;
}

/* Parse command-line arguments */
static void parse_args(int argc, char **argv, options_t *opts) {
    /* Initialize options */
    opts->makefile = NULL;
    opts->pretty = 0;
    opts->help = 0;
    opts->targets = NULL;
    opts->target_count = 0;
    
    /* Process each argument */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            opts->help = 1;
        } else if (strcmp(argv[i], "-p") == 0) {
            opts->pretty = 1;
        } else if (strcmp(argv[i], "-f") == 0) {
            /* Next argument is filename */
            if (i + 1 < argc) {
                opts->makefile = argv[++i];
            }
        } else {
            /* Treat as target name */
            opts->targets = realloc(opts->targets, 
                                    sizeof(char *) * (opts->target_count + 1));
            if (!opts->targets)
                error_exit("Memory allocation failed");
            opts->targets[opts->target_count++] = argv[i];
        }
    }
}

/* Find makefile automatically (makefile, then Makefile) */
static char *find_makefile(void) {
    if (file_exists("makefile"))
        return "makefile";
    if (file_exists("Makefile"))
        return "Makefile";
    return NULL;
}

/* Main minimake logic */
static int run_minimake(options_t *opts) {
    char *makefile = opts->makefile;
    
    /* Auto-detect makefile if not specified */
    if (!makefile) {
        makefile = find_makefile();
        if (!makefile) {
            /* Different error messages based on context */
            if (opts->target_count == 0) {
                error_exit("No targets specified and no makefile found");
            } else {
                error_exit("No rule to make target");
            }
            return 2;
        }
    }
    
    /* Initialize systems */
    variable_init();
    rules_init();
    
    /* Parse the makefile */
    if (parse_makefile(makefile) != 0)
        return 2;
    
    /* Handle pretty-print mode */
    if (opts->pretty) {
        pretty_print();
        return 0;
    }
    
    /* Build targets */
    if (opts->target_count == 0) {
        /* No targets specified: build default (first) rule */
        rule_t *def = rule_get_default();
        if (!def)
            error_exit("No targets");
        return build_target(def->target);
    }
    
    /* Build each specified target in order */
    for (size_t i = 0; i < opts->target_count; i++) {
        if (build_target(opts->targets[i]) != 0)
            return 2;
    }
    
    return 0;
}

/* Main entry point */
int main(int argc, char **argv) {
    /* Check for empty arguments */
    if (check_empty_args(argc, argv) != 0)
        return 2;
    
    /* Parse command-line options */
    options_t opts;
    parse_args(argc, argv, &opts);
    
    /* Handle help option (priority over all) */
    if (opts.help) {
        print_help();
        return 0;
    }
    
    /* Run minimake */
    int ret = run_minimake(&opts);
    
    /* Cleanup */
    variable_free();
    rules_free();
    free(opts.targets);
    
    return ret;
}
