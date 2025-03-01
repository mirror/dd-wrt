/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2013, 2017 Oracle and/or its affiliates.  All rights reserved.
 */

#include "db_config.h"

#include "db_int.h"
#include "dbinc/db_page.h"
#include "dbinc/db_verify.h"
#include "dbinc/db_am.h"
#include "dbinc/blob.h"
#include "dbinc/fop.h"
#include "dbinc/txn.h"
#include "dbinc_auto/sequence_ext.h"

static int __blob_open_meta_db __P((
    DB *, DB_TXN *, DB **, DB_SEQUENCE **, int, int, int));
static int __blob_clean_dir
    __P((ENV *, DB_TXN *, const char *, const char *, int));
static int __blob_copy_dir __P((DB *, const char *, const char *));

#define	BLOB_ID_KEY "blob_id"
#define	BLOB_SEQ_DB_NAME "blob_id_seq"
#define	BLOB_DIR_ID_KEY "blob_dir_id"
#define	BLOB_DIR_SEQ_DB_NAME "blob_dir_id_seq"

/*
 * __blob_make_sub_dir --
 *	Create the name of the subdirectory in the blob directory
 * for the given database file and subdatabase ids.
 *
 * PUBLIC: int __blob_make_sub_dir __P((ENV *, char **, db_seq_t, db_seq_t));
 */
int
__blob_make_sub_dir(env, blob_sub_dir, file_id, db_id)
	ENV *env;
	char **blob_sub_dir;
	db_seq_t file_id;
	db_seq_t db_id;
{
	char fname[MAX_BLOB_PATH_SZ], dname[MAX_BLOB_PATH_SZ];
	int ret;
	size_t len;

	*blob_sub_dir = NULL;
	memset(fname, 0, MAX_BLOB_PATH_SZ);
	memset(dname, 0, MAX_BLOB_PATH_SZ);

	if (db_id == 0 && file_id == 0)
		return (0);

	if (db_id < 0 || file_id < 0)
		return (USR_ERR(env, EINVAL));

	/* The master db has no subdb id. */
	if (db_id != 0)
		(void)snprintf(dname, MAX_BLOB_PATH_SZ,
		    "%s%llu", BLOB_DIR_PREFIX, (unsigned long long)db_id);
	(void)snprintf(fname, MAX_BLOB_PATH_SZ, "%s%llu",
	    BLOB_DIR_PREFIX, (unsigned long long)file_id);

	len = strlen(fname) + (db_id ? strlen(dname) : 0) + 3;
	if ((ret = __os_malloc(env, len, blob_sub_dir)) != 0)
		goto err;
	if (db_id != 0)
		(void)sprintf(*blob_sub_dir, "%s%c%s%c", fname,
		    PATH_SEPARATOR[0], dname, PATH_SEPARATOR[0]);
	else
		(void)sprintf(*blob_sub_dir, "%s%c", fname, PATH_SEPARATOR[0]);

	return (0);

err:	if (*blob_sub_dir != NULL)
		__os_free(env, *blob_sub_dir);

	return (ret);
}

/*
 * __blob_make_meta_fname --
 *	Construct a (usually partial) path name of a blob metadata data file.
 *	It usually is relative to the environment home directory; only when a
 *	blob directory is configured and is an absolute path does this make a
 *	full path.
 *
 *	When dbp is set it constructs the blob metadata filename for that db;
 *	otherwise it constructs the environment-wide directory id filename.
 *
 * PUBLIC: int __blob_make_meta_fname __P((ENV *, DB *, char **));
 */
int
__blob_make_meta_fname(env, dbp, meta_fname)
	ENV *env;
	DB *dbp;
	char **meta_fname;
{
	char *fname, *sub_dir;
	int ret;
	size_t len;

	fname = NULL;
	len = strlen(BLOB_META_FILE_NAME) + 1;
	if (dbp == NULL) {
		sub_dir = "";
	} else {
		sub_dir = dbp->blob_sub_dir;
		DB_ASSERT(env, sub_dir != NULL);
		len += strlen(sub_dir);
	}
	if ((ret = __os_malloc(env, len, &fname)) != 0)
		goto err;

	snprintf(fname, len, "%s%s", sub_dir, BLOB_META_FILE_NAME);
	*meta_fname = fname;
	return (0);
err:
	if (fname != NULL)
		__os_free(env, fname);
	return (ret);
}

/*
 * __blob_get_dir --
 *	Get the root directory of this database's blob files.
 *
 * PUBLIC: int __blob_get_dir __P((DB *, char **));
 */
int
__blob_get_dir(dbp, dirp)
	DB *dbp;
	char **dirp;
{
	char *blob_dir;
	int ret;

	*dirp = NULL;

	if (dbp->blob_sub_dir == NULL)
		return (0);

	/* Get the path of the blob directory for this database. */
	if ((ret = __db_appname(dbp->env,
	    DB_APP_BLOB, dbp->blob_sub_dir, NULL, &blob_dir)) != 0)
		goto err;

	*dirp = blob_dir;
	return (0);

err:	if (blob_dir != NULL)
		__os_free(dbp->env, blob_dir);

	return (ret);
}

