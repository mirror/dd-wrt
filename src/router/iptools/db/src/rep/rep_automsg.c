/* Do not edit: automatically built by gen_msg.awk. */

#include "db_config.h"

#include "db_int.h"
#include "dbinc/db_page.h"
#include "dbinc/db_am.h"
#include "dbinc/mp.h"
#include "dbinc/txn.h"

/*
 * PUBLIC: int __rep_bulk_marshal __P((ENV *, __rep_bulk_args *,
 * PUBLIC:	 u_int8_t *, size_t, size_t *));
 */
int
__rep_bulk_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_bulk_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_BULK_SIZE
	    + (size_t)argp->bulkdata.size)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->len);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.offset);
	DB_HTONL_COPYOUT(env, bp, argp->bulkdata.size);
	if (argp->bulkdata.size > 0) {
		memcpy(bp, argp->bulkdata.data, argp->bulkdata.size);
		bp += argp->bulkdata.size;
	}

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_bulk_unmarshal __P((ENV *, __rep_bulk_args *,
 * PUBLIC:	 u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_bulk_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_bulk_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	size_t needed;

	needed = __REP_BULK_SIZE;
	if (max < needed)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->len, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.offset, bp);
	DB_NTOHL_COPYIN(env, argp->bulkdata.size, bp);
	if (argp->bulkdata.size == 0)
		argp->bulkdata.data = NULL;
	else
		argp->bulkdata.data = bp;
	needed += (size_t)argp->bulkdata.size;
	if (max < needed)
		goto too_few;
	bp += argp->bulkdata.size;

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_bulk message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_control_marshal __P((ENV *, __rep_control_args *,
 * PUBLIC:	 u_int8_t *, size_t, size_t *));
 */
int
__rep_control_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_control_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_CONTROL_SIZE)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->rep_version);
	DB_HTONL_COPYOUT(env, bp, argp->log_version);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.offset);
	DB_HTONL_COPYOUT(env, bp, argp->rectype);
	DB_HTONL_COPYOUT(env, bp, argp->gen);
	DB_HTONL_COPYOUT(env, bp, argp->msg_sec);
	DB_HTONL_COPYOUT(env, bp, argp->msg_nsec);
	DB_HTONL_COPYOUT(env, bp, argp->flags);

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_control_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_control_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_control_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_control_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_CONTROL_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->rep_version, bp);
	DB_NTOHL_COPYIN(env, argp->log_version, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.offset, bp);
	DB_NTOHL_COPYIN(env, argp->rectype, bp);
	DB_NTOHL_COPYIN(env, argp->gen, bp);
	DB_NTOHL_COPYIN(env, argp->msg_sec, bp);
	DB_NTOHL_COPYIN(env, argp->msg_nsec, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_control message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_egen_marshal __P((ENV *, __rep_egen_args *,
 * PUBLIC:	 u_int8_t *, size_t, size_t *));
 */
int
__rep_egen_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_egen_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_EGEN_SIZE)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->egen);

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_egen_unmarshal __P((ENV *, __rep_egen_args *,
 * PUBLIC:	 u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_egen_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_egen_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_EGEN_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->egen, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_egen message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_fileinfo_marshal __P((ENV *,
 * PUBLIC:	 __rep_fileinfo_args *, u_int8_t *, size_t, size_t *));
 */
int
__rep_fileinfo_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_fileinfo_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_FILEINFO_SIZE
	    + (size_t)argp->uid.size
	    + (size_t)argp->info.size
	    + (size_t)argp->dir.size)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->pgsize);
	DB_HTONL_COPYOUT(env, bp, argp->pgno);
	DB_HTONL_COPYOUT(env, bp, argp->max_pgno);
	DB_HTONL_COPYOUT(env, bp, argp->filenum);
	DB_HTONL_COPYOUT(env, bp, argp->finfo_flags);
	DB_HTONL_COPYOUT(env, bp, argp->type);
	DB_HTONL_COPYOUT(env, bp, argp->db_flags);
	DB_HTONL_COPYOUT(env, bp, argp->uid.size);
	if (argp->uid.size > 0) {
		memcpy(bp, argp->uid.data, argp->uid.size);
		bp += argp->uid.size;
	}
	DB_HTONL_COPYOUT(env, bp, argp->info.size);
	if (argp->info.size > 0) {
		memcpy(bp, argp->info.data, argp->info.size);
		bp += argp->info.size;
	}
	DB_HTONL_COPYOUT(env, bp, argp->dir.size);
	if (argp->dir.size > 0) {
		memcpy(bp, argp->dir.data, argp->dir.size);
		bp += argp->dir.size;
	}
	DB_HTONL_COPYOUT(env, bp, argp->blob_fid_lo);
	DB_HTONL_COPYOUT(env, bp, argp->blob_fid_hi);

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_fileinfo_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_fileinfo_args **, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_fileinfo_unmarshal(env, argpp, bp, max, nextp)
	ENV *env;
	__rep_fileinfo_args **argpp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	size_t needed;
	__rep_fileinfo_args *argp;
	int ret;

	needed = __REP_FILEINFO_SIZE;
	if (max < needed)
		goto too_few;
	if ((ret = __os_malloc(env, sizeof(*argp), &argp)) != 0)
		return (ret);

	DB_NTOHL_COPYIN(env, argp->pgsize, bp);
	DB_NTOHL_COPYIN(env, argp->pgno, bp);
	DB_NTOHL_COPYIN(env, argp->max_pgno, bp);
	DB_NTOHL_COPYIN(env, argp->filenum, bp);
	DB_NTOHL_COPYIN(env, argp->finfo_flags, bp);
	DB_NTOHL_COPYIN(env, argp->type, bp);
	DB_NTOHL_COPYIN(env, argp->db_flags, bp);
	DB_NTOHL_COPYIN(env, argp->uid.size, bp);
	if (argp->uid.size == 0)
		argp->uid.data = NULL;
	else
		argp->uid.data = bp;
	needed += (size_t)argp->uid.size;
	if (max < needed)
		goto too_few;
	bp += argp->uid.size;
	DB_NTOHL_COPYIN(env, argp->info.size, bp);
	if (argp->info.size == 0)
		argp->info.data = NULL;
	else
		argp->info.data = bp;
	needed += (size_t)argp->info.size;
	if (max < needed)
		goto too_few;
	bp += argp->info.size;
	DB_NTOHL_COPYIN(env, argp->dir.size, bp);
	if (argp->dir.size == 0)
		argp->dir.data = NULL;
	else
		argp->dir.data = bp;
	needed += (size_t)argp->dir.size;
	if (max < needed)
		goto too_few;
	bp += argp->dir.size;
	DB_NTOHL_COPYIN(env, argp->blob_fid_lo, bp);
	DB_NTOHL_COPYIN(env, argp->blob_fid_hi, bp);

	if (nextp != NULL)
		*nextp = bp;
	*argpp = argp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_fileinfo message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_fileinfo_v7_marshal __P((ENV *,
 * PUBLIC:	 __rep_fileinfo_v7_args *, u_int8_t *, size_t, size_t *));
 */
