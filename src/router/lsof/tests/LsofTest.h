/*
 * LsofTest.h -- header file for lsof tests
 */

/*
 * Copyright 2002 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

/*
 * $Id: LsofTest.h,v 1.13 2018/02/14 14:21:44 abe Exp $
 */

#if !defined(LSOF_TEST_H)
#    define LSOF_TEST_H 1

/*
 * The following define keeps gcc>=2.7 from complaining about the failure
 * of the Exit() function to return.
 *
 * Paul Eggert <eggert@twinsun.com> supplied it.
 */

#    if defined(__GNUC__) &&                                                   \
        !(__GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7))
#        define exiting __attribute__((__noreturn__))
#    else /* !gcc || gcc<2.7 */
#        define exiting
#    endif /* gcc && gcc>=2.7 */

/*
 * Necessary header files.
 */

#    include <stdio.h>
#    include <ctype.h>
#    include <errno.h>
#    include <signal.h>
#    include <inttypes.h>

#    include <sys/types.h>

#    if defined(LT_DIAL_linux) && LT_VERS >= 414014
#        undef major
#        include <sys/sysmacros.h>
#    endif /* defined(LT_DIAL_linux) && LT_VERS>=414014 */

#    include <sys/param.h>
#    include <sys/stat.h>

/*
 * Definitions that may be revoked by a particular dialect.
 */

#    define USE_GETCWD     /* use the POSIX getcwd() function in               \
                            * place of getwd() */
#    define USE_LSOF_C_OPT /* use lsof's -C option */
#    undef USE_LSOF_X_OPT  /* don't use lsof's -X option */

#    if defined(LT_DIAL_aix)
/*
 * AIX-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#        include <sys/access.h>
#        undef USE_LSOF_C_OPT
#        define USE_LSOF_X_OPT
#    endif /* defined(LT_DIAL_aix) */

#    if defined(LT_DIAL_bsdi)
/*
 * BSDI-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#        include <sys/wait.h>
#    endif /* defined(LT_DIAL_bsdi) */

#    if defined(LT_DIAL_darwin)
/*
 * Darwin-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#        include <sys/wait.h>
#        undef USE_LSOF_C_OPT
#    endif /* defined(LT_DIAL_darwin) */

#    if defined(LT_DIAL_du)
/*
 * DEC_OSF/1|Digital_UNIX|Tru64_UNIX-specific items
 */

#        include <fcntl.h>
#        include <string.h>
#        include <unistd.h>
#        include <sys/wait.h>

#        if LT_VERS < 50000
#            define snprintf snpf /* use lsof's snpf() */
#        endif                    /* LT_VERS<50000 */
#    endif                        /* defined(LT_DIAL_du) */

#    if defined(LT_DIAL_freebsd)
/*
 * FreeBSD-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#        include <sys/wait.h>
#        undef USE_LSOF_C_OPT
#    endif /* defined(LT_DIAL_freebsd) */

#    if defined(LT_DIAL_linux)
/*
 * Linux-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#        include <sys/wait.h>
#        undef USE_LSOF_C_OPT
#    endif /* defined(LT_DIAL_linux) */

#    if defined(LT_DIAL_hpux)
/*
 * HP-UX-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <strings.h>
#        include <unistd.h>
#    endif /* defined(LT_DIAL_hpux) */

#    if defined(LT_DIAL_netbsd)
/*
 * NetBSD-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#        include <sys/wait.h>
#    endif /* defined(LT_DIAL_netbsd) */

#    if defined(LT_DIAL_openbsd)
/*
 * OpenBSD-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#        include <sys/wait.h>
#        undef USE_LSOF_C_OPT
#    endif /* defined(LT_DIAL_openbsd) */

#    if defined(LT_DIAL_ou)
/*
 * OpenUNIX-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#    endif /* defined(LT_DIAL_ou) */

#    if defined(LT_DIAL_osr)
/*
 * OSR-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#    endif /* defined(LT_DIAL_osr) */

#    if defined(LT_DIAL_ns)
/*
 * NEXTSTEP-specific items
 */

#        include <stdlib.h>
#        include <string.h>
#        include <sys/file.h>
#        include <sys/wait.h>

typedef int pid_t;
#        define snprintf snpf

#        undef USE_GETCWD
#    endif /* defined(LT_DIAL_ns) */

#    if defined(LT_DIAL_solaris)
/*
 * Solaris-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <strings.h>
#        include <unistd.h>
#        include <sys/wait.h>

#        if defined(LT_VPATH)
#            undef USE_LSOF_C_OPT
#        endif /* defined(LT_VPATH) */
#    endif     /* defined(LT_DIAL_solaris) */

#    if defined(LT_DIAL_uw)
/*
 * UnixWare-specific items
 */

#        include <fcntl.h>
#        include <stdlib.h>
#        include <string.h>
#        include <unistd.h>
#    endif /* defined(LT_DIAL_uw) */

/*
 * Local definitions, including ones may have been left undefined by
 * dialect-specific header files
 */

#    define LT_DONT_DO_TEST "this test does not run on this dialect."
#    if !defined(LT_DEF_LSOF_PATH)
#        define LT_DEF_LSOF_PATH "../lsof"
#    endif /* !defined(LT_DEF_LSOF_PATH) */

#    if !defined(MAXPATHLEN)
#        define MAXPATHLEN 1024
#    endif /* !defined(MAXPATHLEN) */

/*
 * Local structure definitions
 */

typedef struct LTdev { /* local device parameters */
    unsigned int maj;  /* major device number */
    unsigned int min;  /* minor device number */
    unsigned int unit; /* unit number (where applicable) */
} LTdev_t;

typedef struct LTfldo { /* lsof field output information */
    char ft;            /* field identifier (see the LSOF_FID_*
                         * definitions in ../lsof_fields.h) */
    char *v;            /* field value character string */
} LTfldo_t;
#    define LT_FLDO_ALLOC 16 /* LTfldo_t allocation increment */

/*
 * Lsof test library global variable external declarations:
 *
 *	these global variables may be found in LTlib.c.
 */

extern int LsofFd;     /* lsof pipe FD */
extern FILE *LsofFs;   /* stream for lsof pipe FD */
extern char *LsofPath; /* path to lsof executable */
extern pid_t LsofPid;  /* PID of lsof child process */
extern int LTopt_h;    /* "-h" option's switch value */
extern char *LTopt_p;  /* "-p path" option's path value */
extern int MsgStat;    /* message status */

/*
 * External declarations
 */

extern int errno; /* error number */

/*
 * Lsof test library function prototypes:
 *
 *	these functions may be found in LTlib.c.
 */

extern char *CanRdKmem(void);
extern char *ConvStatDev(dev_t *dev, LTdev_t *ldev);
extern char *ConvLsofDev(char *dev, LTdev_t *ldev);
extern char *ExecLsof(char **opt);
extern char *IsLsofExec(void);
extern void LTlibClean(void);
extern char *MkStrCpy(char *src, int *len);
extern LTfldo_t *RdFrLsof(int *nf, char **em);
extern void PrtMsg(char *mp, char *pn);
extern void PrtMsgX(char *mp, char *pn, void (*f)(), int xv);
extern int ScanArg(int ac, char *av[], char *opt, char *pn);
extern void StopLsof(void);

#endif /* LSOF_TEST_H */
