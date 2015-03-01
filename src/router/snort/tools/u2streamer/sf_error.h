
#ifndef __SF_ERROR_H__
#define __SF_ERROR_H__
/*! \defgroup SF_ERROR
 */
/** \addtogroup */
/*@{*/
#define SF_SUCCESS        0   /* success */
#define SF_EINVAL         1   /* Invalid argument */
#define SF_ENOSYS         2   /* Unimplemented */
#define SF_ENOMEM         3   /* Out of memory */
#define SF_ERANGE         4   /* Out of range */
#define SF_EPERM          5   /* Not allowed */
#define SF_ENOENT         6   /* No entry */
#define SF_EEXIST         7   /* Already exists */
#define SF_EDATABASE      8   /* Generic database error */
#define SF_ESYNTAX        9   /* Syntax error */
#define SF_NOUSER         10  /* required User value missing */
#define SF_NOUSERROLE     11  /* required User Role value missing */
#define SF_NOTIMESPENT    12  /* required TimeSpent value missing */
#define SF_NOCOMMENT      13  /* required Comment value missing */
#define SF_ETRANSACT      14  /* we need transaction support in the DB */
#define SF_NOTYPE         15  /* required Type value missing */
#define SF_NOSTATE        16  /* required State value missing */
#define SF_NOSUMMARY      17  /* required Summary value missing */
#define SF_EBUSY          18  /* Resource busy */
#define SF_ENOSPC         19  /* No space */
#define SF_EREAD          20  /* General read error */
#define SF_END_OF_FILE    21  /* End of file */
#define SF_EAGAIN         22  /* Try again */
#define SF_EREAD_PARTIAL  23    /* Partial read */
#define SF_ENOTCONN       24    /* Not connected */
#define SF_EREAD_TRUNCATED  25  /* Truncated read */
#define SF_CLOSED           26  /* Closed */
#define SF_ENOPROTOSUPPORT  27  /* Protocol not supported */
#define SF_ENOSUPPORT       28  /* Not supported */
#define SF_EWRITE           29  /* Write error */
#define SF_EWRITE_PARTIAL   30  /* Write error */
#define SF_EBADLEN          31  /* Bad length */
#define SF_EPROTOCOL_VIOLATION  32  /* Protocol violation */
#define SF_EPEER                33  /* peer error */
#define SF_ENOTDIR          34  /* not a directory */
#define SF_EMUTEX           35
#define SF_EMUTEX_INVAL     36  /* invalid lock */
#define SF_EMUTEX_DEADLK    37  /* operation would cause deadlock */
#define SF_EOPEN            38  /* open failed */
#define SF_ELOCKED          39  /* resource locked */
#define SF_ESSL             40  /* SSL error */
#define SF_ELICENSE_INVAL   41  /* Invalid license */
#define SF_ELICENSE_PLATFORM 42 /* Not a valid license for this platform */
#define SF_ELICENSE_CORRUPT 43  /* Corrupt license */
#define SF_ESSL_NOCIPHERS   44  /* No valid ciphers */
#define SF_ESSL_CRLEXPIRED  45  /* CRL expired */
#define SF_ENOMATCH         46  /* does not match */
#define SF_ESOCKET          47  /* Socket error */
#define SF_ENITRO           48  /* Error from Nitro database */
#define SF_ENOLICENSE       49  /* No license */
#define SF_EHASLICENSE      50  /* Already has a license */
#define SF_ECORRUPT         51
#define SF_EBAD_MAGIC       52
#define SF_EBAD_LINKTYPE    53
#define SF_ECONT            54  /* Continue */
#define SF_EINVHOST         55  /* invalid hostname entered */
#define SF_EUSER_LIMIT_REACHED  56  /* Couldn't create user - license limit reached*/
#define SF_EDELETE          57  /* Error in deleting file or entry in memory */
#define SF_EMEM             58  /* Error in manipulating memory */
#define SF_NITRO_DUPLICATE  114 /* duplicate key */

/**
 * Retrieve the text description of the specified error number.
 *
 * @param errnum    the error number
 *
 * @returns descriptive error string
 */
const char *sf_strerror(int errnum);

/*@}*/
#endif /* __SF_ERROR_H__ */
