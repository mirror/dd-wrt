/*
 * system.c
 */
/***********************************************************
        Copyright 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
/*
 * System dependent routines go here
 */
#include <net-snmp/net-snmp-config.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <sys/types.h>

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NET_IF_H
#include <net/if.h>
#endif

#if HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_NLIST_H
#include <nlist.h>
#endif

#if HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#if HAVE_KSTAT_H
#include <kstat.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if defined(hpux10) || defined(hpux11)
#include <sys/pstat.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/utilities.h>
#include <net-snmp/library/system.h>    /* for "internal" definitions */

#include <net-snmp/library/snmp_api.h>

#ifndef IFF_LOOPBACK
#	define IFF_LOOPBACK 0
#endif

#ifdef  INADDR_LOOPBACK
# define LOOPBACK    INADDR_LOOPBACK
#else
# define LOOPBACK    0x7f000001
#endif



/*
 * ********************************************* 
 */
#ifdef							WIN32
#	define WIN32_LEAN_AND_MEAN
#	define WIN32IO_IS_STDIO
#	define PATHLEN	1024

#	include <tchar.h>
#	include <windows.h>


/*
 * The idea here is to read all the directory names into a string table
 * * (separated by nulls) and when one of the other dir functions is called
 * * return the pointer to the current file name.
 */
