/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
 */
#include "userauth.h"

/*
 * One SQLite source file(sqlite/ext/userauth/userauth.c, updated at 
 * 2014-09-08) is appended at the end of this file. Several changes are added 
 * into this SQLite code to support the key-store based authentication. 
 */

#ifdef SQLITE_USER_AUTHENTICATION
static DB_ENV* authGetDbEnv(sqlite3 *);

static DB_ENV* authGetDbEnv(sqlite3 *db)
{
	struct Btree *pBtree;
	if (db != NULL && db->nDb > 0 && db->aDb != NULL) {
		pBtree = db->aDb[0].pBt;
		if (pBtree->pBt != NULL)
			return pBtree->pBt->dbenv;
	}

	return (NULL);
}
#endif

/***********************************************************************
 * Key-store based user authentication specific code.
 ***********************************************************************/
#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE

#define AUTH_CLOSE(fhp)	do {						\
	__os_closehandle(NULL, fhp);					\
	(fhp) = NULL;							\
} while (0)

#define AUTH_EXISTS(filepath)						\
	!__os_exists(NULL, filepath, NULL)	

#define AUTH_FILESIZE(fhp, mbytes, bytes)				\
	__os_ioinfo(NULL, NULL, fhp, mbytes, bytes, NULL)

#define AUTH_IO(fhp, op, offset, addr, len, niop)			\
	__os_io(NULL, op, fhp, 0, 0, offset, len,			\
	(u_int8_t*)(addr), niop) 

#define AUTH_OPEN(filepath, fhpp, mode, perm)				\
	__os_open(NULL, filepath, 0, mode, perm, fhpp) 

#define AUTH_READ(fhp, addr, len, nrp)					\
	__os_read(NULL, fhp, addr, len, nrp) 

#define AUTH_REMOVE(filepath)						\
	__os_unlink(NULL, filepath, 0)

#define AUTH_RENAME(src, des)						\
	__os_rename(NULL, src, des, 0)

#define AUTH_SEEK(fhp, offset) 						\
	__os_seek(NULL, fhp, 0, 0, offset)

#define AUTH_WRITE(fhp, addr, len, nwp)					\
	__os_write(NULL, fhp, addr, len, nwp) 

static int authComputeKsChksum(DB_FH *, u_int8_t *);
static int authCopyFile(const char *, const char *);
static int authCreateKseData(sqlite3 *, const char *, int, u_int8_t *, 
    KS_ENTRY_DATA **);
static int authCreateKseHeader(sqlite3 *, const char *, KS_ENTRY_DATA *, 
    KS_ENTRY_HDR **);
static int authCreateOneKsFile(char *, DB_FH **); 
static int authDecryptKseData(const char *, int, KS_ENTRY_DATA *);
static void authGetFilePath(sqlite3 *, const char *, char *);
static void authGetKsBakFile(sqlite3 *, char *);
static void authGetKsDelTmpFile(sqlite3 *, char *);
static void authGetKsFile(sqlite3 *, char *);
static void authGetKsLckFile(sqlite3 *, char *);
static void authGetKsTmpFile(sqlite3 *, char *);
static int authGetKsVersion(const char *, u_int32_t *);
static void authGetPwdHash(const char *, int, u_int8_t *, u_int8_t *);
static int authGetSaltFromUserTable(sqlite3 *, const char *, u_int8_t *);
static int authIncreaseKsVersion(const char *);
static int authInitEncryptedTmpEnv(const u_int8_t *, int, DB_ENV **);
static int authProcessKsFile(sqlite3 *, const char *, const char *, int, 
    int (*)(KS_CB_ARG *), u_int32_t);
static int authUpdateKsChksum(const char *);
static int authVerifyKsChksum(sqlite3 *);
static int authZeroKsVersion(const char *);

/* Keystore routines called in the user authenticaion API. */
static void auth_keystore_backup(sqlite3 *);
static int auth_keystore_create(sqlite3 *);
static int auth_keystore_lock(sqlite3 *);
static int auth_keystore_remove(sqlite3 *);
static int auth_keystore_unlock(sqlite3 *);
static int auth_useradd_key(sqlite3 *, int);
static int auth_useradd_keystore(sqlite3 *, const char *, const char *, int);
static int auth_useradd_keystore_cb(KS_CB_ARG *);
static int auth_useredit_keystore(sqlite3 *, const char *, const char *, int);
static int auth_useredit_keystore_cb(KS_CB_ARG *);
static int auth_userdelete_keystore(sqlite3 *, const char *);
static int auth_userdelete_keystore_cb(KS_CB_ARG *);
static int auth_userlogin_keystore(sqlite3 *, const char *, const char *, int);
static int auth_userlogin_keystore_cb(KS_CB_ARG *);

static void authGetFilePath(sqlite3 *db, const char *fname, char *filepath)
{
	struct BtShared *pBt;

	assert(db->aDb[0].pBt != NULL);
	pBt = db->aDb[0].pBt->pBt;

	if (pBt == NULL) {
		/* No env directory, use the current working directory. */
		sqlite3_snprintf(BT_MAX_PATH, filepath, fname);
		return;
	}
	sqlite3_mutex_enter(pBt->mutex);
	sqlite3_snprintf(BT_MAX_PATH, filepath, "%s/%s", pBt->dir_name, fname);
	sqlite3_mutex_leave(pBt->mutex);
}

static void authGetKsFile(sqlite3 *db, char *filepath)
{
	authGetFilePath(db, KS_FILE, filepath);
}

static void authGetKsBakFile(sqlite3 *db, char *filepath)
{
	authGetFilePath(db, KS_BAK_FILE, filepath);
}

static void authGetKsLckFile(sqlite3 *db, char *filepath)
{
	authGetFilePath(db, KS_LCK_FILE, filepath);
}

static void authGetKsTmpFile(sqlite3 *db, char *filepath)
{
	authGetFilePath(db, KS_TMP_FILE, filepath);
}

static void authGetKsDelTmpFile(sqlite3 *db, char *filepath)
{
	authGetFilePath(db, KS_DEL_TMP_FILE, filepath);
}

/* 
 * Create a file if it does not exist yet. If fhpp is NULL, we just close the
 * file handle; otherwise return the file handle by fhpp.
 */
static int authCreateOneKsFile(char *filepath, DB_FH **fhpp) 
{
	DB_FH *fhp;
	int rc;

	if ((rc = AUTH_OPEN(filepath, &fhp, DB_OSO_CREATE | DB_OSO_EXCL, 
	    0600)) == 0) {
		if (fhpp == NULL)
			AUTH_CLOSE(fhp);
		else 
			*fhpp = fhp;
	}
	return rc;
}

static int authCopyFile(const char *from, const char *to) 
{
	DB_FH *rfhp, *wfhp;
	size_t nr, nw, buf_size;
	int rc, ret;
	u_int8_t *buf;

	buf = NULL;
	buf_size = 4096;
	rfhp = wfhp = NULL;
	rc = SQLITE_OK;

	if ((buf = (u_int8_t*)sqlite3_malloc(buf_size)) == NULL) {
		rc = SQLITE_NOMEM;
		goto err;
	}

	if (AUTH_OPEN(from, &rfhp, DB_OSO_RDONLY, 0) != 0 ||
	    AUTH_OPEN(to, &wfhp, DB_OSO_CREATE | DB_OSO_TRUNC, 0600) != 0)
		goto io_err;

	while ((ret = AUTH_READ(rfhp, buf, buf_size, &nr)) == 0 && nr > 0)
		if (AUTH_WRITE(wfhp, buf, nr, &nw) != 0 || nr != nw)
			goto io_err;
	if (ret != 0)
		goto io_err;

	goto done;

io_err:
	rc = SQLITE_IOERR;

err:
done:	
	if (rfhp)
		AUTH_CLOSE(rfhp);
	if (wfhp)
		AUTH_CLOSE(wfhp);
	if (buf)
		sqlite3_free(buf);
	return rc;
}

