
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <errno.h>
#include "ndpi_config.h"
#include "ndpi_protocol_ids.h"

#include "ndpi_api.h"

#include "third_party/src/ndpi_patricia.c"

#define _P(a) [a] = #a

#include "ndpi_network_list_compile.h"

union in_addr_any {
	struct in_addr v4;
	struct in6_addr v6;
};

struct net_cidr {
	union in_addr_any a;
	uint16_t pad;
	uint16_t masklen;
};

struct net_cidr_list {
	size_t		alloc,use,max_use;
	struct net_cidr addr[0];
} *net_cidr_list[NDPI_LAST_IMPLEMENTED_PROTOCOL+1], 
  *net6_cidr_list[NDPI_LAST_IMPLEMENTED_PROTOCOL+1];


int ip4r = 0, ip4ra = 0;
int ip6r = 0, ip6ra = 0;

const char *get_proto_by_id(uint16_t proto) {
const char *s;
if(proto > NDPI_LAST_IMPLEMENTED_PROTOCOL) return "UNKNOWN";
s = proto_def[proto];
/* Skip NDPI_PROTOCOL_ or NDPI_CONTENT_ */
s += 12;
if(*s != '_') s++;
if(*s != '_') abort();
return s+1;
}

uint16_t get_proto_by_name(const char *name) {
const char *s;
size_t i;
if(!name || !*name) return NDPI_PROTOCOL_UNKNOWN;
for(i=0; i < sizeof(proto_def)/sizeof(proto_def[0]); i++) {
	s = proto_def[i];
	if(!s) continue;
	/* Skip NDPI_PROTOCOL_ or NDPI_CONTENT_ */
	s += 12;
	if(*s != '_') s++;
	if(*s != '_') abort();
	s++;
	if(!strcasecmp(s,name)) return i;
}
return NDPI_PROTOCOL_UNKNOWN;
}

/* for patricia */
void *ndpi_calloc(unsigned long int nmemb, size_t size) {
	return calloc((size_t)nmemb,size);
}
void ndpi_free(void *buf) {
	free(buf);
}

int valid_cidr(int af,const union in_addr_any *ip,int masklen);
static char *prefix_str(const ndpi_prefix_t *px, int proto,char *lbuf,size_t bufsize);

ndpi_prefix_t *fill_prefix(int af, ndpi_prefix_t *prefix, const union in_addr_any *pin, int masklen ) {
	memset((char *)prefix, 0, sizeof(ndpi_prefix_t));
	if(af == AF_INET)
		prefix->add.sin.s_addr = pin->v4.s_addr;
	    else
		prefix->add.sin6 = pin->v6;
	prefix->family = af;
	prefix->bitlen = masklen;
	prefix->ref_count = 0;
	if(masklen == 0 || !valid_cidr(af,pin,masklen)) {
	        char lbuf[128];
        	fprintf(stderr,"BAD addr %s\n",
	                prefix_str(prefix, -1, lbuf, sizeof lbuf));
		abort();
	}
	return prefix;
}

int valid_cidr(int af,const union in_addr_any *ip,int masklen) {
	const uint8_t *addr = (uint8_t *)ip;
	int offs = masklen >> 3;
	int i,l;
	uint8_t m;
	if(af == AF_INET) {
		l = 4;
		if(masklen > 32) return 0;
		if(masklen == 32) return 1;
	} else {
		l = 16;
		if(masklen > 128) return 0;
		if(masklen == 128) return 1;
	}
	for(i=l-1; i > offs; i--) if(addr[i]) {
		//fprintf(stderr,"masklen %d ip[%d] %02x\n",masklen,i,addr[i]);
		return 0;
	}
	m = (0xff << (8 - (masklen & 7))) & 0xff;
	if(addr[offs] & ~m ) {
		//fprintf(stderr,"masklen %d offs %d m %x ip[%d] %x\n",masklen,offs,m,offs,addr[offs]);
		return 0;
	}
	return 1;
}

