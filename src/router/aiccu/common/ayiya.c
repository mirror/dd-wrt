/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/ayiya.c - AYIYA - Anything In Anything
***********************************************************
 $Author: jeroen $
 $Id: ayiya.c,v 1.15 2007-01-07 16:17:48 jeroen Exp $
 $Date: 2007-01-07 16:17:48 $
**********************************************************/

#include "aiccu.h"
#include "ayiya.h"
#include "tic.h"
#include "tun.h"

struct pseudo_ayh
{
	struct ayiyahdr	ayh;
	struct in6_addr	identity;
	sha1_byte	hash[SHA1_DIGEST_LENGTH];
	char		payload[2048];
};

struct in_addr		ayiya_ipv4_pop;			/* IPv4 remote endpoint */
struct in6_addr		ayiya_ipv6_local,		/* IPv6 local  endpoint */
			ayiya_ipv6_pop;			/* IPv6 remote endpoint */
sha1_byte		ayiya_hash[SHA1_DIGEST_LENGTH];	/* SHA1 Hash of the shared secret. */

TLSSOCKET ayiya_socket = NULL;

static const char reader_name[]	= "tundev->tun";
static const char writer_name[]	= "tun->tundev";
static const char beat_name[]	= "beat";

void ayiya_log(int level, const char *what, struct sockaddr_storage *clientaddr, socklen_t addrlen, const char *fmt, ...);
void ayiya_log(int level, const char *what, struct sockaddr_storage *clientaddr, socklen_t addrlen, const char *fmt, ...)
{
	char	buf[1024];
	char	clienthost[NI_MAXHOST];
	char	clientservice[NI_MAXSERV];
	va_list	ap;

	/* Clear them just in case */
	memset(buf, 0, sizeof(buf));
	memset(clienthost, 0, sizeof(clienthost));
	memset(clientservice, 0, sizeof(clientservice));

	if (clientaddr)
	{
		int ret;
		ret = getnameinfo((struct sockaddr *)clientaddr, addrlen,
			clienthost, sizeof(clienthost),
			clientservice, sizeof(clientservice),
			NI_NUMERICHOST|NI_NUMERICSERV);
		if (ret != 0)
		{
			dolog(LOG_ERR, "ayiya_log() getnameinfo() ret: %d, errno: %u, %s\n", ret, errno, strerror(errno));
		}
	}

	/* Print the host+port this is coming from */
	snprintf(buf, sizeof(buf), "[AYIYA-%s]%s%s%s%s : ",
		what,
		clientaddr ? " [" : "",
		clientaddr ? clienthost : "" ,
		clientaddr ? "]:" : "",
		clientservice ? clientservice : "");

	/* Print the log message behind it */
	va_start(ap, fmt);
	vsnprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), fmt, ap);
	va_end(ap);
	
	/* Actually Log it */
	dolog(level, buf);
}