static int authZeroKsVersion(const char *filepath)
{
	DB_FH *fhp;
	int rc;
	size_t nio;
	u_int8_t verBytes[KS_VER_LEN];

	fhp = NULL;
	rc = SQLITE_OK;
	memset(verBytes, 0, KS_VER_LEN);

	/* Process version field as byte array to avoid endianness issue. */
	if (AUTH_OPEN(filepath, &fhp, 0, 0) != 0 || AUTH_IO(fhp, DB_IO_WRITE, 
	    KS_CHKSUM_LEN, verBytes, KS_VER_LEN, &nio) != 0)
		goto io_err;

	goto done;

io_err:
	rc = SQLITE_IOERR;

done:
	if (fhp)
		AUTH_CLOSE(fhp);
	return rc;
}

static int authGetKsVersion(const char *filepath, u_int32_t *version)
{
	DB_FH *fhp;
	size_t nio;
	int rc, i;
	u_int8_t verBytes[KS_VER_LEN];
	u_int32_t ver;

	*version = ver = 0;
	fhp = NULL;
	rc = SQLITE_OK;

	/* Process version field as byte array to avoid endianness issue. */
	if (AUTH_OPEN(filepath, &fhp, DB_OSO_RDONLY, 0) != 0 || AUTH_IO(fhp, 
	    DB_IO_READ, KS_CHKSUM_LEN, verBytes, KS_VER_LEN, &nio) != 0)
		goto io_err;

	for (i = 0; i < KS_VER_LEN; i++)
		ver = (ver << 8) + verBytes[i];
	*version = ver;

	goto done;

io_err:
	rc = SQLITE_IOERR;

done:
	if (fhp)
		AUTH_CLOSE(fhp);
	return rc;
}

static int authIncreaseKsVersion(const char *filepath)
{
	DB_FH *fhp;
	int rc, i;
	size_t nio;
	u_int8_t verBytes[KS_VER_LEN];

	rc = SQLITE_OK;
	fhp = NULL;

	/* Process version field as byte array to avoid endianness issue. */
	if (AUTH_OPEN(filepath, &fhp, 0, 0) != 0 || AUTH_IO(fhp, DB_IO_READ, 
	    KS_CHKSUM_LEN, verBytes, KS_VER_LEN, &nio) != 0)
		goto io_err;

	for (i = KS_VER_LEN - 1; i >= 0; i--)
		if (verBytes[i] < 255) {
			verBytes[i]++;
			break;
		} else
			verBytes[i] = 0;

	if (i < 0) {
		/* Version overflowed. Should never happen. */
		rc = SQLITE_ERROR;
		goto err;
	}

	if (AUTH_IO(fhp, DB_IO_WRITE, KS_CHKSUM_LEN, verBytes, KS_VER_LEN, 
	    &nio) != 0)
		goto io_err;

	goto done;

io_err:
	rc = SQLITE_IOERR;

err:
done:
	if (fhp)
		AUTH_CLOSE(fhp);
	return rc;
}

static int authComputeKsChksum(DB_FH *fhp, u_int8_t *chksum) 
{
	int rc, ret;
	size_t buf_len, nbytes, nr;
	u_int8_t *buf;

	nbytes = 4096;
	buf_len = nbytes + KS_CHKSUM_LEN;
	rc = SQLITE_OK;

	if ((buf = (u_int8_t*)sqlite3_malloc(buf_len)) == NULL) {
		rc = SQLITE_NOMEM;
		goto err;
	}

	if (AUTH_SEEK(fhp, KS_CHKSUM_LEN) != 0)
		goto io_err;

	/*
	 * We compute the checksum by every 4096 bytes. The previous checksum 
	 * value will be put at the beginning of the buffer for computation of 
	 * next checksum. If this is an empty file, the chksum will be filled 
	 * with 0.
	 */
	memset(chksum, 0, KS_CHKSUM_LEN);
	memset(buf, 0, buf_len);
	while ((ret = AUTH_READ(fhp, buf + KS_CHKSUM_LEN, nbytes, &nr)) == 0 
	    && nr > 0) {
		__db_chksum(NULL, buf, KS_CHKSUM_LEN + nr, NULL, chksum);
		memcpy(buf, chksum, KS_CHKSUM_LEN);
	}
	if (ret != 0)
		goto io_err;

	goto done;

io_err:
	rc = SQLITE_IOERR;

err:
done:
	if (buf)
		sqlite3_free(buf);
	return rc;
}

static int authUpdateKsChksum(const char *ksPath) 
{
	DB_FH *fhp;
	int rc;
	size_t nio;
	u_int8_t chksum[KS_CHKSUM_LEN];

	fhp = NULL;
	rc = SQLITE_OK;

	if (AUTH_OPEN(ksPath, &fhp, 0, 0) != 0)
		goto io_err;

	if ((rc = authComputeKsChksum(fhp, chksum)) != SQLITE_OK)
		goto err;

	if (AUTH_IO(fhp, DB_IO_WRITE, 0, chksum, KS_CHKSUM_LEN, &nio) != 0)
		goto io_err;
	
	goto done;

io_err:
	rc = SQLITE_IOERR;

err:
done:
	if (fhp)
		AUTH_CLOSE(fhp);
	return rc;
}

static int authVerifyKsChksum(sqlite3 *db)
{
	DB_FH *fhp;
	DB_ENV *dbenv;
	char ksPath[BT_MAX_PATH], ksBakPath[BT_MAX_PATH];
	u_int8_t chksum[KS_CHKSUM_LEN], oldChksum[KS_CHKSUM_LEN];
	int rc;
	size_t nr;
	u_int32_t ksVer, ksBakVer;

	fhp = NULL;
	rc = SQLITE_OK;
	dbenv = authGetDbEnv(db);
	authGetKsFile(db, ksPath);

again:
	if (!AUTH_EXISTS(ksPath)) {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv, "Keystore file missing.");
		goto err;
	}

	if (AUTH_OPEN(ksPath, &fhp, DB_OSO_RDONLY, 0) != 0 ||
	    AUTH_READ(fhp, oldChksum, KS_CHKSUM_LEN, &nr) != 0 || 
	    nr != KS_CHKSUM_LEN) {
		rc = SQLITE_IOERR;
		goto err;
	}

	if ((rc = authComputeKsChksum(fhp, chksum)) != SQLITE_OK)
		goto err;

	if (memcmp(chksum, oldChksum, KS_CHKSUM_LEN) != 0) {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv,
			"Keystore file corrupted. Restore it from backup.");
		goto err;
	}

	goto done;

err:
	authGetKsBakFile(db, ksBakPath);
	if (AUTH_EXISTS(ksBakPath)) { 
		if (fhp)
			AUTH_CLOSE(fhp);

		/*
		 * We only retry with the keystore backup file when the backup
		 * file exists and: 1. the keystore file is missing or 2. the 
		 * keystore file exists and the backup file version equals to 
		 * the keystore file version. We rename the backup file to 
		 * keystore file so we will at most retry once.
		 */
		if (!AUTH_EXISTS(ksPath) || 
		    (authGetKsVersion(ksPath, &ksVer) == SQLITE_OK &&
		    authGetKsVersion(ksBakPath, &ksBakVer) == SQLITE_OK &&
		    ksVer == ksBakVer))
			if (AUTH_RENAME(ksBakPath, ksPath) == 0) {
				rc = SQLITE_OK;
				goto again;
			}
	}
