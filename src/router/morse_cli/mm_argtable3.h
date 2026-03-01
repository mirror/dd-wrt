/*
 * Copyright 2023 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */
#pragma once

#include <stdlib.h>
#include <string.h>
#include "utilities.h"
#include "argtable3/argtable3.h"

extern char TOOL_NAME[];

#define MM_ARGTABLE_ENABLE_REGEX "(enable|disable|1|0)"
#define MM_ARGTABLE_ENABLE_DATATYPE "{enable|disable}"

struct mm_argtable {
    int count;
    const char *desc;
    struct arg_lit *help;
    struct arg_end *end;
    void **argtable;
};

void mctrl_print(const char* format, ...);

/* Returns 1 if any of the given argtables parsed --help in their argument list,
 * otherwise returns 0
 */
static inline int mm_check_help_argtable(struct mm_argtable * tables[], size_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (tables[i]->help->count)
        {
            return 1;
        }
    }
    return 0;
}

static inline void mm_short_help_argtable(const char *name, struct mm_argtable *mm_args)
{
    mctrl_print("    %-26s%s\n", name, mm_args->desc);
}

static inline void mm_help_argtable(const char *name, struct mm_argtable *mm_args)
{
    mctrl_print("    %s", name);
    arg_print_syntax(stdout, mm_args->argtable, "\n");
    if (mm_args->desc)
    {
        mctrl_print("        %s\n", mm_args->desc);
    }
    arg_print_glossary(stdout, mm_args->argtable, "        %-30s%s\n");
}

static inline void mm_help_main_argtable(struct mm_argtable *mm_args)
{
    mctrl_print("%s", TOOL_NAME);
    arg_print_syntax(stdout, mm_args->argtable, "\n");
    if (mm_args->desc)
        mctrl_print("    %s\n", mm_args->desc);
    arg_print_glossary(stdout, mm_args->argtable, "        %-30s%s\n");
    return;
}

static inline int mm_parse_argtable_noerror(const char *name, struct mm_argtable *mm_args,
                                            int argc, char **argv)
{
    int nerrors;

    nerrors = arg_parse(argc, argv, mm_args->argtable);

    if (mm_args->help->count > 0)
    {
        mm_help_argtable(name, mm_args);
        return -1;
    }
    return nerrors;
}

static inline int mm_parse_argtable(const char *name, struct mm_argtable *mm_args,
                                    int argc, char **argv)
{
    int nerrors = mm_parse_argtable_noerror(name, mm_args, argc, argv);

    if (nerrors > 0)
    {
        arg_print_errors(stdout, mm_args->end, name != NULL ? name : TOOL_NAME);
        mctrl_print("Try %s --help for more information\n", TOOL_NAME);
    }

    return nerrors;
}

static inline void mm_print_missing_argument(struct arg_hdr *arg)
{
    mctrl_print("Missing argument: ");

    if (arg->shortopts)
    {
        mctrl_print("-%s", arg->shortopts);
    }
    if (arg->shortopts && arg->longopts)
    {
        mctrl_print("/");
    }
    if (arg->longopts)
    {
        mctrl_print("--%s", arg->longopts);
    }
    if (arg->shortopts || arg->longopts)
    {
        mctrl_print(" ");
    }
    if (arg->datatype)
    {
        mctrl_print("%s", arg->datatype);
    }
    mctrl_print("\n");
}

static inline void mm_free_argtable(struct mm_argtable *mm_args)
{
    arg_freetable(mm_args->argtable, mm_args->count);
    free(mm_args->argtable);
}

// NOLINT(-whitespace/comma)
#define MM_INIT_ARGTABLE(_argtable, _desc, ...)                            \
    do {                                                                   \
        void *tmp_table[] = {                                              \
            (_argtable)->help = arg_lit0("h", "help", NULL),               \
            ##__VA_ARGS__,                                                 \
            (_argtable)->end = arg_end(20)                                 \
    };                                                                     \
        (_argtable)->desc = (_desc);                                       \
        (_argtable)->argtable = malloc(sizeof(tmp_table));                 \
        memcpy((_argtable)->argtable, tmp_table, sizeof(tmp_table));       \
        (_argtable)->count = sizeof(tmp_table) / sizeof((_argtable)->end); \
    } while (0)
// NOLINT(+whitespace/comma)
