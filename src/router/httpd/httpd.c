/* milli_httpd - pretty small HTTP server
** A combination of
** micro_httpd - really small HTTP server
** and
** mini_httpd - small HTTP server
**
** Copyright  1999,2000 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/
#define _GNU_SOURCE

#include "modules/combine.c"

static int wfsendfile(int fd, off_t offset, size_t nbytes, webs_t wp);
static char *wfgets(char *buf, int len, webs_t fp, int *eof);
size_t websWrite(webs_t wp, char *fmt, ...);
size_t wfwrite(void *buf, size_t size, size_t n, webs_t fp);
static size_t wfread(void *buf, size_t size, size_t n, webs_t fp);
static int wfclose(webs_t fp);
int wfflush(webs_t fp);
#ifndef VALIDSOURCE
#ifndef VISUALSOURCE

int wfputs(char *buf, webs_t fp);
#endif
#endif
/* Basic authorization userid and passwd limit */

static void send_authenticate(webs_t conn_fp);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <stdarg.h>
#include <syslog.h>
#include <cy_conf.h>
#include "httpd.h"
#include <bcmnvram.h>
#include <code_pattern.h>
#include <utils.h>
#include <shutils.h>
#include <sys/time.h>
#include <signal.h>
#include <airbag.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include <wlutils.h>
#ifndef HAVE_MICRO
#include <semaphore.h>
#endif
#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#endif

#ifdef HAVE_MATRIXSSL
#include <matrixSsl.h>
#include <matrixssl_xface.h>
#endif

#ifdef HAVE_POLARSSL
#include "polarssl/entropy.h"
#include <polarssl/ctr_drbg.h>
#include <polarssl/certs.h>
#include <polarssl/x509.h>
#include <polarssl/ssl.h>
#include <polarssl/net.h>
//#include <xyssl_xface.h>
#endif

#ifdef __UCLIBC__
#include <error.h>
#endif
#include <sys/signal.h>
#include <ucontext.h>

#ifdef HAVE_IAS
#include <sys/sysinfo.h>
#endif
#ifndef HAVE_MICRO
#include <pthread.h>
#endif

#include <crypt.h>

#define SERVER_NAME "httpd"
#define PROTOCOL "HTTP/1.1"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define TIMEOUT 5

#ifdef HAVE_IPV6
#define USE_IPV6 1
#endif
/* A multi-family sockaddr. */
typedef union {
	struct sockaddr sa;
	struct sockaddr_in sa_in;
#ifdef USE_IPV6
	struct sockaddr_in6 sa_in6;
	struct sockaddr_storage sa_stor;
#endif /* USE_IPV6 */
} usockaddr;

/* Globals. */
#ifdef FILTER_DEBUG
FILE *debout;
#endif

#if defined(TCP_CORK) && !defined(TCP_NOPUSH)
#define TCP_NOPUSH TCP_CORK
/* (Linux's TCP_CORK is basically the same as BSD's TCP_NOPUSH.) */
#endif

#if defined(HAVE_OPENSSL) || defined(HAVE_MATRIXSSL) || defined(HAVE_POLARSSL)

#define DEFAULT_HTTPS_PORT 443
#define CERT_FILE "/etc/cert.pem"
#define KEY_FILE "/etc/key.pem"
#endif

#ifdef HAVE_POLARSSL
static int my_ciphers[] = { TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
			    TLS_DHE_RSA_WITH_AES_256_CBC_SHA,
			    TLS_RSA_WITH_AES_256_CBC_SHA,
			    TLS_RSA_WITH_RC4_128_MD5,
			    TLS_RSA_WITH_RC4_128_SHA,
			    TLS_RSA_WITH_AES_128_CBC_SHA256,
			    TLS_RSA_WITH_AES_256_CBC_SHA256,
			    TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
			    TLS_DHE_RSA_WITH_AES_256_CBC_SHA256,
			    TLS_RSA_WITH_AES_128_GCM_SHA256,
			    TLS_RSA_WITH_AES_256_GCM_SHA384,
			    0 };

static char *dhm_P = "E4004C1F94182000103D883A448B3F80"
		     "2CE4B44A83301270002C20D0321CFD00"
		     "11CCEF784C26A400F43DFB901BCA7538"
		     "F2C6B176001CF5A0FD16D2C48B1D0C1C"
		     "F6AC8E1DA6BCC3B4E1F96B0564965300"
		     "FFA1D0B601EB2800F489AA512C4B248C"
		     "01F76949A60BB7F00A40B1EAB64BDD48"
		     "E8A700D60B7F1200FA8E77B0A979DABF";

static char *dhm_G = "4";
//unsigned char session_table[SSL_SESSION_TBL_LEN];
#endif
struct nvram_param *srouter_defaults;

extern struct nvram_param *load_defaults(void);
extern void free_defaults(struct nvram_param *);

#ifdef HAVE_REGISTER
static dd_atomic8_t registered = -1;
static dd_atomic8_t registered_real = -1;
#endif
#ifdef HAVE_SUPERCHANNEL
static dd_atomic8_t superchannel = -1;
#endif
#ifdef HAVE_REGISTER
#define IS_REGISTERED(conn_fp) (conn_fp->isregistered)
#define IS_REGISTERED_REAL(conn_fp) (conn_fp->isregistered_real)
#else
#define IS_REGISTERED(conn_fp) (1)
#define IS_REGISTERED_REAL(conn_fp) (1)
#endif

int isregistered(void);
int isregistered_real(void);

#define DEFAULT_HTTP_PORT 80
static char pid_file[80];
static char *server_dir = NULL;

extern char *get_mac_from_ip(char *mac, char *ip);

/* Forwards. */
static int initialize_listen_socket(usockaddr *usaP);
static int auth_check(webs_t conn_fp);
static void send_error(webs_t conn_fp, int noheader, int status, char *title, char *extra_header, const char *fmt, ...);
static void send_headers(webs_t conn_fp, int status, char *title, char *extra_header, char *mime_type, int length,
			 char *attach_file, int nocache);
static int b64_decode(const char *str, unsigned char *space, int size);
static int match(const char *pattern, const char *string);
static int match_one(const char *pattern, int patternlen, const char *string);
static void *handle_request(void *conn_fp);

#if !defined(HAVE_MICRO) && !defined(__UCLIBC__)

#define PTHREAD_MUTEX_INIT pthread_mutex_init
#define PTHREAD_MUTEX_LOCK pthread_mutex_lock
#define PTHREAD_MUTEX_UNLOCK pthread_mutex_unlock
#define PTHREAD_MUTEX_TRYLOCK pthread_mutex_trylock

#define SEM_WAIT sem_wait
#define SEM_POST sem_post
#define SEM_INIT sem_init

#define HTTP_MAXCONN 16
static sem_t semaphore;
#ifdef __UCLIBC__
static pthread_mutex_t crypt_mutex;
#endif
#ifndef HAVE_MUSL
static pthread_mutex_t httpd_mutex;
#endif
static pthread_mutex_t input_mutex;

#ifdef __UCLIBC__
#define CRYPT_MUTEX_INIT pthread_mutex_init
#define CRYPT_MUTEX_LOCK pthread_mutex_lock
#define CRYPT_MUTEX_UNLOCK pthread_mutex_unlock
#else
#define CRYPT_MUTEX_INIT(m, v) \
	do {                   \
	} while (0)
#define CRYPT_MUTEX_LOCK(m) \
	do {                \
	} while (0)
#define CRYPT_MUTEX_UNLOCK(m) \
	do {                  \
	} while (0)

#endif

#else
#define PTHREAD_MUTEX_INIT(m, v) \
	do {                     \
	} while (0)
#define PTHREAD_MUTEX_LOCK(m) \
	do {                  \
	} while (0)
#define PTHREAD_MUTEX_UNLOCK(m) \
	do {                    \
	} while (0)
#define PTHREAD_MUTEX_TRYLOCK(m) \
	do {                     \
	} while (0)
#define SEM_WAIT(sem) \
	do {          \
	} while (0)
#define SEM_POST(sem) \
	do {          \
	} while (0)
#define SEM_INIT(sem, p, v) \
	do {                \
	} while (0)
#define CRYPT_MUTEX_INIT(m, v) \
	do {                   \
	} while (0)
#define CRYPT_MUTEX_LOCK(m) \
	do {                \
	} while (0)
#define CRYPT_MUTEX_UNLOCK(m) \
	do {                  \
	} while (0)
#endif

static void lookup_hostname(usockaddr *usa4P, size_t sa4_len, int *gotv4P, usockaddr *usa6P, size_t sa6_len, int *gotv6P, int port)
{
#ifdef USE_IPV6

	struct addrinfo hints;
	char portstr[10];
	int gaierr;
	struct addrinfo *ai;
	struct addrinfo *ai2;
	struct addrinfo *aiv6;
	struct addrinfo *aiv4;

	(void)memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	(void)snprintf(portstr, sizeof(portstr), "%d", (int)port);
	if ((gaierr = getaddrinfo(NULL, portstr, &hints, &ai)) != 0) {
		exit(1);
	}

	/* Find the first IPv6 and IPv4 entries. */
	aiv6 = NULL;
	aiv4 = NULL;
	for (ai2 = ai; ai2 != NULL; ai2 = ai2->ai_next) {
		switch (ai2->ai_family) {
		case AF_INET6:
			if (!aiv6)
				aiv6 = ai2;
			break;
		case AF_INET:
			if (!aiv4)
				aiv4 = ai2;
			break;
		}
	}

	if (!aiv6)
		*gotv6P = 0;
	else {
		if (sa6_len < aiv6->ai_addrlen) {
			exit(1);
		}
		(void)memset(usa6P, 0, sa6_len);
		(void)memmove(usa6P, aiv6->ai_addr, aiv6->ai_addrlen);
		*gotv6P = 1;
	}

	if (!aiv4)
		*gotv4P = 0;
	else {
		if (sa4_len < aiv4->ai_addrlen) {
			exit(1);
		}
		(void)memset(usa4P, 0, sa4_len);
		(void)memmove(usa4P, aiv4->ai_addr, aiv4->ai_addrlen);
		*gotv4P = 1;
	}

	freeaddrinfo(ai);

#else /* USE_IPV6 */

	struct hostent *he;

	*gotv6P = 0;

	(void)memset(usa4P, 0, sa4_len);
	usa4P->sa.sa_family = AF_INET;
	usa4P->sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
	usa4P->sa_in.sin_port = htons(port);
	*gotv4P = 1;

#endif /* USE_IPV6 */
}

