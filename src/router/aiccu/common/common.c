/**********************************************************
 SixXS - Automatic IPv6 Connectivity Configuration Utility
***********************************************************
 Copyright 2003-2005 SixXS - http://www.sixxs.net
***********************************************************
 common/common.c - Common Functions
***********************************************************
 $Author: jeroen $
 $Id: common.c,v 1.14 2006-12-21 14:08:50 jeroen Exp $
 $Date: 2006-12-21 14:08:50 $
**********************************************************/

/* Dirty dependency for Windows:GUI version */
#ifdef _WIN32
#ifndef AICCU_CONSOLE
#include "../windows-gui/stdafx.h"
#include "../windows-gui/AICCUApp.h"
extern CAICCUApp theApp;
#endif
#endif

#include "aiccu.h"
#include "common.h"

/* getline debugging? */
/*
#define E(x) x
 */
#define E(x) {}

void dologA(int level, const char *fmt, va_list ap)
{
#ifdef _WIN32
	char buf[1024];
#endif
	/* Don't show noise */
	if (g_aiccu && !g_aiccu->verbose && level == LOG_DEBUG) return;

#ifndef _WIN32
	if (g_aiccu && g_aiccu->daemonize > 0) vsyslog(LOG_LOCAL7|level, fmt, ap);
	else
	{
		vfprintf(stderr, fmt, ap);
		fflush(stderr);
	}
#else
	vsnprintf(buf, sizeof(buf), fmt, ap);

#ifndef AICCU_CONSOLE
	/* Use the debug facility */
	OutputDebugString(buf);

	/* Store it in a log file if we are running in verbose mode */
	if (g_aiccu && g_aiccu->verbose)
	{
		char	logfile[1024];
		FILE	*f;

		/* Figure out the "C:\Windows" location */
		/* as that is where we store our configuration */
		GetWindowsDirectory(logfile, sizeof(logfile));
		strncat(logfile, "\\aiccu.log", sizeof(logfile));
		f = fopen(logfile, "w+");
		if (f)
		{
			fwrite(buf, strlen(buf), 1, f);
			fclose(f);
		}
	}

	/*
	 * Always store the last message
	 * which can be displayed as errors etc.
	 */

	/* strip the \n */
	if (strlen(buf) > 0) buf[strlen(buf)-1] = '\0';
	theApp.m_sMessage = buf;
#else
	OutputDebugString("dolog() - ");
	OutputDebugString(buf);
	fprintf(stderr, "%s", buf);
#endif /* AICCU_CONSOLE */
#endif /* !_WIN32 */
}

void dolog(int level, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	dologA(level, fmt, ap);
	va_end(ap);
}

/* 
 * Check if an address is RFC1918 based
 * This allows us to warn the user that they are behind a NAT
 */
bool is_rfc1918(char *ipv4)
{
	unsigned int	addr = inet_addr(ipv4);
	bool		ret = false;



	ret = (	/* 10.0.0.0/8 */
		((addr & htonl(0xff000000)) == htonl(0x0a000000)) ||
		/* 172.16.0.0/12 */
		((addr & htonl(0xfff00000)) == htonl(0xac100000)) ||
		/* 192.168.0.0/16 */
		((addr & htonl(0xffff0000)) == htonl(0xc0a80000))) ? true : false;

	dolog(LOG_DEBUG, "is_rfc1918(%s) = %s\n", ipv4, ret ? "yes" : "false");

	return ret;
}

void sock_printf(TLSSOCKET sock, const char *fmt, ...)
{
	char		buf[2048];
	unsigned int	len = 0, done = 0;
	int		ret;

	va_list ap;
	va_start(ap, fmt);
	/* When not a socket send it to the logs */
	if (sock == NULL || sock->socket == -1) dologA(LOG_INFO, fmt, ap);
	else
	{
		/* Format the string */
		len = vsnprintf(buf, sizeof(buf), fmt, ap);

		/* Send the line(s) over the network */

		while (done < len)
		{
#ifdef AICCU_GNUTLS
			if (sock->tls_active) ret = gnutls_record_send(sock->session, &buf[done], len-done);
			else
#endif
			ret = send(sock->socket, &buf[done], len-done, 0);
			
			if (ret > 0) done+=ret;
			else break;
		}

		/* Show this as debug output */
		if (g_aiccu->verbose)
		{
			/* Strip the last \n */
			len = (int)strlen(buf);
			if (len > 0) buf[len-1] = '\0';
			/* dump the information */
			dolog(LOG_DEBUG, "sock_printf()  : \"%s\"\n", buf);
		}
	}
	va_end(ap);
}

