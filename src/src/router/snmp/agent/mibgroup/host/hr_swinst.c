/*
 *  Host Resources MIB - Installed Software group implementation - hr_swinst.c
 *
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/stat.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_DIRENT_H
#include <dirent.h>
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
#ifdef HAVE_PKGLOCS_H
#include <pkglocs.h>
#endif
#ifdef HAVE_PKGINFO_H
#include <pkginfo.h>
#endif

#ifdef HAVE_LIBRPM
#include <rpm/rpmlib.h>
#include <rpm/header.h>
#include <fcntl.h>
#endif

#ifdef HAVE_RPMGETPATH
#include <rpm/rpmmacro.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include "host_res.h"
#include "hr_swinst.h"
#include <net-snmp/utilities.h>

#define HRSWINST_MONOTONICALLY_INCREASING

        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

/*
 * Reorganize the global data into a single static structure.
 *
 *      Old                     New
 *======================================================
 *      HRSW_directory          swi->swi_directory
 *      HRSW_name[100]          swi->swi_name[SNMP_MAXPATH]
 *      HRSW_index              swi->swi_index
 *
 *                              swi->swi_dbpath         (RPM only)
 *                              swi->swi_maxrec         (RPM only)
 *                              swi->swi_nrec           (RPM only)
 *                              swi->swi_recs           (RPM only)
 *      rpm_db                  swi->swi_rpmdb          (RPM only)
 *                              swi->swi_h              (RPM only)
 *                              swi->swi_prevx          (RPM only)
 *
 *      dp                      swi->swi_dp
 *      de_p                    swi->swi_dep
 */
typedef struct {
#if HAVE_LIBRPM
    char           *swi_directory;
#else
    const char     *swi_directory;
#endif
    char            swi_name[SNMP_MAXPATH];     /* XXX longest file name */
    int             swi_index;

#if HAVE_LIBRPM
    const char     *swi_dbpath;

    time_t          swi_timestamp;      /* modify time on database */
    int             swi_maxrec; /* no. of allocations */
    int             swi_nrec;   /* no. of valid offsets */
    int            *swi_recs;   /* db record offsets */
    rpmdb           swi_rpmdb;
    Header          swi_h;
    int             swi_prevx;
#else
    DIR            *swi_dp;
    struct dirent  *swi_dep;
#endif

} SWI_t;

static SWI_t    _myswi = { NULL, "", 0 };       /* XXX static for now */

int             header_hrswinst(struct variable *, oid *, size_t *, int,
                                size_t *, WriteMethod **);
int             header_hrswInstEntry(struct variable *, oid *, size_t *,
                                     int, size_t *, WriteMethod **);

extern struct timeval starttime;

        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/
extern void     Init_HR_SWInst(void);
extern int      Get_Next_HR_SWInst(void);
extern void     End_HR_SWInst(void);
extern void     Save_HR_SW_info(int ix);

#ifdef HAVE_LIBRPM
static void     Mark_HRSW_token(void);
static void     Release_HRSW_token(void);
#else
#define	Mark_HRSW_token()
#define	Release_HRSW_token()
#endif


#define	HRSWINST_CHANGE		1
#define	HRSWINST_UPDATE		2
#define	HRSWINST_INDEX		3
#define	HRSWINST_NAME		4
#define	HRSWINST_ID		5
#define	HRSWINST_TYPE		6
#define	HRSWINST_DATE		7

struct variable4 hrswinst_variables[] = {
    {HRSWINST_CHANGE, ASN_TIMETICKS, RONLY, var_hrswinst, 1, {1}},
    {HRSWINST_UPDATE, ASN_TIMETICKS, RONLY, var_hrswinst, 1, {2}},
    {HRSWINST_INDEX, ASN_INTEGER, RONLY, var_hrswinst, 3, {3, 1, 1}},
    {HRSWINST_NAME, ASN_OCTET_STR, RONLY, var_hrswinst, 3, {3, 1, 2}},
    {HRSWINST_ID, ASN_OBJECT_ID, RONLY, var_hrswinst, 3, {3, 1, 3}},
    {HRSWINST_TYPE, ASN_INTEGER, RONLY, var_hrswinst, 3, {3, 1, 4}},
    {HRSWINST_DATE, ASN_OCTET_STR, RONLY, var_hrswinst, 3, {3, 1, 5}}
};
oid             hrswinst_variables_oid[] = { 1, 3, 6, 1, 2, 1, 25, 6 };


