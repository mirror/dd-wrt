/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#ifdef HAVE_SLICES

#include "db_int.h"
#include "dbinc/db_page.h"
#include "dbinc/btree.h"
#include "dbinc/crypto.h"
#include "dbinc/fop.h"
#include "dbinc/hash.h"
#include "dbinc/heap.h"
#include "dbinc/lock.h"
#include "dbinc/mp.h"
#include "dbinc/qam.h"
#include "dbinc/slice.h"
#include "dbinc/txn.h"

/* This limits the bytes displayed for DBTs in verbose & diagnostic messages. */
#define	DB_VERB_SLICE_PRINTLEN	30

/*
 * __db_slice_open_pp --
 *	DB->open pre/post processing for sliced db.
 *
 * PUBLIC: int __db_slice_open_pp __P((DB *, DB_TXN *,
 * PUBLIC:     const char *, const char *, DBTYPE, u_int32_t, int));
 */
int
__db_slice_open_pp(dbp, txn, fname, dname, type, flags, mode)
	DB *dbp;
	DB_TXN *txn;
	const char *fname, *dname;
	DBTYPE type;
	u_int32_t flags;
	int mode;
{
	DB_THREAD_INFO *ip;
	ENV *env;
	int ret, t_ret, txn_local;
#ifdef HAVE_SLICED_REPLICATION
	int handle_check;
#endif
	/*
	 * Use the normal open for sub-databases, in-memory databases or
	 * non-sliced databases.
	 */
	if (!LF_ISSET(DB_SLICED) || dname != NULL || fname == NULL)
		return (__db_open_pp(dbp,
		    txn, fname, dname, type, flags, mode));

	txn_local = 0;
	env = dbp->env;
	ENV_ENTER(env, ip);

	/*
	 * Save the flags.  We do this here because we don't pass all of the
	 * flags down into the actual DB->open method call, we strip
	 * DB_AUTO_COMMIT at this layer.
	 */
	dbp->open_flags = flags;

	/* Save the current DB handle flags for refresh. */
	dbp->orig_flags = dbp->flags;

#ifdef HAVE_SLICED_REPLICATION
	/* Reminder: this needs to be looked at for sliced replication. */

	/* Check for replication block. */
	handle_check = IS_ENV_REPLICATED(env);
	if (handle_check &&
	    (ret = __db_rep_enter(dbp, 1, 0, IS_REAL_TXN(txn))) != 0) {
		handle_check = 0;
		goto err;
	}

	/*
	 * A replication client can't create a database, but it's convenient to
	 * allow a repmgr application to specify DB_CREATE anyway.  Thus for
	 * such an application the meaning of DB_CREATE becomes "create it if
	 * I'm a master, and otherwise ignore the flag".  A repmgr application
	 * running as master can't be sure that it won't spontaneously become a
	 * client, so there's a race condition.
	 */
	if (IS_REP_CLIENT(env) && !F_ISSET(dbp, DB_AM_NOT_DURABLE))
		LF_CLR(DB_CREATE);
#endif

	/*
	 * Create local transaction as necessary, check for consistent
	 * transaction usage.
	 */
	if (txn == NULL || IS_ENV_AUTO_COMMIT(env, txn, flags)) {
		if ((ret = __db_txn_auto_init(env, ip, &txn)) != 0)
			goto err;
		txn_local = 1;
	} else if (txn != NULL && !TXN_ON(env) &&
	    (!CDB_LOCKING(env) || !F_ISSET(txn, TXN_FAMILY))) {
		ret = __db_not_txn_env(env);
		goto err;
	}
	LF_CLR(DB_AUTO_COMMIT);

	/*
	 * We check arguments after possibly creating a local transaction,
	 * which is unusual -- the reason is some flags are illegal if any
	 * kind of transaction is in effect.
	 */
	if ((ret = __db_open_arg(dbp, txn, fname, NULL, type, flags)) == 0 &&
	    (ret =
		__db_slice_open(dbp, ip, txn, fname, type, flags, mode)) != 0)
		goto err;

	if (txn_local && (t_ret = __db_txn_auto_resolve(env,
	    txn, F_ISSET(dbp, DB_AM_CREATED), ret)) && ret == 0)
		ret = t_ret;

err:
#ifdef HAVE_SLICED_REPLICATION
	/* Release replication block. */
	if (handle_check && (t_ret = __env_db_rep_exit(env)) != 0 && ret == 0)
		ret = t_ret;
#endif

	ENV_LEAVE(env, ip);
	return (ret);
}

/*
 * __db_slice_alloc --
 *	Allocate, create, and clone the db handles of a container's db slices;
 *	do nothing if they are already allocated.
 *
 *	This also verifies the container db's slice-relevant metadata when
 *	opening an existing database, or adds it when creating the db.
 *
 *	The slices' databases are not opened here, but in __db_open_pp().
 *
 * PUBLIC: int __db_slice_alloc __P((DB *, DB_THREAD_INFO *, DB_TXN *));
 */
int
__db_slice_alloc(dbp, ip, txn)
	DB *dbp;
	DB_THREAD_INFO *ip;
	DB_TXN *txn;
{
	DB_ENV *dbenv, *slice;
	DB *sl_dbp;
	ENV *env;
	int i, ret;

	env = dbp->env;
	dbenv = env->dbenv;
	DB_ASSERT(env, dbenv->slice_cnt != 0);
	if (dbp->db_slices != NULL)
		return (0);

	/* Create a NULL terminated array of slice databases. */
	if ((ret = __os_calloc(env,
	    dbenv->slice_cnt + 1, sizeof(DB *), &dbp->db_slices)) != 0)
		return (ret);

	/* Verify or create the slice metadata. */
	if ((ret = __db_slice_metachk(dbp, ip, txn)) != 0)
		goto err;

	for (i = -1; (slice = __slice_iterate(dbenv, &i)) != NULL; ) {
		if ((ret = db_create(&dbp->db_slices[i], slice, 0)) != 0) {
			__db_err(env, ret,
			    "create of database %s slice %d", dbp->fname, i);
			goto err;
		}
		sl_dbp = dbp->db_slices[i];
		sl_dbp->db_container = dbp;

		/*
		 * Copy configuration from dbp: settings, etc. As with DB_ENV,
		 * these are sorted by the name of the DB->set_xxx() function.
		 */
		if ((ret = __db_slice_configure(dbp, sl_dbp)) != 0) {
			__db_err(env, ret,
			    "configure of \"%s\" slice %d", dbp->fname, i);
			goto err;
		}
	}

	return (0);

err:
	(void)__db_slice_free(dbp, DB_NOSYNC);
	return (ret);
}

/*
 * __db_slice_free --
 *	Free all the db handles underneath a container's db.
 *
 * PUBLIC: int __db_slice_free __P((DB *, u_int32_t));
 */
int
__db_slice_free(dbp, flags)
	DB *dbp;
	u_int32_t flags;
{
	DB *sl_dbp;
	db_slice_t  i, slice_cnt;
	int ret, t_ret;

	ret = 0;
	if (dbp->db_slices != NULL) {
		slice_cnt = dbp->dbenv->slice_cnt;
		for (i = 0; i != slice_cnt; i++)  {
			sl_dbp = dbp->db_slices[i];
			if (sl_dbp != NULL && (t_ret =
			     __db_close_pp(sl_dbp, flags)) != 0 && ret == 0)
				ret = t_ret;
		}
		__os_free(dbp->env, dbp->db_slices);
		dbp->db_slices = NULL;
	}
	return (ret);
}

