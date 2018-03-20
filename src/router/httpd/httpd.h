/*
 * milli_httpd - pretty small HTTP server
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: httpd.h,v 1.2 2005/11/24 19:38:45 seg Exp $
 */

#include <cy_conf.h>
#ifndef _httpd_h_
#define _httpd_h_

#if defined(DEBUG) && defined(DMALLOC)
#include <dmalloc.h>
#endif

#ifdef HAVE_MATRIXSSL
#define DDWRT
#include <matrixSsl.h>
#include <matrixssl_xface.h>
#endif

#ifdef HAVE_OPENSSL
#include <ssl.h>
extern BIO *bio_err;
#endif

#include <bcmnvram.h>
#if defined(linux)
/* Use SVID search */
#define __USE_GNU
#include <search.h>
#endif

#define AUTH_MAX 64

#define USE_LAN 0
#define USE_WAN 1

typedef struct {
//persistent
	int generate_key;
	int clone_wan_mac;
	int filter_id;
} persistent_vars;

typedef struct {
	FILE *fp;
	int threadid;
	int userid;
	int conn_fd;
	int post;
#ifdef HAVE_HTTPS
	int do_ssl;
#endif
#ifdef HAVE_OPENSSL
	SSL *ssl;
#endif
#ifdef HAVE_POLARSSL
	ssl_context ssl;
#endif
#ifdef HAVE_MATRIXSSL
	ssl_t *ssl;
#endif

	char *post_buf;
	char *request_url;
	char auth_realm[AUTH_MAX];
	char auth_userid[AUTH_MAX];
	char auth_passwd[AUTH_MAX];
	char *authorization;

//internal vars only
	FILE *s_fp;
	unsigned char *s_filebuffer;
	int s_filecount;
	int s_filelen;
	char label[64];
	int upgrade_ret;
	int restore_ret;
	int gozila_action;
	char http_client_ip[sizeof("000.000.000.000\0") + 1];
	char http_client_mac[sizeof("00:00:00:00:00:00\0") + 1];
	int browser_method;
#ifdef HAVE_REGISTER
	int isregistered;
	int isregistered_real;
#endif
#ifdef HAVE_SUPERCHANNEL
	int issuperchannel;
#endif
	persistent_vars *p;
/* CGI hash table */
	struct hsearch_data htab;
} webs;

#ifdef HAVE_HTTPS
#define DO_SSL(wp) wp->do_ssl
#else
#define DO_SSL(wp) 0
#endif

typedef webs *webs_t;

extern int wfsendfile(int fd, off_t offset, size_t nbytes, webs_t wp);
extern char *wfgets(char *buf, int len, webs_t fp);
extern int wfprintf(webs_t fp, char *fmt, ...);
extern size_t wfwrite(char *buf, int size, int n, webs_t fp);
extern size_t wfread(char *buf, int size, int n, webs_t fp);
extern int wfclose(webs_t fp);
extern int wfflush(webs_t fp);
#ifndef VALIDSOURCE
#ifndef VISUALSOURCE

extern int wfputs(char *buf, webs_t fp);
#endif
#endif
/* Basic authorization userid and passwd limit */

extern void send_authenticate(webs_t conn_fp);

/* Generic MIME type handler */
struct mime_handler {
	char *pattern;
	char *mime_type;
	char *extra_header;
	void (*input) (char *path, webs_t stream, int len, char *boundary);
	void (*output) (unsigned char method, struct mime_handler * handler, char *path, webs_t stream);
	int (*auth) (webs_t wp, int (*auth_check) (webs_t conn_fp));
	unsigned char send_headers;
	unsigned char handle_options;
};

#define METHOD_INVALID 0
#define METHOD_GET 1
#define METHOD_POST 2
#define METHOD_OPTIONS 3

typedef struct {
	const char *path;	/* Web page URL path */
	unsigned int size;	/* Size of web page in bytes */
} websRomPageIndexType;

//extern void setLength(long len);

extern struct mime_handler mime_handlers[];

/* Regular file handler */
extern void do_file(unsigned char method, struct mime_handler *handler, char *path, webs_t stream);
extern void do_file_attach(struct mime_handler *handler, char *path, webs_t stream, char *attachment);
extern void send_headers(webs_t conn_fp, int status, char *title, char *extra_header, char *mime_type, int length, char *attach_file, int nocache);

/* GoAhead 2.1 compatibility */
//typedef FILE * webs_t;
typedef char char_t;
#define T(s) (s)
#define __TMPVAR(x) tmpvar ## x
#define _TMPVAR(x) __TMPVAR(x)
#define TMPVAR _TMPVAR(__LINE__)

#define websDebugWrite(wp, fmt, args...)
#define websError(wp, code, msg, args...)

#define websHeader(wp) wfputs("<html lang=\"en\">", wp)
#define websFooter(wp) wfputs("</html>", wp)
#define websDone(wp, code) wfflush(wp)

#ifndef VALIDSOURCE
#ifndef VISUALSOURCE
//char *websGetVar(webs_t wp, char *var, char *d);
//int websGetVari(webs_t wp, char *var, int d);
#endif
#endif

struct Webenvironment {
	void (*Pdo_ej_buffer) (char *buffer, webs_t stream);
	int (*Phttpd_filter_name) (char *old_name, char *new_name, size_t size, int type);
	char *(*PwebsGetVar) (webs_t wp, char *var, char *d);
	int (*PwebsGetVari) (webs_t wp, char *var, int d);
	int (*PwebsWrite) (webs_t wp, char *fmt, ...);
	struct wl_client_mac *Pwl_client_macs;
	void (*Pdo_ej) (unsigned char method, struct mime_handler * handler, char *path, webs_t stream);	// jimmy, https, 8/4/2003
	FILE *(*PgetWebsFile) (char *path);
	int (*Pwfputs) (char *buf, webs_t fp);
	char *(*PGOZILA_GET) (webs_t wp, char *name);
	char *(*Plive_translate) (const char *tran);
	void (*Pvalidate_cgi) (webs_t fp);
	const websRomPageIndexType *PwebsRomPageIndex;
};

#define websSetVar(wp, var, value) set_cgi(wp, var, value)
#define websDefaultHandler(wp, urlPrefix, webDir, arg, url, path) ({ do_ej(METHOD_GET, path, wp,""); fflush(wp); 1; })
#define websWriteData(wp, buf, nChars) ({ int TMPVAR = wfwrite(buf, 1, nChars, wp); wfflush(wp); TMPVAR; })
#define websWriteDataNonBlock websWriteData
#define a_assert(a)

/* GoAhead 2.1 Embedded JavaScript compatibility */
extern int getWebsFileLen(char *path);

#ifndef VISUALSOURCE
#ifndef VALIDSOURCE
extern FILE *getWebsFile(char *path);
extern int ejArgs(int argc, char_t ** argv, char_t * fmt, ...);
static void do_ej(unsigned char method, struct mime_handler *handler, char *path, webs_t stream);
static void do_ej_buffer(char *buffer, webs_t stream);
extern int websWrite(webs_t wp, char *fmt, ...);
#endif
#endif
static int do_auth(webs_t wp, int (*auth_check) (webs_t conn_fp));
void Initnvramtab(void);
static void *call_ej(char *name, void *handle, webs_t wp, int argc, char_t ** argv);

#endif				/* _httpd_h_ */