done:
	if (fhp)
		AUTH_CLOSE(fhp);
	return rc;
}

/*
 * Create a temporary environment with encryption key as the given password.
 * We use the environment's encryption handler to do the encryption/decryption.
 */
static int authInitEncryptedTmpEnv(const u_int8_t *aPW, int nPW, DB_ENV **tEnv)
{
	int rc;
	char *pwd;
    
	rc = 0;
	pwd = NULL;

	if ((rc = db_env_create(tEnv, 0)) != 0)
		goto err;

	/* Make a null-terminated password string. */
	if ((pwd = (char*)sqlite3_malloc(nPW + 1)) == NULL) {
		rc = ENOMEM;
		goto err;
	}
	memcpy(pwd, aPW, nPW);
	pwd[nPW] = '\0';

	if ((rc = (*tEnv)->set_encrypt(*tEnv, pwd, DB_ENCRYPT_AES)) != 0)
		goto err;

	if ((rc = (*tEnv)->open(*tEnv, NULL, 
	    DB_CREATE | DB_PRIVATE | DB_INIT_MPOOL, 0)) != 0)
		goto err;

err:
	if (pwd)
		sqlite3_free(pwd);
	return rc;
}

/*
 * Once this function returned successfully, the keystore entry header should 
 * be freed by the caller. 
 */
static int authCreateKseHdr(sqlite3 *db, const char *zUsername, 
    KS_ENTRY_DATA *data, KS_ENTRY_HDR **hdr) 
{
	int rc, entry_len, username_len;
	KS_ENTRY_HDR *t_hdr;
	DB_ENV *dbenv;

	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;
	username_len = strlen(zUsername);
	entry_len = 0;
	entry_len += sizeof(u_int8_t) * 2 + username_len;
	assert(data);
	entry_len += KSE_DATA_LEN(data);
	if (entry_len > MAX_KSE_LEN) {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv, "Username/password too long.");
		goto err;
	}

	*hdr = (KS_ENTRY_HDR*)sqlite3_malloc(entry_len);
	if ((t_hdr = *hdr) == NULL) {
		rc = SQLITE_NOMEM;
		goto err;
	}
	
	t_hdr->entry_len = entry_len;
	t_hdr->username_len = username_len;
	memcpy(t_hdr->username, zUsername, username_len);

err:
	return rc;
}

static void authGetPwdHash(const char *aPW, int nPW, u_int8_t *salt, 
    u_int8_t *hash)
{
	int i;

	__db_chksum(NULL, (u_int8_t*)aPW, nPW, salt, hash);

	/*
	 * The password hash will be used as a key to init BDB encryption;
	 * BDB takes a NULL terminated string as a key password, so we have to 
	 * remove the '\0' in this password hash.
	 */
	for (i = 0; i < AUTH_PW_HASH_LEN; i++)
		if (hash[i] == 0)
			hash[i]++;
}

/* 
 * Fetch the salt part of the 'pw' field from the user's record in 
 * 'sqlite_user' table. 
 */
static int authGetSaltFromUserTable(sqlite3 *db, const char *zUsername, 
    u_int8_t *salt) 
{
	sqlite3_stmt *pStmt;
	char *zSql;
	DB_ENV *dbenv;
	int rc;
	u_int8_t savedAuthLevel;

	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;
	pStmt = NULL;
	/* Temporarily change the logged user to an admin user. */
	savedAuthLevel = db->auth.authLevel;
	db->auth.authLevel = UAUTH_Admin;

	zSql = "SELECT pw from sqlite_user WHERE uname=?";
	sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
	sqlite3_bind_text(pStmt, 1, zUsername, -1, SQLITE_STATIC);
	if ((rc = sqlite3_step(pStmt)) == SQLITE_ROW) {
		memcpy(salt, sqlite3_column_blob(pStmt, 0), AUTH_PW_SALT_LEN);
		rc = SQLITE_OK;
	} else {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv, "No user %s in sqlite_user table.",
			    zUsername);
	}

	if (pStmt) 
		sqlite3_finalize(pStmt);
	db->auth.authLevel = savedAuthLevel;
	return rc;
}

/* 
 * Once this function returned successfully, the keystore entry data should be 
 * freed by the caller. 
 */
static int authCreateKseData(sqlite3 *db, const char *aPW, int nPW, 
    u_int8_t *salt, KS_ENTRY_DATA **data)
{
	DB_ENV *tEnv;
	int rc, nKey;
	size_t enc_data_len, ajust_len;
	KS_ENTRY_DATA *t_data;
	u_int8_t hash[AUTH_PW_HASH_LEN];
	struct BtShared *pBt;
	char *aKey;

	rc = 0;
	tEnv = NULL;
	assert(db->aDb[0].pBt != NULL);
	pBt = db->aDb[0].pBt->pBt;
	assert(pBt->encrypted);
	aKey = pBt->encrypt_pwd;
	nKey = pBt->encrypt_pwd_len;

	authGetPwdHash(aPW, nPW, salt, hash);
	if ((rc = authInitEncryptedTmpEnv(hash, AUTH_PW_HASH_LEN, &tEnv)) != 0)
		goto err;

	/* Buffer size ajusted by the bytes need to be encrypted. */
	enc_data_len = nKey + sizeof(u_int8_t);
	ajust_len = 0;
	__env_encrypt_adj_size(tEnv, enc_data_len, &ajust_len);
	enc_data_len += ajust_len;

	*data = (KS_ENTRY_DATA*)sqlite3_malloc(KSE_DATA_LEN_NOKEY + 
	    enc_data_len);
	if ((t_data = *data) == NULL) {
		rc = ENOMEM;
		goto err;
	}
	memcpy(t_data->salt, salt, AUTH_PW_SALT_LEN);
	t_data->key_len = nKey;
	t_data->buf_len = (u_int8_t)enc_data_len - sizeof(u_int8_t);
	memcpy(t_data->buf, aKey, nKey);

	rc = __env_encrypt(tEnv, t_data->iv, (u_int8_t*)&t_data->key_len,
	    KSE_DATA_ENCR_LEN(t_data));

err:
	if (tEnv)
		tEnv->close(tEnv, 0);
	return dberr2sqlite(rc, NULL);
}


static int authDecryptKseData(const char *aPW, int nPW, KS_ENTRY_DATA *data)
{
	DB_ENV *tEnv;
	int rc;
	u_int8_t hash[AUTH_PW_HASH_LEN];

	rc = 0;
	tEnv = NULL;
	memset(hash, 0, AUTH_PW_HASH_LEN);

	authGetPwdHash(aPW, nPW, data->salt, hash);
	if ((rc = authInitEncryptedTmpEnv(hash, AUTH_PW_HASH_LEN, &tEnv)) != 0)
		goto err;

	if ((rc = __env_decrypt(tEnv, data->iv, (u_int8_t*)&data->key_len,
	    KSE_DATA_ENCR_LEN(data))) != 0)
		goto err;

err:
	if (tEnv != NULL)
		tEnv->close(tEnv, 0);
	return dberr2sqlite(rc, NULL);
}

