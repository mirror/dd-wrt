/*
 * common.h - common header file for lsof
 */

/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
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
 * $Id: common.h,v 1.70 2018/03/26 21:50:45 abe Exp $
 */

#if !defined(COMMON_H)
#    define COMMON_H 1

#    include "lsof.h"
#    if defined(AUTOTOOLS)
#        include "autotools.h"
#    endif
#    include "machine.h"

#    if !defined(FSV_DEFAULT)
#        define FSV_DEFAULT 0
#    endif /* !defined(FSV_DEFAULT) */

#    include "lsof_fields.h"

#    include <ctype.h>
#    include <errno.h>

#    if defined(HASSETLOCALE)
#        include <locale.h>
#    endif /* defined(HASSETLOCALE) */

#    include <netdb.h>
#    include <pwd.h>
#    include <stdio.h>

#    include <sys/stat.h>
#    include <sys/wait.h>

/*
 * Definitions and structures that may be needed by dlsof.h
 */

#    if !defined(INODETYPE)
#        define INODETYPE unsigned long /* node number storage type */
#        define INODEPSPEC                                                     \
            "l" /* node number printf specification                            \
                 * modifier */
#    endif      /* !defined(INODETYPE) */

struct l_dev {
    dev_t rdev;      /* device */
    INODETYPE inode; /* inode number */
    char *name;      /* name */
    int v;           /* has been verified
                      * (when DCUnsafe == 1) */
};

#    if defined(HASEPTOPTS)
/*
 * End point definitions
 */

#        define CHEND_PIPE 0x01 /* pipe endpoint ID */
#        define CHEND_PTY 0x02  /* pseudoterminal endpoint ID */

#        if defined(HASUXSOCKEPT)
#            define CHEND_UXS 0x04 /* UNIX socket endpoint ID */
#        endif                     /* defined(HASUXSOCKEPT) */

#        define CHEND_NETS 0x08  /* INET socket endpoint ID */
#        define CHEND_PSXMQ 0x10 /* Posix MQ endpoint ID */

#        if defined(HASIPv6)
#            define CHEND_NETS6 0x20 /* INET6 socket endpoint ID */
#        endif                       /* defined(HASIPv6) */

#        define CHEND_EVTFD 0x40 /* eventfd endpoint ID */

#        define EPT_PIPE 0x01     /* process has pipe file */
#        define EPT_PIPE_END 0x02 /* process has pipe end point file */
#        define EPT_PTY 0x04      /* process has a pseudoterminal file */
#        define EPT_PTY_END                                                    \
            0x08 /* process has a pseudoterminal end                           \
                  * point file */

#        if defined(HASUXSOCKEPT)
#            define EPT_UXS 0x10 /* process has a UNIX socket file */
#            define EPT_UXS_END                                                \
                0x20 /* process has a UNIX socket end point                    \
                      * file */
#        endif       /* defined(HASUXSOCKEPT) */

#        define EPT_NETS 0x40 /* process has a INET socket file */
#        define EPT_NETS_END                                                   \
            0x80 /* process has a INET socket end point                        \
                  * file */

#        define EPT_PSXMQ 0x100 /* process has a POSIX MQ file*/
#        define EPT_PSXMQ_END                                                  \
            0x200 /* process has a POSIX MQ end point                          \
                   * file*/

#        if defined(HASIPv6)
#            define EPT_NETS6 0x400 /* process has a INET6 socket file */
#            define EPT_NETS6_END                                              \
                0x800 /* process has a INET6 socket end point                  \
                       * file */
#        endif        /* defined(HASIPv6) */

#        define EPT_EVTFD 0x1000 /* process has a eventfd file*/
#        define EPT_EVTFD_END                                                  \
            0x2000 /* process has a eventfd end point                          \
                    * file*/
#    endif         /* defined(HASEPTOPTS) */

/*
 * FILE_FLAG column names
 */

#    define FF_AIO "AIO"
#    define FF_APPEND "AP"
#    define FF_ASYNC "ASYN"
#    define FF_BLKANDSET "BAS"
#    define FF_BLKINUSE "BKIU"
#    define FF_BLKSEEK "BSK"
#    define FF_CIO "CIO"
#    define FF_CLONE "CLON"
#    define FF_CLREAD "CLRD"
#    define FF_COPYAVOID "CA"
#    define FF_CREAT "CR"
#    define FF_DATAFLUSH "DFLU"
#    define FF_DEFER "DF"
#    define FF_DEFERIND "DFI"
#    define FF_DELAY "DLY"
#    define FF_DIRECT "DIR"
#    define FF_DIRECTORY "DTY"
#    define FF_DOCLONE "DOCL"
#    define FF_DSYNC "DSYN"
#    define FF_EVTONLY "EVO"
#    define FF_EXCL "EXCL"
#    define FF_EXEC "EX"
#    define FF_EXLOCK "XL"
#    define FF_FILE_MBLK "MBLK"
#    define FF_FSYNC "FSYN"
#    define FF_GCFDEFER "GCDF"
#    define FF_GCFMARK "GCMK"
#    define FF_GENTTY "GTTY"
#    define FF_HASLOCK "LCK"
#    define FF_HUP "HUP"
#    define FF_KERNEL "KERN"
#    define FF_KIOCTL "KIOC"
#    define FF_LARGEFILE "LG"
#    define FF_MARK "MK"
#    define FF_MOUNT "MNT"
#    define FF_MSYNC "MSYN"
#    define FF_NBDRM "NBDR"
#    define FF_NBIO "NBIO"
#    define FF_NBLOCK "NB"
#    define FF_NBUF "NBF"
#    define FF_NMFS "NMFS"
#    define FF_NDELAY "ND"
#    define FF_NET "NET"
#    define FF_NOATM "NATM"
#    define FF_NOCACHE "NC"
#    define FF_NOCTTY "NTTY"
#    define FF_NODSYNC "NDSY"
#    define FF_NOFOLNK "NFLK"
#    define FF_NOTOSTOP "NOTO"
#    define FF_NSHARE "NSH"
#    define FF_OLRMIRROR "OLRM"
#    define FF_PATH "PATH"
#    define FF_POSIX_AIO "PAIO"
#    define FF_POSIX_PIPE "PP"
#    define FF_RAIOSIG "RAIO"
#    define FF_RCACH "RC"
#    define FF_RDWR "RW"
#    define FF_READ "R"
#    define FF_REVOKED "REV"
#    define FF_RSHARE "RSH"
#    define FF_RSYNC "RSYN"
#    define FF_SETBLK "BL"
#    define FF_SHLOCK "SL"
#    define FF_SNAP "SNAP"
#    define FF_SOCKET "SOCK"
#    define FF_SQTSH1 "SQS1"
#    define FF_SQTSH2 "SQS2"
#    define FF_SQTREPAIR "SQR"
#    define FF_SQTSH "SQSH"
#    define FF_SQTSVM "SQSV"
#    define FF_STOPIO "STPI"
#    define FF_SYNC "SYN"
#    define FF_SYNCRON "SWR"
#    define FF_TCP_MDEVONLY "TCPM"
#    define FF_TERMIO "TIO"
#    define FF_TMPFILE "TMPF"
#    define FF_TRUNC "TR"
#    define FF_VHANGUP "VH"
#    define FF_VTEXT "VTXT"
#    define FF_WAKEUP "WKUP"
#    define FF_WAITING "WTG"
#    define FF_WRITE "W"

