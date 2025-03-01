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
 * __dbt_usercopy --
 *	Take a copy of the user's data, if a callback is supplied.
 *
 * PUBLIC: int __dbt_usercopy __P((ENV *, DBT *));
 */
int
__dbt_usercopy(env, dbt)
	ENV *env;
	DBT *dbt;
{
	void *buf;
	int ret;

	if (dbt == NULL || !F_ISSET(dbt, DB_DBT_USERCOPY) || dbt->size == 0 ||
	    dbt->data != NULL)
		return (0);

	buf = NULL;
	if ((ret = __os_umalloc(env, dbt->size, &buf)) != 0 ||
	    (ret = env->dbt_usercopy(dbt, 0, buf, dbt->size,
	    DB_USERCOPY_GETDATA)) != 0)
		goto err;
	dbt->data = buf;

	return (0);

err:	if (buf != NULL) {
		__os_ufree(env, buf);
		dbt->data = NULL;
	}

	return (ret);
}

/*
 * __dbt_userfree --
 *	Free a copy of the user's data, if necessary.
 *
 * PUBLIC: void __dbt_userfree __P((ENV *, DBT *, DBT *, DBT *));
 */
void
__dbt_userfree(env, key, pkey, data)
	ENV *env;
	DBT *key, *pkey, *data;
{
	if (key != NULL &&
	    F_ISSET(key, DB_DBT_USERCOPY) && key->data != NULL) {
		__os_ufree(env, key->data);
		key->data = NULL;
	}
	if (pkey != NULL &&
	    F_ISSET(pkey, DB_DBT_USERCOPY) && pkey->data != NULL) {
		__os_ufree(env, pkey->data);
		pkey->data = NULL;
	}
	if (data != NULL &&
	    F_ISSET(data, DB_DBT_USERCOPY) && data->data != NULL) {
		__os_ufree(env, data->data);
		data->data = NULL;
	}
}

/*
 * __dbt_defcmp --
 *	The default DBT comparison routine that returns < 0, == 0, or > 0.
 *
 *	Keep track of how far along in the two keys we find matching
 *	characters, and use that as an offset into the keys to begin
 *	future comparisons. This will save us the overhead of always
 *	starting the comparisons on the first character.
 *
 * PUBLIC: int __dbt_defcmp __P((DB *, const DBT *, const DBT *, size_t *));
 */
int
__dbt_defcmp(dbp, a, b, locp)
	DB *dbp;
	const DBT *a, *b;
	size_t *locp;
{
	size_t len, i, start;
	u_int8_t *p1, *p2;

	COMPQUIET(dbp, NULL);
	start = (locp == NULL ? 0 : *locp);
	/*
	 * Returns:
	 *	< 0 if a is < b
	 *	= 0 if a is = b
	 *	> 0 if a is > b
	 *
	 * We start the comparison from 'locp' and store the last match
	 * location in 'locp'.
	 *
	 * !!!
	 * If a size_t doesn't fit into a long, or if the difference between
	 * any two characters doesn't fit into an int, this routine can lose.
	 * What we need is a signed integral type that's guaranteed to be at
	 * least as large as a size_t, and there is no such thing.
	 */
	p1 = (u_int8_t *)a->data + start;
	p2 = (u_int8_t *)b->data + start;
	len = a->size > b->size ? b->size : a->size;
	for (i = start; i < len; ++p1, ++p2, ++i)
		if (*p1 != *p2) {
			if (locp != NULL)
				*locp = i;
			return (*p1 < *p2 ? -1 : 1);
		}
	if (locp != NULL)
		*locp = len;
	return (a->size == b->size ? 0 : (a->size < b->size ? -1 : 1));
}
