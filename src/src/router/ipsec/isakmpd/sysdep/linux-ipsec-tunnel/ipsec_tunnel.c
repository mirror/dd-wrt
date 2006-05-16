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

#include "ipsec_tunnel.h"
#include "util.h"

#define IPROUTE2_PATCHED 0

static int ipsec_dev_map[IPSEC_TUNNEL_DEV_MAX];
static int tunnel_dev = 0;
static int clear_routing (int table);

static int get_nextdev (struct sockaddr* sa)
{
	FILE* pipe = 0;
	char cmd[80], devname[32];
	char* addr = 0;
	int iface = 0;

	sockaddr2text (sa, (char**) &addr, 0);
#if IPROUTE2_PATCHED
	snprintf (cmd, sizeof cmd, "ip -n route get %s", addr);
#else
	snprintf (cmd, sizeof cmd, "ip route get %s", addr);
#endif
	pipe = popen (cmd, "r");
	if (pipe == NULL)
	{
		int err = errno;
		log_error("error: cannot run '%s': %s\n",
				  cmd, strerror(err));
		goto fini;
	}

	if (fgets (cmd, sizeof cmd, pipe) != NULL &&
		(sscanf (cmd, "%*s dev %s", devname) == 1 ||
		 sscanf (cmd, "%*s via %*s dev %s", devname) == 1))
	{
		/* direct connection */
		struct ifreq ifr;
		int fd, st;

		fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
		if (fd == -1)
		{
			int err = errno;
			log_error("error: cannot open socket: %s\n", 
					strerror (err));
			goto fini;
		}

		memset(&ifr, 0, sizeof(struct ifreq));
		strcpy(ifr.ifr_name, devname);
		st = ioctl(fd, SIOCGIFINDEX, &ifr);
		close(fd);

		if (st == 0)
			iface = ifr.ifr_ifindex;
	}
	
fini:
	if (addr) free (addr);
	if (pipe) pclose (pipe);
	return iface;
}


static int ipsec_tunnel_dev_open(const char *name, struct ifreq *ifr, int quiet) 
{
	int fd, st;

	fd = ipsec_tunnel_open();
	if (fd == -1)
		return -1;
	
	memset(ifr, 0, sizeof(struct ifreq));
	strcpy(ifr->ifr_name, name);
	st = ioctl(fd, SIOCGIFHWADDR, ifr);
	if (st != 0) {
		if (!quiet) {
			int err = errno;
			log_error("error: cannot open %s [%s]\n",
				  name, strerror(err));
			if (err == ENODEV && strcmp(name, IPSEC_DEV_NAME) == 0)
				log_error("make sure the kernel module is loaded.\n");
		}
		close(fd);
		return -1;
	}
	
	if (ifr->ifr_hwaddr.sa_family != ARPHRD_IPSEC) {
		if (!quiet)
			log_error("not an IPsec device!\n");
		close(fd);
		return -1;
	}
	
	return(fd);
}

static void 
ipsec_tunnel_delete (uint32_t spi)
{
	int i, fd, st;
	char ifname[64];
	struct ipsec_tunnel_parm ipsec;
	struct ifreq ifr;
	

	for (i=0; i<=tunnel_dev; ++i)
	{
		if (! ipsec_dev_map[i]) continue;

		sprintf(ifname, "%s%d", IPSEC_TUNNEL_DEV_NAME, i+1);
		
		fd = ipsec_tunnel_dev_open(ifname, &ifr, 0);
		if (fd == -1) continue;

		memset(&ipsec, 0, sizeof(struct ipsec_tunnel_parm));
		ifr.ifr_data = (char*)&ipsec;
		st = ioctl(fd, SIOCIPSEC_GET_TUNNEL, &ifr);
		if (st != 0)
			st = ioctl(fd, SIOCIPSEC_GET_TUNNEL_OLD, &ifr);
		
		if (st != 0)
		{
			close(fd);
			continue;
		}
		
		if (ipsec.spi == spi)
		{
			st = ioctl(fd, SIOCIPSEC_DEL_TUNNEL, &ifr);
			close (fd);

			if (st != 0) {
				log_error("Cannot delete tunnel '%s'! [%s]", ifname, strerror(errno));
			} 
			else 
			{
				ipsec_dev_map[i] = 0;
				clear_routing (i+100);
			}

			break;
		}

		close(fd);
	}					

return;
}