/*
 * Process open file flag names
 */

#    define POF_ALLOCATED "ALLC"
#    define POF_BNRD "BR"
#    define POF_BNWR "BW"
#    define POF_BNHUP "BHUP"
#    define POF_CLOEXEC "CX"
#    define POF_CLOSING "CLSG"
#    define POF_FDLOCK "LCK"
#    define POF_INUSE "USE"
#    define POF_MAPPED "MP"
#    define POF_FSHMAT "SHMT"
#    define POF_RESERVED "OPIP"
#    define POF_RSVWT "RSVW"

/*
 * Cross-over (-x) option values
 */

#    define XO_FILESYS 0x1 /* file system mount points */
#    define XO_SYMLINK 0x2 /* symbolic links */
#    define XO_ALL (XO_FILESYS | XO_SYMLINK)

#    include "dlsof.h"

#    include <sys/types.h> /* just in case -- because utmp.h
                            * may need it */
#    include <regex.h>

#    if defined(EMPTY)
#        undef EMPTY
#    endif /* defined(EMPTY) */

#    if defined(HASUTMPX)
#        include <utmpx.h>
#    else /* !defined(HASUTMPX) */
#        include <utmp.h>
#    endif /* defined(HASUTMPX) */

extern int errno;
extern char *optarg;
extern int optind;

#    define ACCESSERRFMT "%s: WARNING: access %s: %s\n"

#    if defined(HASDCACHE)
#        define CRC_POLY 0120001 /* CRC-16 polynomial */
#        define CRC_TBLL 256     /* crc table length for software */
#        define CRC_BITS 8       /* number of bits contributing */
#    endif                       /* defined(HASDCACHE) */
#    define CMDL                                                               \
        9              /* maximum number of characters from                    \
                        * command name to print in COMMAND                     \
                        * column */
#    define CWD " cwd" /* current working directory fd name */
#    define FDLEN 8    /* fd printing array length */
#    define FSV_FA 0x1 /* file struct addr status */
#    define FSV_CT 0x2 /* file struct count status */
#    define FSV_FG 0x4 /* file struct flags */
#    define FSV_NI 0x8 /* file struct node ID status */

#    if !defined(GET_MAJ_DEV)
#        define GET_MAJ_DEV                                                    \
            major /* if no dialect specific macro has                          \
                   * been defined, use standard major()                        \
                   * macro */
#    endif        /* !defined(GET_MAJ_DEV) */

#    if !defined(GET_MIN_DEV)
#        define GET_MIN_DEV                                                    \
            minor /* if no dialect specific macro has                          \
                   * been defined, use standard minor()                        \
                   * macro */
#    endif        /* !defined(GET_MIN_DEV) */

#    if defined(HASSELINUX)
#        define HASHCNTX                                                       \
            128 /* security context hash bucket count                          \
                 * -- MUST BE A POWER OF 2!!! */
#    endif      /* defined(HASSELINUX) */

#    if defined(HASZONES)
#        define HASHZONE                                                       \
            128 /* zone hash bucket count -- MUST BE                           \
                 * A POWER OF 2!!! */
#    endif      /* defined(HASZONES) */

#    define IDINCR 10 /* PID/PGID table malloc() increment */

#    if !defined(INADDR_LOOPBACK)
#        define INADDR_LOOPBACK (u_long)0x7f000001
#    endif /* !defined(INADDR_LOOPBACK) */

#    define IPROTOL 8 /* Internet protocol length */

#    if !defined(KA_T_FMT_X)
#        define KA_T_FMT_X                                                     \
            "0x%08lx" /* format for printing kernel                            \
                       * addresses in 0x... format */
#    endif            /* !defined(KA_T_FMT_X) */

#    if !defined(LOGINML)
#        if defined(HASUTMPX)
#            define LOGINML sizeof(((struct utmpx *)0)->ut_user)
/* login name length */
#        else /* !defined(HASUTMPX) */
#            define LOGINML sizeof(((struct utmp *)0)->ut_name)
/* login name length */
#        endif /* defined(HASUTMPX) */
#    endif     /* !defined(LOGINML) */

#    define LPROCINCR 128 /* Lproc[] allocation increment */
#    define LSOF_GITHUB_URL "https://github.com/lsof-org"
#    define LSOF_REPO "lsof"
#    define LSOF_BRANCH "master"
#    define LSOF_REPO_URL LSOF_GITHUB_URL "/" LSOF_REPO
#    define LSOF_FAQ_URL LSOF_REPO_URL "/blob/" LSOF_BRANCH "/00FAQ"
#    define LSOF_MAN_URL LSOF_REPO_URL "/blob/" LSOF_BRANCH "/Lsof.8"
#    define MIN_AF_ADDR sizeof(struct in_addr)
/* minimum AF_* address length */

#    if defined(HASIPv6)
#        define MAX_AF_ADDR sizeof(struct in6_addr)
/* maximum AF_* address length */
#    else                               /* !defined(HASIPv6) */
#        define MAX_AF_ADDR MIN_AF_ADDR /* maximum AF_* address length */
#    endif                              /* defined(HASIPv6) */

#    define MAXDCPATH 4 /* paths in DCpath[] */
#    define MAXNWAD 100 /* maximum network addresses */

#    if !defined(MEMMOVE)
#        define MEMMOVE memmove
#    endif /* !defined*MEMMOVE) */