#ifdef PKGLOC                   /* Description from HRSW_dir/.../pkginfo: DESC= */
#define	_PATH_HRSW_directory	PKGLOC
#endif
#ifdef hpux9                    /* Description from HRSW_dir/.../index:   fd: */
#define	_PATH_HRSW_directory	"/system"
#endif
#ifdef hpux10                   /* Description from HRSW_dir/.../pfiles/INDEX: title */
#define	_PATH_HRSW_directory	"/var/adm/sw/products"
#endif
#ifdef hpux11                   /* Description from HRSW_dir/.../pfiles/INDEX: title */
#define	_PATH_HRSW_directory	"/var/adm/sw/products"
#endif
#ifdef freebsd2
#define	_PATH_HRSW_directory	"/var/db/pkg"
#endif

void
init_hr_swinst(void)
{
#if defined(HAVE_LIBRPM) || defined(_PATH_HRSW_directory)
    SWI_t          *swi = &_myswi;      /* XXX static for now */
#endif
#ifdef HAVE_LIBRPM
    struct stat     stat_buf;
#endif

    /*
     * Read settings from config file,
     * or take system-specific defaults 
     */

#ifdef HAVE_LIBRPM
    if (swi->swi_directory == NULL) {
        char            path[SNMP_MAXPATH];

        /*
         * XXX distinguish between rpm-2.5.x and rpm-2.9x 
         */
#ifdef HAVE_RPMGETPATH
        rpmReadConfigFiles(NULL, NULL);
        swi->swi_dbpath = rpmGetPath("%{_dbpath}", NULL);
#else
        rpmReadConfigFiles(NULL, NULL, NULL, 0);
        swi->swi_dbpath = rpmGetVar(RPMVAR_DBPATH);
#endif
        if (swi->swi_directory != NULL)
            free(swi->swi_directory);
        snprintf(path, sizeof(path), "%s/Packages", swi->swi_dbpath);
        if (stat(path, &stat_buf) == -1)
            snprintf(path, sizeof(path), "%s/packages.rpm", swi->swi_dbpath);
        path[ sizeof(path)-1 ] = 0;
        swi->swi_directory = strdup(path);
    }
#else
#  ifdef _PATH_HRSW_directory
    if (swi->swi_directory == NULL) {
        swi->swi_directory = _PATH_HRSW_directory;
    }
    strcpy(swi->swi_name, "[installed name]");  /* default name */
#  else
    /*
     * XXX SunOS4 package directory is ?? -MJS 
     */
    return;                     /* packages not known - don't register */
#  endif
#endif

    REGISTER_MIB("host/hr_swinst", hrswinst_variables, variable4,
                 hrswinst_variables_oid);
}

/*
 * header_hrswinst(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's 
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 */

