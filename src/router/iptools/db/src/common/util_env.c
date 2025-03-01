/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"

/*
 * __db_util_env_create --
 *	Create a new environment handle and initialize it for error reporting
 *	and encryption.
 *
 * PUBLIC: int __db_util_env_create __P((DB_ENV **, const char *,
 * PUBLIC:     const char *, const char *));
 */
int
__db_util_env_create(dbenvp, progname, password, msgpfx)
	DB_ENV **dbenvp;
	const char *progname;
	const char *password;
	const char *msgpfx;
{
	DB_ENV *dbenv;
	int ret;

	if ((ret = db_env_create(dbenvp, 0)) != 0) {
		fprintf(stderr, "%s: db_env_create: %s\n",
		    progname, db_strerror(ret));
		return (ret);
	}

	dbenv = *dbenvp;
	dbenv->set_errfile(dbenv, stderr);
	dbenv->set_errpfx(dbenv, progname);
	if (msgpfx != NULL)
		dbenv->set_msgpfx(dbenv, msgpfx);

	if (password != NULL && (ret = dbenv->set_encrypt(dbenv,
	    password, DB_ENCRYPT_AES)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->set_encrypt");
		return (ret);
	}

	return (0);
}

/*
 * __db_util_env_open --
 *	Attach to an existing environment. If that fails, create a private one
 *	if that is useful. Return in is_privatep whether the opened environment
 *	is private.
 *
 * PUBLIC: int __db_util_env_open __P((DB_ENV *, const char *, int, int,
 * PUBLIC:     int, u_int32_t, int *));
 */
int
__db_util_env_open(dbenv, home, attach_flags, allow_private, private_flags,
    private_cachesize, is_privatep)
	DB_ENV *dbenv;
	const char *home;
	int attach_flags;
	int allow_private;
	int private_flags;
	u_int32_t private_cachesize;
	int *is_privatep;
{
	u_int32_t aflags, pflags;
	int ret;

	/* Add flags common to all environments opened by utility programs. */
	aflags = attach_flags | DB_USE_ENVIRON;
	pflags = private_flags | DB_CREATE | DB_PRIVATE | DB_USE_ENVIRON;

	if (is_privatep != NULL)
		*is_privatep = 0;

	/* First, try to attach to an existing environment. */
	if ((ret = dbenv->open(dbenv, home, aflags, 0)) != 0 &&
	    ret != DB_VERSION_MISMATCH && ret != DB_REP_LOCKOUT &&
	    allow_private) {
		/* Next, if a private environment works, try to create one. */
		if (is_privatep != NULL)
			*is_privatep = 1;
		/* Set the cache size for the private environment. */
		if (private_cachesize != 0 && (ret = dbenv->set_cachesize(
		    dbenv, 0, private_cachesize, 1)) != 0) {
			dbenv->err(dbenv, ret, "DB_ENV->set_cachesize");
			return (ret);
		}
		/* Create the private environment now. */
		ret = dbenv->open(dbenv, home, pflags, 0);
	}

	if (ret != 0)
		dbenv->err(dbenv, ret, "DB_ENV->open: %s", home);
	return (ret);
}