#    define N_REGLR 0      /* regular file system node */
#    define N_AFS 1        /* AFS node */
#    define N_AFPFS 2      /* Apple Darwin AppleShare */
#    define N_ANON_INODE 3 /* Linux anon_inodefs node */
#    define N_AUSX 4       /* Auspex LFS node */
#    define N_AUTO 5       /* automount node */
#    define N_BLK 6        /* block device node */
#    define N_CACHE 7      /* cached file system node */
#    define N_CDFS 8       /* CD-ROM node */
#    define N_CFS 9        /* CFS node */
#    define N_CHR 10       /* character device node */
#    define N_COM 11       /* streams common device node */
#    define N_CTFSADIR 12  /* Solaris CTFS adir node */
#    define N_CTFSBUND 13  /* Solaris CTFS bundle node */
#    define N_CTFSCDIR 14  /* Solaris CTFS cdir node */
#    define N_CTFSCTL 15   /* Solaris CTFS ctl node */
#    define N_CTFSEVT 16   /* Solaris CTFS events node */
#    define N_CTFSLATE 17  /* Solaris CTFS latest node */
#    define N_CTFSROOT 18  /* Solaris CTFS root node */
#    define N_CTFSSTAT 19  /* Solaris CTFS status node */
#    define N_CTFSSYM 20   /* Solaris CTFS symbolic node */
#    define N_CTFSTDIR 21  /* Solaris CTFS type node */
#    define N_CTFSTMPL 22  /* Solaris CTFS template node */
#    define N_DEV 23       /* DEV FS node */
#    define N_DOOR 24      /* DOOR node */
#    define N_FD 25        /* FD node */
#    define N_FIFO 26      /* FIFO node */
#    define N_HSFS 27      /* High Sierra node */
#    define N_KERN 28      /* BSD /kern node */
#    define N_LOFS 29      /* loopback node */
#    define N_MNT 30       /* mount file system device node */
#    define N_MPC 31       /* multiplexed device node */
#    define N_MVFS 32      /* multi-volume file system node (?) */
#    define N_NFS 33       /* NFS node */
#    define N_NFS4 34      /* NFS version 4 node */
#    define N_NM 35        /* named file system node */
#    define N_OBJF 36      /* objfs file system node */
#    define N_PCFS 37      /* PC file system node */
#    define N_PIPE 38      /* pipe device node */
#    define N_PORT 39      /* port node */
#    define N_PROC 40      /* /proc node */
#    define N_PSEU 41      /* pseudofs node */
#    define N_SAMFS 42     /* Solaris SAM-FS */
#    define N_SANFS 43     /* AIX SANFS */
#    define N_SDEV 44      /* Solaris sdev file system node */
#    define N_SHARED 45    /* Solaris sharedfs */
#    define N_SOCK 46      /* sock_vnodeops node */
#    define N_SPEC 47      /* spec_vnodeops node */
#    define N_STREAM 48    /* stream node */
#    define N_TMP 49       /* tmpfs node */
#    define N_UFS 50       /* UNIX file system node */
#    define N_UNKN 51      /* unknown node type */
#    define N_VXFS 52      /* Veritas file system node */
#    define N_XFS 53       /* XFS node */
#    define N_ZFS 54       /* ZFS node */
#    define N_MQUEUE 55    /* Posix mqueue node on Linux */

#    if !defined(OFFDECDIG)
#        define OFFDECDIG                                                      \
            8 /* maximum number of digits in the                               \
               * offset decimal form (0t...) */
#    endif    /* !defined(OFFDECDIG) */

#    if !defined(USELOCALREADDIR)
#        define CloseDir closedir /* use standard closedir() */
#        define OpenDir opendir   /* use standard opendir() */
#        define ReadDir readdir   /* use standard readdir() */
#    endif                        /* !defined(USELOCALREADDIR) */

#    define RPTTM 15   /* default repeat seconds */
#    define RTD " rtd" /* root directory fd name */
#    define TASKCMDL                                                           \
        9 /* maximum number of characters from                                 \
           * command name to print in TASKCMD                                  \
           * column */
#    define TCPTPI_FLAGS                                                       \
        0x0001                    /* report TCP/TPI socket options and         \
                                   * state, and TCP_NODELAY state */
#    define TCPTPI_QUEUES 0x0002  /* report TCP/TPI queue lengths */
#    define TCPTPI_STATE 0x0004   /* report TCP/TPI state */
#    define TCPTPI_WINDOWS 0x0008 /* report TCP/TPI window sizes */
#    define TCPTPI_ALL (TCPTPI_QUEUES | TCPTPI_STATE | TCPTPI_WINDOWS)
/* report all TCP/TPI info */
#    define TCPUDPALLOC                                                        \
        32                 /* allocation amount for TCP and UDP                \
                            * state tables */
#    define TMLIMIT 15     /* readlink() & stat() timeout sec */
#    define TMLIMMIN 2     /* minimum timeout */
#    define TYPEL 8        /* type character length */
#    define UIDCACHEL 1024 /* UID cache length */
#    define UIDINCR 10     /* UID table malloc() increment */
#    define USERPRTL 8     /* UID/login print length limit */

#    if !defined(SZOFFTYPE)
#        define SZOFFTYPE unsigned long /* type for size and offset */
#        undef SZOFFPSPEC
#        define SZOFFPSPEC                                                     \
            "l" /* SZOFFTYPE printf specification                              \
                 * modifier */
#    endif      /* !defined(SZOFFTYPE) */

#    if !defined(TIMEVAL_LSOF)
#        define TIMEVAL_LSOF timeval
#    endif /* !defined(TIMEVAL_LSOF) */

#    if !defined(XDR_PMAPLIST)
#        define XDR_PMAPLIST xdr_pmaplist
#    endif /* !defined(XDR_PMAPLIST) */

#    if !defined(XDR_VOID)
#        define XDR_VOID xdr_void
#    endif /* !defined(XDR_VOID) */

/*
 * Output title definitions
 */

#    define CMDTTL "COMMAND"
extern int CmdColW;
#    define CNTXTTL "SECURITY-CONTEXT"
extern int CntxColW;
#    define DEVTTL "DEVICE"
extern int DevColW;
#    define FCTTL "FCT"
extern int FcColW;
#    define FDTTL "FD"
extern int FdColW;
#    define FGTTL "FILE-FLAG"
extern int FgColW;
#    define FSTTL "FILE-ADDR"
extern int FsColW;
#    define NITTL "NODE-ID"
extern int NiColW;
extern char *NiTtl;
#    define NLTTL "NLINK"
extern int NlColW;
#    define NMTTL "NAME"
extern int NmColW;
#    define NODETTL "NODE"
extern int NodeColW;
#    define OFFTTL "OFFSET"
#    define PGIDTTL "PGID"
extern int PgidColW;
#    define PIDTTL "PID"
extern int PidColW;
#    define PPIDTTL "PPID"
extern int PpidColW;
#    define SZTTL "SIZE"
#    define SZOFFTTL "SIZE/OFF"
extern int SzOffColW;
#    define TASKCMDTTL "TASKCMD"
extern int TaskCmdColW;
#    define TASKTIDTTL "TID"
extern int TaskTidColW;
#    define TYPETTL "TYPE"
extern int TypeColW;
#    define USERTTL "USER"
extern int UserColW;
#    define ZONETTL "ZONE"
extern int ZoneColW;

/*
 * Selection flags
 */

#    define PS_PRI                                                             \
        1 /* primary process selection -- e.g.,                                \
           * by PID or UID */
#    define PS_SEC                                                             \
        2                   /* secondary process selection -- e.g.,            \
                             * by directory or file */
#    define SELCMD 0x0001   /* select process by command name */
#    define SELCNTX 0x0002  /* select security context (-Z) */
#    define SELFD 0x0004    /* select file by descriptor name */
#    define SELNA 0x0008    /* select socket by address (-i@...) */
#    define SELNET 0x0010   /* select Internet socket files (-i) */
#    define SELNFS 0x0020   /* select NFS files (-N) */
#    define SELNLINK 0x0040 /* select based on link count */
#    define SELNM 0x0080    /* select by name */
#    define SELPGID 0x0100  /* select process group IDs (-g) */
#    define SELPID 0x0200   /* select PIDs (-p) */
#    define SELUID 0x0400   /* select UIDs (-u) */
#    define SELUNX 0x0800   /* select UNIX socket (-U) */
#    define SELZONE 0x1000  /* select zone (-z) */
#    define SELEXCLF 0x2000 /* file selection excluded */
#    define SELTASK 0x4000  /* select tasks (-K) */
#    define SELPINFO                                                           \
        0x8000 /* selected for pipe info (cleared in                           \
                * link_lfile() */
