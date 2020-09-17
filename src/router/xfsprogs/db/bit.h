// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#define	bitize(s)	((s) * NBBY)
#define	bitsz(t)	bitize(sizeof(t))
#define	bitszof(x,y)	bitize(szof(x,y))
#define	byteize(s)	((s) / NBBY)
#define	bitoffs(s)	((s) % NBBY)
#define	byteize_up(s)	(((s) + NBBY - 1) / NBBY)

#define	BVUNSIGNED	0
#define	BVSIGNED	1

extern int64_t		getbitval(void *obj, int bitoff, int nbits, int flags);
extern void		setbitval(void *obuf, int bitoff, int nbits, void *ibuf);
extern int		getbit_l(char *ptr, int bit);
extern void		setbit_l(char *ptr, int bit, int val);