/*
 * __blob_open_meta_db --
 *	Open or create a blob meta database. This can be either
 *	the environment-wide db used to generate blob directory ids (__db1), or
 *	the per-db db used to generate blob ids (__db.bl001).
 */
static int
__blob_open_meta_db(dbp, txn, meta_db, seq, file, create, force_txn)
	DB *dbp;
	DB_TXN *txn;
	DB **meta_db;
	DB_SEQUENCE **seq;
	int file;
	int create;
	int force_txn;
{
#ifdef HAVE_64BIT_TYPES
	ENV *env;
	DB *blob_meta_db;
	DBT key;
	DB_SEQUENCE *blob_seq;
	DB_THREAD_INFO *ip;
	DB_TXN *local_txn;
	char *fullname, *fname, *dname, *path;
	int free_paths, ret, use_txn;
	u_int32_t flags;

	flags = 0;
	fullname = fname = NULL;
	blob_meta_db = NULL;
	blob_seq = NULL;
	local_txn = NULL;
	env = dbp->env;
	free_paths = use_txn = 0;
	memset(&key, 0, sizeof(DBT));

	/*
	 * Get the directory of the database, the meta db file name,
	 * and the sub-db name.
	 * file: blob directory/meta-file-name
	 * else: blob directory/per-db-blobdir/meta-file-name
	 */
	if (file) {
		key.data = BLOB_DIR_ID_KEY;
		key.size = (u_int32_t)strlen(BLOB_DIR_ID_KEY);
		dname = BLOB_DIR_SEQ_DB_NAME;
		fname = BLOB_META_FILE_NAME;
	} else {
		key.data = BLOB_ID_KEY;
		key.size = (u_int32_t)strlen(BLOB_ID_KEY);
		dname = BLOB_SEQ_DB_NAME;
		if ((ret = __blob_make_meta_fname(env,
		    file ? NULL : dbp, &fname)) < 0)
			goto err;
		free_paths = 1;
		if (dbp->open_flags & DB_THREAD)
			LF_SET(DB_THREAD);
	}

	if ((ret = __db_appname(env, DB_APP_BLOB, fname, NULL, &fullname)) != 0)
		goto err;

	path = fullname;
	/*
	 * Create the blob, database file, and database name directories. The
	 * mkdir isn't logged, so __fop_create_recover needs to do this as well.
	 */
	if (__os_exists(env, fullname, NULL) != 0) {
	    if (!create) {
		    ret = USR_ERR(env, ENOENT);
		    goto err;
	    } else if ((ret = __db_mkpath(env, path)) != 0)
		    goto err;
	}

	if ((ret = __db_create_internal(&blob_meta_db, env, 0)) != 0)
		goto err;

	if (create) {
		LF_SET(DB_CREATE);
		/*
		 * Make the pagesize of the meta database the same as that
		 * of the dbp passed in.  For the environment-wide db this
		 * will be the first database created.  For a per-db db
		 * this will be the owning database.
		 */
		if (dbp->pgsize != 0) {
			if ((ret = __db_set_pagesize(
			    blob_meta_db, dbp->pgsize)) != 0)
				goto err;
		}
	}

	/* Disable blobs in the blob meta databases themselves. */
	if ((ret = __db_set_blob_threshold(blob_meta_db, 0, 0)) != 0)
		goto err;

	/*
	 * To avoid concurrency issues, the blob meta database is
	 * opened and operated on in a local transaction.  One
	 * exception is when the blob meta database is created in the
	 * same txn as the parent db.  Then the blob meta database
	 * shares the given txn, so if the txn is rolled back, the
	 * creation of the blob meta database will also be rolled back.
	 * Another exception is when the master in replication opens
	 * the meta database to find the highest blob id. In that case
	 * it needs to be able to roll back the open call so the log
	 * is not propagated to the clients.
	 */
	if ((!file && IS_REAL_TXN(dbp->cur_txn)) || force_txn)
		use_txn = 1;

	ENV_GET_THREAD_INFO(env, ip);
	if (IS_REAL_TXN(txn)) {
		if (use_txn)
			local_txn = txn;
		else {
			if ((ret = __txn_begin(
			    env, ip, NULL, &local_txn, DB_IGNORE_LEASE)) != 0)
				goto err;
		}
	}
	if ((ret = __db_open(blob_meta_db, ip, local_txn, fname, dname,
	    DB_BTREE, flags | DB_INTERNAL_BLOB_DB, 0, PGNO_BASE_MD)) != 0)
		goto err;

	/* Open the sequence that holds the blob ids. */
	if ((ret = db_sequence_create(&blob_seq, blob_meta_db, 0)) != 0)
		goto err;

	/* No-op if already initialized, 0 is an invalid value for blob ids. */
	if ((ret = __seq_initial_value(blob_seq, 1)) != 0)
		goto err;
	if ((ret = __seq_open(blob_seq, local_txn, &key, flags)) != 0)
		goto err;

	if (local_txn != NULL && use_txn == 0 &&
	    (ret = __txn_commit(local_txn, 0)) != 0) {
		local_txn = NULL;
		goto err;
	}
	__os_free(env, fullname);
	if (free_paths)
		__os_free(env, fname);
	*meta_db = blob_meta_db;
	*seq = blob_seq;
	return (0);

err:
	if (fullname)
		__os_free(env, fullname);
	if (fname != NULL && free_paths)
		__os_free(env, fname);
	if (local_txn != NULL && use_txn == 0)
		(void)__txn_abort(local_txn);
	if (blob_seq != NULL)
		(void)__seq_close(blob_seq, 0);
	if (blob_meta_db != NULL)
		(void)__db_close(blob_meta_db, NULL, 0);
	return (ret);

#else /*HAVE_64BIT_TYPES*/
	__db_errx(dbp->env, DB_STR("0217",
	    "library build did not include support for external files"));
	return (DB_OPNOTSUP);
#endif
}

