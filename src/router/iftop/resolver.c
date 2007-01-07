/*
 * resolver.c:
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "ns_hash.h"
#include "iftop.h"

#include "threadprof.h"

#include "options.h"


#define RESOLVE_QUEUE_LENGTH 20

struct in_addr resolve_queue[RESOLVE_QUEUE_LENGTH];

pthread_cond_t resolver_queue_cond;
pthread_mutex_t resolver_queue_mutex;

hash_type* ns_hash;

int head;
int tail;

extern options_t options;


/* 
 * We have a choice of resolver methods. Real computers have getnameinfo or
 * gethostbyaddr_r, which are reentrant and therefore thread safe. Other
 * machines don't, and so we can use non-reentrant gethostbyaddr and have only
 * one resolver thread.  Alternatively, we can use the MIT ares asynchronous
 * DNS library to do this.
 */

#if defined(USE_GETNAMEINFO)
/**
 * Implementation of do_resolve for platforms with getaddrinfo.
 *
 * This is a fairly sane function with a uniform interface which is even --
 * shock! -- standardised by POSIX and in RFC 2553. Unfortunately systems such
 * as NetBSD break the RFC and implement it in a non-thread-safe fashion, so
 * for the moment, the configure script won't try to use it.
 */
char *do_resolve(struct in_addr *addr) {
    struct sockaddr_in sin = {0};
    char buf[NI_MAXHOST]; /* 1025 */
    int res;
    sin.sin_family = AF_INET;
    sin.sin_addr = *addr;
    sin.sin_port = 0;

    if (getnameinfo((struct sockaddr*)&sin, sizeof sin, buf, sizeof buf, NULL, 0, NI_NAMEREQD) == 0)
        return xstrdup(buf);
    else
        return NULL;
}

#elif defined(USE_GETHOSTBYADDR_R)
/**
 * Implementation of do_resolve for platforms with working gethostbyaddr_r
 *
 * Some implementations of libc choose to implement gethostbyaddr_r as
 * a non thread-safe wrapper to gethostbyaddr.  An interesting choice...
 */
char* do_resolve(struct in_addr * addr) {
    struct hostent hostbuf, *hp;
    size_t hstbuflen = 1024;
    char *tmphstbuf;
    int res;
    int herr;
    char * ret = NULL;

    /* Allocate buffer, remember to free it to avoid memory leakage.  */            
    tmphstbuf = xmalloc (hstbuflen);

    /* Some machines have gethostbyaddr_r returning an integer error code; on
     * others, it returns a struct hostent*. */
#ifdef GETHOSTBYADDR_R_RETURNS_INT
    while ((res = gethostbyaddr_r((char*)addr, sizeof(struct in_addr), AF_INET,
                                  &hostbuf, tmphstbuf, hstbuflen,
                                  &hp, &herr)) == ERANGE)
#else
    /* ... also assume one fewer argument.... */
    while ((hp = gethostbyaddr_r((char*)addr, sizeof(struct in_addr), AF_INET,
                           &hostbuf, tmphstbuf, hstbuflen, &herr)) == NULL
            && errno == ERANGE)
#endif
            {
        
        /* Enlarge the buffer.  */
        hstbuflen *= 2;
        tmphstbuf = realloc (tmphstbuf, hstbuflen);
      }

    /*  Check for errors.  */
    if (res || hp == NULL) {
        /* failed */
        /* Leave the unresolved IP in the hash */
    }
    else {
        ret = xstrdup(hp->h_name);

    }
    xfree(tmphstbuf);
    return ret;
}

#elif defined(USE_GETHOSTBYADDR)

/**
 * Implementation using gethostbyname. Since this is nonreentrant, we have to
 * wrap it in a mutex, losing all benefit of multithreaded resolution.
 */
char *do_resolve(struct in_addr *addr) {
    static pthread_mutex_t ghba_mtx = PTHREAD_MUTEX_INITIALIZER;
    char *s = NULL;
    struct hostent *he;
    pthread_mutex_lock(&ghba_mtx);
    he = gethostbyaddr((char*)addr, sizeof *addr, AF_INET);
    if (he)
        s = xstrdup(he->h_name);
    pthread_mutex_unlock(&ghba_mtx);
    return s;
}


#elif defined(USE_LIBRESOLV)

#include <arpa/nameser.h>
#include <resolv.h>

/**
 * libresolv implementation 
 * resolver functions may not be thread safe
 */
