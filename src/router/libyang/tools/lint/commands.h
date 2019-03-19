/**
 * @file main.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool commands header
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#include <stdlib.h>

#include "libyang.h"

#define PROMPT "> "

struct model_hint {
	char* hint;
	struct model_hint* next;
};

typedef struct {
	char *name; /* User printable name of the function. */
	int (*func)(const char*); /* Function to call to do the command. */
	void (*help_func)(void); /* Display command help. */
	char *helpstring; /* Documentation for this function. */
} COMMAND;

LYS_INFORMAT get_schema_format(const char *path);

extern COMMAND commands[];

#endif /* COMMANDS_H_ */