static void ipsec_tunnel_stayalive (struct exchange *exchange, void *vconn, int fail)
{
	char *conn = vconn;
	struct sa *sa;
	
	/* XXX What if it is phase 1?  */
	sa = sa_lookup_by_name (conn, 2);
	if (sa)
		sa->flags |= SA_FLAG_STAYALIVE;
}

/* delete existing SAs */
void ipsec_tunnel_flush()
{
	struct ipsec_sa_parm sa;
	struct ifreq ifr;
	int fd, st;
	char name[32];

	/* delete all tunnels */
	while (tunnel_dev > 0)
	{
		if (! ipsec_dev_map[tunnel_dev-1]) continue;

		sprintf(name, "%s%d", IPSEC_TUNNEL_DEV_NAME, tunnel_dev--);

		ipsec_dev_map[tunnel_dev] = 0;
		
		fd = ipsec_tunnel_dev_open(name, &ifr, 0);
		if (fd == -1) continue;

		st = ioctl(fd, SIOCIPSEC_DEL_TUNNEL, &ifr);
		if (st != 0)
			log_error("Cannot delete tunnel! [%s]", strerror(errno));

		close(fd);
	}					

	/* delete sas */
	fd = ipsec_tunnel_dev_open(IPSEC_DEV_NAME, &ifr, 0);
	if (fd == -1)
		return;
	
	memset(&sa, 0, sizeof(struct ipsec_sa_parm));
	ifr.ifr_data = (char*)&sa;

	sa.version = IPSEC_SA_VERSION;
	sa.dst = INADDR_ANY;
	sa.src = INADDR_ANY;
	sa.spi = IPSEC_SPI_ANY;

	st = ioctl(fd, SIOCIPSEC_DEL_SA, &ifr);
	/* ...ignore errors */
}

/* check tunnel connection */
void ipsec_tunnel_connection_check (char *name) 
{
	if (!sa_lookup_by_name (name, 2)) {
		LOG_DBG ((LOG_SYSDEP, 70, "ipsec_tunnel_connection_check: SA for %s missing",
			  name));
		exchange_establish (name, ipsec_tunnel_stayalive, name);
	} else
		LOG_DBG ((LOG_SYSDEP, 70, "ipsec_tunnel_connection_check: SA for %s exists",
			  name));
}

/* get new tunnel device name */
static int ipsec_tunnel_get_name(char *name) 
{
	int i;
	for (i=0; ipsec_dev_map[i] && i<=tunnel_dev+1; ++i);
	
	if(i >= IPSEC_TUNNEL_DEV_MAX)
		return(-1);

	if (i > tunnel_dev) tunnel_dev = i;
	ipsec_dev_map[i] = 1;
	
	sprintf(name, "%s%d", IPSEC_TUNNEL_DEV_NAME, i+1);
	
	LOG_DBG ((LOG_SYSDEP, 70, "ipsec_tunnel_get_name: requested new device %s",
		  name));
	
	return(i);
}

static int ipsec_dev_to_index (char* dev)
{
	if (strlen (dev) < sizeof IPSEC_TUNNEL_DEV_NAME) return -1;
	dev += sizeof IPSEC_TUNNEL_DEV_NAME - 1;
	if (*dev < '0' || *dev > '9') return -1;
	return atoi(dev)-1;
}

static char* addr_prefix (char* str, uint32_t addr, uint32_t mask)
{
	struct in_addr ia;
	char saddr[INET_ADDRSTRLEN];
	int p;
	
	ia.s_addr = addr;
	inet_ntop (AF_INET, &ia, saddr, sizeof (saddr));

	for (p=0; p<32 && (mask & 01); mask>>=1, ++p);
	if (! str) str = malloc (INET_ADDRSTRLEN+3);
	sprintf (str, "%s/%u", saddr, p);
		
	return str;
}

