/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"
#ifdef HAVE_SLICES

#include "dbinc/crypto.h"
#include "dbinc/db_page.h"
#include "dbinc/db_am.h"
#include "dbinc/fop.h"
#include "dbinc/lock.h"
#include "dbinc/slice.h"
#include "dbinc/txn.h"

static int __env_get_slice_fileids __P((DB_ENV *, DB *, const char *));

/*
 * __env_slice_config --
 *	 Finish setting the names of slices' db_home.
 *
 * PUBLIC: int __env_slice_db_home __P((DB_ENV *, const char *));
 */
int
__env_slice_db_home(dbenv, db_home)
	DB_ENV *dbenv;
	const char *db_home;
{
	DB_ENV *slice;
	size_t len;
	int i, ret;
	char *home, home_buf[DB_MAXPATHLEN];

	/*
	 * A slice can have an explicit absolute or relative path in the
	 * container's DB_CONFIG, or it defaults to __db.sliceNNN in the
	 * container's db_home. This loop sets the slices' db_home in
	 * the cases in which they are relative.
	 */
	SLICE_FOREACH(dbenv, slice, i) {
		if ((home = slice->env->db_home) == NULL) {
			/*
			 * DB_SLICE_SUBDIR is the printf format string
			 * which generates the default home.
			 */
			home = home_buf;
			snprintf(home, sizeof(home_buf), DB_SLICE_SUBDIR, i);
		}
		else if (__os_abspath(home))
			continue;

		/* Prefix this relative path with the container's db_home. */
		len = strlen(db_home) +
		    strlen(PATH_SEPARATOR) + strlen(home) + 1;
		if ((ret =
		    __os_malloc(slice->env, len, &slice->env->db_home)) != 0)
			return (ret);
		snprintf(slice->env->db_home, len,
		    "%s%c%s", db_home, PATH_SEPARATOR[0], home);
	}
	return (0);
}

/*
 * __env_slice_open --
 *	 Help a containing environment's __env_open() open all of its slices.
 * __env_config has already obtained the environments' settings.
 *
 *	The container is set to always autocommit, in hopes of limiting the
 *	slice-awareness of various DS-style operations (e.g., db_dump) which
 *	do not start any transactions.
 *
 * PUBLIC: #ifdef HAVE_SLICES
 * PUBLIC: int __env_slice_open __P((DB_ENV *, const char *, u_int32_t, int));
 * PUBLIC: #endif
 */
int
__env_slice_open(dbenv, db_home, flags, mode)
	DB_ENV *dbenv;
	const char *db_home;
	u_int32_t flags;
	int mode;
{
	DB_ENV *slice;
	ENV *env;
	int i, ret;
	db_slice_t id;
	char *sl_home;

	env = dbenv->env;

	for (id = 0; id < dbenv->slice_cnt; id++) {
		slice = env->slice_envs[id];
		sl_home = slice->env->db_home;
		/*
		 * This always adds the permission bits so that the owner can
		 * read, write, and execute slices' home directories.
		 */
		if (access(sl_home, R_OK | W_OK | X_OK) < 0 &&
		    mkdir(sl_home, mode | S_IRUSR | S_IWUSR | S_IXUSR) < 0) {
			ret = USR_ERR(env, __os_get_errno());
			__db_err(env, ret, "Cannot create %s", sl_home);
			goto err;
		}
		/*
		 * Set encryption on the slices.  This has to be done here
		 * since encryption has to be set before opening the
		 * environment, and the container environment is not known
		 * to be sliced until its open function is called.  The 
		 * container has not been open yet, so the passwd field has
		 * not been deleted yet.
		 */
		if (dbenv->passwd != NULL) {
			if ((ret = __env_set_encrypt(
			    slice, dbenv->passwd, dbenv->encrypt_flags)) != 0)
				goto err;
		}

		if ((ret = __env_open(slice, sl_home, flags, mode)) != 0)
			goto err;
	}

	/*
	 * Change any container-specific DB_ENV member functions here.
	 */
	dbenv->close = __env_slice_close_pp;
	dbenv->txn_checkpoint = __env_slice_txn_checkpoint_pp;
	dbenv->lsn_reset = __env_slice_lsn_reset_pp;
	dbenv->fileid_reset = __env_slice_fileid_reset_pp;
	F_SET(dbenv, DB_ENV_AUTO_COMMIT);

	return (0);

err:
	if (SLICES_ON(env)) {
		SLICE_FOREACH(dbenv, slice, i)
			(void)__env_close_pp(slice, 0);
		__os_free(env, env->slice_envs);
		env->slice_envs = NULL;
	}

	/*
	 * The sliced version of DB_ENV->open() gets db_home from the container
	 * and DB_CONFIG, not this parameter.
	 */
	COMPQUIET(db_home, NULL);
	return (ret);
}

