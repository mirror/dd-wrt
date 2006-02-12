/*
 * Copyright (c) 1997
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @(#) $Header: /home/cvsroot/kaid/ifaddrlist.h,v 1.2 2004/12/07 13:09:05 luis Exp $ (LBL)
 */

#ifndef KAIUI__H
#define KAIUI__H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
		
struct ifaddrlist {
	u_int32_t addr;
	char *device;
};

int	ifaddrlist(struct ifaddrlist **, char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */
		
#endif
