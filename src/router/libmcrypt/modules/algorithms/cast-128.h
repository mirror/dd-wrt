/*
 * $Id: cast-128.h,v 1.1.1.1 2000/05/22 13:08:18 nmav Exp $
 *
 *	CAST-128 in C
 *	Written by Steve Reid <sreid@sea-to-sky.net>
 *	100% Public Domain - no warranty
 *	Released 1997.10.11
 */

#ifndef _CAST_H_INCLUDED
#define _CAST_H_INCLUDED

#define CAST_MIN_KEYSIZE 5
#define CAST_MAX_KEYSIZE 16
#define CAST_BLOCKSIZE 8

#define CAST_SMALL_KEY 10
#define CAST_SMALL_ROUNDS 12
#define CAST_FULL_ROUNDS 16

typedef struct cast_key {
	word32 xkey[32];	/* Key, after expansion */
	unsigned rounds;		/* Number of rounds to use, 12 or 16 */
} CAST_KEY;

#endif /* ifndef _CAST_H_INCLUDED */

