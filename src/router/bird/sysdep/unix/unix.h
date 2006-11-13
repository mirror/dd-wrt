/*
 *	BIRD -- Declarations Common to Unix Port
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_UNIX_H_
#define _BIRD_UNIX_H_

struct pool;

/* main.c */

void async_config(void);
void async_dump(void);
void async_shutdown(void);
void cmd_reconfig(char *name);

/* io.c */

extern int async_config_flag;
extern int async_dump_flag;
extern int async_shutdown_flag;

#ifdef IPV6
#define BIRD_PF PF_INET6
#define BIRD_AF AF_INET6
typedef struct sockaddr_in6 sockaddr;
#else
#define BIRD_PF PF_INET
#define BIRD_AF AF_INET
typedef struct sockaddr_in sockaddr;
#endif

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) + strlen ((ptr)->sun_path))
#endif

struct birdsock;

void io_init(void);
void io_loop(void);
void fill_in_sockaddr(sockaddr *sa, ip_addr a, unsigned port);
void get_sockaddr(sockaddr *sa, ip_addr *a, unsigned *port, int check);
int sk_open_unix(struct birdsock *s, char *name);
void *tracked_fopen(struct pool *, char *name, char *mode);
void test_old_bird(char *path);


/* krt.c bits */

void krt_io_init(void);

/* log.c */

void log_init(int debug, int init);
void log_init_debug(char *);		/* Initialize debug dump to given file (NULL=stderr, ""=off) */
void log_switch(int debug, struct list *);

struct log_config {
  node n;
  unsigned int mask;			/* Classes to log */
  void *fh;				/* FILE to log to, NULL=syslog */
  int terminal_flag;
};

#endif
