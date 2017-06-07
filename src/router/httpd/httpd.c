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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
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
#include <sys/wait.h>

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

#ifdef HAVE_IAS
#include <sys/sysinfo.h>
#endif
#ifndef HAVE_MICRO
#include <pthread.h>
#endif

#define SERVER_NAME "httpd"
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define TIMEOUT	5

/* A multi-family sockaddr. */
typedef union {
	struct sockaddr sa;
	struct sockaddr_in sa_in;
} usockaddr;

/* Globals. */
#ifdef FILTER_DEBUG
FILE *debout;
#endif

#if defined(HAVE_OPENSSL) || defined(HAVE_MATRIXSSL) || defined(HAVE_POLARSSL)

#define DEFAULT_HTTPS_PORT 443
#define CERT_FILE "/etc/cert.pem"
#define KEY_FILE "/etc/key.pem"
#endif

#ifdef HAVE_POLARSSL
int my_ciphers[] = {
	TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
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
	0
};

char *dhm_P =
    "E4004C1F94182000103D883A448B3F80"
    "2CE4B44A83301270002C20D0321CFD00"
    "11CCEF784C26A400F43DFB901BCA7538" "F2C6B176001CF5A0FD16D2C48B1D0C1C" "F6AC8E1DA6BCC3B4E1F96B0564965300" "FFA1D0B601EB2800F489AA512C4B248C" "01F76949A60BB7F00A40B1EAB64BDD48" "E8A700D60B7F1200FA8E77B0A979DABF";

char *dhm_G = "4";
//unsigned char session_table[SSL_SESSION_TBL_LEN];
#endif

#define DEFAULT_HTTP_PORT 80
int server_port;
char pid_file[80];
char *server_dir = NULL;

int httpd_level;

char http_client_ip[20];
extern char *get_mac_from_ip(char *ip);

/* Forwards. */
static int initialize_listen_socket(usockaddr * usaP);
static int auth_check(webs_t conn_fp);
static void send_error(webs_t conn_fp, int status, char *title, char *extra_header, char *text);
void send_headers(webs_t conn_fp, int status, char *title, char *extra_header, char *mime_type, int length, char *attach_file);
static int b64_decode(const char *str, unsigned char *space, int size);
static int match(const char *pattern, const char *string);
static int match_one(const char *pattern, int patternlen, const char *string);
static void *handle_request(void *conn_fp);
static int numthreads = 0;

#ifndef HAVE_MICRO
pthread_mutex_t httpd_mutex;
#endif

static int initialize_listen_socket(usockaddr * usaP)
{
	int listen_fd;
	int i;

	memset(usaP, 0, sizeof(usockaddr));
	usaP->sa.sa_family = AF_INET;
	usaP->sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
	usaP->sa_in.sin_port = htons(server_port);

	listen_fd = socket(usaP->sa.sa_family, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("socket");
		return -1;
	}
	(void)fcntl(listen_fd, F_SETFD, 1);
	i = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&i, sizeof(i)) < 0) {
		perror("setsockopt");
		return -1;
	}
	if (bind(listen_fd, &usaP->sa, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return -1;
	}
	if (listen(listen_fd, 1024) < 0) {
		perror("listen");
		return -1;
	}
	return listen_fd;
}

static int auth_check(webs_t conn_fp)
{
	char authinfo[500];
	unsigned char *authpass;
	int l;

	/* Is this directory unprotected? */
	if (!strlen(conn_fp->auth_passwd)) {
		/* Yes, let the request go through. */
		return 1;
	}

	/* Basic authorization info? */
	if (!conn_fp->authorization || strncmp(conn_fp->authorization, "Basic ", 6) != 0) {
		ct_syslog(LOG_INFO, httpd_level, "Authentication fail");

		return 0;
	}

	/* Decode it. */
	l = b64_decode(&(conn_fp->authorization[6]), (unsigned char *)authinfo, sizeof(authinfo));
	authinfo[l] = '\0';
	/* Split into user and password. */
	authpass = strchr((char *)authinfo, ':');
	if (authpass == (unsigned char *)0) {
		/* No colon?  Bogus auth info. */
		return 0;
	}
	*authpass++ = '\0';

	char *crypt(const char *, const char *);

	char buf1[36];
	char buf2[36];
	char *enc1;
	char *enc2;
	memdebug_enter();
	enc1 = crypt(authinfo, (const char *)conn_fp->auth_userid);

	if (strcmp(enc1, conn_fp->auth_userid)) {
		return 0;
	}
	char dummy[128];
	enc2 = crypt(authpass, (const char *)conn_fp->auth_passwd);
	if (strcmp(enc2, conn_fp->auth_passwd)) {
		syslog(LOG_INFO, "httpd login failure - bad passwd !\n");
		while (wfgets(dummy, 64, conn_fp) > 0) {
			//fprintf(stderr, "flushing %s\n", dummy);
		}
		return 0;
	}
	memdebug_leave();

	return 1;
}

void send_authenticate(webs_t conn_fp)
{
	u_int64_t auth_time = (u_int64_t)(atoll(nvram_safe_get("auth_time")));
	u_int64_t curr_time = (u_int64_t)time(NULL);
	char s_curr_time[24];
	sprintf(s_curr_time, "%llu", curr_time);

	char *header;

	(void)asprintf(&header, "WWW-Authenticate: Basic realm=\"%s\"", conn_fp->auth_realm);
	send_error(conn_fp, 401, "Unauthorized", header,
#if defined(HAVE_BUFFALO) && defined(HAVE_IAS)
		   "Authorization required. please note that the default username is \"admin\" in all newer releases");
#else
		   "Authorization required. please note that the default username is \"root\" in all newer releases");