int is_next_cidr(int af,const union in_addr_any *ip_next,const union in_addr_any *ip_start,int masklen) {
	union in_addr_any ip_tmp;
	uint8_t *addr = &(ip_tmp.v6.s6_addr[0]);
	uint16_t m,m2;
	int offs =  masklen >> 3;
	int i,l;

	if(af == AF_INET) {
		l = 4;
		if(!masklen || masklen > 32) abort();
	} else {
		l = 16;
		if(!masklen || masklen > 128) abort();
	}

	ip_tmp = *ip_start;
	if(masklen & 7) {
		m = 0x1 << (8-(masklen & 7));
	} else {
		offs--;
		m=1;
	}
	m2 = m;

	for( i=offs; i >= 0; i--) {
		uint16_t t = m + addr[i];
		if(t > 0xff) {
			addr[i] = 0;
			m = 1;
		} else {
		   addr[i] = (uint8_t)(t & 0xff);
		   break;
		}
	}
	if(0 && af == AF_INET6){
	ndpi_prefix_t prefix,prefix1,prefix2;
	char lbuf[128],lbuf2[128],lbuf3[128];
	fill_prefix(af,&prefix, ip_start,masklen);
	fill_prefix(af,&prefix1,ip_next,masklen);
	fill_prefix(af,&prefix2,&ip_tmp,masklen);
	fprintf(stderr,"Next addr o %d m %u %s + %s = %s\n", offs, m2,
		prefix_str(&prefix, -1,lbuf,sizeof lbuf),
		prefix_str(&prefix1,-1,lbuf2,sizeof lbuf2),
		prefix_str(&prefix2,-1,lbuf3,sizeof lbuf3)
		);
	}

	return memcmp(addr,(char *)ip_next,l) == 0;
}

void add_net_cidr_proto(ndpi_prefix_t *prefix,uint16_t proto) {
int af;
union in_addr_addr *addr;
uint16_t masklen;
struct net_cidr_list *nl;

	if(proto > NDPI_LAST_IMPLEMENTED_PROTOCOL) abort();
	af = prefix->family;
	addr = (union in_addr_addr *)&prefix->add;
	masklen = prefix->bitlen;
	nl = af == AF_INET ? net_cidr_list[proto]: net6_cidr_list[proto];
	if(0){
	        char lbuf[128];
        	printf("add addr %s\n",prefix_str(prefix, proto, lbuf, sizeof lbuf));
	}
	if(!nl) {
		size_t l = sizeof(struct net_cidr_list) + 32 * sizeof(struct net_cidr);
		nl = malloc(l);
		if(!nl) abort();
		if(af == AF_INET)
			net_cidr_list[proto] = nl;
		    else
			net6_cidr_list[proto] = nl;
		bzero((char *)nl,l);
		nl->alloc = 32;
	}
	if(nl->use == nl->alloc) {
		size_t n = nl->alloc + 32;
		nl = realloc(nl,sizeof(struct net_cidr_list) + n * sizeof(struct net_cidr));
		if(!nl) abort();
		if(af == AF_INET)
			net_cidr_list[proto] = nl;
		    else
			net6_cidr_list[proto] = nl;
		nl->alloc = n;
	}
	memcpy((char *)&nl->addr[nl->use].a,(char *)addr,af == AF_INET ? 4:16);
	nl->addr[nl->use].masklen = masklen;
	nl->use++;

	if(af == AF_INET)
		ip4r++;
	    else
		ip6r++;
}


static void free_ptree_data(void *data __attribute__((unused))) { ; };