/* Tun -> Socket */
void ayiya_reader(char *buf, unsigned int length);
void ayiya_reader(char *buf, unsigned int length)
{
	struct pseudo_ayh	*s = (struct pseudo_ayh *)buf, s2;
	int			lenout;
	SHA_CTX			sha1;
	sha1_byte		hash[SHA1_DIGEST_LENGTH];
	struct sockaddr_in	target;

	/* We tunnel over IPv4 */
	memcpy(&target.sin_addr, &ayiya_ipv4_pop, sizeof(target.sin_addr));
	target.sin_family = AF_INET;
	target.sin_port = htons(atoi(AYIYA_PORT));
	
	/* Prefill some standard AYIYA values */
	memset(&s, 0, sizeof(s));
	s2.ayh.ayh_idlen	= 4;			/* 2^4 = 16 bytes = 128 bits (IPv6 address) */
	s2.ayh.ayh_idtype	= ayiya_id_integer;
	s2.ayh.ayh_siglen	= 5;			/* 5*4 = 20 bytes = 160 bits (SHA1) */
	s2.ayh.ayh_hshmeth	= ayiya_hash_sha1;
	s2.ayh.ayh_autmeth	= ayiya_auth_sharedsecret;
	s2.ayh.ayh_opcode	= ayiya_op_forward;
	s2.ayh.ayh_nextheader	= IPPROTO_IPV6;

	/* Our IPv6 side of this tunnel */
	memcpy(&s2.identity, &ayiya_ipv6_local, sizeof(s2.identity));

	/* The payload */
	memcpy(&s2.payload, buf, length);

	/* Fill in the current time */
	s2.ayh.ayh_epochtime = htonl((u_long)time(NULL));

	/*
	 * The hash of the shared secret needs to be in the
	 * spot where we later put the complete hash
	 */
	memcpy(&s2.hash, ayiya_hash, sizeof(s2.hash));

	/* Generate a SHA1 */
	SHA1_Init(&sha1);
	/* Hash the complete AYIYA packet */
	SHA1_Update(&sha1, (sha1_byte *)&s2, sizeof(s2)-sizeof(s2.payload)+length);
	/* Store the hash in the packets hash */
	SHA1_Final(hash, &sha1);

	/* Store the hash in the actual packet */
	memcpy(&s2.hash, &hash, sizeof(s2.hash));

	/* Send it onto the network */
	length = sizeof(s2)-sizeof(s2.payload)+length;
#if defined(_FREEBSD) || defined(_DFBSD) || defined(_OPENBSD) || defined(_DARWIN) || defined(_NETBSD)
	lenout = send(ayiya_socket->socket, (const char *)&s2, length, 0);
#else
	lenout = sendto(ayiya_socket->socket, (const char *)&s2, length, 0, (struct sockaddr *)&target, sizeof(target));
#endif
	if (lenout < 0)
	{
		ayiya_log(LOG_ERR, reader_name, NULL, 0, "Error (%d) while sending %u bytes to network: %s (%d)\n", lenout, length, strerror(errno), errno);
	}
	else if (length != (unsigned int)lenout)
	{
		ayiya_log(LOG_ERR, reader_name, NULL, 0, "Only %u of %u bytes sent to network: %s (%s)\n", lenout, length, strerror(errno), errno);
	}
}

struct tun_reader ayiya_tun = { (TUN_PROCESS)ayiya_reader };

