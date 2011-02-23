/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/tun.h - Tunnel Device Handling
***********************************************************
 $Author: jeroen $
 $Id: tun.h,v 1.3 2006-07-13 19:33:39 jeroen Exp $
 $Date: 2006-07-13 19:33:39 $
**********************************************************/

#ifndef TUN_H
#define TUN_H "H5K7:W3NDY5UU5N1K1N1C0l3"

#include "common.h"

#ifdef _WIN32
/* Windows writev() support */
struct iovec
{
	u_long	iov_len;
	char	*iov_base;
};

int writev(SOCKET sock, const struct iovec *vector, DWORD count);
void tun_list_tap_adapters(void);
#endif

#ifndef _WIN32
	typedef void (*TUN_PROCESS)(char *, unsigned int);
#else
	typedef void (*TUN_PROCESS)(char *, unsigned int);
#endif

struct tun_reader
{
	TUN_PROCESS	function;
};

void tun_write(char *buf, unsigned int length);
bool tun_start(struct tun_reader *tun);

#endif /* TUN_H */

