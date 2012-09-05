#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE	1
#endif

#ifndef _ALL_SOURCE
#define _ALL_SOURCE             1
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS	64
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CONFIG2_H
#include "config2.h"
#endif

#include "os_dep.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <errno.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <sys/types.h>

#ifndef __USE_XOPEN
#define U_X
#define __USE_XOPEN
#endif
#ifndef _XOPEN_SOURCE
#define X_S
#define _XOPEN_SOURCE	5	/* The 5 is a kludge to get a strptime() prototype in NetBSD */
#endif
#ifdef TIME_WITH_SYS_TIME
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#else
#if defined(TM_IN_SYS_TIME) && defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#elif defined(HAVE_TIME_H)
#include <time.h>
#endif
#endif
#ifdef X_S
#undef _XOPEN_SOURCE
#endif
#ifdef U_X
#undef __USE_XOPEN
#endif


#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <signal.h>
/*#ifdef HAVE_SIGACTION_H
#include <sigaction.h>
#endif*/
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_CYGWIN_H
#include <sys/cygwin.h>
#endif
#ifdef HAVE_INTERIX_INTERIX_H
#include <interix/interix.h>
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_GRP_H
#include <grp.h>
#endif
/*
#ifdef HAVE_WAIT_H
#include <wait.h>
#endif
*/

#ifdef HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#else
#ifdef HAVE_NETINET_IN_SYSTEM_H
#include <netinet/in_system.h>
#endif
#endif
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <utime.h>

#include <termios.h>

#ifdef HAVE_LONG_LONG
#define longlong long long
#else
#define longlong double
#endif

#if defined(__INTERIX) && defined(HAVE_STRTOQ)
extern quad_t
#if defined(__cdecl) || defined(_MSC_VER)
__cdecl
#endif
strtoq(const char *, char **, int);
#endif

#if defined(__hpux) && defined(__LP64__)
#undef HAVE_SOCKLEN_T
#endif

#ifndef HAVE_SOCKLEN_T
#define socklen_t int
#endif

#ifndef PF_INET
#define PF_INET AF_INET
#endif
#ifndef PF_UNIX
#define PF_UNIX AF_UNIX
#endif

#ifdef HAVE_SSL
#include <openssl/ssl.h>
#include <openssl/rand.h>
#endif

#include "os_depx.h"

#include "setup.h"

#define LINKS_COPYRIGHT "(C) 1999 - 2011 Mikulas Patocka"
#define LINKS_COPYRIGHT_8859_1 "(C) 1999 - 2011 Mikul\341s Patocka"
#define LINKS_COPYRIGHT_8859_2 "(C) 1999 - 2011 Mikul\341\271 Pato\350ka"

#ifdef HAVE_POINTER_COMPARISON_BUG
#define DUMMY ((void *)1L)
#else
#define DUMMY ((void *)-1L)
#endif

#define RET_OK		0
#define RET_ERROR	1
#define RET_SIGNAL	2
#define RET_SYNTAX	3
#define RET_FATAL	4

#ifndef HAVE_MEMMOVE
void *memmove(void *, const void *, size_t);
#endif
#ifndef HAVE_RAISE
int raise(int);
#endif
#ifndef HAVE_STRTOUL
unsigned long strtoul(const char *, char **, int);
#endif
#ifndef HAVE_STRERROR
char *strerror(int);
#endif
#ifndef HAVE_GETTIMEOFDAY
struct timeval {
	long tv_sec;
	long tv_usec;
};
struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};
int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif
#ifndef HAVE_STRLEN
size_t strlen(const char *s);
#endif
#ifndef HAVE_STRCPY
char *strcpy(char *dst, const char *src);
#endif
#ifndef HAVE_STRCHR
char *strchr(const char *s, int c);
#endif
#ifndef HAVE_STRRCHR
char *strrchr(const char *s, int c);
#endif
#ifndef HAVE_STRCMP
int strcmp(const char *s1, const char *s2);
#endif
#ifndef HAVE_STRNCMP
int strncmp(const char *s1, const char *s2, size_t n);
#endif
#ifndef HAVE_STRCSPN
size_t strcspn(const char *s, const char *reject);
#endif
#ifndef HAVE_STRSTR
char *strstr(const char *haystack, const char *needle);
#endif
#ifndef HAVE_TEMPNAM
char *tempnam(const char *dir, const char *pfx);
#endif

#define option option_dirty_workaround_for_name_clash_with_include_on_cygwin
#define table table_dirty_workaround_for_name_clash_with_libraries_on_macos
#define scroll scroll_dirty_workaround_for_name_clash_with_libraries_on_macos

/* error.c */

void do_not_optimize_here(void *p);
void check_memory_leaks();
void error(unsigned char *, ...);
void debug_msg(unsigned char *, ...);
void int_error(unsigned char *, ...);
extern int errline;
extern unsigned char *errfile;

#define internal while (1) errfile = __FILE__, errline = __LINE__, int_error
#define debug errfile = __FILE__, errline = __LINE__, debug_msg

#ifdef SPECIAL_MALLOC

void *sp_malloc(size_t);
void sp_free(void *);
void *sp_realloc(void *, size_t);

#define xmalloc sp_malloc
#define xfree sp_free
#define xrealloc sp_realloc

#else

#define xmalloc malloc
#define xfree free
#define xrealloc realloc

#endif

/* inline */

void fatal_tty_exit(void);

#ifdef LEAK_DEBUG

extern long mem_amount;
extern long last_mem_amount;

#ifndef LEAK_DEBUG_LIST
struct alloc_header {
	size_t size;
};
#else
struct alloc_header {
	struct alloc_header *next;
	struct alloc_header *prev;
	size_t size;
	int line;
	unsigned char *file;
	unsigned char *comment;
};
#endif

#define L_D_S ((sizeof(struct alloc_header) + 15) & ~15)

#endif

#define overalloc()							\
do {									\
	error("ERROR: attempting to allocate too large block at %s:%d", __FILE__, __LINE__);\
	fatal_tty_exit();						\
	exit(RET_FATAL);						\
} while (1)	/* while (1) is not a type --- it's here to allow the
	compiler that doesn't know that exit doesn't return to do better
	optimizations */

#ifdef LEAK_DEBUG

void *debug_mem_alloc(unsigned char *, int, size_t);
void debug_mem_free(unsigned char *, int, void *);
void *debug_mem_realloc(unsigned char *, int, void *, size_t);
void set_mem_comment(void *, unsigned char *, int);

#define mem_alloc(x) debug_mem_alloc(__FILE__, __LINE__, x)
#define mem_free(x) debug_mem_free(__FILE__, __LINE__, x)
#define mem_realloc(x, y) debug_mem_realloc(__FILE__, __LINE__, x, y)

#else

static inline void *mem_alloc(size_t size)
{
	void *p;
	if (!size) return DUMMY;
	if (size > MAXINT) overalloc();
	if (!(p = malloc(size))) {
		error("ERROR: out of memory (malloc returned NULL)");
		fatal_tty_exit();
		exit(RET_FATAL);
		return NULL;
	}
	return p;
}

static inline void mem_free(void *p)
{
	if (p == DUMMY) return;
	if (!p) {
		internal("mem_free(NULL)");
		return;
	}
	free(p);
}

static inline void *mem_realloc(void *p, size_t size)
{
	if (p == DUMMY) return mem_alloc(size);
	if (!p) {
		internal("mem_realloc(NULL, %d)", size);
		return NULL;
	}
	if (!size) {
		mem_free(p);
		return DUMMY;
	}
	if (size > MAXINT) overalloc();
	if (!(p = realloc(p, size))) {
		error("ERROR: out of memory (realloc returned NULL)");
		fatal_tty_exit();
		exit(RET_FATAL);
		return NULL;
	}
	return p;
}

static inline void *debug_mem_alloc(unsigned char *f, int l, size_t s) { return mem_alloc(s); }
static inline void debug_mem_free(unsigned char *f, int l, void *p) { mem_free(p); }
static inline void *debug_mem_realloc(unsigned char *f, int l, void *p, size_t s) { return mem_realloc(p, s); }
static inline void set_mem_comment(void *p, unsigned char *c, int l) {}

#endif

static inline unsigned char upcase(unsigned char a)
{
	if (a >= 'a' && a <= 'z') a -= 0x20;
	return a;
}

static inline int xstrcmp(unsigned char *s1, unsigned char *s2)
{
        if (!s1 && !s2) return 0;
        if (!s1) return -1;
        if (!s2) return 1;
        return strcmp(s1, s2);
}

static inline int cmpbeg(unsigned char *str, unsigned char *b)
{
	while (*str && upcase(*str) == upcase(*b)) str++, b++;
	return !!*b;
}