#endif
	free(header);
	/* init these after a successful auth */
	nvram_set("auth_time", s_curr_time);
}

static void send_error(webs_t conn_fp, int status, char *title, char *extra_header, char *text)
{

	// jimmy, https, 8/4/2003, fprintf -> wfprintf, fflush -> wfflush
	send_headers(conn_fp, status, title, extra_header, "text/html", -1, NULL);
	(void)wfprintf(conn_fp, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY BGCOLOR=\"#cc9999\"><H4>%d %s</H4>\n", status, title, status, title);
	(void)wfprintf(conn_fp, "%s\n", text);
	(void)wfprintf(conn_fp, "</BODY></HTML>\n");
	(void)wfflush(conn_fp);
}

void send_headers(webs_t conn_fp, int status, char *title, char *extra_header, char *mime_type, int length, char *attach_file)
{
	time_t now;
	char timebuf[100];

	wfprintf(conn_fp, "%s %d %s\r\n", PROTOCOL, status, title);
	if (mime_type != (char *)0)
		wfprintf(conn_fp, "Content-Type: %s\r\n", mime_type);

	wfprintf(conn_fp, "Server: %s\r\n", SERVER_NAME);
	now = time((time_t *) 0);
	strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
	wfprintf(conn_fp, "Date: %s\r\n", timebuf);
	wfprintf(conn_fp, "Connection: close\r\n");
	wfprintf(conn_fp, "Cache-Control: no-store, no-cache, must-revalidate\r\n");
	wfprintf(conn_fp, "Cache-Control: post-check=0, pre-check=0\r\n");
	wfprintf(conn_fp, "Pragma: no-cache\r\n");
	if (attach_file)
		wfprintf(conn_fp, "Content-Disposition: attachment; filename=%s\r\n", attach_file);
	if (extra_header != (char *)0 && *extra_header)
		wfprintf(conn_fp, "%s\r\n", extra_header);
	if (length != -1)
		wfprintf(conn_fp, "Content-Length: %ld\r\n", length);
	wfprintf(conn_fp, "\r\n");
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
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 00-0F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 10-1F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,	/* 20-2F */
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,	/* 30-3F */
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,	/* 40-4F */
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,	/* 50-5F */
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	/* 60-6F */
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,	/* 70-7F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 80-8F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* 90-9F */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* A0-AF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* B0-BF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* C0-CF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* D0-DF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* E0-EF */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1	/* F0-FF */
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
	const char *or;

	for (;;) {
		or = strchr(pattern, '|');
		if (or == (char *)0)
			return match_one(pattern, strlen(pattern), string);
		if (match_one(pattern, or - pattern, string))
			return 1;
		pattern = or + 1;
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

static void do_file_2(struct mime_handler *handler, char *path, webs_t stream, char *query, char *attach)	//jimmy, https, 8/4/2003
{

	size_t len;
	FILE *web = getWebsFile(path);

	if (web == NULL) {
		if (!(web = fopen(path, "rb")))
			return;
		fseek(web, 0, SEEK_END);
		len = ftell(web);
		fseek(web, 0, SEEK_SET);

	} else {
		len = getWebsFileLen(path);
	}
	if (!handler->send_headers)
		send_headers(stream, 200, "Ok", handler->extra_header, handler->mime_type, len, attach);
	char *buffer = malloc(4096);
	while (len) {
		size_t ret = fread(buffer, 1, len > 4096 ? 4096 : len, web);
		len -= ret;
		wfwrite(buffer, ret, 1, stream);
	}
	free(buffer);
	fclose(web);
}

void
//do_file(char *path, FILE *stream)
do_file(unsigned char method, struct mime_handler *handler, char *path, webs_t stream, char *query)	//jimmy, https, 8/4/2003
{

	do_file_2(handler, path, stream, query, NULL);
}

void do_file_attach(struct mime_handler *handler, char *path, webs_t stream, char *query, char *attachment)	//jimmy, https, 8/4/2003
{

	do_file_2(handler, path, stream, query, attachment);
}

static int check_connect_type(void)
{
	struct wl_assoc_mac *wlmac = NULL;
	int count_wl = 0;
	int i, j, ret = 0;
	char temp[32];
	int c = get_wl_instances();

	for (j = 0; j < c; j++) {
		sprintf(temp, "wl%d_web_filter", j);
		if (nvram_invmatchi(temp, 1))
			continue;

		wlmac = get_wl_assoc_mac(j, &count_wl);

		for (i = 0; i < count_wl; i++) {
			if (!strcmp(wlmac[i].mac, nvram_safe_get("http_client_mac"))) {
				cprintf("Can't accept wireless access\n");
				ret = -1;
			}
		}
		free(wlmac);
	}

	return ret;
}

static char *last_log_ip = NULL;
static int registered = -1;
static int registered_real = -1;
#ifdef HAVE_IAS
char ias_sid[20];
char ias_http_client_mac[20];
int ias_sid_timeout;
void ias_sid_set();
int ias_sid_valid();
#endif

#define LINE_LEN 10000
static void *handle_request(void *arg)
{
	webs_t conn_fp = (webs_t)arg;
	char *query;
	char *cur;
	char *method, *path, *protocol, *authorization, *boundary, *referer, *host, *useragent, *language;
	char *cp;
	char *file = NULL;
	FILE *exec;
	int len;
	struct mime_handler *handler;
	int cl = 0, count, flags;
	char line[LINE_LEN + 1];
	long method_type;

	/* Initialize the request variables. */
	authorization = referer = boundary = host = NULL;
	bzero(line, sizeof(line));
	/* Parse the first line of the request. */
	int cnt = 0;
	char *str;
	for (cnt = 0; cnt < 10; cnt++) {
		str = wfgets(line, LINE_LEN, conn_fp);
		if (strlen(line) > 0)
			break;
	}
	if (!strlen(line)) {
		send_error(conn_fp, 400, "Bad Request", (char *)0, "No request found.");
		goto out;
	}

	/* To prevent http receive https packets, cause http crash (by honor 2003/09/02) */
	if (strncasecmp(line, "GET", 3) && strncasecmp(line, "POST", 4) && strncasecmp(line, "OPTIONS", 7)) {
		goto out;
	}
	method = path = line;
	strsep(&path, " ");
	if (!path) {		// Avoid http server crash, added by honor 2003-12-08
		send_error(conn_fp, 400, "Bad Request", (char *)0, "Can't parse request.");
		goto out;
	}
	while (*path == ' ')
		path++;
	protocol = path;
	strsep(&protocol, " ");
	if (!protocol) {	// Avoid http server crash, added by honor 2003-12-08
		send_error(conn_fp, 400, "Bad Request", (char *)0, "Can't parse request.");
		goto out;
	}
	while (*protocol == ' ')
		protocol++;
	cp = protocol;
	strsep(&cp, " ");
	cur = protocol + strlen(protocol) + 1;
	/* Parse the rest of the request headers. */

	while (wfgets(cur, line + LINE_LEN - cur, conn_fp) != 0)	//jimmy,https,8/4/2003
	{

		if (strcmp(cur, "\n") == 0 || strcmp(cur, "\r\n") == 0) {
			break;
		} else if (strncasecmp(cur, "Authorization:", 14) == 0) {
			cp = &cur[14];
			cp += strspn(cp, " \t");
			authorization = cp;
			cur = cp + strlen(cp) + 1;
		} else if (strncasecmp(cur, "Referer:", 8) == 0) {
			cp = &cur[8];
			cp += strspn(cp, " \t");
			referer = cp;
			cur = cp + strlen(cp) + 1;
		} else if (strncasecmp(cur, "Host:", 5) == 0) {
			cp = &cur[5];
			cp += strspn(cp, " \t");
			host = cp;
			cur = cp + strlen(cp) + 1;
		} else if (strncasecmp(cur, "Content-Length:", 15) == 0) {
			cp = &cur[15];
			cp += strspn(cp, " \t");
			cl = strtoul(cp, NULL, 0);

		} else if ((cp = strstr(cur, "boundary="))) {
			boundary = &cp[9];
			for (cp = cp + 9; *cp && *cp != '\r' && *cp != '\n'; cp++) ;
			*cp = '\0';
			cur = ++cp;
		} else if (strncasecmp(cur, "User-Agent:", 11) == 0) {
			cp = &cur[11];
			cp += strspn(cp, " \t");
			useragent = cp;
			cur = cp + strlen(cp) + 1;
		} else if (strncasecmp(cur, "Accept-Language:", 16) == 0) {
			cp = &cur[17];
			cp += strspn(cp, " \t");
			language = cp;
			cur = cp + strlen(cp) + 1;
		}
	}
	method_type = METHOD_INVALID;
	if (!strcasecmp(method, "get"))
		method_type = METHOD_GET;
	if (!strcasecmp(method, "post"))
		method_type = METHOD_POST;
	if (!strcasecmp(method, "options"))
		method_type = METHOD_OPTIONS;

	if (method_type == METHOD_INVALID) {
		send_error(conn_fp, 501, "Not Implemented", (char *)0, "That method is not implemented.");
		goto out;
	}
	if (path[0] != '/') {
		send_error(conn_fp, 400, "Bad Request", (char *)0, "Bad filename.");
		goto out;
	}
	file = &(path[1]);
	len = strlen(file);
	if (file[0] == '/' || strcmp(file, "..") == 0 || strncmp(file, "../", 3) == 0 || strstr(file, "/../") != (char *)0 || strcmp(&(file[len - 3]), "/..") == 0) {
		send_error(conn_fp, 400, "Bad Request", (char *)0, "Illegal filename.");
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

		ISOMAP isomap[] = {
			{
			 "de", "german"},	//
			{
			 "es", "spanish"},	//
			{
			 "fr", "french"},	//
			{
			 "hr", "croatian"},	//
			{
			 "hu", "hungarian"},	//
			{
			 "nl", "dutch"},	//
			{
			 "it", "italian"},	//
			{
			 "lv", "latvian"},	//
			{
			 "jp", "japanese"},	//
			{
			 "pl", "polish"},	//
			{
			 "pt", "portuguese_braz"},	// 
			{
			 "ro", "romanian"},	//
			{
			 "ru", "russian", "RU"},	// 
			{
			 "sl", "slovenian"},	//
			{
			 "sr", "serbian"},	//
			{
			 "sv", "swedish"},	//
			{
			 "zh", "chinese_simplified"},	//
			{
			 "tr", "turkish"},	//
			NULL
		};
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
		send_error(conn_fp, 400, "Bad Request", (char *)0, "Cross Site Action detected!");
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
					send_error(conn_fp, 400, "Bad Request", (char *)0, "Cross Site Action detected!");
					goto out;
				}
				if (host[c++] != referer[a]) {
					send_error(conn_fp, 400, "Bad Request", (char *)0, "Cross Site Action detected!");
					goto out;
				}
				if (c == hlen) {
					a++;
					break;
				}
			}
			if (c != hlen || referer[a] != '/') {
				send_error(conn_fp, 400, "Bad Request", (char *)0, "Cross Site Action detected!");
				goto out;
			}
		}

	}
	// seg change for status site
