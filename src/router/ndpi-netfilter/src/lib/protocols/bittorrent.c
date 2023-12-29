/*
 * bittorrent.c
 *
 * Copyright (C) 2009-11 - ipoque GmbH
 * Copyright (C) 2011-22 - ntop.org
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ndpi_protocol_ids.h"

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_BITTORRENT

#include "ndpi_api.h"
#include "ndpi_private.h"

#include "btlib.h"

#define NDPI_PROTOCOL_UNSAFE_DETECTION 	0
//#define NDPI_PROTOCOL_SAFE_DETECTION 		1

#define NDPI_PROTOCOL_PLAIN_DETECTION 	0
#define NDPI_PROTOCOL_WEBSEED_DETECTION 	2

#define NDPI_NO_STD_INC 1

#ifdef __KERNEL__

extern struct kmem_cache *bt_port_cache;
#define BT_MALLOC(a) ndpi_malloc(a)
#define BT_N_MALLOC(a)         kmem_cache_zalloc (bt_port_cache, GFP_ATOMIC)
#define BT_FREE(a) ndpi_free(a)
#define BT_N_FREE(a) kmem_cache_free(bt_port_cache,a)

#else
#define BT_MALLOC(a) ndpi_malloc(a)
#define BT_N_MALLOC(a) ndpi_malloc(a)
#define BT_FREE(a) ndpi_free(a)
#define BT_N_FREE(a) ndpi_free(a)

#endif

time_t ndpi_bt_node_expire = 1200; /* time in seconds */

#ifndef __KERNEL__

//typedef int bool;
#define true 1
#define false 0

#else
extern unsigned long 
	ndpi_pto,
	ndpi_ptss,ndpi_ptsd,
	ndpi_ptds,ndpi_ptdd;
extern unsigned long 
	ndpi_puo,
	ndpi_pusf, ndpi_pusr,
	ndpi_pudf, ndpi_pudr,
	ndpi_ptussf, ndpi_ptusdr,
	ndpi_ptussr, ndpi_ptusdf,
	ndpi_ptudsf, ndpi_ptuddr,
	ndpi_ptudsr, ndpi_ptuddf;
extern unsigned long
	ndpi_btp_tm[20]; /* 3600/ 3m */

static void diagram(unsigned long *d,size_t n,int var) {
int i = 3600/n;
if(var < 0) var = 0;
i = var/i;
d[i >= n ? n-1:i]++;
}

#endif
#define BITTORRENT_PROTO_STRING          "BitTorrent protocol"

// #define BITTORRENT_CACHE_DEBUG 1

struct ndpi_utp_hdr {
  u_int8_t h_version:4, h_type:4, next_extension;
  u_int16_t connection_id;
  u_int32_t ts_usec, tdiff_usec, window_size;
  u_int16_t sequence_nr, ack_nr;
};


/* Forward declaration */
static void ndpi_search_bittorrent(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *flow);
static void ndpi_search_bittorrent_hash(struct ndpi_detection_module_struct *ndpi_struct,
					struct ndpi_flow_struct *flow, int bt_offset);

/* *********************************************** */

static int search_bittorrent_again(struct ndpi_detection_module_struct *ndpi_struct,
				   struct ndpi_flow_struct *flow) {
  ndpi_search_bittorrent(ndpi_struct, flow);
  ndpi_search_bittorrent_hash(ndpi_struct, flow, -1);
  
  /* Possibly more processing */
  return flow->extra_packets_func != NULL;
}

/* *********************************************** */

static u_int8_t is_utpv1_pkt(const u_int8_t *payload, u_int payload_len) {
  struct ndpi_utp_hdr *h = (struct ndpi_utp_hdr*)payload;

  if(payload_len < sizeof(struct ndpi_utp_hdr)) return(0);
  if(h->h_version != 1)             return(0);
  if(h->h_type > 4)                 return(0);
  if(h->next_extension > 2)         return(0);
  if(ntohl(h->window_size) > 65565) return(0);

  if((h->window_size == 0) && (payload_len != sizeof(struct ndpi_utp_hdr)))
    return(0);

  return(1);
}

int ndpi_search_dht_again(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow);

#define NDPI_STATICSTRING_LEN( s ) ( sizeof( s ) - 1 )
#define NDPI_STATICSTRING( s )  s , ( sizeof( s ) - 1 )

int memcmp_packet_hdr(struct ndpi_packet_struct *packet,
		      size_t l,const char *str,size_t len, int offs) {

if(!packet->hdr_line) return 1;
if(l >= last_hdr_idx) return 1;
if(!packet->hdr_line[l].ptr) return 1;

if(offs < 0) {
	offs = packet->hdr_line[l].len-len;
	if(offs < 0) return 1;
}
if(packet->hdr_line[l].len < len+offs) return 1;
if(packet->hdr_line[l].ptr+offs+len > packet->payload + packet->l3_packet_len) return 1;

return memcmp(packet->hdr_line[l].ptr+offs,str,len);
}

int memcmp_packet_line(struct ndpi_packet_struct *packet,
		      size_t l,const char *str,size_t len, int offs) {

if(l >= packet->parsed_lines ) return 1;
if(!packet->line[l].ptr) return 1;

if(offs < 0) {
	offs = packet->hdr_line[l].len-len;
	if(offs < 0) return 1;
}

if(packet->line[l].len < len+offs) return 1;
if(packet->line[l].ptr+offs+len > packet->payload + packet->l3_packet_len) return 1;

return memcmp(packet->line[l].ptr+offs,str,len);

}

static struct hash_ip4p_table *hash_ip4p_init(int size) {
int key;
struct hash_ip4p_table *ht;

if(size < 1024) return NULL;

ht = BT_MALLOC(sizeof(struct hash_ip4p_table) + 
		sizeof(struct hash_ip4p)*size);
if(!ht) return ht;

memset((char *)ht,0,sizeof(struct hash_ip4p_table) + 
			sizeof(struct hash_ip4p)*size);
ht->size = size;

for(key=0; key < size; key++) {
	spin_lock_init(&ht->tbl[key].lock);
}
return ht;
}

static void hash_ip4p_del(struct hash_ip4p_table *ht) {
size_t key;
struct hash_ip4p_node *n,*t;

for(key=0; key < ht->size; key++) {
	// Locking is not needed!
	n = ht->tbl[key].top;
	while(n) {
		t = n->next;
		BT_N_FREE(n);
		n = t;
	}
	ht->tbl[key].top = NULL;
}
BT_FREE(ht);
}

static void move_up(struct hash_ip4p *t,struct hash_ip4p_node *n) {
struct hash_ip4p_node *n0,*n2;

if(!n->prev) return;

n0 = n->prev;
n2 = n->next;

n0->next = n2;
if(n2) n2->prev = n0;

n->prev = NULL;
n->next = t->top;

if(n->next)
	n->next->prev = n;

t->top = n;
}

static int remove_old(struct hash_ip4p_table *ht, struct hash_ip4p *t,
		struct hash_ip4p_node *n) {
struct hash_ip4p_node *p;
int ret = 0;

if(n == t->top)
	t->top = NULL;
 else
	n->prev->next = NULL;

while(n) {
	p = n->next;
	t->len--;
	atomic_dec(&ht->count);
	ret++;
	BT_N_FREE(n);
	n = p;
}
return ret;
}

static inline u_int32_t  hash_calc(ndpi_ip_addr_t *ip,u_int16_t port,u_int32_t size) {
	u_int32_t M;
	u_int8_t *ipp = (u_int8_t *)ip;
	u_int32_t key;
	M=31; // if(!(M & 1)) M++;
	key = (((ipp[0] * M) + ipp[1] * M) + ipp[2]) * M +ipp[3];
	ipp = (u_int8_t *)&port;
	key = ((key * M) + ipp[0] * M) + ipp[1];
	return key % (size-1);
}

#ifdef NDPI_DETECTION_SUPPORT_IPV6
static inline u_int32_t  hash_calc6(ndpi_ip_addr_t *ip,u_int16_t port,u_int32_t size) {
	u_int32_t M,I;
	u_int8_t *ipp = (u_int8_t *)&I;
	u_int32_t key;
	M=103;
	I = ip->ipv6.u6_addr.u6_addr32[0] + ip->ipv6.u6_addr.u6_addr32[1] + ip->ipv6.u6_addr.u6_addr32[2] + ip->ipv6.u6_addr.u6_addr32[3];
	key = (((ipp[0] * M) + ipp[1] * M) + ipp[2]) * M +ipp[3];
	ipp = (u_int8_t *)&port;
	key = ((key * M) + ipp[0] * M) + ipp[1];
	return key % (size-1);
}
#endif