#if !(defined(LEAK_DEBUG) && defined(LEAK_DEBUG_LIST))

static inline unsigned char *memacpy(const unsigned char *src, size_t len)
{
	unsigned char *m;
	m = mem_alloc(len + 1);
	memcpy(m, src, len);
	m[len] = 0;
	return m;
}

static inline unsigned char *stracpy(const unsigned char *src)
{
	return src ? memacpy(src, src != DUMMY ? strlen(src) : 0) : NULL;
}

#else

static inline unsigned char *debug_memacpy(unsigned char *f, int l, unsigned char *src, size_t len)
{
	unsigned char *m;
	m = debug_mem_alloc(f, l, len + 1);
	memcpy(m, src, len);
	m[len] = 0;
	return m;
}

#define memacpy(s, l) debug_memacpy(__FILE__, __LINE__, s, l)

static inline unsigned char *debug_stracpy(unsigned char *f, int l, unsigned char *src)
{
	return src ? debug_memacpy(f, l, src, src != DUMMY ? strlen(src) : 0) : NULL;
}

#define stracpy(s) debug_stracpy(__FILE__, __LINE__, s)

#endif

static inline int snprint(unsigned char *s, int n, off_t num)
{
	off_t q = 1;
	while (q <= num / 10) q *= 10;
	while (n-- > 1 && q) *(s++) = num / q + '0', num %= q, q /= 10;
	*s = 0;
	return !!q;
}

static inline int snzprint(unsigned char *s, int n, off_t num)
{
	if (n > 1 && num < 0) *(s++) = '-', num = -num, n--;
	return snprint(s, n, num);
}

static inline void add_to_strn(unsigned char **s, unsigned char *a)
{
	unsigned char *p;
	size_t l1 = strlen(*s), l2 = strlen(a);
	if (((l1 | l2) | (l1 + l2 + 1)) > MAXINT) overalloc();
	p = mem_realloc(*s, l1 + l2 + 1);
	strcat(p, a);
	*s = p;
}

#define ALLOC_GR	0x040		/* must be power of 2 */

#define init_str() init_str_x(__FILE__, __LINE__)

static inline unsigned char *init_str_x(unsigned char *file, int line)
{
	unsigned char *s = debug_mem_alloc(file, line, ALLOC_GR);
	*s = 0;
	return s;
}

static inline void add_to_str(unsigned char **s, int *l, unsigned char *a)
{
	unsigned char *p;
	size_t ll = strlen(a);
	if (ll > MAXINT || *l + ll > MAXINT) overalloc();
	p = *s;
	if (((size_t)*l & ~(ALLOC_GR - 1)) != ((*l + ll) & ~(ALLOC_GR - 1))) {
		p = *s = mem_realloc(*s, (*l + ll + ALLOC_GR) & ~(ALLOC_GR - 1));
	}
	strcpy(p + *l, a); *l += ll;
}

static inline void add_bytes_to_str(unsigned char **s, int *l, unsigned char *a, int ll)
{
	unsigned char *p;
	if (*l + ll < 0) overalloc();
	p = *s;
	if ((*l & ~(ALLOC_GR - 1)) != ((*l + ll) & ~(ALLOC_GR - 1))) {
		p = *s = mem_realloc(*s, (*l + ll + ALLOC_GR) & ~(ALLOC_GR - 1));
	}
	memcpy(p + *l, a, ll); p[*l += ll] = 0;
}

static inline void add_chr_to_str(unsigned char **s, int *l, unsigned char a)
{
	unsigned char *p;
	if (*l + 1 < 0) overalloc();
	p = *s;
	if ((*l & (ALLOC_GR - 1)) == ALLOC_GR - 1) {
		p = *s = mem_realloc(*s, (*l + 1 + ALLOC_GR) & ~(ALLOC_GR - 1));
	}
	*(p + *l) = a; *(p + ++(*l)) = 0;
}

static inline void add_num_to_str(unsigned char **s, int *l, off_t n)
{
	unsigned char a[64];
	/*sprintf(a, "%d", n);*/
	snzprint(a, 64, n);
	add_to_str(s, l, a);
}

static inline void add_knum_to_str(unsigned char **s, int *l, int n)
{
	unsigned char a[13];
	if (n && n / (1024 * 1024) * (1024 * 1024) == n) snzprint(a, 12, n / (1024 * 1024)), strcat(a, "M");
	else if (n && n / 1024 * 1024 == n) snzprint(a, 12, n / 1024), strcat(a, "k");
	else snzprint(a, 13, n);
	add_to_str(s, l, a);
}

static inline long strtolx(unsigned char *c, unsigned char **end)
{
	long l;
	if (c[0] == '0' && upcase(c[1]) == 'X' && c[2]) l = strtol((char *)c + 2, (char **)end, 16);
	else l = strtol((char *)c, (char **)end, 10);
	if (!*end) return l;
	if (upcase(**end) == 'K') {
		(*end)++;
		if (l < -MAXINT / 1024) return -MAXINT;
		if (l > MAXINT / 1024) return MAXINT;
		return l * 1024;
	}
	if (upcase(**end) == 'M') {
		(*end)++;
		if (l < -MAXINT / (1024 * 1024)) return -MAXINT;
		if (l > MAXINT / (1024 * 1024)) return MAXINT;
		return l * (1024 * 1024);
	}
	return l;
}

/* Copies at most dst_size chars into dst. Ensures null termination of dst. */
static inline unsigned char *safe_strncpy(unsigned char *dst, const unsigned char *src, size_t dst_size)
{
	size_t to_copy;
	if (!dst_size) return dst;
	to_copy = strlen(src);
	if (to_copy >= dst_size) to_copy = dst_size - 1;
	memcpy(dst, src, to_copy);
	memset(dst + to_copy, 0, dst_size - to_copy);
	return dst;
}


struct list_head {
	void *next;
	void *prev;
};

struct xlist_head {
	struct xlist_head *next;
	struct xlist_head *prev;
};

#define init_list(x) {do_not_optimize_here(&x); (x).next=&(x); (x).prev=&(x); do_not_optimize_here(&x);}
#define list_empty(x) ((x).next == &(x))
#define del_from_list(x) {do_not_optimize_here(x); ((struct list_head *)(x)->next)->prev=(x)->prev; ((struct list_head *)(x)->prev)->next=(x)->next; do_not_optimize_here(x);}
/*#define add_to_list(l,x) {(x)->next=(l).next; (x)->prev=(typeof(x))&(l); (l).next=(x); if ((l).prev==&(l)) (l).prev=(x);}*/

#if defined(HAVE_TYPEOF) && !(defined(__GNUC__) && __GNUC__ >= 4)
/* GCC 4 emits warnings about this and I haven't found a way to stop it */
#define add_at_pos(p,x) do {do_not_optimize_here(p); (x)->next=(p)->next; (x)->prev=(p); (p)->next=(x); (x)->next->prev=(x);do_not_optimize_here(p);} while(0)
#define add_to_list(l,x) add_at_pos((typeof(x))(void *)&(l),(x))
#define foreach(e,l) for ((e)=(l).next; (e)!=(typeof(e))(void *)&(l); (e)=(e)->next)
#define foreachback(e,l) for ((e)=(l).prev; (e)!=(typeof(e))(void *)&(l); (e)=(e)->prev)
#else
#define add_at_pos(p,x) do {do_not_optimize_here(p); (x)->next=(p)->next; (x)->prev=(void *)(p); (p)->next=(x); (x)->next->prev=(x); do_not_optimize_here(p); } while(0)
#define add_to_list(l,x) add_at_pos(&(l),(x))
#define foreach(e,l) for ((e)=(l).next; (e)!=(void *)&(l); (e)=(e)->next)
#define foreachback(e,l) for ((e)=(l).prev; (e)!=(void *)&(l); (e)=(e)->prev)
#endif
#define free_list(l) {struct xlist_head *a__;do_not_optimize_here(&l);foreach(a__,l) do_not_optimize_here(a__);foreachback(a__,l) do_not_optimize_here(a__); while ((l).next != &(l)) {a__=(l).next; del_from_list(a__); mem_free(a__); } do_not_optimize_here(&l);}

#define WHITECHAR(x) ((x) == 9 || (x) == 10 || (x) == 12 || (x) == 13 || (x) == ' ')
#define U(x) ((x) == '"' || (x) == '\'')

static inline int isA(unsigned char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
	        c == '_' || c == '-';
}

static inline int casecmp(unsigned char *c1, unsigned char *c2, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++) if (upcase(c1[i]) != upcase(c2[i])) return 1;
	return 0;
}

