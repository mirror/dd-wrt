/*
 * dproto.h - SCO OpenServer function prototypes for lsof
 *
 * The _PROTOTYPE macro is defined in the common proto.h.
 */


/*
 * Copyright 1995 Purdue Research Foundation, West Lafayette, Indiana
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
 * $Id: dproto.h,v 1.5 99/06/22 08:22:28 abe Exp $
 */

_PROTOTYPE(extern int is_file_named,(char *p, int cd));
_PROTOTYPE(extern void process_socket,(struct inode *i));
_PROTOTYPE(extern int get_max_fd,(void));

#if	OSRV<500
_PROTOTYPE(extern int endservent,(void));
_PROTOTYPE(extern int setservent,(int));

# if	defined(HASSTATLSTAT)
_PROTOTYPE(extern int statlstat,(const char *, struct stat *));
# endif	/* defined(HASTSTATLSTAT) */

_PROTOTYPE(extern int strcasecmp,(char *, char *));
_PROTOTYPE(extern int strncasecmp,(char *, char *, unsigned int));
_PROTOTYPE(extern pid_t wait,());
#endif	/* OSRV<500 */

_PROTOTYPE(extern int sysi86,());
_PROTOTYPE(extern int sysfs,());
_PROTOTYPE(extern void udp_tm,(time_t tm));

#if	!defined(N_UNIX)
_PROTOTYPE(extern char *get_nlist_path,(int pd));
#endif	/* !defined(N_UNIX) */
