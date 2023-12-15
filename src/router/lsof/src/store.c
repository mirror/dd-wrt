/*
 * store.c - common global storage for lsof
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

#include "common.h"

/*
 * Global storage definitions
 */

int CkPasswd = 0; /* time to check /etc/passwd for change */

#if defined(HAS_STD_CLONE)
struct clone *Clone = (struct clone *)NULL;
/* clone device list */
#endif /* defined(HAS_STD_CLONE) */

int CmdColW;       /* COMMAND column width */

#if defined(HASSELINUX)
int CntxColW;       /* security context column width */
int CntxStatus = 0; /* security context status: 0 == disabled,
                     * 1 == enabled */
#endif              /* defined(HASSELINUX) */

int DChelp = 0; /* -D? status */

int DevColW; /* DEVICE column width */

/*
 * Externals for a stkdir(), dumbed-down for older AIX compilers.
 */

int ErrStat = 0;  /* path stat() error count */
uid_t Euid;       /* effective UID of this lsof process */
int FcColW;       /* FCT column width */
int Fcntx = 0;    /* -Z option status */
int FdColW;       /* FD column width */
int Ffilesys = 0; /* -f option status:
                   *    0 = paths may be file systems
                   *    1 = paths are just files
                   *    2 = paths must be file systems */

#if defined(HASNCACHE)
int NcacheReload = 1; /* 1 == call ncache_load() */
#endif                /* defined(HASNCACHE) */

int Ffield = 0;  /* -f and -F status */
int FgColW;      /* FILE-FLAG column width */
int Fhelp = 0;   /* -h option status */
int Fhost = 1;   /* -H option status */
int Fnlink = 0;  /* -L option status */
int Fport = 1;   /* -P option status */

#if !defined(HASNORPC_H)
#    if defined(HASPMAPENABLED)
int FportMap = 1; /* +|-M option status */
#    else         /* !defined(HASPMAPENABLED) */
int FportMap = 0; /* +|-M option status */
#    endif        /* defined(HASPMAPENABLED) */
#endif            /* !defined(HASNORPC_H) */

int Fpgid = 0;              /* -g option status */
int Fppid = 0;              /* -R option status */
int Fhuman = 0;             /* -H option status */
int FsColW;                 /* FSTR-ADDR column width */
int Fsv = FSV_DEFAULT;      /* file struct value selections */
int NiColW;                 /* NODE-ID column width */
char *NiTtl = NITTL;        /* NODE-ID column title */
int FsearchErr = 1;         /* -Q option status */
int Ftcptpi = TCPTPI_STATE; /* -T option status */
int Fterse = 0;             /* -t option status */
int Futol = 1;              /* -l option status */
int Fverbose = 0;           /* -V option status */

int Fxover = 0; /* -x option value */
int Fzone = 0;  /* -z option status */