int
header_hrswinst(struct variable *vp,
                oid * name,
                size_t * length,
                int exact, size_t * var_len, WriteMethod ** write_method)
{
#define HRSWINST_NAME_LENGTH	9
    oid             newname[MAX_OID_LEN];
    int             result;

    DEBUGMSGTL(("host/hr_swinst", "var_hrswinst: "));
    DEBUGMSGOID(("host/hr_swinst", name, *length));
    DEBUGMSG(("host/hr_swinst", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, vp->namelen * sizeof(oid));
    newname[HRSWINST_NAME_LENGTH] = 0;
    result = snmp_oid_compare(name, *length, newname, vp->namelen + 1);
    if ((exact && (result != 0)) || (!exact && (result >= 0)))
        return (MATCH_FAILED);
    memcpy((char *) name, (char *) newname,
           (vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;

    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */
    return (MATCH_SUCCEEDED);
}

int
header_hrswInstEntry(struct variable *vp,
                     oid * name,
                     size_t * length,
                     int exact,
                     size_t * var_len, WriteMethod ** write_method)
{
#define HRSWINST_ENTRY_NAME_LENGTH	11
    oid             newname[MAX_OID_LEN];
    int             swinst_idx, LowIndex = -1;
    int             result;

    DEBUGMSGTL(("host/hr_swinst", "var_hrswinstEntry: "));
    DEBUGMSGOID(("host/hr_swinst", name, *length));
    DEBUGMSG(("host/hr_swinst", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name, vp->namelen * sizeof(oid));
    /*
     * Find "next" installed software entry 
     */

    Init_HR_SWInst();
    while ((swinst_idx = Get_Next_HR_SWInst()) != -1) {
        DEBUGMSG(("host/hr_swinst", "(index %d ....", swinst_idx));

        newname[HRSWINST_ENTRY_NAME_LENGTH] = swinst_idx;
        DEBUGMSGOID(("host/hr_swinst", newname, *length));
        DEBUGMSG(("host/hr_swinst", "\n"));
        result = snmp_oid_compare(name, *length, newname, vp->namelen + 1);
        if (exact && (result == 0)) {
            LowIndex = swinst_idx;
            Save_HR_SW_info(LowIndex);
            break;
        }
        if ((!exact && (result < 0)) &&
            (LowIndex == -1 || swinst_idx < LowIndex)) {
            LowIndex = swinst_idx;
            Save_HR_SW_info(LowIndex);
#ifdef HRSWINST_MONOTONICALLY_INCREASING
            break;
#endif
        }
    }

    Mark_HRSW_token();
    End_HR_SWInst();

    if (LowIndex == -1) {
        DEBUGMSGTL(("host/hr_swinst", "... index out of range\n"));
        return (MATCH_FAILED);
    }

    memcpy((char *) name, (char *) newname,
           (vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */

    DEBUGMSGTL(("host/hr_inst", "... get installed S/W stats "));
    DEBUGMSGOID(("host/hr_inst", name, *length));
    DEBUGMSG(("host/hr_inst", "\n"));
    return LowIndex;
}

        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/


u_char         *
var_hrswinst(struct variable * vp,
             oid * name,
             size_t * length,
             int exact, size_t * var_len, WriteMethod ** write_method)
{
    SWI_t          *swi = &_myswi;      /* XXX static for now */
    int             sw_idx = 0;
    static char     string[SNMP_MAXPATH];
    u_char         *ret = NULL;
    struct stat     stat_buf;

    if (vp->magic < HRSWINST_INDEX) {
        if (header_hrswinst(vp, name, length, exact, var_len, write_method)
            == MATCH_FAILED)
            return NULL;
    } else {

        sw_idx =
            header_hrswInstEntry(vp, name, length, exact, var_len,
                                 write_method);
        if (sw_idx == MATCH_FAILED)
            return NULL;
    }

    switch (vp->magic) {
    case HRSWINST_CHANGE:
    case HRSWINST_UPDATE:
        string[0] = '\0';

        if (swi->swi_directory != NULL) {
            strncpy(string, swi->swi_directory, sizeof(string));
            string[ sizeof(string)-1 ] = 0;
        }

        if (*string && (stat(string, &stat_buf) != -1)) {
            if (stat_buf.st_mtime > starttime.tv_sec)
                /*
                 * changed 'recently' - i.e. since this agent started 
                 */
                long_return = (stat_buf.st_mtime - starttime.tv_sec) * 100;
            else
                long_return = 0;        /* predates this agent */
        } else
#if NO_DUMMY_VALUES
            return NULL;
#else
            long_return = 363136200;
#endif
        ret = (u_char *) & long_return;
        break;

    case HRSWINST_INDEX:
        long_return = sw_idx;
        ret = (u_char *) & long_return;
        break;
    case HRSWINST_NAME:
        {
            strncpy(string, swi->swi_name, sizeof(string) - 1);
            /*
             * This will be unchanged from the initial "null"
             * value, if swi->swi_name is not defined 
             */
            string[sizeof(string) - 1] = '\0';
            *var_len = strlen(string);
            ret = (u_char *) string;
        }
        break;
    case HRSWINST_ID:
        *var_len = nullOidLen;
        ret = (u_char *) nullOid;
        break;
    case HRSWINST_TYPE:
        {
#ifdef HAVE_PKGINFO
            /*
             * at least on solaris2 this works 
             */
            char           *catg = pkgparam(swi->swi_name, "CATEGORY");

            if (catg == NULL) {
                long_return = 1;        /*  unknown  */
            } else {
                if (strstr(catg, "system") != NULL) {
                    long_return = 2;    /*  operatingSystem  */
                } else if (strstr(catg, "application") != NULL) {
                    long_return = 4;    /*  applcation  */
                } else {
                    long_return = 1;    /*  unknown  */
                }
                free(catg);
            }
#else
            long_return = 1;    /* unknown */
#endif
            ret = (u_char *) & long_return;
        }
        break;
    case HRSWINST_DATE:
        {
#ifdef HAVE_LIBRPM
            int_32         *rpm_data;
            headerGetEntry(swi->swi_h, RPMTAG_INSTALLTIME, NULL,
                           (void **) &rpm_data, NULL);
            if (rpm_data != NULL) {
                time_t          installTime = *rpm_data;
                ret = date_n_time(&installTime, var_len);
            } else {
                ret = date_n_time(0, var_len);
            }
#else
            if (swi->swi_directory != NULL) {
                snprintf(string, sizeof(string), "%s/%s",
                         swi->swi_directory, swi->swi_name);
                string[ sizeof(string)-1 ] = 0;
                stat(string, &stat_buf);
                ret = date_n_time(&stat_buf.st_mtime, var_len);
            } else {
#if NO_DUMMY_VALUES
                return NULL;
#endif
                sprintf(string, "back in the mists of time");
                *var_len = strlen(string);
                ret = (u_char *) string;
            }
#endif
        }
        break;
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_hrswinst\n",
                    vp->magic));
        ret = NULL;
        break;
    }
    Release_HRSW_token();
    return ret;
}


        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/

#ifdef	HAVE_LIBRPM
static void
Check_HRSW_cache(void *xxx)
{
    SWI_t          *swi = (SWI_t *) xxx;

    /*
     * Make sure cache is up-to-date 
     */
    if (swi->swi_recs != NULL) {
        struct stat     sb;
        lstat(swi->swi_directory, &sb);
        if (swi->swi_timestamp == sb.st_mtime)
            return;
        swi->swi_timestamp = sb.st_mtime;
    }

    /*
     * Get header offsets 
     */
    {
        int             ix = 0;
        int             offset;

#if defined(RPMDBI_PACKAGES)
        rpmdbMatchIterator mi = NULL;
        Header          h;
        mi = rpmdbInitIterator(swi->swi_rpmdb, RPMDBI_PACKAGES, NULL, 0);
        while ((h = rpmdbNextIterator(mi)) != NULL) {
            offset = rpmdbGetIteratorOffset(mi);
#else
        for (offset = rpmdbFirstRecNum(swi->swi_rpmdb);
             offset != 0;
             offset = rpmdbNextRecNum(swi->swi_rpmdb, offset)) {
#endif

            if (ix >= swi->swi_maxrec) {
                swi->swi_maxrec += 256;
                swi->swi_recs = (swi->swi_recs == NULL)
                    ? (int *) malloc(swi->swi_maxrec * sizeof(int))
                    : (int *) realloc(swi->swi_recs,
                                      swi->swi_maxrec * sizeof(int));
            }
            swi->swi_recs[ix++] = offset;

#if !defined(RPMDBI_PACKAGES)
        }
#else
        }
        rpmdbFreeIterator(mi);
#endif

        swi->swi_nrec = ix;
    }
}
#endif                          /* HAVE_LIBRPM */

void
Init_HR_SWInst(void)
{
    SWI_t          *swi = &_myswi;      /* XXX static for now */
    swi->swi_index = 0;

#ifdef HAVE_LIBRPM
    if (swi->swi_rpmdb != NULL)
        return;
    if (rpmdbOpen("", &swi->swi_rpmdb, O_RDONLY, 0644) != 0)
        swi->swi_index = -1;
    Check_HRSW_cache(swi);
#else
    if (swi->swi_directory != NULL) {
        if (swi->swi_dp != NULL) {
            closedir(swi->swi_dp);
            swi->swi_dp = NULL;
        }
        if ((swi->swi_dp = opendir(swi->swi_directory)) == NULL)
            swi->swi_index = -1;
    } else
        swi->swi_index = -1;
#endif
}

int
Get_Next_HR_SWInst(void)
{
    SWI_t          *swi = &_myswi;      /* XXX static for now */

    if (swi->swi_index == -1)
        return -1;

#ifdef HAVE_LIBRPM
    /*
     * XXX Watchout: index starts with 1 
     */
    if (0 <= swi->swi_index && swi->swi_index < swi->swi_nrec)
        return ++swi->swi_index;
#else
    if (swi->swi_directory != NULL) {
        while ((swi->swi_dep = readdir(swi->swi_dp)) != NULL) {
            if (swi->swi_dep->d_name[0] == '.')
                continue;

            /*
             * Ought to check for "properly-formed" entry 
             */

            return ++swi->swi_index;
        }
    }
#endif

    return -1;
}

void
Save_HR_SW_info(int ix)
{
    SWI_t          *swi = &_myswi;      /* XXX static for now */

#ifdef HAVE_LIBRPM
    /*
     * XXX Watchout: ix starts with 1 
     */
    if (1 <= ix && ix <= swi->swi_nrec && ix != swi->swi_prevx) {
        int             offset;
        Header          h;
        char           *n, *v, *r;

        offset = swi->swi_recs[ix - 1];

#if defined(RPMDBI_PACKAGES)
        {
            rpmdbMatchIterator mi;
            mi = rpmdbInitIterator(swi->swi_rpmdb, RPMDBI_PACKAGES,
                                   &offset, sizeof(offset));
            if ((h = rpmdbNextIterator(mi)) != NULL)
                h = headerLink(h);
            rpmdbFreeIterator(mi);
        }
#else
        h = rpmdbGetRecord(swi->swi_rpmdb, offset);
#endif

        if (h == NULL)
            return;
        if (swi->swi_h != NULL)
            headerFree(swi->swi_h);
        swi->swi_h = h;
        swi->swi_prevx = ix;

        headerGetEntry(swi->swi_h, RPMTAG_NAME, NULL, (void **) &n, NULL);
        headerGetEntry(swi->swi_h, RPMTAG_VERSION, NULL, (void **) &v,
                       NULL);
        headerGetEntry(swi->swi_h, RPMTAG_RELEASE, NULL, (void **) &r,
                       NULL);
        snprintf(swi->swi_name, sizeof(swi->swi_name), "%s-%s-%s", n, v, r);
        swi->swi_name[ sizeof(swi->swi_name)-1 ] = 0;
    }
#else
    snprintf(swi->swi_name, sizeof(swi->swi_name), swi->swi_dep->d_name);
    swi->swi_name[ sizeof(swi->swi_name)-1 ] = 0;
#endif
}

#ifdef	HAVE_LIBRPM
void
Mark_HRSW_token(void)
{
}

void
Release_HRSW_token(void)
{
    SWI_t          *swi = &_myswi;      /* XXX static for now */
    if (swi != NULL && swi->swi_h) {
        headerFree(swi->swi_h);
        swi->swi_h = NULL;
        swi->swi_prevx = -1;
    }
}
#endif                          /* HAVE_LIBRPM */

void
End_HR_SWInst(void)
{
    SWI_t          *swi = &_myswi;      /* XXX static for now */

#ifdef HAVE_LIBRPM
    rpmdbClose(swi->swi_rpmdb); /* or only on finishing ? */
    swi->swi_rpmdb = NULL;
#else
    if (swi->swi_dp != NULL)
        closedir(swi->swi_dp);
    swi->swi_dp = NULL;
#endif
}