static inline int can_write(int fd)
{
	fd_set fds;
	struct timeval tv = {0, 0};
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	return select(fd + 1, NULL, &fds, NULL, &tv);
}

static inline int can_read(int fd)
{
	fd_set fds;
	struct timeval tv = {0, 0};
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	return select(fd + 1, &fds, NULL, NULL, &tv);
}

#define CI_BYTES	1
#define CI_FILES	2
#define CI_LOCKED	3
#define CI_LOADING	4
#define CI_TIMERS	5
#define CI_TRANSFER	6
#define CI_CONNECTING	7
#define CI_KEEP		8
#define CI_LIST		9

/* os_dep.c */

struct terminal;

struct open_in_new {
	unsigned char *text;
	unsigned char *hk;
	void (*fn)(struct terminal *term, unsigned char *, unsigned char *);
};

void close_fork_tty();
int get_system_env();
int is_xterm();
int can_twterm();
int get_terminal_size(int, int *, int *);
void handle_terminal_resize(int, void (*)());
void unhandle_terminal_resize(int);
void set_bin(int);
int c_pipe(int *);
int get_input_handle();
int get_output_handle();
int get_ctl_handle();
void want_draw();
void done_draw();
void terminate_osdep();
void *handle_mouse(int, void (*)(void *, unsigned char *, int), void *);
void unhandle_mouse(void *);
int check_file_name(unsigned char *);
int start_thread(void (*)(void *, int), void *, int);
char *get_clipboard_text();
void set_clipboard_text(char *);
void set_window_title(unsigned char *);
unsigned char *get_window_title();
int is_safe_in_shell(unsigned char);
unsigned char *escape_path(unsigned char *);
void check_shell_security(unsigned char **);
int check_shell_url(unsigned char *);
void block_stdin();
void unblock_stdin();
void init_os(void);
void init_os_terminal(void);
void get_path_to_exe(void);
unsigned char *os_conv_to_external_path(unsigned char *, unsigned char *);
unsigned char *os_fixup_external_program(unsigned char *);
int exe(char *);
int resize_window(int, int);
int can_resize_window(int);
int can_open_os_shell(int);
struct open_in_new *get_open_in_new(int);
void set_highpri();
#ifdef HAVE_OPEN_PREALLOC
int can_prealloc(char *);
int open_prealloc(char *, int, int, off_t);
void prealloc_truncate(int, off_t);
#else
static inline void prealloc_truncate(int x, off_t y) {}
#endif
void os_cfmakeraw(struct termios *t);

/* select.c */

#ifndef FD_SETSIZE
#define FD_SETSIZE 1024
#endif

typedef long ttime;
typedef unsigned long uttime;
typedef unsigned tcount;

extern int terminate;

long select_info(int);
void select_loop(void (*)());
int register_bottom_half(void (*)(void *), void *);
void check_bottom_halves();
int install_timer(ttime, void (*)(void *), void *);
void kill_timer(int);
ttime get_time();

#define H_READ	0
#define H_WRITE	1
#define H_ERROR	2
#define H_DATA	3

void *get_handler(int, int);
void set_handlers(int, void (*)(void *), void (*)(void *), void (*)(void *), void *);
void install_signal_handler(int, void (*)(void *), void *, int);
void interruptible_signal(int sig, int in);
void set_sigcld();

/* dns.c */

typedef unsigned ip;

int do_real_lookup(unsigned char *, ip *);
void shrink_dns_cache(int);
int find_host(unsigned char *, ip *, void **, void (*)(void *, int), void *);
int find_host_no_cache(unsigned char *, ip *, void **, void (*)(void *, int), void *);
void kill_dns_request(void **);

/* cache.c */

struct cache_entry {
	struct cache_entry *next;
	struct cache_entry *prev;
	unsigned char *url;
	unsigned char *head;
	unsigned char *redirect;
	int redirect_get;
	off_t length;
	int incomplete;
	int tgc;
	unsigned char *last_modified;
	off_t data_size;
	struct list_head frag;
	tcount count;
	int refcount;
#ifdef HAVE_SSL
	unsigned char *ssl_info;
#endif
};

struct fragment {
	struct fragment *next;
	struct fragment *prev;
	off_t offset;
	off_t length;
	off_t real_length;
	unsigned char data[1];
};

extern int page_size;

void init_cache();
long cache_info(int);
int find_in_cache(unsigned char *, struct cache_entry **);
int get_cache_entry(unsigned char *, struct cache_entry **);
int add_fragment(struct cache_entry *, off_t, unsigned char *, off_t);
void defrag_entry(struct cache_entry *);
void truncate_entry(struct cache_entry *, off_t, int);
void free_entry_to(struct cache_entry *, off_t);
void delete_entry_content(struct cache_entry *);
void delete_cache_entry(struct cache_entry *);
void garbage_collection(int);

/* sched.c */

#define PRI_MAIN	0
#define PRI_DOWNLOAD	0
#define PRI_FRAME	1
#define PRI_NEED_IMG	2
#define PRI_IMG		3
#define PRI_PRELOAD	4
#define PRI_CANCEL	5
#define N_PRI		6

struct remaining_info {
	int valid;
	off_t size, loaded, last_loaded, cur_loaded;
	off_t pos;
	ttime elapsed;
	ttime last_time;
	ttime dis_b;
	int data_in_secs[CURRENT_SPD_SEC];
	int timer;
};

struct connection {
	struct connection *next;
	struct connection *prev;
	tcount count;
	unsigned char *url;
	int running;
	int state;
	int prev_error;
	off_t from;
	int pri[N_PRI];
	int no_cache;
	int sock1;
	int sock2;
	void *dnsquery;
	pid_t pid;
	int tries;
	struct list_head statuss;
	void *info;
	void *buffer;
	struct conn_info *newconn;
	void (*conn_func)(void *);
	struct cache_entry *cache;
	off_t received;
	off_t est_length;
	int unrestartable;
	struct remaining_info prg;
	int timer;
	int detached;
#ifdef HAVE_SSL
	SSL *ssl;
	int no_tsl;
#endif
};

extern struct list_head queue;

struct k_conn {
	struct k_conn *next;
	struct k_conn *prev;
	void (*protocol)(struct connection *);
	unsigned char *host;
	int port;
	int conn;
	ttime timeout;
	ttime add_time;
};

extern struct list_head keepalive_connections;

static inline int getpri(struct connection *c)
{
	int i;
	for (i = 0; i < N_PRI; i++) if (c->pri[i]) return i;
	internal("connection has no owner");
	return N_PRI;
}

#define NC_ALWAYS_CACHE	0
#define NC_CACHE	1
#define NC_IF_MOD	2
#define NC_RELOAD	3
#define NC_PR_NO_CACHE	4

#define S_WAIT		0
#define S_DNS		1
#define S_CONN		2
#define S_SSL_NEG	3
#define S_SENT		4
#define S_LOGIN		5
#define S_GETH		6
#define S_PROC		7
#define S_TRANS		8

#define S_OK			(-2000000000)
#define S_INTERRUPTED		(-2000000001)
#define S_EXCEPT		(-2000000002)
#define S_INTERNAL		(-2000000003)
#define S_OUT_OF_MEM		(-2000000004)
#define S_NO_DNS		(-2000000005)
#define S_CANT_WRITE		(-2000000006)
#define S_CANT_READ		(-2000000007)
#define S_MODIFIED		(-2000000008)
#define S_BAD_URL		(-2000000009)
#define S_TIMEOUT		(-2000000010)
#define S_RESTART		(-2000000011)
#define S_STATE			(-2000000012)
#define S_LARGE_FILE		(-2000000014)

#define S_HTTP_ERROR		(-2000000100)
#define S_HTTP_100		(-2000000101)
#define S_HTTP_204		(-2000000102)

#define S_FILE_TYPE		(-2000000200)
#define S_FILE_ERROR		(-2000000201)

#define S_FTP_ERROR		(-2000000300)
#define S_FTP_UNAVAIL		(-2000000301)
#define S_FTP_LOGIN		(-2000000302)
#define S_FTP_PORT		(-2000000303)
#define S_FTP_NO_FILE		(-2000000304)
#define S_FTP_FILE_ERROR	(-2000000305)

#define S_SSL_ERROR		(-2000000400)
#define S_NO_SSL		(-2000000401)

#define S_NO_SMB_CLIENT		(-2000000500)

#define S_WAIT_REDIR		(-2000000600)

#define S_UNKNOWN_ERROR		(-2000000700)

#define S_MAX			(-2000000900)

extern struct s_msg_dsc {
	int n;
	unsigned char *msg;
} msg_dsc[];