extern char tic_buf[2048];

/* 
 * Read a line from a socket and store it in ubuf
 * Note: uses internal caching, this should be the only function
 * used to read from the sock! The internal cache is rbuf.
 */
int sock_getline(TLSSOCKET sock, char *rbuf, unsigned int rbuflen, unsigned int *filled, char *ubuf, unsigned int ubuflen)
{
	unsigned int i;
	
	if (!sock) return -1;
	
	/* A closed socket? -> clear the buffer */
	if (sock->socket == -1)
	{
		memset(rbuf, 0, rbuflen);
		*filled = 0;
		return -1;
	}

	/* Clear the caller supplied buffer, just in case */
	memset(ubuf, 0, ubuflen);

	for (;;)
	{
		E(dolog(LOG_DEBUG, "gl() - Filled %d\n", *filled);)

		/* Did we still have something in the buffer? */
		if (*filled > 0)
		{
			E(dolog(LOG_DEBUG, "gl() - Seeking newline\n");)

			/* Walk to the end or until we reach a \n */
			for (i=0; (i < (*filled-1)) && (rbuf[i] != '\n'); i++);

			E(dolog(LOG_DEBUG, "gl() - Seeking newline - end\n");)

			/* Did we find a newline? */
			if (rbuf[i] == '\n')
			{
				E(dolog(LOG_DEBUG, "gl() - Found newline at %i\n", i+1);)

				/* Newline with a Linefeed in front of it ? -> remove it */
				if (rbuf[i] == '\n' && rbuf[i-1] == '\r')
				{
					E(dolog(LOG_DEBUG, "gl() - Removing LF\n");)
					i--;
				}
				E(else dolog(LOG_DEBUG, "gl() - No LF\n");)

				/* Copy this over to the caller */
				memcpy(ubuf, rbuf, i);

				E(dolog(LOG_DEBUG, "gl() - Copied %d bytes from %x to %x\n", i, rbuf, ubuf);)

				/* Count the \r if it is there */
				if (rbuf[i] == '\r') i++;
				/* Count the \n */
				i++;

				/* filled = what is left in the buffer */
				*filled -= i;
				
				E(dolog(LOG_DEBUG, "gl() - %d bytes left in the buffer\n", *filled);)

				/* Now move the rest of the buffer to the front */
				if (*filled > 0) memmove(rbuf, &rbuf[i], *filled);
				else *filled = 0;

				/* Show this as debug output */
				if (g_aiccu->verbose) dolog(LOG_DEBUG, "sock_getline() : \"%s\"\n", ubuf);

				/* We got ourselves a line in 'buf' thus return to the caller */
				return i;
			}
		}

		E(dolog(LOG_DEBUG, "gl() - Trying to receive (max=%d)...\n", rbuflen-*filled-10);)

		/* Fill the rest of the buffer */
#ifdef AICCU_GNUTLS
		if (sock->tls_active) i = gnutls_record_recv(sock->session, &rbuf[*filled], rbuflen-*filled-10);
		else
#endif
		i = recv(sock->socket, &rbuf[*filled], rbuflen-*filled-10, 0);

		E(dolog(LOG_DEBUG, "gl() - Received %d\n", i);)

		/* Fail on errors */
		if (i <= 0) return -1;

		/* We got more filled space! */
		*filled+=i;

		/* Buffer overflow? */
		if (	*filled >= (rbuflen-10) ||
			*filled >= (ubuflen-10) )
		{
			dolog(LOG_ERR, "Buffer almost flowed over without receiving a newline\n");
			return -1;
		}

		/* And try again in this loop ;) */
	}

	/* Never reached */
	return -1;
}