char* do_resolve(struct in_addr * addr) {
  char msg[PACKETSZ];
  char s[35];
  int l;
  unsigned char* a;
  char * ret = NULL;

  a = (unsigned char*)addr;

  snprintf(s, 35, "%d.%d.%d.%d.in-addr.arpa.",a[3], a[2], a[1], a[0]);

  l = res_search(s, C_IN, T_PTR, msg, PACKETSZ);
  if(l != -1) {
    ns_msg nsmsg;
    ns_rr rr;
    if(ns_initparse(msg, l, &nsmsg) != -1) {
      int c;
      int i;
      c = ns_msg_count(nsmsg, ns_s_an);
      for(i = 0; i < c; i++) {
        if(ns_parserr(&nsmsg, ns_s_an, i, &rr) == 0){
          if(ns_rr_type(rr) == T_PTR) {
            char buf[256];
            ns_name_uncompress(msg, msg + l, ns_rr_rdata(rr), buf, 256);
            ret = xstrdup(buf);
          }
        }
      }
    }
  }
  return ret;
}

#elif defined(USE_ARES)

/**
 * ares implementation
 */

#include <sys/time.h>
#include <ares.h>
#include <arpa/nameser.h>

/* callback function for ares */
struct ares_callback_comm {
    struct in_addr *addr;
    int result;
    char *name;
};

static void do_resolve_ares_callback(void *arg, int status, unsigned char *abuf, int alen) {
    struct hostent *he;
    struct ares_callback_comm *C;
    C = (struct ares_callback_comm*)arg;

    if (status == ARES_SUCCESS) {
        C->result = 1;
        ares_parse_ptr_reply(abuf, alen, C->addr, sizeof *C->addr, AF_INET, &he);
        C->name = xstrdup(he->h_name);;
        ares_free_hostent(he);
    } else {
        C->result = -1;
    }
}

char *do_resolve(struct in_addr * addr) {
    struct ares_callback_comm C;
    char s[35];
    unsigned char *a;
    ares_channel *chan;
    static pthread_mutex_t ares_init_mtx = PTHREAD_MUTEX_INITIALIZER;
    static pthread_key_t ares_key;
    static int gotkey;

    /* Make sure we have an ARES channel for this thread. */
    pthread_mutex_lock(&ares_init_mtx);
    if (!gotkey) {
        pthread_key_create(&ares_key, NULL);
        gotkey = 1;
        
    }
    pthread_mutex_unlock(&ares_init_mtx);
    
    chan = pthread_getspecific(ares_key);
    if (!chan) {
        chan = xmalloc(sizeof *chan);
        pthread_setspecific(ares_key, chan);
        if (ares_init(chan) != ARES_SUCCESS) return NULL;
    }
    
    a = (unsigned char*)addr;
    sprintf(s, "%d.%d.%d.%d.in-addr.arpa.", a[3], a[2], a[1], a[0]);
    
    C.result = 0;
    C.addr = addr;
    ares_query(*chan, s, C_IN, T_PTR, do_resolve_ares_callback, &C);
    while (C.result == 0) {
        int n;
        fd_set readfds, writefds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        n = ares_fds(*chan, &readfds, &writefds);
        ares_timeout(*chan, NULL, &tv);
        select(n, &readfds, &writefds, NULL, &tv);
        ares_process(*chan, &readfds, &writefds);
    }

    /* At this stage, the query should be complete. */
    switch (C.result) {
        case -1:
        case 0:     /* shouldn't happen */
            return NULL;

        default:
            return C.name;
    }
}

#elif defined(USE_FORKING_RESOLVER)

/**
 * Resolver which forks a process, then uses gethostbyname.
 */

#include <signal.h>

#define NAMESIZE        64

int forking_resolver_worker(int fd) {
    while (1) {
        struct in_addr a;
        struct hostent *he;
        char buf[NAMESIZE] = {0};
        if (read(fd, &a, sizeof a) != sizeof a)
            return -1;

        he = gethostbyaddr((char*)&a, sizeof a, AF_INET);
        if (he)
            strncpy(buf, he->h_name, NAMESIZE - 1);

        if (write(fd, buf, NAMESIZE) != NAMESIZE)
            return -1;
    }
}

