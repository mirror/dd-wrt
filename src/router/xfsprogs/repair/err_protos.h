// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

/* abort, internal error */
void  __attribute__((noreturn)) do_abort(char const *, ...)
	__attribute__((format(printf,1,2)));
/* abort, system error */
void  __attribute__((noreturn)) do_error(char const *, ...)
	__attribute__((format(printf,1,2)));
/* issue warning */
void do_warn(char const *, ...)
	__attribute__((format(printf,1,2)));
/* issue log message */
void do_log(char const *, ...)
	__attribute__((format(printf,1,2)));