/* Socket -> Tun */
#ifndef _WIN32
void *ayiya_writer(void UNUSED *arg);
void *ayiya_writer(void UNUSED *arg)
#else
DWORD WINAPI ayiya_writer(LPVOID arg);
DWORD WINAPI ayiya_writer(LPVOID arg)
#endif
{
	unsigned char		buf[2048];
	struct pseudo_ayh	*s = (struct pseudo_ayh *)buf;
	struct sockaddr_storage	ci;
	socklen_t		cl;
	int			i, n;
	unsigned int		payloadlen = 0;
	SHA_CTX			sha1;
	sha1_byte		their_hash[SHA1_DIGEST_LENGTH],
				our_hash[SHA1_DIGEST_LENGTH];

	ayiya_log(LOG_INFO, writer_name, NULL, 0, "(Socket to TUN) started\n");

	/* Tun/TAP device is now running */
	g_aiccu->tunrunning = true;

	while (true)
	{
		cl = sizeof(ci);
		memset(buf, 0, sizeof(buf));
		n = recvfrom(ayiya_socket->socket, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&ci, &cl);

		if (n < 0) continue;

		if (n < (int)sizeof(struct ayiyahdr))
		{
			ayiya_log(LOG_WARNING, writer_name, &ci, cl, "Received packet is too short");
			continue;
		}

		if (	s->ayh.ayh_idlen != 4 ||
			s->ayh.ayh_idtype != ayiya_id_integer ||
			s->ayh.ayh_siglen != 5 ||
			s->ayh.ayh_hshmeth != ayiya_hash_sha1 ||
			s->ayh.ayh_autmeth != ayiya_auth_sharedsecret ||
			(s->ayh.ayh_nextheader != IPPROTO_IPV6 &&
			 s->ayh.ayh_nextheader != IPPROTO_NONE) ||
			(s->ayh.ayh_opcode != ayiya_op_forward &&
			 s->ayh.ayh_opcode != ayiya_op_echo_request &&
			 s->ayh.ayh_opcode != ayiya_op_echo_request_forward))
		{
			/* Invalid AYIYA packet */
			ayiya_log(LOG_ERR, writer_name, &ci, cl, "Dropping invalid AYIYA packet\n");
			ayiya_log(LOG_ERR, writer_name, &ci, cl, "idlen:   %u != %u\n", s->ayh.ayh_idlen, 4);
			ayiya_log(LOG_ERR, writer_name, &ci, cl, "idtype:  %u != %u\n", s->ayh.ayh_idtype, ayiya_id_integer);
			ayiya_log(LOG_ERR, writer_name, &ci, cl, "siglen:  %u != %u\n", s->ayh.ayh_siglen, 5);
			ayiya_log(LOG_ERR, writer_name, &ci, cl, "hshmeth: %u != %u\n", s->ayh.ayh_hshmeth, ayiya_hash_sha1);
			ayiya_log(LOG_ERR, writer_name, &ci, cl, "autmeth: %u != %u\n", s->ayh.ayh_autmeth, ayiya_auth_sharedsecret);
			ayiya_log(LOG_ERR, writer_name, &ci, cl, "nexth  : %u != %u || %u\n", s->ayh.ayh_nextheader, IPPROTO_IPV6, IPPROTO_NONE);
			ayiya_log(LOG_ERR, writer_name, &ci, cl, "opcode : %u != %u || %u || %u\n", s->ayh.ayh_opcode, ayiya_op_forward, ayiya_op_echo_request, ayiya_op_echo_request_forward);
			continue;
		}

		if (memcmp(&s->identity, &ayiya_ipv6_pop, sizeof(s->identity)) != 0)
		{
			memset(buf, 0, sizeof(buf));
			inet_ntop(AF_INET6, &s->identity, (char *)&buf, sizeof(buf));
			ayiya_log(LOG_WARNING, writer_name, &ci, cl, "Received packet from a wrong identity \"%s\"\n", buf);
			continue;
		}
	
		/* Verify the epochtime */
		i = tic_checktime(ntohl(s->ayh.ayh_epochtime));
		if (i != 0)
		{
			memset(buf, 0, sizeof(buf));
			inet_ntop(AF_INET6, &s->identity, (char *)&buf, sizeof(buf));
			ayiya_log(LOG_WARNING, writer_name, &ci, cl, "Time is %d seconds off for %s\n", i, buf);
			continue;
		}
	
		/* How long is the payload? */
		payloadlen = n - (sizeof(*s) - sizeof(s->payload));
	
		/* Save their hash */
		memcpy(&their_hash, &s->hash, sizeof(their_hash));

		/* Copy in our SHA1 hash */
		memcpy(&s->hash, &ayiya_hash, sizeof(s->hash));

		/* Generate a SHA1 of the header + identity + shared secret */
		SHA1_Init(&sha1);
		/* Hash the Packet */
		SHA1_Update(&sha1, (sha1_byte *)s, n);
		/* Store the hash */
		SHA1_Final(our_hash, &sha1);

		memcpy(&s->hash, &our_hash, sizeof(s->hash));

		/* Compare the SHA1's */
		if (memcmp(&their_hash, &our_hash, sizeof(their_hash)) != 0)
		{
			ayiya_log(LOG_WARNING, writer_name, &ci, cl, "Incorrect Hash received\n");
			continue;
		}

		if (s->ayh.ayh_nextheader == IPPROTO_IPV6)
		{
			/* Verify that this is really IPv6 */
			if (s->payload[0] >> 4 != 6)
			{
				ayiya_log(LOG_ERR, writer_name, &ci, cl, "Received packet didn't start with a 6, thus is not IPv6\n");
				continue;
			}

			/* Forward the packet to the kernel */
			tun_write(s->payload, payloadlen);
		}
	}

	/* Tun/TAP device is not running anymore */
	g_aiccu->tunrunning = false;

#ifndef _WIN32
	return NULL;
#else
	return 0;
#endif
}