#ifdef HAVE_REGISTER
	if (registered_real == -1) {
		registered_real = isregistered_real();
	}
	if (!registered_real)
		registered = isregistered();
	else
		registered = registered_real;
#endif

	// save the originally requested url
	conn_fp->request_url = strdup(file);

#ifdef HAVE_SKYTRON
	if (file[0] == '\0' || file[len - 1] == '/') {
		file = "setupindex.asp";
	}
#else
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

		if (!endswith(file, ".js") && !endswith(file, "detect.asp")
		    && ias_detected == 0 && nvram_matchi("ias_startup", 3)) {

			fprintf(stderr, "[HTTP PATH] %s redirect\n", file);
			sprintf(redirect_path, "Location: http://%s/detect.asp", nvram_get("lan_ipaddr"));
			send_headers(conn_fp, 302, "Found", redirect_path, "", -1, NULL);
			goto out;

		} else if (ias_detected == 1) {
			fprintf(stderr, "[HTTP PATH] %s redirected\n", file);
			nvram_seti("ias_startup", 2);
		}
	}

	if (!ias_sid_valid()
	    && (endswith(file, "InternetAtStart.asp") || endswith(file, "detect.asp") || endswith(file, "?ias_detect"))
	    && strstr(useragent, "Android"))
		ias_sid_set();

	char hostname[32];
	if (strlen(host) < 32 && ias_detected == 1 && (nvram_matchi("ias_dnsresp", 1))) {
		strncpy(hostname, host, strspn(host, "123456789.:"));
		if (!strcmp(hostname, nvram_get("lan_ipaddr"))) {

			nvram_unset("ias_dnsresp");
			char cmd[64];
			sprintf(cmd, "%s:55300", nvram_get("lan_ipaddr"));
			eval("iptables", "-t", "nat", "-D", "PREROUTING", "-i", "br0", "-p", "udp", "--dport", "53", "-j", "DNAT", "--to", cmd);

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

		if (server_dir != NULL && strcmp(server_dir, "/www"))	// to allow to use router as a WEB server
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
#endif

	{
		if (strncmp(file, "bigfile.bin", 11)) {
			/* extract url args if present */
			query = strchr(file, '?');
			if (query) {
				//see token length in createpageToken
				char token[16] = "0";
				strncpy(token, &query[1], sizeof(token));
				nvram_set("token", token);
				*query++ = 0;
			} else {
				nvram_seti("token", 0);
			}
		}
		int changepassword = 0;
#ifdef HAVE_REGISTER
		if (!registered_real) {
			if (endswith(file, "About.htm"))
				file = "register.asp";
		}
		if (!registered) {
			if (endswith(file, ".asp"))
				file = "register.asp";
			else if (endswith(file, ".htm"))
				file = "register.asp";
			else if (endswith(file, ".html"))
				file = "register.asp";
		} else
#endif
		{
			if (((nvram_match("http_username", DEFAULT_USER)
			      && nvram_match("http_passwd", DEFAULT_PASS))
			     || nvram_match("http_username", "")
			     || nvram_match("http_passwd", "admin"))
			    && !endswith(file, "register.asp")
			    && !endswith(file, "vsp.html")) {
				changepassword = 1;
				if (endswith(file, ".asp"))
					file = "changepass.asp";
				else if (endswith(file, ".htm")
					 && !endswith(file, "About.htm"))
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
		int file_found = 1;

		for (handler = &mime_handlers[0]; handler->pattern; handler++) {
			if (match(handler->pattern, file)) {
#ifdef HAVE_REGISTER
				if (registered)
#endif
				{
					memdebug_enter();
					if (!changepassword && handler->auth && (!handler->handle_options || method_type != METHOD_OPTIONS)) {
						conn_fp->authorization = authorization;
						int result = handler->auth(conn_fp, auth_check);

#ifdef HAVE_IAS
						if (!result && !((!strcmp(file, "apply.cgi") || !strcmp(file, "InternetAtStart.ajax.asp"))
								 && strstr(useragent, "Android")
								 && ias_sid_valid())) {
							fprintf(stderr, "[AUTH FAIL]: %s", useragent);
#else
						if (!result) {
#endif
							conn_fp->auth_fail = 0;
							send_authenticate(conn_fp);
							goto out;
						}
					}
					memdebug_leave_info("auth");
				}
				conn_fp->post = 0;
				if (method_type == METHOD_POST) {
					conn_fp->post = 1;
				}
				memdebug_enter();
				if (handler->input)
					handler->input(file, conn_fp, cl, boundary);
				memdebug_leave_info("input");
#if defined(linux)
#ifdef HAVE_HTTPS
				if (!conn_fp->do_ssl && (flags = fcntl(fileno(conn_fp->fp), F_GETFL)) != -1 && fcntl(fileno(conn_fp->fp), F_SETFL, flags | O_NONBLOCK) != -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp->fp) != EOF)
						(void)fgetc(conn_fp->fp);
					fcntl(fileno(conn_fp->fp), F_SETFL, flags);
				}
#else
				if ((flags = fcntl(fileno(conn_fp->fp), F_GETFL)) != -1 && fcntl(fileno(conn_fp->fp), F_SETFL, flags | O_NONBLOCK) != -1) {
					/* Read up to two more characters */
					if (fgetc(conn_fp->fp) != EOF)
						(void)fgetc(conn_fp->fp);
					fcntl(fileno(conn_fp->fp), F_SETFL, flags);
				}
#endif
#endif
				memdebug_enter();
				if (check_connect_type() < 0) {
					send_error(conn_fp, 401, "Bad Request", (char *)0, "Can't use wireless interface to access GUI.");
					goto out;
				}
				memdebug_leave_info("connect");
				memdebug_enter();
				if (conn_fp->auth_fail == 1) {
					send_authenticate(conn_fp);
					conn_fp->auth_fail = 0;
					goto out;
				} else {
					if (handler->output != do_file)
						if (handler->send_headers)
							send_headers(conn_fp, 200, "Ok", handler->extra_header, handler->mime_type, -1, NULL);
				}
				memdebug_leave_info("auth_output");
				memdebug_enter();
				// check for do_file handler and check if file exists
				file_found = 1;
				if (handler->output == do_file) {
					if (getWebsFileLen(file) == 0) {
						if (!(fp = fopen(file, "rb"))) {
							file_found = 0;
						} else {
							fclose(fp);
						}
					}
				}
				if (handler->output && file_found) {
					handler->output(method_type, handler, file, conn_fp, query);
				} else {
					send_error(conn_fp, 404, "Not Found", (char *)0, "File not found.");
				}
				break;
				memdebug_leave_info("output");
			}

			if (!handler->pattern)
				send_error(conn_fp, 404, "Not Found", (char *)0, "File not found.");
		}
	}

      out:;
	wfclose(conn_fp);
	close(conn_fp->conn_fd);
	if (conn_fp->request_url)
		free(conn_fp->request_url);
	if (conn_fp->post_buf)
		free(conn_fp->post_buf);
//      fprintf(stderr, "destroy thread %d\n", conn_fp->threadid);
	memset(conn_fp, 0, sizeof(webs));	// erase to delete any traces of stored passwords or usernames

	free(conn_fp);
	pthread_mutex_lock(&httpd_mutex);
	numthreads--;
	pthread_mutex_unlock(&httpd_mutex);

	return NULL;
}

void				// add by honor 2003-04-16
get_client_ip_mac(int conn_fp)
{
	struct sockaddr_in sa;
	unsigned int len = sizeof(struct sockaddr_in);
	char *m;

	getpeername(conn_fp, (struct sockaddr *)&sa, &len);
	char client[32];
	char *peer = (char *)inet_ntop(AF_INET, &sa.sin_addr, client, 16);

	nvram_set("http_client_ip", peer);
	m = get_mac_from_ip(peer);
	nvram_set("http_client_mac", m);
#ifdef HAVE_IAS
	sprintf(ias_http_client_mac, "%s\0", m);
#endif
}

static void handle_server_sig_int(int sig)
{
	ct_syslog(LOG_INFO, httpd_level, "httpd server shutdown");
	exit(0);
}

void settimeouts(int sock, int secs)
{
	struct timeval tv;

	tv.tv_sec = secs;
	tv.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
		perror("setsockopt(SO_SNDTIMEO)");
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
		perror("setsockopt(SO_RCVTIMEO)");
}

static void sigchld(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0) ;
}

static void set_sigchld_handler(void)
{
	struct sigaction act;

	memset(&act, 0, sizeof(act));
	act.sa_handler = sigchld;
	sigaction(SIGCHLD, &act, 0);
}

#ifdef HAVE_OPENSSL
#define SSLBUFFERSIZE 16384
struct sslbuffer {
	unsigned char *sslbuffer;
	unsigned int count;
	SSL *ssl;
};

static struct sslbuffer *initsslbuffer(SSL * ssl)
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
	free(buffer->sslbuffer);
	int sockfd = SSL_get_fd(buffer->ssl);
	SSL_shutdown(buffer->ssl);
	close(sockfd);
	SSL_free(buffer->ssl);
	free(buffer);
}

