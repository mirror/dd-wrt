
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ndpi_config.h"
#include "ndpi_protocol_ids.h"

#include "ndpi_api.h"

#include "third_party/src/ndpi_patricia.c"

#define _P(a) [a] = #a

#include "ndpi_network_list_compile.h"

struct net_cidr {
	struct in_addr a;
	uint16_t masklen;
};

struct net_cidr_list {
	size_t		alloc,use;
	char		comments[512];
	struct net_cidr addr[0];
} *net_cidr_list[NDPI_LAST_IMPLEMENTED_PROTOCOL+1];

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


void add_net_cidr_proto(struct in_addr *addr,uint16_t masklen, uint16_t proto) {
struct net_cidr_list *nl;

if(proto > NDPI_LAST_IMPLEMENTED_PROTOCOL) abort();
nl = net_cidr_list[proto];
if(!nl) {
	size_t l = sizeof(struct net_cidr_list) + 32 * sizeof(struct net_cidr);
	nl = malloc(l);
	if(!nl) abort();
	net_cidr_list[proto] = nl;
	bzero((char *)nl,l);
	nl->alloc = 32;
}
if(nl->use == nl->alloc) {
	size_t n = nl->alloc + 32;
	nl = realloc(nl,sizeof(struct net_cidr_list) + n * sizeof(struct net_cidr));
	if(!nl) abort();
	net_cidr_list[proto] = nl;
	nl->alloc = n;
}
if(addr) {
	nl->addr[nl->use].a = *addr;
	nl->addr[nl->use].masklen = masklen;
	nl->use++;
}
}

static void free_ptree_data(void *data) { ; };

ndpi_prefix_t *fill_ipv4_prefix(ndpi_prefix_t *prefix,struct in_addr *pin, int masklen ) {
	memset((char *)prefix, 0, sizeof(ndpi_prefix_t));
	prefix->add.sin.s_addr = pin->s_addr;
	prefix->family = AF_INET;
	prefix->bitlen = masklen;
	prefix->ref_count = 0;
	return prefix;
}