// ndpi_ip_addr_t 
static struct hash_ip4p_node *hash_ip4p_add(struct hash_ip4p_table *ht, 
		ndpi_ip_addr_t *ip, u_int16_t port, time_t lchg,int flag) {
struct hash_ip4p_node *n,*t;

u_int32_t key = 
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	ht->ipv6 ? hash_calc6(ip,port,ht->size) :
#endif
	hash_calc(ip,port,ht->size);

n = NULL;

spin_lock(&ht->tbl[key].lock);

    n = ht->tbl[key].top;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
    if(ht->ipv6) {
	struct hash_ip6p_node *n6 = (struct hash_ip6p_node *)n;
	while(n6) {
	    if(!memcmp(&n6->ip,ip->ipv6.u6_addr.u6_addr8,16) && n6->port == port) {
		    n6->lchg = lchg;
		    n6->flag |= flag;
		    move_up(&ht->tbl[key],(struct hash_ip4p_node *)n6);
		    goto unlock;
	    }
	    n6 = n6->next;
	}
	n6 = BT_N_MALLOC(sizeof(struct hash_ip6p_node));
	if(!n6) goto unlock;
	memcpy(&n6->ip,ip->ipv6.u6_addr.u6_addr8,16);
	n = (struct hash_ip4p_node *)n6;
    } else {
#endif
	while(n) {
	    if(n->ip == ip->ipv4 && n->port == port) {
		    n->lchg = lchg;
		    n->flag |= flag;
		    move_up(&ht->tbl[key],n);
		    goto unlock;
	    }
	    n = n->next;
	}
	n = BT_N_MALLOC(sizeof(struct hash_ip4p_node));
	if(!n) goto unlock;
	n->ip = ip->ipv4;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
    }
#endif
    t = ht->tbl[key].top;
    n->next = t;
    n->prev = NULL;
    n->lchg = lchg;
    n->port = port;
    n->flag = flag;
    n->count = 0;
    
    ht->tbl[key].top = n;
    if(t) t->prev = n;
    ht->tbl[key].len++;
    atomic_inc(&ht->count);

unlock:
    spin_unlock(&ht->tbl[key].lock);
return n;
}

static struct hash_ip4p_node *hash_ip4p_find(struct hash_ip4p_table *ht,
		ndpi_ip_addr_t *ip, u_int16_t port, time_t lchg) {
struct hash_ip4p_node *n;

u_int16_t key = 
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	ht->ipv6 ? hash_calc6(ip,port,ht->size) :
#endif
	hash_calc(ip,port,ht->size);

n = NULL;
spin_lock(&ht->tbl[key].lock);

    n = ht->tbl[key].top;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
    if(ht->ipv6) {
	while(n) {
	    if(!memcmp(&n->ip,ip->ipv6.u6_addr.u6_addr8,16) && n->port == port)
		    break;
	    n = n->next;
	} 
    } else {
#endif
	while(n) {
	    if(n->ip == ip->ipv4 && n->port == port)  break;
	    n = n->next;
	}
#ifdef NDPI_DETECTION_SUPPORT_IPV6
    }
#endif
    if(n) {
#ifdef __KERNEL__
	diagram(ndpi_btp_tm,sizeof(ndpi_btp_tm)/sizeof(ndpi_btp_tm[0]),lchg - n->lchg);
#endif
	n->lchg = lchg;
	if(n->count < 0xfff) n->count++;
	move_up(&ht->tbl[key],n);
    }
    spin_unlock(&ht->tbl[key].lock);
return n;

}

int ndpi_bittorrent_gc(struct hash_ip4p_table *ht,int key,time_t now) {

struct hash_ip4p_node *n=NULL;
int ret = 0;

now -= ndpi_bt_node_expire;

spin_lock(&ht->tbl[key].lock);

    n = ht->tbl[key].top;
    while(n) {
	    if(n->lchg > now) {
		    n = n->next;
		    continue;
	    }
	    ret = remove_old(ht,&ht->tbl[key],n);
	    break;
    }
spin_unlock(&ht->tbl[key].lock);
return ret;
}


/* copy from https://secure.wand.net.nz/trac/libprotoident/browser/lib/udp/lpi_dht_dict.cc */
#define ANY_BT -1

#define MASKOCTET(x) \
        ((x) == ANY_BT ? 0U : 255U)

#define FORMUP(a,b,c,d) \
        (unsigned)((((a)&0xFF)<<24)|(((b)&0xFF)<<16)|(((c)&0xFF)<<8)|((d)&0xFF))

#define FORMUPMASK(a,b,c,d) \
        FORMUP(MASKOCTET(a),MASKOCTET(b),MASKOCTET(c),MASKOCTET(d))

#define MATCH(x,a,b,c,d) \
                ((x&FORMUPMASK(a,b,c,d))==(FORMUP(a,b,c,d)&FORMUPMASK(a,b,c,d)))

/* Separate modules for dictionary-style DHT (which has a much stronger rule)
 * and Vuze DHTs (which are not so strong) 
 *
 * This source file also covers the uTP protocol, which typically shares the
 * same flow as the dictionary DHTs
 */

static inline bool match_utp_query(uint32_t payload, uint32_t len) {

	//if(MATCH(payload, 0x01, 0x00, ANY_BT, ANY_BT))
        //        return true;
        if(MATCH(payload, 0x11, 0x00, ANY_BT, ANY_BT) && len == 20)
                return true;
	if(MATCH(payload, 0x21, 0x02, ANY_BT, ANY_BT) && len == 30)
                return true;
        if(MATCH(payload, 0x21, 0x00, ANY_BT, ANY_BT) && len == 20)
                return true;
	if(MATCH(payload, 0x31, 0x02, ANY_BT, ANY_BT) && len == 30)
                return true;
        if(MATCH(payload, 0x31, 0x00, ANY_BT, ANY_BT) && len == 20)
                return true;
	if(MATCH(payload, 0x41, 0x02, ANY_BT, ANY_BT) && len == 30)
                return true;
        if(MATCH(payload, 0x41, 0x00, ANY_BT, ANY_BT) && len == 20)
                return true;
        return false;	

}

static inline bool match_utp_reply(uint32_t payload, uint32_t len) {

	if(len == 0)
		return true;
        if(MATCH(payload, 0x11, 0x00, ANY_BT, ANY_BT) && len == 20)
                return true;
	if(MATCH(payload, 0x21, 0x02, ANY_BT, ANY_BT) && (len == 30 || len == 33))
                return true;
        if(MATCH(payload, 0x21, 0x01, ANY_BT, ANY_BT) && (len == 26 || len == 23))
                return true;
        if(MATCH(payload, 0x21, 0x00, ANY_BT, ANY_BT) && len == 20)
                return true;
	if(MATCH(payload, 0x31, 0x02, ANY_BT, ANY_BT) && len == 30)
                return true;
        if(MATCH(payload, 0x31, 0x00, ANY_BT, ANY_BT) && len == 20)
                return true;
	if(MATCH(payload, 0x41, 0x02, ANY_BT, ANY_BT) && (len == 33 || len == 30))
                return true;

	return false;
}

static inline bool num_seq_match(uint32_t query, uint32_t resp) {
        uint32_t query_seq = (ntohl(query)) & 0x0000ffff;
        uint32_t resp_seq = (ntohl(resp)) & 0x0000ffff;
        if (query_seq == resp_seq)
                return true;
        /* Allowed to be seq +/- 1 as well, apparently */
        if (query_seq == resp_seq + 1)
                return true;
        if (query_seq == resp_seq - 1)
                return true;
        return false;
}
static inline bool match_utp_query_reply(uint32_t *ppayload, uint32_t *bt_seq,
		uint32_t len, uint32_t *reply) {
	uint32_t payload;

	if(len < 20) return false;

	payload = htonl(*ppayload);
	if(match_utp_query(payload,len)) {
                if (MATCH(payload, 0x01, 0x00, ANY_BT, ANY_BT)) {
                        if (len <= 20 ||
                            (payload == 0x1000004ul && 
                             (htonl(ppayload[3]) & 0xffff0000ul) == 0x1030000ul)) {
                                *reply = 0xffff;
                                return false; // WoT
                        }
                        if (!*bt_seq) {
                                *bt_seq = payload;
                                return false;
                        }
                        if (!*bt_seq || !num_seq_match(payload,*bt_seq)) {
                                return false;
                        }
                }
		*reply = 0;
		return true;
	}
	if(match_utp_reply(payload,len)) {
		*reply = 1;
		return true;
	}
	return false;
}