/*
 * This is the entry point for the user authentication API to access the
 * keystore file. We do the keystore file processing in a copy-on-write way. 
 * If we need to modify the keystore file, we copy it to a temporary file and 
 * do updates on the temporary file. We rename the temporary file to the 
 * keystore file when we are done. 
 *
 * We first find the location of an existing user entry in the keystore file, 
 * or the location to insert a new user entry. Then we pass this keystore file
 * stream and other information to the callback function. The file stream and 
 * KS_ENTRY_DATA parameters passed to the callback function will not be used 
 * when returned from the callback, so callback function could modify them 
 * freely. If callback function need to close the file stream, it must set the 
 * original file stream pointer to NULL.
 */
static int authProcessKsFile(sqlite3 *db, const char *zUsername, 
    const char *aPW, int nPW, int (*cb)(KS_CB_ARG*), u_int32_t flags)
{
	char *curKsPath;
	int rc, ret, username_len, readonly, append;
	size_t nr;
	off_t offset;
	u_int8_t buf[MAX_KSE_LEN];
	DB_FH *fhp;
	DB_ENV *dbenv;
	KS_ENTRY_HDR *hdr;
	KS_ENTRY_DATA *data;
	KS_CB_ARG cb_arg;
	char ksPath[BT_MAX_PATH], tmpPath[BT_MAX_PATH];

	cb_arg.db = db;
	cb_arg.fhpp = &fhp;
	offset = KS_CHKSUM_LEN + KS_VER_LEN;
	cb_arg.aPW = aPW;
	cb_arg.nPW = nPW;
	cb_arg.zUsername = zUsername;
	fhp = NULL;
	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;
	username_len = strlen(zUsername);
	readonly = flags & AUTH_KS_READONLY;
	append = flags & AUTH_KS_APPEND;

	authGetKsFile(db, ksPath);

	if ((rc = authVerifyKsChksum(db)) != SQLITE_OK)
		goto err;

	/* 
	 * We have verified the keystore file checksum, so we assume the 
	 * keystore file is good to use. We will use a temporary copy of the 
	 * keystore file if we are about to make changes in it. 
	 */
	if (!readonly) {
		authGetKsTmpFile(db, tmpPath);
		authCreateOneKsFile(tmpPath, NULL);
		if (authCopyFile(ksPath, tmpPath) != SQLITE_OK)
			goto io_err;
		curKsPath = tmpPath;
	} else 
		curKsPath = ksPath;
	cb_arg.ksPath = curKsPath;

	if (append) {
		/* 
		 * We are adding a keystore entry. Just pass control to the 
		 * callback function. 
		 */
		if ((rc = (*cb)(&cb_arg)) != SQLITE_OK)
			goto err;
		goto update_ks;
	}

	/* We need to look up user's entry in the keystore. */
        if (AUTH_OPEN(curKsPath, &fhp, 0, 0) != 0 || 
	    AUTH_SEEK(fhp, offset) != 0)
		goto io_err;
	hdr = (KS_ENTRY_HDR*)buf;

	/* Read one byte for the length of the next user entry. */
	while ((ret = AUTH_READ(fhp, &hdr->entry_len, 1, &nr)) == 0 && 
	    nr == 1) {
		/* Read the content of the next user entry. */
		if (AUTH_READ(fhp, &hdr->username_len, hdr->entry_len - 1, 
		    &nr) != 0 || nr != hdr->entry_len - 1)
			goto io_err;

		if (username_len == hdr->username_len && 
		    strncmp((char *)hdr->username, zUsername,
		        username_len) == 0) {
			/* We have found the user's entry in the keystore. */
			data = (KS_ENTRY_DATA*)(buf + KSE_HDR_LEN(hdr));
			cb_arg.hdr = hdr;
			cb_arg.data = data;
			cb_arg.offset = offset;
			/* Pass control to the callback function. */
			if ((rc = (*cb)(&cb_arg)) != SQLITE_OK)
				goto err;
			goto update_ks;
		}
		offset += hdr->entry_len;
	}
	if (ret != 0)
		goto io_err;

	/* We have not found the user's entry in the keystore file. */
	rc = SQLITE_ERROR;

	if (dbenv != NULL)
		dbenv->errx(dbenv,
		    "No user %s was found in the keystore file.", zUsername);
	goto err;

update_ks:
	if (fhp) {
		AUTH_CLOSE(fhp);
		fhp = NULL;
	}
	if (!readonly) {
		if ((rc = authIncreaseKsVersion(curKsPath)) != SQLITE_OK ||
		    (rc = authUpdateKsChksum(curKsPath)) != SQLITE_OK)
			goto err;

		/* Rename the temporary file to the keystore file. */
		if (AUTH_RENAME(curKsPath, ksPath) != 0) {
			rc = SQLITE_ERROR;
			goto err;
		}
	}
	goto done;

io_err:
	rc = SQLITE_IOERR;

err:
	if (fhp)
		AUTH_CLOSE(fhp);
	if (!readonly)
		AUTH_REMOVE(tmpPath);
done:
	return rc;
}

static int auth_useradd_key(sqlite3 *db, int isAdmin)
{
	struct BtShared *pBt;
	DB_ENV *dbenv;
	u_int8_t *key;
	int rc, i, database_existed;
	u_int8_t key_len;

	assert(db->aDb[0].pBt != NULL);
	pBt = db->aDb[0].pBt->pBt;
	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;
	key = NULL;
	database_existed = AUTH_EXISTS(pBt->full_name);

	/*
	 * Encryption is mandatory for keystore based user authentication. If 
	 * the database already exists but no encryption key is provided yet, 
	 * we return an error from here. This is because:
	 * 1. if database is not encrypted, it violates the encryption 
	 * mandatory rule;
	 * 2. if database is encrypted, the next operation to add user into
	 * database will fail with BDB encryption errors.
	 */ 
	if (database_existed && pBt->encrypt_pwd == NULL) {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv,
"Cannot add a user to an existing database environment without an encryption "
"key provided or a valid user authenticated."); 
		goto err;
	}
	
	/*
	 * The database does not exist. This means we are in a bootstrap. 
	 * If no encryption key is provided, we will generate one.
	 */
	if (!database_existed && pBt->encrypt_pwd == NULL) {
		/*
		 * We are in a bootstrap, so the first user's isAdmin 
		 * settting must be true.
		 */
		if (!isAdmin) {
			rc = SQLITE_AUTH;
			if (dbenv != NULL)
				dbenv->errx(dbenv,
				    "The first user must be an admin user.");
			goto err;
		}

		/* Set key length to be a random number between 16 and 31. */
		sqlite3_randomness(sizeof(key_len), &key_len);
		key_len = key_len % DB_AES_CHUNK + DB_AES_CHUNK;
		if ((key = sqlite3_malloc(key_len)) == NULL) {
			rc = SQLITE_NOMEM;
			goto err;
		}
		sqlite3_randomness(key_len, key);
		/*
		 * BDB takes a NULL terminated string as a key password, so we 
		 * have to remove the '\0' in this generated key.
		 */
		for (i = 0; i < key_len; i++)
			if (key[i] == 0)
				sqlite3_randomness(1, &key[i]);

		if ((rc = sqlite3_key(db, key, key_len)) != SQLITE_OK)
			goto err;
	}

err:
	if (key)
		sqlite3_free(key);
	return rc;
}

static int auth_keystore_create(sqlite3 *db)
{
	DB_ENV *dbenv;
	char ksPath[BT_MAX_PATH];
	int rc, ret;

	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;
	authGetKsFile(db, ksPath);
	if ((ret = authCreateOneKsFile(ksPath, NULL)) != 0) { 
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv,
			    ret == EEXIST ? "Keystore file already exists." :
			    "Keystore file creation failed.");
		goto err;
	}

	if ((rc = authZeroKsVersion(ksPath)) != SQLITE_OK ||
	    (rc = authUpdateKsChksum(ksPath)) != SQLITE_OK)
		goto err;

