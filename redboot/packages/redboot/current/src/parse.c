//==========================================================================
//
//      parse.c
//
//      RedBoot command line parsing routine
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include CYGHWR_MEMORY_LAYOUT_H
#include <cyg/hal/hal_tables.h>

// Define table boundaries
extern struct cmd __RedBoot_CMD_TAB__[], __RedBoot_CMD_TAB_END__;

//
// Scan through an input line and break it into "arguments".  These
// are space delimited strings.  Return a structure which points to
// the strings, similar to a Unix program. Multiple commands in the line
// are separated by ; similar to sh. If we find a semi we stop processing the 
// line, terminate the current command with a null and return the start 
// of the next command in *line. parse() can then be called again to 
// process the next command on the line.
// Note: original input is destroyed by replacing the delimiters with 
// null ('\0') characters for ease of use.
//
struct cmd *
parse(char **line, int *argc, char **argv)
{
    char *cp = *line;
    char *pp;
    int indx = 0;
    int semi = 0;

    while (*cp) {
        // Skip leading spaces
        while (*cp && *cp == ' ') cp++;
        if (!*cp) {
            break;  // Line ended with a string of spaces
        }
	if (*cp == ';') {
            *cp = '\0';
            semi=1;
            break;
	}
        if (indx < MAX_ARGV) {
            argv[indx++] = cp;
        } else {
            diag_printf("Too many arguments - stopped at: '%s'\n", cp);
        }
        while (*cp) {
            if (*cp == ' ') {
                *cp++ = '\0';
                break;
            } else if (*cp == ';') {
                break;
	    } else if (*cp == '"') {
                // Swallow quote, scan till following one
                if (argv[indx-1] == cp) {
                    argv[indx-1] = ++cp;
                }
                pp = cp;
                while (*cp && *cp != '"') {
                    if (*cp == '\\') {
                        // Skip over escape - allows for escaped '"'
                        cp++;
                    }
                    // Move string to swallow escapes
                    *pp++ = *cp++;
                }
                if (!*cp) {
                    diag_printf("Unbalanced string!\n");
                } else {
                    if (pp != cp) *pp = '\0';
                    *cp++ = '\0';
                    break;
                }
            } else {
                cp++;
            }
        }
    }
    if (semi) {
        *line = cp + 1;
    } else {
        *line = cp;
    }
    *argc = indx;
    return cmd_search(__RedBoot_CMD_TAB__, &__RedBoot_CMD_TAB_END__, argv[0]);
}

//
// Search through a list of commands
//
struct cmd *
cmd_search(struct cmd *tab, struct cmd *tabend, char *arg)
{
    int cmd_len;
    struct cmd *cmd, *cmd2;
    // Search command table
    cmd_len = strlen(arg);
    cmd = tab;
    while (cmd != tabend) {
        if (strncasecmp(arg, cmd->str, cmd_len) == 0) {
            if (strlen(cmd->str) > cmd_len) {
                // Check for ambiguous commands here
                // Note: If there are commands which are not length-unique
                // then this check will be invalid.  E.g. "du" and "dump"
                bool first = true;
                cmd2 = tab;
                while (cmd2 != tabend) {
                    if ((cmd != cmd2) && 
                        (strncasecmp(arg, cmd2->str, cmd_len) == 0)) {
                        if (first) {
                            diag_printf("Ambiguous command '%s', choices are: %s", 
                                        arg, cmd->str);
                            first = false;
                        }
                        diag_printf(" %s", cmd2->str);
                    }
                    cmd2++;
                }
                if (!first) {
                    // At least one ambiguity found - fail the lookup
                    diag_printf("\n");
                    return (struct cmd *)0;
                }
            }
            return cmd;
        }
        cmd++;
    }
    return (struct cmd *)0;
}

void
cmd_usage(struct cmd *tab, struct cmd *tabend, char *prefix)
{
    struct cmd *cmd;

    diag_printf("Usage:\n"); 
    for (cmd = tab;  cmd != tabend;  cmd++) {
        diag_printf("  %s%s %s\n", prefix, cmd->str, cmd->usage);
    }
}

// Option processing

// Initialize option table entry (required because these entries
// may have dynamic contents, thus cannot be statically initialized)
//
void
init_opts(struct option_info *opts, char flag, bool takes_arg, 
          int arg_type, void *arg, bool *arg_set, char *name)
{
    opts->flag = flag;
    opts->takes_arg = takes_arg;
    opts->arg_type = arg_type,
    opts->arg = arg;
    opts->arg_set = arg_set;
    opts->name = name;
}