DIR            *
opendir(const char *filename)
{
    DIR            *p;
    long            len;
    long            idx;
    char            scannamespc[PATHLEN];
    char           *scanname = scannamespc;
    struct stat     sbuf;
    WIN32_FIND_DATA FindData;
    HANDLE          fh;

    /*
     * check to see if filename is a directory 
     */
    if (stat(filename, &sbuf) < 0 || sbuf.st_mode & S_IFDIR == 0) {
        return NULL;
    }

    /*
     * get the file system characteristics 
     */
    /*
     * if(GetFullPathName(filename, SNMP_MAXPATH, root, &dummy)) {
     * *    if(dummy = strchr(root, '\\'))
     * *        *++dummy = '\0';
     * *    if(GetVolumeInformation(root, volname, SNMP_MAXPATH, &serial,
     * *                            &maxname, &flags, 0, 0)) {
     * *        downcase = !(flags & FS_CASE_IS_PRESERVED);
     * *    }
     * *  }
     * *  else {
     * *    downcase = TRUE;
     * *  }
     */

    /*
     * Create the search pattern 
     */
    strcpy(scanname, filename);

    if (strchr("/\\", *(scanname + strlen(scanname) - 1)) == NULL)
        strcat(scanname, "/*");
    else
        strcat(scanname, "*");

    /*
     * do the FindFirstFile call 
     */
    fh = FindFirstFile(scanname, &FindData);
    if (fh == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    /*
     * Get us a DIR structure 
     */
    p = (DIR *) malloc(sizeof(DIR));
    /*
     * Newz(1303, p, 1, DIR); 
     */
    if (p == NULL)
        return NULL;

    /*
     * now allocate the first part of the string table for
     * * the filenames that we find.
     */
    idx = strlen(FindData.cFileName) + 1;
    p->start = (char *) malloc(idx);
    /*
     * New(1304, p->start, idx, char);
     */
    if (p->start == NULL) {
        free(p);
        return NULL;
    }
    strcpy(p->start, FindData.cFileName);
    /*
     * if(downcase)
     * *    strlwr(p->start);
     */
    p->nfiles = 0;

    /*
     * loop finding all the files that match the wildcard
     * * (which should be all of them in this directory!).
     * * the variable idx should point one past the null terminator
     * * of the previous string found.
     */
    while (FindNextFile(fh, &FindData)) {
        len = strlen(FindData.cFileName);
        /*
         * bump the string table size by enough for the
         * * new name and it's null terminator
         */
        p->start = (char *) realloc((void *) p->start, idx + len + 1);
        /*
         * Renew(p->start, idx+len+1, char);
         */
        if (p->start == NULL) {
            free(p);
            return NULL;
        }
        strcpy(&p->start[idx], FindData.cFileName);
        /*
         * if (downcase) 
         * *        strlwr(&p->start[idx]);
         */
        p->nfiles++;
        idx += len + 1;
    }
    FindClose(fh);
    p->size = idx;
    p->curr = p->start;
    return p;
}


/*
 * Readdir just returns the current string pointer and bumps the
 * * string pointer to the nDllExport entry.
 */
struct direct  *
readdir(DIR * dirp)
{
    int             len;
    static int      dummy = 0;

    if (dirp->curr) {
        /*
         * first set up the structure to return 
         */
        len = strlen(dirp->curr);
        strcpy(dirp->dirstr.d_name, dirp->curr);
        dirp->dirstr.d_namlen = len;

        /*
         * Fake an inode 
         */
        dirp->dirstr.d_ino = dummy++;

        /*
         * Now set up for the nDllExport call to readdir 
         */
        dirp->curr += len + 1;
        if (dirp->curr >= (dirp->start + dirp->size)) {
            dirp->curr = NULL;
        }

        return &(dirp->dirstr);
    } else
        return NULL;
}

/*
 * free the memory allocated by opendir 
 */
int
closedir(DIR * dirp)
{
    free(dirp->start);
    free(dirp);
    return 1;
}

#ifndef HAVE_GETTIMEOFDAY

int
gettimeofday(struct timeval *tv, struct timezone *tz)
{
    struct _timeb   timebuffer;

    _ftime(&timebuffer);
    tv->tv_usec = timebuffer.millitm * 1000;
    tv->tv_sec = timebuffer.time;
    return (0);
}
#endif                          /* !HAVE_GETTIMEOFDAY */

in_addr_t
get_myaddr(void)
{
    char            local_host[130];
    int             result;
    LPHOSTENT       lpstHostent;
    SOCKADDR_IN     in_addr, remote_in_addr;
    SOCKET          hSock;
    int             nAddrSize = sizeof(SOCKADDR);

    in_addr.sin_addr.s_addr = INADDR_ANY;

    result = gethostname(local_host, sizeof(local_host));
    if (result == 0) {
        lpstHostent = gethostbyname((LPSTR) local_host);
        if (lpstHostent) {
            in_addr.sin_addr.s_addr =
                *((u_long FAR *) (lpstHostent->h_addr));
            return ((in_addr_t) in_addr.sin_addr.s_addr);
        }
    }

    /*
     * if we are here, than we don't have host addr 
     */
    hSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (hSock != INVALID_SOCKET) {
        /*
         * connect to any port and address 
         */
        remote_in_addr.sin_family = AF_INET;
        remote_in_addr.sin_port = htons(IPPORT_ECHO);
        remote_in_addr.sin_addr.s_addr = inet_addr("128.22.33.11");
        result =
            connect(hSock, (LPSOCKADDR) & remote_in_addr,
                    sizeof(SOCKADDR));
        if (result != SOCKET_ERROR) {
            /*
             * get local ip address 
             */
            getsockname(hSock, (LPSOCKADDR) & in_addr,
                        (int FAR *) &nAddrSize);
        }
        closesocket(hSock);
    }
    return ((in_addr_t) in_addr.sin_addr.s_addr);
}

long
get_uptime(void)
{
    long            return_value = 0;
    DWORD           buffersize = (sizeof(PERF_DATA_BLOCK) +
                                  sizeof(PERF_OBJECT_TYPE)),
        type = REG_EXPAND_SZ;
    PPERF_DATA_BLOCK perfdata = NULL;

    /*
     * min requirement is one PERF_DATA_BLOCK plus one PERF_OBJECT_TYPE 
     */
    perfdata = (PPERF_DATA_BLOCK) malloc(buffersize);


    memset(perfdata, 0, buffersize);

    RegQueryValueEx(HKEY_PERFORMANCE_DATA,
                    "Global", NULL, &type, (LPBYTE) perfdata, &buffersize);

    /*
     * we can not rely on the return value since there is always more so
     * we check the signature 
     */

    if (wcsncmp(perfdata->Signature, L"PERF", 4) == 0) {
        /*
         * signature ok, and all we need is in the in the PERF_DATA_BLOCK 
         */
        return_value = (long) ((perfdata->PerfTime100nSec.QuadPart /
                                (LONGLONG) 100000));
    } else
        return_value = GetTickCount() / 10;

    RegCloseKey(HKEY_PERFORMANCE_DATA);
    free(perfdata);

    return return_value;
}

char           *
winsock_startup(void)
{
    WORD            VersionRequested;
    WSADATA         stWSAData;
    int             i;
    static char     errmsg[100];

    VersionRequested = MAKEWORD(1, 1);
    i = WSAStartup(VersionRequested, &stWSAData);
    if (i != 0) {
        if (i == WSAVERNOTSUPPORTED)
            sprintf(errmsg,
                    "Unable to init. socket lib, does not support 1.1");
        else {
            sprintf(errmsg, "Socket Startup error %d", i);
        }
        return (errmsg);
    }
    return (NULL);
}

void
winsock_cleanup(void)
{
    WSACleanup();
}

#else                           /* ! WIN32 */
/*******************************************************************/

/*
 * XXX  What if we have multiple addresses?  Or no addresses for that matter?
 * XXX  Could it be computed once then cached?  Probably not worth it (not
 *                                                           used very often).
 */
in_addr_t
get_myaddr(void)
{
    int             sd, i, lastlen = 0;
    struct ifconf   ifc;
    struct ifreq   *ifrp = NULL;
    in_addr_t       addr;
    char           *buf = NULL;

    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return 0;
    }

    /*
     * Cope with lots of interfaces and brokenness of ioctl SIOCGIFCONF on
     * some platforms; see W. R. Stevens, ``Unix Network Programming Volume
     * I'', p.435.  
     */

    for (i = 8;; i += 8) {
        buf = (char *) calloc(i, sizeof(struct ifreq));
        if (buf == NULL) {
            close(sd);
            return 0;
        }
        ifc.ifc_len = i * sizeof(struct ifreq);
        ifc.ifc_buf = (caddr_t) buf;

        if (ioctl(sd, SIOCGIFCONF, (char *) &ifc) < 0) {
            if (errno != EINVAL || lastlen != 0) {
                /*
                 * Something has gone genuinely wrong.  
                 */
                free(buf);
                close(sd);
                return 0;
            }
            /*
             * Otherwise, it could just be that the buffer is too small.  
             */
        } else {
            if (ifc.ifc_len == lastlen) {
                /*
                 * The length is the same as the last time; we're done.  
                 */
                break;
            }
            lastlen = ifc.ifc_len;
        }
        free(buf);
    }

    for (ifrp = ifc.ifc_req;
        (char *)ifrp < (char *)ifc.ifc_req + ifc.ifc_len;
#ifdef STRUCT_SOCKADDR_HAS_SA_LEN
        ifrp = (struct ifreq *)(((char *) ifrp) +
                                sizeof(ifrp->ifr_name) +
                                ifrp->ifr_addr.sa_len)
#else
        ifrp++
#endif
        ) {
        if (ifrp->ifr_addr.sa_family != AF_INET) {
            continue;
        }
        addr = ((struct sockaddr_in *) &(ifrp->ifr_addr))->sin_addr.s_addr;

        if (ioctl(sd, SIOCGIFFLAGS, (char *) ifrp) < 0) {
            continue;
        }
        if ((ifrp->ifr_flags & IFF_UP)
#ifdef IFF_RUNNING
            && (ifrp->ifr_flags & IFF_RUNNING)
#endif                          /* IFF_RUNNING */
            && !(ifrp->ifr_flags & IFF_LOOPBACK)
            && addr != LOOPBACK) {
            /*
             * I *really* don't understand why this is necessary.  Perhaps for
             * some broken platform?  Leave it for now.  JBPN  
             */
#ifdef SYS_IOCTL_H_HAS_SIOCGIFADDR
            if (ioctl(sd, SIOCGIFADDR, (char *) ifrp) < 0) {
                continue;
            }
            addr =
                ((struct sockaddr_in *) &(ifrp->ifr_addr))->sin_addr.
                s_addr;
#endif
            free(buf);
            close(sd);
            return addr;
        }
    }
    free(buf);
    close(sd);
    return 0;
}