static int clear_routing (int table)
{
	FILE* pipe = 0;
	char cmd[80];
#if IPROUTE2_PATCHED
	snprintf (cmd, sizeof cmd, "ip -n rule show");
#else
	snprintf (cmd, sizeof cmd, "ip rule show");
#endif
	pipe = popen (cmd, "r");
	if (pipe == NULL)
	{
		int err = errno;
		log_error("error: cannot run '%s': %s\n",
				  cmd, strerror(err));
		goto del_routes;
	}

	while (fgets (cmd, sizeof cmd, pipe) != NULL)
	{
		char from[32], to[32], iif[32];
		int tbl = -1;
		
		if (sscanf (cmd, "%*d: from %s to %s iif %s lookup %d", 
					from, to, iif, &tbl) == 4 && tbl == table)
		{
			if (! strcmp(from, "all")) strcpy (from, "0.0.0.0/0");
			if (! strcmp(to, "all")) strcpy (to, "0.0.0.0/0");
			sprintf (cmd, "ip rule del from %s to %s iif %s table %d", 
					from, to, iif, tbl);
		}
		else if (sscanf (cmd, "%*d: from %s to %s lookup %d", 
					from, to, &tbl) == 3 && tbl == table)
		{
			if (! strcmp(from, "all")) strcpy (from, "0.0.0.0/0");
			sprintf (cmd, "ip rule del from %s to %s table %d", from, to, tbl);
		}
		else if (sscanf (cmd, "%*d: from %s lookup %d", from, &tbl) == 2 &&
				tbl == table)
		{
			if (! strcmp(from, "all")) strcpy (from, "0.0.0.0/0");
			sprintf (cmd, "ip rule del from %s table %d", from, tbl);
		}
		
		if (tbl == table) system (cmd);
	}
	pclose (pipe);

del_routes:
#if IPROUTE2_PATCHED
	snprintf (cmd, sizeof cmd, "ip -n route show table %d", table);
#else
	snprintf (cmd, sizeof cmd, "ip route show table %d", table);
#endif
	pipe = popen (cmd, "r");
	if (pipe == NULL)
	{
		int err = errno;
		log_error("error: cannot run '%s': %s\n",
				  cmd, strerror(err));
		return 0;
	}

	while (fgets (cmd, sizeof cmd, pipe) != NULL)
	{
		char addr[80];
		if (sscanf (cmd, "%s ", addr) == 1)
		{
			sprintf (cmd, "ip route del %s table %d", addr, table);
			system (cmd);
		}
	}
	pclose (pipe);


	return 0;
}

/* add route to tunnel endpoint */
static int ipsec_tunnel_add_hostroute(uint32_t src, uint32_t src_mask,
																		  uint32_t dst, uint32_t dst_mask, 
																			char *dev)
{
	char cmd[80];
	char np_src[INET_ADDRSTRLEN+3], np_dst[INET_ADDRSTRLEN+3];
	int table = ipsec_dev_to_index (dev);

	if (table < 0)
	{
		log_error ("ipsec_tunnel_add_hostroute: "
				       "cannot recognize [%s] index\n",
							 dev);
		return -1;
	}
	
	clear_routing (table+100);
	
	addr_prefix (np_src, src, src_mask);
	addr_prefix (np_dst, dst, dst_mask);
	if (src_mask == 0xffffffff)
	{
		snprintf (cmd, sizeof cmd,
				"ip rule add iif lo to %s table %d",
				np_dst, table+100);
	}
	else
	{
		snprintf (cmd, sizeof cmd, 
				"ip rule add from %s to %s table %d", 
				np_src, np_dst, table+100);
	}
	system (cmd);

	snprintf (cmd, sizeof cmd, 
			"ip route add %s dev %s table %d", 
			np_dst, dev, table+100);
	system (cmd);
	
	return(0);
}

