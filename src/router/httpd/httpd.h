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
# include <matrixSsl.h>
# include <matrixssl_xface.h>
#endif

#ifdef HAVE_OPENSSL
#include <ssl.h>
extern BIO *bio_err;
#endif

//   typedef  BIO * webs_t;
//   #define wfgets(buf,len,fp)     BIO_gets(fp,buf,len)
//   #define wfputc(c,fp)           BIO_printf(fp,"%c",c)
//   #define wfputs(buf,fp)         BIO_puts(fp,buf)
//   #define wfprintf(fp, args...)  BIO_printf(fp, ## args)
//   #define wfwrite(buf,size,n,fp) BIO_write(fp,buf,n * size)
//   #define wfread(buf,size,n,fp)  BIO_read(fp,buf,n * size)
//   #define wfflush(fp)            BIO_flush(fp)
//   #define wfclose(fp)            BIO_free_all(fp)
//#else
//   typedef FILE * webs_t;
//   #define wfgets(buf,len,fp)     fgets(buf,len,fp)
//   #define wfputc(c,fp)           fputc(c,fp)
//   #define wfputs(buf,fp)         fputs(buf,fp)
//   #define wfprintf(fp, args...)  fprintf(fp, ## args)
//   #define wfwrite(buf,size,n,fp) fwrite(buf,size,n,fp)
//   #define wfread(buf,size,n,fp)  fread(buf,size,n,fp)
//   #define wfflush(fp)            fflush(fp)
//   #define wfclose(fp)            fclose(fp)
//#endif

//#define FILTER_DEBUG 1

typedef FILE * webs_t;
extern char * wfgets(char *buf, int len, FILE *fp);
extern int wfputc(char c, FILE *fp);
extern int wfputs(char *buf, FILE *fp);
extern int wfprintf(FILE *fp, char *fmt,...);
extern size_t wfwrite(char *buf, int size, int n, FILE *fp);
extern size_t wfread(char *buf, int size, int n, FILE *fp);
extern int wfflush(FILE *fp);
extern int wfclose(FILE *fp);
#ifdef HAVE_HTTPS  
int do_ssl;
#endif
/* Basic authorization userid and passwd limit */
#define AUTH_MAX 64

extern char auth_realm[AUTH_MAX];
extern void send_authenticate (char *realm);

/* Generic MIME type handler */
struct mime_handler {
	char *pattern;
	char *mime_type;
	char *extra_header;
	void (*input)(char *path, FILE *stream, int len, char *boundary);
	void (*output)(char *path, FILE *stream);
	int (*auth)(char *userid, char *passwd, char *realm);
};
typedef struct
{
  char *path;			/* Web page URL path */
  unsigned char *page;		/* Web page data */
  int size;			/* Size of web page in bytes */
//  int csize;			/* Size of web page in bytes */
} websRomPageIndexType;


extern struct mime_handler mime_handlers[];

/* CGI helper functions */
extern void init_cgi(char *query);
extern char * get_cgi(char *name);
extern void set_cgi(char *name, char *value);
extern int count_cgi();

/* Regular file handler */
extern void do_file(char *path, webs_t stream);

/* GoAhead 2.1 compatibility */
//typedef FILE * webs_t;
typedef char char_t;
#define T(s) (s)
#define __TMPVAR(x) tmpvar ## x
#define _TMPVAR(x) __TMPVAR(x)
#define TMPVAR _TMPVAR(__LINE__)

//#define websWrite(wp, fmt, args...) ({ int TMPVAR = wfprintf(wp, fmt, ## args); wfflush(wp); TMPVAR; })
int websWrite(webs_t wp, char *fmt, ...);
#define websDebugWrite(wp, fmt, args...)
//#define websDebugWrite(wp, fmt, args...) ({ error_value = 1; wfputs("<!--", wp); int TMPVAR = wfprintf(wp, fmt, ## args); wfputs("-->", wp); wfflush(wp); TMPVAR; })
#define websError(wp, code, msg, args...)

//#define websError(wp, code, msg, args...) wfprintf(wp, msg, ## args)
#define websHeader(wp) wfputs("<html lang=\"en\">", wp)
#define websFooter(wp) wfputs("</html>", wp)
#define websDone(wp, code) wfflush(wp)


extern char *websGetVar(webs_t wp, char *var, char *d);

#define websSetVar(wp, var, value) set_cgi(var, value)
#define websDefaultHandler(wp, urlPrefix, webDir, arg, url, path, query) ({ do_ej(path, wp); fflush(wp); 1; })
#define websWriteData(wp, buf, nChars) ({ int TMPVAR = wfwrite(buf, 1, nChars, wp); wfflush(wp); TMPVAR; })
#define websWriteDataNonBlock websWriteData
#define a_assert(a)

extern int ejArgs(int argc, char_t **argv, char_t *fmt, ...);

/* GoAhead 2.1 Embedded JavaScript compatibility */
extern char *getWebsFile(char *path);
extern int getWebsFileLen(char *path);

extern void do_ej(char *path, webs_t stream);
extern void do_ej_buffer(char *buffer, webs_t stream);
int do_auth (char *userid, char *passwd, char *realm);
void Initnvramtab (void);



struct ej_handler {
	char *pattern;
	void (*output)(int eid, webs_t wp, int argc, char_t **argv);
};
extern struct ej_handler ej_handlers[];



#endif /* _httpd_h_ */