static int sockaddr_check(usockaddr *usaP)
{
	switch (usaP->sa.sa_family) {
	case AF_INET:
		return 1;
#ifdef USE_IPV6
	case AF_INET6:
		return 1;
#endif /* USE_IPV6 */
	default:
		return 0;
	}
}

static size_t sockaddr_len(usockaddr *usaP)
{
	switch (usaP->sa.sa_family) {
	case AF_INET:
		return sizeof(struct sockaddr_in);
#ifdef USE_IPV6
	case AF_INET6:
		return sizeof(struct sockaddr_in6);
#endif /* USE_IPV6 */
	default:
		return 0; /* shouldn't happen */
	}
}

static void setnaggle(webs_t wp, int on)
{
	int r;

	if (!DO_SSL(wp)) {
#if defined(TCP_NOPUSH)
		/* Set the TCP_NOPUSH socket option, to try and avoid the 0.2 second
		 ** delay between sending the headers and sending the data.  A better
		 ** solution is writev() (as used in thttpd), or send the headers with
		 ** send(MSG_MORE) (only available in Linux so far).
		 */
		r = on;
		(void)setsockopt(wp->conn_fd, IPPROTO_TCP, TCP_NOPUSH, (void *)&r, sizeof(r));
#endif
		if (on) {
			r = 1;
			(void)setsockopt(wp->conn_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&r, sizeof(r));
		}
	}
}

static void set_ndelay(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		return;
	}
	fcntl(fd, F_SETFL, flags | O_NDELAY);
}

static void clear_ndelay(int fd)
{
	int newflags;
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		return;
	}
	newflags = flags & ~(int)O_NDELAY;
	if (newflags != flags) {
		fcntl(fd, F_SETFL, newflags);
	}
}

static int initialize_listen_socket(usockaddr *usaP)
{
	int listen_fd;
	int i;
	int flags;
	/* Check sockaddr. */
	if (!sockaddr_check(usaP)) {
		return -1;
	}

	listen_fd = socket(usaP->sa.sa_family, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		return -1;
	}

	(void)fcntl(listen_fd, F_SETFD, 1);

	i = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&i, sizeof(i)) < 0) {
		dd_logerror("httpd", "setsockopt SO_REUSEADDR - %m");
		perror("setsockopt SO_REUSEADDR");
		return -1;
	}

	if (bind(listen_fd, &usaP->sa, sockaddr_len(usaP)) < 0) {
		return -1;
	}

	set_ndelay(listen_fd);
	if (listen(listen_fd, 1024) < 0) {
		dd_logerror("httpd", "listen - %m");
		perror("listen");
		return -1;
	}

	return listen_fd;
}

#ifdef __UCLIBC__
struct crypt_data {
	char __buf[256];
};

static char *crypt_r(const char *authinfo, const char *authdata, struct crypt_data *data)
{
	CRYPT_MUTEX_LOCK(&crypt_mutex);
	char *enc1 = crypt(authinfo, authdata);
	strcpy(data->__buf, enc1);
	CRYPT_MUTEX_UNLOCK(&crypt_mutex);
	return data->__buf;
}
#endif

static int auth_check(webs_t conn_fp)
{
	char *authinfo;
	char *authpass;
	int l;
	int ret = 0;
	authinfo = malloc(500);
	/* Is this directory unprotected? */
	if (!*(conn_fp->auth_passwd)) {
		/* Yes, let the request go through. */
		ret = 1;
		goto out;
	}

	/* Basic authorization info? */
	if (!conn_fp->authorization || strncmp(conn_fp->authorization, "Basic ", 6) != 0) {
		dd_loginfo("httpd", "Authentication fail");

		goto out;
	}
	bzero(authinfo, 500);
	/* Decode it. */
	l = b64_decode(&(conn_fp->authorization[6]), (unsigned char *)authinfo, 499);
	authinfo[l] = '\0';
	/* Split into user and password. */
	authpass = strchr((char *)authinfo, ':');
	if (authpass == NULL) {
		/* No colon?  Bogus auth info. */
		goto out;
	}
	*authpass++ = '\0';

	char *enc1;
	char *enc2;
	struct crypt_data data;
	enc1 = crypt_r(authinfo, (const char *)conn_fp->auth_userid, &data);
	char dummy[128];
	if (!enc1 || strcmp(enc1, conn_fp->auth_userid)) {
		dd_loginfo("httpd", "httpd login failure for %s", conn_fp->http_client_ip);
		add_blocklist("httpd", conn_fp->http_client_ip);
		//              while (wfgets(dummy, 64, conn_fp, NULL)) {
		//              }
		goto out;
	}
	enc2 = crypt_r(authpass, (const char *)conn_fp->auth_passwd, &data);

	if (!enc2 || strcmp(enc2, conn_fp->auth_passwd)) {
		dd_loginfo("httpd", "httpd login failure for %s", conn_fp->http_client_ip);
		add_blocklist("httpd", conn_fp->http_client_ip);
		//              while (wfgets(dummy, 64, conn_fp, NULL)) {
		//              }
		goto out;
	}
	ret = 1;
out:;
	debug_free(authinfo);

	return ret;
}

static void send_authenticate(webs_t conn_fp)
{
	char *header;
	(void)asprintf(&header, "WWW-Authenticate: Basic realm=\"%s\"", conn_fp->auth_realm);
	send_error(conn_fp, 0, 401, live_translate(conn_fp, "share.unauthorized"), header,
		   live_translate(conn_fp, "share.auth_required"));
	debug_free(header);
}

void do_error_style(webs_t wp, int code, char *title, char *text);