/*
 * __env_slice_close_pp --
 *	DB_ENV->close pre/post processor for sliced environments.
 *
 * PUBLIC: int __env_slice_close_pp __P((DB_ENV *, u_int32_t));
 */
int
__env_slice_close_pp(dbenv, flags)
	DB_ENV *dbenv;
	u_int32_t flags;
{
	DB_ENV *slice;
	int i, ret, t_ret;

	ret = 0;
	DB_ASSERT(dbenv->env, dbenv->slice_cnt != 0);
	SLICE_FOREACH(dbenv, slice, i)
		if ((t_ret = __env_close_pp(slice, flags)) != 0 && ret == 0)
			ret = t_ret;
	if ((t_ret = __env_close_pp(dbenv, flags)) != 0 && ret == 0)
		ret = t_ret;
	return (ret);
}

/*
 * __env_slice_txn_checkpoint_pp --
 *	DB_ENV->txn_checkpoint pre/post processor for sliced environments.
 *
 * PUBLIC: int __env_slice_txn_checkpoint_pp
 * PUBLIC:     __P((DB_ENV *, u_int32_t, u_int32_t, u_int32_t));
 */
int
__env_slice_txn_checkpoint_pp(dbenv, kbyte, min, flags)
	DB_ENV *dbenv;
	u_int32_t kbyte;
	u_int32_t min;
	u_int32_t flags;
{
	DB_ENV *slice;
	int i, ret, t_ret;

	ret = 0;
	DB_ASSERT(dbenv->env, dbenv->slice_cnt != 0);
	SLICE_FOREACH(dbenv, slice, i) {
		if ((t_ret = __txn_checkpoint_pp(slice,
		    kbyte, min, flags)) != 0 && ret == 0)
			ret = t_ret;
	}
	if ((t_ret = __txn_checkpoint_pp(dbenv, kbyte, min, flags)) != 0 &&
	    ret == 0)
		ret = t_ret;
	return (ret);
}

/*
 * __env_slice_lsn_reset_pp --
 *	ENV->lsn_reset pre/post processing for a sliced environment.
 *
 * PUBLIC: int __env_slice_lsn_reset_pp
 * PUBLIC:     __P((DB_ENV *, const char *, u_int32_t));
 */
int
__env_slice_lsn_reset_pp(dbenv, name, flags)
	DB_ENV *dbenv;
	const char *name;
	u_int32_t flags;
{
	DB_ENV *slice;
	int i, ret;
	u_int32_t metaflags;

	if ((ret = __env_lsn_reset_pp(dbenv, name, flags)) != 0)
		goto err;
	if ((ret = __db_get_metaflags(dbenv->env, name, &metaflags)) != 0)
		goto err;
	if (FLD_ISSET(metaflags, DBMETA_SLICED))
		SLICE_FOREACH(dbenv, slice, i)
			if ((ret = __env_lsn_reset_pp(slice, name, flags)) != 0)
				break;
err:
	return (ret);
}

/*
 * __env_slice_fileid_reset_pp --
 *	ENV->fileid_reset pre/post processing for a sliced environment.
 *
 * PUBLIC: int __env_slice_fileid_reset_pp
 * PUBLIC:     __P((DB_ENV *, const char *, u_int32_t));
 */