static char *prefix_str(const ndpi_prefix_t *px, int proto,char *lbuf,size_t bufsize) {
char ibuf[64];
int k;
	lbuf[0] = 0;
	inet_ntop(px->family,(void *)&px->add,ibuf,sizeof(ibuf)-1);
	k = strlen(ibuf);
	if((px->family == AF_INET  && px->bitlen < 32 ) ||
	   (px->family == AF_INET6 && px->bitlen < 128 ))
		snprintf(&ibuf[k],sizeof(ibuf)-k,"/%d",px->bitlen);
	if(proto < 0)
		snprintf(lbuf,bufsize,"%s",ibuf);
	else
		snprintf(lbuf,bufsize,"%-18s %s",ibuf,
			proto > NDPI_LAST_IMPLEMENTED_PROTOCOL ?
				"unknown":get_proto_by_id(proto));
	return lbuf;
}

static void list_ptree(ndpi_patricia_tree_t *pt)
{
	ndpi_patricia_node_t *Xstack[PATRICIA_MAXBITS+1], **Xsp, *node;

	Xsp = &Xstack[0];
	node = pt->head;
	while (node) {
	    if (node->prefix)
		add_net_cidr_proto(node->prefix,node->value.u.uv32.user_value);

	    if (node->l) {
		if (node->r) {
		    *Xsp++ = node->r;
		}
		node = node->l;
		continue;
	    }
	    if (node->r) {
		node = node->r;
		continue;
	    }
	    node = Xsp != Xstack ? *(--Xsp): NULL;
	}
}

int is_protocol(char *line,uint16_t *protocol) {

    *protocol = get_proto_by_name(line);

    return *protocol != NDPI_PROTOCOL_UNKNOWN;
}

void print_comments(FILE *fd,char *str,char *prefix) {
char *c;
do {
	c = strchr(str,'\n');
	if(c) *c = '\0';
	fprintf(fd,"%s%s\n",prefix,str);
	if(c) *c++ = '\n';
	str = c;
} while(str);
}

void usage(void) {
	fprintf(stderr,"ndpi_network_list_compile [-a] [-v] [-o outputfile] [infile]\n");
	fprintf(stderr,"\t-a - do not aggregate ip network\n");
	fprintf(stderr,"\t-v - Verbose output\n");
	exit(1);
}
#if 0
void testing(void) {
  char lbuf[128];
  int i;
  union in_addr_any pin,pin2;
  ndpi_prefix_t prefix,prefix2;
  pin.v4.s_addr  = 0x80010101;
  pin2.v4.s_addr = 0x00020101;
  fill_prefix(AF_INET,&prefix,&pin,25);
  fill_prefix(AF_INET,&prefix2,&pin2,25);
  printf("%s\n",prefix_str(&prefix2,-1,lbuf,sizeof lbuf));
  for(i=32; i > 1; i--) {
   pin.v4.s_addr = htonl(0x1ul << (32 - i));
   pin2.v4.s_addr = htonl(0x1ul << (32 - i));
   fill_prefix(AF_INET,&prefix,&pin,i);
   fill_prefix(AF_INET,&prefix2,&pin2,i);
   printf("%s\n",prefix_str(&prefix,-1,lbuf,sizeof lbuf));
   is_next_cidr(AF_INET,&pin2,&pin,i);
  }
  exit(0);
}
#endif