int
__rep_fileinfo_v7_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_fileinfo_v7_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_FILEINFO_V7_SIZE
	    + (size_t)argp->uid.size
	    + (size_t)argp->info.size
	    + (size_t)argp->dir.size)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->pgsize);
	DB_HTONL_COPYOUT(env, bp, argp->pgno);
	DB_HTONL_COPYOUT(env, bp, argp->max_pgno);
	DB_HTONL_COPYOUT(env, bp, argp->filenum);
	DB_HTONL_COPYOUT(env, bp, argp->finfo_flags);
	DB_HTONL_COPYOUT(env, bp, argp->type);
	DB_HTONL_COPYOUT(env, bp, argp->db_flags);
	DB_HTONL_COPYOUT(env, bp, argp->uid.size);
	if (argp->uid.size > 0) {
		memcpy(bp, argp->uid.data, argp->uid.size);
		bp += argp->uid.size;
	}
	DB_HTONL_COPYOUT(env, bp, argp->info.size);
	if (argp->info.size > 0) {
		memcpy(bp, argp->info.data, argp->info.size);
		bp += argp->info.size;
	}
	DB_HTONL_COPYOUT(env, bp, argp->dir.size);
	if (argp->dir.size > 0) {
		memcpy(bp, argp->dir.data, argp->dir.size);
		bp += argp->dir.size;
	}

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_fileinfo_v7_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_fileinfo_v7_args **, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_fileinfo_v7_unmarshal(env, argpp, bp, max, nextp)
	ENV *env;
	__rep_fileinfo_v7_args **argpp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	size_t needed;
	__rep_fileinfo_v7_args *argp;
	int ret;

	needed = __REP_FILEINFO_V7_SIZE;
	if (max < needed)
		goto too_few;
	if ((ret = __os_malloc(env, sizeof(*argp), &argp)) != 0)
		return (ret);

	DB_NTOHL_COPYIN(env, argp->pgsize, bp);
	DB_NTOHL_COPYIN(env, argp->pgno, bp);
	DB_NTOHL_COPYIN(env, argp->max_pgno, bp);
	DB_NTOHL_COPYIN(env, argp->filenum, bp);
	DB_NTOHL_COPYIN(env, argp->finfo_flags, bp);
	DB_NTOHL_COPYIN(env, argp->type, bp);
	DB_NTOHL_COPYIN(env, argp->db_flags, bp);
	DB_NTOHL_COPYIN(env, argp->uid.size, bp);
	if (argp->uid.size == 0)
		argp->uid.data = NULL;
	else
		argp->uid.data = bp;
	needed += (size_t)argp->uid.size;
	if (max < needed)
		goto too_few;
	bp += argp->uid.size;
	DB_NTOHL_COPYIN(env, argp->info.size, bp);
	if (argp->info.size == 0)
		argp->info.data = NULL;
	else
		argp->info.data = bp;
	needed += (size_t)argp->info.size;
	if (max < needed)
		goto too_few;
	bp += argp->info.size;
	DB_NTOHL_COPYIN(env, argp->dir.size, bp);
	if (argp->dir.size == 0)
		argp->dir.data = NULL;
	else
		argp->dir.data = bp;
	needed += (size_t)argp->dir.size;
	if (max < needed)
		goto too_few;
	bp += argp->dir.size;

	if (nextp != NULL)
		*nextp = bp;
	*argpp = argp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_fileinfo_v7 message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_fileinfo_v6_marshal __P((ENV *,
 * PUBLIC:	 __rep_fileinfo_v6_args *, u_int8_t *, size_t, size_t *));
 */