static int sslbufferread(struct sslbuffer *buffer, char *data, int datalen)
{
	return SSL_read(buffer->ssl, data, datalen);
}

static int sslbufferpeek(struct sslbuffer *buffer, char *data, int datalen)
{
	return SSL_peek(buffer->ssl, data, datalen);
}

static int sslbufferwrite(struct sslbuffer *buffer, char *data, int datalen)
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
	usockaddr usa;
	int listen_fd;
	int conn_fd;
	socklen_t sz = sizeof(usa);
	int c;
	int timeout = TIMEOUT;
	struct stat stat_dir;

	set_sigchld_handler();
	nvram_seti("gozila_action", 0);
#ifdef HAVE_HTTPS
	int do_ssl = 0;
#endif
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
#ifndef HAVE_MICRO
	pthread_mutex_init(&httpd_mutex, NULL);
#endif
	strcpy(pid_file, "/var/run/httpd.pid");
	server_port = DEFAULT_HTTP_PORT;

	while ((c = getopt(argc, argv, "Sih:p:d:t:s:g:e:")) != -1)
		switch (c) {
#ifdef HAVE_HTTPS
		case 'S':
#if defined(HAVE_OPENSSL) || defined(HAVE_MATRIXSSL) || defined(HAVE_POLARSSL)
			do_ssl = 1;
			server_port = DEFAULT_HTTPS_PORT;
			strcpy(pid_file, "/var/run/httpsd.pid");
#else
			fprintf(stderr, "No SSL support available\n");
			exit(0);
#endif
			break;
#endif
		case 'h':
			server_dir = optarg;
			break;
		case 'p':
			server_port = atoi(optarg);
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
			fprintf(stderr, "Usage: %s [-S] [-p port]\n"
#ifdef HAVE_HTTPS
				"	-S : Support https (port 443)\n"
#endif
				"	-p port : Which port to listen?\n" "	-t secs : How many seconds to wait before timing out?\n"
#ifdef DEBUG_CIPHER
				"	-s ciphers: set cipher lists\n" "	-g: get cipher lists\n"
#endif
				"	-h: home directory: use directory\n", argv[0]);
			exit(0);
			break;
		default:
			break;
		}

	httpd_level = ct_openlog("httpd", LOG_PID | LOG_NDELAY, LOG_DAEMON, "LOG_HTTPD");
