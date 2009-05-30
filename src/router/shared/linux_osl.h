/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: linux_osl.h,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */

/*
 * #include <arpa/inet.h> #include <assert.h> #include <ctype.h> #include
 * <errno.h> #include <fcntl.h> #include <netdb.h> #include <netinet/tcp.h> 
 * #include <stdio.h> #include <stdlib.h> #include <string.h> #include
 * <sys/socket.h> #include <sys/stat.h> #include <sys/uio.h> #include
 * <time.h> #include <unistd.h> #include <stdarg.h> // for va_list, etc.
 * 
 */
/*
 * damn - the unix and vxworks version of inet_aton return different error
 * codes. 
 */
#define UPNP_INET_ATON(a,b)   (inet_aton(a,b)!=0)

#define OSL_NULL_FILE "/dev/null"

#include "typedefs.h"
#include <time.h>
#include <stdarg.h>

int osl_join_multicast(struct iface *pif, int fd, ulong ipaddr, ushort port);	// added 
										// - 
										// tofu