#    define SELUXSINFO                                                         \
        0x10000 /* selected for UNIX socket info;                              \
                 * cleared in link_lfile() */
#    define SELPTYINFO                                                         \
        0x20000 /* selected for pseudoterminal info;                           \
                 * cleared in link_lfile() */
#    define SELNETSINFO                                                        \
        0x40000 /* selected for INET socket info;                              \
                 * cleared in link_lfile() */
#    define SELPSXMQINFO                                                       \
        0x80000 /* selected for POSIX MQ socket info;                          \
    cleared in link_lfile() */
#    define SELNETS6INFO                                                       \
        0x100000 /* selected for INET6 socket info;                            \
                  * cleared in link_lfile() */
#    define SELEVTFDINFO                                                       \
        0x200000 /* selected for evetnfd info;                                 \
                  * cleared in link_lfile() */

#    define SELALL                                                             \
        (SELCMD | SELCNTX | SELFD | SELNA | SELNET | SELNM | SELNFS | SELPID | \
         SELUID | SELUNX | SELZONE | SELTASK)
#    define SELPROC                                                            \
        (SELCMD | SELCNTX | SELPGID | SELPID | SELUID | SELZONE | SELTASK)
/* process selecters */
#    define SELFILE (SELFD | SELNFS | SELNLINK | SELNM) /* file selecters */
#    define SELNW (SELNA | SELNET | SELUNX)             /* network selecters */

/*
 * Exit Status
 */

enum ExitStatus {
    LSOF_EXIT_SUCCESS,
    LSOF_EXIT_ERROR,
};
#    define LSOF_SEARCH_FAILURE                                                \
        (FsearchErr ? LSOF_EXIT_ERROR : LSOF_EXIT_SUCCESS)

/*
 * Structure definitions
 */

#    if defined(HAS_AFS)
struct afsnode { /* AFS pseudo-node structure */
    dev_t dev;
    unsigned char ino_st;   /* 1 if inode has a value */
    unsigned char nlink_st; /* 1 if nlink has a value */
    INODETYPE inode;
    unsigned long size;
    long nlink;
};
#    endif /* defined(HAS_AFS) */

#    if defined(HAS_STD_CLONE)
struct clone {
    int dx;             /* index of device entry in Devtp[] */
    struct clone *next; /* forward link */
};
extern struct clone *Clone;
#    endif /* defined(HAS_STD_CLONE) */

#    if defined(HASNLIST)
struct drive_Nl { /* data to drive build_Nl() */
    char *nn;     /* nickname for lookups */
    char *knm;    /* kernel variable for name list */
};
extern struct drive_Nl Drive_Nl[]; /* defined in dstore.c */
#    endif                         /* defined(HASNLIST) */

/*
 * Global storage definitions (including their structure definitions)
 */

typedef struct efsys_list {
    char *path;              /* path to file system for which kernel
                              * blocks are to be eliminated */
    int pathl;               /* path length */
    int rdlnk;               /* avoid readlink(2) if non-zero */
    struct mounts *mp;       /* local mount table entry pointer */
    struct efsys_list *next; /* next efsys_list entry pointer */
} efsys_list_t;

struct int_lst {
    int i; /* integer argument */
    int f; /* find state -- meaningful only if
            * x == 0 */
    int x; /* excluded state */
};

typedef struct lsof_rx { /* regular expression table entry */
    char *exp;           /* original regular expression */
    regex_t cx;          /* compiled expression */
    int mc;              /* match count */
} lsof_rx_t;

#    if defined(HASFSTRUCT)
struct pff_tab { /* print file flags table structure */
    long val;    /* flag value */
    char *nm;    /* name to print for flag */
};
#    endif /* defined(HASFSTRUCT) */

struct seluid {
    uid_t uid;          /* User ID */
    char *lnm;          /* specified login name (NULL = none) */
    unsigned char excl; /* excluded state */
    unsigned char f;    /* selected User ID find state
                         * (meaningful only if excl == 0) */
};

extern int CkPasswd;

struct str_lst {
    char *str;            /* string */
    int len;              /* string length */
    short f;              /* selected string find state */
    short x;              /* exclusion (if non-zero) */
    struct str_lst *next; /* next list entry */
};

typedef struct cntxlist {
    char *cntx;            /* zone name */
    int f;                 /* "find" flag (used only in CntxArg) */
    struct cntxlist *next; /* next zone hash entry */
} cntxlist_t;
extern int CntxStatus;

extern int DChelp;
extern int ErrStat;
extern uid_t Euid;
extern int Fcntx;
extern int Ffield;
extern int Ffilesys;
extern int Fhelp;
extern int Fhost;

#    if defined(HASNCACHE)
extern int NcacheReload;
#    endif /* defined(HASNCACHE) */

extern int Fnlink;
extern int Fport;

#    if !defined(HASNORPC_H)
extern int FportMap;
#    endif /* !defined(HASNORPC_H) */

extern int Fpgid;
extern int Fppid;
extern int FsearchErr;
extern int Fhuman;
extern int Fsv;
extern int Ftcptpi;
extern int Fterse;
extern int Funix;
extern int Futol;
extern int Fverbose;

extern int Fxover;
extern int Fzone;

struct fd_lst {
    enum lsof_fd_type fd_type; /* file descriptor type;
                                * range if LSOF_FD_NUMERIC */
    int lo;                    /* range start (if nm NULL) */
    int hi;                    /* range end (if nm NULL) */
    struct fd_lst *next;
};

struct fieldsel {
    char id;          /* field ID character */
    unsigned char st; /* field status */
    char *nm;         /* field name */
    int *opt;         /* option variable address */
    int ov;           /* value to OR with option variable */
};
extern struct fieldsel FieldSel[];

extern int Hdr;

enum IDType { PGID, PID };
extern char *InodeFmt_d;
extern char *InodeFmt_x;
extern int LastPid;

struct lfile {
    enum lsof_file_access_mode access;
    enum lsof_lock_mode lock;
    unsigned char dev_def;   /* device number definition status */
    unsigned char inp_ty;    /* inode/iproto type
                              * 0: neither inode nor iproto
                              * 1: print inode in decimal
                              * 2: iproto contains string
                              * 3: print inode in hex
                              */
    unsigned char is_com;    /* common stream status */
    unsigned char is_nfs;    /* NFS file status */
    unsigned char is_stream; /* stream device status */

#    if defined(HASVXFS) && defined(HASVXFSDNLC)
    unsigned char is_vxfs; /* VxFS file status */
#    endif                 /* defined(HASVXFS) && defined(HASVXFSDNLC) */