int
__env_slice_fileid_reset_pp(dbenv, name, flags)
	DB_ENV *dbenv;
	const char *name;
	u_int32_t flags;
{
	ENV *env;
	DB *dbp;
	DB_TXN *txn;
	DB_THREAD_INFO *ip;
	db_slice_t i;
	u_int32_t metaflags;
	int ret;
	char *real_name;

	env = dbenv->env;
	dbp = NULL;
	real_name = NULL;
	txn = NULL;

	if ((ret = __db_get_metaflags(dbenv->env, name, &metaflags)) != 0)
		goto err;
	if ((ret = __env_fileid_reset_pp(dbenv, name, flags)) != 0)
		goto err;
	if (FLD_ISSET(metaflags, DBMETA_SLICED)) {
		for (i = 0; i < dbenv->slice_cnt; i++)
			if ((ret = __env_fileid_reset_pp(env->slice_envs[i],
			    name, flags)) != 0)
				goto err;
		/*
		 * Update the fileids of the slices in the container.
		 * These operations should not be logged or replicated,
		 * so open the database with DB_TXN_NOT_DURABLE.  
		 */
		ENV_GET_THREAD_INFO(env, ip);
		if ((ret = __db_create_internal(&dbp, env, 0)) != 0)
			goto err;
		
		if ((ret = __db_set_flags(
		    dbp, flags | DB_TXN_NOT_DURABLE)) != 0)
			goto err;

		if ((ret = __txn_begin(
		    env, ip, NULL, &txn, DB_IGNORE_LEASE)) != 0)
			goto err;

		if ((ret = __db_open(dbp, ip, txn,
		    name, NULL, DB_UNKNOWN, flags, 0, PGNO_BASE_MD)) != 0)
			goto err;

		if ((ret = __txn_commit(txn, 0)) != 0)
			goto err;
		txn = NULL;

		if ((ret = __os_calloc(env,
		    dbenv->slice_cnt, sizeof(DB *), &dbp->db_slices)) != 0)
			goto err;
		for (i = 0; i < dbenv->slice_cnt; i++) {
			if ((ret = db_create(
			    &dbp->db_slices[i], env->slice_envs[i], 0)) != 0) {
				__db_err(env, ret,
				    "create of database %s slice %d",
				    dbp->fname, i);
				goto err;
			}
		}
		if ((ret = __env_get_slice_fileids(dbenv, dbp, name)) != 0)
		  	goto err;

		for (i = 0; i < dbenv->slice_cnt; i++) {
		  	if ((ret = __db_slice_fileid_metachk(
		      	    dbp, NULL, NULL, i, 1)) != 0)
		  	  	goto err;
		}
	}
err:
	if (txn != NULL)
		(void)__txn_abort(txn);
	if (dbp != NULL) {
		if (dbp->db_slices != NULL) {
			for (i = 0; i < dbenv->slice_cnt; i++) {
				if (dbp->db_slices[i] != NULL)
					(void)__db_close(
					    dbp->db_slices[i], NULL, 0);
			}
			__os_free(env, dbp->db_slices);
		}
		(void)__db_close(dbp, NULL, 0);
	}
	if (real_name != NULL)
		__os_free(env, real_name);
	return (ret);
}

/*
 * __env_get_slice_fileids
 *
 * Read the fileids of the sliced databases.  Assumes the slices are not
 * opened, and no other threads are accessing them.
 *
 * Also assumes there are no subdatabases.
 *
 */	  
static int
__env_get_slice_fileids(dbenv, dbp, name)
	DB_ENV *dbenv;
	DB *dbp;
	const char *name;
{
	DB_ENV *slice;
	DB_FH *fhp;
	DBMETA *meta;
	int i, ret;
	char *real_name;
	u_int8_t mbuf[DBMETASIZE];

	ret = 0;
	meta = (DBMETA *)mbuf;
	fhp = NULL;
	real_name = NULL;
	for (i = -1; (slice = __slice_iterate(dbenv, &i)) != NULL; ) {
		if ((ret = __db_appname(slice->env,
		    DB_APP_DATA, name, NULL, &real_name)) != 0)
			goto err;
		if ((ret = __os_open(
		    slice->env, real_name, 0, 0, 0, &fhp)) != 0)
			goto err;
		if ((ret = __fop_read_meta(slice->env,
		    real_name, mbuf, DBMETASIZE, fhp, 1, NULL)) != 0)
			goto err;
		if ((ret = __db_chk_meta(
		    slice->env, NULL, meta, DB_CHK_META)) == 0) {
			memcpy(dbp->db_slices[i]->fileid,
			    meta->uid, DB_FILE_ID_LEN);
		} else
			goto err;
		(void)__os_closehandle(slice->env, fhp);
		fhp = NULL;
		__os_free(slice->env, real_name);
		real_name = NULL;
	}
err:
	if (fhp != NULL)
		(void)__os_closehandle(slice->env, fhp);
	if (real_name != NULL)
		__os_free(slice->env, real_name);
	return (ret);
}