#if !defined(solaris2) && !defined(linux) && !defined(cygwin)
/*
 * Returns boottime in centiseconds(!).
 *      Caches this for future use.
 */
long
get_boottime(void)
{
    static long     boottime_csecs = 0;
#if defined(hpux10) || defined(hpux11)
    struct pst_static pst_buf;
#else
    struct timeval  boottime;
#ifdef	CAN_USE_SYSCTL
    int             mib[2];
    size_t          len;
#else
    int             kmem;
    static struct nlist nl[] = {
#if !defined(hpux)
        {(char *) "_boottime"},
#else
        {(char *) "boottime"},
#endif
        {(char *) ""}
    };
#endif                          /* CAN_USE_SYSCTL */
#endif                          /* hpux10 || hpux 11 */


    if (boottime_csecs != 0)
        return (boottime_csecs);

#if defined(hpux10) || defined(hpux11)
    pstat_getstatic(&pst_buf, sizeof(struct pst_static), 1, 0);
    boottime_csecs = pst_buf.boot_time * 100;
#else
#ifdef CAN_USE_SYSCTL
    mib[0] = CTL_KERN;
    mib[1] = KERN_BOOTTIME;

    len = sizeof(boottime);

    sysctl(mib, 2, &boottime, &len, NULL, NULL);
    boottime_csecs = (boottime.tv_sec * 100) + (boottime.tv_usec / 10000);
#else                           /* CAN_USE_SYSCTL */
    if ((kmem = open("/dev/kmem", 0)) < 0)
        return 0;
    nlist(KERNEL_LOC, nl);
    if (nl[0].n_type == 0) {
        close(kmem);
        return 0;
    }

    lseek(kmem, (long) nl[0].n_value, L_SET);
    read(kmem, &boottime, sizeof(boottime));
    close(kmem);
    boottime_csecs = (boottime.tv_sec * 100) + (boottime.tv_usec / 10000);
#endif                          /* CAN_USE_SYSCTL */
#endif                          /* hpux10 || hpux 11 */

    return (boottime_csecs);
}
#endif