#ifndef __KERNEL__
static void dump_hex(const uint8_t *pl, uint32_t len, int max) {
char printable[32];
int i,l,a;
l = len;
if(max < l) l = max;
a = 0;
for(i=0; i < l; i++) {
  if(!(i & 0xf)) fprintf(stderr,"%03x: ",i);
  fprintf(stderr,"%02x",pl[i]);
  printable[a++] = pl[i] < ' ' || pl[i] & 0x80 ? '.':pl[i];
  printable[a] = '\0';
  if((i & 0xf) == 0xf) {
	  fprintf(stderr, " %s\n",printable);
	  a = 0;
  } else fputc(' ',stderr);
}
if(i & 0xf) fprintf(stderr," %s\n",printable);
}
#endif

static int bdecode(const u_int8_t *b,size_t l,
	struct ndpi_detection_module_struct *ndpi_struct,
	struct ndpi_flow_struct *flow,
	uint32_t *utp_type);

#define getN16(a) htons(*(uint16_t *)(a))
#define getN32(a) htonl(*(uint32_t *)(a))

static bool check_struct_rec(const uint8_t *pl, uint32_t len,
		uint8_t **bp, uint32_t *bpl, bool *sign) {
uint32_t w;
while(len > 4) {
	w = *pl;
	if(!w) {
		w = getN32(pl);
		pl += 4; len -= 4;
	} else {
		pl++; len--;
	}
#ifndef __KERNEL__
	if(0 && bt_parse_debug)
		fprintf(stderr,"len %d rec %d %08x next %08x\n",len,w,w,getN32(pl));
#endif
	if(!w) {
		w = getN32(pl);
		if(len >= 44 && (w == 0x180005 || w == 0x100005 || w == 0x100001)) {
			pl += 44;
			len -= 44;
			continue;
		}
#ifndef __KERNEL__
		if(bt_parse_debug)
			fprintf(stderr,"Wrong 0000000\n");
#endif
		return false;
	}
	if(w <= len) {
		if (pl[0] == 0x14 && (pl[1] == 0 || pl[1] == 1)) {
			*bp = (uint8_t *)(pl+2);
			*bpl = w-2;
		}
		if(len >= 19 && w == 19 && !memcmp(pl, "BitTorrent protocol",19))
			*sign = true;
		pl += w;
		len -= w;
	} else return false;
}
#ifndef __KERNEL__
if(0 && bt_parse_debug) fprintf(stderr,"len %d\n",len);
#endif

return len == 0;
}

static int bt_utp2(const uint8_t *pl, uint32_t len,
		struct ndpi_detection_module_struct *ndpi_struct,
		struct ndpi_flow_struct *flow, uint32_t *utp_type) {
uint32_t w;
uint8_t *bp = NULL;
uint32_t bpl = 0;
bool str_found = false;


if(len < 28) return 0;

if(getN16(pl) != 0x0100) return 0;

/*
 * udp[0:2] = 0x0100
 * udp[12:4] = 0x00040000
 *
 * udp[12:4] = 0x00100000
 * udp[12:4] = 0x03800000
 *
 *    udp[20] = 0x13 + string 'BitTorrent protocol'
 *    udp[20:2] = 0 udp[22:2] = len + payload
 *      payload 0x1400 or 0x1401 + b_encode
 *
 */
w = getN32(pl+12);
if (w == 0x00040000 || (w >> 16) == 0 || 
    w == 0x00100000 || w == 0x03800000) {
	pl += 20; len -= 20;
	if(len > 4 && check_struct_rec(pl,len, &bp, &bpl, &str_found)) {
		if(bp) bdecode(bp,bpl,ndpi_struct,flow,utp_type);
		return 1 | (str_found ? 2:0) | (bp ? 4:0);
	}
}
return 0;
}

static inline bool bt_new_pak(const uint8_t *pl, uint32_t ip, uint32_t len,
		struct ndpi_detection_module_struct *ndpi_struct ) {
uint32_t *w;
if(0)
	NDPI_LOG_DBG2(ndpi_struct, "BT: pak %08x len %d\n",*(uint32_t *)pl,len);
	if(len < 33 ) return false;
	w = (uint32_t *)(&pl[16]);
	if(len >= 42 && w[0] == 0x00000032 && (
	    w[1] == 0x04340000 || w[1] == 0x04330000 || w[1] == 0x04320000 )) {
if(0)
		NDPI_LOG_DBG2(ndpi_struct, "BT: pak 32 00 00 00 00 00 3[234] 4! %08x %08x\n",
			   *(uint32_t *)(&pl[8]),ip);
		if(w[2] == ip) return true;
	}

	if(len == 33) {
if(0)
		NDPI_LOG_DBG2(ndpi_struct, "BT: pak 33 %08x %08x\n",w[2],w[3]);
	   if( w[2] == 0x00000008 && !w[3] && !pl[32])
		return true;
	}
	return false;
}
static inline bool bt_old_pak(const uint8_t *payload, uint32_t len,
		struct ndpi_detection_module_struct *ndpi_struct,
		struct ndpi_flow_struct *flow)
{
  /* Check if this is protocol v0 */
  u_int8_t v0_extension = payload[17];
  u_int8_t v0_flags     = payload[18];

  /* Check if this is protocol v1 */
  u_int8_t v1_version     = payload[0];
  u_int8_t v1_extension   = payload[1];
  u_int32_t v1_window_size = *((u_int32_t*)&payload[12]);

  if((payload[0]== 0x60)
	     && (payload[1]== 0x0)
	     && (payload[2]== 0x0)
	     && (payload[3]== 0x0)
	     && (payload[4]== 0x0)) { /* Heuristic */
	return true;
  }
  /* CSGO/DOTA conflict */
  if(flow->packet_counter > 8 && ((v1_version & 0x0f) == 1)
		&& ((v1_version >> 4) < 5 /* ST_NUM_STATES */)
		&& (v1_extension      < 3 /* EXT_NUM_EXT */)
		&& (v1_window_size    < 32768 /* 32k */)) 
	return true;

  if((v0_flags < 6 /* ST_NUM_STATES */)
	    && (v0_extension < 3 /* EXT_NUM_EXT */)) {
    struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
    u_int32_t ts = ntohl(*((u_int32_t*)&(payload[4])));
    u_int32_t now = packet->current_time;

    if((ts < (now+86400)) && (ts > (now-86400))) {
	return true;
    }
  }
  return false;
}


#ifdef BT_ANNOUNCE

//#define BT_ANN_REC (65536/sizeof(struct bt_announce))

static void bt_add_announce(struct ndpi_detection_module_struct *ndpi_struct,
			struct bt_announce *bt_ann, int bt_ann_rec,
			int ipv6, ndpi_ip_addr_t *s_ip, u_int16_t s_port,
			struct bt_parse_protocol *p, u_int32_t now) {
struct bt_announce *b,*old = bt_ann;
int i;
size_t l;
const uint8_t *i_hash = p->a.info_hash;
u_int32_t t_ip[4];
u_int16_t t_port;

if(ipv6) {
		memcpy(&t_ip,s_ip,16);
  } else {
	t_ip[0] = 0;
	t_ip[1] = 0;
	t_ip[2] = 0xfffffffful;
	t_ip[3] = s_ip->ipv4;
  }
t_port = s_port;

for(i=0,b = bt_ann; i < bt_ann_rec; i++,b++) {
	if( !b->time) break;
	if( old->time > b->time) old = b;
	if( !memcmp(&b->hash[0],i_hash,20) && 
	    !memcmp(&b->ip[0],&t_ip[0],16) && b->port == t_port) {
		b->time = now;
		return;
	}
}

if(i == bt_ann_rec) b = old;

memcpy(&b->hash[0],i_hash,20);
memcpy(&b->ip[0],t_ip,16);
b->port = t_port;
b->time = now;

l = p->a.name_len;
if(l > sizeof(b->name)-1) l = sizeof(b->name)-1;

memcpy(&b->name,p->a.name,l);
b->name[l] = 0;
b->name_len = l;

#ifndef __KERNEL__
{
	char lbuf[256],addr[64];

	inet_ntop(ipv6 ? AF_INET6:AF_INET,s_ip,addr,sizeof(addr));

	l =  snprintf(lbuf,sizeof(lbuf)-1, "%08x%08x%08x%08x%08x %s %d %u %.*s\n",
               b->hash[0],b->hash[1],b->hash[2],b->hash[3],b->hash[4],
	       addr,htons(b->port),b->time,b->name_len,b->name);
	NDPI_LOG_DBG2(ndpi_struct, "BT: add announce %s",lbuf);
}
#endif

return;

}