    unsigned char lmi_srch; /* local mount info search status:
                             * 1 = printname() search required */

#    if defined(HASMNTSTAT)
    unsigned char mnt_stat; /* mount point stat(2) status */
#    endif                  /* defined(HASMNTSTAT) */

    unsigned char nlink_def; /* link count definition status */
    unsigned char off_def;   /* offset definition status */

#    if defined(HASEPTOPTS)
    unsigned char chend; /* communication channel endpoint
                          * file */
    int eventfd_id;      /* evntfd id taken from
                    /proc/$pid/fdinfo */
#        if defined(HASPTYEPT)
    int tty_index; /* pseudoterminal index of slave side
                    * (if this is the master side) */
#        endif     /* defined(HASPTYEPT) */
#    endif         /* defined(HASEPTOPTS) */

    unsigned char rdev_def; /* rdev definition status */
    unsigned char sz_def;   /* size definition status */

#    if defined(HASFSTRUCT)
    unsigned char fsv; /* file struct value status */
#    endif             /* defined(HASFSTRUCT) */

    /* FD column */
    enum lsof_fd_type fd_type;
    int fd_num; /* stores fd number when fd_type is LSOF_FD_NUMERIC,
                   stores raw number when fd_type is one of
                   {LSOF_FD_LIBRARY_REF, LSOF_FD_MMAP_UNKNOWN,
                   LSOF_FD_PREGION_UNKNOWN}, otherwise -1 */

    char iproto[IPROTOL];
    enum lsof_file_type type;
    uint32_t unknown_file_type_number; /* store file type when type ==
                                      LSOF_FILE_UNKNOWN_RAW */

    unsigned int sf; /* select flags -- SEL* symbols */
    int ch;          /* VMPC channel: -1 = none */
    int ntype;       /* node type -- N_* value */
    SZOFFTYPE off;
    SZOFFTYPE sz;
    dev_t dev;
    dev_t rdev;
    INODETYPE inode;
    long nlink; /* link count */
    char *dev_ch;
    char *fsdir; /* file system directory */
    char *fsdev; /* file system device */

#    if defined(HASFSINO)
    INODETYPE fs_ino; /* file system inode number */
#    endif            /* defined HASFSINO) */

    struct linaddr { /* local Internet address information */
        int af;      /* address family: 0 for none; AF_INET;
                      * or AF_INET6 */
        int p;       /* port */
        union {
            struct in_addr a4; /* AF_INET Internet address */

#    if defined(HASIPv6)
            struct in6_addr a6; /* AF_INET6 Internet address */
#    endif                      /* defined(HASIPv6) */

        } ia;
    } li[2];         /* li[0]: local
                      * li[1]: foreign */
    struct ltstate { /* local TCP/TPI state */
        int type;    /* state type:
                      *   -1 == none
                      *    0 == TCP
                      *    1 == TPI or socket (SS_*) */
        union {
            int i;           /* integer state */
            unsigned int ui; /* unsigned integer state */
        } state;

#    if defined(HASSOOPT)
        unsigned char pqlens; /* pqlen status: 0 = none */
        unsigned char qlens;  /* qlen status:  0 = none */
        unsigned char qlims;  /* qlim status:  0 = none */
        unsigned char rbszs;  /* rbsz status:  0 = none */
        unsigned char sbszs;  /* sbsz status:  0 = none */
        int kai;              /* TCP keep-alive interval */
        int ltm;              /* TCP linger time */
        unsigned int opt;     /* socket options */
        unsigned int pqlen;   /* partial connection queue length */
        unsigned int qlen;    /* connection queue length */
        unsigned int qlim;    /* connection queue limit */
        unsigned long rbsz;   /* receive buffer size */
        unsigned long sbsz;   /* send buffer size */
#    endif                    /* defined(HASSOOPT) */

#    if defined(HASSOSTATE)
        unsigned int ss; /* socket state */
#        if defined(HASSBSTATE)
        unsigned int sbs_rcv; /* receive socket buffer state */
        unsigned int sbs_snd; /* send socket buffer state */
#        endif                /* defined(HASSBSTATE) */
#    endif                    /* defined(HASSOSTATE) */

#    if defined(HASTCPOPT)
        unsigned int topt;  /* TCP options */
        unsigned char msss; /* mss status: 0 = none */
        unsigned long mss;  /* TCP maximum segment size */
#    endif                  /* defined(HASTCPOPT) */

#    if defined(HASTCPTPIQ)
        unsigned long rq;  /* receive queue length */
        unsigned long sq;  /* send queue length */
        unsigned char rqs; /* rq status: 0 = none */
        unsigned char sqs; /* sq status: 0 = none */
#    endif                 /* defined(HASTCPTPIQ) */

#    if defined(HASTCPTPIW)
        unsigned char rws; /* rw status: 0 = none */
        unsigned char wws; /* ww status: 0 = none */
        unsigned long rw;  /* read window size */
        unsigned long ww;  /* write window size */
#    endif                 /* defined(HASTCPTPIW) */

    } lts;
    char *nm;
    char *nma; /* NAME column addition */

#    if defined(HASNCACHE) && HASNCACHE < 2
    KA_T na; /* file structure's node address */
#    endif   /* defined(HASNCACHE) && HASNCACHE<2 */

#    if defined(HASNCACHE) && defined(HASNCVPID)
    unsigned long id; /* capability ID */
#    endif            /* defined(HASNCACHE) && defined(HASNCVPID) */

#    if defined(HASLFILEADD)
    HASLFILEADD
#    endif /* defined(HASLFILEADD) */

#    if defined(HASFSTRUCT)
    KA_T fsa; /* file structure address */
    long fct; /* file structure's f_count */
    long ffg; /* file structure's f_flag */
    long pof; /* process open-file flags */
    KA_T fna; /* file structure node address */
#    endif    /* defined(HASFSTRUCT) */

    struct lfile *next;
};

struct lproc {
    char *cmd; /* command name */

#    if defined(HASSELINUX)
    char *cntx; /* security context */
#    endif      /* defined(HASSELINUX) */

    short sf;  /* select flags -- SEL* symbols */
    short pss; /* state: 0 = not selected
                *	  1 = wholly selected
                *	  2 = partially selected */
#    if defined(HASEPTOPTS)
    short ept; /* end point status -- EPT_* values */
#    endif     /* defined(HASEPTOPTS) */

    int pid; /* process ID */

#    if defined(HASTASKS)
    int tid;    /* task ID */
    char *tcmd; /* task command name */
#    endif      /* HASTASKS */

    int pgid;  /* process group ID */
    int ppid;  /* parent process ID */
    uid_t uid; /* user ID */

#    if defined(HASZONES)
    char *zn; /* zone name */
#    endif    /* defined(HASZONES) */

    struct lfile *file; /* open files of process */
};

extern char *Memory;

#    if defined(HASPROCFS)
extern struct mounts *Mtprocfs;
#    endif