/*
 * __blob_generate_dir_ids --
 *
 * Generate the unique ids used to create a blob directory for the database.
 * Only one argument is needed.  Files with one database only need the
 * file id.  The master database only needs the file id, and
 * subdatabases inherit the file id from the master, so they only need the
 * subdatabase id.
 *
 * PUBLIC: int __blob_generate_dir_ids
 * PUBLIC:	__P((DB *, DB_TXN *, db_seq_t *));
 */
int
__blob_generate_dir_ids(dbp, txn, id)
	DB *dbp;
	DB_TXN *txn;
	db_seq_t *id;
{
	DB *blob_meta_db;
	DB_SEQUENCE *blob_seq;
	int ret;
	u_int32_t flags;

#ifdef HAVE_64BIT_TYPES
	flags = 0;
	blob_meta_db = NULL;
	blob_seq = NULL;

	if ((ret = __blob_open_meta_db(
	    dbp, txn, &blob_meta_db, &blob_seq, 1, 1, 0)) != 0)
		goto err;

	if (IS_REAL_TXN(txn))
		LF_SET(DB_AUTO_COMMIT | DB_TXN_NOSYNC);

	DB_ASSERT(dbp->env, id != NULL);
	if (*id == 0) {
		if ((ret = __seq_get(blob_seq, 0, 1, id, flags)) != 0)
			goto err;
	}

err:	if (blob_seq != NULL)
		(void)__seq_close(blob_seq, 0);
	if (blob_meta_db != NULL)
		(void)__db_close(blob_meta_db, NULL, 0);
	return (ret);
#else /*HAVE_64BIT_TYPES*/
	COMPQUIET(dbp, NULL);
	COMPQUIET(txn, NULL);
	__db_errx(dbp->env, DB_STR("0218",
	    "library build did not include support for external files"));
	return (DB_OPNOTSUP);
#endif
}

/*
 * __blob_generate_id --
 * Generate a new blob ID.
 *
 * PUBLIC: int __blob_generate_id __P((DB *, DB_TXN *, db_seq_t *));
 */
int
__blob_generate_id(dbp, txn, blob_id)
	DB *dbp;
	DB_TXN *txn;
	db_seq_t *blob_id;
{
#ifdef HAVE_64BIT_TYPES
	DB_TXN *ltxn;
	int ret;
	u_int32_t flags;
	flags = DB_IGNORE_LEASE;
	ltxn = NULL;

	if (dbp->blob_seq == NULL) {
		if ((ret = __blob_open_meta_db(dbp, txn,
		    &dbp->blob_meta_db, &dbp->blob_seq, 0, 1, 0)) != 0)
			goto err;
	}

	/*
	 * If this is the opening transaction of the database, use it instead
	 * of auto commit.  Otherwise it could deadlock with the transaction
	 * used to open the blob meta database in __blob_open_meta_db.
	 */
	if (IS_REAL_TXN(dbp->cur_txn))
		ltxn = txn;

	if (IS_REAL_TXN(txn) && ltxn == NULL)
		LF_SET(DB_AUTO_COMMIT | DB_TXN_NOSYNC);

	if ((ret = __seq_get(dbp->blob_seq, ltxn, 1, blob_id, flags)) != 0)
		goto err;

err:	return (ret);
#else /*HAVE_64BIT_TYPES*/
	COMPQUIET(blob_id, NULL);
	__db_errx(dbp->env, DB_STR("0219",
	    "library build did not include support for external files"));
	return (DB_OPNOTSUP);
#endif
}

/*
 * __blob_highest_id
 *
 * Returns the highest id in the blob meta database.
 *
 * PUBLIC: int __blob_highest_id __P((DB *, DB_TXN *, db_seq_t *));
 */
