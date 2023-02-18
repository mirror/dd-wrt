/* $OpenBSD: conf.h,v 1.30 2004/06/25 20:25:34 hshoexer Exp $	 */
/* $EOM: conf.h,v 1.13 2000/09/18 00:01:47 ho Exp $	 */

/*
 * Copyright (c) 1998, 1999, 2001 Niklas Hallqvist.  All rights reserved.
 * Copyright (c) 2000, 2003 Håkan Olsson.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This code was written under funding by Ericsson Radio Systems.
 */

#ifndef _CONFFILE_H_
#define _CONFFILE_H_

#include <sys/queue.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

struct conf_list_node {
	TAILQ_ENTRY(conf_list_node) link;
	char	*field;
};

struct conf_list {
	size_t	cnt;
	TAILQ_HEAD(conf_list_fields_head, conf_list_node) fields;
};

extern int      conf_begin(void);
extern int      conf_decode_base64(uint8_t *, uint32_t *, const unsigned char *);
extern int      conf_end(int, int);
extern void     conf_free_list(struct conf_list *);
extern struct sockaddr *conf_get_address(const char *, const char *);
extern struct conf_list *conf_get_list(const char *, const char *);
extern struct conf_list *conf_get_tag_list(const char *, const char *);
extern int      conf_get_num(const char *, const char *, int);
extern _Bool    conf_get_bool(const char *, const char *, _Bool);
extern char    *conf_get_str(const char *, const char *);
extern char    *conf_get_str_with_def(const char *, const char *, char *);
extern char    *conf_get_section(const char *, const char *, const char *);
extern char    *conf_get_entry(const char *, const char *, const char *);
extern int      conf_init_file(const char *);
extern void     conf_cleanup(void);
extern int      conf_match_num(const char *, const char *, int);
extern int      conf_remove(int, const char *, const char *);
extern int      conf_remove_section(int, const char *);
extern void     conf_report(FILE *);
extern int      conf_write(const char *, const char *, const char *, const char *, const char *);

extern const char *modified_by;

/*
 * Convert letter from upper case to lower case
 */
static inline void upper2lower(char *str)
{
	char c;

	while ((c = tolower(*str)))
		*str++ = c;
}


#endif				/* _CONFFILE_H_ */