TLSSOCKET sock_alloc(void);
TLSSOCKET sock_alloc(void)
{
#ifdef AICCU_GNUTLS
	/* Allow connections to servers that have OpenPGP keys as well */
	const int	cert_type_priority[3] = { GNUTLS_CRT_X509, GNUTLS_CRT_OPENPGP, 0 };
	int		ret;
#endif /* AICCU_GNUTLS*/

	TLSSOCKET	sock;	

	sock = (TLSSOCKET)malloc(sizeof(*sock));
	if (!sock) return NULL;
	
	sock->socket = -1;

#ifdef AICCU_GNUTLS
	/* TLS is not active yet (use sock_gotls() for that) */
	sock->tls_active = false;

	/* Initialize TLS session */
	ret = gnutls_init(&sock->session, GNUTLS_CLIENT);
	if (ret != 0)
	{
		dolog(LOG_ERR, "TLS Init failed: %s (%d)\n", gnutls_strerror(ret), ret);
		free(sock);
		return NULL;
	}

	/* Use default priorities */
	gnutls_set_default_priority(sock->session);
	/* XXX: Return value is not documented in GNUTLS documentation! */

	gnutls_certificate_type_set_priority(sock->session, cert_type_priority);
	/* XXX: Return value is not documented in GNUTLS documentation! */

	/* Configure the x509 credentials for the current session */
	gnutls_credentials_set(sock->session, GNUTLS_CRD_CERTIFICATE, g_aiccu->tls_cred);
	/* XXX: Return value is not documented in GNUTLS documentation! */

#endif /* AICCU_GNUTLS*/

	return sock;
}

void sock_free(TLSSOCKET sock)
{
	if (!sock) return;
	
#ifdef AICCU_GNUTLS
	if (sock->tls_active)
	{
		sock->tls_active = false;
		gnutls_bye(sock->session, GNUTLS_SHUT_RDWR);
	}
#endif /* AICCU_GNUTLS*/

	if (sock->socket >= 0)
	{
		/* Stop communications */
		shutdown(sock->socket, SHUT_RDWR);
		closesocket(sock->socket);
		sock->socket = -1;
	}

#ifdef AICCU_GNUTLS
	gnutls_deinit(sock->session);
#endif /* AICCU_GNUTLS*/

	free(sock);
}

/* Connect this client to a server */
TLSSOCKET connect_client(const char *hostname, const char *service, int family, int socktype)
{
	TLSSOCKET	sock;
	struct addrinfo	hints, *res, *ressave;
	
	sock = sock_alloc();
	if (!sock) return NULL;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = socktype;

	if (getaddrinfo(hostname, service, &hints, &res) != 0)
	{
		dolog(LOG_ERR, "Couldn't resolve host %s, service %s\n", hostname, service);
		sock_free(sock);
		return NULL;
	}

	ressave = res;

	while (res)
	{
		sock->socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock->socket == -1) continue;
		if (connect(sock->socket, res->ai_addr, (unsigned int)res->ai_addrlen) == 0) break;
		closesocket(sock->socket);
		sock->socket = -1;
		res = res->ai_next;
	}

	freeaddrinfo(ressave);
	
	if (sock->socket == -1)
	{
		sock_free(sock);
		sock = NULL;
	}

	return sock;
}