int
__rep_fileinfo_v6_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_fileinfo_v6_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_FILEINFO_V6_SIZE
	    + (size_t)argp->uid.size
	    + (size_t)argp->info.size)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->pgsize);
	DB_HTONL_COPYOUT(env, bp, argp->pgno);
	DB_HTONL_COPYOUT(env, bp, argp->max_pgno);
	DB_HTONL_COPYOUT(env, bp, argp->filenum);
	DB_HTONL_COPYOUT(env, bp, argp->finfo_flags);
	DB_HTONL_COPYOUT(env, bp, argp->type);
	DB_HTONL_COPYOUT(env, bp, argp->db_flags);
	DB_HTONL_COPYOUT(env, bp, argp->uid.size);
	if (argp->uid.size > 0) {
		memcpy(bp, argp->uid.data, argp->uid.size);
		bp += argp->uid.size;
	}
	DB_HTONL_COPYOUT(env, bp, argp->info.size);
	if (argp->info.size > 0) {
		memcpy(bp, argp->info.data, argp->info.size);
		bp += argp->info.size;
	}

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_fileinfo_v6_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_fileinfo_v6_args **, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_fileinfo_v6_unmarshal(env, argpp, bp, max, nextp)
	ENV *env;
	__rep_fileinfo_v6_args **argpp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	size_t needed;
	__rep_fileinfo_v6_args *argp;
	int ret;

	needed = __REP_FILEINFO_V6_SIZE;
	if (max < needed)
		goto too_few;
	if ((ret = __os_malloc(env, sizeof(*argp), &argp)) != 0)
		return (ret);

	DB_NTOHL_COPYIN(env, argp->pgsize, bp);
	DB_NTOHL_COPYIN(env, argp->pgno, bp);
	DB_NTOHL_COPYIN(env, argp->max_pgno, bp);
	DB_NTOHL_COPYIN(env, argp->filenum, bp);
	DB_NTOHL_COPYIN(env, argp->finfo_flags, bp);
	DB_NTOHL_COPYIN(env, argp->type, bp);
	DB_NTOHL_COPYIN(env, argp->db_flags, bp);
	DB_NTOHL_COPYIN(env, argp->uid.size, bp);
	if (argp->uid.size == 0)
		argp->uid.data = NULL;
	else
		argp->uid.data = bp;
	needed += (size_t)argp->uid.size;
	if (max < needed)
		goto too_few;
	bp += argp->uid.size;
	DB_NTOHL_COPYIN(env, argp->info.size, bp);
	if (argp->info.size == 0)
		argp->info.data = NULL;
	else
		argp->info.data = bp;
	needed += (size_t)argp->info.size;
	if (max < needed)
		goto too_few;
	bp += argp->info.size;

	if (nextp != NULL)
		*nextp = bp;
	*argpp = argp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_fileinfo_v6 message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_grant_info_marshal __P((ENV *,
 * PUBLIC:	 __rep_grant_info_args *, u_int8_t *, size_t, size_t *));
 */