/* enable sa */
int ipsec_tunnel_enable_sa (struct sa *sa, struct sa *isakmp_sa) 
{ 
	struct ipsec_sa *isa = sa->data;
	struct sockaddr *dst, *src;
	struct proto *proto = TAILQ_FIRST (&sa->protos);
	struct ipsec_tunnel_parm ipsec;
	char tunnel_name[IPSEC_TUNNEL_DEV_MAX];
	struct ifreq ifr, ipr;
	struct sockaddr_in *tunip;
	int fd, st, nextdev = 0;
	int s = -1;
	
	fd = ipsec_tunnel_dev_open(IPSEC_DEV_NAME, &ifr, 0);
	if (fd == -1)
		return(-1);

	sa->transport->vtbl->get_dst (sa->transport, &dst);
	sa->transport->vtbl->get_src (sa->transport, &src);

	/* get new tunnel name */
	if(ipsec_tunnel_get_name((char*)&tunnel_name) < 0)
		return(-1);

	nextdev = get_nextdev (dst);
		
	memset(&ipsec, 0, sizeof(struct ipsec_tunnel_parm));
	ifr.ifr_data = (char*)&ipsec;
	strncpy(ipsec.name, tunnel_name, strlen(tunnel_name));
	ipsec.link = nextdev;
	ipsec.iph.version = 4;
	ipsec.iph.ihl = 5;
	ipsec.iph.protocol = IPPROTO_ESP;
	ipsec.iph.daddr = ((struct sockaddr_in *)(dst))->sin_addr.s_addr;
	ipsec.iph.saddr = ((struct sockaddr_in *)(src))->sin_addr.s_addr;
	
	ipsec.spi = htonl(*(uint32_t*)proto->spi[0]); 
	// ipsec.spi = IPSEC_SPI_ANY;
	
	/*** STEP 0: check for existing tunnel device ***/
	LOG_DBG ((LOG_SYSDEP, 90, "ipsec_tunnel_enable_sa: STEP 0 - check for tunnel device %s/%s", 
		  inet_ntoa(((struct sockaddr_in *)(src))->sin_addr),
		  inet_ntoa(((struct sockaddr_in *)(dst))->sin_addr)));
	
	if((st = ioctl(fd, SIOCIPSEC_CHG_TUNNEL, &ifr)) != 0) {
		LOG_DBG ((LOG_SYSDEP, 90, "ipsec_tunnel_enable_sa: STEP 1 - add new tunnel device %s", tunnel_name));

		if((st = ioctl(fd, SIOCIPSEC_ADD_TUNNEL, &ifr)) != 0) {
			LOG_DBG((LOG_SYSDEP, 10, "ipsec_tunnel_enable_sa: cannot add tunnel %s",
				 tunnel_name));
			ipsec_dev_map[ipsec_dev_to_index (tunnel_name)] = 0;
			if (st != -1)
				close (st);
			return(-1);
		}
	} else {
		goto skip_new_tunnel;
	}

	close(st);
	close(fd);
	
	/*** STEP 2: set tunnel device ip ***/
	
	LOG_DBG ((LOG_SYSDEP, 90, "ipsec_tunnel_enable_sa: STEP 2 - set tunnel ip address (%s)",
		  inet_ntoa(((struct sockaddr_in *)(isa->src_net))->sin_addr)));
	
	s = socket (PF_INET, SOCK_DGRAM, AF_UNSPEC);
	if (s == -1) {
		log_error ("ipsec_tunnel_enable_sa: "
			   "socket(PF_INET, SOCK_DGRAM, AF_UNSPEC) failed");
		return(-1);
	}
	
	memset(&ipr, 0, sizeof(ipr));
	strncpy(ipr.ifr_ifrn.ifrn_name, tunnel_name, strlen(tunnel_name));

	tunip = (void *) &ipr.ifr_ifru.ifru_addr;
	
	tunip->sin_addr.s_addr =
#if 0
		/* not good when tunneling net<->net or net<->host */
		((struct sockaddr_in *)(isa->src_net))->sin_addr.s_addr;
#else
		((struct sockaddr_in *)(src))->sin_addr.s_addr;
#endif
	tunip->sin_family = AF_INET;
	tunip->sin_port = 0;
	
	if ((fd = ioctl(s, SIOCSIFADDR, &ipr)) < 0) {
		log_error ("ipsec_tunnel_enable_sa: ioctl (%d, SIOCSIFADDR, %p) failed", s, tunip);
		if(s != -1)
			close(s);
		return(-1);
	}
	
	memset(&ipr, 0, sizeof(ipr));
	strncpy(ipr.ifr_ifrn.ifrn_name, tunnel_name, strlen(tunnel_name));
	ipr.ifr_ifru.ifru_flags = IFF_UP;
	
	if ((fd = ioctl(s, SIOCSIFFLAGS, &ipr)) < 0) {
		log_error ("ipsec_tunnel_enable_sa: ioctl (%d, SIOCSIFFLAGS, %p) failed", s, tunip);
		if(s != -1)
			close(s);
		return(-1);
	}
	
#if 1 /*** TODO: add host route to tunnel endpoint ***/
	/*** STEP 3: set tunnel route ***/

	ipsec_tunnel_add_hostroute(
			((struct sockaddr_in *)(isa->src_net))->sin_addr.s_addr, 
			((struct sockaddr_in *)(isa->src_mask))->sin_addr.s_addr, 
			((struct sockaddr_in *)(isa->dst_net))->sin_addr.s_addr, 
			((struct sockaddr_in *)(isa->dst_mask))->sin_addr.s_addr, 
			tunnel_name);

#else
	log_error ("ipsec_tunnel_enable_sa: please type 'route add -host %s dev %s' (not yet implemented)", 
		   inet_ntoa(((struct sockaddr_in *)(isa->dst_net))->sin_addr), tunnel_name);
#endif
	
	close (s);

 skip_new_tunnel:
	
	LOG_DBG ((LOG_SYSDEP, 90, "ipsec_tunnel_enable_sa: done")); 
	
	return 0;
}