int
__blob_highest_id(dbp, txn, id)
	DB *dbp;
	DB_TXN *txn;
	db_seq_t *id;
{
#ifdef HAVE_64BIT_TYPES
	int ret;

	*id = 0;
	if (dbp->blob_sub_dir == NULL) {
		if ((ret = __blob_make_sub_dir(dbp->env, &dbp->blob_sub_dir,
		    dbp->blob_file_id, dbp->blob_sdb_id)) != 0)
				goto err;
	}
	if (dbp->blob_seq == NULL) {
		ret = __blob_open_meta_db(dbp, txn,
		    &dbp->blob_meta_db, &dbp->blob_seq, 0, 0, 1);
		/*
		 * It is not an error if the blob meta database does not
		 * exist.
		 */
		if (ret == ENOENT)
			ret = 0;
		if (ret != 0)
			goto err;
	}

	ret = __seq_get(dbp->blob_seq, txn, 0, id, DB_CURRENT);
err:
	return (ret);
#else /*HAVE_64BIT_TYPES*/
	COMPQUIET(id, NULL);
	__db_errx(dbp->env, DB_STR("0245",
	    "library build did not include support for external files"));
	return (DB_OPNOTSUP);
#endif
}

/*
 * __blob_calculate_dirs
 *
 * Use a blob id to to determine the path below the blob subdirectory in
 * which the blob file is located.  Assumes enough space exists in the path
 * variable to hold the path.
 *
 * PUBLIC: void __blob_calculate_dirs __P((db_seq_t, char *, int *, int *));
 */
void
__blob_calculate_dirs(blob_id, path, len, depth)
	db_seq_t blob_id;
	char *path;
	int *len;
	int *depth;
{
	int i;
	db_seq_t factor, tmp;

	/* Calculate the subdirectories from the blob id. */
	factor = 1;
	for ((*depth) = 0, tmp = blob_id/BLOB_DIR_ELEMS;
	    tmp != 0; tmp = tmp/BLOB_DIR_ELEMS, (*depth)++)
		factor *= BLOB_DIR_ELEMS;

	for (i = (*depth); i > 0; i--) {
		tmp = (blob_id / factor) % BLOB_DIR_ELEMS;
		factor /= BLOB_DIR_ELEMS;
		(*len) += sprintf(path + (*len),
			"%03llu%c", (unsigned long long)tmp, PATH_SEPARATOR[0]);
	}
}

/*
 * __blob_id_to_path --
 * Generate the file name and blob specific part of the path for a particular
 * blob_id. The __db_appname API is used to generate a fully qualified path.
 * The caller must deallocate the path.
 *
 * PUBLIC: int __blob_id_to_path __P((ENV *,
 * PUBLIC:	 const char *, db_seq_t, char **, int));
 */
int
__blob_id_to_path(env, blob_sub_dir, blob_id, ppath, create)
	ENV *env;
	const char *blob_sub_dir;
	db_seq_t blob_id;
	char **ppath;
	int create;
{
	char *path, *tmp_path;
	int depth, name_len, ret;
	size_t len;

	name_len = 0;
	path = tmp_path = *ppath = NULL;

	if (blob_id < 1) {
		ret = USR_ERR(env, EINVAL);
		goto err;
	}

	len = MAX_BLOB_PATH_SZ + strlen(blob_sub_dir) + 1;
	if ((ret = __os_malloc(env, len, &path)) != 0)
		goto err;

	memset(path, 0, len);
	name_len += sprintf(path, "%s", blob_sub_dir);

	__blob_calculate_dirs(blob_id, path, &name_len, &depth);

	/*
	 * Populate the file name. Ensure there are 3 digits for each directory
	 * level (even if they are 0).
	 */
	(void)sprintf(path + name_len, "%s%0*llu",
	    BLOB_FILE_PREFIX, (depth + 1) * 3, (unsigned long long)blob_id);

	/* If this is the first file in the directory, ensure it exists. */
	if (blob_id % BLOB_DIR_ELEMS == 0 && depth > 0 && create) {
		if ((ret = __db_appname(
		    env, DB_APP_BLOB, path, NULL, &tmp_path)) != 0 )
			goto err;

		if ((ret = __db_mkpath(env, tmp_path)) != 0) {
			__db_errx(env, DB_STR("0221",
			    "Error creating external file directory."));
			ret = USR_ERR(env, EINVAL);
			goto err;
		}
		__os_free(env, tmp_path);
	}

	*ppath = path;
	return (0);

err:
	if (tmp_path != NULL)
		__os_free(env, tmp_path);
	if (path != NULL)
		__os_free(env, path);

	return (ret);
}

/*
 * __blob_str_to_id
 *
 * If the given string is a positive number, it returns it as a signed
 * 64 bit integer.  Otherwise the number is returned as 0.
 *
 * PUBLIC:  int __blob_str_to_id __P((ENV *, const char **, db_seq_t *));
 */