int
__rep_grant_info_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_grant_info_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_GRANT_INFO_SIZE)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->msg_sec);
	DB_HTONL_COPYOUT(env, bp, argp->msg_nsec);

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_grant_info_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_grant_info_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_grant_info_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_grant_info_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_GRANT_INFO_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->msg_sec, bp);
	DB_NTOHL_COPYIN(env, argp->msg_nsec, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_grant_info message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_logreq_marshal __P((ENV *, __rep_logreq_args *,
 * PUBLIC:	 u_int8_t *, size_t, size_t *));
 */
int
__rep_logreq_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_logreq_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_LOGREQ_SIZE)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->endlsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->endlsn.offset);

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_logreq_unmarshal __P((ENV *, __rep_logreq_args *,
 * PUBLIC:	 u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_logreq_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_logreq_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_LOGREQ_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->endlsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->endlsn.offset, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_logreq message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_newfile_marshal __P((ENV *, __rep_newfile_args *,
 * PUBLIC:	 u_int8_t *, size_t, size_t *));
 */
int
__rep_newfile_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_newfile_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_NEWFILE_SIZE)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->version);

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_newfile_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_newfile_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_newfile_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_newfile_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_NEWFILE_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->version, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_newfile message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_update_marshal __P((ENV *, __rep_update_args *,
 * PUBLIC:	 u_int8_t *, size_t, size_t *));
 */
int
__rep_update_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_update_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_UPDATE_SIZE)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->first_lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->first_lsn.offset);
	DB_HTONL_COPYOUT(env, bp, argp->first_vers);
	DB_HTONL_COPYOUT(env, bp, argp->num_files);

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_update_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_update_args **, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_update_unmarshal(env, argpp, bp, max, nextp)
	ENV *env;
	__rep_update_args **argpp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	__rep_update_args *argp;
	int ret;

	if (max < __REP_UPDATE_SIZE)
		goto too_few;
	if ((ret = __os_malloc(env, sizeof(*argp), &argp)) != 0)
		return (ret);

	DB_NTOHL_COPYIN(env, argp->first_lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->first_lsn.offset, bp);
	DB_NTOHL_COPYIN(env, argp->first_vers, bp);
	DB_NTOHL_COPYIN(env, argp->num_files, bp);

	if (nextp != NULL)
		*nextp = bp;
	*argpp = argp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_update message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_vote_info_marshal __P((ENV *,
 * PUBLIC:	 __rep_vote_info_args *, u_int8_t *, size_t, size_t *));
 */
