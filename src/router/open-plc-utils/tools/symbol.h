/*====================================================================*
 *
 *   symbol.h - symbol table definitions and declarations;
 *
 *   this file is a subset of the original with some additions not in
 *   the original;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SYMBOL_HEADER
#define SYMBOL_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"

/*====================================================================*
 *   functions;
 *--------------------------------------------------------------------*/

char const * synonym (char const * term, const struct _term_ list [], size_t size);
void assist (char const * name, char const * type, const struct _code_ list [], size_t size);
signed lookup (char const * name, struct _code_ const list [], size_t size);
char const * reword (code_t code, struct _code_ const list [], size_t size);
void expect (const struct _code_ list [], size_t size, FILE *);
char const * typename (struct _type_ const list [], size_t size, type_t type, char const * name);
char const * codename (struct _code_ const list [], size_t size, code_t code, char const * name);
void codelist (const struct _code_ list [], size_t size, char const * comma, char const * quote, FILE *);
void typelist (const struct _type_ list [], size_t size, char const * comma, char const * quote, FILE *);
void termlist (const struct _term_ list [], size_t size, char const * comma, char const * quote, FILE *);
signed getargv (signed argc, char const * argv []);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