#    if defined(HASNLIST)
#        if !defined(NLIST_TYPE)
#            define NLIST_TYPE nlist
#        endif /* !defined(NLIST_TYPE) */
extern struct NLIST_TYPE *Nl;
extern int Nll;
#    endif /* defined(HASNLIST) */
extern char *Nmlst;

struct nwad {
    char *arg;                    /* argument */
    char *proto;                  /* protocol */
    int af;                       /* address family -- e.g.,
                                   * AF_INET, AF_INET6 */
    unsigned char a[MAX_AF_ADDR]; /* address */
    int sport;                    /* starting port */
    int eport;                    /* ending port */
    int f;                        /* find state */
    struct nwad *next;            /* forward link */
};
extern struct nwad *Nwad;

extern int OffDecDig;

#    if defined(HASFSTRUCT)
extern struct pff_tab Pff_tab[]; /* file flags table */
extern struct pff_tab Pof_tab[]; /* process open file flags table */
#    endif                       /* defined(HASFSTRUCT) */

#    if defined(HASPROCFS)
struct procfsid {
    pid_t pid;       /* search PID */
    char *nm;        /* search name */
    unsigned char f; /* match found if == 1 */

#        if defined(HASPINODEN)
    INODETYPE inode; /* search inode number */
#        endif       /* defined(HASPINODEN) */

    struct procfsid *next; /* forward link */
};

extern int Procfind;
extern struct procfsid *Procfsid;
extern int Procsrch;
#    endif /* defined(HASPROCFS) */

extern int PrPass;
extern int RptMaxCount;
extern char *SzOffFmt_0t;
extern char *SzOffFmt_d;
extern char *SzOffFmt_dv;
extern char *SzOffFmt_x;
extern int TaskCmdLim;
extern int TaskPrtCmd;
extern int TaskPrtTid;
extern int TcpStAlloc;
extern unsigned char *TcpStI;
extern int TcpStIn;
extern int TcpStOff;
extern unsigned char *TcpStX;
extern int TcpStXn;
extern int TcpNstates;
extern char **TcpSt;
extern char Terminator;
extern int UdpStAlloc;
extern unsigned char *UdpStI;
extern int UdpStIn;
extern int UdpStOff;
extern unsigned char *UdpStX;
extern int UdpStXn;
extern int UdpNstates;
extern char **UdpSt;

struct hsfile {
    struct sfile *s;     /* the Sfile table address */
    struct hsfile *next; /* the next hash bucket entry */
};

typedef struct znhash {
    char *zn;            /* zone name */
    int f;               /* "find" flag (used only in ZoneArg) */
    struct znhash *next; /* next zone hash entry */
} znhash_t;

struct lsof_context {
    /** Parameters */
    /** Linked list of files to search */
    struct sfile *select_files;

    /* file systems for which kernel blocks are
     * to be eliminated */
    efsys_list_t *elim_fs_list;

    /* User IDs to include or exclude */
    struct seluid *sel_uid;
    /* -u option count */
    int sel_uid_size;
    /* capacity of sel_uid */
    int sel_uid_cap;
    /* -u option count of UIDs excluded */
    int num_uid_excluded;
    /* -u option count of UIDs included */
    int num_uid_included;

    /* process group IDs to search for */
    struct int_lst *sel_pgid;
    int sel_pgid_size;     /* -g option count */
    int sel_pgid_cap;      /* capacity of sel_pgid */
    int sel_pgid_incl_num; /* -g option inclusion count */
    int sel_pgid_excl_num; /* -g option exclusion count */

    /* Process IDs to search for */
    struct int_lst *sel_pid;
    int sel_pid_size;  /* -p option count */
    int num_unsel_pid; /* number of unselected PIDs (starts at sel_pid_size) for
                          optimization in examine_lproc() */
    int sel_pid_cap;   /* capacity of sel_pid */
    int sel_pid_incl_num; /* -p option inclusion count */
    int sel_pid_excl_num; /* -p option exclusion count */

    /* Whether all processes are selected */
    int sel_all_proc;

    /* command names selected with -c */
    struct str_lst *sel_cmds;
    int sel_cmd_incl; /* number of command name inclusions selected with -c */
    int sel_cmd_excl; /* number of command name exclusions selected with -c */

    /* command regular expression table for -c option */
    lsof_rx_t *cmd_regex;
    int cmd_regex_size; /* number of cmd_regex[] entries */
    int cmd_regex_cap;  /* capacity of cmd_regex[] */

    /* select by network address */
    struct nwad *sel_net_addr;

    /* security context arguments supplied with -Z */
    cntxlist_t *sel_selinux_context;

    /* device cache paths, indexed by DCpathX
     * when it's >= 0 */
    char *dev_cache_paths[4];
    int dev_cache_path_index;    /* device cache path index:
                                  * -1 = path not defined
                                  *  0 = defined via -D
                                  *  1 = defined via HASENVDC
                                  *  2 = defined via HASSYSDC
                                  *  3 = defined via HASPERSDC and
                                  *      HASPERSDCPATH */
    char *dev_cache_path_arg;    /* device cache path from -D[b|r|u]<path> */
    unsigned dev_cache_checksum; /* device cache file checksum */
    int dev_cache_fd;            /* device cache file descriptor */
    FILE *dev_cache_fp;          /* stream pointer for DCfd */
    int dev_cache_rebuilt;       /* an unsafe device cache file has been
                                  * rebuilt */
    int dev_cache_state;         /* device cache state:
                                  * 0 = ignore (-Di)
                                  * 1 = build (-Db[path])
                                  * 2 = read; don't rebuild (-Dr[path])
                                  * 3 = update; read and rebuild if
                                  *     necessary (-Du[path])
                                  */
    int dev_cache_unsafe;        /* device cache file is potentially unsafe,
                                  * (The [cm]time check failed.) */

#    if defined(HASNLIST)
    /* kernel name list */
    struct NLIST_TYPE *name_list;
    int name_list_size;
#    endif                /* defined(HASNLIST) */
    char *name_list_path; /* namelist file path */
    char *core_file_path; /* core file path */

#    if defined(HASPROCFS)
    /* /proc mount entry */
    struct mounts *procfs_mount;
    int procfs_found; /* 1 when searching for an proc file system
                       * file and one was found */
    /* proc file system PID search table */
    struct procfsid *procfs_table;
    /* 1 if searching for any proc file system
     * file */
    int procfs_search;
#    endif /* defined(HASPROCFS) */

    /* name cache */
    int name_cache_enable; /* -C option status */

    /* local mount info */
    struct mounts *local_mount_info;
    int local_mount_info_valid;

    /** hashSfile() buckets */
    /* hash by file (dev,ino) buckets */
    struct hsfile *sfile_hash_file_dev_inode;
    int sfile_hash_file_dev_inode_count;
    /* hash by file raw device buckets */
    struct hsfile *sfile_hash_file_raw_device;
    int sfile_hash_file_raw_device_count;
    /* hash by file system buckets */
    struct hsfile *sfile_hash_file_system;
    int sfile_hash_file_system_count;
    /* hash by name buckets */
    struct hsfile *sfile_hash_name;
    int sfile_hash_name_count;
    /* hash by clone buckets */
    struct hsfile *sfile_hash_clone;
    int sfile_hash_clone_count;