/*
 * __env_get_slice_count --
 *	DB_ENV->get_slice_count. This is okay to call on non-sliced
 *	environments: it returns zero for them, rather than raising an error.
 *
 * PUBLIC: int  __env_get_slice_count __P((DB_ENV *, u_int32_t *));
 */
int
__env_get_slice_count(dbenv, countp)
	DB_ENV *dbenv;
	u_int32_t *countp;
{
	ENV_ILLEGAL_BEFORE_OPEN(dbenv->env, "DB_ENV->get_slice_count");
	*countp = dbenv->slice_cnt;
	return (0);
}

/*
 * __env_get_slices --
 *	DB_ENV->get_slices
 *
 * PUBLIC: int  __env_get_slices __P((DB_ENV *, DB_ENV ***));
 */
int
__env_get_slices(dbenv, retp)
	DB_ENV *dbenv;
	DB_ENV ***retp;
{
	int ret;

	ret = 0;
	if (dbenv->slice_cnt == 0)
		ret = __env_not_sliced(dbenv->env);
	else
		*retp = dbenv->env->slice_envs;
	return (ret);
}

/*
 * __env_slice_configure --
 *	Share the setting of a container dbenv with one of its slices.
 *
 * PUBLIC: int __env_slice_configure __P((const DB_ENV *, DB_ENV *));
 */