TLSSOCKET listen_server(const char *description, const char *hostname, const char *service, int family, int socktype)
{
	struct addrinfo	hints, *res, *ressave;
	int		n;
	TLSSOCKET	sock;
	socklen_t	on = 1;
/*
	D(dolog(LOG_DEBUG, "[%s] Trying to get socket for [%s]:%s over %s (%d) using %s (%d)\n",
		description, hostname, service,
		family == AF_INET ? "IPv4" : (family == AF_INET6 ? "IPv6" : "??"),
		family,
		socktype == IPPROTO_UDP ? "UDP" : (socktype == IPPROTO_TCP ? "TCP" : "??"),
		socktype);)
*/
	sock = sock_alloc();
	if (!sock) return NULL;

	memset(&hints, 0, sizeof(struct addrinfo));

	/* AI_PASSIVE flag: the resulting address is used to bind
 	   to a socket for accepting incoming connections.
	   So, when the hostname==NULL, getaddrinfo function will
  	   return one entry per allowed protocol family containing
	   the unspecified address for that family. */

	hints.ai_flags    = AI_PASSIVE;
	hints.ai_family   = family;
	hints.ai_socktype = socktype;

	n = getaddrinfo(hostname, service, &hints, &res);
	if (n < 0)
	{
		dolog(LOG_ERR, "[%s] listen_server setup: getaddrinfo error: %s\n", description, gai_strerror(n));
		sock_free(sock);
		return NULL;
	}

	ressave=res;

	/* Try to open socket with each address getaddrinfo returned,
 	   until we get one valid listening socket. */
	while (res)
	{
		sock->socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (!(sock->socket < 0))
		{
			setsockopt(sock->socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on));
			if (bind(sock->socket, res->ai_addr, (unsigned int)res->ai_addrlen) == 0) break;
			closesocket(sock->socket);
			sock->socket = -1;
		}
		res = res->ai_next;
	}

	freeaddrinfo(ressave);

	if (sock->socket < 0)
	{
		dolog(LOG_ERR, "[%s] listen setup: socket error: could not open socket\n", description);
		sock_free(sock);
		return NULL;
	}

	if (listen(sock->socket, LISTEN_QUEUE) == -1)
	{
		dolog(LOG_ERR, "[%s] listen setup: socket error: could not listen on socket\n", description);
		sock_free(sock);
		return NULL;
	}

	dolog(LOG_INFO, "[%s] Listening on [%s]:%s\n", description, hostname, service);

	return sock;
}

/*
 * Put a socket into TLS mode
 */
#ifdef AICCU_GNUTLS
bool sock_gotls(TLSSOCKET sock)
{
	int ret = 0;
	
	if (!sock) return false;
	
	if (sock->tls_active)
	{
		dolog(LOG_ERR, "Can't go into TLS mode twice!?\n");
		return false;
	}

	/* Set the transport */
	gnutls_transport_set_ptr(sock->session, (gnutls_transport_ptr)sock->socket);

	/* Perform the TLS handshake */
	ret = gnutls_handshake(sock->session);
	if (ret < 0)
	{
		dolog(LOG_ERR, "TLS Handshake failed: %s (%d)\n", gnutls_strerror(ret), ret);
		return false;
	}

	dolog(LOG_DEBUG, "TLS Handshake completed succesfully\n");

	sock->tls_active = true;
	return true;
}
#endif

/* Count the number of fields in <s> */
unsigned int countfields(char *s)
{
	int n = 1, i;
	if (s == NULL) return 0;
	for (i=0; s[i] != '\0'; i++) if (s[i] == ' ') n++;
	return n;
}

/*
 * Copy field <n> of string <s> into <buf> with a maximum of buflen
 * First field is 1
 */
bool copyfield(char *s, unsigned int n, char *buf, unsigned int buflen)
{
	unsigned int begin = 0, i=0;

	/* Clear the buffer */
	memset(buf, 0, buflen);

	while (s[i] != '\0')
	{
		n--;
		begin = i;

		/* Find next delimiter */
		for (; s[i] != '\0' && s[i] != ' '; i++);

		if (n == 0)
		{
			i-=begin;
			strncpy(buf, s+begin, i > buflen ? buflen : i);
			/* dolog(LOG_DEBUG, "copyfield() : '%s', begin = %d, len = %d\n", buf, begin, i); */
			return true;
		}
		
		i++;
	}
	dolog(LOG_WARNING, "copyfield() - Field %u didn't exist in '%s'\n", n, s);
	return false;
}