    /* zone arguments supplied with -z */
    znhash_t **sel_zone;

    /* command name limit */
    int cmd_limit;

    /** When frozen, parameters must not be changed */
    uint8_t frozen;

    /* node type (see N_* symbols) */
    int node_type;

    /* device table pointer */
    struct l_dev *dev_table;
    int dev_table_size; /* number of entries in dev_table[] */
    /* pointer to dev_table[] pointers, sorted
     * by device */
    struct l_dev **dev_table_sorted;

    /* block device table pointer */
    struct l_dev *block_dev_table;
    int block_dev_table_size; /* number of entries in block_dev_table[] */
    /* pointer to BDevtp[] pointers, sorted
     * by device */
    struct l_dev **block_dev_table_sorted;

    /* selection flags -- see SEL* macros */
    int sel_flags;
    /* SELPROC flags, modified by IgnTasks */
    int sel_proc;
    /* SELALL flags, modified by IgnTasks */
    int sel_all;
    /* select only Internet socket files */
    int sel_inet;

    /* -N option status: 0==none, 1==find all,
     * 2==some found*/
    int sel_nfs;

    /* -K option value */
    int sel_task;

    /* -a option status */
    int logic_and;

    /* -b option status */
    int avoid_blocking;

    /* -O option status */
    int avoid_forking;

    /* -X option status */
    int x_opt;

    /* -E option status:
     * 0==none,
     * 1==info,
     * 2==info+files */
    int endpoint_status;

    /* ignore tasks when non-zero */
    int ign_tasks;

    /* maximum file descriptors to close */
    int max_fd;

    /* file descriptors selected with -d */
    struct fd_lst *fd_list;
    /* fd_list[] type:
     * -1 == none
     * 0 == include
     * 1 == exclude */
    int fd_list_ty;

    /* mount supplement state:
     * 0 == none
     * 1 == create
     * 2 == read */
    int mnt_sup_state;
    /* mount supplement path -- if MntSup == 2 */
    char *mnt_sup_path;

    /* report nlink values below this number
     * (0 = report all nlink values) */
    long nlink;

    /* Readlink() and stat() timeout (seconds) */
    int time_limit;

    int my_pid;      /* lsof's process ID */
    uid_t my_uid;    /* real UID of this lsof process */
    gid_t my_gid;    /* real GID of this lsof process */
    int setgid;      /* setgid state */
    int setuid_root; /* setuid-root state */

    /* directory stack */
    char **dir_stack;    /* the directory stack */
    int dir_stack_index; /* dir_stack[] index */
    int dir_stack_size;  /* dir_stack[] entries allocated */

    /* allocated (possibly unused) entries in TCP
     * state tables */
    int tcp_state_alloc;
    /* included TCP states */
    unsigned char *tcp_state_incl;
    int tcp_state_incl_num; /* number of entries in tcp_state_incl[] */
    int tcp_state_off;      /* offset for TCP state number to adjust
                             * negative numbers to an index into tcp_states[],
                             * tcp_state_incl[] and tcp_state_excl[] */
    /* excluded TCP states */
    unsigned char *tcp_state_excl;
    int tcp_state_excl_num; /* number of entries in tcp_state_excl[] */
    int tcp_num_states;     /* number of TCP states -- either in
                             * tcpstates[] or tcp_states[] */
    char **tcp_states;      /* local TCP state names, indexed by system
                             * state value */

    /* allocated (possibly unused) entries in UDP
     * state tables */
    int udp_state_alloc;
    /* included UDP states */
    unsigned char *udp_state_incl;
    int udp_state_incl_num; /* number of entries in udp_state_incl[] */
    int udp_state_off;      /* offset for UDP state number to adjust
                             * negative numbers to an index into udp_states[],
                             * udp_state_incl[] and udp_state_excl[] */
    unsigned char *udp_state_excl;
    /* excluded UDP states */
    int udp_state_excl_num; /* number of entries in udp_state_excl[] */
    int udp_num_states;     /* number of UDP states in udp_states[] */
    char **udp_states;      /* local UDP state names, indexed by system
                             * state number */

    int unix_socket; /* -U option status */

    dev_t dev_dev; /* device number of /dev or its equivalent */

    int net;      /* -i option status: 0==none
                   *                   1==find all
                   *                   2==some found */
    int net_type; /* Fnet type request: AF_UNSPEC==all
                   *                    AF_INET==IPv4
                   *                    AF_INET6==IPv6 */

    /* Fsv was set by +f */
    int fsv_set_f;

    /* hex format status for FSV_FG */
    int fsv_hex;

    /* repeat time -- set by -r */
    int repeat_time;

    /* -o option status */
    int show_offset;

    /* -s option status */
    int show_size;

    /** Temporary */
    /* name characters for printing */
    char *name_buf;
    /* sizeof(name_buf) */
    size_t name_buf_size;

    /** Output */
    /** Pointer to current process */
    struct lproc *cur_proc;
    /** Pointer to all processes */
    struct lproc *procs;
    /** Length and capacity of `procs` */
    size_t procs_size;
    size_t procs_cap;

    /** Pointer to current file */
    struct lfile *cur_file;
    /** Pointer to previous file */
    struct lfile *prev_file;

    /** Warnings and errors */
    FILE *err;
    char *program_name;
    int warn; /* 1=suppress warnings */

    /** dialect specific fields, see dlsof.h */
    struct lsof_context_dialect dialect;
};