#ifdef HAVE_HTTPS
	ct_syslog(LOG_INFO, httpd_level, "httpd server %sstarted at port %d\n", do_ssl ? "(ssl support) " : "", server_port);
#else
	ct_syslog(LOG_INFO, httpd_level, "httpd server started at port %d\n", server_port);
#endif
	/* Ignore broken pipes */
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, handle_server_sig_int);	// kill

	if (server_dir && stat(server_dir, &stat_dir) == 0)
		chdir(server_dir);

	/* Build our SSL context */
#ifdef HAVE_HTTPS
	if (do_ssl) {
#ifdef HAVE_OPENSSL
		SSLeay_add_ssl_algorithms();
		SSL_load_error_strings();
		ctx = SSL_CTX_new(SSLv23_server_method());
#ifdef HAVE_CUSTOMSSLCERT
		if (SSL_CTX_use_certificate_file(ctx, nvram_safe_get("https_cert_file"), SSL_FILETYPE_PEM)
#else
		if (SSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM)
#endif
		    == 0) {
			cprintf("Can't read %s\n", CERT_FILE);
			ERR_print_errors_fp(stderr);
			exit(1);

		}
#ifdef HAVE_CUSTOMSSLCERT
		if (SSL_CTX_use_PrivateKey_file(ctx, nvram_safe_get("https_key_file"), SSL_FILETYPE_PEM)
#else
		if (SSL_CTX_use_PrivateKey_file(ctx, KEY_FILE, SSL_FILETYPE_PEM)
#endif
		    == 0) {
			cprintf("Can't read %s\n", KEY_FILE);
			ERR_print_errors_fp(stderr);
			exit(1);

		}
		if (SSL_CTX_check_private_key(ctx) == 0) {
			cprintf("Check private key fail\n");
			ERR_print_errors_fp(stderr);
			exit(1);
		}
#endif
#ifdef HAVE_POLARSSL
		if ((ret = ctr_drbg_init(&ctr_drbg, entropy_func, &entropy, (const unsigned char *)pers, strlen(pers))) != 0) {
			fprintf(stderr, " failed\n  ! ctr_drbg_init returned %d\n", ret);
		}

		memset(&ssl, 0, sizeof(ssl));
		memset(&srvcert, 0, sizeof(x509_crt));
		x509_crt_init(&srvcert);
		ret = x509_crt_parse_file(&srvcert, CERT_FILE);
		if (ret != 0) {
			printf("x509_read_crtfile failed\n");
			exit(0);
		}
		ret = pk_parse_keyfile(&rsa, KEY_FILE, NULL);
		if (ret != 0) {
			printf("x509_read_keyfile failed\n");
			exit(0);
		}
#endif

#ifdef HAVE_MATRIXSSL
		matrixssl_init();
#ifdef HAVE_CUSTOMSSLCERT
		if (f_exists(nvram_safe_get("https_cert_file")) && f_exists(nvram_safe_get("https_key_file"))) {
			if (0 != matrixSslReadKeys(&keys, nvram_safe_get("https_cert_file"), nvram_safe_get("https_key_file"), NULL, NULL)) {
				fprintf(stderr, "Error reading or parsing %s / %s.\n", nvram_safe_get("https_cert_file"), nvram_safe_get("https_key_file"));
			}
		} else
#endif
		{
			if (0 != matrixSslReadKeys(&keys, CERT_FILE, KEY_FILE, NULL, NULL)) {
				fprintf(stderr, "Error reading or parsing %s.\n", KEY_FILE);
				exit(0);
			}
		}
#endif
	}
#endif

	/* Initialize listen socket */
	if ((listen_fd = initialize_listen_socket(&usa)) < 0) {
		ct_syslog(LOG_ERR, httpd_level, "Can't bind to any address");
		exit(errno);
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
		webs_t conn_fp;
		conn_fp = safe_malloc(sizeof(webs));
		if (!conn_fp) {
			ct_syslog(LOG_ERR, httpd_level, "Out of memory while creating new connection");
			continue;
		}
		memset(conn_fp, 0, sizeof(webs));

		if ((conn_fp->conn_fd = accept(listen_fd, &usa.sa, &sz)) < 0) {
			perror("accept");
			return errno;
		}

		/* Make sure we don't linger a long time if the other end disappears */
		settimeouts(conn_fp->conn_fd, timeout);
		fcntl(conn_fp->conn_fd, F_SETFD, fcntl(conn_fp->conn_fd, F_GETFD) | FD_CLOEXEC);

		if (check_action() == ACT_TFTP_UPGRADE ||	// We don't want user to use web during tftp upgrade.
		    check_action() == ACT_SW_RESTORE || check_action() == ACT_HW_RESTORE) {
			fprintf(stderr, "http(s)d: nothing to do...\n");
			return -1;
		}
#ifdef HAVE_HTTPS
		if (do_ssl) {
			if (check_action() == ACT_WEB_UPGRADE) {	// We don't want user to use web (https) during web (http) upgrade.
				fprintf(stderr, "httpsd: nothing to do...\n");
				return -1;
			}
#ifdef HAVE_OPENSSL
			const char *allowedCiphers =
			    "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
			conn_fp->ssl = SSL_new(ctx);

#ifdef NID_X9_62_prime256v1
			EC_KEY *ecdh = NULL;
			if (ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1)) {
				SSL_CTX_set_tmp_ecdh(ctx, ecdh);
				EC_KEY_free(ecdh);
			}
#endif

			// Setup available ciphers
			SSL_CTX_set_cipher_list(ctx, allowedCiphers);

			// Enforce our desired cipher order, disable obsolete protocols
			SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_SAFARI_ECDHE_ECDSA_BUG);

			SSL_set_fd(conn_fp->ssl, conn_fp->conn_fd);
			r = SSL_accept(conn_fp->ssl);
			if (r <= 0) {
				//berr_exit("SSL accept error");
//                              ERR_print_errors_fp(stderr);
//                              fprintf(stderr,"ssl accept return %d, ssl error %d %d\n",r,SSL_get_error(ssl,r),RAND_status());
				ct_syslog(LOG_ERR, httpd_level, "SSL accept error");
				close(conn_fp->conn_fd);
				SSL_free(conn_fp->ssl);
				continue;
			}

			conn_fp->fp = (FILE *) initsslbuffer(conn_fp->ssl);

#elif defined(HAVE_MATRIXSSL)
			matrixssl_new_session(conn_fp->conn_fd);

			conn_fp->fp = (FILE *) conn_fp->conn_fd;
#endif
#ifdef HAVE_POLARSSL
			ssl_free(&conn_fp->ssl);
			if ((ret = ssl_init(&conn_fp->ssl)) != 0) {
				printf("ssl_init failed\n");
				close(conn_fp->conn_fd);
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
				continue;
			}

			conn_fp->fp = (webs_t)(&ssl);
#endif
		} else