/* Construct a beat and send it outwards */
void ayiya_beat(void)
{
	SHA_CTX			sha1;
	sha1_byte		hash[SHA1_DIGEST_LENGTH];
	struct sockaddr_in	target;
	struct pseudo_ayh	s;
	int			lenout, n;

	/* We tunnel over IPv4 */
	memcpy(&target.sin_addr, &ayiya_ipv4_pop, sizeof(target.sin_addr));
	target.sin_family	= AF_INET;
	target.sin_port		= htons(atoi(AYIYA_PORT));

	/* Prefill some standard AYIYA values */
	memset(&s, 0, sizeof(s));
	s.ayh.ayh_idlen		= 4;			/* 2^4 = 16 bytes = 128 bits (IPv6 address) */
	s.ayh.ayh_idtype	= ayiya_id_integer;
	s.ayh.ayh_siglen	= 5;			/* 5*4 = 20 bytes = 160 bits (SHA1) */
	s.ayh.ayh_hshmeth	= ayiya_hash_sha1;
	s.ayh.ayh_autmeth	= ayiya_auth_sharedsecret;
	s.ayh.ayh_opcode	= ayiya_op_noop;
	s.ayh.ayh_nextheader	= IPPROTO_NONE;

	/* Our IPv6 side of this tunnel */
	memcpy(&s.identity, &ayiya_ipv6_local, sizeof(s.identity));

	/* No Payload */

	/* Fill in the current time */
	s.ayh.ayh_epochtime = htonl((u_long)time(NULL));

	/* Our IPv6 side of this tunnel */
	memcpy(&s.identity, &ayiya_ipv6_local, sizeof(s.identity));

	/*
	 * The hash of the shared secret needs to be in the
	 * spot where we later put the complete hash
	 */
        memcpy(&s.hash, ayiya_hash, sizeof(s.hash));

	/* Generate a SHA1 */
	SHA1_Init(&sha1);
	/* Hash the complete AYIYA packet */
	SHA1_Update(&sha1, (sha1_byte *)&s, sizeof(s)-sizeof(s.payload));
	/* Store the hash in the packets hash */
	SHA1_Final(hash, &sha1);

	/* Store the hash in the actual packet */
	memcpy(&s.hash, &hash, sizeof(s.hash));

	/* Send it onto the network */
	n = sizeof(s)-sizeof(s.payload);
#if defined(_FREEBSD) || defined(_DFBSD) || defined(_OPENBSD) || defined(_DARWIN) || defined(_NETBSD)
	lenout = send(ayiya_socket->socket, (const char *)&s, (unsigned int)n, 0);
#else
	lenout = sendto(ayiya_socket->socket, (const char *)&s, (unsigned int)n, 0, (struct sockaddr *)&target, sizeof(target));
#endif
	if (lenout < 0)
	{
		ayiya_log(LOG_ERR, beat_name, NULL, 0, "Error (%d) while sending %u bytes sent to network: %s (%d)\n", lenout, n, strerror(errno), errno);
	}
	else if (n != lenout)
	{
		ayiya_log(LOG_ERR, beat_name, NULL, 0, "Only %u of %u bytes sent to network: %s (%d)\n", lenout, n, strerror(errno), errno);
	}
}