/*
 * __db_slice_configure --
 *	Share the setting of a container db with one of its slices.
 *
 * PUBLIC: int __db_slice_configure __P((const DB *, DB *));
 */
int
__db_slice_configure(container, slice)
	const DB *container;
	DB *slice;
{
	int ret;

	ret = 0;
	DB_ASSERT(container->env, container->dbenv->slice_cnt != 0);

	/* Copy the customizable values inherited from the container. */
	__db_copy_config(container, slice, 1);

#ifdef HAVE_HEAP
	if (container->type == DB_HEAP) {
		((HEAP *)slice->heap_internal)->gbytes =
		    ((HEAP *)container->heap_internal)->gbytes;
		((HEAP *)slice->heap_internal)->bytes =
		    ((HEAP *)container->heap_internal)->bytes;
		((HEAP *)slice->heap_internal)->region_size =
		    ((HEAP *)container->heap_internal)->region_size;
	}
#endif

	return (ret);
}

/*
 * __db_slice_default_callback  -
 *      Default slice specification DBT constructor: use the whole key.
 *
 * PUBLIC: int __db_slice_default_callback
 * PUBLIC:     __P((const DB *, const DBT *key, DBT *));
 */
int
__db_slice_default_callback(dbp, key, slice)
	const DB *dbp;
	const DBT *key;
	DBT *slice;
{
	slice->data = key->data;
	slice->size = key->size;
	COMPQUIET(dbp, NULL);
	return (0);
}

/*
 * __db_slice_metadata
 *	Fetch or insert a single key-value pair (of string values).
 *
 *	The 'expect' DBT is either inserted (if the db is still being created
 *	or the operation is an insert) or compared to the value actually
 * 	present.
 *
 * PUBLIC: int __db_slice_metadata __P((DB *,
 * PUBLIC:     DB_THREAD_INFO *, DB_TXN *, const char *, DBT *, int));
 */
int
__db_slice_metadata(dbp, ip, txn, name, expect, insert)
	DB *dbp;
	DB_THREAD_INFO *ip;
	DB_TXN *txn;
	const char *name;
	DBT *expect;
	int insert;
{
	DBT key, actual;
	ENV *env;
	int ret;
	char actual_buf[DB_MAXPATHLEN];

	env = dbp->env;

	DB_INIT_DBT(key, name, strlen(name));
	if (F_ISSET(dbp, DB_AM_CREATED) || insert != 0) {
		if ((ret = __db_put(dbp, ip, txn, &key, expect, 0)) != 0)
			__db_err(env, ret,
			    "Database %s could not insert slice metadata(%s)",
			    dbp->fname, name);
	} else {
		DB_INIT_DBT_USERMEM(actual, actual_buf, sizeof(actual_buf));
		if ((ret = __db_get(dbp, ip, txn, &key, &actual, 0)) != 0) {
			ret = USR_ERR(env, DB_SLICE_CORRUPT);
			__db_err(env, ret, DB_STR_A("0787",
			    "Database %s has no metadata \"%s\"", "%s %s"),
			    dbp->fname, name);
		}
		else if (__dbt_defcmp(dbp, &actual, expect, NULL) != 0) {
			/*
			 * The value isn't exactly what was expected. Usually
			 * that says db has corrupt metadata, but if this is the
			 * "version", an upgrade could be done. If that is ever
			 * needed this could copy the actual value back into the
			 * the passed-in DBT, for the caller to do as it wishes.
			 */
			ret = USR_ERR(env, DB_SLICE_CORRUPT);
		}
	}
	return (ret);
}

/*
 * __db_slice_fileid_metachk --
 *	Verify or insert the fileid metadata for a slice.
 *
 * PUBLIC: int __db_slice_fileid_metachk
 * PUBLIC:     __P((DB *, DB_THREAD_INFO *, DB_TXN *, db_slice_t, int));
 */
int
__db_slice_fileid_metachk(dbp, ip, txn, id, insert)
	DB *dbp;
	DB_THREAD_INFO *ip;
	DB_TXN *txn;
	db_slice_t id;
	int insert;
{
	DBT value;
	char fileid_name[sizeof(DB_SLICE_METADATA_FILEID_FMT)];
	int ret;

	snprintf(fileid_name, sizeof(fileid_name),
	    DB_SLICE_METADATA_FILEID_FMT, id);
	DB_INIT_DBT_USERMEM(value, dbp->db_slices[id]->fileid, DB_FILE_ID_LEN);
	value.size = DB_FILE_ID_LEN;
	if ((ret = __db_slice_metadata(
	    dbp, ip, txn, fileid_name, &value, insert)) != 0)
		__db_errx(dbp->env, DB_STR_A("0788",
		    "Sliced database %s has bad metadata for %s", "%s %s"),
		    dbp->fname, fileid_name);
	return (ret);
}

/*
 * __db_slice_metachk --
 *	Verify or insert the version and slice count metadata of a container db.
 *
 *	The container DB needs to have certain metadata.
 *   #records	key		value
 *	1	version		slice metadata version number as string
 *	1	count		slice count as a string
 *	#slices	fileid#%03d	the fileid of that slice's section
 *
 *	The version and count are checked here, if the file has been opened.
 *	The fileid metadata is checked later, after each slice is opened.
 *
 *	If a slice is missing or corrupt, return DB_SLICE_CORRUPT.
 *
 * PUBLIC: int __db_slice_metachk __P((DB *, DB_THREAD_INFO *, DB_TXN *));
 */
int
__db_slice_metachk(dbp, ip, txn)
	DB *dbp;
	DB_THREAD_INFO *ip;
	DB_TXN *txn;
{
	DBT value;
	char value_buf[DB_MAXPATHLEN];
	int ret;

	/*
	 * Skip the metadata checks for db_verify, etc., which don't really open
	 * the database. Db_verify does set DB_AM_OPEN_CALLED,
	 * so use fname == NULL.
	 */
	if (dbp->fname == NULL)
		return (0);

	DB_INIT_DBT_USERMEM(value, value_buf, sizeof(value_buf));

	/* Make sure that the version number is not too high, or low. */
	value.size = (u_int32_t)snprintf(value.data,
	    value.ulen, "%u", DB_SLICE_METADATA_VERSION);
	if ((ret = __db_slice_metadata(dbp, ip, txn, "version", &value, 0)) != 0)
		goto err;

	/* Make sure that the slice count matches the environment. */
	value.size = (u_int32_t)
	    snprintf(value.data, value.ulen, "%u", dbp->dbenv->slice_cnt);
	ret = __db_slice_metadata(dbp, ip, txn, "count", &value, 0);

err:
	return (ret);
}

/*
 * __db_slice_open --
 *	Finish opening up a sliced database by creating and opening its slices.
 *
 *	The container DB itself has already been opened.
 *
 *	Opens the relative filename in each of the slices' databases. Each
 *	takes places in its own environment and transaction.
 *	If a slice is missing or corrupt, return DB_SLICE_CORRUPT.
 *
 * PUBLIC: int __db_slice_open __P((DB *, DB_THREAD_INFO *,
 * PUBLIC:      DB_TXN *, const char *, DBTYPE, u_int32_t, int));
 */