/** Convenience macros to access context */
/* Local process */
#    define Lp (ctx->cur_proc)
/* All local processes */
#    define Lproc (ctx->procs)
/* Local file */
#    define Lf (ctx->cur_file)
/* Previous local file */
#    define Plf (ctx->prev_file)
/* Length of local processes */
#    define Nlproc (ctx->procs_size)
/* Error output */
#    define Pn (ctx->program_name)
/* Suppress warnings */
#    define Fwarn (ctx->warn)
/* Name buffer */
#    define Namech (ctx->name_buf)
#    define Namechl (ctx->name_buf_size)
/* Selection flags */
#    define SelAll (ctx->sel_all)
#    define Selflags (ctx->sel_flags)
#    define SelProc (ctx->sel_proc)
#    define Selinet (ctx->sel_inet)
/* dev_t of /dev */
#    define DevDev (ctx->dev_dev)
/* TCP states */
#    define TcpNstates (ctx->tcp_num_states)
#    define TcpSt (ctx->tcp_states)
#    define TcpStI (ctx->tcp_state_incl)
#    define TcpStIn (ctx->tcp_state_incl_num)
#    define TcpStX (ctx->tcp_state_excl)
#    define TcpStXn (ctx->tcp_state_excl_num)
#    define TcpStOff (ctx->tcp_state_off)
#    define TcpStAlloc (ctx->tcp_state_alloc)
/* UDP states */
#    define UdpNstates (ctx->udp_num_states)
#    define UdpSt (ctx->udp_states)
#    define UdpStI (ctx->udp_state_incl)
#    define UdpStIn (ctx->udp_state_incl_num)
#    define UdpStX (ctx->udp_state_excl)
#    define UdpStXn (ctx->udp_state_excl_num)
#    define UdpStOff (ctx->udp_state_off)
#    define UdpStAlloc (ctx->udp_state_alloc)
/* select unix socket */
#    define Funix (ctx->unix_socket)
/* select inet socket */
#    define Fnet (ctx->net)
#    define FnetTy (ctx->net_type)
/* select nfs files */
#    define Fnfs (ctx->sel_nfs)
/* -a option */
#    define Fand (ctx->logic_and)
/* -x option */
#    define Fxopt (ctx->x_opt)
/* avoid blocking */
#    define Fblock (ctx->avoid_blocking)
/* avoid forking overhead */
#    define Fovhd (ctx->avoid_forking)
/* endpoint status */
#    define FeptE (ctx->endpoint_status)
/* select tasks */
#    define Ftask (ctx->sel_task)
/* fd list */
#    define Fdl (ctx->fd_list)
#    define FdlTy (ctx->fd_list_ty)
/* ignore tasks */
#    define IgnTasks (ctx->ign_tasks)
/* maximum fd number */
#    define MaxFd (ctx->max_fd)
/* mount supplement */
#    define MntSup (ctx->mnt_sup_state)
#    define MntSupP (ctx->mnt_sup_path)
/* nlink limit */
#    define Nlink (ctx->nlink)
/* Time limit */
#    define TmLimit (ctx->time_limit)
/* number of unselected PIDs */
#    define Npuns (ctx->num_unsel_pid)
/* pid/uid/gid of current process */
#    define Mypid (ctx->my_pid)
#    define Myuid (ctx->my_uid)
#    define Mygid (ctx->my_gid)
/* setgid state */
#    define Setgid (ctx->setgid)
/* setuid-root state */
#    define Setuidroot (ctx->setuid_root)
/* directory stack */
#    define Dstk (ctx->dir_stack)
#    define Dstkx (ctx->dir_stack_index)
#    define Dstkn (ctx->dir_stack_size)
/* selection files */
#    define Sfile (ctx->select_files)
/* fs to eliminate blocking syscalls */
#    define Efsysl (ctx->elim_fs_list)
/* select uid */
#    define Suid (ctx->sel_uid)
#    define Nuid (ctx->sel_uid_size)
#    define Nuidincl (ctx->num_uid_included)
#    define Nuidexcl (ctx->num_uid_excluded)
#    define Mxuid (ctx->sel_uid_cap)
/* select pid */
#    define Spid (ctx->sel_pid)
#    define Npid (ctx->sel_pid_size)
#    define Npidi (ctx->sel_pid_incl_num)
#    define Npidx (ctx->sel_pid_excl_num)
#    define Mxpid (ctx->sel_pid_cap)
/* select pgid */
#    define Spgid (ctx->sel_pgid)
#    define Npgid (ctx->sel_pgid_size)
#    define Npgidi (ctx->sel_pgid_incl_num)
#    define Npgidx (ctx->sel_pgid_excl_num)
#    define Mxpgid (ctx->sel_pgid_cap)
/* select all procs */
#    define AllProc (ctx->sel_all_proc)
/* select command */
#    define Cmdl (ctx->sel_cmds)
#    define Cmdni (ctx->sel_cmd_incl)
#    define Cmdnx (ctx->sel_cmd_excl)
#    define CmdRx (ctx->cmd_regex)
#    define NCmdRxA (ctx->cmd_regex_cap)
#    define NCmdRxU (ctx->cmd_regex_size)
/* select by network address */
#    define Nwad (ctx->sel_net_addr)
/* device table pointer */
#    define Devtp (ctx->dev_table)
#    define Ndev (ctx->dev_table_size)
#    define Sdev (ctx->dev_table_sorted)
/* block device table */
#    define BDevtp (ctx->block_dev_table)
#    define BNdev (ctx->block_dev_table_size)
#    define BSdev (ctx->block_dev_table_sorted)
/* select selinux context */
#    define CntxArg (ctx->sel_selinux_context)
/* device cache */
#    define DCpath (ctx->dev_cache_paths)
#    define DCpathArg (ctx->dev_cache_path_arg)
#    define DCpathX (ctx->dev_cache_path_index)
#    define DCcksum (ctx->dev_cache_checksum)
#    define DCfd (ctx->dev_cache_fd)
#    define DCfs (ctx->dev_cache_fp)
#    define DCrebuilt (ctx->dev_cache_rebuilt)
#    define DCstate (ctx->dev_cache_state)
#    define DCunsafe (ctx->dev_cache_unsafe)
/* name list */
#    define Nl (ctx->name_list)
#    define Nll (ctx->name_list_size)
#    define Nmlst (ctx->name_list_path)
#    define Memory (ctx->core_file_path)
/* procfs */
#    define Mtprocfs (ctx->procfs_mount)
#    define Procsrch (ctx->procfs_search)
#    define Procfsid (ctx->procfs_table)
#    define Procfind (ctx->procfs_found)
/* name cache */
#    define Fncache (ctx->name_cache_enable)
/* local mount info */
#    define Lmi (ctx->local_mount_info)
#    define Lmist (ctx->local_mount_info_valid)
/* hash buckets in hashSfile() */
#    define HbyFdi (ctx->sfile_hash_file_dev_inode)
#    define HbyFdiCt (ctx->sfile_hash_file_dev_inode_count)
#    define HbyFrd (ctx->sfile_hash_file_raw_device)
#    define HbyFrdCt (ctx->sfile_hash_file_raw_device_count)
#    define HbyFsd (ctx->sfile_hash_file_system)
#    define HbyFsdCt (ctx->sfile_hash_file_system_count)
#    define HbyNm (ctx->sfile_hash_name)
#    define HbyNmCt (ctx->sfile_hash_name_count)
#    define HbyCd (ctx->sfile_hash_clone)
#    define HbyCdCt (ctx->sfile_hash_clone_count)
/* solaris zone */
#    define ZoneArg (ctx->sel_zone)
/* command name limit */
#    define CmdLim (ctx->cmd_limit)
/* node type */
#    define Ntype (ctx->node_type)
/* Fsv was set by +f */
#    define FsvByf (ctx->fsv_set_f)
/* hex format status for FSV_FG */
#    define FsvFlagX (ctx->fsv_hex)
/* repeat time */
#    define RptTm (ctx->repeat_time)
/* -o option status */
#    define Foffset (ctx->show_offset)
/* -s option status */
#    define Fsize (ctx->show_size)

/* Utility macro to free if non-null and set the pointer to null */
#    define CLEAN(ptr)                                                         \
        do {                                                                   \
            free(ptr);                                                         \
            ptr = NULL;                                                        \
        } while (0);

#    include "proto.h"
#    include "dproto.h"

#endif /* COMMON_H */