bool ayiya(struct TIC_Tunnel *hTunnel)
{
        SHA_CTX			sha1;
	struct addrinfo hints, *res, *ressave;
#ifndef _WIN32
	pthread_t		thread;
#else
	DWORD			pID;
	HANDLE			h;
#endif

	/* Setup the tunnel */
	if (!tun_start(&ayiya_tun)) return false;

	/* Resolve hTunnel entries */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(hTunnel->sIPv4_POP, AYIYA_PORT, &hints, &res) != 0)
	{
		dolog(LOG_ERR, "Couldn't resolve PoP IPv4 %s\n", hTunnel->sIPv4_POP);
		return false;
	}
	ressave = res;
        while (res)
        {
		if (res->ai_family != AF_INET)
		{
			res = res->ai_next;
			continue;
		}
		memcpy(&ayiya_ipv4_pop, &((struct sockaddr_in *)res->ai_addr)->sin_addr, 4);
		break;
        }
        freeaddrinfo(ressave);
        if (res == NULL)
        {
		dolog(LOG_ERR, "No valid IPv4 address for PoP address %s could be found\n", hTunnel->sIPv4_POP);
		return false;
        }

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	if (getaddrinfo(hTunnel->sIPv6_Local, NULL, &hints, &res) != 0)
	{
		dolog(LOG_ERR, "Couldn't resolve Local IPv6 %s\n", hTunnel->sIPv6_Local);
		return false;
	}
	ressave = res;
        while (res)
        {
		if (res->ai_family != AF_INET6)
		{
			res = res->ai_next;
			continue;
		}
		memcpy(&ayiya_ipv6_local, &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
		break;
        }
        freeaddrinfo(ressave);
        if (res == NULL)
        {
		dolog(LOG_ERR, "No valid IPv6 address for Local IPv6 address %s could be found\n", hTunnel->sIPv6_Local);
		return false;
        }

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	if (getaddrinfo(hTunnel->sIPv6_POP, NULL, &hints, &res) != 0)
	{
		dolog(LOG_ERR, "Couldn't resolve Local IPv6 %s\n", hTunnel->sIPv6_POP);
		return false;
	}
	ressave = res;
        while (res)
        {
		if (res->ai_family != AF_INET6)
		{
			res = res->ai_next;
			continue;
		}
		memcpy(&ayiya_ipv6_pop, &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr, 16);
		break;
        }
        freeaddrinfo(ressave);
        if (res == NULL)
        {
		dolog(LOG_ERR, "No valid IPv6 address for POP IPv6 address %s could be found\n", hTunnel->sIPv6_POP);
		return false;
        }

	if (!hTunnel->sPassword)
	{
		dolog(LOG_ERR, "A password is required for using AYIYA tunnels\n");
		return false;
	}

	/* Generate a SHA1 of the shared secret */
	SHA1_Init(&sha1);
	SHA1_Update(&sha1, (const sha1_byte *)hTunnel->sPassword, (unsigned int)strlen(hTunnel->sPassword));
	SHA1_Final(ayiya_hash, &sha1);

	/* Setup listening socket */
	ayiya_socket = connect_client(hTunnel->sIPv4_POP , AYIYA_PORT, AF_INET, SOCK_DGRAM);
	if (!ayiya_socket)
	{
		ayiya_log(LOG_ERR, "start", NULL, 0, "Connection error:: could not create connection to AYIYA server\n");
		return false;
	}

	/* Let AICCU configure the thing */
	if (!aiccu_setup(hTunnel, false))
	{
		return false;
	}

	/* Show that we have started */
	ayiya_log(LOG_INFO, "start", NULL, 0, "Anything in Anything (%s)\n", AYIYA_VERSION);

	/* Launch a thread for reader */
#ifndef _WIN32
	pthread_create(&thread, NULL, ayiya_writer, (void *)hTunnel);
#else
	h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ayiya_writer, hTunnel, 0, &pID);
#endif

	return true;
}
