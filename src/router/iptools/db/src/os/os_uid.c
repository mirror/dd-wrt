/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2001, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"

/*
 * __os_unique_id --
 *	Return a unique 32-bit value.
 *
 * PUBLIC: void __os_unique_id __P((ENV *, u_int32_t *));
 */
void
__os_unique_id(env, idp)
	ENV *env;
	u_int32_t *idp;
{
	DB_ENV *dbenv;
	db_timespec v;
	pid_t pid;
	u_int32_t id;

	dbenv = env == NULL ? NULL : env->dbenv;

	/*
	 * Our randomized value is comprised of our process ID, the current
	 * time of day and a stack address, all XOR'd together.
	 */
	__os_id(dbenv, &pid, NULL);
	__os_gettime(env, &v, 0);

	id = (u_int32_t)pid ^
	    (u_int32_t)v.tv_sec ^ (u_int32_t)v.tv_nsec ^ P_TO_UINT32(&pid);

	if (DB_GLOBAL(random_seeded) == 0)
		__os_srandom(id);
	id ^= __os_random();

	*idp = id;
}

/*
 * __os_srandom --
 *	Set the random number generator seed for BDB.
 *
 * PUBLIC: void __os_srandom __P((u_int));
 */
void
__os_srandom(seed)
	u_int seed;
{
	DB_GLOBAL(random_seeded) = 1;
#ifdef HAVE_RANDOM_R
	(void)initstate_r(seed, &DB_GLOBAL(random_state[0]),
	    sizeof(DB_GLOBAL(random_state)), &DB_GLOBAL(random_data));
	(void)srandom_r(seed, &DB_GLOBAL(random_data));
#elif defined(HAVE_RANDOM)
	srandom(seed);
#else
	srand(seed);
#endif
}

/*
 * __os_random --
 *	Return the next the random number generator for BDB.
 *
 * PUBLIC: u_int __os_random __P((void));
 */
u_int
__os_random()
{
#ifdef HAVE_RANDOM_R
	int32_t result;
#endif
	if (DB_GLOBAL(random_seeded) == 0)
		__os_srandom((u_int)time(NULL));
#ifdef HAVE_RANDOM_R
	random_r(&DB_GLOBAL(random_data), &result);
	return ((u_int)result);
#elif defined(HAVE_RANDOM)
	return ((u_int)random());
#else
	return ((u_int)rand());
#endif
}