struct status {
	struct status *next;
	struct status *prev;
	struct connection *c;
	struct cache_entry *ce;
	int state;
	int prev_error;
	int pri;
	void (*end)(struct status *, void *);
	void *data;
	struct remaining_info *prg;
};

void check_queue(void *dummy);
long connect_info(int);
void send_connection_info(struct connection *c);
void setcstate(struct connection *c, int);
int get_keepalive_socket(struct connection *c);
void add_keepalive_socket(struct connection *c, ttime);
void run_connection(struct connection *c);
int is_connection_restartable(struct connection *c);
void retry_connection(struct connection *c);
void abort_connection(struct connection *c);
void end_connection(struct connection *c);
int load_url(unsigned char *, struct status *, int, int);
void change_connection(struct status *, struct status *, int);
void detach_connection(struct status *, off_t);
void abort_all_connections();
void abort_background_connections();
int is_entry_used(struct cache_entry *);
void connection_timeout(struct connection *);
void set_timeout(struct connection *);
void add_blacklist_entry(unsigned char *, int);
void del_blacklist_entry(unsigned char *, int);
int get_blacklist_flags(unsigned char *);
void free_blacklist();

#define BL_HTTP10		1
#define BL_NO_ACCEPT_LANGUAGE	2
#define BL_NO_CHARSET		4
#define BL_NO_RANGE		8

/* url.c */

struct session;

#define POST_CHAR 1

static inline int end_of_dir(unsigned char c)
{
	return c == POST_CHAR || c == '#' || c == ';' || c == '?';
}

int parse_url(unsigned char *, int *, unsigned char **, int *, unsigned char **, int *, unsigned char **, int *, unsigned char **, int *, unsigned char **, int *, unsigned char **);
unsigned char *get_host_name(unsigned char *);
unsigned char *get_host_and_pass(unsigned char *);
unsigned char *get_user_name(unsigned char *);
unsigned char *get_pass(unsigned char *);
unsigned char *get_port_str(unsigned char *);
int get_port(unsigned char *);
void (*get_protocol_handle(unsigned char *))(struct connection *);
void (*get_external_protocol_function(unsigned char *))(struct session *, unsigned char *);
unsigned char *get_url_data(unsigned char *);
unsigned char *join_urls(unsigned char *, unsigned char *);
unsigned char *translate_url(unsigned char *, unsigned char *);
unsigned char *extract_position(unsigned char *);
void get_filename_from_url(unsigned char *, unsigned char **, int *);
void add_conv_str(unsigned char **s, int *l, unsigned char *b, int ll, int encode_special);

/* connect.c */

struct read_buffer {
	int sock;
	int len;
	int close;
	void (*done)(struct connection *, struct read_buffer *);
	unsigned char data[1];
};

void exception(struct connection *);
void close_socket(int *);
void make_connection(struct connection *, int, int *, void (*)(struct connection *));
int get_pasv_socket(struct connection *, int, int *, unsigned char *);
void write_to_socket(struct connection *, int, unsigned char *, int, void (*)(struct connection *));
struct read_buffer *alloc_read_buffer(struct connection *c);
void read_from_socket(struct connection *, int, struct read_buffer *, void (*)(struct connection *, struct read_buffer *));
void kill_buffer_data(struct read_buffer *, int);

/* cookies.c */

int set_cookie(struct terminal *, unsigned char *, unsigned char *);
void send_cookies(unsigned char **, int *, unsigned char *);
void init_cookies();
void cleanup_cookies();

/* http.c */

unsigned char *parse_http_header(unsigned char *, unsigned char *, unsigned char **);
unsigned char *parse_header_param(unsigned char *, unsigned char *);
void http_func(struct connection *);
void proxy_func(struct connection *);


/* https.c */

void https_func(struct connection *c);
#ifdef HAVE_SSL
void ssl_finish(void);
SSL *getSSL(void);
#endif

/* file.c */

void file_func(struct connection *);

/* finger.c */

void finger_func(struct connection *);

/* ftp.c */

#if defined(IP_TOS) && defined(IPTOS_THROUGHPUT)
#define HAVE_IPTOS
#endif

void ftp_func(struct connection *);

/* smb.c */

void smb_func(struct connection *);

/* mailto.c */

void mailto_func(struct session *, unsigned char *);
void telnet_func(struct session *, unsigned char *);
void tn3270_func(struct session *, unsigned char *);
void mms_func(struct session *, unsigned char *);

/* kbd.c */

#define BM_BUTT		3
#define B_LEFT		0
#define B_MIDDLE	1
#define B_RIGHT		2
#define BM_ACT		12
#define B_DOWN		0
#define B_UP		4
#define B_DRAG		8

#define KBD_ENTER	0x100
#define KBD_BS		0x101
#define KBD_TAB		0x102
#define KBD_ESC		0x103
#define KBD_LEFT	0x104
#define KBD_RIGHT	0x105
#define KBD_UP		0x106
#define KBD_DOWN	0x107
#define KBD_INS		0x108
#define KBD_DEL		0x109
#define KBD_HOME	0x10a
#define KBD_END		0x10b
#define KBD_PAGE_UP	0x10c
#define KBD_PAGE_DOWN	0x10d

#define KBD_F1		0x120
#define KBD_F2		0x121
#define KBD_F3		0x122
#define KBD_F4		0x123
#define KBD_F5		0x124
#define KBD_F6		0x125
#define KBD_F7		0x126
#define KBD_F8		0x127
#define KBD_F9		0x128
#define KBD_F10		0x129
#define KBD_F11		0x12a
#define KBD_F12		0x12b

#define KBD_CTRL_C	0x200

#define KBD_SHIFT	1
#define KBD_CTRL	2
#define KBD_ALT		4

void handle_trm(int, int, int, int, int, void *, int);
void free_all_itrms();
void resize_terminal();
void dispatch_special(unsigned char *);
void kbd_ctrl_c();
int is_blocked();

extern unsigned char init_seq[];
extern unsigned char init_seq_x_mouse[];
extern unsigned char init_seq_tw_mouse[];
extern unsigned char term_seq[];
extern unsigned char term_seq_x_mouse[];
extern unsigned char term_seq_tw_mouse[];

/* terminal.c */

typedef unsigned short chr;

struct event {
	long ev;
	long x;
	long y;
	long b;
};

#define EV_INIT		0
#define EV_KBD		1
#define EV_MOUSE	2
#define EV_REDRAW	3
#define EV_RESIZE	4
#define EV_ABORT	5

struct window {
	struct window *next;
	struct window *prev;
	void (*handler)(struct window *, struct event *, int fwd);
	void *data;
	int xp, yp;
	struct terminal *term;
};

#define MAX_TERM_LEN	32	/* this must be multiple of 8! (alignment problems) */

#define MAX_CWD_LEN	256	/* this must be multiple of 8! (alignment problems) */	

#define ENV_XWIN	1
#define ENV_SCREEN	2
#define ENV_OS2VIO	4
#define ENV_BE		8
#define ENV_TWIN	16
#define ENV_WIN32	32
#define ENV_INTERIX	64

struct terminal {
	struct terminal *next;
	struct terminal *prev;
	int master;
	int fdin;
	int fdout;
	int x;
	int y;
	int environment;
	unsigned char term[MAX_TERM_LEN];
	unsigned char cwd[MAX_CWD_LEN];
	unsigned *screen;
	unsigned *last_screen;
	struct term_spec *spec;
	int cx;
	int cy;
	int lcx;
	int lcy;
	int dirty;
	int redrawing;
	int blocked;
	unsigned char *input_queue;
	int qlen;
	struct list_head windows;
	unsigned char *title;
};

struct term_spec {
	struct term_spec *next;
	struct term_spec *prev;
	unsigned char term[MAX_TERM_LEN];
	int mode;
	int m11_hack;
	int restrict_852;
	int block_cursor;
	int col;
	int charset;
};

#define TERM_DUMB	0
#define TERM_VT100	1
#define TERM_LINUX	2
#define TERM_KOI8	3
#define TERM_FREEBSD	4

#define ATTR_FRAME	0x8000

extern struct list_head term_specs;
extern struct list_head terminals;