/* return new spi */
u_int8_t *ipsec_tunnel_get_spi (size_t *sz, u_int8_t proto, struct sockaddr *src,
				struct sockaddr *dst, u_int32_t seq) 
{
	u_int8_t *spi;
	u_int32_t spinum;
	
	*sz = IPSEC_SPI_SIZE;
	spi = malloc (*sz);
	if (!spi)
		return 0;
	do
		spinum = sysdep_random ();
	while (spinum < IPSEC_SPI_LOW);
	spinum = htonl (spinum);
	memcpy (spi, &spinum, *sz);
	
	LOG_DBG ((LOG_SYSDEP, 90, "ipsec_tunnel_get_spi: spi %p (0x%x)", spi, *sz));
	
	return spi;
}

/* ??? */
int ipsec_tunnel_group_spis (struct sa *sa, struct proto *proto1,
			     struct proto *proto2, int incoming) { return 0; }

/* open ipsec tunnel */
int ipsec_tunnel_open (void) 
{ 
	int fd;
	
	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd == -1) {
		log_error("ipsec_tunnel_open: cannot open socket\n");
		return -1;
	}
	
	return fd; 
}

/* add new spi */
int ipsec_tunnel_set_spi (struct sa *sa, struct proto *proto, int incoming,
			  struct sa *isakmp_sa) 
{
        struct ipsec_sa_parm sap;
	struct ipsec_proto *iproto = proto->data;
	struct sockaddr *dst, *src;
	struct ifreq ifr;
	int fd, st, keylen, hashlen, digest = 1;

	fd = ipsec_tunnel_dev_open(IPSEC_DEV_NAME, &ifr, 0);
	if (fd == -1)
		return -1;

	memset(&sap, 0, sizeof(struct ipsec_sa_parm));
	ifr.ifr_data = (char*)&sap;
	
	sap.version = IPSEC_SA_VERSION;
	
	/* connection */
	sa->transport->vtbl->get_dst (sa->transport, &dst);
	sa->transport->vtbl->get_src (sa->transport, &src);
	sap.dst = ((struct sockaddr_in *)(incoming ? src : dst))->sin_addr.s_addr;
	sap.src = ((struct sockaddr_in *)(incoming ? dst : src))->sin_addr.s_addr;