int
__blob_str_to_id(env, path, id)
	ENV *env;
	const char **path;
	db_seq_t *id;
{
	db_seq_t i;
	const char *p;
	char buf[2];

	p = *path;
	i = 10;
	*id = 0;
	buf[1] = '\0';
	while (p[0] >= '0' && p[0] <= '9') {
		*id *= i;
		buf[0] = p[0];
		*id += atoi(buf);
		if (*id < 0) {
			__db_errx(env, DB_STR("0246",
			    "External file id integer overflow."));
			return (USR_ERR(env, EINVAL));
		}
		p++;
	}
	*path = p;
	return (0);
}

/*
 * __blob_path_to_dir_ids --
 * Get the file and subdatabase ids from a path to a blob file
 * or a path in the blob directory structure.  Skips the
 * subdatabase directory id if sdb_id is NULL.
 *
 * PUBLIC: int __blob_path_to_dir_ids
 * PUBLIC:	__P((ENV *, const char *, db_seq_t *, db_seq_t *));
 */
int
__blob_path_to_dir_ids(env, path, file_id, sdb_id)
	ENV *env;
	const char *path;
	db_seq_t *file_id;
	db_seq_t *sdb_id;
{
	int ret;
	size_t len;
	const char *p;

	*file_id = 0;
	if (sdb_id != NULL)
		*sdb_id = 0;
	ret = 0;
	p = path;

	/*
	 * The blob file and subdatabase directories are of the form __db###,
	 * so search the string for any directories that match that form.
	 */
	len = strlen(path);
	do {
		p = strstr(p, BLOB_DIR_PREFIX);
		if (p == NULL || p > (path + len + 4))
			return (ret);
		p += 4;
	} while (p[0] < '0' || p[0] > '9');

	/* The file id should be next in the path. */
	if ((ret = __blob_str_to_id(env, &p, file_id)) != 0)
		return (ret);

	/* Quit now if a subdatabase argument was not passed. */
	if (sdb_id == NULL)
		return (ret);

	p = strstr(p, BLOB_DIR_PREFIX);
	/* It is okay for the path not to include a sdb_id. */
	if (p == NULL || p > (path + 4 + len))
		return (ret);

	p += 4;
	ret = __blob_str_to_id(env, &p, sdb_id);

	return (ret);
}

/*
 * __blob_salvage --
 *
 * Print a blob file during salvage.  The function assumes the DBT already has
 * a buffer large enough to hold "size" bytes.
 *
 * PUBLIC: int __blob_salvage __P((ENV *, db_seq_t, off_t, size_t,
 * PUBLIC:	db_seq_t, db_seq_t, DBT *));
 */
int
__blob_salvage(env, blob_id, offset, size, file_id, sdb_id, dbt)
	ENV *env;
	db_seq_t blob_id;
	off_t offset;
	size_t size;
	db_seq_t file_id;
	db_seq_t sdb_id;
	DBT *dbt;
{
	DB_FH *fhp;
	char *blob_sub_dir, *dir, *path;
	int isdir, ret;
	size_t bytes;

	blob_sub_dir = dir = path = NULL;
	fhp = NULL;

	if (blob_id < 1 || file_id < 0 || 
	    sdb_id < 0 || (file_id == 0 && sdb_id == 0)) {
		ret = USR_ERR(env, ENOENT);
		goto err;
	}

	if ((ret = __blob_make_sub_dir(env, &blob_sub_dir,
	    file_id, sdb_id)) != 0 || blob_sub_dir == NULL) {
	  	if (ret == 0)
			ret = USR_ERR(env, ENOENT);
		goto err;
	}

	if ((ret = __blob_id_to_path(env, blob_sub_dir, blob_id, &dir, 0)) != 0)
		goto err;

	if ((ret = __db_appname(env, DB_APP_BLOB, dir, NULL, &path)) != 0)
		goto err;

	if ((__os_exists(env, path, &isdir)) != 0 || isdir != 0) {
	  	ret = USR_ERR(env, ENOENT);
	    	goto err;
	}

	if ((ret = __os_open(env, path, 0, DB_OSO_RDONLY, 0, &fhp)) != 0)
		goto err;

	if ((ret = __os_seek(env, fhp, 0, 0, offset)) != 0)
		goto err;

	if ((ret = __os_read(env, fhp, dbt->data, size, &bytes)) != 0)
		goto err;

	dbt->size = (u_int32_t)bytes;
	if (bytes != size)
		ret = EIO;

err:	if (fhp != NULL)
		(void)__os_closehandle(env, fhp);
	if (dir != NULL)
		__os_free(env, dir);
	if (path != NULL)
		__os_free(env, path);
	if (blob_sub_dir != NULL)
		__os_free(env, blob_sub_dir);
	return (ret);
}

/*
 * __blob_vrfy --
 *
 * Checks that a blob file for the given blob id exists, and is the given size.
 *
 * PUBLIC: int __blob_vrfy __P((ENV *, db_seq_t, off_t,
 * PUBLIC:	db_seq_t, db_seq_t, db_pgno_t, u_int32_t));
 */