int hard_write(int, unsigned char *, int);
int hard_read(int, unsigned char *, int);
unsigned char *get_cwd();
void set_cwd(unsigned char *);
struct terminal *init_term(int, int, void (*)(struct window *, struct event *, int));
void sync_term_specs();
struct term_spec *new_term_spec(unsigned char *);
void free_term_specs();
void destroy_terminal(struct terminal *);
void redraw_terminal(struct terminal *);
void redraw_terminal_all(struct terminal *);
void redraw_terminal_cls(struct terminal *);
void cls_redraw_all_terminals();
void redraw_from_window(struct window *);
void redraw_below_window(struct window *);
void add_window(struct terminal *, void (*)(struct window *, struct event *, int), void *);
void add_window_at_pos(struct terminal *, void (*)(struct window *, struct event *, int), void *, struct window *);
void delete_window(struct window *);
void delete_window_ev(struct window *, struct event *ev);
void set_window_ptr(struct window *, int, int);
void get_parent_ptr(struct window *, int *, int *);
struct window *get_root_window(struct terminal *);
void add_empty_window(struct terminal *, void (*)(void *), void *);
void redraw_screen(struct terminal *);
void redraw_all_terminals();
void set_char(struct terminal *, int, int, unsigned);
unsigned get_char(struct terminal *, int, int);
void set_color(struct terminal *, int, int, unsigned);
void set_only_char(struct terminal *, int, int, unsigned);
void set_line(struct terminal *, int, int, int, chr *);
void set_line_color(struct terminal *, int, int, int, unsigned);
void fill_area(struct terminal *, int, int, int, int, unsigned);
void draw_frame(struct terminal *, int, int, int, int, unsigned, int);
void print_text(struct terminal *, int, int, int, unsigned char *, unsigned);
void set_cursor(struct terminal *, int, int, int, int);
void destroy_all_terminals();
void block_itrm(int);
int unblock_itrm(int);
void exec_thread(unsigned char *, int);
void close_handle(void *);

#define TERM_FN_TITLE	1
#define TERM_FN_RESIZE	2

void exec_on_terminal(struct terminal *, unsigned char *, unsigned char *, int);
void set_terminal_title(struct terminal *, unsigned char *);
void do_terminal_function(struct terminal *, unsigned char, unsigned char *);

/* language.c */

#include "language.h"

extern unsigned char dummyarray[];

extern int current_language;

void init_trans();
void shutdown_trans();
unsigned char *get_text_translation(unsigned char *, struct terminal *term);
unsigned char *get_english_translation(unsigned char *);
void set_language(int);
int n_languages();
unsigned char *language_name(int);

#define _(_x_, _y_) get_text_translation(_x_, _y_)
#define TEXT_(x) (dummyarray + x) /* TEXT causes name clash on windows */

/* af_unix.c */

int bind_to_af_unix();
void af_unix_close();

/* main.c */

extern int retval;

extern unsigned char *path_to_exe;
extern unsigned char **g_argv;
extern int g_argc;

void unhandle_terminal_signals(struct terminal *term);
int attach_terminal(int, int, int, void *, int);
void program_exit();
void shrink_memory(int);

/* types.c */

struct assoc {
	struct assoc *next;
	struct assoc *prev;
	tcount cnt;
	unsigned char *label;
	unsigned char *ct;
	unsigned char *prog;
	int cons;
	int xwin;
	int block;
	int ask;
	int system;
};

struct extension {
	struct extension *next;
	struct extension *prev;
	tcount cnt;
	unsigned char *ext;
	unsigned char *ct;
};

struct protocol_program {
	struct protocol_program *next;
	struct protocol_program *prev;
	unsigned char *prog;
	int system;
};

extern struct list_head assoc;
extern struct list_head extensions;

extern struct list_head mailto_prog;
extern struct list_head telnet_prog;
extern struct list_head tn3270_prog;
extern struct list_head mms_prog;

unsigned char *get_content_type(unsigned char *, unsigned char *);
struct assoc *get_type_assoc(struct terminal *term, unsigned char *);
void update_assoc(struct assoc *);
void update_ext(struct extension *);
void update_prog(struct list_head *, unsigned char *, int);
unsigned char *get_prog(struct list_head *);
void free_types();

void menu_add_ct(struct terminal *, void *, void *);
void menu_del_ct(struct terminal *, void *, void *);
void menu_list_assoc(struct terminal *, void *, void *);
void menu_add_ext(struct terminal *, void *, void *);
void menu_del_ext(struct terminal *, void *, void *);
void menu_list_ext(struct terminal *, void *, void *);

int is_html_type(unsigned char *ct);

/* session.c */

struct link_def {
	unsigned char *link;
	unsigned char *target;
};

struct line {
	int l;
	chr c;
	chr *d;
};

struct point {
	int x;
	int y;
};

struct form {
	unsigned char *action;
	unsigned char *target;
	int method;
	int num;
};

#define FM_GET		0
#define FM_POST		1
#define FM_POST_MP	2

#define FC_TEXT		1
#define FC_PASSWORD	2
#define FC_FILE		3
#define FC_TEXTAREA	4
#define FC_CHECKBOX	5
#define FC_RADIO	6
#define FC_SELECT	7
#define FC_SUBMIT	8
#define FC_IMAGE	9
#define FC_RESET	10
#define FC_HIDDEN	11

struct form_control {
	struct form_control *next;
	struct form_control *prev;
	int form_num;
	int ctrl_num;
	int g_ctrl_num;
	int position;
	int method;
	unsigned char *action;
	unsigned char *target;
	int type;
	unsigned char *name;
	unsigned char *alt;
	int ro;
	unsigned char *default_value;
	int default_state;
	int size;
	int cols, rows, wrap;
	int maxlength;
	int nvalues;
	unsigned char **values;
	unsigned char **labels;
	struct menu_item *menu;
};

struct form_state {
	int form_num;
	int ctrl_num;
	int g_ctrl_num;
	int position;
	int type;
	unsigned char *value;
	int state;
	int vpos;
	int vypos;
};

struct link {
	int type;
	int num;
	unsigned char *where;
	unsigned char *target;
	unsigned char *where_img;
	struct form_control *form;
	unsigned sel_color;
	int n;
	struct point *pos;
};

#define L_LINK		0
#define L_BUTTON	1
#define L_CHECKBOX	2
#define L_SELECT	3
#define L_FIELD		4
#define L_AREA		5

#define SIZEOF_F_DATA sizeof(struct f_data)

struct link_bg {
	int x, y;
	unsigned c;
};

struct tag {
	struct tag *next;
	struct tag *prev;
	int x;
	int y;
	unsigned char name[1];
};

struct rgb {
	unsigned char r, g, b;
	unsigned char pad;
};

struct document_setup {
	int assume_cp, hard_assume;
	int tables, frames, images;
	int margin;
	int num_links, table_order;
};

struct document_options {
	int xw, yw; /* size of window */
	int xp, yp; /* pos of window */
	int col, cp, assume_cp, hard_assume;
	int tables, frames, images, margin;  /* if you add anything, fix it in compare_opt */
	int plain;
	int num_links, table_order;
	struct rgb default_fg;
	struct rgb default_bg;
	struct rgb default_link;
	struct rgb default_vlink;
	unsigned char *framename;
	int real_cp;    /* codepage of document. Does not really belong here. Must not be compared. Used only in get_attr_val */
};

static inline void ds2do(struct document_setup *ds, struct document_options *doo)
{
	doo->assume_cp = ds->assume_cp;
	doo->hard_assume = ds->hard_assume;
	doo->tables = ds->tables;
	doo->frames = ds->frames;
	doo->images = ds->images;
	doo->margin = ds->margin;
	doo->num_links = ds->num_links;
	doo->table_order = ds->table_order;
}

struct node {
	struct node *next;
	struct node *prev;
	int x, y;
	int xw, yw;
};

struct search {
	unsigned char c;
	int n:24;	/* This structure is size-critical */
	int x, y;
};

struct frameset_desc;

struct frame_desc {
	struct frameset_desc *subframe;
	unsigned char *name;
	unsigned char *url;
	int line;
	int xw, yw;
};

struct frameset_desc {
	int n;
	int x, y;
	int xp, yp;
	struct frame_desc f[1];
};

struct f_data {
	struct f_data *next;
	struct f_data *prev;
	int refcount;
	unsigned char *url;
	struct document_options opt;
	unsigned char *title;
	int cp, ass;
	int x, y; /* size of document */
	ttime time_to_get;
	tcount use_tag;
	int frame;
	struct frameset_desc *frame_desc;
	int bg;
	struct line *data;
	struct link *links;
	int nlinks;
	struct link **lines1;
	struct link **lines2;
	struct list_head forms;
	struct list_head tags;
	struct list_head nodes;
	struct search *search;
	int nsearch;
	struct search **slines1;
	struct search **slines2;
};

struct f_data_c {
	struct f_data_c *next;
	struct f_data_c *prev;
	int used;
	unsigned char *name;
	struct f_data *f_data;
	int xw, yw; /* size of window */
	int xp, yp; /* pos of window */
	int xl, yl; /* last pos of window */
	struct link_bg *link_bg;
	int link_bg_n;
	unsigned char **search_word;
	struct view_state *vs;
	int depth;
};