err:
	return rc;
}

static int auth_keystore_remove(sqlite3 *db)
{
	char ksPath[BT_MAX_PATH];
	authGetKsFile(db, ksPath);
	return AUTH_REMOVE(ksPath);
}

static int auth_keystore_lock(sqlite3 *db)
{
	int retry, rc;
	DB_FH *fhp;
	char ksLckPath[BT_MAX_PATH];
	DB_ENV *dbenv;

	retry = 5;
	fhp = NULL;
	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;

	authGetKsLckFile(db, ksLckPath);
	while (--retry >= 0 && authCreateOneKsFile(ksLckPath, &fhp) == EEXIST)
		/* The lock file exists. Wait for 0.2 second then retry. */
		__os_yield(NULL, 0, 200000);

	if (retry < 0) {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv, "Cannot lock the keystore file. "
			    "Check if the lock file: %s already exists.", ksLckPath);
	}

	if (fhp)
		AUTH_CLOSE(fhp);
	return rc;
}

static int auth_keystore_unlock(sqlite3 *db)
{
	char ksLckPath[BT_MAX_PATH];
	authGetKsLckFile(db, ksLckPath);
	return AUTH_REMOVE(ksLckPath);
}

/* Copy the updated keystore file to its backup. */
static void auth_keystore_backup(sqlite3 *db)
{
	char ksPath[BT_MAX_PATH], ksBakPath[BT_MAX_PATH];
	authGetKsFile(db, ksPath);
	authGetKsBakFile(db, ksBakPath);
	authCreateOneKsFile(ksBakPath, NULL);
	authCopyFile(ksPath, ksBakPath);
}


static int auth_useradd_keystore_cb(KS_CB_ARG *args)
{
	int rc, nPW;
	size_t hdr_len, data_len, nw;
	sqlite3 *db;
	const char *zUsername, *aPW, *filepath;
	DB_FH *fhp;
	KS_ENTRY_HDR *hdr;
	KS_ENTRY_DATA *data;
	u_int8_t salt[AUTH_PW_SALT_LEN];
	u_int32_t mbytes, bytes;

	rc = SQLITE_OK;
	fhp = NULL;
	hdr = NULL;
	data = NULL;
	db = args->db;
	aPW = args->aPW;
	nPW = args->nPW;
	zUsername = args->zUsername;
	filepath = args->ksPath;

	if ((rc = authGetSaltFromUserTable(db, zUsername, salt)) != SQLITE_OK ||
	    (rc = authCreateKseData(db, aPW, nPW, salt, &data)) != SQLITE_OK ||
	    (rc = authCreateKseHdr(db, zUsername, data, &hdr)) != SQLITE_OK)
		goto err;

	hdr_len = KSE_HDR_LEN(hdr);
	data_len = KSE_DATA_LEN(data);

	/* Append the user entry header and data at the end of keystore file. */
	if (AUTH_OPEN(filepath, &fhp, 0, 0) != 0 ||
	    AUTH_FILESIZE(fhp, &mbytes, &bytes) != 0 ||
	    AUTH_SEEK(fhp, mbytes * MEGABYTE + bytes) != 0 ||
	    AUTH_WRITE(fhp, hdr, hdr_len, &nw) != 0 || hdr_len != nw ||
	    AUTH_WRITE(fhp, data, data_len, &nw) != 0 || data_len != nw) {
		rc = SQLITE_IOERR;
		goto err;
	}

err:
	if (hdr)
		sqlite3_free(hdr);
	if (data)
		sqlite3_free(data);
	if (fhp)
		AUTH_CLOSE(fhp);

	return rc;
}

static int auth_useradd_keystore(sqlite3 *db, const char *zUsername, 
    const char *aPW, int nPW)
{
	return authProcessKsFile(db, zUsername, aPW, nPW, 
	    auth_useradd_keystore_cb, AUTH_KS_APPEND);
}

static int auth_userlogin_keystore_cb(KS_CB_ARG *arg)
{
	DB_ENV *dbenv;
	KS_ENTRY_DATA *data;
	sqlite3 *db;
	int  rc;

	rc = SQLITE_OK;
	data = arg->data;
	db = arg->db;
	dbenv = authGetDbEnv(db);

	if ((rc = authDecryptKseData(arg->aPW, arg->nPW, data)) != SQLITE_OK)
		goto err;
	
	if (data->key_len > data->buf_len) {
		/*
		 * We get a wrong key. Most likely user has input an incorrect 
		 * password.
		 */
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv, "Fetched an wrong key from "
			"keystore. Check if the user's password is correct.");
		goto err;
	}

	if ((rc = sqlite3_key(arg->db, data->buf, data->key_len)) != SQLITE_OK)
		goto err;

err:
	return rc;
}

static int auth_userlogin_keystore(sqlite3 *db, const char *zUsername, 
    const char *aPW, int nPW)
{
	struct BtShared *pBt;

	assert(db->aDb[0].pBt != NULL);
	pBt = db->aDb[0].pBt->pBt;

	if (pBt->encrypt_pwd == NULL)
		return authProcessKsFile(db, zUsername, aPW, nPW, 
		    auth_userlogin_keystore_cb, AUTH_KS_READONLY);

	return SQLITE_OK;
}

static int auth_useredit_keystore_cb(KS_CB_ARG *arg)
{
	KS_ENTRY_DATA *newData;
	const char *aPW, *zUsername;
	off_t offset;
	int rc, nPW;
	size_t nio, data_len;
	DB_FH *fhp;
	u_int8_t salt[AUTH_PW_SALT_LEN];
	sqlite3 *db;

	newData = NULL;
	rc = SQLITE_OK;
	fhp = *(arg->fhpp);
	/* The file offset of the data part of user's entry in ks file. */
	offset = arg->offset + KSE_HDR_LEN(arg->hdr);
	aPW = arg->aPW;
	nPW = arg->nPW;
	db = arg->db;
	zUsername = arg->zUsername;

	if ((rc = authGetSaltFromUserTable(db, zUsername, salt)) != SQLITE_OK ||
	    (rc = authCreateKseData(db, aPW, nPW, salt, &newData)) != SQLITE_OK)
		goto err;

	data_len = KSE_DATA_LEN(newData);

	/* 
	 * The two KS_ENTRY_DATA object has the same encryption key, which 
	 * means they are the same size. So we just overwrite the original 
	 * object with the new one. 
	 */
	if (AUTH_IO(fhp, DB_IO_WRITE, offset, newData, data_len, &nio) != 0) {
		rc = SQLITE_IOERR;
		goto err;
	}

err:
	if (newData)
		sqlite3_free(newData);
	return rc;
}

static int auth_useredit_keystore(sqlite3 *db, const char *zUsername, 
    const char *zPW, int nPW)
{
	return authProcessKsFile(db, zUsername, zPW, nPW, 
	    auth_useredit_keystore_cb, 0);
}

