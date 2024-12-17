/*
 * Shell-like utility functions
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: shutils.h,v 1.4 2005/11/21 15:03:16 seg Exp $
 */

#ifndef _shutils_h_
#define _shutils_h_
#include <string.h>
#include <stdio.h>

#define MAX_NVPARSE 256

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

#define DEBUG_CONSOLE 0
#define DEBUG_HTTPD 1
#define DEBUG_SERVICE 2

extern void dd_debug(int target, const char *fmt, ...);

#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || (defined(HAVE_RB600) && !defined(HAVE_WDR4900)) //special treatment
extern int debug_ready(void);
#else
#define debug_ready() (1)
#endif

/*
 * Reads file and returns contents
 * @param       path    path to file
 * @return      contents of file or NULL if an error occurred
 */
extern char *file2str(const char *path);

/*
 * Waits for a file descriptor to become available for reading or unblocked signal
 * @param       fd      file descriptor
 * @param       timeout seconds to wait before timing out or 0 for no timeout
 * @return      1 if descriptor changed status or 0 if timed out or -1 on error
 */
extern int waitfor(int fd, int timeout);

/*
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param       argv    argument list
 * @param       path    NULL, ">output", or ">>output"
 * @param       timeout seconds to wait before timing out or 0 for no timeout
 * @param       ppid    NULL to wait for child termination or pointer to pid
 * @return      return value of executed command or errno
 */

int _evalpid(char *const argv[], char *path, int timeout, int *ppid);
int _log_evalpid(char *const argv[], char *path, int timeout, int *ppid);

//extern int _eval(char *const argv[]);
extern int eval_va(const char *cmd, ...);
extern int log_eval_va(const char *cmd, ...);
extern int eval_va_space(const char *cmd, ...);
extern int eval_va_silence(const char *cmd, ...);
extern int eval_va_silence_space(const char *cmd, ...);

#define eval(cmd, args...) eval_va(cmd, ##args, NULL)
#define log_eval(cmd, args...) log_eval_va(cmd, ##args, NULL)
#define eval_space(cmd, args...) eval_va_space(cmd, ##args, NULL)
#define eval_silence(cmd, args...) eval_va_silence(cmd, ##args, NULL)

int check_pid(int pid, char *name);
int check_pidfromfile(char *pidfile, char *name);

/*
 * Kills process whose PID is stored in plaintext in pidfile
 * @param       pidfile PID file
 * @return      0 on success and errno on failure
 */
extern int kill_pidfile(char *pidfile);

/*
 * fread() with automatic retry on syscall interrupt
 * @param       ptr     location to store to
 * @param       size    size of each element of data
 * @param       nmemb   number of elements
 * @param       stream  file stream
 * @return      number of items successfully read
 */
extern int safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

/*
 * fwrite() with automatic retry on syscall interrupt
 * @param       ptr     location to read from
 * @param       size    size of each element of data
 * @param       nmemb   number of elements
 * @param       stream  file stream
 * @return      number of items successfully written
 */
extern int safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/*
 * Convert Ethernet address string representation to binary data
 * @param       a       string in xx:xx:xx:xx:xx:xx notation
 * @param       e       binary data
 * @return      TRUE if conversion was successful and FALSE otherwise
 */
extern int ether_atoe(const char *a, char *e);

int indexof(char *str, char c);

/*
 * Convert Ethernet address binary data to string representation
 * @param       e       binary data
 * @param       a       string in xx:xx:xx:xx:xx:xx notation
 * @return      a
 */
extern char *ether_etoa(const char *e, char *a);

extern int nvifname_to_osifname(const char *nvifname, char *osifname_buf, int osifname_buf_len);
extern int osifname_to_nvifname(const char *osifname, char *nvifname_buf, int nvifname_buf_len);

struct nvram_param {
	char *name;
	char *value;
};

extern struct nvram_param *load_defaults(void);
extern void free_defaults(struct nvram_param *);

extern char *strattach(char *src, char *attach, char *delimiter);
extern char *strspcattach(char *src, char *attach);

