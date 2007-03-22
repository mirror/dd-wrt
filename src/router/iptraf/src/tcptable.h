/***

   tcptable.h   -- table manipulation for the statistics display.
   Written by Gerard Paul Java
   Copyright (c) Gerard Paul Java 1997-2005
   
***/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <curses.h>
#include <panel.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_fddi.h>
#include <linux/if_tr.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include "servname.h"
#include "tcphdr.h"
#include "links.h"

#define max(a, b) (( a > b) ? a : b)

#define FLAG_SYN	1
#define FLAG_RST	2
#define FLAG_PSH	4
#define FLAG_ACK	8
#define FLAG_URG	16

#define CLOSED		64

#define ENTRIES_IN_HASH_TABLE	1543

struct tcptableent {
    struct in_addr saddr;
    struct in_addr daddr;
    char s_fqdn[45];            /* fully-qualified domain names */
    char d_fqdn[45];
    int s_fstat;
    int d_fstat;
    unsigned int sport;
    unsigned int dport;
    char smacaddr[15];
    char s_sname[11];           /* Service names, maxlen=10 */
    char d_sname[11];
    unsigned int protocol;
    unsigned long pcount;       /* packet count */
    unsigned long bcount;       /* byte count */
    unsigned int stat;          /* TCP flags */
    unsigned int win;
    unsigned int psize;
    unsigned long finack;
    int partial;
    int finsent;
    char ifname[8];
    unsigned int index;
    int reused;
    int timedout;
    int inclosed;
    int half_bracket;
    unsigned long spanbr;
    time_t lastupdate;
    time_t starttime;
    time_t conn_starttime;
    struct tcp_hashentry *hash_node;
    struct tcptableent *oth_connection; /* the other half of the connection */
    struct tcptableent *prev_entry;
    struct tcptableent *next_entry;
};

struct closedlist {
    struct tcptableent *closedentry;
    struct tcptableent *pair;
    struct closedlist *next_entry;
};

struct tcp_hashentry {
    unsigned int index;
    unsigned int hp;            /* index position in bucket array */
    struct tcptableent *tcpnode;
    struct tcp_hashentry *prev_entry;
    struct tcp_hashentry *next_entry;
};

struct tcptable {
    struct tcp_hashentry *hash_table[ENTRIES_IN_HASH_TABLE];
    struct tcp_hashentry *hash_tails[ENTRIES_IN_HASH_TABLE];
    struct tcptableent *head;
    struct tcptableent *tail;
    struct closedlist *closedentries;
    struct closedlist *closedtail;
    struct tcptableent *firstvisible;
    struct tcptableent *lastvisible;
    struct tcptableent *barptr;
    unsigned int baridx;
    unsigned int lastpos;
    unsigned int count;
    unsigned int bmaxy;         /* number of lines of the border window */
    unsigned int imaxy;         /* number of lines inside the border */
    WINDOW *tcpscreen;
    PANEL *tcppanel;
    WINDOW *borderwin;
    PANEL *borderpanel;
};

void init_tcp_table(struct tcptable *table);

struct tcptableent *addentry(struct tcptable *table,
                             unsigned long int saddr,
                             unsigned long int daddr,
                             unsigned int sport, unsigned int dport,
                             int protocol,
                             char *ifname, int *rev_lookup, int rvnamedon,
                             int servnames, int *nomem);

struct tcptableent *in_table(struct tcptable *table,
                             unsigned long saddr, unsigned long daddr,
                             unsigned int sport, unsigned int dport,
                             char *ifname,
                             int logging, FILE * logfile, int *nomem,
                             struct OPTIONS *opts);

void updateentry(struct tcptable *table, struct tcptableent *tableentry,
                 struct tcphdr *transpacket, char *packet, int linkproto,
                 unsigned long packetlength,
                 unsigned int bcount, unsigned int fragofs, int logging,
                 int *revlook, int rvnfd,
                 struct OPTIONS *opts, FILE * logfile, int *nomem);

void addtoclosedlist(struct tcptable *table,
                     struct tcptableent *tableentry, int *nomem);

void clearaddr(struct tcptable *table, struct tcptableent *tableentry,
               unsigned int screen_idx);

void printentry(struct tcptable *table, struct tcptableent *tableentry,
                unsigned int screen_idx, int mode);

void refreshtcpwin(struct tcptable *table, unsigned int idx, int mode);

void destroytcptable(struct tcptable *table);

void flushclosedentries(struct tcptable *table, unsigned long *screen_idx,
                        int logging, FILE * logfile, struct OPTIONS *opts);

void write_timeout_log(int logging, FILE * logfile,
                       struct tcptableent *tcpnode, struct OPTIONS *opts);