static int auth_userdelete_keystore_cb(KS_CB_ARG *arg)
{
	int rc, ret;
	size_t entry_start_loc, entry_len, cur_loc, cur_len, buf_len, nr, nw;
	DB_FH *fhp, *tfhp;
	u_int8_t *buf;
	char tmpPath[BT_MAX_PATH];
	const char *ksPath;
	sqlite3 *db;

	/* To delete one user entry from the keystore file, we open a temporary
	 * file, copy the contents before and after the user entry from keystore
	 * file to the temporary file, then rename the temporary file back to
	 * the keystore file.
	 */
	fhp = *(arg->fhpp);
	tfhp = NULL;
	buf_len = 4096;
	rc = SQLITE_OK;
	entry_len = arg->hdr->entry_len;
	entry_start_loc = arg->offset;
	cur_loc = 0;
	db = arg->db;
	ksPath = arg->ksPath;

	if ((buf = (u_int8_t*)sqlite3_malloc(buf_len)) == NULL) {
		rc = SQLITE_NOMEM;
		goto err;
	}

	authGetKsDelTmpFile(db, tmpPath);
	authCreateOneKsFile(tmpPath, NULL);

	if (AUTH_OPEN(tmpPath, &tfhp, 0, 0) != 0 || AUTH_SEEK(fhp, 0) != 0)
		goto io_err;

	/* Copy the file content before the given user entry. */
	while (cur_loc < entry_start_loc) {
		cur_len = MIN(buf_len, entry_start_loc - cur_loc);
		if (AUTH_READ(fhp, buf, cur_len, &nr) != 0 || nr != cur_len ||
		    AUTH_WRITE(tfhp, buf, nr, &nw) != 0 || nr != nw)
			goto io_err;
		cur_loc += cur_len;
	}

	/* Ignore the given user entry. */
	if (AUTH_SEEK(fhp, entry_start_loc + entry_len) != 0)
		goto io_err;

	/* Copy the file content after the given user entry. */
	while ((ret = AUTH_READ(fhp, buf, buf_len, &nr)) == 0 && nr > 0)
		if (AUTH_WRITE(tfhp, buf, nr, &nw) != 0 || nr != nw)
			goto io_err;
	if (ret != 0)
		goto io_err;

	AUTH_CLOSE(fhp);
	*(arg->fhpp) = NULL;
	AUTH_CLOSE(tfhp);
	tfhp = NULL;

	if (AUTH_RENAME(tmpPath, ksPath) != 0)
		goto io_err;

	goto done;

io_err:
	rc = SQLITE_IOERR;

err:
done:
	if (buf)
		sqlite3_free(buf);
	if (tfhp)
		AUTH_CLOSE(tfhp);
	AUTH_REMOVE(tmpPath);
	return rc;
}

static int auth_userdelete_keystore(sqlite3 *db, const char *zUsername)
{
	return authProcessKsFile(db, zUsername, NULL, 0, 
	    auth_userdelete_keystore_cb, 0);
}
#endif /* BDBSQL_USER_AUTHENTICATION_KEYSTORE */

/*********************************************************************
 * Code below is a private copy from sqlite/ext/userauth/userauth.c.
 ********************************************************************/

/*
** 2014-09-08
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains the bulk of the implementation of the
** user-authentication extension feature.  Some parts of the user-
** authentication code are contained within the SQLite core (in the
** src/ subdirectory of the main source code tree) but those parts
** that could reasonable be separated out are moved into this file.
**
** To compile with the user-authentication feature, append this file to
** end of an SQLite amalgamation, then add the SQLITE_USER_AUTHENTICATION
** compile-time option.  See the user-auth.txt file in the same source
** directory as this file for additional information.
*/
#ifdef SQLITE_USER_AUTHENTICATION
#ifndef _SQLITEINT_H_
# include "sqliteInt.h"
#endif

/*
** Prepare an SQL statement for use by the user authentication logic.
** Return a pointer to the prepared statement on success. Return a
** NULL pointer if there is an error of any kind.
*/
static sqlite3_stmt* sqlite3UserAuthPrepare(sqlite3 *db, 
    const char *zFormat, ...)
{
	sqlite3_stmt *pStmt;
	char *zSql;
	int rc;
	va_list ap;
	int savedFlags = db->flags;

	va_start(ap, zFormat);
	zSql = sqlite3_vmprintf(zFormat, ap);
	va_end(ap);
	if (zSql == 0) 
		return 0;
	db->flags |= SQLITE_WriteSchema;
	rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
	db->flags = savedFlags;
	sqlite3_free(zSql);
	if (rc) {
		sqlite3_finalize(pStmt);
		pStmt = 0;
	}
	return pStmt;
}

/*
** Check to see if the sqlite_user table exists in database zDb.
*/
static int userTableExists(sqlite3 *db, const char *zDb, int *res)
{
	int rc = SQLITE_OK;
	char *zErr = NULL;
	sqlite3_mutex_enter(db->mutex);
	sqlite3BtreeEnterAll(db);
	if (db->init.busy == 0 && (rc = sqlite3Init(db, &zErr)) != SQLITE_OK)
		goto err;
	*res = sqlite3FindTable(db, "sqlite_user", zDb) != 0;
	sqlite3BtreeLeaveAll(db);
	sqlite3_mutex_leave(db->mutex);

err:
	if (zErr)
		sqlite3DbFree(db, zErr);
	return rc;
}

/*
** Check to see if database zDb has a "sqlite_user" table and if it does
** whether that table can authenticate zUser with nPw,zPw.  Write one of
** the UAUTH_* user authorization level codes into *peAuth and return a
** result code.
*/
static int userAuthCheckLogin(
    sqlite3 *db,               /* The database connection to check */
    const char *zDb,           /* Name of specific database to check */
    u8 *peAuth)                /* OUT: One of UAUTH_* constants */
{
	sqlite3_stmt *pStmt;
	int rc, usrTblExists;

	rc = SQLITE_OK;
	pStmt = NULL;
	usrTblExists = 0;

	*peAuth = UAUTH_Unknown;
	if ((rc = userTableExists(db, "main", &usrTblExists)) != SQLITE_OK)
		return rc;

	if (!usrTblExists) {
		/* No sqlite_user table.  Everybody is admin. */
		*peAuth = UAUTH_Admin;  
		return SQLITE_OK;
	}
	if (db->auth.zAuthUser == 0) {
		*peAuth = UAUTH_Fail;
		return SQLITE_OK;
	}
	pStmt = sqlite3UserAuthPrepare(db, "SELECT pw=sqlite_crypt(?1,pw), "
	    "isAdmin FROM \"%w\".sqlite_user WHERE uname=?2", zDb);
	if (pStmt == 0) 
		return SQLITE_NOMEM;
	sqlite3_bind_blob(pStmt, 1, db->auth.zAuthPW, db->auth.nAuthPW,
	    SQLITE_STATIC);
	sqlite3_bind_text(pStmt, 2, db->auth.zAuthUser, -1, SQLITE_STATIC);
	rc = sqlite3_step(pStmt);
	if (rc == SQLITE_ROW && sqlite3_column_int(pStmt,0))
		*peAuth = sqlite3_column_int(pStmt, 1) + UAUTH_User;
	else
		*peAuth = UAUTH_Fail;
	return sqlite3_finalize(pStmt);
}

int sqlite3UserAuthCheckLogin(
    sqlite3 *db,               /* The database connection to check */
    const char *zDb,           /* Name of specific database to check */
    u8 *peAuth)                /* OUT: One of UAUTH_* constants */
{
	int rc;
	u8 savedAuthLevel;
	assert(zDb != 0);
	assert(peAuth != 0);
	savedAuthLevel = db->auth.authLevel;
	db->auth.authLevel = UAUTH_Admin;
	rc = userAuthCheckLogin(db, zDb, peAuth);
	db->auth.authLevel = savedAuthLevel;
	return rc;
}

/*
** If the current authLevel is UAUTH_Unknown, then take actions to figure
** out what authLevel should be.
*/
void sqlite3UserAuthInit(sqlite3 *db)
{
	if (db->auth.authLevel == UAUTH_Unknown) {
		u8 authLevel = UAUTH_Fail;
		sqlite3UserAuthCheckLogin(db, "main", &authLevel);
		db->auth.authLevel = authLevel;
		if (authLevel < UAUTH_Admin) 
			db->flags &= ~SQLITE_WriteSchema;
	}
}

