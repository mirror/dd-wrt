/*
 * cfgfile.c:
 *
 * Copyright (c) 2003 DecisionSoft Ltd.
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "stringmap.h"
#include "iftop.h"
#include "options.h"
#include "cfgfile.h"

#define CONFIG_TYPE_STRING 0
#define CONFIG_TYPE_BOOL   1
#define CONFIG_TYPE_INT    2

#define MAX_CONFIG_LINE     2048

char * config_directives[] = {
	"interface", 
	"dns-resolution",
	"port-resolution",
	"filter-code",
	"show-bars", 
	"promiscuous",
	"hide-source",
	"hide-destination",
	"use-bytes", 
	"sort", 
	"line-display", 
	"show-totals", 
	"log-scale", 
	"max-bandwidth",
	"net-filter", 
	"port-display", 
	NULL
};

stringmap config;

extern options_t options ;

int is_cfgdirective_valid(const char *s) {
    char **t;
    for (t = config_directives; *t != NULL; ++t)
        if (strcmp(s, *t) == 0) return 1;
    return 0;
}

int config_init() {
    config = stringmap_new();
    return config != NULL;
}

/* read_config_file:
 * Read a configuration file consisting of key: value tuples, returning a
 * stringmap of the results. Prints errors to stderr, rather than using
 * syslog, since this file is called at program startup. Returns 1 on success
 * or 0 on failure. */
int read_config_file(const char *f, int whinge) {
    int ret = 0;
    FILE *fp;
    char *line;
    int i = 1;

    line = xmalloc(MAX_CONFIG_LINE);

    fp = fopen(f, "rt");
    if (!fp) {
        if(whinge) fprintf(stderr, "%s: %s\n", f, strerror(errno)); 
        goto fail;
    }

    while (fgets(line, MAX_CONFIG_LINE, fp)) {
        char *key, *value, *r;

        for (r = line + strlen(line) - 1; r > line && *r == '\n'; *(r--) = 0);

        /* Get continuation lines. Ugly. */
        while (*(line + strlen(line) - 1) == '\\') {
            if (!fgets(line + strlen(line) - 1, MAX_CONFIG_LINE - strlen(line), fp))
                break;
            for (r = line + strlen(line) - 1; r > line && *r == '\n'; *(r--) = 0);
        }

        /* Strip comment. */
        key = strpbrk(line, "#\n");
        if (key) *key = 0;

        /*    foo  : bar baz quux
         * key^    ^value          */
        key = line + strspn(line, " \t");
        value = strchr(line, ':');

        if (value) {
            /*    foo  : bar baz quux
             * key^  ^r ^value         */
            ++value;

            r = key + strcspn(key, " \t:");
            if (r != key) {
                item *I;
                *r = 0;

                /*    foo\0: bar baz quux
                 * key^      ^value      ^r */
                value += strspn(value, " \t");
                r = value + strlen(value) - 1;
                while (strchr(" \t", *r) && r > value) --r;
                *(r + 1) = 0;

                /* (Removed check for zero length value.) */

                /* Check that this is a valid key. */
                if (!is_cfgdirective_valid(key))
                    fprintf(stderr, "%s:%d: warning: unknown directive \"%s\"\n", f, i, key);
                else if ((I = stringmap_insert(config, key, item_ptr(xstrdup(value)))))
                    /* Don't warn of repeated directives, because they
                     * may have been specified via the command line
                     * Previous option takes precedence.
                     */
                    fprintf(stderr, "%s:%d: warning: repeated directive \"%s\"\n", f, i, key);
            }
        }

        memset(line, 0, MAX_CONFIG_LINE); /* security paranoia */

        ++i;
    }

    ret = 1;

fail:
    if (fp) fclose(fp);
    if (line) xfree(line);

    return ret;
}

int config_get_int(const char *directive, int *value) {
    stringmap S;
    char *s, *t;

    if (!value) return -1;

    S = stringmap_find(config, directive);
    if (!S) return 0;

    s = (char*)S->d.v;
    if (!*s) return -1;
    errno = 0;
    *value = strtol(s, &t, 10);
    if (*t) return -1;

    return errno == ERANGE ? -1 : 1;
}

/* config_get_float:
 * Get an integer value from a config string. Returns 1 on success, -1 on
 * failure, or 0 if no value was found. */
int config_get_float(const char *directive, float *value) {
    stringmap S;
    item *I;
    char *s, *t;

    if (!value) return -1;

    if (!(S = stringmap_find(config, directive)))
        return 0;

    s = (char*)S->d.v;
    if (!*s) return -1;
    errno = 0;
    *value = strtod(s, &t);
    if (*t) return -1;

    return errno == ERANGE ? -1 : 1;
}

/* config_get_string;
 * Get a string value from the config file. Returns NULL if it is not
 * present. */
char *config_get_string(const char *directive) {
    stringmap S;

    S = stringmap_find(config, directive);
    if (S) return (char*)S->d.v;
    else return NULL;
}

/* config_get_bool:
 * Get a boolean value from the config file. Returns false if not present. */
int config_get_bool(const char *directive) {
    char *s;

    s = config_get_string(directive);
    if (s && (strcmp(s, "yes") == 0 || strcmp(s, "true") == 0))
        return 1;
    else
        return 0;
}

/* config_get_enum:
 * Get an enumeration value from the config file. Returns false if not 
 * present or an invalid value is found. */
int config_get_enum(const char *directive, config_enumeration_type *enumeration, int *value) {
    char *s;
    config_enumeration_type *t;
    s = config_get_string(directive);
    if(s) {
        for(t = enumeration; t->name; t++) {
            if(strcmp(s,t->name) == 0) {
                *value = t->value;
                return 1;
            }
        }
        fprintf(stderr,"Invalid enumeration value \"%s\" for directive \"%s\"\n", s, directive);
    }
    return 0;
}

/* config_set_string; Sets a value in the config, possibly overriding
 * an existing value
 */
void config_set_string(const char *directive, const char* s) {
    stringmap S;

    S = stringmap_find(config, directive);
    if (S) stringmap_delete_free(S);
    stringmap_insert(config, directive, item_ptr(xstrdup(s)));
}

int read_config(char *file, int whinge_on_error) {
    void* o;

    return read_config_file(file, whinge_on_error);
}