struct fieldsel FieldSel[] = {
    {LSOF_FID_ACCESS, 0, LSOF_FNM_ACCESS, NULL, 0},              /*  0 */
    {LSOF_FID_CMD, 0, LSOF_FNM_CMD, NULL, 0},                    /*  1 */
    {LSOF_FID_CT, 0, LSOF_FNM_CT, &Fsv, FSV_CT},                 /*  2 */
    {LSOF_FID_DEVCH, 0, LSOF_FNM_DEVCH, NULL, 0},                /*  3 */
    {LSOF_FID_DEVN, 0, LSOF_FNM_DEVN, NULL, 0},                  /*  4 */
    {LSOF_FID_FD, 0, LSOF_FNM_FD, NULL, 0},                      /*  5 */
    {LSOF_FID_FA, 0, LSOF_FNM_FA, &Fsv, FSV_FA},                 /*  6 */
    {LSOF_FID_FG, 0, LSOF_FNM_FG, &Fsv, FSV_FG},                 /*  7 */
    {LSOF_FID_INODE, 0, LSOF_FNM_INODE, NULL, 0},                /*  8 */
    {LSOF_FID_NLINK, 0, LSOF_FNM_NLINK, &Fnlink, 1},             /*  9 */
    {LSOF_FID_TID, 0, LSOF_FNM_TID, NULL, 0},                    /* 11 */
    {LSOF_FID_LOCK, 0, LSOF_FNM_LOCK, NULL, 0},                  /* 11 */
    {LSOF_FID_LOGIN, 0, LSOF_FNM_LOGIN, NULL, 0},                /* 12 */
    {LSOF_FID_MARK, 1, LSOF_FNM_MARK, NULL, 0},                  /* 13 */
    {LSOF_FID_TCMD, 0, LSOF_FNM_TCMD, NULL, 0},                  /* 14 */
    {LSOF_FID_NAME, 0, LSOF_FNM_NAME, NULL, 0},                  /* 15 */
    {LSOF_FID_NI, 0, LSOF_FNM_NI, &Fsv, FSV_NI},                 /* 16 */
    {LSOF_FID_OFFSET, 0, LSOF_FNM_OFFSET, NULL, 0},              /* 17 */
    {LSOF_FID_PID, 1, LSOF_FNM_PID, NULL, 0},                    /* 18 */
    {LSOF_FID_PGID, 0, LSOF_FNM_PGID, &Fpgid, 1},                /* 19 */
    {LSOF_FID_PROTO, 0, LSOF_FNM_PROTO, NULL, 0},                /* 20 */
    {LSOF_FID_RDEV, 0, LSOF_FNM_RDEV, NULL, 0},                  /* 21 */
    {LSOF_FID_PPID, 0, LSOF_FNM_PPID, &Fppid, 1},                /* 22 */
    {LSOF_FID_SIZE, 0, LSOF_FNM_SIZE, NULL, 0},                  /* 23 */
    {LSOF_FID_STREAM, 0, LSOF_FNM_STREAM, NULL, 0},              /* 24 */
    {LSOF_FID_TYPE, 0, LSOF_FNM_TYPE, NULL, 0},                  /* 25 */
    {LSOF_FID_TCPTPI, 0, LSOF_FNM_TCPTPI, &Ftcptpi, TCPTPI_ALL}, /* 26 */
    {LSOF_FID_UID, 0, LSOF_FNM_UID, NULL, 0},                    /* 27 */
    {LSOF_FID_ZONE, 0, LSOF_FNM_ZONE, &Fzone, 1},                /* 28 */
    {LSOF_FID_CNTX, 0, LSOF_FNM_CNTX, &Fcntx, 1},                /* 29 */
    {LSOF_FID_TERM, 0, LSOF_FNM_TERM, NULL, 0},                  /* 30 */
    {' ', 0, NULL, NULL, 0}};

int Hdr = 0; /* header print status */
char *InodeFmt_d = (char *)NULL;
/* INODETYPE decimal printf specification */
char *InodeFmt_x = (char *)NULL;
/* INODETYPE hexadecimal printf specification */
int LastPid = -1; /* last PID listed (for eliminating duplicates
                   * in terse output) */

int NlColW;                /* NLINK column width */
int NmColW;                /* NAME column width */
int NodeColW;              /* NODE column width */
int OffDecDig = OFFDECDIG; /* offset decimal form (0t...) digit limit */
int OffColW;               /* OFFSET column width */
int PgidColW;              /* PGID column width */
int PidColW;               /* PID column width */
int PpidColW;              /* PPID column width */

int PrPass = 0;                   /* print pass: 0 = compute column widths
                                   *	       1 = print */
int RptMaxCount = 0;              /* count of repeasts: 0 = no limit
                                   * -- set by -r */
int SzColW;                       /* SIZE column width */
int SzOffColW;                    /* SIZE/OFF column width */
char *SzOffFmt_0t = (char *)NULL; /* SZOFFTYPE 0t%u printf specification */
char *SzOffFmt_d = (char *)NULL;  /* SZOFFTYPE %d printf specification */
char *SzOffFmt_dv = (char *)NULL; /* SZOFFTYPE %*d printf specification */
char *SzOffFmt_x = (char *)NULL;  /* SZOFFTYPE %#x printf specification */
int TaskCmdColW = 0;              /* task command column width */
int TaskCmdLim = TASKCMDL;        /* TASKCMD column width limit (same as
                                   * CmdLim) */
int TaskPrtCmd = 0;               /* task print task command flag */
int TaskPrtTid = 0;               /* task print TID flag */
char Terminator = '\n';           /* output field terminator */
int TaskTidColW = 0;              /* task TID column width */
int TypeColW;                     /* TYPE column width */
int UserColW;                     /* USER column width */

int ZoneColW; /* ZONE column width */