static char *prefix_str(ndpi_prefix_t *px, int proto,char *lbuf,size_t bufsize) {
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
		add_net_cidr_proto(&node->prefix->add.sin,node->prefix->bitlen,node->value.u.uv32.user_value);

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

/*
 * Return:
 *	0: bad format
 *	1: comment
 *	2: word
 *	3: list
 */

int parse_line(char *l,int *sp, char *word,size_t wlen,char **optarg) {

char *s = l,*sw,*dlm;

  *sp = 0;
  *word = '\0';
  *optarg = NULL;

  dlm = strchr(s,'\r');
  if(dlm) *dlm = '\0';
  dlm = strchr(s,'\n');
  if(dlm) *dlm = '\0';

  while(*s && (*s == ' ' || *s == '\t')) { (*sp) += *s == ' ' ? 1:8; s++; }
  if(!*s) return 1;
  if(*s == '#') return 1;
  sw = s;
  bzero(word,wlen);
  if(*s == '-') { // element of list
	s++;
	while(*s && (*s == ' ' || *s == '\t')) s++;
	if(!*s) return 1;
	strncpy(word,s,wlen);
	return 3;
  }
  dlm = strchr(sw,':');
  if(!dlm) return 0;
  strncpy(word,s,wlen < (size_t)(dlm - sw) ? wlen : (size_t)(dlm - sw));
  if(strchr(word,' ') || strchr(word,'\t')) return 0;
  dlm++;
  while(*dlm && (*dlm == ' ' || *dlm == '\t')) dlm++;
  *optarg = *dlm ? dlm : NULL;
  return 2;
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
	fprintf(stderr,"ndpi_network_list_compile [-a] [-v] [-o outputfile] [-y output.yaml] [infile]\n");
	fprintf(stderr,"\t-a - do not aggregate ip network\n");
	fprintf(stderr,"\t-v - Verbose output\n");
	exit(1);
}

int main(int argc,char **argv) {

  struct in_addr pin;
  ndpi_patricia_node_t *node;
  ndpi_patricia_tree_t *ptree;
  ndpi_prefix_t prefix,prefix1;
  int i,line=0,code,nsp,psp;
  uint16_t protocol;
  char lbuf[128],lbuf2[128];
  char word[128],lastword[128],*wordarg;
  int verbose = 0,netaggr = 1,opt;
  FILE *fd = NULL,*ifd = NULL,*ofd = NULL,*yfd = NULL;
  char *youtfile = NULL;
  char *outfile = NULL;
  char *infile = NULL;
  struct net_cidr_list *pnl = NULL;
  char *protocol_name = NULL;

  while ((opt = getopt(argc, argv, "ahvo:y:")) != EOF) {
	switch(opt) {
	  case 'h': usage(); break;
	  case 'v': verbose++; break;
	  case 'a': netaggr = 0; break;
	  case 'o': outfile = strdup(optarg); break;
	  case 'y': youtfile = strdup(optarg); break;
	  default: usage();
	}
  }

  ptree = ndpi_patricia_new(32);
  if(!ptree) {
	fprintf(stderr,"Out of memory\n");
	exit(1);
  }

  if (optind < argc) {
    infile = strdup(argv[optind++]);
    fd = fopen(infile,"r");
    if(!fd) {
	fprintf(stderr,"Error: can't oprn file '%s' : %s\n",
				infile,strerror(errno));
	exit(1);
    }
    ifd = fd;
  } else {
    ifd = stdin;
  }

  while(ifd) {
    line = 0;
    psp = 0;
    while(!feof(ifd)) {
	char lbuf[256],*s;
	if(!(s = fgets(lbuf,sizeof(lbuf),ifd)))
		break;

	line++;

	code = parse_line(s,&nsp,word,sizeof(word)-1,&wordarg);

	if(code == 0) {
		fprintf(stderr,"Error: Invalid line %d: '%s'\n",line,s);
		exit(1);
	}
	if(code == 1) continue;

	/*
	 * Line is:
	 * protocol:
	 *   ip:
	 *   source:
	 */
	if(code == 2) {
		if(!nsp) {
			if(!is_protocol(word,&protocol)) {
				fprintf(stderr,"Error: unknown protocol '%s' line %d: '%s'\n",
						word,line,s);
				exit(1);
			}
			if(protocol_name) free(protocol_name);
			protocol_name = strdup(word);
			add_net_cidr_proto(NULL,0,protocol);
			pnl = net_cidr_list[protocol];
			lastword[0] = '\0';
			psp = 0;
			continue;
		}
		if(psp && psp != nsp) {
			fprintf(stderr,"Invalid line %d: '%s'\n",line,s);
			exit(1);
		}
		psp = nsp;

		if(!strcmp(word,"source")) {
			strncpy(lastword,word,sizeof(lastword)-1);
			if(wordarg)
				strncat(pnl->comments,wordarg,sizeof(pnl->comments)-1);
			continue;
		}
		if(!strcmp(word,"ip")) {
			if(wordarg) {
				fprintf(stderr,"Invalid line %d: '%s'\n",line,s);
				exit(1);
			}
			strncpy(lastword,word,sizeof(lastword));
			continue;
		}
		fprintf(stderr,"Invalid line %d: '%s'\n",line,s);
		exit(1);
	}
	/*
	 * Line is element of list ip/source
	 */
	if( code == 3 ) {
		if(!psp || nsp <= psp) {
			fprintf(stderr,"Invalid list line %d: '%s'\n",line,s);
			exit(1);
		}
		if(!strcmp(lastword,"ip")) {
			char *nm = strchr(word,'/');
			int ml = nm ? atoi(nm+1) : 32;
			if(nm) *nm = 0;
			if(!inet_aton(word,&pin)) {
				fprintf(stderr,"Invalid ip in line %d: '%s'\n",line,s);
				exit(1);
			}
			if(nm) *nm = '/';
			if((pin.s_addr & htonl(~(0xfffffffful << (32 - ml)))) != 0)
				fprintf(stderr,"Warning: line %4d: '%s' is not network (%s)\n",line,word,
						protocol_name ? protocol_name : "unknown");
			if((pin.s_addr & htonl(0xf0000000ul)) == htonl(0xe0000000ul))
				fprintf(stderr,"Warning: line %4d: '%s' is multicast address!\n",line,word);
			if((pin.s_addr & htonl(0x7f000000ul)) == htonl(0x7f000000ul))
				fprintf(stderr,"Warning: line %4d: '%s' is loopback network!\n",line,word);

			pin.s_addr &= htonl(0xfffffffful << (32 - ml));
			fill_ipv4_prefix(&prefix,&pin,ml);

			node = ndpi_patricia_search_best(ptree, &prefix);
			if(verbose) {
			  if(node && node->prefix && protocol != node->value.u.uv32.user_value) {

			    fprintf(stderr,"%-32s subnet %s\n",
				prefix_str(&prefix,protocol,lbuf2,sizeof lbuf2),
				prefix_str(node->prefix,node->value.u.uv32.user_value,lbuf,sizeof lbuf)
				);
			  }
			}
			node = ndpi_patricia_lookup(ptree, &prefix);
			if(node) 
			  node->value.u.uv32.user_value = protocol;

			continue;
		}
		if(!strcmp(lastword,"source")) {
			if(pnl->comments[0]) {
				strncat(pnl->comments,"\n",sizeof(pnl->comments));
			}
			strncat(pnl->comments,word,sizeof(pnl->comments)-1);
			continue;
		}
		fprintf(stderr,"Invalid list word '%s'\n",lastword);
		exit(1);
	}
	printf("code = %d\n",code);
	abort();
      }
      ifd = NULL;
      if(fd) {
	fclose(fd);

	if (optind < argc) {
		if(infile) free(infile);
		infile = strdup(argv[optind++]);
		fd = fopen(infile,"r");
		if(!fd) {
			fprintf(stderr,"Error: can't oprn file '%s' : %s\n",
					infile,strerror(errno));
			exit(1);
      		}
		ifd = fd;
	}
      }
  }

  fd = NULL;

  if(!line) exit(0);
  list_ptree(ptree);

  ndpi_patricia_destroy(ptree, free_ptree_data);

  if(youtfile) {
	yfd = fopen(youtfile,"w");
	if(!yfd) {
		fprintf(stderr,"Error: can't create file '%s' : %s\n",
				youtfile,strerror(errno));
		exit(1);
	}
  }
  if(outfile) {
	fd = fopen(outfile,"w");
	if(!fd) {
		fprintf(stderr,"Error: can't create file '%s' : %s\n",
				outfile,strerror(errno));
		exit(1);
	}
  }
  ofd = fd ? fd:stdout;

  fprintf(ofd,"/*\n\n\tDon't edit this file!\n\n\tchange *.yaml files\n\n */\n\n");
  fprintf(ofd,"NDPI_STATIC ndpi_network host_protocol_list[] = {\n");

  for(i=0; i <= NDPI_LAST_IMPLEMENTED_PROTOCOL; i++) {
	struct net_cidr_list *nl = net_cidr_list[i];
	size_t l;
	int mg;
	if(!nl) continue;
        
	const  char *pname = get_proto_by_id(i);

	if(nl->comments[0]) {
		fprintf(ofd,"  /*\n");
		 print_comments(ofd,nl->comments,"    ");
		fprintf(ofd,"   */\n");
	} else 
		fprintf(ofd,"  /*\n    %s\n   */\n",pname);
	if(yfd) {
		fprintf(yfd,"%s:\n",pname);
		if(nl->comments[0]) {
			fprintf(yfd,"\tsource:\n");
			print_comments(yfd,nl->comments,"\t  - ");
		}
	}
	mg = 1;
	while(mg && netaggr) {
	    mg = 0;
	    for(l=0; l < nl->use; l++) {
		if(l+1 < nl->use) {
			if(nl->addr[l].masklen > 0 &&
			   nl->addr[l].masklen == nl->addr[l+1].masklen) {
				uint32_t a = htonl(nl->addr[l].a.s_addr);
				uint32_t m = 0xfffffffful << (32 - nl->addr[l].masklen);
				if( (a & ~(m << 1)) == 0 &&
				     a + (~m + 1) == htonl(nl->addr[l+1].a.s_addr)) {
					size_t l2;
					if(verbose) {
						fill_ipv4_prefix(&prefix, &nl->addr[l].a,nl->addr[l].masklen);
						fill_ipv4_prefix(&prefix1,&nl->addr[l+1].a,nl->addr[l+1].masklen);
						fprintf(stderr,"Aggregate %s + %s\n",
							prefix_str(&prefix, -1,lbuf2,sizeof lbuf2),
							prefix_str(&prefix1,-1,lbuf,sizeof lbuf));
					}
					nl->addr[l].masklen--;
					for(l2 = l+1; l2+1 < nl->use; l2++)
						nl->addr[l2] = nl->addr[l2+1];
					nl->use--;
					mg++;
				}
			}
		}
	    }
	}

	for(l=0; l < nl->use; l++) {
		if(!l && yfd) fprintf(yfd,"\tip:\n");
		fill_ipv4_prefix(&prefix1,&nl->addr[l].a,nl->addr[l].masklen);
		prefix_str(&prefix1,-1,lbuf,sizeof lbuf);

		if(yfd) fprintf(yfd,"\t  - %s\n",lbuf);

		fprintf(ofd,"  { 0x%08X, %d , %s /* %-18s */  },\n",
				htonl(prefix1.add.sin.s_addr), nl->addr[l].masklen,
				proto_def[i], lbuf);
	}
  }
  fprintf(ofd,"  { 0x0, 0, 0 }\n};\n");

  if(fd) fclose(fd);
  if(yfd) fclose(yfd);
  if(outfile) free(outfile);
  if(infile) free(infile);
  if(protocol_name) free(protocol_name);
  return(0);
}