int
__env_slice_configure(container, slice)
	const DB_ENV *container;
	DB_ENV *slice;
{
	int ret;

	ret = 0;
	DB_ASSERT(container->env, container->slice_cnt != 0);

	slice->env->slice_container = container->env;

	/*
	 * Copy the values inherited from the container. These are sorted not by
	 * the field names, but by the name of the DB_ENV->set_xxx() function.
	 */
	slice->db_malloc = container->db_malloc;
	slice->db_realloc = container->db_realloc;
	slice->db_free = container->db_free;
	slice->app_dispatch = container->app_dispatch;
	if ((ret = __env_backup_copy(slice, container)) != 0)
		return (ret);
	slice->blob_threshold = container->blob_threshold;
	slice->mp_gbytes = container->mp_gbytes;
	slice->mp_bytes = container->mp_bytes;
	slice->mp_ncache = container->mp_ncache;
	slice->mp_max_gbytes = container->mp_max_gbytes;
	slice->mp_max_bytes = container->mp_max_bytes;
	slice->env->data_len = container->env->data_len;
	__env_set_errcall(slice, container->db_errcall);
	__env_set_errfile(slice, container->db_errfile);
	slice->db_errpfx = container->db_errpfx;
	slice->db_event_func = container->db_event_func;
	slice->db_feedback = container->db_feedback;
	slice->flags = container->flags;
#ifdef HAVE_CRYPTO
	if (container->passwd != NULL && (ret =
	    __env_set_encrypt(slice, container->passwd, DB_ENCRYPT_AES)) != 0)
		return (ret);
#endif
	if (container->intermediate_dir_mode == NULL)
		slice->intermediate_dir_mode = NULL;
	else if ((ret = __os_strdup(slice->env,
	    container->intermediate_dir_mode,
	    &slice->intermediate_dir_mode)) != 0)
		return (ret);
	slice->is_alive = container->is_alive;
	slice->lg_bsize = container->lg_bsize;
	slice->lg_filemode = container->lg_filemode;
	slice->lg_regionmax = container->lg_regionmax;
	slice->lg_size = container->lg_size;
	slice->lg_flags = container->lg_flags;
	if (container->lk_modes != 0 && (ret = __lock_set_lk_conflicts(slice,
	    container->lk_conflicts, container->lk_modes)) != 0)
		return (ret);
	slice->lk_detect = container->lk_detect;
	slice->lk_max_lockers = container->lk_max_lockers;
	slice->lk_max = container->lk_max;
	slice->lk_max_objects = container->lk_max_objects;
	slice->lk_partitions = container->lk_partitions;
	slice->lk_init = container->lk_init;
	slice->lk_init_objects = container->lk_init_objects;
	slice->lk_init_lockers = container->lk_init_lockers;
	slice->lg_fileid_init = container->lg_fileid_init;
	slice->mp_maxopenfd = container->mp_maxopenfd;
	slice->object_t_size = container->object_t_size;
	slice->thr_init = container->thr_init;
	slice->tx_init = container->tx_init;
	slice->memory_max = container->memory_max;
	slice->mp_maxopenfd = container->mp_maxopenfd;
	slice->mp_mtxcount = container->mp_mtxcount;
	slice->mp_maxwrite = container->mp_maxwrite;
	slice->mp_maxwrite_sleep = container->mp_maxwrite_sleep;
	slice->db_msgcall = container->db_msgcall;
	slice->db_msgfile = container->db_msgfile;
	slice->db_msgpfx = container->db_msgpfx;
	slice->mp_pagesize = container->mp_pagesize ;
	slice->mp_tablesize = container->mp_tablesize;
	slice->db_paniccall = container->db_paniccall;
	slice->thr_max = container->thr_max;
	slice->thread_id = container->thread_id;
	slice->thread_id_string = container->thread_id_string;
	slice->envreg_timeout = container->envreg_timeout;
	slice->mutex_failchk_timeout = container->mutex_failchk_timeout;
	if ((ret = __env_set_timeout(slice,
	    container->lk_timeout, DB_SET_LOCK_TIMEOUT)) != 0)
		return (ret);
	if ((ret = __env_set_timeout(slice,
	    container->tx_timeout, DB_SET_TXN_TIMEOUT)) != 0)
		return (ret);
	slice->verbose = container->verbose;
	slice->tx_init = container->tx_init;
	slice->tx_max = container->tx_max;
	slice->tx_timeout = container->tx_timeout;
	slice->tx_timestamp = container->tx_timestamp;

	/* Directories are inherited only when they are relative paths. */

	return (ret);
}

/*
 * __env_set_slice_count --
 *	DB_ENV->set_slice_count.
 *
 * PUBLIC: int  __env_set_slice_count __P((DB_ENV *, u_int32_t));
 */
int
__env_set_slice_count(dbenv, count)
	DB_ENV *dbenv;
	u_int32_t count;
{
	DB_ENV *slice;
	ENV *env;
	int ret;
	db_slice_t i;

	env = dbenv->env;
	ret = 0;
	ENV_ILLEGAL_AFTER_OPEN(env, "set_slice_count")

	if (dbenv->slice_cnt != 0) {
		 if (dbenv->slice_cnt == count)
			return (0);

		ret = USR_ERR(env, EINVAL);
		__db_errx(env, DB_STR("1598",
	    "set_slice_count has already been specified for this environment"));
		return (ret);
	}

	if (count > DB_MAX_SLICES) {
		ret = USR_ERR(env, EINVAL);
		__db_errx(env, DB_STR_A("1599",
		    "An environment may have at most %d slices", "%d"),
		    DB_MAX_SLICES);
		return (ret);
	}

	dbenv->slice_cnt = count;

	/*
	 * Create the slices' environment handles so that they can be configured
	 * by later lines in DB_CONFIG.
	 */
	if ((ret = __os_calloc(env,
	    dbenv->slice_cnt + 1, sizeof(DB_ENV *), &env->slice_envs)) != 0)
		goto err;
	for (i = 0; i < dbenv->slice_cnt; i++) {
		if ((ret = db_env_create(&slice, 0)) != 0)
			goto err;
		slice->env->slice_index = i;
		env->slice_envs[i] = slice;

		/*
		 * Copy the sensible configuration settings from the containing
		 * environment to the slice, including callback functions.
		 */
		if ((ret = __env_slice_configure(dbenv, slice)) != 0)
			goto err;
	}

err:
	return (ret);

}

