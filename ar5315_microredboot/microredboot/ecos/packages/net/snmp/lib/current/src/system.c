//==========================================================================
//
//      ./lib/current/src/system.c
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
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
#include <config.h>
#include <stdio.h>
#include <ctype.h>

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

#include "asn1.h"
#include "snmp_api.h"
#include "tools.h"
#include "system.h"
#include "snmp_logging.h"

#define NUM_NETWORKS    32   /* max number of interfaces to check */

#ifndef IFF_LOOPBACK
#	define IFF_LOOPBACK 0
#endif

#define LOOPBACK    0x7f000001



/* ********************************************* */
#ifdef							WIN32
#	define WIN32_LEAN_AND_MEAN
#	define WIN32IO_IS_STDIO
#	define PATHLEN	1024

#	include <tchar.h>
#	include <windows.h>


/* The idea here is to read all the directory names into a string table
 * (separated by nulls) and when one of the other dir functions is called
 * return the pointer to the current file name.
 */
DIR *
opendir(const char *filename)
{
    DIR            *p;
    long            len;
    long            idx;
    char            scannamespc[PATHLEN];
    char       *scanname = scannamespc;
    struct stat     sbuf;
    WIN32_FIND_DATA FindData;
    HANDLE          fh;

    /* check to see if filename is a directory */
    if (stat(filename, &sbuf) < 0 || sbuf.st_mode & S_IFDIR == 0) {
	return NULL;
    }

    /* get the file system characteristics */
/*  if(GetFullPathName(filename, SNMP_MAXPATH, root, &dummy)) {
 *	if(dummy = strchr(root, '\\'))
 *	    *++dummy = '\0';
 *	if(GetVolumeInformation(root, volname, SNMP_MAXPATH, &serial,
 *				&maxname, &flags, 0, 0)) {
 *	    downcase = !(flags & FS_CASE_IS_PRESERVED);
 *	}
 *  }
 *  else {
 *	downcase = TRUE;
 *  }
 */

    /* Create the search pattern */
    strcpy(scanname, filename);

    if(strchr("/\\", *(scanname + strlen(scanname) - 1)) == NULL)
	strcat(scanname, "/*");
    else
	strcat(scanname, "*");

    /* do the FindFirstFile call */
    fh = FindFirstFile(scanname, &FindData);
    if(fh == INVALID_HANDLE_VALUE) {
	return NULL;
    }

    /* Get us a DIR structure */
    p = (DIR*)malloc(sizeof(DIR));
    /* Newz(1303, p, 1, DIR); */
    if(p == NULL)
	return NULL;

    /* now allocate the first part of the string table for
     * the filenames that we find.
     */
    idx = strlen(FindData.cFileName)+1;
    p->start = (char*)malloc(idx);
    /* New(1304, p->start, idx, char);*/
    if(p->start == NULL) {
		free(p);
		return NULL;
    }
    strcpy(p->start, FindData.cFileName);
/*  if(downcase)
 *	strlwr(p->start);
 */
    p->nfiles = 0;

    /* loop finding all the files that match the wildcard
     * (which should be all of them in this directory!).
     * the variable idx should point one past the null terminator
     * of the previous string found.
     */
    while (FindNextFile(fh, &FindData)) {
	len = strlen(FindData.cFileName);
	/* bump the string table size by enough for the
	 * new name and it's null terminator
	 */
	p->start = (char*)realloc((void*)p->start, idx+len+1);
	/* Renew(p->start, idx+len+1, char);*/
	if(p->start == NULL) {
		free(p);
	    return NULL;
	}
	strcpy(&p->start[idx], FindData.cFileName);
/*	if (downcase) 
 *	    strlwr(&p->start[idx]);
 */
		p->nfiles++;
		idx += len+1;
	}
	FindClose(fh);
	p->size = idx;
	p->curr = p->start;
	return p;
}


/* Readdir just returns the current string pointer and bumps the
 * string pointer to the nDllExport entry.
 */
struct direct *
readdir(DIR *dirp)
{
    int         len;
    static int  dummy = 0;

    if (dirp->curr) {
	/* first set up the structure to return */
	len = strlen(dirp->curr);
	strcpy(dirp->dirstr.d_name, dirp->curr);
	dirp->dirstr.d_namlen = len;

	/* Fake an inode */
	dirp->dirstr.d_ino = dummy++;

	/* Now set up for the nDllExport call to readdir */
	dirp->curr += len + 1;
	if (dirp->curr >= (dirp->start + dirp->size)) {
	    dirp->curr = NULL;
	}

	return &(dirp->dirstr);
    } 
    else
	return NULL;
}