struct view_state {
	int view_pos;
	int view_posx;
	int orig_view_pos;
	int orig_view_posx;
	int current_link;
	int orig_link;
	int plain;
	unsigned char *goto_position;
	unsigned char *goto_position_end;
	struct form_state *form_info;
	int form_info_len;
	unsigned char url[1];
};

struct frame {
	struct frame *next;
	struct frame *prev;
	unsigned char *name;
	int redirect_cnt;
	struct view_state vs;
};

struct location {
	struct location *next;
	struct location *prev;
	struct list_head frames;
	struct status stat;
	struct view_state vs;
};

#define WTD_NO		0
#define WTD_FORWARD	1
#define WTD_IMGMAP	2
#define WTD_RELOAD	3
#define WTD_BACK	4

#define cur_loc(x) ((struct location *)((x)->history.next))

struct kbdprefix {
	int rep;
	int rep_num;
	int prefix;
};

struct download {
	struct download *next;
	struct download *prev;
	unsigned char *url;
	struct status stat;
	unsigned char *file;
	off_t last_pos;
	int handle;
	int redirect_cnt;
	unsigned char *prog;
	int prog_flags;
	time_t remotetime;
	struct session *ses;
	struct window *win;
	struct window *ask;
};

extern struct list_head downloads;

struct file_to_load {
	struct file_to_load *next;
	struct file_to_load *prev;
	struct session *ses;
	int req_sent;
	int pri;
	struct cache_entry *ce;
	unsigned char *url;
	struct status stat;
};

struct session {
	struct session *next;
	struct session *prev;
	struct list_head history;
	struct terminal *term;
	struct window *win;
	int id;
	struct f_data_c *screen;
	struct list_head scrn_frames;
	struct status loading;
	int wtd;
	unsigned char *wtd_target;
	unsigned char *loading_url;
	int display_timer;
	struct list_head more_files;
	unsigned char *goto_position;
	unsigned char *imgmap_href_base;
	unsigned char *imgmap_target_base;
	struct document_setup ds;
	struct kbdprefix kbdprefix;
	int reloadlevel;
	int redirect_cnt;
	struct status tq;
	unsigned char *tq_url;
	struct cache_entry *tq_ce;
	unsigned char *tq_goto_position;
	unsigned char *tq_prog;
	int tq_prog_flags;
	unsigned char *dn_url;
	unsigned char *search_word;
	unsigned char *last_search_word;
	int search_direction;
	int exit_query;
};

extern struct list_head sessions;

time_t parse_http_date(const char *);
unsigned char *encode_url(unsigned char *);
unsigned char *decode_url(unsigned char *);
unsigned char *subst_file(unsigned char *, unsigned char *, int);
int are_there_downloads();
void free_strerror_buf();
int get_error_from_errno(int errn);
unsigned char *get_err_msg(int);
void print_screen_status(struct session *);
void print_error_dialog(struct session *, struct status *, unsigned char *);
void start_download(struct session *, unsigned char *);
void display_download(struct terminal *, struct download *, struct session *);
int create_download_file(struct terminal *, unsigned char *, int);
struct file_to_load *request_additional_file(struct session *, unsigned char *, int);
struct file_to_load *request_additional_loading_file(struct session *, unsigned char *, struct status *, int);
void process_file_requests(struct session *);
int read_session_info(int, struct session *, void *, int);
void *create_session_info(int, unsigned char *, int *);
void win_func(struct window *, struct event *, int);
void goto_url_f(struct session *, unsigned char *, unsigned char *);
void goto_url(struct session *, unsigned char *);
void abort_loading(struct session *);
void goto_imgmap(struct session *, unsigned char *, unsigned char *, unsigned char *);
void go_back(struct session *);
void reload(struct session*, int);
struct frame *ses_find_frame(struct session *, unsigned char *);
struct frame *ses_change_frame_url(struct session *, unsigned char *, unsigned char *);
void map_selected(struct terminal *, struct link_def *, struct session *);
void load_frames(struct session *, struct f_data_c *);
void destroy_session(struct session *);
void destroy_all_sessions();
void abort_all_downloads();
void destroy_location(struct location *);

/* Information about the current document */
unsigned char *get_current_url(struct session *, unsigned char *, size_t);
unsigned char *get_current_title(struct session *, unsigned char *, size_t);

unsigned char *get_current_link_url(struct session *, unsigned char *, size_t);

/* bfu.c */

struct memory_list {
	int n;
	void *p[1];
};

struct memory_list *getml(void *, ...);
void add_to_ml(struct memory_list **, ...);
void freeml(struct memory_list *);

#define MENU_FUNC (void (*)(struct terminal *, void *, void *))

extern unsigned char m_bar;

#define M_BAR	(&m_bar)

struct menu_item {
	unsigned char *text;
	unsigned char *rtext;
	unsigned char *hotkey;
	void (*func)(struct terminal *, void *, void *);
	void *data;
	int in_m;
	int free_i;
};

struct menu {
	int selected;
	int view;
	int xp, yp;
	int x, y, xw, yw;
	int ni;
	void *data;
	struct window *win;
	struct menu_item *items;
};

struct mainmenu {
	int selected;
	int sp;
	int ni;
	void *data;
	struct window *win;
	struct menu_item *items;
};

struct history_item {
	struct history_item *next;
	struct history_item *prev;
	unsigned char d[1];
};

struct history {
	int n;
	struct list_head items;
};

#define D_END		0
#define D_CHECKBOX	1
#define D_FIELD		2
#define D_FIELD_PASS	3
#define D_BUTTON	4
#define D_BOX		5

#define B_ENTER		1
#define B_ESC		2

struct dialog_item_data;
struct dialog_data;

struct dialog_item {
	int type;
	int gid, gnum; /* for buttons: gid - flags B_XXX */	/* for fields: min/max */ /* for box: gid is box height */
	int (*fn)(struct dialog_data *, struct dialog_item_data *);
	struct history *history;
	int dlen;
	unsigned char *data;
	void *udata; /* for box: holds list */
	unsigned char *text;
};

struct dialog_item_data {
	int x, y, l;
	int vpos, cpos;
	int checked;
	struct dialog_item *item;
	struct list_head history;
	struct history_item *cur_hist;
	unsigned char *cdata;
};

#define	EVENT_PROCESSED		0
#define EVENT_NOT_PROCESSED	1

struct dialog {
	unsigned char *title;
	void (*fn)(struct dialog_data *);
	int (*handle_event)(struct dialog_data *, struct event *);
	void (*abort)(struct dialog_data *);
	void *udata;
	void *udata2;
	int align;
	void (*refresh)(void *);
	void *refresh_data;
	struct dialog_item items[1];
};

struct dialog_data {
	struct window *win;
	struct dialog *dlg;
	int x, y, xw, yw;
	int n;
	int selected;
	struct memory_list *ml;
	struct dialog_item_data items[1];
};

struct menu_item *new_menu(int);
void add_to_menu(struct menu_item **, unsigned char *, unsigned char *, unsigned char *, void (*)(struct terminal *, void *, void *), void *, int);
void do_menu(struct terminal *, struct menu_item *, void *);
void do_menu_selected(struct terminal *, struct menu_item *, void *, int);
void do_mainmenu(struct terminal *, struct menu_item *, void *, int);
void do_dialog(struct terminal *, struct dialog *, struct memory_list *);
int check_number(struct dialog_data *, struct dialog_item_data *);
int check_nonempty(struct dialog_data *, struct dialog_item_data *);
void max_text_width(struct terminal *, unsigned char *, int *);
void min_text_width(struct terminal *, unsigned char *, int *);
void dlg_format_text(struct terminal *, struct terminal *, unsigned char *, int, int *, int, int *, int, int);
void max_buttons_width(struct terminal *, struct dialog_item_data *, int, int *);
void min_buttons_width(struct terminal *, struct dialog_item_data *, int, int *);
void dlg_format_buttons(struct terminal *, struct terminal *, struct dialog_item_data *, int, int, int *, int, int *, int);
void checkboxes_width(struct terminal *, unsigned char **, int *, void (*)(struct terminal *, unsigned char *, int *));
void dlg_format_checkbox(struct terminal *, struct terminal *, struct dialog_item_data *, int, int *, int, int *, unsigned char *);
void dlg_format_checkboxes(struct terminal *, struct terminal *, struct dialog_item_data *, int, int, int *, int, int *, unsigned char **);
void dlg_format_field(struct terminal *, struct terminal *, struct dialog_item_data *, int, int *, int, int *, int);
void max_group_width(struct terminal *, unsigned char **, struct dialog_item_data *, int, int *);
void min_group_width(struct terminal *, unsigned char **, struct dialog_item_data *, int, int *);
void dlg_format_group(struct terminal *, struct terminal *, unsigned char **, struct dialog_item_data *, int, int, int *, int, int *);
void dlg_format_box(struct terminal *, struct terminal *, struct dialog_item_data *, int, int *, int, int *, int);
void checkbox_list_fn(struct dialog_data *);
void group_fn(struct dialog_data *);
void center_dlg(struct dialog_data *);
void draw_dlg(struct dialog_data *);
void display_dlg_item(struct dialog_data *, struct dialog_item_data *, int);
int ok_dialog(struct dialog_data *, struct dialog_item_data *);
int cancel_dialog(struct dialog_data *, struct dialog_item_data *);
void msg_box(struct terminal *, struct memory_list *, unsigned char *, int, /*unsigned char *, void *, int,*/ ...);
void input_field_fn(struct dialog_data *);
void input_field(struct terminal *, struct memory_list *, unsigned char *, unsigned char *, unsigned char *, unsigned char *, void *, struct history *, int, unsigned char *, int, int, int (*)(struct dialog_data *, struct dialog_item_data *), void (*)(void *, unsigned char *), void (*)(void *));
void add_to_history(struct history *, unsigned char *, int);

