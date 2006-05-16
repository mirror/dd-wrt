/* 
 * Syslog functions.
 * Copyright (C) 2003, 2004 Mondru AB.
 * 
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 * 
 */

#ifndef _SYSERR_H
#define _SYSERR_H

#define SYSERR_MSGSIZE 256

void sys_err(int pri, char *filename, int en, int line, char *fmt, ...);
void sys_errpack(int pri, char *fn, int ln, int en, struct sockaddr_in *peer,
		 void *pack, unsigned len, char *fmt, ...);

#endif	/* !_SYSERR_H */