int
__rep_vote_info_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_vote_info_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_VOTE_INFO_SIZE)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->egen);
	DB_HTONL_COPYOUT(env, bp, argp->nsites);
	DB_HTONL_COPYOUT(env, bp, argp->nvotes);
	DB_HTONL_COPYOUT(env, bp, argp->priority);
	DB_HTONL_COPYOUT(env, bp, argp->spare_pri);
	DB_HTONL_COPYOUT(env, bp, argp->tiebreaker);
	DB_HTONL_COPYOUT(env, bp, argp->data_gen);

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_vote_info_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_vote_info_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_vote_info_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_vote_info_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_VOTE_INFO_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->egen, bp);
	DB_NTOHL_COPYIN(env, argp->nsites, bp);
	DB_NTOHL_COPYIN(env, argp->nvotes, bp);
	DB_NTOHL_COPYIN(env, argp->priority, bp);
	DB_NTOHL_COPYIN(env, argp->spare_pri, bp);
	DB_NTOHL_COPYIN(env, argp->tiebreaker, bp);
	DB_NTOHL_COPYIN(env, argp->data_gen, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_vote_info message"));
	return (EINVAL);
}

/*
 * PUBLIC: int __rep_vote_info_v5_marshal __P((ENV *,
 * PUBLIC:	 __rep_vote_info_v5_args *, u_int8_t *, size_t, size_t *));
 */
int
__rep_vote_info_v5_marshal(env, argp, bp, max, lenp)
	ENV *env;
	__rep_vote_info_v5_args *argp;
	u_int8_t *bp;
	size_t *lenp, max;
{
	u_int8_t *start;

	if (max < __REP_VOTE_INFO_V5_SIZE)
		return (ENOMEM);
	start = bp;

	DB_HTONL_COPYOUT(env, bp, argp->egen);
	DB_HTONL_COPYOUT(env, bp, argp->nsites);
	DB_HTONL_COPYOUT(env, bp, argp->nvotes);
	DB_HTONL_COPYOUT(env, bp, argp->priority);
	DB_HTONL_COPYOUT(env, bp, argp->tiebreaker);

	*lenp = (size_t)(bp - start);
	return (0);
}

/*
 * PUBLIC: int __rep_vote_info_v5_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_vote_info_v5_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_vote_info_v5_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_vote_info_v5_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_VOTE_INFO_V5_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->egen, bp);
	DB_NTOHL_COPYIN(env, argp->nsites, bp);
	DB_NTOHL_COPYIN(env, argp->nvotes, bp);
	DB_NTOHL_COPYIN(env, argp->priority, bp);
	DB_NTOHL_COPYIN(env, argp->tiebreaker, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_vote_info_v5 message"));
	return (EINVAL);
}

/*
 * PUBLIC: void __rep_lsn_hist_key_marshal __P((ENV *,
 * PUBLIC:	 __rep_lsn_hist_key_args *, u_int8_t *));
 */
void
__rep_lsn_hist_key_marshal(env, argp, bp)
	ENV *env;
	__rep_lsn_hist_key_args *argp;
	u_int8_t *bp;
{
	DB_HTONL_COPYOUT(env, bp, argp->version);
	DB_HTONL_COPYOUT(env, bp, argp->gen);
}

/*
 * PUBLIC: int __rep_lsn_hist_key_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_lsn_hist_key_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_lsn_hist_key_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_lsn_hist_key_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_LSN_HIST_KEY_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->version, bp);
	DB_NTOHL_COPYIN(env, argp->gen, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_lsn_hist_key message"));
	return (EINVAL);
}

/*
 * PUBLIC: void __rep_lsn_hist_data_marshal __P((ENV *,
 * PUBLIC:	 __rep_lsn_hist_data_args *, u_int8_t *));
 */
