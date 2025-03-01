/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"

/*
 * __os_rename --
 *	Rename a file.
 */
int
__os_rename(env, oldname, newname, silent)
	ENV *env;
	const char *oldname, *newname;
	u_int32_t silent;
{
	DB_ENV *dbenv;
	_TCHAR *toldname, *tnewname;
	int ret, retryCount;

	dbenv = env == NULL ? NULL : env->dbenv;

	if (dbenv != NULL &&
	    FLD_ISSET(dbenv->verbose, DB_VERB_FILEOPS | DB_VERB_FILEOPS_ALL))
		__db_msg(env, DB_STR_A("0036", "fileops: rename %s to %s",
		    "%s %s"), oldname, newname);

	TO_TSTRING(env, oldname, toldname, ret);
	if (ret != 0)
		return (ret);
	TO_TSTRING(env, newname, tnewname, ret);
	if (ret != 0) {
		FREE_STRING(env, toldname);
		return (ret);
	}

	if (lstrcmp(toldname, tnewname) == 0) {
		FREE_STRING(env, tnewname);
		FREE_STRING(env, toldname);
		return (0);
	}

	LAST_PANIC_CHECK_BEFORE_IO(env);

	retryCount = 0;
	do {
		ret = 0;

#ifndef DB_WINCE
		if (__os_is_winnt()) {
			if (!MoveFileEx(
			    toldname, tnewname, MOVEFILE_REPLACE_EXISTING))
				ret = __os_get_syserr();
		} else
#endif
		{
			/*
			 * There is no MoveFileEx for Win9x/Me/CE, so we have to
			 * do the best we can.
			 */
			if (!MoveFile(toldname, tnewname))
				ret = __os_get_syserr();

			if (__os_posix_err(ret) == EEXIST) {
				(void)DeleteFile(tnewname);
				if (!MoveFile(toldname, tnewname))
					ret = __os_get_syserr();
			}
		}
		/*
		 * MoveFile and MoveFileEx may return ERROR_ACCESS_DENIED
		 * sporadically with no apparent reason.  A workaround is
		 * to wait for a few milliseconds and retry.
		 */
		if ((ret = __os_posix_err(ret)) == EPERM)
			__os_yield(env, 0, 50);
	} while (ret == EPERM && ++retryCount < 2);

	FREE_STRING(env, tnewname);
	FREE_STRING(env, toldname);

	if (ret != 0) {
		(void)USR_ERR(env, ret);
		if (silent == 0)
			__db_syserr(env, __os_get_syserr(), DB_STR_A("0037",
			    "MoveFileEx %s %s", "%s %s"), oldname, newname);
	}

	return (ret);
}
