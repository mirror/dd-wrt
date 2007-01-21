#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <assert.h>

#include "nstxfun.h"
#include "nstxdns.h"

/* lbl2str: Decodes a domainname to a string
 * <len1><NAME1><len2><NAME2><len3><NAME3>'\0' ->
 * NAME1.NAME2.NAME3                                
 * A lenght-byte may not be larger than 63 bytes (the two most significant
 * bits are reserved/used for compression */

/* Reverses lbl2str */

static const char *
str2lbl(const char *data)
{
   const char *ptr;
   char *ptr2, *buf;
   unsigned int buflen, chunklen;
   
   ptr = data;
   buf = NULL;
   buflen = 0;
   
   while ((ptr2 = strchr(ptr, '.'))) {
      chunklen = ptr2 - ptr;
      if ((chunklen > 63) || (chunklen <= 0)) {
	 DEBUG("Too long or zero-length label");
	 if (buf)
	   free(buf);
	 return NULL;
      }
      buf = realloc(buf, buflen + chunklen + 1);
      buf[buflen] = chunklen;
      memcpy(buf+buflen+1, ptr, chunklen);
      buflen += chunklen + 1;
      ptr = ptr2 + 1;
   }
   chunklen = strlen(ptr);
   buf = realloc(buf, buflen + chunklen + 2);
   buf[buflen] = chunklen;
   memcpy(buf+buflen+1, ptr, chunklen);
   buflen += chunklen+1;
   buf[buflen] = '\0';
   buflen++;
   
   return buf;
}

/* decompress_label decompresses the label pointed to by 'lbl' inside the
 * DNS-packet 'msg'. */

static char *
decompress_label(const char *msg, unsigned int msglen, const char *lbl)
{
   const char *ptr = lbl;
   char *buf;
   unsigned int chunklen, offset, buflen, followed = 0;
   
   buf = NULL;
   buflen = 0;
   
   while ((chunklen = *ptr)) {
      if (chunklen > 63) {
	 if ((ptr-msg) >= ((signed int)msglen-1)) {
	    DEBUG("Bad pointer at end of msg");
	    if (buf)
	      free(buf);
	    return NULL;
	 }	    
	 if (followed > 20) {
	    DEBUG("Too many pointer-loops");
	    if (buf)
	      free(buf);
	    return NULL;
	 }
	 offset = (chunklen % 64)*256 + *(ptr+1);
	 if (offset >= msglen) {
	    DEBUG("offset behind message");
	    if (buf)
	      free(buf);
	    return NULL;
	 }
	 ptr = msg + offset;
	 followed++;
      }
      else {
	 buf = realloc(buf, buflen + chunklen + 2);
	 if ((ptr + chunklen + 1) >= (msg + msglen)) {
	    DEBUG("Invalid chunklen");
	    if (buf)
	      free(buf);
	    return NULL;
	 }
	 memcpy(buf+buflen, ptr, chunklen + 1);
	 buflen += chunklen + 1;
	 ptr += chunklen + 1;
      }
   }
   if (buf) {
     buf[buflen] = 0;
     buflen++;
   }
   return buf;
}

static const unsigned char *
_cstringify(const unsigned char *data, int *dlen, unsigned int clen)
{
   static unsigned char *buf;
   
   const unsigned char *s = data;
   unsigned char *d;
   unsigned int llen, len;
   
   len = *dlen;
   *dlen = 0;
   
   d = buf = realloc(buf, len+len/clen+2);
   
   while (len > 0) {
      llen = (len > clen) ? clen : len;
      *(d++) = llen;
      memcpy(d, s, llen);
      d += llen;
      (*dlen) += llen + 1;
      s += llen;
      len -= llen;
   }
   *d = '\0';
   (*dlen)++;
   
   return buf;
}

static const unsigned char *
data2lbl (const unsigned char *data)
{
   int len;
   
   len = strlen((char*)data);
   return _cstringify(data, &len, 63);
}

const unsigned char *
data2txt (const unsigned char *data, int *len)
{
   return _cstringify(data, len, 255);
}

const unsigned char *
txt2data (const unsigned char *data, int *dlen)
{
   static unsigned char *buf;
   
   const unsigned char *s = data;
   unsigned char *d;
   unsigned int len, llen;
   
   len = *dlen;
   
   d = buf = realloc(buf, len);
   do
     {
	llen = *s++;
	if (llen > len - (s - data))
	  return NULL;
	memcpy(d, s, llen);
	s += llen;
	d += llen;
     } while (llen);
   
   *d = '\0';
   *dlen = d - buf;
   return buf;
}

static const unsigned char *
lbl2data (const unsigned char *data, size_t len)
{
   static signed char *buf = NULL;
   const unsigned char *s = data;
   signed char *d;
   signed int llen;
   
   d = buf = realloc(buf, len);
   assert(d);
   do
     {
	llen = *s++;
	if ((llen > 63) || (llen > (signed int)(len - (s - data))))
	  break;
	memcpy(d, s, llen);
	s += llen;
	d += llen;
     } while (llen);
   *d = '\0';
   return (const unsigned char*)buf;
}

