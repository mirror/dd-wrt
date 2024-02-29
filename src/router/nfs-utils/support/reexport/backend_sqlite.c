#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_GETRANDOM
# include <sys/random.h>
# if !defined(SYS_getrandom) && defined(__NR_getrandom)
   /* usable kernel-headers, but old glibc-headers */
#  define SYS_getrandom __NR_getrandom
# endif
#endif

#include "conffile.h"
#include "reexport_backend.h"
#include "xlog.h"

#define REEXPDB_DBFILE NFS_STATEDIR "/reexpdb.sqlite3"
#define REEXPDB_DBFILE_WAIT_USEC (5000)

static sqlite3 *db;
static int init_done;

#if !defined(HAVE_GETRANDOM) && defined(SYS_getrandom)
/* libc without function, but we have syscall */
static int getrandom(void *buf, size_t buflen, unsigned int flags)
{
	return (syscall(SYS_getrandom, buf, buflen, flags));
}
# define HAVE_GETRANDOM
#endif

static int prng_init(void)
{
	int seed;

	if (getrandom(&seed, sizeof(seed), 0) != sizeof(seed)) {
		xlog(L_ERROR, "Unable to obtain seed for PRNG via getrandom()");
		return -1;
	}

	srand(seed);
	return 0;
}

static void wait_for_dbaccess(void)
{
	usleep(REEXPDB_DBFILE_WAIT_USEC + (rand() % REEXPDB_DBFILE_WAIT_USEC));
}

static bool sqlite_plug_init(void)
{
	char *sqlerr;
	int ret;

	if (init_done)
		return true;

	if (prng_init() != 0)
		return false;

	ret = sqlite3_open_v2(conf_get_str_with_def("reexport", "sqlitedb", REEXPDB_DBFILE),
			      &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
			      NULL);
	if (ret != SQLITE_OK) {
		xlog(L_ERROR, "Unable to open reexport database: %s", sqlite3_errstr(ret));
		return false;
	}

again:
	ret = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS fsidnums (num INTEGER PRIMARY KEY CHECK (num > 0 AND num < 4294967296), path TEXT UNIQUE); CREATE INDEX IF NOT EXISTS idx_ids_path ON fsidnums (path);", NULL, NULL, &sqlerr);
	switch (ret) {
	case SQLITE_OK:
		init_done = 1;
		ret = 0;
		break;
	case SQLITE_BUSY:
	case SQLITE_LOCKED:
		wait_for_dbaccess();
		goto again;
	default:
		xlog(L_ERROR, "Unable to init reexport database: %s", sqlite3_errstr(ret));
		sqlite3_free(sqlerr);
		sqlite3_close_v2(db);
		ret = -1;
	}

	return ret == 0 ? true : false;
}

static void sqlite_plug_destroy(void)
{
	if (!init_done)
		return;

	sqlite3_close_v2(db);
}

static bool get_fsidnum_by_path(char *path, uint32_t *fsidnum, bool *found)
{
	static const char fsidnum_by_path_sql[] = "SELECT num FROM fsidnums WHERE path = ?1;";
	sqlite3_stmt *stmt = NULL;
	bool success = false;
	int ret;

	*found = false;

	ret = sqlite3_prepare_v2(db, fsidnum_by_path_sql, sizeof(fsidnum_by_path_sql), &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_WARNING, "Unable to prepare SQL query '%s': %s", fsidnum_by_path_sql, sqlite3_errstr(ret));
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 1, path, -1, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_WARNING, "Unable to bind SQL query '%s': %s", fsidnum_by_path_sql, sqlite3_errstr(ret));
		goto out;
	}

again:
	ret = sqlite3_step(stmt);
	switch (ret) {
	case SQLITE_ROW:
		*fsidnum = sqlite3_column_int(stmt, 0);
		success = true;
		*found = true;
		break;
	case SQLITE_DONE:
		/* No hit */
		success = true;
		*found = false;
		break;
	case SQLITE_BUSY:
	case SQLITE_LOCKED:
		wait_for_dbaccess();
		goto again;
	default:
		xlog(L_WARNING, "Error while looking up '%s' in database: %s", path, sqlite3_errstr(ret));
	}

out:
	sqlite3_finalize(stmt);
	return success;
}