/* free the memory allocated by opendir */
int
closedir(DIR *dirp)
{
    free(dirp->start);
    free(dirp);
    return 1;
}

#ifndef HAVE_GETTIMEOFDAY

int gettimeofday(struct timeval *tv,
		 struct timezone *tz)
{
    struct _timeb timebuffer;

    _ftime(&timebuffer);
    tv->tv_usec = timebuffer.millitm * 1000;
    tv->tv_sec = timebuffer.time;
    return(0);
}
#endif	/* !HAVE_GETTIMEOFDAY */

in_addr_t get_myaddr(void)
{
  char local_host[130];
  int result;
  LPHOSTENT lpstHostent;
  SOCKADDR_IN in_addr, remote_in_addr;
  SOCKET hSock;
  int nAddrSize = sizeof(SOCKADDR);

  in_addr.sin_addr.s_addr = INADDR_ANY;

  result = gethostname(local_host, sizeof(local_host));
  if (result == 0)
  {
	lpstHostent = gethostbyname((LPSTR)local_host);
	if (lpstHostent)
	{
	  in_addr.sin_addr.s_addr = *((u_long FAR *) (lpstHostent->h_addr));
	  return((in_addr_t)in_addr.sin_addr.s_addr);
	}
  }

  /* if we are here, than we don't have host addr */
  hSock = socket(AF_INET, SOCK_DGRAM, 0);
  if (hSock != INVALID_SOCKET)
  {
	  /* connect to any port and address */
	  remote_in_addr.sin_family = AF_INET;
	  remote_in_addr.sin_len = sizeof(remote_in_addr);
	  remote_in_addr.sin_port = htons(IPPORT_ECHO);
	  remote_in_addr.sin_addr.s_addr = inet_addr("128.22.33.11");
	  result=connect(hSock,(LPSOCKADDR)&remote_in_addr,sizeof(SOCKADDR)); 
	  if (result != SOCKET_ERROR)
	  {
	      /* get local ip address */
	      getsockname(hSock, (LPSOCKADDR)&in_addr,(int FAR *)&nAddrSize);
	  }
	  closesocket(hSock);
  }
  return((in_addr_t)in_addr.sin_addr.s_addr);
}

long get_uptime (void)
{
    return (0); /* not implemented */
}

char *
winsock_startup (void)
{
 WORD VersionRequested;
 WSADATA stWSAData;
 int i;
 static char errmsg[100];

 VersionRequested = MAKEWORD(1,1);
 i = WSAStartup(VersionRequested, &stWSAData); 
 if (i != 0)
 {
  if (i == WSAVERNOTSUPPORTED)
    sprintf(errmsg,"Unable to init. socket lib, does not support 1.1");
  else
  {
    sprintf(errmsg,"Socket Startup error %d", i);
  }
  return(errmsg);
 }
 return(NULL);
}

void winsock_cleanup (void)
{
   WSACleanup();
}

#else							/* ! WIN32 */
/*******************************************************************/

/*
 * XXX	What if we have multiple addresses?
 * XXX	Could it be computed once then cached?
 */
in_addr_t get_myaddr (void)
{
    int sd;
    struct ifconf ifc;
    struct ifreq *ifrp, ifreq;
    
    struct sockaddr *sa;
    struct sockaddr_in *in_addr;
    char conf[1024];
    
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return 0;
    ifc.ifc_len = sizeof(conf);
    ifc.ifc_buf = (caddr_t)conf;
    memset(conf,0,sizeof(conf));
    if (ioctl(sd, SIOCGIFCONF, (char *)&ifc) < 0){
      close(sd);
      return 0;
    }

    ifrp = ifc.ifc_req;
    
    while (ifc.ifc_len && ifrp->ifr_name[0]) {
      ifreq = *ifrp;
      if (ioctl(sd, SIOCGIFFLAGS, (char *)&ifreq) >= 0) {
        in_addr = (struct sockaddr_in *)&ifrp->ifr_addr;
        if ((ifreq.ifr_flags & IFF_UP)
#ifdef IFF_RUNNING
            && (ifreq.ifr_flags & IFF_RUNNING)
#endif /* IFF_RUNNING */
            && !(ifreq.ifr_flags & IFF_LOOPBACK)
            && in_addr->sin_addr.s_addr != LOOPBACK){
#ifdef SYS_IOCTL_H_HAS_SIOCGIFADDR
          if (ioctl(sd, SIOCGIFADDR, (char *)&ifreq) >= 0) {
            in_addr = (struct sockaddr_in *)&(ifreq.ifr_addr);
#endif
            close(sd);
            return in_addr->sin_addr.s_addr;
          }
        }
      }
      do {
        sa = &ifrp->ifr_addr;
        if (sa->sa_len <= sizeof(*sa)) {          
          ifrp++;
        } else {
          ifrp=(struct ifreq *)(sa->sa_len + (char *)sa);
          ifc.ifc_len -= sa->sa_len - sizeof(*sa);
        }
        ifc.ifc_len -= sizeof(*ifrp);          
        
      } while (!ifrp->ifr_name[0] && ifc.ifc_len);      
    }
    close(sd);
    return 0;
}