extern int dd_system(const char *command);
extern int sysprintf(const char *fmt, ...);
extern int f_exists(const char *path); // note: anything but a directory

extern char *get_filter_services(void);

typedef struct filters // l7 and p2p filters
{
	char *name;
	unsigned short portfrom;
	unsigned short portto;
	unsigned char proto; // 1 = tcp, 2 = udp, 3 = both, 4 = l7, 5 = dpi
} filters;

extern void free_filters(filters *filter);

extern filters *get_filters_list(void);
extern int get_risk_by_name(char *name);
extern char *get_dep_by_name(char *name);

extern int get_ifname_unit(const char *ifname, int *unit, int *subunit);

extern int strhas(char *list, char *key);

/*
 * Concatenate two strings together into a caller supplied buffer
 * @param       s1      first string
 * @param       s2      second string
 * @param       buf     buffer large enough to hold both strings
 * @return      buf
 */
char *strcat_r(const char *s1, const char *s2, char *buf);
char *strlcat_r(const char *s1, const char *s2, char *buf, size_t len);
char *dd_strncat(char *dst, size_t len, const char *src);

#define strcat_r(s1, s2, buf) (sizeof(buf) == sizeof(void *) ? strcat_r(s1, s2, buf) : strlcat_r(s1, s2, buf, sizeof(buf)))
#define strcat(buf, s1) sizeof(buf) == sizeof(void *) ? strcat(buf, s1) : dd_strncat(buf, sizeof(buf), s1)

#ifndef FROM_NVRAM
extern int dd_sprintf(char *str, const char *fmt, ...);
extern int dd_snprintf(char *str, int len, const char *fmt, ...);
extern void *dd_malloc(size_t len);

