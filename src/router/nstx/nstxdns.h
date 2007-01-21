#ifndef _NSTXDNS_H
#define _NSTXDNS_H

#define DNS_QUERY    0x01
#define DNS_RESPONSE 0x02

#define DNS_MAXPKT 512

struct rr
{
   char *data;
   int len;
   int link;
   
   struct rr *next;
};

struct dnspkt
{
   unsigned short id;
   int type;
   struct rr *query;
   struct rr *answer;
};

void dns_setsuffix (char *);

struct dnspkt *dns_alloc (void);
void dns_free (struct dnspkt *);

void dns_setid (struct dnspkt *, unsigned short);
void dns_settype (struct dnspkt *, int);
int dns_addquery (struct dnspkt *, const char *);
int dns_addanswer (struct dnspkt *, const char *, int, int);

int dns_getpktsize (const struct dnspkt *);
struct dnspkt *dns_extractpkt (const unsigned char *, int);
const char *dns_getquerydata (struct dnspkt *);
char *dns_getanswerdata (struct dnspkt *, int *);

const char *dns_fqdn2data (const char *);
const char *dns_data2fqdn (const char *);

const unsigned char *txt2data (const unsigned char *, int *);
unsigned char *dns_constructpacket (struct dnspkt *, int *);

int dns_getfreespace (const struct dnspkt *, int);

#endif /* _NSTXDNS_H */