static void send_error(webs_t conn_fp, int noheader, int status, char *title, char *extra_header, const char *fmt, ...)
{
	int flags, newflags;

	if (!SSL_ENABLED() || !DO_SSL(conn_fp)) {
		clear_ndelay(conn_fp->conn_fd);
	}
	char *text;
	va_list args;
	va_start(args, (char *)fmt);
	vasprintf(&text, fmt, args);
	va_end(args);
	//      dd_logerror("httpd", "Request Error Code %d: %s\n", status, text);
	// jimmy, https, 8/4/2003, fprintf -> websWrite, fflush -> wfflush
	if (!noheader)
		send_headers(conn_fp, status, title, extra_header, "text/html", -1, NULL, 1);
	char *translate = "";
	if (!nvram_match("language", "english"))
		translate = " translate=\"no\"";
	websWrite(conn_fp, "<html%s>\n", translate);
	do_error_style(conn_fp, status, title, text);

	char *charset = live_translate(conn_fp, "lang_charset.set");
	websWrite(conn_fp,
		  "<!DOCTYPE html>\n" //
		  "<head>\n" //
		  "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\" />\n",
		  charset);
	websWrite(conn_fp, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n");
	websWrite(conn_fp, "<title>%d %s</title></head>\n<body class=\"error_page\"><h4>%d %s</h4>\n", status, title, status,
		  title);
	websWrite(conn_fp, "%s\n", text);
	websWrite(conn_fp, "</body>");
	websWrite(conn_fp, "</html>\n");

	(void)wfflush(conn_fp);
	debug_free(text);
}

static void send_headers(webs_t conn_fp, int status, char *title, char *extra_header, char *mime_type, int length,
			 char *attach_file, int nocache)
{
	time_t now;
	char timebuf[100];
	websWrite(conn_fp, "%s %d %s\r\n", PROTOCOL, status, title);
	if (mime_type != NULL)
		websWrite(conn_fp, "Content-Type: %s\r\n", mime_type);

	websWrite(conn_fp, "Server: %s\r\n", SERVER_NAME);
	now = time(NULL);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	websWrite(conn_fp, "Date: %s\r\n", timebuf);
	websWrite(conn_fp, "Connection: close\r\n");
	if (nocache) {
		websWrite(conn_fp, "Cache-Control: no-store, no-cache, must-revalidate\r\n");
		websWrite(conn_fp, "Cache-Control: post-check=0, pre-check=0\r\n");
		websWrite(conn_fp, "Pragma: no-cache\r\n");
	} else {
		websWrite(conn_fp, "Cache-Control: private, max-age=600\r\n");
	}
	if (attach_file) {
		int i, len = strlen(attach_file);
		int cnt = 0;
		char *newname = malloc((len * 3) + 1);
		if (!newname) {
			dd_syslog(LOG_ERR, "out of memory in %s / attach_file\n", __func__);
			return;
		}
		for (i = 0; i < len; i++) {
			if (attach_file[i] == ' ') {
				newname[cnt++] = '%';
				newname[cnt++] = '2';
				newname[cnt++] = '0';
			} else {
				newname[cnt++] = attach_file[i];
			}
		}
		newname[cnt++] = 0;
		websWrite(conn_fp, "Content-Disposition: attachment; filename=%s\r\n", newname);
		debug_free(newname);
	}
	if (extra_header != NULL && *extra_header)
		websWrite(conn_fp, "%s\r\n", extra_header);
	if (length != -1)
		websWrite(conn_fp, "Content-Length: %ld\r\n", length);
	websWrite(conn_fp, "\r\n");
}

/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static const int b64_decode_table[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 00-0F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 10-1F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, /* 20-2F */
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, /* 30-3F */
	-1, 0,	1,  2,	3,  4,	5,  6,	7,  8,	9,  10, 11, 12, 13, 14, /* 40-4F */
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, /* 50-5F */
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /* 60-6F */
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, /* 70-7F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 80-8F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 90-9F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* A0-AF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* B0-BF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* C0-CF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* D0-DF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* E0-EF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 /* F0-FF */
};

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
static int b64_decode(const char *str, unsigned char *space, int size)
{
	const char *cp;
	int space_idx, phase;
	int d, prev_d = 0;
	unsigned char c;

	space_idx = 0;
	phase = 0;
	for (cp = str; *cp != '\0'; ++cp) {
		d = b64_decode_table[(int)*cp];
		if (d != -1) {
			switch (phase) {
			case 0:
				++phase;
				break;
			case 1:
				c = ((prev_d << 2) | ((d & 0x30) >> 4));
				if (space_idx < size)
					space[space_idx++] = c;
				++phase;
				break;
			case 2:
				c = (((prev_d & 0xf) << 4) | ((d & 0x3c) >> 2));
				if (space_idx < size)
					space[space_idx++] = c;
				++phase;
				break;
			case 3:
				c = (((prev_d & 0x03) << 6) | d);
				if (space_idx < size)
					space[space_idx++] = c;
				phase = 0;
				break;
			}
			prev_d = d;
		}
	}
	return space_idx;
}

/* Simple shell-style filename matcher.  Only does ? * and **, and multiple
** patterns separated by |.  Returns 1 or 0.
*/
static int match(const char *pattern, const char *string)
{
	const char * or ;

	char *path = strdup(string);
	char *query = strchr(path, '?');
	if (query)
		*query++ = 0;

	for (;;) {
		or = strchr(pattern, '|');
		if (or == NULL) {
			int ret = match_one(pattern, strlen(pattern), path);
			debug_free(path);
			return ret;
		}
		if (match_one(pattern, or -pattern, path)) {
			debug_free(path);
			return 1;
		}
		pattern = or +1;
	}
}

static int match_one(const char *pattern, int patternlen, const char *string)
{
	const char *p;

	for (p = pattern; p - pattern < patternlen; ++p, ++string) {
		if (*p == '?' && *string != '\0')
			continue;
		if (*p == '*') {
			int i, pl;

			++p;
			if (*p == '*') {
				/* Double-wildcard matches anything. */
				++p;
				i = strlen(string);
			} else
				/* Single-wildcard matches anything but slash. */
				i = strcspn(string, "/");
			pl = patternlen - (p - pattern);
			for (; i >= 0; --i)
				if (match_one(p, pl, &(string[i])))
					return 1;
			return 0;
		}
		if (*p != *string)
			return 0;
	}
	if (*string == '\0')
		return 1;
	return 0;
}

static int do_file_2(unsigned char method, struct mime_handler *handler, char *path, webs_t stream,
		     char *attach) //jimmy, https, 8/4/2003
{
	size_t len;
	FILE *web = _getWebsFile(stream, path, &len);
	if (!web)
		return -1;
	if (handler && !handler->send_headers) {
		send_headers(stream, 200, "OK", handler->extra_header, handler->mime_type, len, attach, 0);
		if (method == METHOD_HEAD)
			return 0;
	}
	if (DO_SSL(stream)) {
		char *buffer = malloc(4096);
		while (len && !feof(web)) {
			size_t ret = fread(buffer, 1, len > 4096 ? 4096 : len, web);
			if (ferror(web)) {
				dd_loginfo("httpd", "%s: cannot read from local file stream (%s)", __func__, strerror(errno));
				break; // deadlock prevention
			}
			if (ret > 0) {
				len -= ret;
				wfwrite(buffer, ret, 1, stream);
			}
		}
		debug_free(buffer);
	} else {
		wfflush(stream);
		wfsendfile(fileno(web), ftell(web), len, stream);
	}
	fclose(web);
	return 0;
}

int do_file(unsigned char method, struct mime_handler *handler, char *path,
	webs_t stream) //jimmy, https, 8/4/2003
{
	return do_file_2(method, handler, path, stream, NULL);
}

static int do_file_attach(struct mime_handler *handler, char *path, webs_t stream,
			  char *attachment) //jimmy, https, 8/4/2003
{
	return do_file_2(METHOD_GET, handler, path, stream, attachment);
}

static int check_connect_type_vap(const char *prefix, webs_t wp, char *nvprefix)
{
	struct wl_assoc_mac *wlmac = NULL;
	int count_wl = 0;
	int i, j;
	char temp[32];
	sprintf(temp, "%s_web_filter", nvprefix);
	nvram_default_geti(temp, 1);
	if (nvram_invmatchi(temp, 0))
		return 0;

	wlmac = get_wl_assoc_mac(prefix, &count_wl);

	for (i = 0; i < count_wl; i++) {
		if (!strcasecmp(wlmac[i].mac, wp->http_client_mac)) {
			cprintf("Can't accept wireless access\n");
			debug_free(wlmac);
			return -1;
		}
	}
	debug_free(wlmac);

	return 0;
}

static int check_connect_type(webs_t wp)
{
	struct wl_assoc_mac *wlmac = NULL;
	int count_wl = 0;
	int i, j;
	char temp[32];
	int c = getdevicecount();
	for (j = 0; j < c; j++) {
#if defined(HAVE_ATH9K) || defined(HAVE_MADWIFI)
		sprintf(temp, "wlan%d", j);
		char *ifname = temp;
#else
		sprintf(temp, "wl%d", j);
		char *ifname = get_wl_instance_name(j);
#endif
		if (check_connect_type_vap(ifname, wp, temp))
			return -1;
		char *names = nvram_nget("%s_vifs", temp);
		char *next;
		char var[32];
		foreach(var, names, next)
		{
			if (check_connect_type_vap(var, wp, var))
				return -1;
		}
	}
	return 0;
}

#ifdef HAVE_IAS
char ias_sid[20];
int ias_sid_timeout;
void ias_sid_set(webs_t wp);
int ias_sid_valid(webs_t wp);
#endif
static persistent_vars global_vars;
#define LINE_LEN 10000
static void *handle_request(void *arg)
{
	webs_t conn_fp = (webs_t)arg;
	char *cur = NULL;
	char *method, *path, *protocol, *authorization, *boundary, *referer, *host;
#ifdef HAVE_IAS
	char *language, *useragent;
#endif
	char *cp = NULL;
	char *file = NULL;
	int len;
	struct mime_handler *handler;
	size_t content_length = 0;
	char *line;
	long method_type;
	conn_fp->p = &global_vars;
	if (!conn_fp->p->filter_id)
		conn_fp->p->filter_id = 1;

	dd_logdebug("httpd", "thread start");
	PTHREAD_MUTEX_LOCK(&input_mutex); // barrier. block until input is done. otherwise global members get async
	PTHREAD_MUTEX_UNLOCK(&input_mutex);

#ifndef HAVE_MUSL
	PTHREAD_MUTEX_LOCK(&httpd_mutex);
#endif
#ifdef HAVE_REGISTER
	conn_fp->isregistered = registered;
	conn_fp->isregistered_real = registered_real;
#endif
#ifdef HAVE_SUPERCHANNEL
	conn_fp->issuperchannel = superchannel;
#endif
#ifndef HAVE_MUSL
	PTHREAD_MUTEX_UNLOCK(&httpd_mutex);
#endif
	setnaggle(conn_fp, 1);

	line = malloc(LINE_LEN);
	/* Initialize the request variables. */
	authorization = referer = boundary = host = NULL;
	bzero(line, LINE_LEN);

	/* Parse the first line of the request. */
	int cnt = 0;
	int eof = 0;
	dd_logdebug("httpd", "parse line");
	wfgets(line, LINE_LEN, conn_fp, &eof);
	if (eof) {
		send_error(conn_fp, 0, 408, live_translate(conn_fp, "share.tcp_error"), NULL,
			   live_translate(conn_fp, "share.unexpected_connection_close"));
		goto out;
	}

	if (!*(line)) {
#if 0
		char debug[128];
		sprintf(debug, "%s errno %d, cnt %d thread %d\n", live_translate(conn_fp, "share.request_timeout_desc"), errno, cnt, threadnum);
		send_error(conn_fp, 0, 408, live_translate(conn_fp, "share.request_timeout"), NULL, debug);
#endif
		goto out;
	}

	/* To prevent http receive https packets, cause http crash (by honor 2003/09/02) */
	if (strncasecmp(line, "GET", 3) && strncasecmp(line, "POST", 4) && strncasecmp(line, "OPTIONS", 7)) {
		goto out;
	}
	method = path = line;
	strsep(&path, " ");
	if (!path) { // Avoid http server crash, added by honor 2003-12-08
		send_error(conn_fp, 0, 400, live_translate(conn_fp, "share.bad_request"), NULL,
			   live_translate(conn_fp, "share.cant_parse_no_path"));
		goto out;
	}
	while (*path == ' ')
		path++;
	protocol = path;
	strsep(&protocol, " ");
	if (!protocol) { // Avoid http server crash, added by honor 2003-12-08
		send_error(conn_fp, 0, 400, live_translate(conn_fp, "share.bad_request"), NULL,
			   live_translate(conn_fp, "share.cant_parse_no_proto"));
		goto out;
	}
	while (*protocol == ' ')
		protocol++;

	cp = protocol;
	strsep(&cp, " ");
	cur = protocol + strlen(protocol) + 1;
	/* Parse the rest of the request headers. */
	while ((line + LINE_LEN - cur) > 1 && wfgets(cur, line + LINE_LEN - cur, conn_fp, &eof) != 0) //jimmy,https,8/4/2003
	{
		if (eof) {
			send_error(conn_fp, 0, 408, live_translate(conn_fp, "share.tcp_error"), NULL,
				   live_translate(conn_fp, "share.unexpected_connection_close_2"));
			goto out;
		}
		if (strcmp(cur, "\n") == 0 || strcmp(cur, "\r\n") == 0) {
			break;
		} else if (strncasecmp(cur, "Authorization:", 14) == 0) {
			cp = &cur[14];
			cp += strspn(cp, " \t");
			authorization = cp;
			dd_logdebug("httpd", "Auth: %s", cp);
			cur = cp + strlen(cp) + 1;
		} else if (strncasecmp(cur, "Referer:", 8) == 0) {
			cp = &cur[8];
			cp += strspn(cp, " \t");
			referer = cp;
			dd_logdebug("httpd", "Referer: %s", cp);
			cur = cp + strlen(cp) + 1;
		} else if (strncasecmp(cur, "Host:", 5) == 0) {
			cp = &cur[5];
			cp += strspn(cp, " \t");
			host = cp;
			dd_logdebug("httpd", "Host: %s", cp);
			cur = cp + strlen(cp) + 1;
		} else if (strncasecmp(cur, "Content-Length:", 15) == 0) {
			cp = &cur[15];
			cp += strspn(cp, " \t");
			dd_logdebug("httpd", "Content-Length: %s", cp);
			content_length = strtoul(cp, NULL, 0);
		} else if ((cp = strstr(cur, "boundary="))) {
			boundary = &cp[9];
			for (cp = cp + 9; *cp && *cp != '\r' && *cp != '\n'; cp++)
				;
			*cp = '\0';
			cur = ++cp;
		}
#ifdef HAVE_IAS
		else if (strncasecmp(cur, "User-Agent:", 11) == 0) {
			cp = &cur[11];
			cp += strspn(cp, " \t");
			dd_logdebug("httpd", "User-Agent: %s", cp);
			useragent = cp;
			cur = cp + strlen(cp) + 1;
		} else if (strncasecmp(cur, "Accept-Language:", 16) == 0) {
			cp = &cur[17];
			cp += strspn(cp, " \t");
			dd_logdebug("httpd", "Accept-Language: %s", cp);
			language = cp;
			cur = cp + strlen(cp) + 1;
		}
#endif
	}
	method_type = METHOD_INVALID;
	if (!strcasecmp(method, "get"))
		method_type = METHOD_GET;
	if (!strcasecmp(method, "post"))
		method_type = METHOD_POST;
	if (!strcasecmp(method, "options"))
		method_type = METHOD_OPTIONS;
	//      if (!strcasecmp(method, "head"))
	//              method_type = METHOD_HEAD;

	if (method_type == METHOD_INVALID) {
		send_error(conn_fp, 0, 501, live_translate(conn_fp, "share.not_implemented"), NULL,
			   live_translate(conn_fp, "share.method_unimpl"), method);
		goto out;
	}

	if (path[0] != '/') {
		send_error(conn_fp, 0, 400, live_translate(conn_fp, "share.bad_request"), NULL,
			   live_translate(conn_fp, "share.no_slash"));
		goto out;
	}
	file = &(path[1]);
	len = strlen(file);
	if (file[0] == '/' || strcmp(file, "..") == 0 || strncmp(file, "../", 3) == 0 || strstr(file, "/../") != NULL ||
	    strcmp(&(file[len - 3]), "/..") == 0) {
		send_error(conn_fp, 0, 400, live_translate(conn_fp, "share.bad_request"), NULL,
			   live_translate(conn_fp, "share.threaten_fs"));
		goto out;
	}

	int nodetect = 0;
	if (nvram_matchi("status_auth", 0) && endswith(file, "Info.htm"))
		nodetect = 1;
	if (nvram_matchi("no_crossdetect", 1))
		nodetect = 1;
#ifdef HAVE_IAS
	if (nvram_matchi("ias_startup", 3) || nvram_matchi("ias_startup", 2)) {
		nodetect = 1;
		typedef struct {
			char *iso;
			char *mapping;
			char *countryext;
		} ISOMAP;

		ISOMAP isomap[] = { { "de", "german" }, //
				    { "es", "spanish" }, //
				    { "fr", "french" }, //
				    { "hr", "croatian" }, //
				    { "hu", "hungarian" }, //
				    { "nl", "dutch" }, //
				    { "it", "italian" }, //
				    { "jp", "japanese" }, //
				    { "pl", "polish" }, //
				    { "pt", "portuguese_braz" }, //
				    { "ro", "romanian" }, //
				    { "ru", "russian", "RU" }, //
				    { "sl", "slovenian" }, //
				    { "sr", "serbian" }, //
				    { "sv", "swedish" }, //
				    { "zh", "chinese_simplified" }, //
				    { "tr", "turkish" }, //
				    NULL };
		if (nvram_match("langprop", "") || nvram_get("langprop") == NULL) {
			int cnt = 0;
			nvram_set("langprop", "english");
			while (isomap[cnt].iso != NULL) {
				if (strncasecmp(language, isomap[cnt].iso, 2) == 0) {
					nvram_set("langprop", isomap[cnt].mapping);
					if (isomap[cnt].countryext)
						nvram_set("country", isomap[cnt].countryext);
					break;
				}
				cnt++;
			}
		}
	}
#endif

	if (!referer && method_type == METHOD_POST && nodetect == 0) {
		send_error(conn_fp, 0, 400, live_translate(conn_fp, "share.bad_request"), NULL,
			   live_translate(conn_fp, "share.cross_site"));
		goto out;
	}

	if (referer && host && nodetect == 0) {
		int i;
		int hlen = strlen(host);
		int rlen = strlen(referer);
		int slashs = 0;

		for (i = 0; i < rlen; i++) {
			if (referer[i] == '/')
				slashs++;
			if (slashs == 2) {
				i++;
				break;
			}
		}
		if (slashs == 2) {
			int a;
			int c = 0;

			for (a = 0; a < hlen; a++)
				if (host[a] == ' ' || host[a] == '\r' || host[a] == '\n' || host[a] == '\t')
					host[a] = 0;
			hlen = strlen(host);
			for (a = i; a < rlen; a++) {
				if (referer[a] == '/') {
					send_error(conn_fp, 0, 400, live_translate(conn_fp, "share.bad_request"), NULL,
						   live_translate(conn_fp, "share.cross_site_ref"), referer);
					goto out;
				}
				if (host[c++] != referer[a]) {
					send_error(conn_fp, 0, 400, live_translate(conn_fp, "share.bad_request"), NULL,
						   live_translate(conn_fp, "share.cross_site_ref"), referer);
					goto out;
				}
				if (c == hlen) {
					a++;
					break;
				}
			}
			if (c != hlen || referer[a] != '/') {
				send_error(conn_fp, 0, 400, live_translate(conn_fp, "share.bad_request"), NULL,
					   live_translate(conn_fp, "share.cross_site_ref"), referer);
				goto out;
			}
		}
	}
	// seg change for status site
#ifdef HAVE_REGISTER
	if (conn_fp->isregistered_real == -1) {
		conn_fp->isregistered_real = isregistered_real();
	}
	if (!conn_fp->isregistered_real)
		conn_fp->isregistered = isregistered();
	else
		conn_fp->isregistered = conn_fp->isregistered_real;
#endif
#ifdef HAVE_SUPERCHANNEL
	if (conn_fp->issuperchannel == -1)
		conn_fp->issuperchannel = issuperchannel();
#endif

	// save the originally requested url
	conn_fp->request_url = strdup(file);

#ifdef HAVE_BUFFALO
#ifdef HAVE_IAS
	int ias_startup = nvram_geti("ias_startup");
	int ias_detected = 0;
	char redirect_path[48];

	if (ias_startup > 1) {
		if (endswith(file, "?ias_detect")) {
			ias_detected = 1;
			fprintf(stderr, "[HTTP PATH] %s detected (%d)\n", file, ias_startup);
		}

		if (!endswith(file, ".js") && !endswith(file, "detect.asp") && ias_detected == 0 &&
		    nvram_matchi("ias_startup", 3)) {
			fprintf(stderr, "[HTTP PATH] %s redirect\n", file);
			snprintf(redirect_path, sizeof(redirect_path), "Location: http://%s/detect.asp", nvram_get("lan_ipaddr"));
			send_headers(conn_fp, 302, "Found", redirect_path, "", -1, NULL, 1);
			goto out;

		} else if (ias_detected == 1) {
			fprintf(stderr, "[HTTP PATH] %s redirected\n", file);
			nvram_seti("ias_startup", 2);
		}
	}

	if (!ias_sid_valid(conn_fp) &&
	    (endswith(file, "InternetAtStart.asp") || endswith(file, "detect.asp") || endswith(file, "?ias_detect")) &&
	    strstr(useragent, "Android"))
		ias_sid_set(conn_fp);

	char hostname[32];
	if (strlen(host) < 32 && ias_detected == 1 && (nvram_matchi("ias_dnsresp", 1))) {
		strncpy(hostname, host, strspn(host, "123456789.:"));
		if (!strcmp(hostname, nvram_get("lan_ipaddr"))) {
			nvram_unset("ias_dnsresp");
			char cmd[64];
			snprintf(cmd, sizeof(cmd), "%s:55300", nvram_get("lan_ipaddr"));
			eval("iptables", "-t", "nat", "-D", "PREROUTING", "-i", "br0", "-p", "udp", "--dport", "53", "-j", "DNAT",
			     "--to", cmd);

			char buf[128];
			char call[64];
			int pid;
			FILE *fp;

			sprintf(call, "ps|grep \"dns_responder\"");
			fp = popen(call, "r");
			if (fp) {
				while (fgets(buf, sizeof(buf), fp)) {
					if (strstr(buf, nvram_get("lan_ipaddr"))) {
						if (sscanf(buf, "%d ", &pid)) {
							kill(pid, SIGTERM);
						}
					}
				}
				pclose(fp);
			}
		}
	}
#endif
#endif

	if (file[0] == '\0' || file[len - 1] == '/') {
		if (server_dir != NULL && strcmp(server_dir,
						 "/www")) // to allow to use router as a WEB server
		{
			file = "index.htm";
		} else {
#ifdef HAVE_IAS
			file = "index.asp";
#else
			if (nvram_invmatchi("status_auth", 0))
				file = "Info.htm";
			else
				file = "index.asp";
#endif
		}
	} else {
		if (nvram_invmatchi("status_auth", 1))
			if (strcmp(file, "Info.htm") == 0)
				file = "index.asp";
	}
	int changepassword = 0;
	if (!IS_REGISTERED_REAL(conn_fp)) {
		if (endswith(file, "About.htm"))
			file = "register.asp";
	}
	if (!IS_REGISTERED(conn_fp)) {
		if (endswith(file, ".asp"))
			file = "register.asp";
		else if (endswith(file, ".htm"))
			file = "register.asp";
		else if (endswith(file, ".html"))
			file = "register.asp";
	} else {
		if (((nvram_match("http_username", DEFAULT_USER) && nvram_match("http_passwd", DEFAULT_PASS)) ||
		     nvram_match("http_username", "") || nvram_match("http_passwd", "admin")) &&
		    !endswith(file, "register.asp") && !endswith(file, "vsp.html")) {
			changepassword = 1;
			if (endswith(file, ".asp"))
				file = "changepass.asp";
			else if (endswith(file, ".htm") && !endswith(file, "About.htm"))
				file = "changepass.asp";
			else if (endswith(file, ".html"))
				file = "changepass.asp";
		} else {
			if (endswith(file, "changepass.asp")) {
				if (nvram_invmatchi("status_auth", 0))
					file = "Info.htm";
				else
					file = "index.asp";
			}
		}
	}

	FILE *fp;
	int file_error;
	int noheader;
	if (!SSL_ENABLED() || !DO_SSL(conn_fp)) {
		clear_ndelay(conn_fp->conn_fd);
	}
	PTHREAD_MUTEX_LOCK(&input_mutex);
	for (handler = &mime_handlers[0]; handler->pattern; handler++) {
		if (!match(handler->pattern, file))
			continue;
		airbag_setpostinfo(handler->pattern);
		if (method_type == METHOD_HEAD && handler->output != do_file) {
			send_error(conn_fp, 0, 501, live_translate(conn_fp, "share.not_implemented"), NULL,
				   live_translate(conn_fp, "share.method_unimpl"), method);
			goto out;
		}
		if (IS_REGISTERED(conn_fp) && !changepassword && handler->auth &&
		    (!handler->handle_options || method_type != METHOD_OPTIONS)) {
			if (authorization)
				conn_fp->authorization = strdup(authorization);

			int result = handler->auth(conn_fp, auth_check);

#ifdef HAVE_IAS
			if (!result && !((!strcmp(file, "apply.cgi") || !strcmp(file, "InternetAtStart.ajax.asp")) &&
					 strstr(useragent, "Android") && ias_sid_valid(conn_fp))) {
				fprintf(stderr, "[AUTH FAIL]: %s", useragent);
#else
			if (!result) {
#endif
				send_authenticate(conn_fp);
				goto out;
			}
		}
		conn_fp->post = 0;
		if (method_type == METHOD_POST) {
			conn_fp->post = 1;
		}

		if (handler->input) {
			if (handler->input(file, conn_fp, content_length, boundary)) {
				goto out;
			}
		}
#if 0 // defined(linux)
		if (!DO_SSL(conn_fp) && (flags = fcntl(fileno(conn_fp->fp), F_GETFL)) != -1 && fcntl(fileno(conn_fp->fp), F_SETFL, flags | O_NONBLOCK) != -1) {
			/* Read up to two more characters */
			if (fgetc(conn_fp->fp) != EOF)
				(void)fgetc(conn_fp->fp);
			fcntl(fileno(conn_fp->fp), F_SETFL, flags);
		}
#endif
		if (check_connect_type(conn_fp) < 0) {
			send_error(conn_fp, 0, 401, live_translate(conn_fp, "share.bad_request"), NULL,
				   live_translate(conn_fp, "share.no_wifi_access"));
			goto out;
		}
		if (handler->send_headers) {
			send_headers(conn_fp, 200, "OK", handler->extra_header, handler->mime_type, -1, NULL, 1);
			noheader = 1;
		}
		// check for do_file handler and check if file exists
		if (handler->output) {
			if (!strstr(file, "MyPage")) {
				PTHREAD_MUTEX_TRYLOCK(&input_mutex);
				PTHREAD_MUTEX_UNLOCK(&input_mutex);
			}
			file_error = handler->output(method_type, handler, file, conn_fp);
			if (!file_error) {
				goto out;
			}
		}
		break;
	}
	send_error(conn_fp, noheader, 404, live_translate(conn_fp, "share.not_found"), NULL,
		   live_translate(conn_fp, "share.file_not_found"), file);

out:;
	PTHREAD_MUTEX_TRYLOCK(&input_mutex);
	PTHREAD_MUTEX_UNLOCK(&input_mutex);
	setnaggle(conn_fp, 0);

	debug_free(line);
	if (conn_fp->request_url)
		debug_free(conn_fp->request_url);
	if (conn_fp->authorization)
		debug_free(conn_fp->authorization);
	if (conn_fp->post_buf)
		debug_free(conn_fp->post_buf);
#ifndef HAVE_MUSL
	PTHREAD_MUTEX_LOCK(&httpd_mutex);
#endif
#ifdef HAVE_REGISTER
	registered = conn_fp->isregistered;
	registered_real = conn_fp->isregistered_real;
#endif
#ifdef HAVE_ISSUPERCHANNEL
	superchannel = conn_fp->issuperchannel;
#endif
#ifndef HAVE_MUSL
	PTHREAD_MUTEX_UNLOCK(&httpd_mutex);
#endif
	SEM_POST(&semaphore);

	wfflush(conn_fp);
	wfclose(conn_fp);
	bzero(conn_fp,
	      sizeof(webs)); // erase to delete any traces of stored passwords or usernames
	debug_free(conn_fp);
	return NULL;
}

static void // add by honor 2003-04-16
get_client_ip_mac(webs_t conn_fp)
{
	get_ipfromsock(conn_fp->conn_fd, conn_fp->http_client_ip);
	get_mac_from_ip(conn_fp->http_client_mac, conn_fp->http_client_ip);
}

static void handle_server_sig_int(int sig)
{
	dd_loginfo("httpd", "httpd server shutdown");
	free_defaults(srouter_defaults);
	exit(0);
}

static void settimeouts(webs_t wp, int secs)
{
	struct timeval tv;
	tv.tv_sec = secs;
	tv.tv_usec = 0;
	if (setsockopt(wp->conn_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
		perror("setsockopt(SO_SNDTIMEO)");
	if (setsockopt(wp->conn_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
		perror("setsockopt(SO_RCVTIMEO)");
}

static void handle_sigchld(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
}

#ifdef HAVE_OPENSSL
#define SSLBUFFERSIZE 16384
struct sslbuffer {
	unsigned char *sslbuffer;
	unsigned int count;
	SSL *ssl;
};

static struct sslbuffer *initsslbuffer(SSL *ssl)
{
	struct sslbuffer *buffer;
	buffer = malloc(sizeof(struct sslbuffer));
	buffer->ssl = ssl;
	buffer->count = 0;
	buffer->sslbuffer = malloc(SSLBUFFERSIZE);
	return buffer;
}

static void sslbufferflush(struct sslbuffer *buffer)
{
	SSL_write(buffer->ssl, buffer->sslbuffer, buffer->count);
	buffer->count = 0;
}

static void sslbufferfree(struct sslbuffer *buffer)
{
	debug_free(buffer->sslbuffer);
	int sockfd = SSL_get_fd(buffer->ssl);
	SSL_shutdown(buffer->ssl);
	close(sockfd);
	SSL_free(buffer->ssl);
	debug_free(buffer);
}

static int sslbufferread(struct sslbuffer *buffer, char *data, size_t datalen)
{
	return SSL_read(buffer->ssl, data, datalen);
}

static int sslbufferpeek(struct sslbuffer *buffer, char *data, size_t datalen)
{
	return SSL_peek(buffer->ssl, data, datalen);
}

static size_t sslbufferwrite(struct sslbuffer *buffer, char *data, size_t datalen)
{
	int targetsize = SSLBUFFERSIZE - buffer->count;
	while (datalen >= targetsize) {
		memcpy(&buffer->sslbuffer[buffer->count], data, targetsize);
		datalen -= targetsize;
		data += targetsize;
		SSL_write(buffer->ssl, buffer->sslbuffer, SSLBUFFERSIZE);
		buffer->count = 0;
		targetsize = SSLBUFFERSIZE;
	}
	if (datalen) {
		memcpy(&buffer->sslbuffer[buffer->count], data, datalen);
		buffer->count += datalen;
	}
	return datalen;
}

#endif

#ifdef HAVE_MATRIXSSL
sslKeys_t *keys;
#endif

int main(int argc, char **argv)
{
	usockaddr host_addr4;
	usockaddr host_addr6;
	usockaddr ssl_host_addr4;
	usockaddr ssl_host_addr6;
	usockaddr usa;
	int server_port;
	int ssl_server_port;
	int gotv4 = 0, gotv6 = 0;
	int ssl_gotv4 = 0, ssl_gotv6 = 0;
	int listen4_fd = -1;
	int listen6_fd = -1;
	int ssl_listen4_fd = -1;
	int ssl_listen6_fd = -1;
	socklen_t sz = sizeof(usa);
	int c;
	int timeout = TIMEOUT;
	struct stat stat_dir;
	fd_set lfdset;
	int maxfd;
	airbag_init();
	srouter_defaults = load_defaults();
#ifdef HAVE_HTTPS
	int do_ssl = 0;
#else
#define do_ssl 0
#endif
	int no_ssl = 0;
	/* SEG addition */
	Initnvramtab();
#ifdef HAVE_OPENSSL
	SSL_CTX *ctx = NULL;
	int r;
#endif
#ifdef HAVE_POLARSSL
	int ret, len;
	x509_crt srvcert;
	pk_context rsa;
	entropy_context entropy;
	ctr_drbg_context ctr_drbg;
	const char *pers = "ssl_server";
#endif

	CRYPT_MUTEX_INIT(&crypt_mutex, NULL);
	SEM_INIT(&semaphore, 0, HTTP_MAXCONN);
#ifndef HAVE_MUSL
	PTHREAD_MUTEX_INIT(&httpd_mutex, NULL);
#endif
	PTHREAD_MUTEX_INIT(&input_mutex, NULL);
#if !defined(HAVE_MICRO) && !defined(__UCLIBC__)
	PTHREAD_MUTEX_INIT(&global_vars.mutex_contr, NULL);
#ifdef HAVE_WIVIZ
	PTHREAD_MUTEX_INIT(&global_vars.wiz_mutex_contr, NULL);
#endif
#endif

	strcpy(pid_file, "/var/run/httpd.pid");
	server_port = DEFAULT_HTTP_PORT;

	while ((c = getopt(argc, argv, "Snih:p:m:d:t:s:g:e:")) != -1)
		switch (c) {
#ifdef HAVE_HTTPS
		case 'S':
			do_ssl = 1;
			ssl_server_port = DEFAULT_HTTPS_PORT;
			break;
#endif
		case 'n':
			no_ssl = 1;
			break;
		case 'h':
			server_dir = optarg;
			break;
		case 'p':
			server_port = atoi(optarg);
			break;
		case 'm':
			ssl_server_port = atoi(optarg);
			break;
		case 't':
			timeout = atoi(optarg);
			break;
#ifdef DEBUG_CIPHER
		case 's':
			set_ciphers = optarg;
			break;
		case 'g':
			get_ciphers = 1;
			break;
#endif
		case 'i':
			fprintf(stderr,
				"Usage: %s [-S] [-p port]\n"
#ifdef HAVE_HTTPS
				"	-S : Support HTTPS (port 443)\n"
				"	-m port : Which SSL port to listen?\n"
#endif
				"	-n : Support HTTP (port 80)\n"
				"	-p port : Which port to listen?\n"
				"	-t secs : How many seconds to wait before timing out?\n"
#ifdef DEBUG_CIPHER
				"	-s ciphers: Set cipher lists\n"
				"	-g: Get cipher lists\n"
#endif
				"	-h: home directory: Use directory\n",
				argv[0]);
			exit(0);
			break;
		default:
			break;
		}
	openlog("httpd", LOG_PID | LOG_NDELAY, LOG_DAEMON);
	if (!do_ssl && !no_ssl) {
		dd_loginfo("httpd", "httpd cannot start. SSL and/or HTTP must be selected");
		exit(0);
	}
	if (SSL_ENABLED()) {
		if (no_ssl)
			dd_loginfo("httpd", "httpd server started at port %d", server_port);
		if (do_ssl)
			dd_loginfo("httpd", "httpd SSL server started at port %d", ssl_server_port);
	} else
		dd_loginfo("httpd", "httpd server started at port %d", server_port);
	/* Ignore broken pipes */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, handle_server_sig_int); // kill
	signal(SIGCHLD, handle_sigchld);
#if 0
	struct sigaction sa1;
#ifdef SA_SIGINFO
	sa1.sa_flags = SA_SIGINFO;
#else
	sa1.sa_flags = 0;
#endif
	sigemptyset(&sa1.sa_mask);
	sa1.sa_sigaction = handle_server_sigsegv;
	sigaction(SIGSEGV, &sa1, NULL);

	struct sigaction sa2;
#ifdef SA_SIGINFO
	sa2.sa_flags = SA_SIGINFO;
#else
	sa2.sa_flags = 0;
#endif
	sigemptyset(&sa2.sa_mask);
	sa2.sa_sigaction = handle_server_sigsegv;
	sigaction(SIGBUS, &sa2, NULL);
#endif
	if (server_dir && stat(server_dir, &stat_dir) == 0)
		chdir(server_dir);

	/* Build our SSL context */
	if (SSL_ENABLED() && do_ssl) {
#if defined(HAVE_OPENSSL) || defined(HAVE_MATRIXSSL) || defined(HAVE_POLARSSL)
		char *cert = nvram_safe_get("https_cert");
		char *key = nvram_safe_get("https_key");
		char *certfile = NULL;
		char *keyfile = NULL;
		if (*cert) {
			certfile = "/tmp/https_cert";
			writenvram("https_cert", certfile);
		}
		if (*key) {
			keyfile = "/tmp/https_key";
			writenvram("https_key", keyfile);
		}
		if (!certfile)
			certfile = nvram_safe_get("https_cert_file");
		if (!*certfile)
			certfile = CERT_FILE;
		if (!keyfile)
			keyfile = nvram_safe_get("https_key_file");
		if (!*keyfile)
			keyfile = KEY_FILE;
#endif
#ifdef HAVE_OPENSSL
		SSLeay_add_ssl_algorithms();
		SSL_load_error_strings();
		ctx = SSL_CTX_new(SSLv23_server_method());
		if (SSL_CTX_use_certificate_file(ctx, certfile, SSL_FILETYPE_PEM) == 0) {
			cprintf("Can't read %s\n", CERT_FILE);
			exit(1);
		}
		if (SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM) == 0) {
			cprintf("Can't read %s\n", KEY_FILE);
			exit(1);
		}
		if (SSL_CTX_check_private_key(ctx) == 0) {
			cprintf("Check private key fail\n");
			exit(1);
		}
#endif
#ifdef HAVE_POLARSSL
		if ((ret = ctr_drbg_init(&ctr_drbg, entropy_func, &entropy, (const unsigned char *)pers, strlen(pers))) != 0) {
			fprintf(stderr, " failed\n  ! ctr_drbg_init returned %d\n", ret);
		}

		bzero(&ssl, sizeof(ssl));
		bzero(&srvcert, sizeof(x509_crt));
		x509_crt_init(&srvcert);
		ret = x509_crt_parse_file(&srvcert, certfile);
		if (ret != 0) {
			printf("x509_read_crtfile failed\n");
			exit(0);
		}
		ret = pk_parse_keyfile(&rsa, keyfile, NULL);
		if (ret != 0) {
			printf("x509_read_keyfile failed\n");
			exit(0);
		}
#endif

#ifdef HAVE_MATRIXSSL
		matrixssl_init();
		if (0 != matrixSslReadKeys(&keys, certfile, keyfile, NULL, NULL)) {
			fprintf(stderr, "Error reading or parsing %s.\n", KEY_FILE);
			exit(0);
		}
#endif
	}

	/* Look up hostname. */
	if (no_ssl)
		lookup_hostname(&host_addr4, sizeof(host_addr4), &gotv4, &host_addr6, sizeof(host_addr6), &gotv6, server_port);

	if (SSL_ENABLED() && do_ssl)
		lookup_hostname(&ssl_host_addr4, sizeof(ssl_host_addr4), &ssl_gotv4, &ssl_host_addr6, sizeof(ssl_host_addr6),
				&ssl_gotv6, ssl_server_port);

	if (!(gotv4 || gotv6 || ssl_gotv4 || ssl_gotv6)) {
		exit(1);
	}

	/* Initialize listen sockets.  Try v6 first because of a Linux peculiarity;
	 ** like some other systems, it has magical v6 sockets that also listen for
	 ** v4, but in Linux if you bind a v4 socket first then the v6 bind fails.
	 */
#ifdef USE_IPV6
	if (no_ssl && gotv6) {
		listen6_fd = initialize_listen_socket(&host_addr6);
	} else
		listen6_fd = -1;

	if (SSL_ENABLED() && do_ssl && ssl_gotv6) {
		ssl_listen6_fd = initialize_listen_socket(&ssl_host_addr6);
	} else
		ssl_listen6_fd = -1;

#endif
	if (no_ssl && gotv4) {
		listen4_fd = initialize_listen_socket(&host_addr4);
	} else
		listen4_fd = -1;

	if (SSL_ENABLED() && do_ssl && ssl_gotv4) {
		ssl_listen4_fd = initialize_listen_socket(&ssl_host_addr4);
	} else
		ssl_listen4_fd = -1;

	/* If we didn't get any valid sockets, fail. */
	if (listen4_fd == -1 && listen6_fd == -1 && ssl_listen4_fd == -1 && ssl_listen6_fd == -1) {
		dd_logerror("httpd", "Can't bind to any address");
		exit(1);
	}
#if !defined(DEBUG)
	FILE *pid_fp;

	/* Daemonize and log PID */
	if (daemon(1, 1) == -1) {
		perror("daemon");
		exit(errno);
	}
	if (!(pid_fp = fopen(pid_file, "w"))) {
		perror(pid_file);
		return errno;
	}
	fprintf(pid_fp, "%d", getpid());
	fclose(pid_fp);
#endif

	/* Loop forever handling requests */
	for (;;) {
		webs_t conn_fp = safe_malloc(sizeof(webs));
		if (!conn_fp) {
			dd_logerror("httpd", "Out of memory while creating new connection");
			continue;
		}
		bzero(conn_fp, sizeof(webs));
		SEM_WAIT(&semaphore);
		errno = 0; // workaround for musl bug
		FD_ZERO(&lfdset);
		maxfd = -1;
		if (no_ssl) {
			if (listen4_fd != -1) {
				FD_SET(listen4_fd, &lfdset);
				if (listen4_fd > maxfd)
					maxfd = listen4_fd;
			}
#ifdef USE_IPV6
			if (listen6_fd != -1) {
				FD_SET(listen6_fd, &lfdset);
				if (listen6_fd > maxfd)
					maxfd = listen6_fd;
			}
#endif
		}

		if (SSL_ENABLED() && do_ssl) {
			if (ssl_listen4_fd != -1) {
				FD_SET(ssl_listen4_fd, &lfdset);
				if (ssl_listen4_fd > maxfd)
					maxfd = ssl_listen4_fd;
			}
#ifdef USE_IPV6
			if (ssl_listen6_fd != -1) {
				FD_SET(ssl_listen6_fd, &lfdset);
				if (ssl_listen6_fd > maxfd)
					maxfd = ssl_listen6_fd;
			}
#endif
		}
		dd_logdebug("httpd", "select() %d", maxfd + 1);
		if (select(maxfd + 1, &lfdset, NULL, NULL, NULL) < 0) {
			if (errno == EINTR || errno == EAGAIN) {
				SEM_POST(&semaphore);
				continue; /* try again */
			}
			perror("select");
			SEM_POST(&semaphore);
			continue;
		}

		sz = sizeof(usa);
#ifdef USE_IPV6
		if (no_ssl && listen6_fd != -1 && FD_ISSET(listen6_fd, &lfdset)) {
			dd_logdebug("httpd", "accept6() %d", listen6_fd);
			conn_fp->conn_fd = accept(listen6_fd, &usa.sa, &sz);
		} else if (SSL_ENABLED() && do_ssl && ssl_listen6_fd != -1 && FD_ISSET(ssl_listen6_fd, &lfdset)) {
			dd_logdebug("httpd", "accept6_ssl() %d", ssl_listen6_fd);
			conn_fp->conn_fd = accept(ssl_listen6_fd, &usa.sa, &sz);
			conn_fp->ssl_enabled = 1;
		} else
#endif
			if (no_ssl && listen4_fd != -1 && FD_ISSET(listen4_fd, &lfdset)) {
			dd_logdebug("httpd", "accept4() %d", listen4_fd);
			conn_fp->conn_fd = accept(listen4_fd, &usa.sa, &sz);
		} else if (SSL_ENABLED() && do_ssl && ssl_listen4_fd != -1 && FD_ISSET(ssl_listen4_fd, &lfdset)) {
			dd_logdebug("httpd", "accept4_ssl() %d", ssl_listen4_fd);
			conn_fp->conn_fd = accept(ssl_listen4_fd, &usa.sa, &sz);
			conn_fp->ssl_enabled = 1;
		}
		if (conn_fp->conn_fd < 0) {
			dd_logdebug("httpd", "error on accept errno %d", errno);
			perror("accept");
			SEM_POST(&semaphore);
			continue;
		}
		if (!SSL_ENABLED() || !DO_SSL(conn_fp)) {
			set_ndelay(conn_fp->conn_fd);
		}

		/* Make sure we don't linger a long time if the other end disappears */
		settimeouts(conn_fp, timeout);
		//              fcntl(conn_fp->conn_fd, F_SETFD, fcntl(conn_fp->conn_fd, F_GETFD) | FD_CLOEXEC);
		int action = check_action();
		if (action == ACT_SW_RESTORE || action == ACT_HW_RESTORE) {
			fprintf(stderr, "http(s)d: nothing to do...\n");
			close(conn_fp->conn_fd);
			SEM_POST(&semaphore);
			continue;
		}
		get_client_ip_mac(conn_fp);
		if (check_blocklist("httpd", conn_fp->http_client_ip)) {
			close(conn_fp->conn_fd);
			SEM_POST(&semaphore);
			continue;
		}

		if (SSL_ENABLED() && DO_SSL(conn_fp)) {
			if (action == ACT_WEB_UPGRADE) { // We don't want user to use web (https) during web (http) upgrade.
				fprintf(stderr, "http(s)d: nothing to do...\n");
				close(conn_fp->conn_fd);
				SEM_POST(&semaphore);
				continue;
			}
#ifdef HAVE_OPENSSL
			const char *allowedCiphers =
				"ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
			conn_fp->ssl = SSL_new(ctx);

#ifdef NID_X9_62_prime256v1
			EC_KEY *ecdh = NULL;
			if ((ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1))) {
				SSL_CTX_set_tmp_ecdh(ctx, ecdh);
				EC_KEY_free(ecdh);
			}
#endif

			// Setup available ciphers
			SSL_CTX_set_cipher_list(ctx, allowedCiphers);

			// Enforce our desired cipher order, disable obsolete protocols
#ifndef SSL_OP_SAFARI_ECDHE_ECDSA_BUG
#define SSL_OP_SAFARI_ECDHE_ECDSA_BUG 0
#endif

			SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
							 SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_SAFARI_ECDHE_ECDSA_BUG);

			SSL_set_fd(conn_fp->ssl, conn_fp->conn_fd);
			r = SSL_accept(conn_fp->ssl);
			if (r <= 0) {
				close(conn_fp->conn_fd);

				SSL_free(conn_fp->ssl);
				SEM_POST(&semaphore);
				continue;
			}

			conn_fp->fp_in = (FILE *)initsslbuffer(conn_fp->ssl);

#elif defined(HAVE_MATRIXSSL)
			matrixssl_new_session(conn_fp->conn_fd);

			conn_fp->fp_in = (FILE *)conn_fp->conn_fd;
#endif
#ifdef HAVE_POLARSSL
			ssl_free(&conn_fp->ssl);
			if ((ret = ssl_init(&conn_fp->ssl)) != 0) {
				printf("ssl_init failed\n");
				close(conn_fp->conn_fd);
				SEM_POST(&semaphore);
				continue;
			}
			ssl_set_endpoint(&conn_fp->ssl, SSL_IS_SERVER);
			ssl_set_authmode(&conn_fp->ssl, SSL_VERIFY_NONE);
			ssl_set_rng(&conn_fp->ssl, ctr_drbg_random, &ctr_drbg);
			ssl_set_ca_chain(&conn_fp->ssl, srvcert.next, NULL, NULL);

			ssl_set_bio(&conn_fp->ssl, net_recv, &conn_fp->conn_fd, net_send, &conn_fp->conn_fd);
			ssl_set_ciphersuites(&conn_fp->ssl, my_ciphers);
			ssl_set_own_cert(&conn_fp->ssl, &srvcert, &rsa);

			//              ssl_set_sidtable(&ssl, session_table);
			ssl_set_dh_param(&conn_fp->ssl, dhm_P, dhm_G);

			ret = ssl_handshake(&conn_fp->ssl);
			if (ret != 0) {
				printf("ssl_server_start failed\n");
				close(conn_fp->conn_fd);
				SEM_POST(&semaphore);
				continue;
			}

			conn_fp->fp_in = (webs_t)(&ssl);

#endif
		} else {
			if (SSL_ENABLED() &&
			    action == ACT_WEBS_UPGRADE) { // We don't want user to use web (http) during web (https) upgrade.
				fprintf(stderr, "httpd: nothing to do...\n");
				close(conn_fp->conn_fd);
				SEM_POST(&semaphore);
				continue;
			}
			conn_fp->conn_fd_out = dup(conn_fp->conn_fd);
			dd_logdebug("httpd", "fdopen()");
			if (!(conn_fp->fp_in = fdopen(conn_fp->conn_fd, "r"))) {
				dd_logdebug("httpd", "fd error error %d", errno);
				close(conn_fp->conn_fd);
				SEM_POST(&semaphore);
				continue;
			}
			if (!(conn_fp->fp_out = fdopen(conn_fp->conn_fd_out, "w"))) {
				dd_logdebug("httpd", "fd error error %d", errno);
				close(conn_fp->conn_fd);
				SEM_POST(&semaphore);
				continue;
			}
			//                      setlinebuf(conn_fp->fp_in);
			//                      setlinebuf(conn_fp->fp_out);
			//                        setvbuf(conn_fp->fp_in, NULL, _IONBF, 0);
			//                        setvbuf(conn_fp->fp_out, NULL, _IONBF, 0);
		}

#if !defined(HAVE_MICRO) && !defined(__UCLIBC__)
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		//              pthread_attr_setstacksize(&attr, 4096);
		pthread_t thread;
		dd_logdebug("httpd", "createthread()");
		if (pthread_create(&thread, &attr, handle_request, conn_fp) != 0) {
			SEM_POST(&semaphore);
			fprintf(stderr, "Failed to create thread\n");
		}
		pthread_attr_destroy(&attr);

//              conn_fp->ssl_enabled = do_ssl;
//              FORK(handle_request(conn_fp));
#else
		handle_request(conn_fp);
#endif
	}

	if (listen4_fd != -1) {
		shutdown(listen4_fd, 2);
		close(listen4_fd);
	}
	if (SSL_ENABLED() && ssl_listen4_fd != -1) {
		shutdown(ssl_listen4_fd, 2);
		close(ssl_listen4_fd);
	}
#ifdef USE_IPV6
	if (listen6_fd != -1) {
		shutdown(listen6_fd, 2);
		close(listen6_fd);
	}
	if (SSL_ENABLED() && ssl_listen6_fd != -1) {
		shutdown(ssl_listen6_fd, 2);
		close(ssl_listen6_fd);
	}
#endif
	free_defaults(srouter_defaults);
	return 0;
}

static char *wfgets(char *buf, int len, webs_t wp, int *rfeof)
{
	char *ret = NULL;
	int i;
	errno = 0;
	for (i = 0; i < 500; i++) {
		if (DO_SSL(wp)) {
#ifdef HAVE_OPENSSL
			int eof = 1;
			int i;
			char c;
			int sr = sslbufferpeek((struct sslbuffer *)wp->fp_in, buf, len);
			if (sr <= 0) {
				if (sr == 0 && rfeof)
					*rfeof = 1;
				goto out;
			}
			for (i = 0; i < len; i++) {
				c = buf[i];
				if (c == '\n' || c == 0) {
					eof = 0;
					goto next;
				}
			}
next:;
			sr = sslbufferread((struct sslbuffer *)wp->fp_in, buf, i + 1);
			if (sr <= 0) {
				if (sr == 0 && rfeof)
					*rfeof = 1;
				goto out;
			}
			if (!eof) {
				buf[i + 1] = 0;
				ret = buf;
				goto out;
			} else {
				if (rfeof)
					*rfeof = 1;
				goto out;
			}

#elif defined(HAVE_MATRIXSSL)
			ret = (char *)matrixssl_gets(wp->fp_in, buf, len);
#elif defined(HAVE_POLARSSL)

			int r = ssl_read((ssl_context *)wp->fp_in, (unsigned char *)buf, len);
			ret = buf;
#endif
		} else {
			if (feof(wp->fp_in)) {
				if (rfeof)
					*rfeof = 1;
			} else
				ret = fgets(buf, len, wp->fp_in);
		}
out:;
		if (!*(buf) && (errno == EINTR || errno == EAGAIN)) {
			struct timespec tim, tim2;
			tim.tv_sec = 0;
			tim.tv_nsec = 10000000L;
			nanosleep(&tim, &tim2);
			continue;
		}
		break;
	}

	return ret;
}

int wfputs(char *buf, webs_t wp)
{
	airbag_setpostinfo(buf);
	int ret;
	if (DO_SSL(wp)) {
#ifdef HAVE_OPENSSL
		ret = sslbufferwrite((struct sslbuffer *)wp->fp_in, buf, strlen(buf));

#elif defined(HAVE_MATRIXSSL)
		ret = matrixssl_puts(wp->fp_in, buf);

#elif defined(HAVE_POLARSSL)
		ret = ssl_write((ssl_context *)wp->fp_in, (unsigned char *)buf, strlen(buf));
		fprintf(stderr, "SSL write str %d\n", strlen(buf));

#endif
	} else {
		ret = fputs(buf, wp->fp_out);
	}
	return ret;
}

size_t vwebsWrite(webs_t wp, char *fmt, va_list args)
{
	char *buf;
	int ret;
	if (!wp)
		return -1;

	airbag_setpostinfo(fmt);

	vasprintf(&buf, fmt, args);
	if (DO_SSL(wp)) {
#ifdef HAVE_OPENSSL
		ret = sslbufferwrite((struct sslbuffer *)wp->fp_in, buf, strlen(buf));
#elif defined(HAVE_MATRIXSSL)
		ret = matrixssl_printf(wp->fp_in, "%s", buf);
#elif defined(HAVE_POLARSSL)
		fprintf(stderr, "SSL write buf %d\n", strlen(buf));
		ret = ssl_write((ssl_context *)wp->fp_in, buf, strlen(buf));
#endif
	} else {
		ret = fprintf(wp->fp_out, "%s", buf);
	}
	debug_free(buf);

	return ret;
}

size_t websWrite(webs_t wp, char *fmt, ...)
{
	if (!wp || !fmt)
		return -1;
	va_list args;
	va_start(args, fmt);
	size_t ret = vwebsWrite(wp, fmt, args);
	va_end(args);

	return ret;
}

size_t wfwrite(void *buf, size_t size, size_t n, webs_t wp)
{
	size_t ret;
	if (DO_SSL(wp)) {
#ifdef HAVE_OPENSSL
		{
			ret = sslbufferwrite((struct sslbuffer *)wp->fp_in, buf, n * size);
		}
#elif defined(HAVE_MATRIXSSL)
		ret = matrixssl_write(wp->fp_in, (unsigned char *)buf, n * size);
#elif defined(HAVE_POLARSSL)
		{
			fprintf(stderr, "SSL write buf %d\n", n * size);
			ret = ssl_write((ssl_context *)wp->fp_in, (unsigned char *)buf, n * size);
		}
#endif
	} else {
		ret = fwrite(buf, size, n, wp->fp_out);
	}
	return ret;
}

static int wfsendfile(int fd, off_t offset, size_t nbytes, webs_t wp)
{
	off_t lo = offset;
	return sendfile(wp->conn_fd, fd, &lo, nbytes);
}

static size_t wfread(void *p, size_t size, size_t n, webs_t wp)
{
	char *buf = (void *)p;
	size_t ret;
	int i;
	errno = 0;
	for (i = 0; i < 500; i++) {
		if (DO_SSL(wp)) {
#ifdef HAVE_OPENSSL
			ret = sslbufferread((struct sslbuffer *)wp->fp_in, buf, n * size);
#elif defined(HAVE_MATRIXSSL)
			//do it in chains
			size_t cnt = (size * n) / 0x4000;
			size_t i;
			size_t len = 0;

			for (i = 0; i < cnt; i++) {
				len += matrixssl_read(fp, buf, 0x4000);
				*buf += 0x4000;
			}
			len += matrixssl_read(wp->fp_in, buf, (size * n) % 0x4000);

			ret = len;
#elif defined(HAVE_POLARSSL)
			size_t len = n * size;
			fprintf(stderr, "read SSL %d\n", len);
			ret = ssl_read((ssl_context *)wp->fp_in, (unsigned char *)buf, &len);
#endif
		} else {
			ret = fread(buf, size, n, wp->fp_in);
		}
		if (!ret && (errno == EINTR || errno == EAGAIN)) {
			struct timespec tim, tim2;
			tim.tv_sec = 0;
			tim.tv_nsec = 10000000L;
			nanosleep(&tim, &tim2);
			continue;
		}
		break;
	}
	return ret;
}

int wfflush(webs_t wp)
{
	int ret;

	if (DO_SSL(wp)) {
#ifdef HAVE_OPENSSL
		/* ssl_write doesn't buffer */
		sslbufferflush((struct sslbuffer *)wp->fp_in);
		ret = 1;
#elif defined(HAVE_MATRIXSSL)
		ret = matrixssl_flush(wp->fp_in);
#elif defined(HAVE_POLARSSL)
		ssl_flush_output((ssl_context *)wp->fp_in);
		ret = 1;
#endif
	} else {
		ret = fflush(wp->fp_out);
	}
	return ret;
}

static int wfclose(webs_t wp)
{
	int ret = 0;
	wfflush(wp);
	struct timespec tim, tim2;
	tim.tv_sec = 0;
	tim.tv_nsec = 50000000;
	nanosleep(&tim, &tim2);

	if (DO_SSL(wp)) {
#ifdef HAVE_OPENSSL
		sslbufferflush((struct sslbuffer *)wp->fp_in);
		sslbufferfree((struct sslbuffer *)wp->fp_in);
		ret = 1;
#elif defined(HAVE_MATRIXSSL)
		ret = matrixssl_free_session(wp->fp_in);
#elif defined(HAVE_POLARSSL)
		ssl_close_notify((ssl_context *)wp->fp_in);
		ssl_free((ssl_context *)wp->fp_in);
		ret = 1;
#endif
		close(wp->conn_fd);
	} else {
		ret = fclose(wp->fp_in);
		ret |= fclose(wp->fp_out);
		wp->fp_in = NULL;
		wp->fp_out = NULL;
		close(wp->conn_fd);
		close(wp->conn_fd_out);
	}

	return ret;
}

#ifdef HAVE_IAS
static void ias_sid_set(webs_t wp)
{
	struct sysinfo sinfo;

	sysinfo(&sinfo);
	if (*(wp->http_client_mac)) {
		ias_sid_timeout = sinfo.uptime + 300;
		sprintf(ias_sid, "%s", wp->http_client_mac);
		fprintf(stderr, "[IAS SID SET] %d %s\n", ias_sid_timeout, ias_sid);
	}
	return;
}

static int ias_sid_valid(webs_t wp)
{
	struct sysinfo sinfo;
	char *mac;

	if (!ias_sid_timeout && !*(ias_sid))
		return 0;

	sysinfo(&sinfo);
	mac = wp->http_client_mac;
	if (sinfo.uptime > ias_sid_timeout || (strcmp(mac, ias_sid) && *(mac))) {
		fprintf(stderr, "[IAS SID RESET] %d<>%d %s<>%s\n", sinfo.uptime, ias_sid_timeout, mac, ias_sid);
		ias_sid_timeout = 0;
		sprintf(ias_sid, "");
		return 0;
	} else
		ias_sid_timeout = sinfo.uptime + 300;
	return 1;
}
#endif