	sap.spi = htonl(*(uint32_t*)proto->spi[incoming]);
	
	/* mode, cipher */
	switch (proto->proto) {
	case IPSEC_PROTO_IPSEC_ESP:
		keylen = ipsec_esp_enckeylength (proto);
		hashlen = ipsec_esp_authkeylength (proto);
		      
		switch (proto->id) {
			/** 128 bit blocksize ciphers **/
		case IPSEC_ESP_AES:
		case IPSEC_ESP_AES_RIJNDAEL:
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_AES_RIJNDAEL_CBC,
				sizeof(IPSEC_CRYPTOAPI_AES_RIJNDAEL_CBC));
			break;
			      
		case IPSEC_ESP_AES_TWOFISH:
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_AES_TWOFISH_CBC,
				sizeof(IPSEC_CRYPTOAPI_AES_TWOFISH_CBC));
			break;

		case IPSEC_ESP_AES_SERPENT:
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_AES_SERPENT_CBC,
				sizeof(IPSEC_CRYPTOAPI_AES_SERPENT_CBC));
			break;

		case IPSEC_ESP_AES_RC6:
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_AES_RC6_CBC,
				sizeof(IPSEC_CRYPTOAPI_AES_RC6_CBC));
			break;

		case IPSEC_ESP_AES_MARS:
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_AES_MARS_CBC,
				sizeof(IPSEC_CRYPTOAPI_AES_MARS_CBC));
			      
			/** 64 bit blocksize ciphers **/
		case IPSEC_ESP_BLOWFISH:
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_BLOWFISH_CBC,
				sizeof(IPSEC_CRYPTOAPI_BLOWFISH_CBC));
			break;
			      
		case IPSEC_ESP_3DES:
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_DES_EDE3_CBC,
				sizeof(IPSEC_CRYPTOAPI_DES_EDE3_CBC));
			break;

		case IPSEC_ESP_RC5:
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_RC5_CBC,
				sizeof(IPSEC_CRYPTOAPI_RC5_CBC));
			break;

		case IPSEC_ESP_CAST:
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_CAST_CBC,
				sizeof(IPSEC_CRYPTOAPI_CAST_CBC));
			break;

		case IPSEC_ESP_IDEA:
			LOG_DBG ((LOG_SYSDEP, 10,
				  "ipsec_tunnel_set_spi: The IDEA(tm) block cipher is covered by patents."
				  "NOT RECOMMENDED - use a patent-free cipher!"));
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_IDEA_CBC,
				sizeof(IPSEC_CRYPTOAPI_IDEA_CBC));
			break;

		case IPSEC_ESP_DES: /* insecure backward compatibility */
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_DES_CBC,
				sizeof(IPSEC_CRYPTOAPI_DES_CBC));
			break;

		case IPSEC_ESP_NULL: /* for testing, no encryption */
			strncpy(sap.cipher, IPSEC_CRYPTOAPI_NULL_CBC,
				sizeof(IPSEC_CRYPTOAPI_NULL_CBC));
			break;
			      
		default:
			LOG_DBG ((LOG_SYSDEP, 10,
				  "ipsec_tunnel_set_spi: cipher/auth not supported (%d)", (proto->id|(iproto->auth<<8))));
			return -1;
		}

		switch (iproto->auth) {
		case IPSEC_AUTH_HMAC_MD5:
			strncpy(sap.digest, IPSEC_CRYPTOAPI_MD5, sizeof(IPSEC_CRYPTOAPI_MD5));
			break;
		case IPSEC_AUTH_HMAC_SHA:
			strncpy(sap.digest, IPSEC_CRYPTOAPI_SHA1, sizeof(IPSEC_CRYPTOAPI_SHA1));
			break;
		case IPSEC_AUTH_HMAC_SHA2_256:
			strncpy(sap.digest, IPSEC_CRYPTOAPI_SHA256, sizeof(IPSEC_CRYPTOAPI_SHA512));
			break;
		case IPSEC_AUTH_HMAC_SHA2_384:
			strncpy(sap.digest, IPSEC_CRYPTOAPI_SHA384, sizeof(IPSEC_CRYPTOAPI_SHA384));
			break;
		case IPSEC_AUTH_HMAC_SHA2_512:
			strncpy(sap.digest, IPSEC_CRYPTOAPI_SHA512, sizeof(IPSEC_CRYPTOAPI_SHA512));
			break;
		case IPSEC_AUTH_HMAC_RIPEMD:
			strncpy(sap.digest, IPSEC_CRYPTOAPI_RIPEMD, sizeof(IPSEC_CRYPTOAPI_RIPEMD));
			break;

		default:
			digest = 0;
		}

		break;
	default:
		log_error("ipsec_tunnel_set_spi: only esp mode supported (%d)", proto->proto);
		return -1;
	}
		
	/* key, keylen */
	sap.cipher_key = malloc(keylen);
	memcpy ((void*)sap.cipher_key, iproto->keymat[incoming], keylen);
	sap.cipher_keylen = keylen;

	/* hmac-(sha1|md5) authentication header */
	if(digest) {
	        LOG_DBG ((LOG_SYSDEP, 50, "ipsec_tunnel_set_spi: adding HMAC-%s-96 authentication (hashlen %d, hmaclen %d)", 
		          sap.digest, hashlen, IPSEC_CRYPTOAPI_HMACLEN));
		sap.digest_key = malloc(hashlen);
		memcpy ((void*)sap.digest_key, iproto->keymat[incoming] + keylen, hashlen);
		sap.digest_keylen = hashlen;
		sap.digest_hmaclen = IPSEC_CRYPTOAPI_HMACLEN/8;
	}

	st = ioctl(fd, SIOCIPSEC_ADD_SA, &ifr);
	if (st != 0) {
	        log_error("ipsec_tunnel_set_spi: cannot add sa - spi: [0x%08x] keylen: [%d]", 
		          sap.spi, sap.cipher_keylen);
		return -1;
	}

	LOG_DBG ((LOG_SYSDEP, 50, "ipsec_tunnel_set_spi: done - spi: [0x%08x] keylen: [%d]", 
		  sap.spi, sap.cipher_keylen));

	free((void*)sap.cipher_key);
	if(digest) 
		free((void*)sap.digest_key);

	return 0;
}

