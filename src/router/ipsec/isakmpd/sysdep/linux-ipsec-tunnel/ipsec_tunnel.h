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

#ifndef _IPSEC_TUNNEL_ISAKMPD_H_
#define _IPSEC_TUNNEL_ISAKMPD_H_

#include <sys/queue.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <net/if.h>
#include <netinet/ip.h>

/* ugly workaround to avoid re- defines in route.h */
#ifndef _LINUX_IF_H
#define _LINUX_IF_H
#endif

#include <linux/route.h>
#include <asm/byteorder.h>

#include "conf.h"
#include "exchange.h"
#include "hash.h"
#include "ipsec.h"
#include "ipsec_doi.h"
#include "ipsec_num.h"
#include "isakmp.h"
#include "log.h"
#include "sa.h"
#include "timer.h"
#include "transport.h"

#ifndef ARPHRD_IPSEC
#define ARPHRD_IPSEC 31
#endif

#define SIOCIPSEC_GET_TUNNEL_OLD (SIOCDEVPRIVATE + 0)
#define SIOCIPSEC_ADD_TUNNEL (SIOCDEVPRIVATE + 1)
#define SIOCIPSEC_DEL_TUNNEL (SIOCDEVPRIVATE + 2)
#define SIOCIPSEC_CHG_TUNNEL (SIOCDEVPRIVATE + 3)
#define SIOCIPSEC_GET_SA (SIOCDEVPRIVATE + 4)
#define SIOCIPSEC_ADD_SA (SIOCDEVPRIVATE + 5)
#define SIOCIPSEC_DEL_SA (SIOCDEVPRIVATE + 6)
#define SIOCIPSEC_CHG_SA (SIOCDEVPRIVATE + 7)
#define SIOCIPSEC_CHK_TUNNEL   (SIOCDEVPRIVATE + 8)
#define SIOCIPSEC_GET_TUNNEL (SIOCDEVPRIVATE + 9)

#define IPSEC_DEV_NAME "ipsec0"
#define IPSEC_TUNNEL_DEV_NAME "ipsec"
#define IPSEC_TUNNEL_DEV_MAX 255

#define IPSEC_SPI_ANY 0
#define IPSEC_MAX_DIGEST_KEYLEN 20
#define IPSEC_CRYPTOAPI_HMACLEN 96 /* HMAC-[HASHALG]-96 */

#define IPSEC_SA_VERSION 0
#define IPSEC_SA_CRYPTOLEN 32

#define IPSEC_CRYPTOAPI_AES_RIJNDAEL_CBC "aes-cbc"
#define IPSEC_CRYPTOAPI_AES_TWOFISH_CBC "twofish-cbc"
#define IPSEC_CRYPTOAPI_AES_SERPENT_CBC "serpent-cbc"
#define IPSEC_CRYPTOAPI_AES_MARS_CBC "mars-cbc"
#define IPSEC_CRYPTOAPI_AES_RC6_CBC "rc6-cbc"
#define IPSEC_CRYPTOAPI_BLOWFISH_CBC "blowfish-cbc"
#ifndef IPSEC_CRYPTOAPI_DES_EDE3_CBC
#define IPSEC_CRYPTOAPI_DES_EDE3_CBC "3des-cbc" /* des_ede3 in old cryptoapi versions */
#endif
#define IPSEC_CRYPTOAPI_RC5_CBC "rc5-cbc"
#define IPSEC_CRYPTOAPI_DES_CBC "des-cbc"
#define IPSEC_CRYPTOAPI_IDEA_CBC "idea-cbc" /* The IDEA(tm) block cipher is covered by patents.
					     * NOT RECOMMENDED - use a patent-free cipher! */
#define IPSEC_CRYPTOAPI_CAST_CBC "cast5-cbc"
#define IPSEC_CRYPTOAPI_NULL_CBC "null-cbc" /* no encryption */
#define IPSEC_CRYPTOAPI_GOST_CBC "gost-cbc" /* currently not supported by ISAKMPD */

#define IPSEC_CRYPTOAPI_MD5 "md5"
#define IPSEC_CRYPTOAPI_SHA1 "sha1"
#define IPSEC_CRYPTOAPI_SHA256 "sha256" /* hmac not tested */
#define IPSEC_CRYPTOAPI_SHA384 "sha384" /* hmac not tested */
#define IPSEC_CRYPTOAPI_SHA512 "sha512" /* hmac not tested */
#define IPSEC_CRYPTOAPI_RIPEMD "ripemd" /* hmac not tested */

struct proto;
struct sa;
struct sockaddr;

struct ipsec_sa_parm
{
        uint32_t         version;

        uint32_t         dst;
        uint32_t         src;
        uint32_t         spi;
        uint32_t         flags;

        char        cipher[IPSEC_SA_CRYPTOLEN];
        int         cipher_keylen;
        const void *cipher_key;
        char        digest[IPSEC_SA_CRYPTOLEN];
        int         digest_keylen;
        const void *digest_key;
        int         digest_hmaclen;
};

struct ipsec_tunnel_parm
{
	char		name[IFNAMSIZ];
	int		link;
	struct iphdr	iph;
	int		spi;
};

extern void ipsec_tunnel_connection_check (char *);
extern int ipsec_tunnel_delete_spi (struct sa *, struct proto *, int);
extern int ipsec_tunnel_enable_sa (struct sa *, struct sa *);
extern u_int8_t *ipsec_tunnel_get_spi (size_t *sz, u_int8_t proto, struct sockaddr *src,
				struct sockaddr *dst, u_int32_t seq);
extern int ipsec_tunnel_group_spis (struct sa *sa, struct proto *proto1,
			     struct proto *proto2, int incoming);
extern int ipsec_tunnel_open (void);
extern void ipsec_tunnel_flush();
extern int ipsec_tunnel_set_spi (struct sa *sa, struct proto *proto, int incoming,
				 struct sa *isakmp_sa);

#endif