#if !defined(solaris2) && !defined(linux) && !defined(cygwin) && !defined(__ECOS)
/*
 * Returns boottime in centiseconds(!).
 *	Caches this for future use.
 */
long get_boottime (void)
{
    static long boottime_csecs = 0;
    struct timeval boottime;
#ifdef	CAN_USE_SYSCTL
    int	    mib[2];
    size_t  len;
#else
    int kmem;
    static struct nlist nl[] = {
#if !defined(hpux)
	    { (char*)"_boottime" },
#else
	    { (char*)"boottime" },
#endif
	    { (char*)"" }
	};
#endif


    if ( boottime_csecs != 0 )
	return( boottime_csecs );

#ifdef CAN_USE_SYSCTL
    mib[0] = CTL_KERN;
    mib[1] = KERN_BOOTTIME;

    len = sizeof(boottime);

    sysctl(mib, 2, &boottime, &len, NULL, NULL);
    boottime_csecs = (boottime.tv_sec * 100) + (boottime.tv_usec / 10000);
#else						/* CAN_USE_SYSCTL */
    if ((kmem = open("/dev/kmem", 0)) < 0)
	return 0;
    nlist(KERNEL_LOC, nl);
    if (nl[0].n_type == 0){
	close(kmem);
	return 0;
    }

    lseek(kmem, (long)nl[0].n_value, L_SET);
    read(kmem, &boottime, sizeof(boottime));
    close(kmem);
    boottime_csecs = (boottime.tv_sec * 100) + (boottime.tv_usec / 10000);
#endif						/* CAN_USE_SYSCTL */

    return( boottime_csecs );
}
#endif

/*
 * Returns uptime in centiseconds(!).
 */
#if !defined(__ECOS)
long get_uptime (void)
{
#if !defined(solaris2) && !defined(linux) && !defined(cygwin)
    struct timeval now;
    long boottime_csecs, nowtime_csecs;

    boottime_csecs = get_boottime();
    if (boottime_csecs == 0)
	return 0;
    gettimeofday(&now,(struct timezone *)0);
    nowtime_csecs = (now.tv_sec * 100) + (now.tv_usec / 10000);

    return (nowtime_csecs - boottime_csecs);
#endif

#ifdef solaris2
    kstat_ctl_t *ksc = kstat_open();
    kstat_t *ks;
    kid_t kid;
    kstat_named_t *named;
    u_long lbolt = 0;

    if (ksc) {
	ks = kstat_lookup (ksc, "unix", -1, "system_misc");
	if (ks) {
	    kid = kstat_read (ksc, ks, NULL);
	    if (kid != -1) {
		named = kstat_data_lookup(ks, "lbolt");
		if (named) {
		    lbolt = named->value.ul;
		}
	    }
	}
	kstat_close(ksc);
    }
    return lbolt;
#endif /* solaris2 */

#ifdef linux
   FILE *in = fopen ("/proc/uptime", "r");
   long uptim = 0, a, b;
   if (in) {
       if (2 == fscanf (in, "%ld.%ld", &a, &b))
	   uptim = a * 100 + b;
       fclose (in);
   }
   return uptim;
#endif /* linux */
   return (0); /* not implemented */
}
#endif // not __ECOS

#ifndef HAVE_GETTIMEOFDAY

int gettimeofday(struct timeval *tv,
		 struct timezone *tz)
{

    tv->tv_usec = 0;
    tv->tv_sec = time(NULL);
    return(0);
}
#endif	/* !HAVE_GETTIMEOFDAY */


#endif							/* ! WIN32 */
/*******************************************************************/

#ifndef HAVE_STRNCASECMP

/* test for NULL pointers before and NULL characters after
 * comparing possibly non-NULL strings.
 * WARNING: This function does NOT check for array overflow.
 */