#endif
		{
#ifdef HAVE_HTTPS
			if (check_action() == ACT_WEBS_UPGRADE) {	// We don't want user to use web (http) during web (https) upgrade.
				fprintf(stderr, "httpd: nothing to do...\n");
				return -1;
			}
#endif

			if (!(conn_fp->fp = fdopen(conn_fp->conn_fd, "r+"))) {
				perror("fdopen");
				return errno;
			}
		}

		memdebug_enter();
		get_client_ip_mac(conn_fp->conn_fd);
		memdebug_leave_info("get_client_ip_mac");

		memdebug_enter();
#ifndef HAVE_MICRO
		pthread_mutex_lock(&httpd_mutex);
		numthreads++;
		pthread_mutex_unlock(&httpd_mutex);
		while (numthreads > 15) {
			fprintf(stderr, "thread limit of %d reached, waiting for release\n", numthreads);
			sleep(1);
		}
		conn_fp->threadid = numthreads;

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_t *thread = malloc(sizeof(pthread_t));
		conn_fp->do_ssl = do_ssl;
		if (pthread_create(thread, &attr, handle_request, conn_fp) != 0)
			fprintf(stderr, "Failed to create thread\n");
		pthread_attr_destroy(&attr);

//              conn_fp->do_ssl = do_ssl;
//              FORK(handle_request(conn_fp));
#else
		handle_request(conn_fp);