/*
 * __slice_iterate --
 *	Return each non-null slice of a sliced environment.
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
 * PUBLIC: DB_ENV *__slice_iterate __P((DB_ENV *, int *));
 */
DB_ENV *
__slice_iterate(dbenv, pos)
	DB_ENV *dbenv;
	int *pos;
{
	ENV *env;
	db_slice_t i;

	env = dbenv->env;
	if (env->slice_envs == NULL)
		return (NULL);
	i = (db_slice_t)(1 + *pos);
	DB_ASSERT(env, i <= dbenv->slice_cnt);
	while (i < dbenv->slice_cnt && env->slice_envs[i] == NULL)
		i++;

	*pos = (int)i;
	/* This returns a good DB_ENV *, or the NULL just after the last one. */
	return (env->slice_envs[i]);
}

/*
 * __env_slice_dbremove --
 *	Invoke dbremove in all slices, continuing on any errors, ignoring
 *	ENOENT. This is called inside of the commit of the container's
 *	transaction that removes the user-visible piece of the db.
 *
 * PUBLIC: int __env_slice_dbremove
 * PUBLIC:     __P((ENV *, const char *, const char *, u_int32_t));
 */
int
__env_slice_dbremove(env, name, subdb, flags)
	ENV *env;
	const char *name;
	const char *subdb;
	u_int32_t flags;
{
	DB_ENV *slice;
	int i, ret, t_ret;
	u_int32_t metaflags;

	if ((ret = __db_get_metaflags(env, name, &metaflags)) != 0) {
		/* If it has been removed already, silently skip it. */
		if (ret == ENOENT)
			ret = 0;
		return (ret);
	}
	if (!FLD_ISSET(metaflags, DBMETA_SLICED))
		return (0);
	for (i = -1; (slice = __slice_iterate(env->dbenv, &i)) != NULL; ) {
		if ((t_ret = __env_dbremove_pp(slice, NULL, name,
		    subdb, flags)) != 0 && t_ret != ENOENT && ret == 0)
			ret = t_ret;
	}
	return (ret);
}
#else

/*
 * These stub functions are used when BDB is configured without --enable-slice.
 */

/*
 * __env_no_slices -
 *	Raise the error that this build was
 *	configured *without* --enable_slices.
 *
 *
 * PUBLIC: #ifndef HAVE_SLICES
 * PUBLIC: int  __env_no_slices __P((ENV *));
 * PUBLIC: #endif
 */
int
__env_no_slices(env)
	ENV *env;
{
	int err;

	err = USR_ERR(env, DB_OPNOTSUP);
	__db_err(env, err, "Library build did not include slice support");
	return (err);
}

int
__env_get_slice_count(dbenv, countp)
	DB_ENV *dbenv;
	u_int32_t *countp;
{
	COMPQUIET(countp, NULL);
	return (__env_no_slices(dbenv->env));
}

int
__env_get_slices(dbenv, retp)
	DB_ENV *dbenv;
	DB_ENV ***retp;
{
	*retp = NULL;
	return (__env_no_slices(dbenv->env));
}

int
__env_set_slice_count(dbenv, count)
	DB_ENV *dbenv;
	u_int32_t count;
{
	COMPQUIET(count, 0);
	return (__env_no_slices(dbenv->env));
}

#endif

/*
 * __env_not_sliced -
 *	Raise the error that this environment's DB_CONFIG did not enable slices;
 *	that is it did not include a set_slice_count parameter, or that slice
 *	support was not enabled in this build.
 *
 * PUBLIC: int  __env_not_sliced __P((ENV *));
 */
int
__env_not_sliced(env)
	ENV *env;
{
	int	err;

#ifdef HAVE_SLICES
	err = USR_ERR(env, EINVAL);
	__db_errx(env, "This environment was not configured with slices");
#else
	err = __env_no_slices(env);
#endif
	return (err);
}