#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

static int bdecode(const u_int8_t *b,size_t l,
	struct ndpi_detection_module_struct *ndpi_struct,
	struct ndpi_flow_struct *flow,
	uint32_t *utp_type)
{
struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
bt_parse_data_cb_t x;
const u_int8_t *s = b;
const u_int8_t *se = b+l;
int r = 0;
int i;

#ifdef NDPI_ENABLE_DEBUG_MESSAGES
char ip6buf[64];
#endif

u_int32_t p_now = packet->current_time;

if(!l) return 0;

memset((char *)&x,0,sizeof(x));
if(!*s) return 0;
while(s != NULL && l != 0 && r >= 0 && *s != '\0') {
	x.level=0;
	x.buf[0] = 0;
	s = bt_decode(s,&l,&r,&x);
	if(s < b || s > se) r = -1;
}
#ifndef __KERNEL__
if(0 && bt_parse_debug)
	fprintf(stderr,"bt_decode r=%d\n",r);
#endif
if(r < 0) return 0;
if(x.p.y_r) *utp_type = 1;

#ifdef BT_ANNOUNCE
if(ndpi_struct->bt_ann && x.p.a.name) {
    u_int16_t s_port =  packet->udp ? packet->udp->source :
			 packet->tcp ? packet->tcp->source : 0;

#ifdef NDPI_DETECTION_SUPPORT_IPV6
	if(packet->iphv6) 
		bt_add_announce(ndpi_struct,
				ndpi_struct->bt_ann, ndpi_struct->bt_ann_len,
				1, (ndpi_ip_addr_t *)&packet->iphv6->ip6_src,
				s_port, &x.p,p_now);
#endif
	if(packet->iph) 
		bt_add_announce(ndpi_struct,
				ndpi_struct->bt_ann, ndpi_struct->bt_ann_len,
				0, (ndpi_ip_addr_t *)&packet->iph->saddr,
				s_port, &x.p,p_now);
}
#endif
#ifdef NDPI_DETECTION_SUPPORT_IPV6
if(packet->iphv6 && ndpi_struct->bt6_ht) {
NDPI_LOG_DBG2(ndpi_struct,
	   "BT: detected valid DHT6 %d %d\n",
	   x.p.r.nn6,x.p.r.nv6);
#ifndef __KERNEL__
if(bt_parse_debug) dump_bt_proto_struct(&x.p);
#endif
	if(x.p.r.nodes6 && x.p.r.nn6) {
	    struct bt_nodes6_data *n = x.p.r.nodes6;
	    for(i=0; i < x.p.r.nn6; i++,n++) {
		hash_ip4p_add(ndpi_struct->bt6_ht,(ndpi_ip_addr_t *)&n->ip,n->port,p_now,0x2);

		NDPI_LOG_DBG2(ndpi_struct,
			   "BT: nodes6 add DHT peer %s:%d\n",
			   inet_ntop(AF_INET6,(void *)&n->ip, ip6buf,sizeof(ip6buf)),
			   htons(n->port));
	    }
	}
	if(x.p.r.values6 && x.p.r.nv6) {
	    struct bt_ipv6p2 *n = (struct bt_ipv6p2 *)x.p.r.values6;
	    for(i=0; i < x.p.r.nv6; i++,n++) {
		hash_ip4p_add(ndpi_struct->bt6_ht,(ndpi_ip_addr_t *)&n->d.ip,n->d.port,p_now,0x4);

		NDPI_LOG_DBG2(ndpi_struct,
			   "BT: values6 add DHT peer %s:%d\n",
			   inet_ntop(AF_INET6,(void *)&n->d.ip, ip6buf,sizeof(ip6buf)),
			   htons(n->d.port));
	    }
	}
	return r >= 0;
}
#endif

if(!ndpi_struct->bt_ht) return r >= 0;

if(!x.p.r.nn && !x.p.r.nv && !x.p.n_peers)
	return r >= 0;

NDPI_LOG_DBG2(ndpi_struct, "BT: detected valid DHT %d %d %d\n",
	   x.p.r.nn,x.p.r.nv,x.p.n_peers);
#ifndef __KERNEL__
if(bt_parse_debug) dump_bt_proto_struct(&x.p);
#endif

if(x.p.r.nodes && x.p.r.nn) {
    struct bt_nodes_data *n = x.p.r.nodes;
    for(i=0; i < x.p.r.nn; i++,n++) {
	hash_ip4p_add(ndpi_struct->bt_ht,(ndpi_ip_addr_t *)&n->ip,n->port,p_now,0x2);
	NDPI_LOG_DBG2(ndpi_struct, "BT: nodes add DHT peer %s:%d\n",
		   inet_ntop(AF_INET,(void *)(&n->ip),ip6buf,sizeof(ip6buf)),
		   n->port);
    }
}
if(x.p.r.values && x.p.r.nv) {
    struct bt_ipv4p2 *n = (struct bt_ipv4p2 *)x.p.r.values;
    for(i=0; i < x.p.r.nv; i++,n++) {
	hash_ip4p_add(ndpi_struct->bt_ht,(ndpi_ip_addr_t *)&n->d.ip,n->d.port,p_now,0x4);
	NDPI_LOG_DBG2(ndpi_struct, "BT: values add DHT peer %s:%d\n",
		   inet_ntop(AF_INET,(void *)(&n->d.ip),ip6buf,sizeof(ip6buf)),
		   n->d.port);
    }
}
if(x.p.peers && x.p.n_peers) {
    struct bt_ipv4p *n = x.p.peers;
    for(i=0; i < x.p.n_peers; i++,n++) {
	hash_ip4p_add(ndpi_struct->bt_ht,(ndpi_ip_addr_t *)&n->ip,n->port,p_now,0x8);
	NDPI_LOG_DBG2(ndpi_struct, "BT: peers add DHT peer %s:%d\n",
		   inet_ntop(AF_INET,(void *)(&n->ip),ip6buf,sizeof(ip6buf)),
		   n->port);
    }
}

return r >= 0;
}

/* *********************************************** */
static void ndpi_bt_add_peer_cache(struct ndpi_detection_module_struct *ndpi_struct,
struct ndpi_packet_struct *packet, uint16_t p_src, uint16_t p_dst) {

#ifdef NDPI_DETECTION_SUPPORT_IPV6
    if(ndpi_struct->bt6_ht && packet->iphv6) {
	if(packet->packet_direction)
		hash_ip4p_add(ndpi_struct->bt6_ht,(ndpi_ip_addr_t *)&packet->iphv6->ip6_src,
				p_src, packet->current_time,1);
	   else
		hash_ip4p_add(ndpi_struct->bt6_ht,(ndpi_ip_addr_t *)&packet->iphv6->ip6_dst,
				p_dst, packet->current_time,1);
    } else 
#endif
    if(ndpi_struct->bt_ht && packet->iph) {
	if(packet->packet_direction)
		hash_ip4p_add(ndpi_struct->bt_ht,(ndpi_ip_addr_t *)&packet->iph->saddr,
				p_src, packet->current_time,1);
	   else
		hash_ip4p_add(ndpi_struct->bt_ht,(ndpi_ip_addr_t *)&packet->iph->daddr,
				p_dst, packet->current_time,1);
    }
}
/* *********************************************** */