/* remove sa from list */
int ipsec_tunnel_delete_spi (struct sa *sa, struct proto *proto, int incoming) 
{
        struct ipsec_sa_parm sap;
	struct sockaddr *dst, *src;
	struct ifreq ifr;
	int fd, st;

	if (incoming == 0)
		ipsec_tunnel_delete ( htonl(*(uint32_t*)proto->spi[incoming]) );
	fd = ipsec_tunnel_dev_open(IPSEC_DEV_NAME, &ifr, 0);
	if (fd == -1)
		return -1;
	
	memset(&sap, 0, sizeof(struct ipsec_sa_parm));
	ifr.ifr_data = (char*)&sap;
	
	sap.version = IPSEC_SA_VERSION;
	
	/* connection */
	sa->transport->vtbl->get_dst (sa->transport, &dst);
	sa->transport->vtbl->get_src (sa->transport, &src);
	sap.dst = ((struct sockaddr_in *)(incoming ? src : dst))->sin_addr.s_addr;
	sap.src = ((struct sockaddr_in *)(incoming ? dst : src))->sin_addr.s_addr;
	
	sap.spi = htonl(*(uint32_t*)proto->spi[incoming]);
	
	st = ioctl(fd, SIOCIPSEC_DEL_SA, &ifr);
	if (st != 0) {
	        LOG_DBG ((LOG_SYSDEP, 10, "ipsec_tunnel_delete_spi: cannot delete sa - spi: [0x%x] keylen: [%d]", 
		          sap.spi, sap.cipher_keylen));
		return -1;
	}

	LOG_DBG ((LOG_SYSDEP, 50, "ipsec_tunnel_delete_spi: done - spi: [0x%x] keylen: [%d]", 
		  sap.spi, sap.cipher_keylen));

	return 0;
}