#endif
		memdebug_leave_info("handle_request");

		showmemdebugstat();
	}

	shutdown(listen_fd, 2);
	close(listen_fd);

	return 0;
}

char *wfgets(char *buf, int len, webs_t wp)
{
	FILE *fp = wp->fp;
#ifdef HAVE_HTTPS
#ifdef HAVE_OPENSSL
	if (wp->do_ssl) {
		int eof = 1;
		int i;
		char c;
		if (sslbufferpeek((struct sslbuffer *)fp, buf, len) <= 0)
			return NULL;
		for (i = 0; i < len; i++) {
			c = buf[i];
			if (c == '\n' || c == 0) {
				eof = 0;
				break;
			}
		}
		if (sslbufferread((struct sslbuffer *)fp, buf, i + 1) <= 0)
			return NULL;
		if (!eof) {
			buf[i + 1] = 0;
			return buf;
		} else
			return NULL;

	} else
#elif defined(HAVE_MATRIXSSL)
	if (wp->do_ssl)
		return (char *)matrixssl_gets(fp, buf, len);
	else
#elif defined(HAVE_POLARSSL)

	fprintf(stderr, "ssl read %d\n", len);
	if (wp->do_ssl) {
		int ret = ssl_read((ssl_context *) fp, (unsigned char *)buf, len);
		fprintf(stderr, "returns %d\n", ret);
		return (char *)buf;
	} else
#endif
#endif
		return fgets(buf, len, fp);
}

int wfputs(char *buf, webs_t wp)
{
	FILE *fp = wp->fp;
#ifdef HAVE_HTTPS
	if (wp->do_ssl)
#ifdef HAVE_OPENSSL
	{

		return sslbufferwrite((struct sslbuffer *)fp, buf, strlen(buf));
	}
#elif defined(HAVE_MATRIXSSL)
		return matrixssl_puts(fp, buf);
#elif defined(HAVE_POLARSSL)
	{
		fprintf(stderr, "ssl write str %d\n", strlen(buf));
		return ssl_write((ssl_context *) fp, (unsigned char *)buf, strlen(buf));
	}
#endif
	else
#endif
		return fputs(buf, fp);
}