static void ndpi_search_bittorrent_hash(struct ndpi_detection_module_struct *ndpi_struct,
					struct ndpi_flow_struct *flow, int bt_offset) {
  const char *bt_hash = NULL; /* 20 bytes long */
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
  
  if(bt_offset == -1) {
    const char *bt_magic = ndpi_strnstr((const char *)packet->payload,
					BITTORRENT_PROTO_STRING, packet->payload_packet_len);
    
    if(bt_magic) {
      if(bt_magic == (const char*)&packet->payload[1])
	bt_hash = (const char*)&packet->payload[28];
      else
	bt_hash = &bt_magic[19];
    }
  } else
    bt_hash = (const char*)&packet->payload[28];
  
  if(bt_hash && (packet->payload_packet_len >= (20 + (bt_hash-(const char*)packet->payload)))) {
    memcpy(flow->protos.bittorrent.hash, bt_hash, 20);
    {
    char tmp_hash[42];
    int i;
    for(i=0; i < 20; i++)
	    sprintf(&tmp_hash[i*2],"%02x",bt_hash[i] & 0xff);
    NDPI_LOG_DBG2(ndpi_struct, "BT: save BitTorrent hash %0.20s\n",tmp_hash);
    }
  }
}

/* *********************************************** */

u_int32_t make_bittorrent_host_key(struct ndpi_flow_struct *flow, int client, int offset) {
  u_int32_t key;

  /* network byte order */
  if(flow->is_ipv6) {
    if(client)
      key = ip_port_hash_funct(ndpi_quick_hash(flow->c_address.v6, 16), htons(ntohs(flow->c_port) + offset));
    else
      key = ip_port_hash_funct(ndpi_quick_hash(flow->s_address.v6, 16), flow->s_port);
  } else {
    if(client)
      key = ip_port_hash_funct(flow->c_address.v4, htons(ntohs(flow->c_port) + offset));
    else
      key = ip_port_hash_funct(flow->s_address.v4, flow->s_port);
  }

  return key;
}

/* *********************************************** */

u_int32_t make_bittorrent_peers_key(struct ndpi_flow_struct *flow) {
  u_int32_t key;

  /* network byte order */
  if(flow->is_ipv6)
    key = ndpi_quick_hash(flow->c_address.v6, 16) + ndpi_quick_hash(flow->s_address.v6, 16);
  else
    key = flow->c_address.v4 + flow->s_address.v4;

  return key;
}

/* *********************************************** */

static void ndpi_add_connection_as_bittorrent(
		struct ndpi_detection_module_struct *ndpi_struct,
		struct ndpi_flow_struct *flow,
		int bt_offset, int check_hash,
		const u_int8_t confidence)
{
  int p1 = 0,p2 = 0;
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);

  if (packet->tcp != NULL) {
    p1 = packet->tcp->source;
    p2 = packet->tcp->dest;
    ndpi_bt_add_peer_cache(ndpi_struct,packet,p1,p2);
  } /* tcp */

  if(packet->udp) {
    p1 = packet->udp->source;
    p2 = packet->udp->dest;
    ndpi_bt_add_peer_cache(ndpi_struct,packet,p1,p2);
  }

  if(check_hash)
     ndpi_search_bittorrent_hash(ndpi_struct, flow, bt_offset);

  if(packet->iph) {
    char ip1[32],ip2[32];
    inet_ntop(AF_INET,(void *)&packet->iph->saddr,ip1,sizeof(ip1));
    inet_ntop(AF_INET,(void *)&packet->iph->daddr,ip2,sizeof(ip2));
    NDPI_LOG_DBG2(ndpi_struct, "BT: add connection %s %s:%d<->%s:%d!\n",
		   packet->tcp ? "tcp":"udp", ip1,htons(p1),ip2,htons(p2));

  }
  ndpi_set_detected_protocol_keeping_master(ndpi_struct, flow, NDPI_PROTOCOL_BITTORRENT,
					    confidence);
#ifndef __KERNEL__
  if(ndpi_struct->bittorrent_cache && packet->iph) {
    u_int32_t key, key1, key2, i;

    key = make_bittorrent_peers_key(flow);
    key1 = make_bittorrent_host_key(flow, 1, 0), key2 = make_bittorrent_host_key(flow, 0, 0);

    ndpi_lru_add_to_cache(ndpi_struct->bittorrent_cache, key1, NDPI_PROTOCOL_BITTORRENT, ndpi_get_current_time(flow));
    ndpi_lru_add_to_cache(ndpi_struct->bittorrent_cache, key2, NDPI_PROTOCOL_BITTORRENT, ndpi_get_current_time(flow));

    /* Now add hosts as twins */
    ndpi_lru_add_to_cache(ndpi_struct->bittorrent_cache,
			  key,
			  NDPI_PROTOCOL_BITTORRENT,
			  ndpi_get_current_time(flow));

    /* Also add +2 ports of the sender in order to catch additional sockets open by the same client */
    for(i=0; i<2; i++) {
      key1 = make_bittorrent_host_key(flow, 1, 1 + i);

      ndpi_lru_add_to_cache(ndpi_struct->bittorrent_cache, key1, NDPI_PROTOCOL_BITTORRENT, ndpi_get_current_time(flow));
    }
    
#ifdef BITTORRENT_CACHE_DEBUG
    printf("[BitTorrent] [%s] *** ADDED ports %u / %u [%u][%u]\n", flow->l4_proto == IPPROTO_TCP ? "TCP" : "UDP", ntohs(flow->c_port), ntohs(flow->s_port), key1, key2);
#endif
  }
#endif // __KERNEL__
  if(flow->protos.bittorrent.hash[0] == '\0') {
    /* Don't use just 1 as in TCP DNS more packets could be returned (e.g. ACK). */
#ifndef __KERNEL__
    flow->max_extra_packets_to_check = 3;
#else
    flow->max_extra_packets_to_check = 255;
#endif
    flow->extra_packets_func = search_bittorrent_again;
  }

}

#define DIRC(a,b) if(packet->packet_direction) b++ ; else a++

static int ndpi_search_bittorrent_tcp_old(struct ndpi_detection_module_struct
                      *ndpi_struct, struct ndpi_flow_struct *flow)
{
    struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
    void *f1,*f2;
    u_int16_t source,dest;

    if(!packet->tcp) return 0;
    source = packet->tcp->source;
    dest = packet->tcp->dest;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
    if(ndpi_struct->bt6_ht && packet->iphv6) {
	f1 = hash_ip4p_find(ndpi_struct->bt6_ht,(ndpi_ip_addr_t *)&packet->iphv6->ip6_src,source,
			    packet->current_time);
	f2 = hash_ip4p_find(ndpi_struct->bt6_ht,(ndpi_ip_addr_t *)&packet->iphv6->ip6_dst,dest,
			    packet->current_time);
#ifdef __KERNEL__
	if(f1)	ndpi_ptss++;
	if(f2)	ndpi_ptdd++;
#endif
    	return f1 != NULL || f2 != NULL;
    }
#endif
    if(ndpi_struct->bt_ht && packet->iph) {
	f1 = hash_ip4p_find(ndpi_struct->bt_ht,(ndpi_ip_addr_t *)&packet->iph->saddr,source,
			    packet->current_time);
	f2 = hash_ip4p_find(ndpi_struct->bt_ht,(ndpi_ip_addr_t *)&packet->iph->daddr,dest,
			    packet->current_time);
#ifdef __KERNEL__
	if(f1)	ndpi_ptss++;
	if(f2)	ndpi_ptdd++;
#endif
    	return f1 != NULL || f2 != NULL;
    }

    return 0;
}
static int ndpi_search_bittorrent_udp_old(struct ndpi_detection_module_struct
                      *ndpi_struct, struct ndpi_flow_struct *flow)
{
    struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
    u_int16_t source,dest;
    void *f1,*f2;

    if(!packet->udp) return 0;
    source = packet->udp->source;
    dest = packet->udp->dest;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
    if(ndpi_struct->bt6_ht && packet->iphv6) {
	f1 = hash_ip4p_find(ndpi_struct->bt6_ht,(ndpi_ip_addr_t *)&packet->iphv6->ip6_src,source,
			    packet->current_time);
	f2 = hash_ip4p_find(ndpi_struct->bt6_ht,(ndpi_ip_addr_t *)&packet->iphv6->ip6_dst,dest,
			    packet->current_time);
#ifdef __KERNEL__
	if(f1) {
		DIRC(ndpi_pusr,ndpi_pusf);
	}
	if(f2)	{
		DIRC(ndpi_pudr,ndpi_pudf);
	}
#endif
    	return f1 != NULL || f2 != NULL;
    }
#endif
    if(ndpi_struct->bt_ht && packet->iph) {
	f1 = hash_ip4p_find(ndpi_struct->bt_ht,(ndpi_ip_addr_t *)&packet->iph->saddr,source,
			    packet->current_time);
	f2 = hash_ip4p_find(ndpi_struct->bt_ht,(ndpi_ip_addr_t *)&packet->iph->daddr,dest,
			    packet->current_time);
#ifdef __KERNEL__
	if(f1) {
		DIRC(ndpi_pusr,ndpi_pusf);
	}
	if(f2)	{
		DIRC(ndpi_pudr,ndpi_pudf);
	}
#endif
	return f1 != NULL || f2 != NULL;
    }
    return 0;
}