/*
 * Returns uptime in centiseconds(!).
 */
long
get_uptime(void)
{
#if !defined(solaris2) && !defined(linux) && !defined(cygwin)
    struct timeval  now;
    long            boottime_csecs, nowtime_csecs;

    boottime_csecs = get_boottime();
    if (boottime_csecs == 0)
        return 0;
    gettimeofday(&now, (struct timezone *) 0);
    nowtime_csecs = (now.tv_sec * 100) + (now.tv_usec / 10000);

    return (nowtime_csecs - boottime_csecs);
#endif

#ifdef solaris2
    kstat_ctl_t    *ksc = kstat_open();
    kstat_t        *ks;
    kid_t           kid;
    kstat_named_t  *named;
    u_long          lbolt = 0;

    if (ksc) {
        ks = kstat_lookup(ksc, "unix", -1, "system_misc");
        if (ks) {
            kid = kstat_read(ksc, ks, NULL);
            if (kid != -1) {
                named = kstat_data_lookup(ks, "lbolt");
                if (named) {
#ifdef KSTAT_DATA_INT32
                    lbolt = named->value.ul;
#else
                    lbolt = named->value.ul;
#endif
                }
            }
        }
        kstat_close(ksc);
    }
    return lbolt;
#endif                          /* solaris2 */

#ifdef linux
    FILE           *in = fopen("/proc/uptime", "r");
    long            uptim = 0, a, b;
    if (in) {
        if (2 == fscanf(in, "%ld.%ld", &a, &b))
            uptim = a * 100 + b;
        fclose(in);
    }
    return uptim;
#endif                          /* linux */

#ifdef cygwin
    return (0);                 /* not implemented */
#endif
}

#endif                          /* ! WIN32 */
/*******************************************************************/

#ifndef HAVE_STRNCASECMP

/*
 * test for NULL pointers before and NULL characters after
 * * comparing possibly non-NULL strings.
 * * WARNING: This function does NOT check for array overflow.
 */
int
strncasecmp(const char *s1, const char *s2, size_t nch)
{
    size_t          ii;
    int             res = -1;

    if (!s1) {
        if (!s2)
            return 0;
        return (-1);
    }
    if (!s2)
        return (1);

    for (ii = 0; (ii < nch) && *s1 && *s2; ii++, s1++, s2++) {
        res = (int) (tolower(*s1) - tolower(*s2));
        if (res != 0)
            break;
    }

    if (ii == nch) {
        s1--;
        s2--;
    }

    if (!*s1) {
        if (!*s2)
            return 0;
        return (-1);
    }
    if (!*s2)
        return (1);

    return (res);
}

