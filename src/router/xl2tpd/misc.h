/*
 * Layer Two Tunnelling Protocol Daemon
 * Copyright (C) 1998 Adtran, Inc.
 * Copyright (C) 2002 Jeff McAdams
 *
 * Mark Spencer
 *
 * This software is distributed under the terms
 * of the GPL, which you should have received
 * along with this source.
 *
 * Misc stuff...
 */

#ifndef _MISC_H
#define _MISC_H

#include <syslog.h>

struct tunnel;
struct buffer
{
    int type;
    void *rstart;
    void *rend;
    void *start;
    int len;
    int maxlen;
#if 0
    unsigned int addr;
    int port;
#else
    struct sockaddr_in peer;
#endif
    struct tunnel *tunnel;      /* Who owns this packet, if it's a control */
    int retries;                /* Again, if a control packet, how many retries? */
};

struct ppp_opts
{
    char option[MAXSTRLEN];
    struct ppp_opts *next;
};

#define IPADDY(a) inet_ntoa(*((struct in_addr *)&(a)))

#ifdef NEED_PRINTF
#define DEBUG c ? c->debug || t->debug : t->debug
#else
#define DEBUG 0
#endif

#ifdef USE_SWAPS_INSTEAD
#define SWAPS(a) ((((a) & 0xFF) << 8 ) | (((a) >> 8) & 0xFF))
#ifdef htons
#undef htons
#endif
#ifdef ntohs
#undef htons
#endif
#define htons(a) SWAPS(a)
#define ntohs(a) SWAPS(a)
#endif

#define halt() printf("Halted.\n") ; for(;;)

extern char hostname[];

#ifdef NEED_PRINTF
extern void l2tp_log (int level, const char *fmt, ...);
#else
#define l2tp_log(level,fmt,...) while(0) {}
#endif
static inline void swaps (void *buf_v, int len)
{
#ifdef __alpha
    /* Reverse byte order alpha is little endian so lest save a step.
       to make things work out easier */
    int x;
    unsigned char t1;
    unsigned char *tmp = (_u16 *) buf_v;
    for (x = 0; x < len; x += 2)
    {
        t1 = tmp[x];
        tmp[x] = tmp[x + 1];
        tmp[x + 1] = t1;
    }
#else

    /* Reverse byte order (if proper to do so) 
       to make things work out easier */
    int x;
	struct hw { unsigned short s; } __attribute__ ((packed)) *p = (struct hw *) buf_v;
	for (x = 0; x < len / 2; x++, p++)
		p->s = ntohs(p->s); 
#endif
}



extern struct buffer *new_buf (int);
extern void udppush_handler (int);
extern int addfcs (struct buffer *buf);
extern void do_packet_dump (struct buffer *);
extern void status (const char *fmt, ...);
extern void status_handler (int signal);
extern int getPtyMaster(char *, int);
extern void do_control (void);
extern void recycle_buf (struct buffer *);
extern void safe_copy (char *, char *, int);
extern void opt_destroy (struct ppp_opts *);
extern struct ppp_opts *add_opt (struct ppp_opts *, char *, ...);
extern void process_signal (void);
#endif