bool parseline(char *line, const char *split, struct pl_rule *rules, void *data)
{
	unsigned int	r, len;
	char		*end = NULL, *val = NULL, *p = NULL;
	void		*store;

	/* Chop off \n and \r and white space */
	p = &line[strlen(line)-1];
	while (	p >= line && (
		*p == '\n' ||
		*p == '\r' ||
		*p == '\t' ||
		*p == ' ')) *p-- = '\0';

	/* Ignore comments and emtpy lines */
	if (	strlen(line) == 0 ||
		line[0] == '#' ||
		line[0] == ';' ||
		(line[0] == '/' && line[1] == '/'))
	{
		return true;
	}

	/* Get the end of the first argument */
	p = line;
	end = &line[strlen(line)-1];
	/* Skip until whitespace */
	while (	p < end &&
		strncmp(p, split, strlen(split)) != 0) p++;
	/* Terminate this argument */
	*p = '\0';
	p++;

	/* Skip whitespace */
	while ( p < end &&
		*p == ' ' &&
		*p == '\t') p++;

	/* Start of the value */
	val = p+(strlen(split)-1);

	/* If starting with quotes, skip until next quote */
	if (*p == '"' || *p == '\'')
	{
		p++;
		/* Find next quote */
		while (p <= end &&
			*p != *val &&
			*p != '\0') p++;
		/* Terminate */
		*p = '\0';
		/* Skip the first quote */
		val++;
	}
	/* Otherwise it is already terminated above */

	/* Walk through all the rules */
	for (r = 0; rules[r].type != PLRT_END; r++)
	{
		len = (int)strlen(rules[r].title);
		if (strncmp(line, rules[r].title, len) != 0) continue;

		store = (void *)((char *)data + rules[r].offset);

		switch (rules[r].type)
		{
		case PLRT_STRING:
			if (*((char **)store)) free(*((char **)store));
			*((char **)store) = strdup(val);
			break;

		case PLRT_INTEGER:
			*((uint32_t *)store) = atoi(val);
			break;

		case PLRT_BOOL:
			if (	strcmp(val, "yes") == 0 ||
				strcmp(val, "true") == 0)
			{
				*((bool *)store) = true;
			}
			else if (strcmp(val, "no") == 0 ||
				strcmp(val, "false") == 0)
			{
				*((bool *)store) = false;
			}
			else
			{
				dolog(LOG_WARNING, "Unknown boolean value \"%s\" for option \"%s\"\n", val, rules[r].title);
			}
			break;

		case PLRT_IPV4:
		 	inet_pton(AF_INET, val, store);
			break;

		case PLRT_IPV6:
		 	inet_pton(AF_INET6, val, store);
			break;

		case PLRT_END:
			return false;
		}
		return true;
	}
	return false;
}

/* 
 * MD5 a string
 * sSignature's size MUST be 32 bytes!
 */
void MD5String(const char *sString, char *sSignature, unsigned int siglen)
{
	struct MD5Context	md5c;
	unsigned char		signature[16];
	unsigned int		i;
	
	if (siglen < 32) return;

	/* Initialize MD5 structure */
	MD5Init(&md5c);
	/* Calculate MD5 of the string */
	MD5Update(&md5c, (unsigned char *)sString, (unsigned int)strlen(sString));
	MD5Final(signature, &md5c);
	
	memset(sSignature, 0, siglen);

	for (i=0; i < sizeof(signature); i++)
	{
		snprintf(&sSignature[i*2], 3, "%02x", signature[i]);
	}
}

#ifdef _AIX
/* AIX doesn't have vsyslog() thus we implement it here */
void vsyslog(int priority, const char *format, va_list ap)
{
        char buf[1024];
        vsnprintf(buf, sizeof(buf), format, ap);
	syslog(priority, buf);
}
#endif

#ifdef _WIN32
const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
{
	if (af == AF_INET)
	{
		struct sockaddr_in in;
		memset(&in, 0, sizeof(in));
		in.sin_family = AF_INET;
		memcpy(&in.sin_addr, src, sizeof(struct in_addr));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	else if (af == AF_INET6)
	{
		struct sockaddr_in6 in;
		memset(&in, 0, sizeof(in));
		in.sin6_family = AF_INET6;
		memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	return NULL;
}

int inet_pton(int af, const char *src, void *dst)
{
	struct addrinfo	hints, *res, *ressave;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = af;

	if (getaddrinfo(src, NULL, &hints, &res) != 0)
	{
		dolog(LOG_ERR, "Couldn't resolve host %s\n", src);
		return -1;
	}

	ressave = res;

	while (res)
	{
		/* Check if AF is correct */
		if (res->ai_family != af)
		{
			res = res->ai_next;
			continue;
		}

		/* This is the one we want */
		memcpy(dst, res->ai_addr, af == AF_INET6 ? sizeof(struct in_addr6) : sizeof(struct in_addr));

		/* We only need one */
		break;
	}

	freeaddrinfo(ressave);
	return 0;
}

#endif
