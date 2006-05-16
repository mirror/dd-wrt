/*
 * $Header: lib/et/error_message.c 1.19 05/06/20 17:59:05-04:00 tytso@think.thunk.org $
 * $Source: lib/et/SCCS/s.error_message.c $
 * $Locker: <Not implemented> $
 *
 * Copyright 1987 by the Student Information Processing Board
 * of the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose is hereby granted, provided that
 * the names of M.I.T. and the M.I.T. S.I.P.B. not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  M.I.T. and the
 * M.I.T. S.I.P.B. make no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "com_err.h"
#include "error_table.h"
#include "internal.h"

static char buffer[25];

struct et_list * _et_list = (struct et_list *) NULL;
struct et_list * _et_dynamic_list = (struct et_list *) NULL;


const char * error_message (errcode_t code)
{
    int offset;
    struct et_list *et;
    errcode_t table_num;
    int started = 0;
    char *cp;

    offset = (int) (code & ((1<<ERRCODE_RANGE)-1));
    table_num = code - offset;
    if (!table_num) {
#ifdef HAS_SYS_ERRLIST
	if (offset < sys_nerr)
	    return(sys_errlist[offset]);
	else
	    goto oops;
#else
	cp = strerror(offset);
	if (cp)
	    return(cp);
	else
	    goto oops;
#endif
    }
    for (et = _et_list; et; et = et->next) {
	if (et->table->base == table_num) {
	    /* This is the right table */
	    if (et->table->n_msgs <= offset)
		goto oops;
	    return(et->table->msgs[offset]);
	}
    }
    for (et = _et_dynamic_list; et; et = et->next) {
	if (et->table->base == table_num) {
	    /* This is the right table */
	    if (et->table->n_msgs <= offset)
		goto oops;
	    return(et->table->msgs[offset]);
	}
    }
oops:
    strcpy (buffer, "Unknown code ");
    if (table_num) {
	strcat (buffer, error_table_name (table_num));
	strcat (buffer, " ");
    }
    for (cp = buffer; *cp; cp++)
	;
    if (offset >= 100) {
	*cp++ = '0' + offset / 100;
	offset %= 100;
	started++;
    }
    if (started || offset >= 10) {
	*cp++ = '0' + offset / 10;
	offset %= 10;
    }
    *cp++ = '0' + offset;
    *cp = '\0';
    return(buffer);
}

/*
 * New interface provided by krb5's com_err library
 */
errcode_t add_error_table(const struct error_table * et)
{
	struct et_list *el;

	if (!(el = (struct et_list *) malloc(sizeof(struct et_list))))
		return ENOMEM;

	el->table = et;
	el->next = _et_dynamic_list;
	_et_dynamic_list = el;

	return 0;
}

/*
 * New interface provided by krb5's com_err library
 */
errcode_t remove_error_table(const struct error_table * et)
{
	struct et_list *el = _et_dynamic_list;
	struct et_list *el2 = 0;

	while (el) {
		if (el->table->base == et->base) {
			if (el2)	/* Not the beginning of the list */
				el2->next = el->next;
			else
				_et_dynamic_list = el->next;
			(void) free(el);
			return 0;
		}
		el2 = el;
		el = el->next;
	}
	return ENOENT;
}

/*
 * Variant of the interface provided by Heimdal's com_err library
 */
void
add_to_error_table(struct et_list *new_table)
{
	add_error_table(new_table->table);
}