static bool sqlite_plug_path_by_fsidnum(uint32_t fsidnum, char **path, bool *found)
{
	static const char path_by_fsidnum_sql[] = "SELECT path FROM fsidnums WHERE num = ?1;";
	sqlite3_stmt *stmt = NULL;
	bool success = false;
	int ret;

	*found = false;

	ret = sqlite3_prepare_v2(db, path_by_fsidnum_sql, sizeof(path_by_fsidnum_sql), &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_WARNING, "Unable to prepare SQL query '%s': %s", path_by_fsidnum_sql, sqlite3_errstr(ret));
		goto out;
	}

	ret = sqlite3_bind_int(stmt, 1, fsidnum);
	if (ret != SQLITE_OK) {
		xlog(L_WARNING, "Unable to bind SQL query '%s': %s", path_by_fsidnum_sql, sqlite3_errstr(ret));
		goto out;
	}

again:
	ret = sqlite3_step(stmt);
	switch (ret) {
	case SQLITE_ROW:
		*path = strdup((char *)sqlite3_column_text(stmt, 0));
		if (*path) {
			*found = true;
			success = true;
		} else {
			xlog(L_WARNING, "Out of memory");
		}
		break;
	case SQLITE_DONE:
		/* No hit */
		*found = false;
		success = true;
		break;
	case SQLITE_BUSY:
	case SQLITE_LOCKED:
		wait_for_dbaccess();
		goto again;
	default:
		xlog(L_WARNING, "Error while looking up '%i' in database: %s", fsidnum, sqlite3_errstr(ret));
	}

out:
	sqlite3_finalize(stmt);
	return success;
}

static bool new_fsidnum_by_path(char *path, uint32_t *fsidnum)
{
	/*
	 * This query is a little tricky. We use SQL to find and claim the smallest free fsid number.
	 * To find a free fsid the fsidnums is left joined to itself but with an offset of 1.
	 * Everything after the UNION statement is to handle the corner case where fsidnums
	 * is empty. In this case we want 1 as first fsid number.
	 */
	static const char new_fsidnum_by_path_sql[] = "INSERT INTO fsidnums VALUES ((SELECT ids1.num + 1 FROM fsidnums AS ids1 LEFT JOIN fsidnums AS ids2 ON ids2.num = ids1.num + 1 WHERE ids2.num IS NULL UNION SELECT 1 WHERE NOT EXISTS (SELECT NULL FROM fsidnums WHERE num = 1) LIMIT 1), ?1) RETURNING num;";

	sqlite3_stmt *stmt = NULL;
	int ret, check = 0;
	bool success = false;

	ret = sqlite3_prepare_v2(db, new_fsidnum_by_path_sql, sizeof(new_fsidnum_by_path_sql), &stmt, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_WARNING, "Unable to prepare SQL query '%s': %s", new_fsidnum_by_path_sql, sqlite3_errstr(ret));
		goto out;
	}

	ret = sqlite3_bind_text(stmt, 1, path, -1, NULL);
	if (ret != SQLITE_OK) {
		xlog(L_WARNING, "Unable to bind SQL query '%s': %s", new_fsidnum_by_path_sql, sqlite3_errstr(ret));
		goto out;
	}

again:
	ret = sqlite3_step(stmt);
	switch (ret) {
	case SQLITE_ROW:
		*fsidnum = sqlite3_column_int(stmt, 0);
		success = true;
		break;
	case SQLITE_CONSTRAINT:
		/* Maybe we lost the race against another writer and the path is now present. */
		check = 1;
		break;
	case SQLITE_BUSY:
	case SQLITE_LOCKED:
		wait_for_dbaccess();
		goto again;
	default:
		xlog(L_WARNING, "Error while looking up '%s' in database: %s", path, sqlite3_errstr(ret));
	}

out:
	sqlite3_finalize(stmt);

	if (check) {
		bool found = false;

		get_fsidnum_by_path(path, fsidnum, &found);
		if (!found)
			xlog(L_WARNING, "SQLITE_CONSTRAINT error while inserting '%s' in database", path);
	}

	return success;
}

static bool sqlite_plug_fsidnum_by_path(char *path, uint32_t *fsidnum, int may_create, bool *found)
{
	bool success;

	success = get_fsidnum_by_path(path, fsidnum, found);
	if (success) {
		if (!*found && may_create) {
			success = new_fsidnum_by_path(path, fsidnum);
			if (success)
				*found = true;
		}
	}

	return success;
}

struct reexpdb_backend_plugin sqlite_plug_ops = {
	.fsidnum_by_path = sqlite_plug_fsidnum_by_path,
	.path_by_fsidnum = sqlite_plug_path_by_fsidnum,
	.initdb = sqlite_plug_init,
	.destroydb = sqlite_plug_destroy,
};
