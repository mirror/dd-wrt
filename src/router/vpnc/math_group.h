/* borrowed from isakmpd-20030718 (-; */

/*	$OpenBSD: math_group.h,v 1.7 2003/06/03 14:28:16 ho Exp $	*/
/*	$EOM: math_group.h,v 1.7 1999/04/17 23:20:40 niklas Exp $	*/

/*
 * Copyright (c) 1998 Niels Provos.  All rights reserved.
 * Copyright (c) 1999 Niklas Hallqvist.  All rights reserved.
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

#ifndef __MATH_GROUP_H__
#define __MATH_GROUP_H__

#include <gcrypt.h>

enum groups {
	MODP  /* F_p, Z modulo a prime */
};

#define OAKLEY_GRP_1	1
#define OAKLEY_GRP_2	2
#define OAKLEY_GRP_5	3

/*
 * The group on which diffie hellmann calculations are done.
 */

/* Description of F_p for Boot-Strapping */

struct modp_dscr {
	int id;
	int bits; /* Key Bits provided by this group */
	const char *prime; /* Prime */
	const char *gen; /* Generator */
};

struct modp_group {
	gcry_mpi_t gen; /* Generator */
	gcry_mpi_t p; /* Prime */
	gcry_mpi_t a, b, c, d;
};

struct group {
	enum groups type;
	int id; /* Group ID */
	int bits; /* Number of key bits provided by this group */
	struct modp_group *group;
	const struct modp_dscr *group_dscr;
	void *a, *b, *c, *d;
	void *gen; /* Group Generator */
	int (*getlen) (struct group *);
	void (*getraw) (struct group *, void *, unsigned char *);
	int (*setraw) (struct group *, void *, unsigned char *, int);
	int (*setrandom) (struct group *, void *);
	int (*operation) (struct group *, void *, void *, void *);
};

/* Prototypes */

void group_init(void);
void group_free(struct group *);
struct group *group_get(int);

#endif /* _MATH_GROUP_H_ */