int
strcasecmp(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, 1000000);
}

#endif                          /* HAVE_STRNCASECMP */


#ifndef HAVE_STRDUP
char           *
strdup(const char *src)
{
    int             len;
    char           *dst;

    len = strlen(src) + 1;
    if ((dst = (char *) malloc(len)) == NULL)
        return (NULL);
    strcpy(dst, src);
    return (dst);
}
#endif                          /* HAVE_STRDUP */

#ifndef HAVE_SETENV
int
setenv(const char *name, const char *value, int overwrite)
{
    char           *cp;
    int             ret;

    if (overwrite == 0) {
        if (getenv(name))
            return 0;
    }
    cp = (char *) malloc(strlen(name) + strlen(value) + 2);
    if (cp == NULL)
        return -1;
    sprintf(cp, "%s=%s", name, value);
    ret = putenv(cp);
#ifdef WIN32
    free(cp);
#endif
    return ret;
}
#endif                          /* HAVE_SETENV */

int
calculate_time_diff(struct timeval *now, struct timeval *then)
{
    struct timeval  tmp, diff;
    memcpy(&tmp, now, sizeof(struct timeval));
    tmp.tv_sec--;
    tmp.tv_usec += 1000000L;
    diff.tv_sec = tmp.tv_sec - then->tv_sec;
    diff.tv_usec = tmp.tv_usec - then->tv_usec;
    if (diff.tv_usec > 1000000L) {
        diff.tv_usec -= 1000000L;
        diff.tv_sec++;
    }
    return ((diff.tv_sec * 100) + (diff.tv_usec / 10000));
}

#ifndef HAVE_STRCASESTR
/*
 * only glibc2 has this.
 */
char           *
strcasestr(const char *haystack, const char *needle)
{
    const char     *cp1 = haystack, *cp2 = needle;
    const char     *cx;
    int             tstch1, tstch2;

    /*
     * printf("looking for '%s' in '%s'\n", needle, haystack); 
     */
    if (cp1 && cp2 && *cp1 && *cp2)
        for (cp1 = haystack, cp2 = needle; *cp1;) {
            cx = cp1;
            cp2 = needle;
            do {
                /*
                 * printf("T'%c' ", *cp1); 
                 */
                if (!*cp2) {    /* found the needle */
                    /*
                     * printf("\nfound '%s' in '%s'\n", needle, cx); 
                     */
                    return (char *) cx;
                }
                if (!*cp1)
                    break;

                tstch1 = toupper(*cp1);
                tstch2 = toupper(*cp2);
                if (tstch1 != tstch2)
                    break;
                /*
                 * printf("M'%c' ", *cp1); 
                 */
                cp1++;
                cp2++;
            }
            while (1);
            if (*cp1)
                cp1++;
        }
    /*
     * printf("\n"); 
     */
    if (cp1 && *cp1)
        return (char *) cp1;

    return NULL;
}
#endif

int
mkdirhier(const char *pathname, mode_t mode, int skiplast)
{
    struct stat     sbuf;
    char           *ourcopy = strdup(pathname);
    char           *entry;
    char            buf[SNMP_MAXPATH];

    entry = strtok(ourcopy, "/");

    buf[0] = '\0';
    /*
     * check to see if filename is a directory 
     */
    while (entry) {
        strcat(buf, "/");
        strcat(buf, entry);
        entry = strtok(NULL, "/");
        if (entry == NULL && skiplast)
            break;
        if (stat(buf, &sbuf) < 0) {
            /*
             * DNE, make it 
             */
            snmp_log(LOG_INFO, "Creating directory: %s\n", buf);
#ifdef WIN32
            CreateDirectory(buf, NULL);
#else
            if (mkdir(buf, mode) == -1) {
                free(ourcopy);
                return SNMPERR_GENERR;
            }
#endif
        } else {
            /*
             * exists, is it a file? 
             */
            if ((sbuf.st_mode & S_IFDIR) == 0) {
                /*
                 * ack! can't make a directory on top of a file 
                 */
                free(ourcopy);
                return SNMPERR_GENERR;
            }
        }
    }
    free(ourcopy);
    return SNMPERR_SUCCESS;
}