char *do_resolve(struct in_addr *addr) {
    struct {
        int fd;
        pid_t child;
    } *workerinfo;
    char name[NAMESIZE];
    static pthread_mutex_t worker_init_mtx = PTHREAD_MUTEX_INITIALIZER;
    static pthread_key_t worker_key;
    static int gotkey;

    /* If no process exists, we need to spawn one. */
    pthread_mutex_lock(&worker_init_mtx);
    if (!gotkey) {
        pthread_key_create(&worker_key, NULL);
        gotkey = 1;
    }
    pthread_mutex_unlock(&worker_init_mtx);
    
    workerinfo = pthread_getspecific(worker_key);
    if (!workerinfo) {
        int p[2];

        if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, p) == -1)
            return NULL;

        workerinfo = xmalloc(sizeof *workerinfo);
        pthread_setspecific(worker_key, workerinfo);
        workerinfo->fd = p[0];
        
        switch (workerinfo->child = fork()) {
            case 0:
                close(p[0]);
                _exit(forking_resolver_worker(p[1]));

            case -1:
                close(p[0]);
                close(p[1]);
                return NULL;

            default:
                close(p[1]);
        }
    }

    /* Now have a worker to which we can write requests. */
    if (write(workerinfo->fd, addr, sizeof *addr) != sizeof *addr
        || read(workerinfo->fd, name, NAMESIZE) != NAMESIZE) {
        /* Something went wrong. Just kill the child and get on with it. */
        kill(workerinfo->child, SIGKILL);
        wait(NULL);
        close(workerinfo->fd);
        xfree(workerinfo);
        pthread_setspecific(worker_key, NULL);
        *name = 0;
    }
    if (!*name)
        return NULL;
    else
        return xstrdup(name);
}

#else

#   warning No name resolution method specified; name resolution will not work

char *do_resolve(struct in_addr *addr) {
    return NULL;
}

#endif

void resolver_worker(void* ptr) {
/*    int thread_number = *(int*)ptr;*/
    pthread_mutex_lock(&resolver_queue_mutex);
    sethostent(1);
    while(1) {
        /* Wait until we are told that an address has been added to the 
         * queue. */
        pthread_cond_wait(&resolver_queue_cond, &resolver_queue_mutex);

        /* Keep resolving until the queue is empty */
        while(head != tail) {
            char * hostname;
            struct in_addr addr = resolve_queue[tail];

            /* mutex always locked at this point */

            tail = (tail + 1) % RESOLVE_QUEUE_LENGTH;

            pthread_mutex_unlock(&resolver_queue_mutex);

            hostname = do_resolve(&addr);

            /*
             * Store the result in ns_hash
             */
            pthread_mutex_lock(&resolver_queue_mutex);

            if(hostname != NULL) {
                char* old;
		union {
		    char **ch_pp;
		    void **void_pp;
		} u_old = { &old };
                if(hash_find(ns_hash, &addr, u_old.void_pp) == HASH_STATUS_OK) {
                    hash_delete(ns_hash, &addr);
                    xfree(old);
                }
                hash_insert(ns_hash, &addr, (void*)hostname);
            }

        }
    }
}

void resolver_initialise() {
    int* n;
    int i;
    pthread_t thread;
    head = tail = 0;

    ns_hash = ns_hash_create();
    
    pthread_mutex_init(&resolver_queue_mutex, NULL);
    pthread_cond_init(&resolver_queue_cond, NULL);

    for(i = 0; i < 2; i++) {
        n = (int*)xmalloc(sizeof *n);
        *n = i;
        pthread_create(&thread, NULL, (void*)&resolver_worker, (void*)n);
    }

}

void resolve(struct in_addr* addr, char* result, int buflen) {
    char* hostname;
    union {
	char **ch_pp;
	void **void_pp;
    } u_hostname = { &hostname };
    int added = 0;

    if(options.dnsresolution == 1) {

        pthread_mutex_lock(&resolver_queue_mutex);

        if(hash_find(ns_hash, addr, u_hostname.void_pp) == HASH_STATUS_OK) {
            /* Found => already resolved, or on the queue */
        }
        else {
            hostname = strdup(inet_ntoa(*addr));
            hash_insert(ns_hash, addr, hostname);

            if(((head + 1) % RESOLVE_QUEUE_LENGTH) == tail) {
                /* queue full */
            }
            else {
                resolve_queue[head] = *addr;
                head = (head + 1) % RESOLVE_QUEUE_LENGTH;
                added = 1;
            }
        }
        pthread_mutex_unlock(&resolver_queue_mutex);

        if(added == 1) {
            pthread_cond_signal(&resolver_queue_cond);
        }

        if(result != NULL && buflen > 1) {
            strncpy(result, hostname, buflen - 1);
            result[buflen - 1] = '\0';
        }
    }
}