int
__db_slice_open(dbp, ip, txn, fname, type, flags, mode)
	DB *dbp;
	DB_THREAD_INFO *ip;
	DB_TXN *txn;
	const char *fname;
	DBTYPE type;
	u_int32_t flags;
	int mode;
{
	DB_ENV *dbenv;
	DB_THREAD_INFO *slice_ip;
	DB_TXN *slice_txn;
	ENV *env;
	int ret;
	u_int32_t slice_flags;
	db_slice_t i;
	const char *mesg;

	env = dbp->env;
	dbenv = env->dbenv;
	mesg = NULL;
	slice_txn = NULL;

	if (!SLICES_ON(env))
		return (0);

	if (fname == NULL)
		mesg = "in-memory";
	else if (dbp->dname != NULL)
		mesg = "sub";
	else if (dbp->type != DB_BTREE && dbp->type != DB_HASH)
		mesg = __db_dbtype_to_string(dbp->type);
	if (mesg != NULL) {
		ret = USR_ERR(env, EINVAL);
		__db_err(env, ret, "%s databases cannot support slices", mesg);
		return (ret);
	}
	if (dbp->slice_callback == NULL)
		dbp->slice_callback = __db_slice_default_callback;

	/* Get the flags of the container. */
	if ((ret = __db_get_flags(dbp, &slice_flags)) != 0)
		return (ret);

	/* Allocate the db_slices array, create and 'clone' its db handles. */
	if ((ret = __db_slice_alloc(dbp, ip, txn)) != 0)
		return (ret);

	/*
	 * Now open each slice, without DB_SLICED so that their DML calls
	 * have non-sliced behavior.
	 */
	LF_CLR(DB_SLICED);
	for (i = 0; i != dbenv->slice_cnt; i++) {
		if ((ret = __db_set_flags(
		    dbp->db_slices[i], slice_flags)) != 0)
			goto err;
		if ((ret = __txn_slice_begin(txn, &slice_txn, i)) != 0) {
			__db_err(env, ret,
			    "txn->begin for db \"%s\" slice %d failed",
			    fname, i);
			goto err;
		}
		ENV_ENTER(dbp->db_slices[i]->env, slice_ip);
		dbp->db_slices[i]->open_flags = flags;
		if ((ret = __db_open(dbp->db_slices[i], slice_ip, slice_txn,
		    fname, NULL, type, flags, mode, PGNO_BASE_MD)) != 0) {
			__db_err(env, ret,
			    "open of database %s slice %d failed", fname, i);
			ENV_LEAVE(dbp->db_slices[i]->env, slice_ip);
			goto err;
		}
		ENV_LEAVE(dbp->db_slices[i]->env, slice_ip);
		ret = __db_slice_fileid_metachk(dbp, ip, txn, i, 0);
		if (ret != 0)
			goto err;
	}

	/* Replace functions which have special handling when db is sliced. */
	dbp->close = __db_slice_close_pp;
	dbp->del = __db_slice_del_pp;
	dbp->exists = __db_slice_exists_pp;
	dbp->get = __db_slice_get_pp;
	dbp->get_slices = __db_slice_get_slices;
	dbp->put = __db_slice_put_pp;
	dbp->pget = __db_slice_pget_pp;
	dbp->slice_lookup = __db_slice_lookup_pp;
	dbp->sync = __db_slice_sync_pp;
	/* Replace these with the generic "not supported" error function. */
	dbp->join =
	    (int (*) __P((DB *, DBC **, DBC **, u_int32_t)))__db_slice_notsup;
	dbp->key_range = (int (*) __P((DB *,
	    DB_TXN *, DBT *, DB_KEY_RANGE *, u_int32_t)))__db_slice_notsup;
	dbp->set_lk_exclusive = (int (*) __P((DB *, int)))__db_slice_notsup;
	dbp->set_partition = (int (*) __P ((DB *, u_int32_t, DBT *, u_int32_t (*)(DB *, DBT *key))))__db_slice_notsup;

	return (0);

err:
	(void)__db_slice_free(dbp, DB_NOSYNC);
	return (USR_ERR(env, DB_SLICE_CORRUPT));
}

/*
 * __db_slice_close_pp --
 *	DB->close pre/post processing for an actually sliced db.
 *
 * PUBLIC: int __db_slice_close_pp __P((DB *, u_int32_t));
 */
int
__db_slice_close_pp(dbp, flags)
	DB *dbp;
	u_int32_t flags;
{
	int ret, t_ret;

	ret = __db_slice_free(dbp, flags);
	if ((t_ret = __db_close_pp(dbp, flags)) != 0 && ret == 0)
		ret = t_ret;
	return (ret);
}

/*
 * __db_slice_iterate --
 *	Return each non-null slice of a sliced database.
 *
 *	The first call of 'foreach' loop starts with *pos == -1.
 *
 *	Returns:
 *		the next non-NULL slice, or NULL when all have been seen. Once
 *		it returns NULL it continues to do so on subsequent calls.
 *
 *	Side Effect:
 *		*pos is set to the position in the slice array of the
 *		returned environment.
 *
 * PUBLIC: DB *__db_slice_iterate __P((DB *, int *));
 */
DB *
__db_slice_iterate(dbp, pos)
	DB *dbp;
	int *pos;
{
	DB *sl_dbp;
	DB_ENV *dbenv;
	ENV *env;
	db_slice_t i;

	env = dbp->env;
	dbenv = env->dbenv;
	sl_dbp = NULL;
	if (!SLICES_ON(env))
		return (NULL);
	i = (db_slice_t)(1 + *pos);
	DB_ASSERT(env, i <= dbenv->slice_cnt);
	while (i < dbenv->slice_cnt && (sl_dbp = dbp->db_slices[i]) == NULL)
		i++;

	*pos = (int)i;
	/* This returns a good DB *, or the NULL if we've seen the last. */
	return (sl_dbp);
}

/*
 * __db_slice_sync_pp --
 *	DB->sync pre/post processing for an actually sliced db.
 *
 * PUBLIC: int __db_slice_sync_pp __P((DB *, u_int32_t));
 */
int
__db_slice_sync_pp(dbp, flags)
	DB *dbp;
	u_int32_t flags;
{
	db_slice_t i;
	int ret;

	ret = __db_sync_pp(dbp, flags);
	for (i = 0; ret == 0 && i != dbp->dbenv->slice_cnt; i++)
		ret = __db_sync_pp(dbp->db_slices[i], flags);
	return (ret);
}

/*
 * __db_slice_map --
 *	Given a container's DB * and a slice DBT, return the corresponding
 *	slice number.
 *
 * PUBLIC: int __db_slice_map __P((DB *, const DBT *, db_slice_t *));
 */
int
__db_slice_map(dbp, slice, slice_indexp)
	DB *dbp;
	const DBT *slice;
	db_slice_t *slice_indexp;
{
	ENV *env;
	db_slice_t hash;

	env = dbp->env;

	if (dbp->db_slices == NULL)
		return (__db_not_sliced(dbp));
	hash = (db_slice_t)__ham_func5(NULL, slice->data, slice->size);
	*slice_indexp = hash % env->dbenv->slice_cnt;
	return (0);
}

/*
 * __db_slice_lookup_pp --
 *	DB->slice_lookup API call
 *
 *	Map a key to its slice, return its DB *.
 *
 * PUBLIC: int __db_slice_lookup_pp __P((DB *, const DBT *, DB **, u_int32_t));
 */
int
__db_slice_lookup_pp(dbp, key, sl_dbpp, flags)
	DB *dbp;
	const DBT *key;
	DB **sl_dbpp;
	u_int32_t flags;
{
	DBT slice;
	int ret;
	db_slice_t id;

	if ((ret = __dbt_usercopy(dbp->env, (DBT *)key)) != 0 ||
	    (ret = __db_fchk(dbp->env, "DB->slice_lookup", flags, 0)) != 0)
		return (ret);

	if ((ret = __db_slice_build(dbp, key, &slice)) != 0 ||
	    (ret = __db_slice_map(dbp, &slice, &id)) != 0)
		*sl_dbpp = NULL;
	else
		*sl_dbpp = dbp->db_slices[id];

	FREE_IF_NEEDED(dbp->env, &slice);
	__dbt_userfree(dbp->env, (DBT *)key, NULL, NULL);
	return (ret);
}

