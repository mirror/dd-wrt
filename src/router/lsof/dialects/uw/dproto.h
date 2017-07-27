/*
 * dproto.h - SCO UnixWare function prototypes for lsof
 *
 * The _PROTOTYPE macro is defined in the common proto.h.
 */


/*
 * Copyright 1996 Purdue Research Foundation, West Lafayette, Indiana
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



_PROTOTYPE(extern int get_max_fd,(void));
_PROTOTYPE(extern int is_file_named,(char *p, int cd));
_PROTOTYPE(extern void process_socket,(char *pr, struct queue *q));
_PROTOTYPE(extern int readbfslino,(struct vnode *v, struct l_ino *i));
_PROTOTYPE(extern int readcdfslino,(struct vnode *v, struct l_ino *i));
_PROTOTYPE(extern int readdosfslino,(struct vnode *v, struct l_ino *i));
_PROTOTYPE(extern int reads5lino,(struct vnode *v, struct l_ino *i));
_PROTOTYPE(extern int readvxfslino,(struct vnode *v, struct l_ino *i));
_PROTOTYPE(extern int strcasecmp,(char *s1, char *s2));
_PROTOTYPE(extern int strncasecmp,(char *s1, char *s2, int n));

#if	UNIXWAREV>=70101
_PROTOTYPE(extern int process_unix_sockstr,(struct vnode *v, KA_T na));
#endif	/* UNIXWAREV>=70101 */