int
__blob_vrfy(env, blob_id, blob_size, file_id, sdb_id, pgno, flags)
	ENV *env;
	db_seq_t blob_id;
	off_t blob_size;
	db_seq_t file_id;
	db_seq_t sdb_id;
	db_pgno_t pgno;
	u_int32_t flags;
{
	DB_FH *fhp;
	char *blob_sub_dir, *dir, *path;
	int isdir, ret;
	off_t actual_size;
	u_int32_t mbytes, bytes;

	blob_sub_dir = dir = path = NULL;
	fhp = NULL;
	isdir = 0;

	if ((ret = __blob_make_sub_dir(
	    env, &blob_sub_dir, file_id, sdb_id)) != 0 || blob_sub_dir == NULL) {
		if (ret != ENOMEM)
			ret = DB_VERIFY_BAD;
		goto err;
	}

	ret = DB_VERIFY_BAD;

	if (__blob_id_to_path(env, blob_sub_dir, blob_id, &dir, 0) != 0) {
		EPRINT((env, DB_STR_A("0222",
		    "Page %lu: Error getting path to external file for %llu",
		    "%lu %llu"), (u_long)pgno, (unsigned long long)blob_id));
		goto err;
	}
	if (__db_appname(env, DB_APP_BLOB, dir, NULL, &path) != 0) {
		EPRINT((env, DB_STR_A("0223",
		    "Page %lu: Error getting path to external file for %llu",
		    "%lu %llu"), (u_long)pgno, (unsigned long long)blob_id));
		goto err;
	}
	if ((__os_exists(env, path, &isdir)) != 0 || isdir != 0) {
		EPRINT((env, DB_STR_A("0224",
		    "Page %lu: external file does not exist at %s",
		    "%lu %s"), (u_long)pgno, path));
		goto err;
	}
	if (__os_open(env, path, 0, DB_OSO_RDONLY, 0, &fhp) != 0) {
		EPRINT((env, DB_STR_A("0225",
		    "Page %lu: Error opening external file at %s",
		    "%lu %s"), (u_long)pgno, path));
		goto err;
	}
	if (__os_ioinfo(env, path, fhp, &mbytes, &bytes, NULL) != 0) {
		EPRINT((env, DB_STR_A("0226",
		    "Page %lu: Error getting external file size at %s",
		    "%lu %s"), (u_long)pgno, path));
		goto err;
	}

	actual_size = ((off_t)mbytes * (off_t)MEGABYTE) + bytes;
	if (blob_size != actual_size) {
		EPRINT((env, DB_STR_A("0227",
"Page %lu: external file size does not match size in database record: %llu %llu",
		    "%lu %llu %llu"), (u_long)pgno,
		    (unsigned long long)actual_size,
		    (unsigned long long)blob_size));
		goto err;
	}

	ret = 0;

err:	if (fhp != NULL)
		(void)__os_closehandle(env, fhp);
	if (dir != NULL)
		__os_free(env, dir);
	if (path != NULL)
		__os_free(env, path);
	if (blob_sub_dir != NULL)
		__os_free(env, blob_sub_dir);
	return (ret);
}

/*
 * __blob_del_hierarchy --
 *
 * Deletes the entire blob directory.  Used by replication.
 *
 * PUBLIC: int __blob_del_hierarchy __P((ENV *));
 */
int
__blob_del_hierarchy(env)
	ENV *env;
{
	int ret;
	char *blob_dir;

	blob_dir = NULL;

	if ((ret = __db_appname(env, DB_APP_BLOB, NULL, NULL, &blob_dir)) != 0)
		goto err;

	if ((ret = __blob_clean_dir(env, NULL, blob_dir, NULL, 0)) != 0)
		goto err;

err:	if (blob_dir != NULL)
		__os_free(env, blob_dir);
	return (ret);
}

/*
 * __blob_del_all --
 *
 * Deletes all the blob files and meta databases in a database's blob
 * directory.  Does not delete the directories if the delete is transactionally
 * protected, since there is no current way to undo a directory delete in case
 * the operation is aborted.
 *
 * PUBLIC: int __blob_del_all __P((DB *, DB_TXN *, int));
 */