/* ************************************* */

static u_int8_t ndpi_int_search_bittorrent_tcp_zero(struct ndpi_detection_module_struct
						    *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
  u_int16_t a = 0;

  if(packet->payload_packet_len == 1 && packet->payload[0] == 0x13) {
    return 0;
  }
  if(flow->packet_counter == 2 && packet->payload_packet_len > 20) {
    if(memcmp(&packet->payload[0], BITTORRENT_PROTO_STRING, 19) == 0) {
      NDPI_LOG_INFO(ndpi_struct, "found BT: plain\n");
      ndpi_add_connection_as_bittorrent(ndpi_struct, flow, 19, 1, NDPI_CONFIDENCE_DPI);
      return 1;
    }
  }


  if(packet->payload_packet_len > 20) {
    /* test for match 0x13+BITTORRENT_PROTO_STRING */
    if(packet->payload[0] == 0x13) {
      if(memcmp(&packet->payload[1], BITTORRENT_PROTO_STRING, 19) == 0) {
	NDPI_LOG_INFO(ndpi_struct, "found BT: plain\n");
	ndpi_add_connection_as_bittorrent(ndpi_struct, flow, 20, 1, NDPI_CONFIDENCE_DPI);
	return 1;
      }
    }
  }

  if(packet->payload_packet_len > 23 && memcmp(packet->payload, "GET /webseed?info_hash=", 23) == 0) {
    NDPI_LOG_INFO(ndpi_struct, "found BT: plain webseed\n");
    ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
    return 1;
  }
  /* seen Azureus as server for webseed, possibly other servers existing, to implement */
  /* is Server: hypertracker Bittorrent? */
  /* no asymmetric detection possible for answer of pattern "GET /data?fid=". */
  if(packet->payload_packet_len > 60
     && memcmp(packet->payload, "GET /data?fid=", 14) == 0 && memcmp(&packet->payload[54], "&size=", 6) == 0) {
    NDPI_LOG_INFO(ndpi_struct, "found BT: plain Bitcomet persistent seed\n");
    ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
    return 1;
  }


  if(packet->payload_packet_len > 90 && (memcmp(packet->payload, "GET ", 4) == 0
					 || memcmp(packet->payload, "POST ", 5) == 0)) {
    const u_int8_t *ptr = &packet->payload[4];
    u_int16_t len = packet->payload_packet_len - 4;
    a = 0;

    /* parse complete get packet here into line structure elements */
    ndpi_parse_packet_line_info(ndpi_struct, flow);
    /* answer to this pattern is HTTP....Server: hypertracker */
    if(     memcmp_packet_hdr(packet,user_agent_line_idx, NDPI_STATICSTRING("Azureus "), 0) == 0
	 || memcmp_packet_hdr(packet,user_agent_line_idx, NDPI_STATICSTRING("BitTorrent"), 0) == 0
	 || memcmp_packet_hdr(packet,user_agent_line_idx, NDPI_STATICSTRING("BTWebClient"), 0) == 0) {
      NDPI_LOG_INFO(ndpi_struct, "Azureus /Bittorrent user agent line detected\n");
      ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
      return 1;
    }

    if( memcmp_packet_hdr(packet,user_agent_line_idx, NDPI_STATICSTRING("Shareaza "),0) == 0 &&
	 memcmp_packet_line(packet,8, NDPI_STATICSTRING("X-Queue: "), 0) == 0) {
      NDPI_LOG_INFO(ndpi_struct, "Bittorrent Shareaza detected.\n");
      ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
      return 1;
    }

    /* this is a self built client, not possible to catch asymmetrically */
    if((packet->parsed_lines == 10 || (packet->parsed_lines == 11 && packet->line[10].len == 0))
	&& memcmp_packet_hdr(packet,user_agent_line_idx, NDPI_STATICSTRING("Mozilla/4.0 "), 0) == 0
	&& memcmp_packet_line(packet,2, NDPI_STATICSTRING("Keep-Alive: 300"), 0) == 0
	&& memcmp_packet_line(packet,3, NDPI_STATICSTRING("Connection: Keep-alive"), 0) == 0
	&& ( memcmp_packet_line(packet,4, NDPI_STATICSTRING("Accpet: */*"), 0) == 0 ||
	     memcmp_packet_line(packet,4, NDPI_STATICSTRING("Accept: */*"), 0) == 0)
	&& memcmp_packet_line(packet,5, NDPI_STATICSTRING("Range: bytes="), 0) == 0
	&& memcmp_packet_line(packet,7, NDPI_STATICSTRING("Pragma: no-cache"), 0) == 0
	&& memcmp_packet_line(packet,8, NDPI_STATICSTRING("Cache-Control: no-cache"), 0) == 0) {

      NDPI_LOG_INFO(ndpi_struct, "found BT: Bitcomet LTS\n");
      ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
      return 1;

    }

    /* FlashGet pattern */
    if(packet->parsed_lines == 8
	&& memcmp_packet_hdr(packet,user_agent_line_idx, 
				NDPI_STATICSTRING("Mozilla/4.0 (compatible; MSIE 6.0;"),0) == 0
	&& packet->host_line.ptr != NULL && packet->host_line.len >= 7
	&& memcmp_packet_line(packet,2, NDPI_STATICSTRING("Accept: */*"), 0) == 0
	&& memcmp_packet_line(packet,3, NDPI_STATICSTRING("Referer: "), 0) == 0
	&& memcmp_packet_line(packet,5, NDPI_STATICSTRING("Range: bytes="), 0) == 0
	&& memcmp_packet_line(packet,6, NDPI_STATICSTRING("Connection: Keep-Alive"), 0) == 0) {

      NDPI_LOG_INFO(ndpi_struct, "found BT: FlashGet\n");
      ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
      return 1;

    }
    if(packet->parsed_lines == 7
	&& memcmp_packet_hdr(packet,user_agent_line_idx,
		NDPI_STATICSTRING("Mozilla/4.0 (compatible; MSIE 6.0;"),0) == 0
	&& packet->host_line.ptr != NULL && packet->host_line.len >= 7
	&& memcmp_packet_line(packet,2, NDPI_STATICSTRING("Accept: */*"),0) == 0
	&& memcmp_packet_line(packet,3, NDPI_STATICSTRING("Referer: "),0) == 0
	&& memcmp_packet_line(packet,5, NDPI_STATICSTRING("Connection: Keep-Alive"), 0) == 0) {

      NDPI_LOG_INFO(ndpi_struct, "found BT: FlashGet\n");
      ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
      return 1;

    }

    /* answer to this pattern is not possible to implement asymmetrically */
    while (1) {
      if(len < 50 || ptr[0] == 0x0d) {
	goto ndpi_end_bt_tracker_check;
      }
      if(memcmp(ptr, "info_hash=", 10) == 0) {
	break;
      }
      len--;
      ptr++;
    }

    NDPI_LOG_DBG2(ndpi_struct, "BT stat: tracker info hash found\n");

    /* len is > 50, so save operation here */
    len -= 10;
    ptr += 10;

    /* parse bt hash */
    for (a = 0; a < 20; a++) {
      if(len < 3) {
	goto ndpi_end_bt_tracker_check;
      }
      if(*ptr == '%') {
	u_int8_t x1 = 0xFF;
	u_int8_t x2 = 0xFF;


	if(ptr[1] >= '0' && ptr[1] <= '9') {
	  x1 = ptr[1] - '0';
	}
	if(ptr[1] >= 'a' && ptr[1] <= 'f') {
	  x1 = 10 + ptr[1] - 'a';
	}
	if(ptr[1] >= 'A' && ptr[1] <= 'F') {
	  x1 = 10 + ptr[1] - 'A';
	}

	if(ptr[2] >= '0' && ptr[2] <= '9') {
	  x2 = ptr[2] - '0';
	}
	if(ptr[2] >= 'a' && ptr[2] <= 'f') {
	  x2 = 10 + ptr[2] - 'a';
	}
	if(ptr[2] >= 'A' && ptr[2] <= 'F') {
	  x2 = 10 + ptr[2] - 'A';
	}

	if(x1 == 0xFF || x2 == 0xFF) {
	  goto ndpi_end_bt_tracker_check;
	}
	ptr += 3;
	len -= 3;
      } else if(*ptr >= 32 && *ptr < 127) {
	ptr++;
	len--;
      } else {
	goto ndpi_end_bt_tracker_check;
      }
    }

    NDPI_LOG_INFO(ndpi_struct, "found BT: tracker info hash parsed\n");
    ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
    return 1;
  }

 ndpi_end_bt_tracker_check:

  if(packet->payload_packet_len == 80) {
    /* Warez 80 Bytes Packet
     * +----------------+---------------+-----------------+-----------------+
     * |20 BytesPattern | 32 Bytes Value| 12 BytesPattern | 16 Bytes Data   |
     * +----------------+---------------+-----------------+-----------------+
     * 20 BytesPattern : 4c 00 00 00 ff ff ff ff 57 00 00 00 00 00 00 00 20 00 00 00
     * 12 BytesPattern : 28 23 00 00 01 00 00 00 10 00 00 00
     * */
    static const char pattern_20_bytes[20] = { 0x4c, 0x00, 0x00, 0x00, 0xff,
					       0xff, 0xff, 0xff, 0x57, 0x00,
					       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00
    };
    static const char pattern_12_bytes[12] = { 0x28, 0x23, 0x00, 0x00, 0x01,
					       0x00, 0x00, 0x00, 0x10, 0x00,
					       0x00, 0x00
    };

    /* did not see this pattern ANY_BTwhere */
    if((memcmp(&packet->payload[0], pattern_20_bytes, 20) == 0)
       && (memcmp(&packet->payload[52], pattern_12_bytes, 12) == 0)) {
      NDPI_LOG_INFO(ndpi_struct, "found BT: Warez - Plain\n");
      ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
      return 1;
    }
  }

  else if(packet->payload_packet_len > 50) {
    if(memcmp(packet->payload, "GET", 3) == 0) {

      ndpi_parse_packet_line_info(ndpi_struct, flow);
      /* haven't fount this pattern ANY_BTwhere */
      if( memcmp_packet_hdr(packet,host_line_idx, NDPI_STATICSTRING("ip2p.com:"),0) == 0) {
	NDPI_LOG_INFO(ndpi_struct, "found BT: Warez - Plain Host: ip2p.com: pattern\n");
	ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 1, NDPI_CONFIDENCE_DPI);
	return 1;
      }
    }
  }
  return 0;
}