int main(int argc,char **argv) {

  union in_addr_any pin;
  ndpi_patricia_node_t *node;
  ndpi_patricia_tree_t *ptree;
  ndpi_patricia_tree_t *ptree6;
  ndpi_prefix_t prefix,prefix1;
  int i;
  size_t ip6len = 0,ip6cnt = 0;
  uint16_t protocol;
  char lbuf[128],lbuf2[128];
  int verbose = 0,netaggr = 1,opt;
  FILE *fd = NULL,*ofd = NULL;
  char *outfile = NULL;
  char *infile = NULL;
  const char *protocol_name = NULL;

//  testing();

  while ((opt = getopt(argc, argv, "ahvo:y:")) != EOF) {
	switch(opt) {
	  case 'h': usage(); break;
	  case 'v': verbose++; break;
	  case 'a': netaggr = 0; break;
	  case 'o': outfile = strdup(optarg); break;
	  default: usage();
	}
  }

  ptree = ndpi_patricia_new(32);
  ptree6 = ndpi_patricia_new(128);
  if(!ptree || !ptree6) {
	fprintf(stderr,"Out of memory\n");
	exit(1);
  }
  for(size_t h=0; h < sizeof(ip4list)/sizeof(ip4list[0]); h++) {
      ndpi_network *ip4l = ip4list[h];
      int ml;
      for(;ip4l->network;ip4l++) {
	pin.v4.s_addr  = htonl(ip4l->network);
	ml = ip4l->cidr;
	protocol = ip4l->value;
	protocol_name = ( protocol < NDPI_LAST_IMPLEMENTED_PROTOCOL) ?
		proto_def[protocol] : proto_def[0];

	if(!protocol_name) { fprintf(stderr,"Bad proto %d\n",protocol); abort(); }
	fill_prefix(AF_INET,&prefix,&pin,ml);
	if(protocol == NDPI_PROTOCOL_MODBUS) {
		prefix_str(&prefix,protocol,lbuf2,sizeof lbuf2),
		fprintf(stderr,"Bad proto %s %s\n %s\n",protocol_name, lbuf2, ip4list_file[h]); abort();
	}

	node = ndpi_patricia_search_best(ptree, &prefix);
	if(verbose) {
	  if(node && node->prefix && protocol != node->value.u.uv32.user_value &&
                ml <= node->prefix->bitlen) {

	    fprintf(stderr,"%-40s != %s\n",
		prefix_str(&prefix,protocol,lbuf2,sizeof lbuf2),
		prefix_str(node->prefix,node->value.u.uv32.user_value,lbuf,sizeof lbuf)
		);
	  }
	}
	node = ndpi_patricia_lookup(ptree, &prefix);
	if(node) 
	  node->value.u.uv32.user_value = protocol;
      }
  }

  for(size_t h=0; h < sizeof(ip6list)/sizeof(ip6list[0]); h++) {
      ndpi_network6 *ip6l = ip6list[h];
      int ml;
      for(;ip6l->network;ip6l++) {
	ip6len += strlen(ip6l->network);
	ip6cnt ++;
	if(inet_pton(AF_INET6,ip6l->network,&pin.v6) != 1) abort();
	ml = ip6l->cidr;
	protocol = ip6l->value;

	protocol_name = ( protocol < NDPI_LAST_IMPLEMENTED_PROTOCOL) ?
		proto_def[protocol] : proto_def[0];

	if(!protocol_name) { fprintf(stderr,"Bad proto %d\n",protocol); abort(); }

	fill_prefix(AF_INET6,&prefix,&pin,ml);
	if(protocol == NDPI_PROTOCOL_MODBUS) {
		prefix_str(&prefix,protocol,lbuf2,sizeof lbuf2),
		fprintf(stderr,"Bad proto %s %s %s\n",protocol_name, lbuf2, ip6list_file[h]); abort();
	}

	node = ndpi_patricia_search_best(ptree6, &prefix);
	if(verbose) {
	  if(node && node->prefix && protocol != node->value.u.uv32.user_value &&
		ml <= node->prefix->bitlen) {

	    fprintf(stderr,"%-40s != %s\n",
		prefix_str(&prefix,protocol,lbuf2,sizeof lbuf2),
		prefix_str(node->prefix,node->value.u.uv32.user_value,lbuf,sizeof lbuf)
		);
	  }
	}
	node = ndpi_patricia_lookup(ptree6, &prefix);
	if(node) 
	  node->value.u.uv32.user_value = protocol;
      }
  }

  fd = NULL;

  list_ptree(ptree);
  list_ptree(ptree6);

  ndpi_patricia_destroy(ptree,  free_ptree_data);
  ndpi_patricia_destroy(ptree6, free_ptree_data);
  if(outfile) {
	fd = fopen(outfile,"w");
	if(!fd) {
		fprintf(stderr,"Error: can't create file '%s' : %s\n",
				outfile,strerror(errno));
		exit(1);
	}
  }
  ofd = fd ? fd:stdout;

  fprintf(ofd,"/*\n\n\tDon't edit this file!\n\n */\n\n");

  for(int ii = 0; ii < 2; ii++) {
     struct net_cidr_list **net_list = (ii == 0) ? &net_cidr_list[0]:&net6_cidr_list[0];
     int af = ii == 0 ? AF_INET:AF_INET6;
     if(af == AF_INET)
     fprintf(ofd,af == AF_INET ? 
		     "ndpi_network host_protocol_list[] = {\n":
		     "ndpi_network6 host_protocol_list_6[] = {\n"
		     );

     for(i=0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
	struct net_cidr_list *nl = net_list[i];
	size_t l;
	int mg,cnt;
	if(!nl) continue;
        
	const  char *pname = get_proto_by_id(i);
	if(verbose) fprintf(stderr,"Proto  %s\n",pname);

	nl->max_use = nl->use;
	mg = 1;
	cnt = 0;
	while(mg && netaggr) {
	    mg = 0;
	    if(verbose) fprintf(stderr,"Pass %d\n",++cnt);
	    for(l=0; l < nl->use-1; l++) {

		if(nl->addr[l].masklen == nl->addr[l+1].masklen) {
		    if(valid_cidr(af,&nl->addr[l].a,nl->addr[l].masklen-1) &&
		       is_next_cidr(af,&nl->addr[l+1].a,&nl->addr[l].a,nl->addr[l].masklen)) {
			    size_t l2;
			    if(verbose) {
				fill_prefix(af,&prefix, &nl->addr[l].a,nl->addr[l].masklen);
				fill_prefix(af,&prefix1,&nl->addr[l+1].a,nl->addr[l+1].masklen);
				fprintf(stderr,"Aggregate %s + %s\n",
					prefix_str(&prefix, -1,lbuf2,sizeof lbuf2),
					prefix_str(&prefix1,-1,lbuf,sizeof lbuf));
			    }
			    nl->addr[l].masklen--;
			    for(l2 = l+1; l2+1 < nl->use; l2++)
					nl->addr[l2] = nl->addr[l2+1];
			    nl->use--;
			    if(af == AF_INET)
				    ip4ra++;
			        else
				    ip6ra++;
			    mg++;
		    }
	        }
	    }
	}

	fprintf(ofd,"  /*\n    %s\n   */\n",pname);
	if(nl->max_use > nl->use) {
		fprintf(ofd,"  /* %u -> %u aggregated */\n",(unsigned int)nl->max_use,(unsigned int)nl->use);
	}
	for(l=0; l < nl->use; l++) {
		fill_prefix(af,&prefix1,&nl->addr[l].a,nl->addr[l].masklen);
		prefix_str(&prefix1,-1,lbuf,sizeof lbuf);
		if(af == AF_INET)
			fprintf(ofd,"  { 0x%08X, %d , %s /* %-18s */  },\n",
					htonl(prefix1.add.sin.s_addr), nl->addr[l].masklen,
					proto_def[i], lbuf);
		    else
			fprintf(ofd,"  { \"%s\", %d , %s  },\n",
					lbuf, nl->addr[l].masklen,
					proto_def[i]);
	}
    }
    if(af == AF_INET)
	fprintf(ofd,"  { 0x0, 0, 0 }\n};\n");
    else
	fprintf(ofd,"  { NULL, 0, 0 }\n};\n");
  }
if(verbose) {
	fprintf(stderr," IPv4 record %d, aggregated %d\n",ip4r,ip4r-ip4ra);
	fprintf(stderr," IPv6 record %d, aggregated %d, sum length %lu %lu\n",
			ip6r,ip6r-ip6ra,ip6len,ip6cnt*16);
}
  if(fd) fclose(fd);
  if(outfile) free(outfile);
  if(infile) free(infile);
  return(0);
}