/*
 * __db_slice_build --
 *	Invoke the major key callback function for the database.
 *
 * PUBLIC: int __db_slice_build __P((const DB *, const DBT *, DBT *));
 */
int
__db_slice_build(dbp, key, slice)
	const DB *dbp;
	const DBT *key;
	DBT *slice;
{
	int ret;

	memset(slice, 0, sizeof(DBT));
	if ((ret = dbp->slice_callback(dbp, key, slice)) != 0) {
		(void)USR_ERR(dbp->env, ret);
		__db_err(dbp->env, ret,
		    "Sliced database callback for %s failed", dbp->fname);
		return (ret);
	}
	return (0);
}

/*
 * __db_slice_activate --
 *	Prepare to access a slice of a container's sliced database, creating the
 *	required transaction as needed.
 *
 *	The DB and DB_TXN parameters belong to the containing environment.
 *	The returned DB and DB_TXN values belong to a slice's environment.
 *
 *	If it needs to begin a transaction, this enters both the container's
 *	environment (here) and the slice's environment (in __txn_slice_begin).
 *
 * PUBLIC: int __db_slice_activate
 * PUBLIC:     __P((DB *, DB_TXN *, const DBT *, DB **, DB_TXN **));
 */
int
__db_slice_activate(dbp, txn, sl_dbt, sl_dbpp, sl_txnp)
	DB *dbp;
	DB_TXN *txn;
	const DBT *sl_dbt;
	DB **sl_dbpp;
	DB_TXN **sl_txnp;
{
	DB *sl_dbp;
	DB_THREAD_INFO *ip;
	DB_TXN *sl_txn;
	ENV *sl_env;
	int ret;
	char *txnmsg;
	db_slice_t slice_index;

	*sl_dbpp = NULL;
	*sl_txnp = NULL;

	if ((ret = __db_slice_map(dbp, sl_dbt, &slice_index)) != 0)
		return (ret);

	sl_dbp = dbp->db_slices[slice_index];
	sl_env = sl_dbp->env;
	if (txn == NULL) {
		txnmsg = "implicit";
		sl_txn = NULL;
	} else if (txn->txn_slices == NULL) {
		txnmsg = "new";
		ENV_ENTER(dbp->env, ip);
		ret = __txn_slice_begin(txn, &sl_txn, slice_index);
		ENV_LEAVE(dbp->env, ip);
	} else if ((sl_txn = txn->txn_slices[slice_index]) == NULL) {
		/*
		 * If txn_slices has been allocated, then there already is a
		 * subordinate transaction for this container's txn. If it
		 *  *is not* for this slice, then it is for another one,
		 * which we don't support for DML.
		 */
		ret = __txn_multislice(txn);
		txnmsg = "denied second txn";
	}
	else
		txnmsg = "existing";

	if (FLD_ISSET(sl_env->dbenv->verbose, DB_VERB_SLICE)) {
		char hexbuf[DB_TOHEX_BUFSIZE(DB_VERB_SLICE_PRINTLEN)];
		u_int32_t printlen;

		if ((printlen = sl_dbt->size) > DB_VERB_SLICE_PRINTLEN)
			printlen = DB_VERB_SLICE_PRINTLEN;
		__db_msg(sl_env, "activate %s slice %d %s txns %08x:%08x",
		    __db_tohex(sl_dbt->data, printlen, hexbuf), slice_index,
		    txnmsg, txn == NULL ?  0 : txn->txnid,
		    sl_txn == NULL ? 0 : sl_txn->txnid);
	}

	*sl_dbpp = sl_dbp;
	*sl_txnp = sl_txn;

	return (ret);
}

/*
 * __db_slice_get_pp --
 *	DB->get pre/post processing when the major key builder is used.
 *
 *	Find which slice this fetch accesses, and direct the call to that
 *	db handle.
 *
 * PUBLIC: int __db_slice_get_pp __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
 */