void box_sel_move(struct dialog_item_data *, int ); 
void show_dlg_item_box(struct dialog_data *, struct dialog_item_data *);
void box_sel_set_visible(struct dialog_item_data *, int ); 

/* menu.c */

extern struct history goto_url_history;

void activate_bfu_technology(struct session *, int);
void dialog_goto_url(struct session *ses, char *url);
void dialog_save_url(struct session *ses);
void free_history_lists();
void query_file(struct session *, unsigned char *, void (*)(struct session *, unsigned char *), void (*)(struct session *));
void search_dlg(struct session *, struct f_data_c *, int);
void search_back_dlg(struct session *, struct f_data_c *, int);
void exit_prog(struct terminal *, void *, struct session *);
void do_auth_dialog(struct session *);

/* charsets.c */

#include "codepage.h"

struct conv_table {
	int t;
	union {
		unsigned char *str;
		struct conv_table *tbl;
	} u;
};

int cp2u(unsigned char ch, int from);
struct conv_table *get_translation_table(int, int);
int get_entity_number(unsigned char *st, int l);
unsigned char *get_entity_string(unsigned char *, int, int);
unsigned char *convert_string(struct conv_table *, unsigned char *, int);
int get_cp_index(unsigned char *n);
unsigned char *get_cp_name(int);
unsigned char *get_cp_mime_name(int);
int is_cp_special(int);
void free_conv_table();

unsigned char charset_upcase(unsigned char, int);
void charset_upcase_string(unsigned char **, int);

/* view.c */

int can_open_in_new(struct terminal *);
void open_in_new_window(struct terminal *, void (*)(struct terminal *, void (*)(struct terminal *, unsigned char *, unsigned char *), struct session *ses), struct session *);
void send_open_in_new_xterm(struct terminal *term, void (*open_window)(struct terminal *term, unsigned char *, unsigned char *), struct session *ses);
void send_open_new_xterm(struct terminal *, void (*)(struct terminal *, unsigned char *, unsigned char *), struct session *);
void destroy_fc(struct form_control *);
void sort_links(struct f_data *);
void destroy_formatted(struct f_data *);
void clear_formatted(struct f_data *);
void init_formatted(struct f_data *);
void detach_formatted(struct f_data_c *);
void init_vs(struct view_state *, unsigned char *);
void destroy_vs(struct view_state *);
void copy_location(struct location *, struct location *);
void draw_doc(struct terminal *, struct f_data_c *, int);
int dump_to_file(struct f_data *, int);
void draw_formatted(struct session *);
void send_event(struct session *, struct event *);
void link_menu(struct terminal *, void *, struct session *);
void save_as(struct terminal *, void *, struct session *);
void save_url(struct session *, unsigned char *);
void menu_save_formatted(struct terminal *, void *, struct session *);
void selected_item(struct terminal *, void *, struct session *);
void toggle(struct session *, struct f_data_c *, int);
void do_for_frame(struct session *, void (*)(struct session *, struct f_data_c *, int), int);
int get_current_state(struct session *);
unsigned char *print_current_link(struct session *);
unsigned char *print_current_title(struct session *);
void loc_msg(struct terminal *, struct location *, struct f_data_c *);
void state_msg(struct session *);
void head_msg(struct session *);
void search_for(struct session *, unsigned char *);
void search_for_back(struct session *, unsigned char *);
void find_next(struct session *, struct f_data_c *, int);
void find_next_back(struct session *, struct f_data_c *, int);
void set_frame(struct session *, struct f_data_c *, int);
struct f_data_c *current_frame(struct session *);

/* html.c */

#define AT_BOLD		1
#define AT_ITALIC	2
#define AT_UNDERLINE	4
#define AT_FIXED	8
#define AT_GRAPHICS	16

#define AL_LEFT		0
#define AL_CENTER	1
#define AL_RIGHT	2
#define AL_BLOCK	3
#define AL_NO		4

#define AL_MASK		0x7f

#define AL_EXTD_TEXT	0x80
	/* DIRTY! for backward compatibility with old menu code */

struct text_attrib_beginning {
	int attr;
	struct rgb fg;
	struct rgb bg;
};

struct text_attrib {
	int attr;
	struct rgb fg;
	struct rgb bg;
	int fontsize;
	unsigned char *link;
	unsigned char *target;
	unsigned char *image;
	struct form_control *form;
	struct rgb clink;
	struct rgb vlink;
	unsigned char *href_base;
	unsigned char *target_base;
	unsigned char *select;
	int select_disabled;
};

#define P_NUMBER	1
#define P_alpha		2
#define P_ALPHA		3
#define P_roman		4
#define P_ROMAN		5
#define P_STAR		1
#define P_O		2
#define P_PLUS		3
#define P_LISTMASK	7
#define P_COMPACT	8

struct par_attrib {
	int align;
	int leftmargin;
	int rightmargin;
	int width;
	int list_level;
	unsigned list_number;
	int dd_margin;
	int flags;
	struct rgb bgcolor;
};

struct html_element {
	struct html_element *next;
	struct html_element *prev;
	struct text_attrib attr;
	struct par_attrib parattr;
	int invisible;
	unsigned char *name;
	int namelen;
	unsigned char *options;
	int linebreak;
	int dontkill;
	struct frameset_desc *frameset;
};

extern int get_attr_val_nl;

extern struct list_head html_stack;
extern int line_breax;

extern unsigned char *startf;
extern unsigned char *eofff;

#define format_ (((struct html_element *)html_stack.next)->attr)
#define par_format (((struct html_element *)html_stack.next)->parattr)
#define html_top (*(struct html_element *)html_stack.next)

extern void *ff;
extern int (*put_chars_f)(void *, unsigned char *, int);
extern void (*line_break_f)(void *);
extern void *(*special_f)(void *, int, ...);

void ln_break(int, void (*)(void *), void *);
void put_chrs(unsigned char *, int, int (*)(void *, unsigned char *, int), void *);

extern int table_level;
extern int empty_format;

extern struct form form;
extern unsigned char *last_form_tag;
extern unsigned char *last_form_attr;
extern unsigned char *last_input_tag;

int parse_element(unsigned char *, unsigned char *, unsigned char **, int *, unsigned char **, unsigned char **);
unsigned char *get_attr_val(unsigned char *, unsigned char *);
int has_attr(unsigned char *, unsigned char *);
int get_num(unsigned char *, unsigned char *);
int get_width(unsigned char *, unsigned char *, int);
int get_color(unsigned char *, unsigned char *, struct rgb *);
int get_bgcolor(unsigned char *, struct rgb *);
void html_stack_dup();
void kill_html_stack_item(struct html_element *);
unsigned char *skip_comment(unsigned char *, unsigned char *);
void parse_html(unsigned char *, unsigned char *, int (*)(void *, unsigned char *, int), void (*)(void *), void *(*)(void *, int, ...), void *, unsigned char *);
int get_image_map(unsigned char *, unsigned char *, unsigned char *, unsigned char *a, struct menu_item **, struct memory_list **, unsigned char *, unsigned char *, int, int, int);
void scan_http_equiv(unsigned char *, unsigned char *, unsigned char **, int *, unsigned char **);

#define SP_TAG		0
#define SP_CONTROL	1
#define SP_TABLE	2
#define SP_USED		3
#define SP_FRAMESET	4
#define SP_FRAME	5
#define SP_NOWRAP	6