void
__rep_lsn_hist_data_marshal(env, argp, bp)
	ENV *env;
	__rep_lsn_hist_data_args *argp;
	u_int8_t *bp;
{
	DB_HTONL_COPYOUT(env, bp, argp->envid);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.offset);
	DB_HTONL_COPYOUT(env, bp, argp->hist_sec);
	DB_HTONL_COPYOUT(env, bp, argp->hist_nsec);
}

/*
 * PUBLIC: int __rep_lsn_hist_data_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_lsn_hist_data_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_lsn_hist_data_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_lsn_hist_data_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_LSN_HIST_DATA_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->envid, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.offset, bp);
	DB_NTOHL_COPYIN(env, argp->hist_sec, bp);
	DB_NTOHL_COPYIN(env, argp->hist_nsec, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_lsn_hist_data message"));
	return (EINVAL);
}

/*
 * PUBLIC: void __rep_blob_update_req_marshal __P((ENV *,
 * PUBLIC:	 __rep_blob_update_req_args *, u_int8_t *));
 */
void
__rep_blob_update_req_marshal(env, argp, bp)
	ENV *env;
	__rep_blob_update_req_args *argp;
	u_int8_t *bp;
{
	DB_HTONLL_COPYOUT(env, bp, argp->blob_fid);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_sid);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_id);
	DB_HTONLL_COPYOUT(env, bp, argp->highest_id);
	DB_HTONL_COPYOUT(env, bp, argp->flags);
}

/*
 * PUBLIC: int __rep_blob_update_req_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_blob_update_req_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_blob_update_req_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_blob_update_req_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_BLOB_UPDATE_REQ_SIZE)
		goto too_few;
	DB_NTOHLL_COPYIN(env, argp->blob_fid, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_sid, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_id, bp);
	DB_NTOHLL_COPYIN(env, argp->highest_id, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_blob_update_req message"));
	return (EINVAL);
}

/*
 * PUBLIC: void __rep_blob_update_req_v8_marshal __P((ENV *,
 * PUBLIC:	 __rep_blob_update_req_v8_args *, u_int8_t *));
 */
void
__rep_blob_update_req_v8_marshal(env, argp, bp)
	ENV *env;
	__rep_blob_update_req_v8_args *argp;
	u_int8_t *bp;
{
	DB_HTONLL_COPYOUT(env, bp, argp->blob_fid);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_sid);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_id);
	DB_HTONLL_COPYOUT(env, bp, argp->highest_id);
}

/*
 * PUBLIC: int __rep_blob_update_req_v8_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_blob_update_req_v8_args *, u_int8_t *, size_t,
 * PUBLIC:	 u_int8_t **));
 */
int
__rep_blob_update_req_v8_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_blob_update_req_v8_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_BLOB_UPDATE_REQ_V8_SIZE)
		goto too_few;
	DB_NTOHLL_COPYIN(env, argp->blob_fid, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_sid, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_id, bp);
	DB_NTOHLL_COPYIN(env, argp->highest_id, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_blob_update_req_v8 message"));
	return (EINVAL);
}

/*
 * PUBLIC: void __rep_blob_update_marshal __P((ENV *,
 * PUBLIC:	 __rep_blob_update_args *, u_int8_t *));
 */
void
__rep_blob_update_marshal(env, argp, bp)
	ENV *env;
	__rep_blob_update_args *argp;
	u_int8_t *bp;
{
	DB_HTONLL_COPYOUT(env, bp, argp->blob_fid);
	DB_HTONLL_COPYOUT(env, bp, argp->highest_id);
	DB_HTONL_COPYOUT(env, bp, argp->flags);
	DB_HTONL_COPYOUT(env, bp, argp->num_blobs);
}

/*
 * PUBLIC: int __rep_blob_update_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_blob_update_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_blob_update_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_blob_update_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_BLOB_UPDATE_SIZE)
		goto too_few;
	DB_NTOHLL_COPYIN(env, argp->blob_fid, bp);
	DB_NTOHLL_COPYIN(env, argp->highest_id, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	DB_NTOHL_COPYIN(env, argp->num_blobs, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_blob_update message"));
	return (EINVAL);
}

/*
 * PUBLIC: void __rep_blob_file_marshal __P((ENV *,
 * PUBLIC:	 __rep_blob_file_args *, u_int8_t *));
 */