/* New DNS-Code */

static struct rr *_new_listitem (struct rr **list)
{
   struct rr *rrp, *tmp;

   rrp = malloc(sizeof(struct rr));
   memset(rrp, 0, sizeof(struct rr));
   
   if (!*list)
      *list = rrp;
   else {
      for (tmp = *list; tmp->next; tmp = tmp->next)
	;
      tmp->next = rrp;
   }
   
   return rrp;
}

static const unsigned char *
_skip_lbl (const unsigned char *ptr, int *len)
{
   while (*ptr) {
      if (*len < 2)
	return NULL;
      if ((*ptr & 0xc0)) {
	 ptr++;
	 (*len)--;
	 break;
      }
      *len -= *ptr;
      if (*len < 1)
	return NULL;
      ptr += *ptr+1;
   }
   
   ptr++;
   (*len)--;
   return ptr;
}
	   

static __inline__ int _get_listlen (const struct rr *list)
{
   int nr = 0;
   
   while (list) {
      list = list->next;
      nr++;
   }
   
   return nr;
}

static const char *suffix = NULL;
static int suffixlen = 0;

void dns_setsuffix (char *suf)
{
   suffix = str2lbl(suf);
   suffixlen = strlen(suf)+1;
}

struct dnspkt *dns_alloc (void)
{
   void *ptr;
   
   ptr = malloc(sizeof(struct dnspkt));
   memset(ptr, 0, sizeof(struct dnspkt));
   
   return ptr;
}

void dns_free (struct dnspkt *pkt)
{
   struct rr *list, *next;
   
   list = pkt->query;
   while (list) {
      if (list->data)
	free(list->data);
      next = list->next;
      free(list);
      list = next;
   }

   list = pkt->answer;
   while (list) {
      if (list->data)
	free(list->data);
      next = list->next;
      free(list);
      list = next;
   }
   
  free(pkt);
}

void dns_setid (struct dnspkt *pkt, unsigned short id)
{
   pkt->id = id;
}

void dns_settype (struct dnspkt *pkt, int type)
{
   pkt->type = type;
}

const char *
dns_data2fqdn (const char *data)
{
   const char *ptr;
   static char *fqdn;
   
   ptr = (char*)data2lbl((unsigned char*)data);
   fqdn = realloc(fqdn, strlen(ptr)+strlen(suffix)+1);
   strcpy(fqdn, ptr);
   strcat(fqdn, suffix);
   
   return fqdn;
}

const char *
dns_fqdn2data (const char *fqdn)
{
   static char	*buf;
   const char	*off;
   
   if (buf)
     free(buf);
   
   off = strstr(fqdn, suffix);
   /* only parse if the fqdn was found, and there is more than the fqdn */
   if (off && off != fqdn)
	buf = strdup((char*)lbl2data((unsigned char*)fqdn, off - fqdn));
   else
	/* Our suffix not found... */
  	buf = NULL; 

   return buf;
}

int
dns_addquery (struct dnspkt *pkt, const char *data)
{
   struct rr *rrp;

   rrp = _new_listitem(&pkt->query);
   rrp->data = strdup(data);
   rrp->len = strlen(data)+1;
   
   return _get_listlen(pkt->query) - 1;
}

int
dns_addanswer(struct dnspkt *pkt, const char *data, int len, int link)
{
   struct rr *rrp;
   const char *ptr;
   char *buf;
   
   ptr = (char*)data2txt((unsigned char*)data, &len);
   buf = malloc(len);
   memcpy(buf, ptr, len);
   
   rrp = _new_listitem(&pkt->answer);
   rrp->data = buf;
   rrp->len = len;
   rrp->link = link;
   
   return _get_listlen(pkt->query) - 1;
}

int
dns_getpktsize(const struct dnspkt *pkt)
{
   int size;
   struct rr *list;
   
   size = 12; /* DNS-header */
   
   for (list = pkt->query; list; list = list->next)
     size += list->len + 4;

   for (list = pkt->answer; list; list = list->next)
     size += list->len + 12;
   
   return size;
}

unsigned char *dns_constructpacket (struct dnspkt *pkt, int *l)
{
   static unsigned char *buf, *ptr;
   int len, *offsets, qdcount, ancount, i;
   struct rr *list;
   
   qdcount = _get_listlen(pkt->query);
   ancount = _get_listlen(pkt->answer);

   len = dns_getpktsize(pkt);
   ptr = buf = malloc(len);
   memset(buf, 0, len);
   
   if (len > 512)
     syslog(LOG_WARNING, "WARNING: Constructed non-conform DNS-packet (size: %d)\n", len);
   
   offsets = alloca(qdcount * 4);
   
   /* Header */
   buf[0] = pkt->id / 256;
   buf[1] = pkt->id % 256;
   if (pkt->type == DNS_RESPONSE) {
      buf[2] = 0x84; /* Flags: Response, Authoritative Answer */
      buf[3] = 0x80; /* Flag: Recursion available */
   } else
     buf[2] = 0x01; /* Flags: Recursion desired */
   buf[5] = qdcount;
   buf[7] = _get_listlen(pkt->answer);
   ptr += 12;
   
   /* Query section */
   for (list = pkt->query, i=0; list; list = list->next, i++) {
      offsets[i] = ptr-buf;
      memcpy(ptr, list->data, list->len);
      ptr += list->len;
      ptr[1] = 16;
      ptr[3] = 1;
      ptr += 4;
   }
   
   /* Answer section */
   for (list = pkt->answer; list; list = list->next) {
      ptr[0] = 0xc0 | (offsets[list->link]/256);
      ptr[1] = offsets[list->link]%256;
      ptr[3] = 16;
      ptr[5] = 1;
      ptr[10] = list->len / 256;
      ptr[11] = list->len % 256;
      ptr += 12;
      memcpy(ptr, list->data, list->len);
      ptr += list->len;
   }
   *l = len;
   dns_free (pkt);
   return buf;
}