//
// Scan command line arguments (argc/argv), processing options, etc.
//
bool
scan_opts(int argc, char *argv[], int first, 
          struct option_info *opts, int num_opts, 
          void *def_arg, int def_arg_type, char *def_descr)
{
    bool ret = true;
    bool flag_ok;
    bool def_arg_set = false;
    int i, j;
    char c, *s;
    struct option_info *opt;

    if (def_arg && (def_arg_type == OPTION_ARG_TYPE_STR)) {
        *(char **)def_arg = (char *)0;
    }
    opt = opts;
    for (j = 0;  j < num_opts;  j++, opt++) {
        if (opt->arg_set) {
            *opt->arg_set = false;
        }
        if (!opt->takes_arg) {
            switch (opt->arg_type) {
            case OPTION_ARG_TYPE_NUM:
                *(int *)opt->arg = 0;
                break;
            case OPTION_ARG_TYPE_FLG:
                *(bool *)opt->arg = false;
                break;
            }
        }
    }
    for (i = first;  i < argc;  i++) {
        if (argv[i][0] == '-') {
            c = argv[i][1];
            flag_ok = false;
            opt = opts;
            for (j = 0;  j < num_opts;  j++, opt++) {
                if (c == opt->flag) {
                    if (opt->arg_set && *opt->arg_set) {
                        diag_printf("** Error: %s already specified\n", opt->name);
                        ret = false;
                    }
                    if (opt->takes_arg) {
                        if (argv[i][2] == '=') {
                            s = &argv[i][3];
                        } else {
                            s = argv[i+1];
                            i++;
                        }
                        switch (opt->arg_type) {
                        case OPTION_ARG_TYPE_NUM:
                            if (!parse_num(s, (unsigned long *)opt->arg, 0, 0)) {
                                diag_printf("** Error: invalid number '%s' for %s\n", 
                                            s, opt->name);
                                ret = false;
                            }
                            break;
                        case OPTION_ARG_TYPE_STR:
                            *(char **)opt->arg = s;
                            break;
                        }
                        *opt->arg_set = true;
                    } else {
                        switch (opt->arg_type) {
                        case OPTION_ARG_TYPE_NUM:
                            *(int *)opt->arg = *(int *)opt->arg + 1;
                            break;
                        case OPTION_ARG_TYPE_FLG:
                            *(bool *)opt->arg = true;
                            break;
                        }
                    }
                    flag_ok = true;
                    break;
                }
            }
            if (!flag_ok) {
                diag_printf("** Error: invalid flag '%c'\n", c);
                ret = false;
            }
        } else {
            if (def_arg) {
                if (def_arg_set) {
                    diag_printf("** Error: %s already specified\n", def_descr);
                    ret = false;
                }
                switch (def_arg_type) {
                case OPTION_ARG_TYPE_NUM:
                    if (!parse_num(argv[i], (unsigned long *)def_arg, 0, 0)) {
                        diag_printf("** Error: invalid number '%s' for %s\n", 
                                    argv[i], def_descr);
                        ret = false;
                    }
                    break;
                case OPTION_ARG_TYPE_STR:
                    *(char **)def_arg = argv[i];
                    break;
                }
                def_arg_set = true;
            } else {
                diag_printf("** Error: no default/non-flag arguments supported\n");
                ret = false;
            }
        }
    }
    return ret;
}

//
// Parse (scan) a number
//
bool
parse_num(char *s, unsigned long *val, char **es, char *delim)
{
    bool first = true;
    int radix = 10;
    char c;
    unsigned long result = 0;
    int digit;

    while (*s == ' ') s++;
    while (*s) {
        if (first && (s[0] == '0') && (_tolower(s[1]) == 'x')) {
            radix = 16;
            s += 2;
        }
        first = false;
        c = *s++;
        if (_is_hex(c) && ((digit = _from_hex(c)) < radix)) {
            // Valid digit
#ifdef CYGPKG_HAL_MIPS
            // FIXME: tx49 compiler generates 0x2539018 for MUL which
            // isn't any good.
            if (16 == radix)
                result = result << 4;
            else
                result = 10 * result;
            result += digit;
#else
            result = (result * radix) + digit;
#endif
        } else {
            if (delim != (char *)0) {
                // See if this character is one of the delimiters
                char *dp = delim;
                while (*dp && (c != *dp)) dp++;
                if (*dp) break;  // Found a good delimiter
            }
            return false;  // Malformatted number
        }
    }
    *val = result;
    if (es != (char **)0) {
        *es = s;
    }
    return true;
}

bool
parse_bool(char *s, bool *val)
{
    while (*s == ' ') s++;
    if ((*s == 't') || (*s == 'T')) {
        char *p = "rue";
        char *P = "RUE";
        // check for (partial) rest of the word and no extra including the
        // terminating zero.  "tru" will match; "truef" will not.
        while ( *++s ) {
            if ( *p != *s && *P != *s ) return false;
            p++; P++;
        }
        *val = true;
    } else 
    if ((*s == 'f') || (*s == 'F')) {
        char *p = "alse";
        char *P = "ALSE";
        while ( *++s ) {
            if ( *p != *s && *P != *s ) return false;
            p++; P++;
        }
        *val = false;
    } else {
        return false;
    }
    return true;
}