void
__rep_blob_file_marshal(env, argp, bp)
	ENV *env;
	__rep_blob_file_args *argp;
	u_int8_t *bp;
{
	DB_HTONLL_COPYOUT(env, bp, argp->blob_sid);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_id);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_size);
}

/*
 * PUBLIC: int __rep_blob_file_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_blob_file_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_blob_file_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_blob_file_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_BLOB_FILE_SIZE)
		goto too_few;
	DB_NTOHLL_COPYIN(env, argp->blob_sid, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_id, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_size, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_blob_file message"));
	return (EINVAL);
}

/*
 * PUBLIC: void __rep_blob_chunk_marshal __P((ENV *,
 * PUBLIC:	 __rep_blob_chunk_args *, u_int8_t *));
 */
void
__rep_blob_chunk_marshal(env, argp, bp)
	ENV *env;
	__rep_blob_chunk_args *argp;
	u_int8_t *bp;
{
	DB_HTONL_COPYOUT(env, bp, argp->flags);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_fid);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_sid);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_id);
	DB_HTONLL_COPYOUT(env, bp, argp->offset);
	DB_HTONL_COPYOUT(env, bp, argp->data.size);
	if (argp->data.size > 0) {
		memcpy(bp, argp->data.data, argp->data.size);
		bp += argp->data.size;
	}
}

/*
 * PUBLIC: int __rep_blob_chunk_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_blob_chunk_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_blob_chunk_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_blob_chunk_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	size_t needed;

	needed = __REP_BLOB_CHUNK_SIZE;
	if (max < needed)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_fid, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_sid, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_id, bp);
	DB_NTOHLL_COPYIN(env, argp->offset, bp);
	DB_NTOHL_COPYIN(env, argp->data.size, bp);
	if (argp->data.size == 0)
		argp->data.data = NULL;
	else
		argp->data.data = bp;
	needed += (size_t)argp->data.size;
	if (max < needed)
		goto too_few;
	bp += argp->data.size;

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_blob_chunk message"));
	return (EINVAL);
}

/*
 * PUBLIC: void __rep_blob_chunk_req_marshal __P((ENV *,
 * PUBLIC:	 __rep_blob_chunk_req_args *, u_int8_t *));
 */
void
__rep_blob_chunk_req_marshal(env, argp, bp)
	ENV *env;
	__rep_blob_chunk_req_args *argp;
	u_int8_t *bp;
{
	DB_HTONLL_COPYOUT(env, bp, argp->blob_fid);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_sid);
	DB_HTONLL_COPYOUT(env, bp, argp->blob_id);
	DB_HTONLL_COPYOUT(env, bp, argp->offset);
}

/*
 * PUBLIC: int __rep_blob_chunk_req_unmarshal __P((ENV *,
 * PUBLIC:	 __rep_blob_chunk_req_args *, u_int8_t *, size_t, u_int8_t **));
 */
int
__rep_blob_chunk_req_unmarshal(env, argp, bp, max, nextp)
	ENV *env;
	__rep_blob_chunk_req_args *argp;
	u_int8_t *bp;
	size_t max;
	u_int8_t **nextp;
{
	if (max < __REP_BLOB_CHUNK_REQ_SIZE)
		goto too_few;
	DB_NTOHLL_COPYIN(env, argp->blob_fid, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_sid, bp);
	DB_NTOHLL_COPYIN(env, argp->blob_id, bp);
	DB_NTOHLL_COPYIN(env, argp->offset, bp);

	if (nextp != NULL)
		*nextp = bp;
	return (0);

too_few:
	__db_errx(env, DB_STR("3675",
	    "Not enough input bytes to fill a __rep_blob_chunk_req message"));
	return (EINVAL);
}

