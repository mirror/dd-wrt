/*
 *	BIRD -- Declarations Common to Unix Port
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_UNIX_H_
#define _BIRD_UNIX_H_

#include <sys/socket.h>

struct pool;
struct iface;
struct birdsock;

/* main.c */

extern char *bird_name;
void async_config(void);
void async_dump(void);
void async_shutdown(void);
void cmd_check_config(char *name);
void cmd_reconfig(char *name, int type, int timeout);
void cmd_reconfig_confirm(void);
void cmd_reconfig_undo(void);
void cmd_shutdown(void);

#define UNIX_DEFAULT_CONFIGURE_TIMEOUT 300


/* io.c */

#define ERR(c) do { s->err = c; return -1; } while (0)
#define ERR2(c) do { s->err = c; goto err; } while (0)
#define ERR_MSG(c) do { errno = 0; s->err = c; return -1; } while (0)


#define SOCKADDR_SIZE 32

typedef struct sockaddr_bird {
  struct sockaddr sa;
  char padding[SOCKADDR_SIZE - sizeof(struct sockaddr)];
} sockaddr;


#ifdef IPV6
#define BIRD_AF AF_INET6
#define _MI6(x1,x2,x3,x4) _MI(x1, x2, x3, x4)
#define ipa_is_link_local(x) ipa_has_link_scope(x)
#define ipa_from_sa(x) ipa_from_sa6(x)
#define ipa_from_u32(x) _MI6(0,0,0xffff,x)
#define ipa_to_u32(x) _I3(x)
#else
#define BIRD_AF AF_INET
#define _I0(X) 0
#define _I1(X) 0
#define _I2(X) 0
#define _I3(X) 0
#define _MI6(x1,x2,x3,x4) IPA_NONE
#define ipa_is_link_local(x) 0
#define ipa_from_sa(x) ipa_from_sa4(x)
#endif


/* This is sloppy hack, it should be detected by configure script */
/* Linux systems have it defined so this is definition for BSD systems */
#ifndef s6_addr32
#define s6_addr32 __u6_addr.__u6_addr32
#endif


static inline ip_addr ipa_from_in4(struct in_addr a)
{ return ipa_from_u32(ntohl(a.s_addr)); }

static inline ip_addr ipa_from_in6(struct in6_addr a)
{ return _MI6(ntohl(a.s6_addr32[0]), ntohl(a.s6_addr32[1]), ntohl(a.s6_addr32[2]), ntohl(a.s6_addr32[3])); }

static inline ip_addr ipa_from_sa4(sockaddr *sa)
{ return ipa_from_in4(((struct sockaddr_in *) sa)->sin_addr); }

static inline ip_addr ipa_from_sa6(sockaddr *sa)
{ return ipa_from_in6(((struct sockaddr_in6 *) sa)->sin6_addr); }

static inline struct in_addr ipa_to_in4(ip_addr a)
{ return (struct in_addr) { htonl(ipa_to_u32(a)) }; }

static inline struct in6_addr ipa_to_in6(ip_addr a)
{ return (struct in6_addr) { .s6_addr32 = { htonl(_I0(a)), htonl(_I1(a)), htonl(_I2(a)), htonl(_I3(a)) } }; }

void sockaddr_fill(sockaddr *sa, int af, ip_addr a, struct iface *ifa, uint port);
int sockaddr_read(sockaddr *sa, int af, ip_addr *a, struct iface **ifa, uint *port);


#ifndef SUN_LEN
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) + strlen ((ptr)->sun_path))
#endif

volatile int async_config_flag;
volatile int async_dump_flag;
volatile int async_shutdown_flag;

void io_init(void);
void io_loop(void);
int sk_open_unix(struct birdsock *s, char *name);
void *tracked_fopen(struct pool *, char *name, char *mode);
void test_old_bird(char *path);


/* krt.c bits */

void krt_io_init(void);

/* log.c */
void main_thread_init(void);
#ifdef NEED_PRINTF
void log_init_debug(char *);		/* Initialize debug dump to given file (NULL=stderr, ""=off) */
void log_switch(int debug, list *l, char *); /* Use l=NULL for initial switch */
#else
#define log_init_debug(d) do {} while(0)
#define log_switch(debug, l, c) do {} while(0)
#endif
struct log_config {
  node n;
  unsigned int mask;			/* Classes to log */
  void *fh;				/* FILE to log to, NULL=syslog */
  int terminal_flag;
};

#endif