/* ************************************* */

/* Search for BitTorrent commands */
static void ndpi_int_search_bittorrent_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);

  if(ndpi_search_bittorrent_tcp_old(ndpi_struct,flow)) {
    if(packet->payload_packet_len > 48)
            ndpi_search_bittorrent_hash(ndpi_struct, flow, -1);
    ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_BITTORRENT, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI_CACHE);
    return;
  }
  if(packet->payload_packet_len == 0) {
    return;
  }

  ndpi_int_search_bittorrent_tcp_zero(ndpi_struct, flow);
}

static char *bt_code_text[8] = {
	"BAD",		//0
	"Struct",	//1
	"Sign",		//2
	"Sign+Struct",	//3
	"BAD",		//4
	"BDEC",		//5
	"BAD",		//6
	"BDEC+Sign+Struct"
	};

static char *bt_search = "BT-SEARCH * HTTP/1.1\r\n";

static u_int8_t is_port(u_int16_t a, u_int16_t b, u_int16_t what) {
  return(((what == a) || (what == b)) ? 1 : 0);
}

static void ndpi_skip_bittorrent(struct ndpi_detection_module_struct *ndpi_struct,
				 struct ndpi_flow_struct *flow,
				 struct ndpi_packet_struct *packet) {
  if(search_into_bittorrent_cache(ndpi_struct, flow))
    ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, 0, NDPI_CONFIDENCE_DPI_CACHE);
  else
    NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
}

/* ************************************* */

