/*                _______              ___       -*- linux-c -*-
 *      _________/ _____ \____________/ _ \____________________  
 * __  |____ _ _ _|_   _| __ ___  _ __ (_)_  __  ___ ___  _ __ |__  
 * \ \ / / _` | '_ \| || '__/ _ \| '_ \| \ \/ / / __/ _ \| '_ ` _ \ 
 *  \ V / (_| | | | | || | | (_) | | | | |>  < | (_| (_) | | | | | |
 *   \_/ \__,_|_| |_|_||_|  \___/|_| |_|_/_/\_(_)___\___/|_| |_| |_|
 *     | contact: secteam@vantronix.net, https://vantronix.net |               
 *     +-------------------------------------------------------+
 * 
 * isakmpd sysdeps for the linux ipsec implementation "ipsec_tunnel"
 *
 * Copyright (c) 2002 Reyk Floeter.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <md5.h>
#include <unistd.h>

#include "sysdep.h"

#ifdef NEED_SYSDEP_APP
#include "app.h"
#include "conf.h"
#include "ipsec.h"
#include "ipsec_tunnel.h"
#endif /* NEED_SYSDEP_APP */
#include "log.h"
#include "sysdep.h"

extern char *__progname;

u_int32_t
sysdep_random ()
{
	u_int32_t rndval;
	u_char sig[16];
	MD5_CTX ctx;
	int fd, i;
	struct {
		struct timeval tv;
		u_int rnd[(128 - sizeof (struct timeval)) / sizeof (u_int)];
	} rdat;
	
	fd = open ("/dev/urandom", O_RDONLY);
	if (fd != -1)
	{
		read (fd, rdat.rnd, sizeof(rdat.rnd));
		close (fd);
	}
	MD5Init (&ctx);
	MD5Update (&ctx, (char *)&rdat, sizeof(rdat));
	MD5Final (sig, &ctx);
	
	rndval = 0;	
	for (i = 0; i < 4; i++)
	{
		u_int32_t *tmp = (u_int32_t *)&sig[i * 4];
		rndval ^= *tmp;
	}
	
	return rndval;
}

char *
sysdep_progname ()
{
	static char pname[] = "isakmpd";
	return pname;
	// return __progname;
}

/* Return the length of the sockaddr struct.  */
u_int8_t
sysdep_sa_len (struct sockaddr *sa)
{
	switch (sa->sa_family)
	{
	case AF_INET:
		return sizeof (struct sockaddr_in);
		/* IPv6 is not yet supported...    
		   case AF_INET6:
		   return sizeof (struct sockaddr_in6); 
		*/
	case AF_INET6:
		log_print ("sysdep_sa_len: no sysdep support for IPv6 in ipsec_tunnel!");
		break;
	default:
		log_print ("sysdep_sa_len: unknown sa family %d", sa->sa_family);
	}

	return (0);
}

/* As regress/ use this file I protect the sysdep_app_* stuff like this.  */
#ifdef NEED_SYSDEP_APP
int
sysdep_app_open ()
{
	/** delete existing SAs **/
	ipsec_tunnel_flush();

	/** open tunnel **/
	return ipsec_tunnel_open ();
}

void
sysdep_app_handler (int fd)
{
}

/* Check that the connection named NAME is active, or else make it active.  */
void
sysdep_connection_check (char *name)
{
	ipsec_tunnel_connection_check (name);
}

/*
 * Generate a SPI for protocol PROTO and the source/destination pair given by
 * SRC, SRCLEN, DST & DSTLEN.  Stash the SPI size in SZ.
 */
u_int8_t *
sysdep_ipsec_get_spi (size_t *sz, u_int8_t proto, struct sockaddr *src,
		      struct sockaddr *dst, u_int32_t seq)
{
	LOG_DBG ((LOG_SYSDEP, 99,
		   "ipsec_tunnel_sysdep: _get_spi."));
	
	if (app_none)
	{
		*sz = 0x1000; /*IPSEC_SPI_SIZE; <--- not implemented */ 
		/* XXX should be random instead I think.  */
		return strdup ("\x12\x34\x56\x78");
	}
	
	return ipsec_tunnel_get_spi (sz, proto, src, dst, seq);
}

int
sysdep_cleartext (int fd, int af)
{
	LOG_DBG ((LOG_SYSDEP, 99,
		   "ipsec_tunnel_sysdep: _cleartext."));
	
	return 0;
}

int
sysdep_ipsec_delete_spi (struct sa *sa, struct proto *proto, int incoming)
{
	LOG_DBG ((LOG_SYSDEP, 99,
		   "ipsec_tunnel_sysdep: _delete_spi."));
	
	return ipsec_tunnel_delete_spi (sa, proto, incoming);
}

int
sysdep_ipsec_enable_sa (struct sa *sa, struct sa *isakmp_sa)
{
	LOG_DBG ((LOG_SYSDEP, 99,
		   "ipsec_tunnel_sysdep: _enable_sa."));
	
	return ipsec_tunnel_enable_sa (sa, isakmp_sa);
}

int
sysdep_ipsec_group_spis (struct sa *sa, struct proto *proto1,
			 struct proto *proto2, int incoming)
{
	LOG_DBG ((LOG_SYSDEP, 99,
		   "ipsec_tunnel_sysdep: _group_spis."));
	
	return ipsec_tunnel_group_spis (sa, proto1, proto2, incoming);
}

int
sysdep_ipsec_set_spi (struct sa *sa, struct proto *proto, int incoming,
		      struct sa *isakmp_sa)
{
	LOG_DBG ((LOG_SYSDEP, 99,
		   "ipsec_tunnel_sysdep: _set_spi."));
	
	return ipsec_tunnel_set_spi (sa, proto, incoming, isakmp_sa);
}
#endif