/*
** Implementation of the sqlite_crypt(X,Y) function.
**
** If Y is NULL then generate a new hash for password X and return that
** hash.  If Y is not null, then generate a hash for password X using the
** same salt as the previous hash Y and return the new hash.
**
** Note: SQLite uses a simple Ceasar-cypher to compute hash value of passwords.
** In Berkeley DB we replace this with a stronger password hash function. The 
** hashed password was stored as 40 bytes(20 bytes of salt + 20 bytes of HMAC). 
*/
void sqlite3CryptFunc(
    sqlite3_context *context,
    int NotUsed,
    sqlite3_value **argv)
{
	u_int8_t *zIn;
	int nIn, nOut;
	u8 *zOut;
	u_int8_t zSalt[AUTH_PW_SALT_LEN];
	zIn = (u_int8_t *)sqlite3_value_blob(argv[0]);
	nIn = sqlite3_value_bytes(argv[0]);
	nOut = AUTH_PW_HASH_LEN + sizeof(zSalt);
	if (sqlite3_value_type(argv[1]) == SQLITE_BLOB && 
	    sqlite3_value_bytes(argv[1])==nOut)
		memcpy(zSalt, sqlite3_value_blob(argv[1]), sizeof(zSalt));
	else
		sqlite3_randomness(sizeof(zSalt), zSalt);
	zOut = sqlite3_malloc(nOut);
	if (zOut == 0)
		sqlite3_result_error_nomem(context);
	else {
		memcpy(zOut, zSalt, sizeof(zSalt));
		__db_chksum(NULL, zIn, nIn, zSalt, zOut + sizeof(zSalt));
		sqlite3_result_blob(context, zOut, nOut, sqlite3_free);
	}
}

static void resetDbAuth(sqlite3 *db)
{
	sqlite3_free(db->auth.zAuthUser);
	sqlite3_free(db->auth.zAuthPW);
	memset(&db->auth, 0, sizeof(db->auth));
}

static int setDbAuth(sqlite3 *db, const char *zUsername, const char *zPW,
    int nPW, u8 authLevel)
{
	if ((db->auth.zAuthUser = sqlite3_mprintf("%s", zUsername)) == 0 ||
	    (db->auth.zAuthPW = sqlite3_malloc(nPW + 1)) == 0)
		return SQLITE_NOMEM;
	memcpy(db->auth.zAuthPW, zPW, nPW);
	db->auth.nAuthPW = nPW;
	db->auth.authLevel = authLevel;
	return SQLITE_OK;
}

/*
** If a database contains the SQLITE_USER table, then the
** sqlite3_user_authenticate() interface must be invoked with an
** appropriate username and password prior to enable read and write
** access to the database.
**
** Return SQLITE_OK on success or SQLITE_ERROR if the username/password
** combination is incorrect or unknown.
**
** If the SQLITE_USER table is not present in the database file, then
** this interface is a harmless no-op returnning SQLITE_OK.
*/
int sqlite3_user_authenticate(
    sqlite3 *db,           /* The database connection */
    const char *zUsername, /* Username */
    const char *zPW,       /* Password or credentials */
    int nPW)               /* Number of bytes in aPW[] */
{
	DB_ENV *dbenv;
	int rc, usrTblExists;
	u8 authLevel;

	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;
	usrTblExists = 0;
	authLevel = UAUTH_Fail;

	resetDbAuth(db);

#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if ((rc = auth_keystore_lock(db)) != SQLITE_OK)
		return rc;
	/* 
	 * If needed, fetch the encryption key from keystore and apply it to 
	 * the database.
	 */
	if (auth_userlogin_keystore(db, zUsername, zPW, nPW) != 0) {
		rc = SQLITE_AUTH;
		goto err;
	}
#endif
	/*
	 * If the database does not require authentication, it is an error to
	 * call this function. All databases that require authentication are
	 * encrypted. So if the encryption key is not available now, it is an
	 * error.
	 */
	if (dbenv == NULL || db->aDb[0].pBt->pBt->encrypt_pwd == NULL) {
		rc = SQLITE_AUTH;
		if (dbenv != NULL)
			dbenv->errx(dbenv,
		"Cannot do authentication to a non-encrypted database.");
		goto err;
	}

	db->auth.authLevel = UAUTH_Admin;
	if ((rc = userTableExists(db, "main", &usrTblExists)) != SQLITE_OK)
		goto err;

	if (!usrTblExists) {
		rc = SQLITE_AUTH;
		if (dbenv != NULL)
			dbenv->errx(dbenv,
	"Cannot do authentication to a non-authentication-required database.");
		goto err;
	}

	if ((rc = setDbAuth(db, zUsername, zPW, nPW, UAUTH_Unknown)) != 
	    SQLITE_OK)
		goto err;
	rc = sqlite3UserAuthCheckLogin(db, "main", &authLevel);
	db->auth.authLevel = authLevel;
	sqlite3ExpirePreparedStatements(db);

	if (!rc && authLevel < UAUTH_User)
		rc = SQLITE_AUTH;	/* Incorrect username and/or password */

err:
#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if (rc)
		resetDbAuth(db);
	auth_keystore_unlock(db);
#endif
	return rc;
}

#define AUTH_DB_EXEC(db, sql)						\
	sqlite3_exec(db, sql, 0, 0, 0)

/*
** The sqlite3_user_add() interface can be used (by an admin user only)
** to create a new user.  When called on a no-authentication-required
** database, this routine converts the database into an authentication-
** required database, automatically makes the added user an
** administrator, and logs in the current connection as that user.
** The sqlite3_user_add() interface only works for the "main" database, not
** for any ATTACH-ed databases.  Any call to sqlite3_user_add() by a
** non-admin user results in an error.
*/
int sqlite3_user_add(
    sqlite3 *db,           /* Database connection */
    const char *zUsername, /* Username to be added */
    const char *aPW,       /* Password or credentials */
    int nPW,               /* Number of bytes in aPW[] */
    int isAdmin)           /* True to give new user admin privilege */
{
	DB_ENV *dbenv;
	sqlite3_stmt *pStmt;
	int rc, usrTblExists;
#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	int ks_init;

	ks_init = 0;
#endif
	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;
	usrTblExists = 0;
	if (!db->autoCommit) {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv,
			    "Cannot add a user in a transaction.");
		return rc;
	}

#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if ((rc = auth_keystore_lock(db)) != SQLITE_OK)
		return rc;
	/* Make sure encryption becomes mandatory when userauth is enabled. */
	if ((rc = auth_useradd_key(db, isAdmin)) != SQLITE_OK)
		goto done_no_trans;
#endif
	/*
	 * We're adding a user, so either this database already requires
	 * authentication or we're going to make it require authentication.
	 * Authentication requires encryption, so the encryption key must
	 * be set by now before userTableExists() tries to access the
	 * database.
	 */
	if (dbenv == NULL || db->aDb[0].pBt->pBt->encrypt_pwd == NULL) {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv,
		"The database must be encrypted to enable authentication.");
		goto done_no_trans;
	}

	sqlite3UserAuthInit(db);
	if (db->auth.authLevel < UAUTH_Admin) {
		rc = SQLITE_AUTH;
		goto done_no_trans;
	}	

	if ((rc = AUTH_DB_EXEC(db, "begin")) != SQLITE_OK)
		goto done_no_trans;

	if ((rc = userTableExists(db, "main", &usrTblExists)) != SQLITE_OK)
		goto err;

	if (usrTblExists)
		goto adduser;

	/* No sqlite_user table. Do bootstrap. */
	if (!isAdmin) {
		rc = SQLITE_AUTH;
		goto err;
	}
	if ((pStmt = sqlite3UserAuthPrepare(db, 
	    "CREATE TABLE sqlite_user(uname TEXT PRIMARY KEY, "
	    "isAdmin BOOLEAN, pw BLOB) WITHOUT ROWID;")) == NULL) {
		rc = SQLITE_NOMEM;
		goto err;
	}
	sqlite3_step(pStmt);
	if ((rc = sqlite3_finalize(pStmt)) != SQLITE_OK)
		goto err;

