/* atmd.h - Functions useful for demons (and some other ATM tools) */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */
 

#ifndef _ATMD_H
#define _ATMD_H

/*--------------------------- Common definitions ----------------------------*/

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* doubly linked list primitives */
 
#define Q_INSERT_HEAD(r,i) ({ (i)->next = r; (i)->prev = NULL; \
  if (r) (r)->prev = i; r = i; })
#define Q_INSERT_AFTER(r,i,a) ({ if (a) { (i)->next = (a)->next; \
  (i)->prev = a; if ((a)->next) (a)->next->prev = i; (a)->next = i; } \
  else { (i)->next = r; (i)->prev = NULL; if (r) (r)->prev = i; r = i; } })
#define Q_INSERT_BEFORE(r,i,b) ({ if (b) { (i)->next = b; \
  (i)->prev = (b)->prev; if ((b)->prev) (b)->prev->next = i; else r = i; \
  (b)->prev = i; } else { (i)->next = r; (i)->prev = NULL; \
  if (r) (r)->prev = i; r = i; } })
#define Q_REMOVE(r,i) ({ if ((i)->next) (i)->next->prev = (i)->prev; \
  if ((i)->prev) (i)->prev->next = (i)->next; else r = (i)->next; })


extern struct timeval now;
extern int debug;


#define alloc_t(t) ((t *) alloc(sizeof(t)))


void *alloc(size_t size);

uint32_t read_netl(void *p);


/*--------------------------- Diagnostic messages ---------------------------*/


#include <stdarg.h>


#define DIAG_DEBUG 	3
#define DIAG_INFO	2
#define DIAG_WARN	1
#define DIAG_ERROR	0
#define DIAG_FATAL	-1


void set_application(const char *name);

void set_logfile(const char *name);

FILE *get_logfile(void);

void set_verbosity(const char *component,int level);

int get_verbosity(const char *component);

void vdiag(const char *component,int severity,const char *fmt,va_list ap);
void diag(const char *component,int severity,const char *fmt,...);
void diag_dump(const char *component,int severity,const char *title,
  const unsigned char *data,int len);


/*------------------------------ Timer support ------------------------------*/


#include <sys/time.h>


typedef struct _timer {
    struct timeval expiration;
    void (*callback)(void *user);
    void *user;
    struct _timer *prev,*next;
} TIMER;

TIMER *start_timer(long usec,void (*callback)(void *user),void *user);
void stop_timer(TIMER *timer);
void (*timer_handler(TIMER *timer))(void *user);
struct timeval *next_timer(void);
void pop_timer(TIMER *timer);
void expire_timers(void);


/*--------------------------- Unix domain sockets ---------------------------*/


#include <sys/socket.h>
#include <sys/un.h>


typedef struct {
    int	s;			/* socket */
    struct sockaddr_un addr;	/* reply address */
    int size;			/* address size */
} UN_CTX;


int un_create(const char *path,mode_t mode);

/*
 * Creates a Unix domain DGRAM socket, binds it to the specified path, and
 * returns the socket descriptor. Returns a negative value on error.
 */

int un_attach(const char *path);

/*
 * Creates a Unix domain DGRAM socket and connects it to the specified path.
 * The local side is bound to an ephemeral address. Returns the socket
 * descriptor on success, a negative value otherwise.
 */

int un_recv_connect(int s,void *buf,int size);

/*
 * Performs a recv(s,buf,size,0) and connects the socket to the sender's
 * address. Returns a negative value on error.
 */

int un_reply(int s,void *buf,int size,
  int (*handler)(void *buf,int len,void *user),void *user);

/*
 * Receives a message from the socket into the buffer provided by the caller,
 * invokes handler for processing and optionally sends back a reply. If the
 * handler returns a negative value or zero, no reply is sent. If the handler
 * returns a positive value, this is interpreted as the length of the reply to
 * send. The data is taken from the buffer. If any system call fails, un_reply
 * returns a negative value. Otherwise, it returns whatever was returned by the
 * handler function.
 */

int un_recv(UN_CTX *ctx,int s,void *buf,int size);

/*
 * Receive a message into the specified buffer and store the information needed
 * to send a reply in ctx. Sets errno and returns a negative value on error.
 */

int un_send(const UN_CTX *ctx,void *buf,int len);

/*
 * Send a reply to the sender identified by ctx. Sets errno and returns a
 * negative value on error.
 */


/* ------------------------- IP address operations ------------------------- */


#include <netinet/in.h>


#define T2I_NAME	1	/* do a name lookup */
#define T2I_ERROR	2	/* print error messages */

uint32_t text2ip(const char *text,const char *component,int flags);

/*
 * Converts a text string to an IP address. If resolution fails, text2ip
 * returns INADDR_NONE. If T2I_ERROR is set, errors messages are printed. If
 * component is non-NULL, they are logged using diag(). Otherwise, they are
 * printed on standard error. Note that T2I_NAME uses gethostbyname() which
 * may attempt resolution via DNS or NIS.
 */


/* ------------------------ Kernel pointer handles ------------------------- */


#include <atm.h>


#define KPRT_PRINT_BUFS 4	/* up to that many buffers are concurrently
				   available */

int kptr_eq(const atm_kptr_t *a,const atm_kptr_t *b);

/*
 * Returns 1 if A and B are equal, 0 otherwise. Note that unused areas of the
 * handles must be initialized to a system-wide constant pattern, e.g. 0.
 */

const char *kptr_print(const atm_kptr_t *p);

/*
 * Returns a pointer to a static buffer containing an ASCII representation of
 * a kernel pointer handle. After KPRT_PRINT_BUFS calls to kptr_print, old
 * buffers are reused.
 */

#ifdef __cplusplus
};
#endif

#endif