#define malloc(len) dd_malloc(len)
#define strcpy(dst, src) (sizeof(dst) == sizeof(void *) ? strcpy(dst, src) : strncpy(dst, src, sizeof(dst) - 1))
#define sprintf(output, format, args...)                                         \
	(sizeof(output) == sizeof(void *) ? dd_sprintf(output, format, ##args) : \
					    dd_snprintf(output, sizeof(output), format, ##args))
#define snprintf(output, len, format, args...) dd_snprintf(output, len, format, ##args)
#define system(cmd) dd_system(cmd)
#endif

#ifdef MEMDEBUG

void *mymalloc(int size, char *func, int line);
void myfree(void *mem, char *func, int line);
void showmemdebugstat();

long getmemfree(void);
long getmemtotal(void);

#define safe_malloc(size) mymalloc(size, __func__, __LINE__)
#define safe_free(mem) myfree(mem, __func__, __LINE__)
#define free(mem) myfree(mem, __func__, __LINE__)

#define memdebug_enter()             \
	struct sysinfo memdebuginfo; \
	sysinfo(&memdebuginfo);      \
	long before = memdebuginfo.freeram;

#define memdebug_leave()                                                                                                    \
	sysinfo(&memdebuginfo);                                                                                             \
	long after = memdebuginfo.freeram;                                                                                  \
	if ((before - after) > 0) {                                                                                         \
		fprintf(stderr, "function %s->%s:%d leaks %ld bytes memory (before: %ld, after: %ld\n", __FILE__, __func__, \
			__LINE__, (before - after), before, after);                                                         \
	}
#define memdebug_leave_info(a)                                                                                                   \
	sysinfo(&memdebuginfo);                                                                                                  \
	long after = memdebuginfo.freeram;                                                                                       \
	if ((before - after) > 0) {                                                                                              \
		fprintf(stderr, "function %s->%s:%d (%s) leaks %ld bytes memory (before: %ld, after: %ld\n", __FILE__, __func__, \
			__LINE__, a, (before - after), before, after);                                                           \
	}
#else
#define safe_malloc malloc
#define safe_free free
#define memdebug_enter()
#define memdebug_leave()
#define memdebug_leave_info(a)
#define showmemdebugstat()
#endif

/*
 * Check for a blank character; that is, a space or a tab 
 */
#define dd_isblank(c) ((c) == ' ' || (c) == '\t')

/*
 * Strip trailing CR/NL from string <s> 
 */
char *chomp(char *s);

/*
 * Simple version of _backtick() 
 */
#define backtick(cmd, args...)                        \
	({                                            \
		char *argv[] = { cmd, ##args, NULL }; \
		_backtick(argv);                      \
	})
/*
 * Return NUL instead of NULL if undefined 
 */
#define safe_getenv(s) (getenv(s) ?: "")

#define HAVE_SILENCE 1
/*
 * Print directly to the console 
 */
#ifndef HAVE_SILENCE

#define cprintf(fmt, args...)                 \
	do {                                  \
		fprintf(stderr, fmt, ##args); \
		fflush(stderr);               \
	} while (0)
#else
#define cprintf(fmt, args...)
#endif

char *foreach_first(char *foreachwordlist, char *word, char *delim, size_t len);

char *foreach_last(char *next, char *word, char *delim, size_t len);

char *getentrybyidx(char *buf, char *list, int idx);
char *getentrybyidx_d(char *buf, char *list, int idx, char *delimiters_short, char *delimiters);

#define GETENTRYBYIDX(name, list, idx) \
	char name##_priv[128];         \
	char *name = getentrybyidx(name##_priv, list, idx);

#define GETENTRYBYIDX_DEL(name, list, idx, delim) \
	char name##_priv[128];                    \
	char *name = getentrybyidx_d(name##_priv, list, idx, "><:,", delim);

/*
 * Copy each token in wordlist delimited by space into word 
 */

#define foreach_delim(word, foreachwordlist, next, delim)                               \
	for (next = foreach_first(foreachwordlist, word, delim, sizeof(word)); word[0]; \
	     next = foreach_last(next, word, delim, sizeof(word)))

#define foreach(word, foreachwordlist, next) foreach_delim(word, foreachwordlist, next, " ")

#define foreach_delimln(word, len, foreachwordlist, next, delim) \
	for (next = foreach_first(foreachwordlist, word, delim, len); word[0]; next = foreach_last(next, word, delim, len))

#define foreachln(word, len, foreachwordlist, next) foreach_delim(word, foreachwordlist, next, " ")

/*
 * Return NUL instead of NULL if undefined 
 */
#define safe_getenv(s) (getenv(s) ?: "")

/*
 * Debug print 
 */
#ifdef DEBUG
#define dprintf(fmt, args...) cprintf("%s: " fmt, __FUNCTION__, ##args)
#else
#define dprintf(fmt, args...)
#endif

#ifdef HAVE_MICRO
#define FORK(a) a;
#else
#define FORK(func)                      \
	{                               \
		switch (fork()) {       \
		case -1:                \
			break;          \
		case 0:                 \
			(void)setsid(); \
			func;           \
			exit(0);        \
			break;          \
		default:                \
			break;          \
		}                       \
	}
#endif

#ifdef HAVE_MICRO
#define FORKWAIT(a) a;
#else
#define FORKWAIT(func)                                    \
	{                                                 \
		int forkpid;                              \
		int forkstatus;                           \
		switch (forkpid = fork()) {               \
		case -1:                                  \
			break;                            \
		case 0:                                   \
			(void)setsid();                   \
			func;                             \
			exit(0);                          \
			break;                            \
		default:                                  \
			waitpid(forkpid, &forkstatus, 0); \
			break;                            \
		}                                         \
	}
#endif
#include <sys/time.h>
void add_blocklist(const char *service, char *ip);
int check_blocklist(const char *service, char *ip);
void add_blocklist_sock(const char *service, int socket);
int check_blocklist_sock(const char *service, int socket);
char *get_ipfromsock(int socket, char *ip);
#if defined(HAVE_SYSLOG) && !defined(HAVE_MICRO)
void airbag_setpostinfo(const char *string);
#else
#undef airbag_setpostinfo
#define airbag_setpostinfo(string) \
	do {                       \
	} while (0)
#endif
/*
#define debug_free(ptr) { \
	airbag_setpostinfo(__func__); \
	free(ptr); \
	}
*/
#define debug_free(ptr)    \
	{                  \
		free(ptr); \
	}

#endif /* _shutils_h_ */