int wfprintf(webs_t wp, char *fmt, ...)
{
	FILE *fp = wp->fp;
	va_list args;
	char *buf;
	int ret;

	va_start(args, fmt);
	vasprintf(&buf, fmt, args);
#ifdef HAVE_HTTPS
	if (wp->do_ssl)
#ifdef HAVE_OPENSSL
	{
		ret = sslbufferwrite((struct sslbuffer *)fp, buf, strlen(buf));
	}
#elif defined(HAVE_MATRIXSSL)
		ret = matrixssl_printf(fp, "%s", buf);
#elif defined(HAVE_POLARSSL)
	{
		fprintf(stderr, "ssl write buf %d\n", strlen(buf));
		ret = ssl_write((ssl_context *) fp, buf, strlen(buf));
	}
#endif
	else
#endif
		ret = fprintf(fp, "%s", buf);
	free(buf);
	va_end(args);
	return ret;
}

int websWrite(webs_t wp, char *fmt, ...)
{
	va_list args;
	char *buf;
	int ret;
	if (!wp || !fmt)
		return -1;
	FILE *fp = wp->fp;

	va_start(args, fmt);
	vasprintf(&buf, fmt, args);
#ifdef HAVE_HTTPS
	if (wp->do_ssl)
#ifdef HAVE_OPENSSL
	{
		ret = sslbufferwrite((struct sslbuffer *)fp, buf, strlen(buf));
	}
#elif defined(HAVE_MATRIXSSL)
		ret = matrixssl_printf(fp, "%s", buf);
#elif defined(HAVE_POLARSSL)
	{
		fprintf(stderr, "ssl write buf %d\n", strlen(buf));
		ret = ssl_write((ssl_context *) fp, buf, strlen(buf));
	}
#endif
	else
#endif
		ret = fprintf(fp, "%s", buf);
	free(buf);
	va_end(args);
	return ret;
}

size_t wfwrite(char *buf, int size, int n, webs_t wp)
{
	FILE *fp = wp->fp;
#ifdef HAVE_HTTPS
	if (wp->do_ssl)
#ifdef HAVE_OPENSSL
	{
		return sslbufferwrite((struct sslbuffer *)fp, buf, n * size);
	}
#elif defined(HAVE_MATRIXSSL)
		return matrixssl_write(fp, (unsigned char *)buf, n * size);
#elif defined(HAVE_POLARSSL)
	{
		fprintf(stderr, "ssl write buf %d\n", n * size);
		return ssl_write((ssl_context *) fp, (unsigned char *)buf, n * size);
	}
#endif
	else
#endif
		return fwrite(buf, size, n, fp);
}

size_t wfread(char *buf, int size, int n, webs_t wp)
{
	FILE *fp = wp->fp;

#ifdef HAVE_HTTPS
	if (wp->do_ssl) {
#ifdef HAVE_OPENSSL
		return sslbufferread((struct sslbuffer *)fp, buf, n * size);
#elif defined(HAVE_MATRIXSSL)
		//do it in chains
		int cnt = (size * n) / 0x4000;
		int i;
		int len = 0;

		for (i = 0; i < cnt; i++) {
			len += matrixssl_read(fp, buf, 0x4000);
			*buf += 0x4000;
		}
		len += matrixssl_read(fp, buf, (size * n) % 0x4000);

		return len;
#elif defined(HAVE_POLARSSL)
		int len = n * size;
		fprintf(stderr, "read ssl %d\n", len);
		return ssl_read((ssl_context *) fp, (unsigned char *)buf, &len);
#endif
	} else
#endif
		return fread(buf, size, n, fp);
}

int wfflush(webs_t wp)
{
	FILE *fp = wp->fp;

#ifdef HAVE_HTTPS
	if (wp->do_ssl) {
#ifdef HAVE_OPENSSL
		/* ssl_write doesn't buffer */
		sslbufferflush((struct sslbuffer *)fp);
		return 1;
#elif defined(HAVE_MATRIXSSL)
		return matrixssl_flush(fp);
#elif defined(HAVE_POLARSSL)
		ssl_flush_output((ssl_context *) fp);
		return 1;
#endif
	} else
#endif
		return fflush(fp);
}

int wfclose(webs_t wp)
{
	FILE *fp = wp->fp;

#ifdef HAVE_HTTPS
	if (wp->do_ssl) {
#ifdef HAVE_OPENSSL
		sslbufferflush((struct sslbuffer *)fp);
		sslbufferfree((struct sslbuffer *)fp);
		return 1;
#elif defined(HAVE_MATRIXSSL)
		return matrixssl_free_session(fp);
#elif defined(HAVE_POLARSSL)
		ssl_close_notify((ssl_context *) fp);
		ssl_free((ssl_context *) fp);
		return 1;
#endif
	} else
#endif
	{
		int ret = fclose(fp);
		wp->fp = NULL;
		return ret;
	}
}

#ifdef HAVE_IAS
void ias_sid_set()
{

	struct sysinfo sinfo;

	sysinfo(&sinfo);
	if (strlen(ias_http_client_mac)) {
		ias_sid_timeout = sinfo.uptime + 300;
		sprintf(ias_sid, "%s", ias_http_client_mac);
		fprintf(stderr, "[IAS SID SET] %d %s\n", ias_sid_timeout, ias_sid);
	}
	return;
}

int ias_sid_valid()
{

	struct sysinfo sinfo;
	char *mac;

	if (!ias_sid_timeout && (!ias_sid || !strcmp(ias_sid, "")))
		return 0;

	sysinfo(&sinfo);
	mac = ias_http_client_mac;
	if (sinfo.uptime > ias_sid_timeout || (strcmp(mac, ias_sid) && strlen(mac))) {
		fprintf(stderr, "[IAS SID RESET] %d<>%d %s<>%s\n", sinfo.uptime, ias_sid_timeout, mac, ias_sid);
		ias_sid_timeout = 0;
		sprintf(ias_sid, "");
		return 0;
	} else
		ias_sid_timeout = sinfo.uptime + 300;
	return 1;
}
#endif