int
__blob_del_all(dbp, txn, istruncate)
	DB *dbp;
	DB_TXN *txn;
	int istruncate;
{
#ifdef HAVE_64BIT_TYPES
	ENV *env;
	char *path;
	int isdir, ret;

	env = dbp->env;
	path = NULL;
	ret = 0;

	if (dbp->blob_sub_dir == NULL) {
		if ((ret = __blob_make_sub_dir(env, &dbp->blob_sub_dir,
		    dbp->blob_file_id, dbp->blob_sdb_id)) != 0)
			goto err;
	}

	/* Do nothing if blobs are not enabled. */
	if (dbp->blob_sub_dir == NULL ||
	    (dbp->blob_file_id == 0 && dbp->blob_sdb_id == 0))
		goto err;

	if ((ret = __blob_get_dir(dbp, &path)) != 0)
		goto err;

	/* Close the blob meta data databases, they are about to be deleted. */
	if (!istruncate) {
		if (dbp->blob_seq != NULL) {
			if ((ret = __seq_close(dbp->blob_seq, 0)) != 0)
			    goto err;
			dbp->blob_seq = NULL;
		}
		if (dbp->blob_meta_db != NULL) {
			if ((ret =
			    __db_close(dbp->blob_meta_db, NULL, 0)) != 0)
			    goto err;
			dbp->blob_meta_db = NULL;
		}
	}

	/*
	 * The blob directory may not exist if blobs were enabled,
	 * but none were created.
	 */
	if (__os_exists(env, path, &isdir) != 0)
		goto err;

	if ((ret = __blob_clean_dir(
	    env, txn, path, dbp->blob_sub_dir, istruncate)) != 0)
		goto err;

	if (!IS_REAL_TXN(txn) && !istruncate) {
		if ((ret = __os_rmdir(env, path)) != 0)
			goto err;
	}

err:	if (path != NULL)
		__os_free(env, path);
	return (ret);

#else /*HAVE_64BIT_TYPES*/
	__db_errx(dbp->env, DB_STR("0220",
	    "library build did not include support for external files"));
	return (DB_OPNOTSUP);
#endif

}

/*
 * __blob_clean_dir --
 *
 * Delete all files in the given directory, and all files
 * in all sub-directories.  Does not remove directories if the operation is
 * transactionally protected.
 */
static int
__blob_clean_dir(env, txn, dir, subdir, istruncate)
	ENV *env;
	DB_TXN *txn;
	const char *dir;
	const char *subdir;
	int istruncate;
{
	DB *meta;
	DB_THREAD_INFO *ip;
	char *blob_dir, **dirs, *fname, full_path[DB_MAXPATHLEN], *local_path;
	int count, i, isdir, ret, t_ret;

	count = 0;
	dirs = NULL;
	fname = NULL;
	meta = NULL;

	/* Get a list of all files in the directory. */
	if ((ret = __os_dirlist(env, dir, 1, &dirs, &count)) != 0) {
		if (ret == ENOENT)
			ret = 0;
		goto err;
	}

	for (i = 0; i < count; i++) {
		(void)sprintf(full_path, "%s%c%s%c",
		    dir, PATH_SEPARATOR[0], dirs[i], '\0');

		if (__os_exists(env, full_path, &isdir) != 0)
			continue;

		/* If it is a directory, clean it.  Else remove the file. */
		if (isdir) {
			if ((ret = __blob_clean_dir(
			    env, txn, full_path, subdir, istruncate)) != 0)
				goto err;
			/* Delete the top directory. */
			if (!IS_REAL_TXN(txn)) {
				if ((ret = __os_rmdir(env, full_path)) != 0)
					goto err;
			}
		} else if (strcmp(dirs[i], BLOB_META_FILE_NAME) == 0 ) {
			/* Ignore the meta db when truncating. */
			if (istruncate)
				continue;
			blob_dir = (env->dbenv->db_blob_dir != NULL ?
			    env->dbenv->db_blob_dir : BLOB_DEFAULT_DIR);
			if ((fname = strstr(full_path, blob_dir)) == NULL)
				goto err;
			fname += strlen(blob_dir) + 1;
			if ((ret = __db_create_internal(&meta, env, 0)) != 0)
				goto err;
			ENV_GET_THREAD_INFO(env, ip);
			if ((ret = __db_remove_int(meta,
			    ip, txn, fname, NULL, 0)) != 0)
				goto err;
			/*
			 * Closing the local DB handle releases the transaction
			 * locks, but those have to remain until the
			 * transaction is resolved, so NULL the DB locker.
			 * See __env_dbremove_pp for more details.
			 */
			if (IS_REAL_TXN(txn))
				meta->locker = NULL;
			if ((t_ret = __db_close(
			    meta, NULL, DB_NOSYNC)) != 0 && ret == 0)
				ret = t_ret;
			meta = NULL;
			if (ret != 0)
				goto err;
		} else {
			if (!IS_REAL_TXN(txn))
				ret = __os_unlink(env, full_path, 0);
			else {
				local_path = (subdir == NULL ? full_path :
				    strstr(full_path, subdir));
				if (local_path != NULL)
					ret = __fop_remove(env, txn, NULL,
					    local_path, NULL, DB_APP_BLOB, 0);
			}
			if (ret != 0)
				goto err;
		}
	}
err:	if (meta != NULL) {
		if ((t_ret = __db_close(
		    meta, NULL, DB_NOSYNC)) != 0 && ret == 0)
			ret = t_ret;
	}
	if (dirs != NULL)
		__os_dirfree(env, dirs, count);

	return (ret);
}

/*
 * __blob_copy_all --
 *	Copy all files in the blob directory.
 *
 * PUBLIC: int __blob_copy_all __P((DB*, const char *, u_int32_t));
 */