struct dnspkt *
dns_extractpkt(const unsigned char *buf, int len)
{
   int qdcount, ancount, remain, *offsets, i, j, off;
   const unsigned char *ptr;
   struct dnspkt *pkt;
   struct rr *rrp;
   
   if (len < 17)
     return NULL;
   
   pkt = dns_alloc();
   pkt->id = buf[0]*256+buf[1];
   
   qdcount = buf[5];
   ancount = buf[7];
   
   offsets = malloc(qdcount * 4);
   
   ptr = buf + 12;
   remain = len - 12;
   
   i = 0;
   while (qdcount--)
     {
	offsets[i++] = ptr - buf;
	rrp = _new_listitem(&pkt->query);
	rrp->data = decompress_label((char*)buf, len, (char*)ptr);
	if (!rrp->data)
	  {
	     syslog(LOG_ERR, "dns_extractpkt: decompress_label choked in qd\n");
	     free(offsets);
	     dns_free(pkt);
	     return NULL;
	  }
	rrp->len = strlen(rrp->data)+1;
	ptr = _skip_lbl(ptr, &remain);
	if (!ptr)
	  {
	     syslog(LOG_ERR, "dns_extractpkt: _skip_lbl choked in qd\n");
	     free(offsets);
	     dns_free(pkt);
	     return NULL;
	  }
	ptr += 4;
	remain -= 4;
     }
   while (ancount--)
     {
	if (remain < 12)
	  {
	     syslog(LOG_ERR, "dns_extractpkt: too less bytes in an\n");
	     free(offsets);
	     dns_free(pkt);
	     return NULL;
	  }
	rrp = _new_listitem(&pkt->answer);
	rrp->link = -1;
	if ((ptr[0] & 0xc0) == 0xc0)
	  {
	     off = (ptr[0] & ~(0xc0)) * 256 + ptr[1];
	     for (j = 0; j < i; j++)
	       if (offsets[j] == off)
		 break;
	     if (j < i)
	       rrp->link = j;
	  }
	//	ptr = _skip_lbl(ptr, &remain);
	//	rrp->len = ptr[8]*256+ptr[9];
	rrp->len = ptr[10]*256+ptr[11];
	ptr += 12;
	remain -= 12;
	if (remain < rrp->len)
	  {
	     syslog(LOG_ERR, "dns_extractpkt: record too long in an (%d->%d)\n",
		    remain, rrp->len);
	     dns_free(pkt);
	     return NULL;
	  }
	rrp->data = malloc(rrp->len);
	memcpy(rrp->data, ptr, rrp->len);
	ptr += rrp->len;
	remain -= rrp->len;
     }
   return pkt;
}

const char *
dns_getquerydata(struct dnspkt *pkt)
{
   struct rr *q;
   static char *ret = NULL;
   
   if (!pkt->query)
     return NULL;
   
   if (ret)
     {
	free(ret);
	ret = NULL;
     }
   
   q = pkt->query;
   pkt->query = pkt->query->next;

   ret = q->data;
   free(q);
   
   return ret;
}

char *dns_getanswerdata (struct dnspkt *pkt, int *len)
{
   struct rr *q;
   static char *ret = NULL;
   
   if (!pkt->answer)
     return NULL;
   
   q = pkt->answer;
   pkt->answer = pkt->answer->next;
   
   if (ret)
     free(ret);
   
   ret = q->data;
   *len = q->len;
   
   free(q);
   return ret;
}

int
dns_getfreespace(const struct dnspkt *pkt, int type)
{
   int raw, ret = 0, maxq;
   
   raw = DNS_MAXPKT - dns_getpktsize(pkt);
   
   if (raw < 0)
     return -1;
   
   if (type == DNS_RESPONSE) {
      ret = raw - 14;
      if (ret > 253)
	ret = 253;
   } else if (type == DNS_QUERY)
     {
//	ret = ((raw-suffixlen)*189-759)/256;
	ret = (189*(254-suffixlen))/256-6;
	if (ret > (maxq = (183-(189*suffixlen)/256)))
	  ret = maxq;
     }
   
   return (ret > 0) ? ret : 0;
}