int strncasecmp(const char *s1, const char *s2, size_t nch)
{
    size_t ii;
    int res = -1;

    if (!s1) {
        if (!s2)  return 0;
        return (-1);
    }
    if (!s2)
        return (1);

    for (ii = 0; (ii < nch) && *s1 && *s2; ii++, s1++, s2++)
    {
        res = (int) (tolower(*s1) - tolower(*s2));
        if (res != 0) break;
    }

    if ( ii == nch ) {
        s1--; s2--;
    }

    if (! *s1) {
        if (! *s2)  return 0;
        return (-1);
    }
    if (! *s2)
        return (1);

    return (res);
}

int strcasecmp(const char *s1, const char *s2)
{
    return strncasecmp(s1, s2, 1000000);
}

#endif /* HAVE_STRNCASECMP */


#ifndef HAVE_STRDUP
char *
strdup(const char *src)
{
    int len;
    char *dst;

    len = strlen(src) + 1;
    if ((dst = (char *)malloc(len)) == NULL)
	return(NULL);
    strcpy(dst, src);
    return(dst);
}
#endif	/* HAVE_STRDUP */

#ifndef HAVE_SETENV
int setenv(const char *name,
	   const char *value,
	   int overwrite)
{
    char *cp;
    int ret;

    if (overwrite == 0) {
	if (getenv(name)) return 0;
    }
    cp = (char*)malloc(strlen(name)+strlen(value)+2);
    if (cp == NULL) return -1;
    sprintf(cp, "%s=%s", name, value);
    ret = putenv(cp);
    return ret;
}
#endif /* HAVE_SETENV */

int
calculate_time_diff(struct timeval *now, struct timeval *then)
{
  struct timeval tmp, diff;
  memcpy(&tmp, now, sizeof(struct timeval));
  tmp.tv_sec--;
  tmp.tv_usec += 1000000L;
  diff.tv_sec = tmp.tv_sec - then->tv_sec;
  diff.tv_usec = tmp.tv_usec - then->tv_usec;
  if (diff.tv_usec > 1000000L){
    diff.tv_usec -= 1000000L;
    diff.tv_sec++;
  }
  return ((diff.tv_sec * 100) + (diff.tv_usec / 10000));
}

#ifndef HAVE_STRCASESTR
/*
 * only glibc2 has this.
 */
char *strcasestr(const char *haystack, const char *needle)
{
    const char *cp1=haystack, *cp2=needle;
    const char *cx;
    int tstch1, tstch2;

    /* printf("looking for '%s' in '%s'\n", needle, haystack); */
    if (cp1 && cp2 && *cp1 && *cp2)
        for (cp1=haystack, cp2=needle; *cp1; ) {
            cx = cp1; cp2 = needle;
            do {
                /* printf("T'%c' ", *cp1); */
                if (! *cp2) { /* found the needle */
                    /* printf("\nfound '%s' in '%s'\n", needle, cx); */
                    return (char *)cx;
                }
                if (! *cp1)
                    break;

                tstch1 = toupper(*cp1);
                tstch2 = toupper(*cp2);
                if (tstch1 != tstch2)
                    break;
                /* printf("M'%c' ", *cp1); */
                cp1++; cp2++;
            }
            while (1);
            if (*cp1)
                cp1++;
        }
    /* printf("\n"); */
    if (cp1 && *cp1)
        return (char *)cp1;

    return NULL;
}
#endif

#if !defined(__ECOS)
int
mkdirhier(const char *pathname, mode_t mode, int skiplast) {
    struct stat     sbuf;
    char *ourcopy = strdup(pathname);
    char *entry;
    char buf[SNMP_MAXPATH];

    entry = strtok( ourcopy, "/" );

    buf[0] = '\0';
    /* check to see if filename is a directory */
    while ( entry ) {
        strcat(buf,"/");
        strcat(buf, entry);
        entry = strtok( NULL, "/");
        if (entry == NULL && skiplast)
            break;
        if (stat(buf, &sbuf) < 0) {
            /* DNE, make it */
            snmp_log(LOG_INFO, "Creating directory: %s\n", buf);
#ifdef WIN32
	    CreateDirectory(buf, NULL);
#else
            mkdir(buf, mode);
#endif
        } else {
            /* exists, is it a file? */
            if ((sbuf.st_mode & S_IFDIR) == 0) {
                /* ack! can't make a directory on top of a file */
                free(ourcopy);
                return SNMPERR_GENERR;
            }
        }
    }
    free(ourcopy);
    return SNMPERR_SUCCESS;
}
#endif


#ifdef __ECOS
#include <cyg/kernel/kapi.h>

long get_boottime (void)
{
    return 1l;
}

long get_uptime (void)
{
    return cyg_current_time();
}

#endif