int __blob_copy_all(dbp, target, flags)
	DB *dbp;
	const char *target;
	u_int32_t flags;
{
	DB_THREAD_INFO *ip;
	ENV *env;
	char *blobdir, *fullname, *metafname, new_target[DB_MAXPATHLEN];
	const char *path;
	int ret;

	env = dbp->env;
	blobdir = NULL;
	fullname = NULL;
	metafname = NULL;
	ret = 0;

	/* Do nothing if blobs are not enabled. */
	if (dbp->blob_sub_dir == NULL || dbp->blob_threshold == 0)
		return (0);

	/* Create the directory structure in the target directory. */
	if (env->dbenv->db_blob_dir != NULL)
		path = env->dbenv->db_blob_dir;
	else
		path = BLOB_DEFAULT_DIR;

	/*
	 * Default blob directory will be maintained in the target
	 * directory only when it is backing up a single directory.
	 */
	(void)snprintf(new_target, sizeof(new_target), "%s%c%s%c%c",
	    target, PATH_SEPARATOR[0], LF_ISSET(DB_BACKUP_SINGLE_DIR) ?
	    BLOB_DEFAULT_DIR : path, PATH_SEPARATOR[0], '\0');
	path = new_target;

	if ((ret = __db_mkpath(env, path)) != 0)
		goto err;

	/* Copy the directory id database. */
	if ((ret = __blob_make_meta_fname(env, NULL, &metafname)) != 0)
		goto err;
	if ((ret = __db_appname(env,
	    DB_APP_BLOB, metafname, NULL, &fullname)) != 0)
		goto err;
	path = fullname;
	/* Remove env home from the full path of directory id database. */
	if (!__os_abspath(fullname) &&
	    env->db_home != NULL && (env->db_home)[0] != '\0')
		path += (strlen(env->db_home) + 1);
	ENV_GET_THREAD_INFO(env, ip);

	if ((ret = __db_dbbackup(
	    dbp->dbenv, ip, path, new_target, 0, 0, metafname)) != 0)
		goto err;

	if ((ret = __blob_get_dir(dbp, &blobdir)) != 0)
		goto err;

	/*
	 * The blob directory may not exist if blobs were enabled,
	 * but none were created.
	 */
	if (__os_exists(env, blobdir, NULL) != 0)
		goto err;

	(void)sprintf(new_target + strlen(new_target),
	     "%s%c", dbp->blob_sub_dir, '\0');
	if ((ret = __blob_copy_dir(dbp, blobdir, new_target)) != 0)
		goto err;

err:	if (blobdir != NULL)
		__os_free(env, blobdir);
	if (metafname != NULL)
		__os_free(env, metafname);
	if (fullname != NULL)
		__os_free(env, fullname);
	return (ret);
}

/*
 * __blob_copy_dir --
 *	Copy all files in the given directory, and all files
 *	in all sub-directories.
 */
static int
__blob_copy_dir(dbp, dir, target)
	DB *dbp;
	const char *dir;
	const char *target;
{
	DB_THREAD_INFO *ip;
	ENV *env;
	char **dirs, full_path[DB_MAXPATHLEN], new_target[DB_MAXPATHLEN];
	int count, i, isdir, ret;

	env = dbp->env;
	count = 0;
	dirs = NULL;

	/* Create the directory sturcture in the target directory. */
	if ((ret = __db_mkpath(env, target)) != 0)
		goto err;

	ENV_GET_THREAD_INFO(env, ip);
	/* Get a list of all files in the directory. */
	if ((ret = __os_dirlist(env, dir, 1, &dirs, &count)) != 0)
		goto err;

	for (i = 0; i < count; i++) {
		(void)sprintf(full_path, "%s%c%s%c",
		    dir, PATH_SEPARATOR[0], dirs[i], '\0');

		if (__os_exists(env, full_path, &isdir) != 0)
			continue;

		/*
		 * If it is a directory, copy the files in it.
		 * Else if it is the meta database, call __db_dbbackup, else
		 * copy the file.
		 */
		if (isdir) {
			(void)sprintf(new_target,
			    "%s%c%s%c%c", target, PATH_SEPARATOR[0],
			    dirs[i], PATH_SEPARATOR[0], '\0');
			if ((ret = __blob_copy_dir(
			    dbp, full_path, new_target)) != 0)
				goto err;
		} else {
			if (strcmp(dirs[i], BLOB_META_FILE_NAME) == 0) {
				(void)sprintf(full_path, "%s%c%s%c",
				    dbp->blob_sub_dir,
				    PATH_SEPARATOR[0], dirs[i], '\0');
				if ((ret = __db_dbbackup(dbp->dbenv, ip,
				    full_path, target, 0, 0,
				    BLOB_META_FILE_NAME)) != 0)
					goto err;
			} else {
				if ((ret = backup_data_copy(
				    dbp->dbenv, dirs[i], dir, target, 0)) != 0)
					goto err;
			}
		}
	}

err:
	if (dirs != NULL)
		__os_dirfree(env, dirs, count);
	return (ret);
}