int
__db_slice_get_pp(dbp, txn, key, data, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key, *data;
	u_int32_t flags;
{
	DB *sl_dbp;
	DBT slice;
	DB_TXN *sl_txn;
	int ret;

	if ((ret = __dbt_usercopy(dbp->env, key)) != 0)
		return (ret);

	if ((ret = __db_slice_build(dbp, key, &slice)) != 0)
		goto err;
	if ((ret = __db_slice_activate(dbp,
	    txn, &slice, &sl_dbp, &sl_txn)) != 0)
		goto err;
	__dbt_userfree(dbp->env, key, NULL, NULL);
	ret = __db_get_pp(sl_dbp, sl_txn, key, data, flags);

	if (0)
err:		__dbt_userfree(dbp->env, key, NULL, NULL);
	FREE_IF_NEEDED(dbp->env, &slice);
	return (ret);
}

/*
 * __db_slice_exists_pp --
 *	Sliced version of DB->exists.
 *
 *	Find which slice this fetch accesses; call exists() on that handle.
 *
 * PUBLIC: int __db_slice_exists_pp __P((DB *, DB_TXN *, DBT *, u_int32_t));
 */
int
__db_slice_exists_pp(dbp, txn, key, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key;
	u_int32_t flags;
{
	DB *sl_dbp;
	DBT slice;
	DB_TXN *sl_txn;
	int ret;

	if ((ret = __dbt_usercopy(dbp->env, key)) != 0)
		return (ret);
	if ((ret = __db_slice_build(dbp, key, &slice)) != 0)
		goto err;
	if ((ret = __db_slice_activate(dbp,
	    txn, &slice, &sl_dbp, &sl_txn)) != 0)
		goto err;

	__dbt_userfree(dbp->env, key, NULL, NULL);
	ret = __db_exists(sl_dbp, sl_txn, key, flags);

	if (0)
err:		__dbt_userfree(dbp->env, key, NULL, NULL);
	FREE_IF_NEEDED(dbp->env, &slice);
	return (ret);
}

/*
 * __db_slice_pget_pp --
 *	Sliced version DB->pget()
 *
 *	This needs to search all slices. Since there is no cross-slice
 *	transaction support, we ignore any txn passed in and use NULL local
 *	txns. It does not start at the same slice each time (e.g. slice 0), but
 *	starts at a random slice; this distributes the workload.
 *
 * PUBLIC: int __db_slice_pget_pp
 * PUBLIC:     __P((DB *, DB_TXN *, DBT *, DBT *, DBT *, u_int32_t));
 */
int
__db_slice_pget_pp(dbp, txn, skey, pkey, data, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *skey, *pkey, *data;
	u_int32_t flags;
{
	DB *sl_dbp;
	db_slice_t count, i, offset;
	int ret;

	ret = 0;
	if (dbp->db_slices == NULL)
		return (__db_not_sliced(dbp));
	/*
	 * Try to pget from each slice in succession.  If any pget() succeeds,
	 * or it returns an error besides DB_NOTFOUND, stop right away.
	 */
	count = dbp->dbenv->slice_cnt;
	offset = __os_random() % count;
	for (i = 0; i != count; i++) {
		sl_dbp = dbp->db_slices[(i + offset) % count];
		if ((ret = __db_pget_pp(sl_dbp,
		    NULL, skey, pkey, data, flags)) != DB_NOTFOUND)
			break;
	}
	COMPQUIET(txn, NULL);
	return (ret);
}

/*
 * __db_slice_put_pp --
 *	Sliced version of DB->put().
 *
 *	Find which slice this fetch accesses; call put() on that handle.
 *
 * PUBLIC: int __db_slice_put_pp __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
 */
int
__db_slice_put_pp(dbp, txn, key, data, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key, *data;
	u_int32_t flags;
{
	DB *sl_dbp;
	DBT slice;
	DB_TXN *sl_txn;
	int ret;

	if ((ret = __dbt_usercopy(dbp->env, key)) != 0)
		return (ret);

	if ((ret = __db_slice_build(dbp, key, &slice)) != 0)
		goto err;
	if ((ret = __db_slice_activate(dbp,
	    txn, &slice, &sl_dbp, &sl_txn)) != 0)
		goto err;
	__dbt_userfree(dbp->env, key, NULL, NULL);
	ret = __db_put_pp(sl_dbp, sl_txn, key, data, flags);

	if (0)
err:		__dbt_userfree(dbp->env, key, NULL, NULL);
	FREE_IF_NEEDED(dbp->env, &slice);
	return (ret);
}

/*
 * __db_slice_del_pp --
 *	Sliced version of DB->del().
 *
 *	Find which slice this fetch accesses; call del() on that handle.
 *
 * PUBLIC: int __db_slice_del_pp __P((DB *, DB_TXN *, DBT *, u_int32_t));
 */
int
__db_slice_del_pp(dbp, txn, key, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key;
	u_int32_t flags;
{
	DB *sl_dbp;
	DBT slice;
	DB_TXN *sl_txn;
	int ret;

	if ((ret = __dbt_usercopy(dbp->env, key)) != 0)
		return (ret);

	if ((ret = __db_slice_build(dbp, key, &slice)) != 0)
		goto err;
	if ((ret = __db_slice_activate(dbp,
	    txn, &slice, &sl_dbp, &sl_txn)) != 0)
		goto err;
	__dbt_userfree(dbp->env, key, NULL, NULL);
	ret = __db_del_pp(sl_dbp, sl_txn, key, flags);

	if (0)
err:		__dbt_userfree(dbp->env, key, NULL, NULL);
	FREE_IF_NEEDED(dbp->env, &slice);
	return (ret);
}

/*
 * __db_slice_secondary_get_pp --
 *	Sliced version __db_secondary_get(), i.e., DB->get() for a secondary DB.
 *
 *	This needs to search all slices. Since there is no cross-slice
 *	transaction support, we ignore any txn passed in and use NULL local
 *	txns. Like __db_slice_pget_pp(), it does not start at the same slice
 *	each time (e.g. slice 0), but starts at a random slice.
 *
 * PUBLIC: int __db_slice_secondary_get_pp
 * PUBLIC:     __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
 */
int
__db_slice_secondary_get_pp(sdbp, txn, skey, data, flags)
	DB *sdbp;
	DB_TXN *txn;
	DBT *skey, *data;
	u_int32_t flags;
{
	DB *sl_dbp;
	ENV *env;
	db_slice_t count, i, offset;
	int ret;

	ret = 0;
	env = sdbp->env;
	count = env->dbenv->slice_cnt;
	DB_ASSERT(env, count != 0);
	/*
	 * Try to get from each slice. If any get() succeeds, or one returns an
	 * error besides DB_NOTFOUND, stop right away. Start at a random slice.
	 */
	offset = __os_random() % count;
	for (i = 0; i != count; i++) {
		sl_dbp = sdbp->db_slices[(i + offset) % count];
		DB_ASSERT(env, F_ISSET(sl_dbp, DB_AM_SECONDARY));
		if ((ret = __db_secondary_get(sl_dbp,
		    NULL, skey, data, flags)) != DB_NOTFOUND)
			break;
	}
	COMPQUIET(txn, NULL);
	return (ret);
}


/*
 * __dbc_slice_init --
 *	Finish initializing a container's sliced cursor.
 *
 *	Change some of the API functions to the sliced cursor equivalents.
 *	The internal access method functions of a sliced cursor must not be
 *	used; their pointers are set to return an error if they are called.
 *
 * PUBLIC: int __dbc_slice_init __P((DBC *));
 */
int
__dbc_slice_init(dbc)
	DBC *dbc;
{
	DB_ASSERT(dbc->env, FLD_ISSET(dbc->dbp->open_flags, DB_SLICED));

	dbc->del = __dbc_slice_del_pp;
	dbc->get = __dbc_slice_get_pp;
	dbc->pget = __dbc_slice_pget_pp;
	dbc->put = __dbc_slice_put_pp;

	return (0);
}

/*
 * __dbc_slice_close --
 *	Close any open cursors on the slices before closing the
 *	top cursor.
 *
 * PUBLIC: int __dbc_slice_close __P((DBC *));
 */
int
__dbc_slice_close(dbc)
	DBC *dbc;
{
	int ret;

	ret = 0;
	if (dbc->dbc_slices[0] != NULL) {
		ret = __dbc_close_pp(dbc->dbc_slices[0]);
		dbc->dbc_slices[0] = NULL;
	}
	return (ret);
}

/*
 * __dbc_slice_activate --
 *	Prepare to access a slice of a sliced container's cursor, creating the
 *	required sub-environment's cursor as needed.
 *
 *	The DBC parameter belongs to the containing environment.
 *	The returned DBC values belong to a slice's environment.
 *
 *	This sometimes enters the slice's environment (when beginning a
 *	transaction there); it *does not* enter the container's environment.
 *
 *	If the cursor command in 'flags' is DB_FIRST or DB_LAST, then this
 *	changes	the container's DBC->get to iterate through all the slices.
 *	More details about that TBD.
 *
 *
 * PUBLIC: int __dbc_slice_activate
 * PUBLIC:     __P((DBC *, const DBT *, DBC **, u_int32_t));
 */
int
__dbc_slice_activate(dbc, key, sl_dbcp, flags)
	DBC *dbc;
	const DBT *key;
	DBC **sl_dbcp;
	u_int32_t flags;
{
	DB *dbp, *sl_dbp;
	DB_TXN *sl_txn;
	DBT slice;
	int ret;
	db_slice_t slice_index;

	*sl_dbcp = NULL;
	dbp = dbc->dbp;

	if ((ret = __db_slice_build(dbp, key, &slice)) != 0)
		return (ret);

	if (dbc->dbc_slices[0] == NULL) {
		if ((ret = __db_slice_activate(dbp,
		    dbc->txn, &slice, &sl_dbp, &sl_txn)) != 0)
			goto err;
		if ((ret = __db_cursor_pp(sl_dbp, sl_txn,
		    &dbc->dbc_slices[0], dbc->open_flags & ~DB_SLICED)) != 0)
			goto err;
	} else if ((ret = __db_slice_map(dbp, &slice, &slice_index)) != 0)
		goto err;
	else if (dbc->dbc_slices[0]->dbp->db_slice_index != slice_index) {
		ret = __txn_multislice(dbc->txn);
		goto err;
	}

	*sl_dbcp = dbc->dbc_slices[0];

err:	FREE_IF_NEEDED(dbp->env, &slice);
	COMPQUIET(flags, 0);
	return (ret);
}

/*
 * __dbc_slice_get_pp --
 *	DBC->get pre/post processing for sliced cursors.
 *
 * PUBLIC: int __dbc_slice_get_pp __P((DBC *, DBT *, DBT *, u_int32_t));
 */
int
__dbc_slice_get_pp(dbc, key, data, flags)
	DBC *dbc;
	DBT *key, *data;
	u_int32_t flags;
{
	DBC *sl_dbc;
	int init_get_all, op, ret;

	init_get_all = 0;
	op = flags & DB_OPFLAGS_MASK;
	switch  (op) {
	case DB_NEXT:
	case DB_NEXT_NODUP:
		if (dbc->dbc_slices[0] != NULL || key->size != 0)
			break;
		/* Fall through to initialize all-slice scan */
	case DB_FIRST:
		init_get_all = 1;
		break;
	case DB_PREV:
	case DB_PREV_NODUP:
		if (dbc->dbc_slices[0] != NULL || key->size != 0)
			break;
		/* Fall through to initialize all-slice backwards scan */
	case DB_LAST:
		init_get_all = 1;
		break;
	default:
		break;
	}

	if ((ret = __dbt_usercopy(dbc->env, key)) != 0)
		return (ret);

	ret = __dbc_slice_activate(dbc, key, &sl_dbc, flags);
	if (ret != 0) {
		__dbt_userfree(dbc->env, key, NULL, NULL);
		return (ret);
	}
	if (init_get_all) {
		if (key->size != 0) {
			ret = USR_ERR(dbc->env, EINVAL);
			__db_err(dbc->env, ret,
			    "sliced DB_FIRST/DB_LAST with key (size %u)",
			    key->size);
			return (ret);
		}
		/* Since key->size is 0, the first slice was activated above. */
		dbc->dbc_curslice = 0;
		dbc->get = __dbc_slice_get_all_pp;
		if (op == DB_FIRST)
			flags = (flags & ~DB_OPFLAGS_MASK) | DB_NEXT;
		else if (op == DB_LAST)
			flags = (flags & ~DB_OPFLAGS_MASK) | DB_PREV;

		/* Invoke the changed 'get' function that was set just above. */
		ret = __dbc_slice_get_all_pp(dbc, key, data, flags);
	} else
		ret = __dbc_get_pp(sl_dbc, key, data, flags);

	return (ret);
}

/*
 * __dbc_slice_fetch_all --
 *	Help DBC->get/pget to iterate over multiple slices, opening and closing
 *	cursors as needed.
 *
 *	The secondary key parameter specifies which function to call
 *		skey == NULL	DBC->get()
 *		skey != NULL	DBC->pget()
 *
 *	There is no guarantee of cross-slice consistency.
 *
 * PUBLIC: int __dbc_slice_fetch_all __P((DBC *,
 * PUBLIC:     DBT *, DBT *, DBT *, u_int32_t));
 */
int
__dbc_slice_fetch_all(dbc, skey, key, data, flags)
	DBC *dbc;
	DBT *skey, *key, *data;
	u_int32_t flags;
{
	DB *dbp;
	DBC *sl_dbc;
	DB_TXN *sl_txn;
	db_slice_t slice_cnt;
	int multi_slice_err, ret;

	dbp = dbc->dbp;
	slice_cnt = dbp->dbenv->slice_cnt;
	multi_slice_err = 0;
	/*
	 * If the current slice is too high, the caller has continued fetching
	 * after the previous call returned DB_NOTFOUND.
	 */
	if (dbc->dbc_curslice >= slice_cnt)
		return (DBC_ERR(dbc, DB_NOTFOUND));

	for (;;) {
		sl_dbc = dbc->dbc_slices[0];
		if (skey == NULL)
			ret = __dbc_get_pp(sl_dbc, key, data, flags);
		else
			ret = __dbc_pget_pp(sl_dbc, skey, key, data, flags);

		/* On success or a real error, we're done here. */
		if (ret != DB_NOTFOUND)
			break;

		/*
		 * If a transaction exists and it is not private, then
		 * the txn is accessing multiple slices, and should return
		 * an error.  However, wait to return the error until
		 * after closing the cursor.
		 */
		sl_txn = dbc->txn;
		if (sl_txn != NULL && !F_ISSET(sl_txn, TXN_PRIVATE))
			multi_slice_err = 1;

		if ((ret = __dbc_close_pp(sl_dbc)) != 0)
			break;
		dbc->dbc_slices[0] = NULL;

		if (multi_slice_err) {
			ret = __txn_multislice(sl_txn);
			break;
		}

		if (++dbc->dbc_curslice >= slice_cnt) {
			ret = DBC_ERR(dbc, DB_NOTFOUND);
			break;
		}

		if ((ret = __db_cursor_pp(dbp->db_slices[dbc->dbc_curslice],
		    NULL, &dbc->dbc_slices[0],
		    dbc->open_flags & ~DB_SLICED)) != 0)
			break;
	}

	return (ret);
}

/*
 * __dbc_slice_get_all_pp --
 *	DBC->get pre/post processing for a DB_FIRST or DB_LAST sliced cursor.
 *
 *	This goes from one slice to the next, when DB_NOTFOUND. Scan slice 0
 *	first, even when moving backwards through the slice (e.g., DB_PREV).
 *	There is no guarantee of cross-slice consistency.
 *
 * PUBLIC: int __dbc_slice_get_all_pp __P((DBC *, DBT *, DBT *, u_int32_t));
 */
int
__dbc_slice_get_all_pp(dbc, key, data, flags)
	DBC *dbc;
	DBT *key, *data;
	u_int32_t flags;
{
	int ret;

	switch (flags & DB_OPFLAGS_MASK) {
	case DB_NEXT:
	case DB_PREV:
		ret = __dbc_slice_fetch_all(dbc, NULL, key, data, flags);
		break;
	default:
		dbc->get = __dbc_slice_get_pp;
		ret = __dbc_slice_get_pp(dbc, key, data, flags);
	}

	return (ret);
}

/*
 * __dbc_slice_pget_pp --
 *	DBC->pget processing for sliced cursors.
 *
 *	This has to open and close cursors in each slice, until it find one or
 *	it sees a real error -- DB_NOTFOUND is not an error here.
 *
 * PUBLIC: int __dbc_slice_pget_pp __P((DBC *, DBT *, DBT *, DBT *, u_int32_t));
 */
int
__dbc_slice_pget_pp(dbc, skey, pkey, data, flags)
	DBC *dbc;
	DBT *skey, *pkey, *data;
	u_int32_t flags;
{
	int ret;

	ret = __dbc_slice_fetch_all(dbc, skey, pkey, data, flags);
	return (ret);
}

/*
 * __dbc_slice_put_pp --
 *	DBC->put pre/post processing for sliced cursors.
 *
 * PUBLIC: int __dbc_slice_put_pp __P((DBC *, DBT *, DBT *, u_int32_t));
 */
int
__dbc_slice_put_pp(dbc, key, data, flags)
	DBC *dbc;
	DBT *key, *data;
	u_int32_t flags;
{
	DBC *sl_dbc;
	int ret;

	if ((ret = __dbt_usercopy(dbc->env, key)) != 0)
		return (ret);

	ret = __dbc_slice_activate(dbc, key, &sl_dbc, flags);
	__dbt_userfree(dbc->env, key, NULL, NULL);
	if (ret == 0)
		ret = __dbc_put_pp(sl_dbc, key, data, flags);

	return (ret);
}

/*
 * __dbc_slice_del_pp --
 *	DBC->del pre/post processing for sliced cursors.
 *
 *	This just forwards the cursor delete to the current cursor.
 *
 * PUBLIC: int __dbc_slice_del_pp __P((DBC *, u_int32_t));
 */
int
__dbc_slice_del_pp(dbc, flags)
	DBC *dbc;
	u_int32_t flags;
{
	DBC *sl_dbc;

	/* It is an error to do a cursor delete before the first get. */
	if ((sl_dbc = dbc->dbc_slices[0]) == NULL)
		return (DBC_ERR(dbc, EINVAL));

	return (__dbc_del_pp(sl_dbc, flags));
}

/*
 * __db_slice_remove --
 *	Extra __env_dbremove() steps for a sliced database that are done before
 *	removing the container's database.
 *
 *	Returns:
 *		DB_SLICE_CORRUPT if a slice's remove fails.
 *
 * PUBLIC: int __db_slice_remove
 * PUBLIC:     __P((DB_ENV *, DB_TXN *, const char *, const char *, u_int32_t));
 */
int
__db_slice_remove(dbenv, txn, name, subdb, flags)
	DB_ENV *dbenv;
	DB_TXN *txn;
	const char *name;
	const char *subdb;
	u_int32_t flags;
{
	DB_ENV *slice;
	DB_TXN *sl_txn;
	ENV *env;
	int i, ret, t_ret;
	u_int32_t metaflags;

	/* Slices do not handle sub-databases. */
	if (subdb != NULL)
		return (0);

	env = dbenv->env;
	/* This function is a nop if the db is not sliced. */
	if ((ret = __db_get_metaflags(env, name, &metaflags)) != 0)
		return (ret);
	if (!FLD_ISSET(metaflags, DBMETA_SLICED))
		return (0);
	/* Return an error if removing a sliced db from a non-sliced env. */
	if (!SLICES_ON(env))
		return (__env_not_sliced(env));

	for (i = -1; (slice = __slice_iterate(dbenv, &i)) != NULL; ) {
		if ((t_ret =
		    __txn_slice_begin(txn, &sl_txn, (db_slice_t)i)) != 0 ||
		    (t_ret = __env_dbremove_pp(slice,
		    sl_txn, name, subdb, flags)) != 0) {
			/*
			 * Until cross slice DDL operations are atomic, any
			 * missing files do not return an error code.
			 */
			if (t_ret == ENOENT)
				continue;
			__db_err(env, t_ret, "dbremove #%d %s", i, name);
			/*
			 * Suppress missing files in slice directories;
			 * cross-slice DDL isn't atomic.
			 */
			if (ret == 0)
				ret = USR_ERR(env, DB_SLICE_CORRUPT);
		}
	}
	return (ret);
}

/*
 * __db_slice_associate --
 *	Extra associate steps for a sliced database, after doing the container.
 *
 *	This requires a cross-slice txn; filling the secondaries (if DB_CREATE)
 *	is not atomic.
 *
 *	Returns:
 *		DB_SLICE_CORRUPT if a slice's associate fails.
 *
 * PUBLIC: int __db_slice_associate __P((DB *, DB_TXN *, DB *,
 * PUBLIC:     int (*)(DB *, const DBT *, const DBT *, DBT *), u_int32_t));
 */
int
__db_slice_associate(dbp, txn, sdbp, callback, flags)
	DB *dbp;
	DB_TXN *txn;
	DB *sdbp;
	int (*callback) __P((DB *, const DBT *, const DBT *, DBT *));
	u_int32_t flags;
{
	DB *sl_dbp;
	DB_TXN *sl_txn;
	int i, ret, t_ret;

	ret = 0;
	for (i = -1; (sl_dbp = __db_slice_iterate(dbp, &i)) != NULL; ) {
		if ((t_ret =
		    __txn_slice_begin(txn, &sl_txn, (db_slice_t)i)) != 0 ||
		    (t_ret = __db_associate_pp(sl_dbp,
			sl_txn, sdbp->db_slices[i], callback, flags)) != 0) {
			__db_err(dbp->env,
			    t_ret, "db_associate #%d %s", i, dbp->fname);
			if (ret == 0)
				ret = t_ret;
		}
	}
	sdbp->get = __db_slice_secondary_get_pp;
	return (ret);
}


/*
 * __db_slice_compact --
 *	Extra compact steps for a sliced database, after doing the container.
 *
 *	Returns:
 *		DB_SLICE_CORRUPT if a slice's compact fails.
 *
 * PUBLIC: int __db_slice_compact __P((DB *,
 * PUBLIC:     DB_TXN *, DBT *, DBT *, DB_COMPACT *, u_int32_t, DBT *));
 */
int
__db_slice_compact(dbp, txn, start, stop, c_data, flags, end)
	DB *dbp;
	DB_TXN *txn;
	DBT *start, *stop;
	DB_COMPACT *c_data;
	u_int32_t flags;
	DBT *end;
{
	DB *sl_dbp;
	DB_TXN *sl_txn;
	int i, ret, t_ret;

	ret = 0;
	/* There is nothing extra to do if the database is not sliced.  */
	if (!FLD_ISSET(dbp->open_flags, DB_SLICED))
		return (0);

	for (i = -1; (sl_dbp = __db_slice_iterate(dbp, &i)) != NULL; ) {
		if ((t_ret =
		    __txn_slice_begin(txn, &sl_txn, (db_slice_t)i)) != 0 ||
		    (t_ret = __db_compact_pp(sl_dbp,
			sl_txn, start, stop, c_data, flags, end)) != 0) {
			__db_err(dbp->env,
			    t_ret, "db_compact #%d %s", i, dbp->fname);
			if (ret == 0)
				ret = t_ret;
		}
	}
	return (ret);
}

/*
 * __db_slice_rename --
 *	Extra __env_dbrename steps for a sliced database that are done before
 *	renaming the container.
 *
 *	Returns:
 *		DB_SLICE_CORRUPT if a slice's rename fails.
 *
 * PUBLIC: int __db_slice_rename __P((DB *,
 * PUBLIC:     DB_TXN *, const char *, const char *, const char *, u_int32_t));
 */
int
__db_slice_rename(dbp, txn, name, subdb, newname, flags)
	DB *dbp;
	DB_TXN *txn;
	const char *name;
	const char *subdb;
	const char *newname;
	u_int32_t flags;
{
	DB_ENV *dbenv, *slice;
	DB_TXN *sl_txn;
	ENV *env;
	int i, ret, t_ret;
	u_int32_t metaflags;

	/* Slices do not handle sub-databases. */
	if (subdb != NULL)
		return (0);

	env = dbp->env;
	dbenv = dbp->dbenv;
	if ((ret = __db_get_metaflags(env, name, &metaflags)) != 0 &&
	    ret != ENOENT)
		return (ret);
	if (!FLD_ISSET(metaflags, DBMETA_SLICED))
		return (0);
	/* Return an error if renaming a sliced db from a non-sliced env. */
	if (!SLICES_ON(env))
		return (__env_not_sliced(env));

	for (i = -1; (slice = __slice_iterate(dbenv, &i)) != NULL; )
	{
		if ((t_ret =
		    __txn_slice_begin(txn, &sl_txn, (db_slice_t)i)) != 0 ||
		    (t_ret = __env_dbrename_pp(slice,
		    sl_txn, name, subdb, newname, flags)) != 0) {
			/*
			 * Until cross slice DDL operations are atomic, any
			 * missing files do not return an error code.
			 */
			if (t_ret == ENOENT)
				continue;
			__db_err(env, t_ret, DB_STR_A("0784",
			    "dbrename #%d %s->%s", "%d %s %s"),
			    i, name, newname);
			if (ret == 0)
				ret = USR_ERR(env, DB_SLICE_CORRUPT);
		}
	}
	return (ret);
}

/*
 * __db_slice_truncate --
 *	Extra truncate steps for a sliced database, after doing the container.
 *
 *	Returns:
 *		DB_SLICE_CORRUPT if a slice's truncate fails.
 *
 * PUBLIC: int __db_slice_truncate __P((DB *,
 * PUBLIC:     DB_TXN *, u_int32_t *, u_int32_t));
 */
int
__db_slice_truncate(dbp, txn, countp, flags)
	DB *dbp;
	DB_TXN *txn;
	u_int32_t *countp;
	u_int32_t flags;
{
	ENV *env;
	DB_TXN *sl_txn;
	db_slice_t i;
	int ret, t_ret;
	u_int32_t slice_records;

	env = dbp->env;
	ret = 0;
	if (countp != NULL)
		*countp = 0;
	/* There is nothing extra to do if the database is not sliced.  */
	if (!FLD_ISSET(dbp->open_flags, DB_SLICED))
		return (0);

	for (i = 0; i != env->dbenv->slice_cnt; i++) {
		slice_records = 0;
		if ((t_ret = __txn_slice_begin(txn, &sl_txn, i)) != 0 ||
		    (t_ret = __db_truncate_pp(dbp->db_slices[i],
		    sl_txn, &slice_records, flags)) != 0) {
			if (FLD_ISSET(env->dbenv->verbose, DB_VERB_SLICE))
				__db_err(env,  t_ret,
				    "db_slice_truncate #%d %s", i, dbp->fname);
			if (ret == 0)
				ret = t_ret;
		}
		if (countp != NULL)
			*countp += slice_records;
	}
	return (ret);
}

/*
 * __db_slice_process --
 *	Extra DB->upgrade/convert processing for a possibly sliced database.
 *
 *	The database has not been opened, so we need to create the slices'
 *	handles, and free them when we're done.
 *
 *	Returns:
 *		DB_SLICE_CORRUPT if a slice cannot be found.
 *
 * PUBLIC: int __db_slice_process __P((DB *, const char *, u_int32_t,
 * PUBLIC:     int (*)(DB *, const char *, u_int32_t), const char *));
 */
int
__db_slice_process(dbp, fname, flags, pfunc, msgpfx)
	DB *dbp;
	const char *fname;
	u_int32_t flags;
	int (*pfunc)(DB *, const char *, u_int32_t);
	const char *msgpfx;
{
	ENV *env;
	db_slice_t i;
	int ret, t_ret;
	u_int32_t metaflags;

	env = dbp->env;
	/*
	 * Common DDL checks for sliced databases:
	 * Nothing to do if not sliced, it is an error to attempt sliced
	 */
	if ((ret = __db_get_metaflags(env, fname, &metaflags)) != 0 &&
	    ret != ENOENT)
		return (ret);
	if (!FLD_ISSET(metaflags, DBMETA_SLICED))
		return (0);
	if (!SLICES_ON(env))
		return (__env_not_sliced(env));

	/*
	 * Upgrading a non-sliced db does not require opening the database, but
	 * the sliced version does,
	 */
	if (!F_ISSET(dbp, DB_AM_OPEN_CALLED) &&
	    (ret = __db_slice_alloc(dbp, NULL, NULL)) != 0)
		return (ret);

	for (i = 0; i != env->dbenv->slice_cnt; i++) {
		if ((t_ret = pfunc(dbp->db_slices[i], fname, flags)) != 0) {
			__db_err(env, t_ret, DB_STR_A("0785",
			    "%s failed for slice #%u: '%s'", "%s %u %s"),
			    msgpfx, i, fname);
			if (ret == 0)
				ret = USR_ERR(env, DB_SLICE_CORRUPT);
		}
	}

	/* No flush needed: each upgrade has already __os_fsync()'d the file. */
	if ((t_ret = __db_slice_free(dbp, DB_NOSYNC)) != 0 && ret == 0)
		ret = t_ret;

	return (ret);
}

/*
 * __dbc_slice_dump_get --
 *	Help __db_dump() to retrieve every key/value pair of all the slices.
 *
 *	There is no attempt to provide cross-slice consistency. It is similar
 *	to __dbc_slice_fetch_all without secondary index support.
 *
 * PUBLIC: int __dbc_slice_dump_get __P((DBC *, DBT *, DBT *, u_int32_t));
 */
int
__dbc_slice_dump_get(dbc, key, data, flags)
	DBC *dbc;
	DBT *key, *data;
	u_int32_t flags;
{
	DB *dbp;
	DBC *sl_dbc;
	int ret;

	dbp = dbc->dbp;
	
	/*
	 * If the current slice is too high, the caller has continued fetching
	 * after the previous call returned DB_NOTFOUND.
	 */
	if (dbc->dbc_curslice >= dbp->dbenv->slice_cnt)
		return (DBC_ERR(dbc, DB_NOTFOUND));

	for (;;) {
		if (dbc->dbc_slices[0] == NULL && (ret =
		    __db_cursor_pp(dbp->db_slices[dbc->dbc_curslice], NULL,
		    &dbc->dbc_slices[0], dbc->open_flags & ~DB_SLICED)) != 0)
			break;
		sl_dbc = dbc->dbc_slices[0];

		ret = __dbc_get_pp(sl_dbc, key, data, flags);

		/* On success or a real error, we're done here. */
		if (ret != DB_NOTFOUND)
			break;

		if ((ret = __dbc_close_pp(sl_dbc)) != 0)
			break;
		dbc->dbc_slices[0] = NULL;

		if (++dbc->dbc_curslice >= dbp->dbenv->slice_cnt) {
			ret = DBC_ERR(dbc, DB_NOTFOUND);
			break;
		}
	}

	return (ret);
}

/*
 * __db_slice_verify --
 *	Extra DB->verify processing for a possibly sliced database.
 *
 *	The database has not been opened, so we need to create the slices'
 *	handles, and free them when we're done, like __db_slice_verify.
 *
 *	Returns:
 *		DB_SLICE_CORRUPT if a slice cannot be found.
 *
 * PUBLIC: int __db_slice_verify __P((DB *, const char *,
 * PUBLIC:     const char *, void *, int (*)(void *, const void *), u_int32_t));
 */
int
__db_slice_verify(dbp, fname, dname, handle, callback, flags)
	DB *dbp;
	const char *fname;
	const char *dname;
	void *handle;
	int (*callback) __P((void *, const void *));
	u_int32_t flags;
{
	ENV *env;
	db_slice_t i;
	int ret, t_ret;
	u_int32_t metaflags;

	/* Slices do not handle sub-databases. */
	if (dname != NULL)
		return (0);

	env = dbp->env;
	/*
	 * Common DDL checks for sliced databases:
	 * Nothing to do if not sliced, it is an error to attempt sliced
	 */
	if ((ret = __db_get_metaflags(env, fname, &metaflags)) != 0 &&
	    ret != ENOENT)
		return (ret);
	if (!FLD_ISSET(metaflags, DBMETA_SLICED))
		return (0);
	if (!SLICES_ON(env))
		return (__env_not_sliced(env));

	if ((ret = __db_slice_alloc(dbp, NULL, NULL)) != 0)
		goto err;
	for (i = 0; i != env->dbenv->slice_cnt; i++) {
		if ((t_ret = __db_verify_internal(dbp->db_slices[i],
		    fname, dname, handle, callback, flags)) != 0) {
			__db_err(env, t_ret, DB_STR_A("0786",
			    "db_verify #%u %s", "%d %s"), i, fname);
			if (ret == 0)
				ret = USR_ERR(env, DB_SLICE_CORRUPT);
		}
	}

	/* Verify closed the dbs but doesn't free the db_slices array. */
	if ((t_ret = __db_slice_free(dbp, DB_NOSYNC)) != 0 && ret == 0)
		ret = t_ret;
err:
	return (ret);
}

#endif