static void ndpi_search_bittorrent(struct ndpi_detection_module_struct *ndpi_struct,
                                   struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
  uint32_t utp_type = 0;
  int bt_code = 0;
  char *detect_type = NULL;
  char *bt_proto = NULL;

  NDPI_LOG_DBG(ndpi_struct, "Search bittorrent\n");

  /* This is broadcast */

  if(packet->iph) {
    if((packet->iph->saddr == 0xFFFFFFFF) || (packet->iph->daddr == 0xFFFFFFFF))
      goto exclude_bt;

    if(packet->udp) {
      u_int16_t sport = ntohs(packet->udp->source), dport = ntohs(packet->udp->dest);

      if(is_port(sport, dport, 3544) /* teredo */
	 || is_port(sport, dport, 5246) || is_port(sport, dport, 5247) /* CAPWAP */) {
      exclude_bt:
	NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
	return;
      }
    }
  }
#ifndef __KERNEL__
  NDPI_LOG_DBG2(ndpi_struct,
	   "BT: BITTORRENT search packet %s len %d %d:%d tcp_retrans %d bt_stage %d pac_cnt %d dir %d\n",
	   packet->tcp ? "tcp" : ( packet->udp ? "udp" : "x"),
	   packet->payload_packet_len,
	   htons(packet->tcp ? packet->tcp->source: packet->udp ? packet->udp->source:0),
	   htons(packet->tcp ? packet->tcp->dest: packet->udp ? packet->udp->dest:0),
	   packet->tcp_retransmission, flow->bittorrent_stage,
	   flow->packet_counter,packet->packet_direction);
  	   if(bt_parse_debug)
		   dump_hex((u_int8_t *)packet->payload,packet->payload_packet_len,128);
#endif

#ifndef __KERNEL__
  if(flow->detected_protocol_stack[0] == NDPI_PROTOCOL_BITTORRENT) {
	if(packet->udp != NULL &&  packet->payload_packet_len > 28 ) {
	    if(bt_utp2(packet->payload, packet->payload_packet_len,ndpi_struct,flow,&utp_type))
		    return;
	    bdecode((const u_int8_t *)packet->payload,packet->payload_packet_len,
			    ndpi_struct,flow,&utp_type);
	    }
	return;
  }
#endif

#ifndef __KERNEL__
    if((flow->packet_counter == 0 /* Do the check once */) && ndpi_struct->bittorrent_cache) {
      u_int32_t key = packet->udp ? (packet->iph->saddr + packet->udp->source) : (packet->iph->saddr + packet->tcp->source);
	u_int16_t cached_proto;
	u_int8_t found = 0;

#ifdef CACHE_DEBUG
	if(packet->udp)
	  printf("[BitTorrent] *** SEARCHING ports %u / %u\n", ntohs(packet->udp->source), ntohs(packet->udp->dest));
	else
	  printf("[BitTorrent] *** SEARCHING ports %u / %u\n", ntohs(packet->tcp->source), ntohs(packet->tcp->dest));
#endif
	
	if(ndpi_lru_find_cache(ndpi_struct->bittorrent_cache, key,
			       &cached_proto, 0, ndpi_get_current_time(flow)))
	  found = 1;
	else {
	  key = packet->udp ? (packet->iph->daddr + packet->udp->dest) : (packet->iph->daddr + packet->tcp->dest);

	  found = ndpi_lru_find_cache(ndpi_struct->bittorrent_cache, key,
				&cached_proto, 0, ndpi_get_current_time(flow));
	}

	if(found)
	  goto bittorrent_found;
      }
#endif

  if(flow->detected_protocol_stack[0] == NDPI_PROTOCOL_BITTORRENT) {
    if(packet->udp ) {
            if( packet->payload_packet_len > 28 &&
	      bdecode((const u_int8_t *)packet->payload,
			packet->payload_packet_len, ndpi_struct,flow,&utp_type)) {
		detect_type = "BDEC";
		goto bittorrent_found;
	  }
    }
  }

    if(packet->tcp != NULL) {
      ndpi_int_search_bittorrent_tcp(ndpi_struct, flow);
      if(flow->packet_counter > 8)
        ndpi_skip_bittorrent(ndpi_struct, flow, packet);
      return;
    }
    if(packet->udp == NULL) return;

      /* UDP */

      if((ntohs(packet->udp->source) < 1024)
	 || (ntohs(packet->udp->dest) < 1024) /* High ports only */) {
	ndpi_skip_bittorrent(ndpi_struct, flow, packet);
	return;
      }

      /*
	Check for uTP http://www.bittorrent.org/beps/bep_0029.html

	wireshark/epan/dissectors/packet-bt-utp.c
      */

      bt_code = bt_utp2(packet->payload, packet->payload_packet_len,
			      ndpi_struct,flow,&utp_type);
      if(bt_code) {
	      detect_type = bt_code_text[bt_code & 7];
	      goto bittorrent_found;
      }
      if(packet->payload_packet_len >= 23 /* min header size */) {
	if(strncmp((const char*)packet->payload, bt_search, strlen(bt_search)) == 0) {
		detect_type = "SearchStr"; 
		goto bittorrent_found;
	}
	
        if(packet->iph && bt_old_pak(packet->payload, packet->payload_packet_len,
				ndpi_struct,flow)) {
		detect_type = "Format1";
		goto bittorrent_found;
	}
	{
	  /* Check if this is protocol v0 */
	  u_int8_t v0_extension = packet->payload[17];
	  u_int8_t v0_flags     = packet->payload[18];

	  detect_type = "String";
	  if(is_utpv1_pkt(packet->payload, packet->payload_packet_len)) {
	    bt_proto = ndpi_strnstr((const char *)&packet->payload[20], BITTORRENT_PROTO_STRING, packet->payload_packet_len-20);
	    goto bittorrent_found;
	  } else if((packet->payload[0]== 0x60)
		    && (packet->payload[1]== 0x0)
		    && (packet->payload[2]== 0x0)
		    && (packet->payload[3]== 0x0)
		    && (packet->payload[4]== 0x0)) {
	    /* Heuristic */
	    bt_proto = ndpi_strnstr((const char *)&packet->payload[20], BITTORRENT_PROTO_STRING, packet->payload_packet_len-20);
	    goto bittorrent_found;
	    /* CSGO/DOTA conflict */
	  } else if((v0_flags < 6 /* ST_NUM_STATES */) && (v0_extension < 3 /* EXT_NUM_EXT */)) {
	    u_int32_t ts = ntohl(*((u_int32_t*)&(packet->payload[4])));
	    u_int32_t now = packet->current_time;

	    if((ts < (now+86400)) && (ts > (now-86400))) {
	      bt_proto = ndpi_strnstr((const char *)&packet->payload[20], BITTORRENT_PROTO_STRING, packet->payload_packet_len-20);
	      goto bittorrent_found;
	    }
	  } else {
		  bt_proto = ndpi_strnstr((const char *)&packet->payload[20], BITTORRENT_PROTO_STRING, packet->payload_packet_len-20);
		  if(bt_proto)
		    goto bittorrent_found;
	  }

	}
      }
      if(match_utp_query_reply((uint32_t *)packet->payload,&flow->bittorrent_seq,
			       packet->payload_packet_len,&utp_type)) {
	      if(utp_type == 0xffff) {
		        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
			return;
	      }
	      detect_type = utp_type ? "UTP_req":"UTP_ack";
	      goto bittorrent_found;
      }
      if(packet->iph && bt_new_pak(packet->payload, packet->iph->saddr,
			      		packet->payload_packet_len,ndpi_struct)) {
	      detect_type = "Format2";
	      goto bittorrent_found;
      }

      flow->bittorrent_stage++;

      if(flow->bittorrent_stage < 5) { // FIXME 10
          if( packet->payload_packet_len > 28 &&
	      bdecode((const u_int8_t *)packet->payload,
			packet->payload_packet_len, ndpi_struct,flow,&utp_type)) {
		detect_type = "BDEC";
		goto bittorrent_found;
	  }
	if(packet->payload_packet_len > 19 /* min size */) {

	  if(memcmp((const char *)&packet->payload[packet->payload_packet_len-9],
				  "/announce",9) == 0 || 
	     (bt_proto = ndpi_strnstr((const char *)packet->payload, "BitTorrent protocol",
						     packet->payload_packet_len)) != NULL) {
		  ndpi_search_bittorrent_hash(ndpi_struct, flow, -1);
		  detect_type = "String";
		  goto bittorrent_found;
	  }
	}

	if(ndpi_search_bittorrent_udp_old(ndpi_struct,flow))
		ndpi_set_detected_protocol(ndpi_struct, flow,
					 NDPI_PROTOCOL_BITTORRENT,
					 NDPI_PROTOCOL_UNKNOWN,NDPI_CONFIDENCE_DPI);
	return;
      }


      ndpi_skip_bittorrent(ndpi_struct, flow, packet);
      return;
      /* UDP */

  bittorrent_found:
       //if(bt_proto && (packet->payload_packet_len > 47))
       //     memcpy(flow->protos.bittorrent.hash, &bt_proto[27], 20);

        NDPI_LOG_INFO(ndpi_struct,
	     "BT: BitTorrent protocol detected: %s\n",detect_type ? detect_type : "(NULL)");
        ndpi_add_connection_as_bittorrent(ndpi_struct, flow, -1, bt_proto ? 1:0, NDPI_CONFIDENCE_DPI);
  return;
}

int ndpi_search_dht_again(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  ndpi_search_bittorrent(ndpi_struct,flow);
  // flow->num_extra_packets_checked = 0;
  return 1;
}


void ndpi_bittorrent_init(struct ndpi_detection_module_struct *ndpi_struct,
		u_int32_t size,u_int32_t size6,u_int32_t tmo,int logsize) {

	ndpi_struct->bt_ht = hash_ip4p_init(size);
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	ndpi_struct->bt6_ht = hash_ip4p_init(size6);
	if(ndpi_struct->bt6_ht)
		ndpi_struct->bt6_ht->ipv6=1;
#endif
	ndpi_bt_node_expire = tmo;
#ifdef BT_ANNOUNCE
	if(logsize > 0) {
		int bt_ann_rec = logsize*1024/sizeof(struct bt_announce); /* 192 bytes */
		ndpi_struct->bt_ann = ndpi_calloc(bt_ann_rec,sizeof(struct bt_announce));
		ndpi_struct->bt_ann_len = bt_ann_rec;
	}
#endif
}

void ndpi_bittorrent_done(struct ndpi_detection_module_struct *ndpi_struct) {
#ifdef BT_ANNOUNCE
	if(ndpi_struct->bt_ann) {
		ndpi_free(ndpi_struct->bt_ann);
		ndpi_struct->bt_ann = NULL;
	}
#endif
if(ndpi_struct->bt_ht) {
	hash_ip4p_del(ndpi_struct->bt_ht);
	ndpi_struct->bt_ht = NULL;
}
#ifdef NDPI_DETECTION_SUPPORT_IPV6
if(ndpi_struct->bt6_ht) {
	hash_ip4p_del(ndpi_struct->bt6_ht);
	ndpi_struct->bt6_ht = NULL;
}
#endif
}

void init_bittorrent_dissector(struct ndpi_detection_module_struct *ndpi_struct,
			       u_int32_t *id)
{
  ndpi_set_bitmask_protocol_detection("BitTorrent", ndpi_struct, *id,
				      NDPI_PROTOCOL_BITTORRENT,
				      ndpi_search_bittorrent,
				      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
				      SAVE_DETECTION_BITMASK_AS_UNKNOWN,
				      ADD_TO_DETECTION_BITMASK);
  *id += 1;
}
