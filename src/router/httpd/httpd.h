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
#ifdef HAVE_MUSL
#include <stdatomic.h>
typedef atomic_int_fast8_t dd_atomic8_t;
#else
typedef char dd_atomic8_t;
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
#include <utils.h>
#if defined(linux)
/* Use SVID search */
#define __USE_GNU
#include <search.h>
#endif
#include <stdarg.h>
#ifdef HAVE_WIVIZ
#ifndef HAVE_MICRO
#include <pthread.h>
#endif
#endif
#define AUTH_MAX 64

#define USE_LAN 0
#define USE_WAN 1

typedef struct {
	//persistent
	volatile int generate_key;
	volatile int clone_wan_mac;
	volatile int filter_id;
	volatile int day_all, week0, week1, week2, week3, week4, week5, week6;
	volatile int start_week, end_week;
	volatile int time_all, start_hour, start_min, start_time, end_hour, end_min, end_time;
	volatile int tod_data_null;
	volatile int nv_count;
	volatile struct wl_client_mac wl_client_macs[MAX_LEASES];
#if !defined(HAVE_MICRO) && !defined(__UCLIBC__)
	pthread_mutex_t mutex_contr;
#ifdef HAVE_WIVIZ
	pthread_mutex_t wiz_mutex_contr;
#endif
#endif
} persistent_vars;

typedef struct {
	FILE *fp_in;
	FILE *fp_out;
	int userid;
	int conn_fd;
	int conn_fd_out;
	int post;
	int ssl_enabled;
#ifdef HAVE_OPENSSL
	SSL *ssl;
#endif
#ifdef HAVE_POLARSSL
	ssl_context ssl;
#endif
#ifdef HAVE_MATRIXSSL
	ssl_t *ssl;
#endif
	char *path;
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
	char http_client_ip[46];
	char http_client_mac[sizeof("00:00:00:00:00:00\0") + 1];
	int browser_method;
#ifdef HAVE_REGISTER
	dd_atomic8_t isregistered;
	dd_atomic8_t isregistered_real;
#endif
#ifdef HAVE_SUPERCHANNEL
	dd_atomic8_t issuperchannel;
#endif
	persistent_vars *p;
	/* CGI hash table */
	struct hsearch_data htab;
} webs;

#ifdef HAVE_HTTPS
#define DO_SSL(wp) wp->ssl_enabled
#define SSL_ENABLED() 1
#else
#define DO_SSL(wp) 0
#define SSL_ENABLED() 0
#endif

typedef webs *webs_t;

/* Generic MIME type handler */
struct mime_handler {
	char *pattern;
	char *mime_type;
	char *extra_header;
	int (*input)(char *path, webs_t stream, size_t len, char *boundary);
	int (*output)(unsigned char method, struct mime_handler *handler, char *path, webs_t stream);
	int (*auth)(webs_t wp, int (*auth_check)(webs_t conn_fp));
	unsigned char send_headers;
	unsigned char handle_options;
};

#define METHOD_INVALID 0
#define METHOD_GET 1
#define METHOD_POST 2
#define METHOD_OPTIONS 3
#define METHOD_HEAD 4

typedef struct {
	const char *path; /* Web page URL path */
	unsigned int size; /* Size of web page in bytes */
} websRomPageIndexType;

//static void setLength(long len);

//static struct mime_handler mime_handlers[];

/* Regular file handler */

/* GoAhead 2.1 compatibility */
//typedef FILE * webs_t;
typedef char char_t;
#define T(s) (s)
#define __TMPVAR(x) tmpvar##x
#define _TMPVAR(x) __TMPVAR(x)
#define TMPVAR _TMPVAR(__LINE__)

#define websDebugWrite(wp, fmt, args...)
#define websError(wp, code, msg, args...)

#define websHeader(wp) wfputs("<html lang=\"en\">", wp)
#define websFooter(wp) wfputs("</html>", wp)
#define websDone(wp, code) wfflush(wp)

#define websSetVar(wp, var, value) set_cgi(wp, var, value)
#define websDefaultHandler(wp, urlPrefix, webDir, arg, url, path) \
	({                                                        \
		do_ej(METHOD_GET, path, wp, "");                  \
		fflush(wp);                                       \
		1;                                                \
	})
#define websWriteData(wp, buf, nChars)                    \
	({                                                \
		int TMPVAR = wfwrite(buf, 1, nChars, wp); \
		wfflush(wp);                              \
		TMPVAR;                                   \
	})
#define websWriteDataNonBlock websWriteData
#define a_assert(a)

#define EJALIAS(old, new)                                      \
	EJ_VISIBLE void new (webs_t wp, int argc, char **argv) \
	{                                                      \
		old(wp, argc, argv);                           \
	}

void show_caption_legend(webs_t wp, const char *caption);
void show_caption_simple(webs_t wp, const char *caption);
void show_ip_cidr(webs_t wp, char *prefix, char *var, int nm, char *type, char *nmname, char *nmtype);
void show_caption_pp(webs_t wp, const char *class, const char *caption, const char *pre, const char *post);
void show_caption(webs_t wp, const char *class, const char *caption, const char *ext);
void show_caption_simple(webs_t wp, const char *caption);
void show_ip(webs_t wp, char *prefix, char *var, int nm, int allow_invalid, char *type);
void showRadioNoDef(webs_t wp, char *propname, char *nvname, int val);
#ifdef HAVE_MADWIFI
void showAutoOption(webs_t wp, char *propname, char *nvname, int nodisable);
#endif
void showOptions(webs_t wp, char *propname, char *names, char *select);
void showOptions_trans(webs_t wp, char *propname, char *names, char **trans, char *select);
void showOptions_ext_trans(webs_t wp, char *propname, char *names, char **trans, char *select, int disabled);
void showOptionsNames(webs_t wp, char *label, char *propname, char *valuenames, char **names, char *select);
void showIfOptions_ext(webs_t wp, char *propname, char *names, char *select, int disabled);
void showIfOptions(webs_t wp, char *propname, char *names, char *select);
void showOptionsChoose(webs_t wp, char *propname, char *names, char **trans, char *select);
void showOptionsLabel(webs_t wp, char *labelname, char *propname, char *names, char *select);
void show_inputlabel(webs_t wp, char *labelname, char *propertyname, int propertysize, char *inputclassname, int inputmaxlength);
void show_custominputlabel(webs_t wp, char *labelname, char *propertyname, char *property, int propertysize);
void show_bgscan_options(webs_t wp, char *prefix);
#ifdef HAVE_ATH9K
#define ATH9K_ENABLED() 1
#else
#define ATH9K_ENABLED() 0
#endif

#endif /* _httpd_h_ */
