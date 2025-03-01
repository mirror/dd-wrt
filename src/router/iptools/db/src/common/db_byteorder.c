/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"
#include "dbinc/db_swap.h"

/*
 * __db_isbigendian --
 *	Return 1 if big-endian (Motorola and Sparc), not little-endian
 *	(Intel and Vax).  We do this work at run-time, rather than at
 *	configuration time so cross-compilation and general embedded
 *	system support is simpler.
 *
 * PUBLIC: int __db_isbigendian __P((void));
 */
int
__db_isbigendian()
{
	union {					/* From Harbison & Steele.  */
		long l;
		char c[sizeof(long)];
	} u;

	u.l = 1;
	return (u.c[sizeof(long) - 1] == 1);
}

/*
 * __db_byteorder --
 *	Return if we need to do byte swapping, checking for illegal
 *	values.
 *
 * PUBLIC: int __db_byteorder __P((ENV *, int));
 */
int
__db_byteorder(env, lorder)
	ENV *env;
	int lorder;
{
	switch (lorder) {
	case 0:
		break;
	case 1234:
		if (!F_ISSET(env, ENV_LITTLEENDIAN))
			return (DB_SWAPBYTES);
		break;
	case 4321:
		if (F_ISSET(env, ENV_LITTLEENDIAN))
			return (DB_SWAPBYTES);
		break;
	default:
		__db_errx(env, DB_STR("0041",
	    "unsupported byte order, only big and little-endian supported"));
		return (EINVAL);
	}
	return (0);
}

/*
 * __db_needswap --
 *	Return if a database requires byte swapping based on its magic value.
 *
 * PUBLIC: int __db_needswap __P((u_int32_t));
 */
int
__db_needswap(magic)
	u_int32_t magic;
{
	int ret;

	ret = 0;

swap_retry:
	switch (magic) {
	case DB_BTREEMAGIC:
	case DB_HASHMAGIC:
	case DB_HEAPMAGIC:
	case DB_QAMMAGIC:
	case DB_RENAMEMAGIC:
		return (ret);
	default:
		if (ret == DB_SWAPBYTES)
			/* It's already been swapped, so it's invalid. */
			return (EINVAL);
		M_32_SWAP(magic);
		ret = DB_SWAPBYTES;
		goto swap_retry;
	}
}