#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	/* The userauth is enabled. Create the keystore file now. */
	ks_init = 1;
	if ((rc = auth_keystore_create(db)) != SQLITE_OK)
		goto err;
#endif

adduser:
	if ((pStmt = sqlite3UserAuthPrepare(db, 
	    "INSERT INTO sqlite_user(uname, isAdmin, pw) VALUES(%Q, %d, "
	    "sqlite_crypt(?1, NULL))", zUsername, isAdmin)) == NULL) {
		rc = SQLITE_NOMEM;
		goto err;
	}
	sqlite3_bind_blob(pStmt, 1, aPW, nPW, SQLITE_STATIC);
	sqlite3_step(pStmt);
	if ((rc = sqlite3_finalize(pStmt)) != SQLITE_OK)
		goto err;

#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if (sqlite3_changes(db) != 0 && (rc = auth_useradd_keystore(db, 
	    zUsername, aPW, nPW)) != SQLITE_OK)
		goto err;
#endif

	if (db->auth.zAuthUser == NULL) {
		assert(isAdmin != 0);
		if ((rc = setDbAuth(db, zUsername, aPW, nPW, UAUTH_Admin))
		    != SQLITE_OK)
			goto err;
	}
	goto done;

err:
#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if (ks_init)
		auth_keystore_remove(db);
#endif

done:
	AUTH_DB_EXEC(db, rc ? "rollback" : "commit");

done_no_trans:
#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if (rc == SQLITE_OK)
		auth_keystore_backup(db);
	auth_keystore_unlock(db);
#endif
	return rc;
}

/*
** The sqlite3_user_change() interface can be used to change a users
** login credentials or admin privilege.  Any user can change their own
** login credentials.  Only an admin user can change another users login
** credentials or admin privilege setting.  No user may change their own 
** admin privilege setting.
*/
int sqlite3_user_change(
    sqlite3 *db,           /* Database connection */
    const char *zUsername, /* Username to change */
    const char *aPW,       /* Modified password or credentials */
    int nPW,               /* Number of bytes in aPW[] */
    int isAdmin)           /* Modified admin privilege for the user */
{
	DB_ENV *dbenv;
	sqlite3_stmt *pStmt;
	int rc, usrTblExists;
	u8 authLevel;

	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;
	usrTblExists = 0;
	authLevel = db->auth.authLevel;

	if (!db->autoCommit) {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv, 
			    "Cannot change a user in a transaction.");
		return rc;
	}

	if (authLevel < UAUTH_User || db->auth.zAuthUser == NULL)
		/* Must be logged in to make a change */
		return SQLITE_AUTH;
	if (strcmp(db->auth.zAuthUser, zUsername) != 0) {
		if (db->auth.authLevel < UAUTH_Admin)
			/* Must be administrator to change a different user */
			return SQLITE_AUTH;
	} else if (isAdmin != (authLevel == UAUTH_Admin)) {
		/* Cannot change the isAdmin setting for self */
		return SQLITE_AUTH;
	}

#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if ((rc = auth_keystore_lock(db)) != SQLITE_OK)
		return rc;
#endif

	if ((rc = AUTH_DB_EXEC(db, "begin")) != SQLITE_OK)
		goto done_no_trans;

	db->auth.authLevel = UAUTH_Admin;

	if ((rc = userTableExists(db, "main", &usrTblExists)) != SQLITE_OK)
		goto err;

	/* This routine is a no-op if the user to be modified does not exist. */
	if (!usrTblExists)
		goto done;

	if ((pStmt = sqlite3UserAuthPrepare(db, "UPDATE sqlite_user SET "
	    "isAdmin=%d, pw=sqlite_crypt(?1,NULL) WHERE uname=%Q", isAdmin, 
	    zUsername)) == NULL) {
		rc = SQLITE_NOMEM;
		goto err;
	}
	sqlite3_bind_blob(pStmt, 1, aPW, nPW, SQLITE_STATIC);
	sqlite3_step(pStmt);
	rc = sqlite3_finalize(pStmt);

#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if (!rc && sqlite3_changes(db) != 0)
		rc = auth_useredit_keystore(db, zUsername, aPW, nPW);
#endif

err:
done:
	AUTH_DB_EXEC(db, rc ? "rollback" : "commit");

done_no_trans:
	db->auth.authLevel = authLevel;
#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if (rc == SQLITE_OK)
		auth_keystore_backup(db);
	auth_keystore_unlock(db);
#endif
	return rc;
}

/*
** The sqlite3_user_delete() interface can be used (by an admin user only)
** to delete a user.  The currently logged-in user cannot be deleted,
** which guarantees that there is always an admin user and hence that
** the database cannot be converted into a no-authentication-required
** database.
*/
int sqlite3_user_delete(
    sqlite3 *db,           /* Database connection */
    const char *zUsername) /* Username to remove */
{
	DB_ENV *dbenv;
	int rc, usrTblExists;
	sqlite3_stmt *pStmt;

	dbenv = authGetDbEnv(db);
	rc = SQLITE_OK;
	usrTblExists = 0;
	if (!db->autoCommit) {
		rc = SQLITE_ERROR;
		if (dbenv != NULL)
			dbenv->errx(dbenv, 
			    "Cannot delete a user in a transaction.");
		return rc;
	}

	if (db->auth.authLevel < UAUTH_Admin || db->auth.zAuthUser == NULL)
		/* Must be an administrator to delete a user */
		return SQLITE_AUTH;
	if (strcmp(db->auth.zAuthUser, zUsername) == 0)
		/* Cannot delete self */
		return SQLITE_AUTH;
	if ((rc = userTableExists(db, "main", &usrTblExists)) != SQLITE_OK)
		return rc;

	/* This routine is a no-op if the user to be deleted does not exist. */
	if (!usrTblExists)
		return SQLITE_OK;

#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if ((rc = auth_keystore_lock(db)) != SQLITE_OK)
		return rc;
#endif

	if ((rc = AUTH_DB_EXEC(db, "begin")) != SQLITE_OK)
		goto done_no_trans;

	if ((pStmt = sqlite3UserAuthPrepare(db, "DELETE FROM sqlite_user "
	    "WHERE uname=%Q", zUsername)) == NULL) {
		rc = SQLITE_NOMEM;
		goto err;
	}
	sqlite3_step(pStmt);
	rc = sqlite3_finalize(pStmt);

#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if (!rc && sqlite3_changes(db) != 0)
		rc = auth_userdelete_keystore(db, zUsername);
#endif

err:
	AUTH_DB_EXEC(db, rc ? "rollback" : "commit");

done_no_trans:
#ifdef BDBSQL_USER_AUTHENTICATION_KEYSTORE
	if (rc == SQLITE_OK)
		auth_keystore_backup(db);
	auth_keystore_unlock(db);
#endif
	return rc;
}

#endif /* SQLITE_USER_AUTHENTICATION */