struct frameset_param {
	struct frameset_desc *parent;
	int x, y;
	int *xw, *yw;
};

struct frame_param {
	struct frameset_desc *parent;
	unsigned char *name;
	unsigned char *url;
};

void free_menu(struct menu_item *);
void do_select_submenu(struct terminal *, struct menu_item *, struct session *);

/* html_r.c */

struct part {
	int x, y;
	int xp, yp;
	int xmax;
	int xa;
	int cx, cy;
	struct f_data *data;
	int bgcolor;
	unsigned char *spaces;
	int spl;
	int link_num;
	struct list_head uf;
};

struct sizes {
	int xmin, xmax, y;
};

extern struct document_options *d_opt;
extern int last_link_to_move;
extern int margin;

void xxpand_line(struct part *, int, int);
void xxpand_lines(struct part *, int);
void xset_hchar(struct part *, int, int, unsigned);
void xset_hchars(struct part *, int, int, int, unsigned);
void align_line(struct part *, int);

void free_table_cache();

struct conv_table *get_convert_table(unsigned char *, int, int, int *, int *, int);
extern int format_cache_entries;
long formatted_info(int);
void shrink_format_cache(int);
void count_format_cache();
void delete_unused_format_cache_entries();
void format_cache_reactivate(struct f_data *);
struct part *format_html_part(unsigned char *, unsigned char *, int, int, int, struct f_data *, int, int, unsigned char *, int);
void cached_format_html(struct view_state *, struct f_data_c *, struct document_options *);
void html_interpret(struct session *);
void get_search_data(struct f_data *);

/* html_tbl.c */

void format_table(unsigned char *, unsigned char *, unsigned char *, unsigned char **, void *);

/* default.c */

#define MAX_STR_LEN	1024

struct option {
	int p;
	unsigned char *(*rd_cmd)(struct option *, unsigned char ***, int *);
	unsigned char *(*rd_cfg)(struct option *, unsigned char *);
	void (*wr_cfg)(struct option *, unsigned char **, int *);
	int min, max;
	void *ptr;
	unsigned char *cfg_name;
	unsigned char *cmd_name;
};

unsigned char *parse_options(int, unsigned char *[]);
void init_home();
unsigned char *read_config_file(unsigned char *name);
int write_to_config_file(unsigned char *, unsigned char *);
unsigned char *get_token(unsigned char **line);
void load_config();
void write_config(struct terminal *);
void write_html_config(struct terminal *);
void end_config();

void load_url_history();
void save_url_history();

extern int anonymous;

extern unsigned char system_name[];

extern unsigned char *links_home;
extern int first_use;

extern int created_home;

extern int no_connect;
extern int base_session;
extern int force_html;

#define D_DUMP		1
#define D_SOURCE	2
extern int dmp;

extern int async_lookup;
extern int download_utime;
extern int max_connections;
extern int max_connections_to_host;
extern int max_tries;
extern int screen_width;
extern int dump_codepage;
extern int receive_timeout;
extern int unrestartable_receive_timeout;

extern struct document_setup dds;

extern int max_format_cache_entries;
extern int memory_cache_size;

extern struct rgb default_fg;
extern struct rgb default_bg;
extern struct rgb default_link;
extern struct rgb default_vlink;

extern unsigned char http_proxy[];
extern unsigned char ftp_proxy[];
extern unsigned char download_dir[];

struct http_bugs {
	int http10;
	int allow_blacklist;
	int bug_302_redirect;
	int bug_post_no_keepalive;
	int no_accept_charset;
};

extern struct http_bugs http_bugs;

struct ftp_options {
	unsigned char anon_pass[MAX_STR_LEN];
	int fast_ftp;
	int passive_ftp;
	int set_tos;
};

extern struct ftp_options ftp_options;

/* bookmarks.c */

/* Where all bookmarks are kept */
extern struct list_head bookmarks;

/* A pointer independent id that bookmarks can be identified by. Guarenteed to 
	be unique between all bookmarks */
typedef int bookmark_id;
extern bookmark_id next_bookmark_id;
#define		BAD_BOOKMARK_ID		(bookmark_id)(-1)


/* Stores display information about a box. Kept in cdata. */
struct dlg_data_item_data_box {
	int sel;	/* Item currently selected */	
	int box_top;	/* Index into items of the item that is on the top line of the box */
	struct list_head items;	/* The list being displayed */
	int list_len;	/* Number of items in the list */
};

/* An item in a box */
struct box_item {
	struct box_item *next;
	struct box_item *prev;
	unsigned char *text;	/* Text to display */
	void *data;	/* data */
};


void show_dlg_item_box(struct dialog_data *, struct dialog_item_data *); 

#define BOX_HILIGHT_FUNC (void (*)(struct terminal *, struct dlg_data_item_data_box *, struct box_item *))
#define BOX_ON_SELECTED_FUNC (int (*)(struct terminal *, struct dlg_data_item_data_box *, struct box_item *))

/* Ops dealing with box data */
/* */
#define get_box_from_dlg_item_data(x) ((struct dlg_data_item_data_box *)(x->cdata))
#define get_box_list_height(x) (x->data_len)

/* V.05 bookmarks: */
/*
struct bookmark {
	struct bookmark *next;
	struct bookmark *prev;
	bookmark_id id;	Bookmark id 
	unsigned char *title;	 title of bookmark 
	unsigned char *url;	 Location of bookmarked item 
};*/

struct bookmark {
	struct bookmark *next;
	struct bookmark *prev;
	bookmark_id id;	/* Bookmark id */
	unsigned char *title;	/* title of bookmark */
	unsigned char *url;	/* Location of bookmarked item */
	unsigned int type; /* The type of bookmark we're dealing with. */
	void *type_data; /* Type-dependent data. */
};

void finalize_bookmarks();

void read_bookmarks();
void write_bookmarks();

void bookmark_menu(struct terminal *, void *, struct session *);

/* Launches bookmark manager */
void menu_bookmark_manager(struct terminal *, void *, struct session *);

void add_bookmark(const unsigned char *, const unsigned char *);

struct bookmark *create_bookmark(const unsigned char *, const unsigned char *);

/* Launches add dialogs */
void launch_bm_add_link_dialog(struct terminal *,struct dialog_data *,struct session *);
void launch_bm_add_doc_dialog(struct terminal *,struct dialog_data *,struct session *);

/* kbdbind.c */

#define KM_MAIN		0
#define KM_EDIT		1
#define KM_MENU		2
#define KM_MAX		3

enum {
	ACT_ADD_BOOKMARK,
	ACT_AUTO_COMPLETE,
	ACT_BACK,
	ACT_BACKSPACE,
	ACT_BOOKMARK_MANAGER,
	ACT_COPY_CLIPBOARD,
	ACT_CUT_CLIPBOARD,
	ACT_DELETE,
	ACT_DOCUMENT_INFO,
	ACT_DOWN,
	ACT_DOWNLOAD,
	ACT_END,
	ACT_ENTER,
	ACT_FILE_MENU,
	ACT_FIND_NEXT,
	ACT_FIND_NEXT_BACK,
	ACT_GOTO_URL,
	ACT_GOTO_URL_CURRENT,
	ACT_GOTO_URL_CURRENT_LINK,
	ACT_HEADER_INFO,
	ACT_HOME,
	ACT_KILL_TO_BOL,
	ACT_KILL_TO_EOL,
	ACT_LEFT,
	ACT_MENU,
	ACT_NEXT_FRAME,
	ACT_OPEN_NEW_WINDOW,
	ACT_OPEN_LINK_IN_NEW_WINDOW,
	ACT_PAGE_DOWN,
	ACT_PAGE_UP,
	ACT_PASTE_CLIPBOARD,
	ACT_PREVIOUS_FRAME,
	ACT_REALLYQUIT,
	ACT_QUIT,
	ACT_RELOAD,
	ACT_RIGHT,
	ACT_SCROLL_DOWN,
	ACT_SCROLL_LEFT,
	ACT_SCROLL_RIGHT,
	ACT_SCROLL_UP,
	ACT_SEARCH,
	ACT_SEARCH_BACK,
	ACT_TOGGLE_DISPLAY_IMAGES,
	ACT_TOGGLE_DISPLAY_TABLES,
	ACT_TOGGLE_HTML_PLAIN,
	ACT_UP,
	ACT_VIEW_IMAGE,
	ACT_ZOOM_FRAME
};

void init_keymaps();
void free_keymaps();
int kbd_action(int, struct event *);
unsigned char *bind_rd(struct option *, unsigned char *);
unsigned char *unbind_rd(struct option *, unsigned char *);
