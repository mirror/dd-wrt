
#define _DEFAULT_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include <time.h>

#include "../src/ndpi_flow_info.h"

/*
 * procfs buffer 256Kb on x86_64 and 64k on i386
 */

#define FLOW_READ_COUNT 256*1024
#define FLOW_PRE_HDR 1024
struct dump_data {
	struct dump_data *next;
	int		 num;
	int		 offs;
	size_t		 len;
	uint8_t		 data[];
};

#define MAX_PROTO_NAMES 512

int verbose = 0;
char *proto_name[MAX_PROTO_NAMES];
int ndpi_last_proto=0;

struct dump_data *head = NULL, *tail = NULL;

static char *proto_buf = NULL;
static size_t proto_buf_len = 0;
static size_t proto_buf_pos = 0;

int append_buf(char *data,size_t len) {
	if(!proto_buf) {
		proto_buf_len = 4096;
		proto_buf = calloc(1,proto_buf_len);
	}
	if(!proto_buf) return 1;
	if(proto_buf_pos + len > proto_buf_len) {
		char *tmp = realloc(proto_buf,proto_buf_len+4096);
		if(!tmp) return 1;
		proto_buf = tmp;
		proto_buf_len+=4096;
	}
	memcpy(proto_buf+proto_buf_pos,data,len);
	proto_buf_pos += len;
	return 0;
}

int move_to_next(struct dump_data *dump, int offset) {
	int tail_len;
	struct dump_data *n;

	n = dump->next;
	if(!n) return 0;
	tail_len = dump->len - offset;
	if(n->offs  < tail_len) return 0;
	n->len += tail_len;
	n->offs -= tail_len;
	memcpy((char *)&n->data[n->offs],(char *)&dump->data[dump->offs+offset],tail_len);
	dump->len -= tail_len;
	return 1;
}

#define REC_PROTO 1
#define REC_START 2
#define REC_FLOW  4
#define REC_LOST  8

static int check_flow_data(struct dump_data *dump) {
struct flow_data_common *c;
char *data;
int offs,rl,n;
int ret=0;
	if(!dump) return 0;
	data = (char*)&dump->data[dump->offs];
	offs = 0;
	n=0;
	while(offs < dump->len) {
		n++;
		c = (struct flow_data_common *)&data[offs];
		switch(c->rec_type) {
		case 0:
			rl = 4;
			if(offs+rl > dump->len) {
				if(!move_to_next(dump,offs)) {
					printf("T0:%d len\n",n); return -1;
				}
				return offs != dump->len ? -1: ret;
			}
			if( !c->host_len ) { printf("T0:%d host_len offs %d\n",n,offs); return -1;}
			rl += c->host_len;
			if(offs+rl > dump->len) {
				if(!move_to_next(dump,offs)) {
					printf("T0:%d len2\n",n); return -1;
				}
				return offs != dump->len ? -1: ret;
			}
			ret |= REC_PROTO;
			offs += rl;
			break;
		case 1:
			rl = 8;
			if(offs+rl > dump->len) {
				if(!move_to_next(dump,offs)) {
					printf("T1:%d len\n",n); return -1; 
				}
				return offs != dump->len ? -1: ret;
			}
			ret |= REC_START;
			offs += rl;
			break;
		case 3:
			rl = sizeof(struct flow_data_common);
			if(offs+rl > dump->len) {
				if(!move_to_next(dump,offs)) {
					printf("T3:%d len\n",n); return -1; 
				}
				return offs != dump->len ? -1: ret;
			}
			offs += sizeof(struct flow_data_common);
			ret |= REC_LOST;
			break;
		case 2:
			rl = sizeof(struct flow_data_common) + 
				( c->family ? sizeof(struct flow_data_v6) :
				  	      sizeof(struct flow_data_v4));
			if(offs+rl > dump->len) {
				if(!move_to_next(dump,offs)) {
					printf("T2:%d len error1. offs %d\n",n,offs); return -1; 
				}
				return offs != dump->len ? -1: ret;
			}
			if(offs+rl+c->opt_len+c->host_len > dump->len) {
				if(!move_to_next(dump,offs)) {
					printf("T2:%d len error1. offs %d\n",n,offs);
					return -1;
				}
				return offs != dump->len ? -1: ret;
			}
			rl += c->opt_len + c->host_len;
			offs += rl;
			ret |= REC_FLOW;
			break;
		}
	}
	return offs != dump->len ? -1: ret;
}

void write_proto_name(int fd) {
char lbuf[256+8],*n;
struct flow_data_common *c = (struct flow_data_common *)lbuf;
uint16_t *p;
int id,len;

memset(lbuf,0,sizeof(lbuf));
n = (char *)&c->time_start;
p = (uint16_t *)&c->proto;

for(id=0; id <= ndpi_last_proto; id++) {
	*p = id;	
	if(proto_name[id]) {
		len = strlen(proto_name[id]);
		if(len > 255) len = 255;
		memcpy(n,proto_name[id],len);
	} else {
		len = snprintf(n,255,"proto%d",id);
	}
	c->host_len = len;
	append_buf(lbuf,(n-lbuf)+len);
}
if(proto_buf && proto_buf_pos)
	(void)write(fd,proto_buf,proto_buf_pos);
if(proto_buf) {
	free(proto_buf);
	proto_buf = 0;
}

}

void ndpi_get_proto_names(void) {
char lbuf[256],mark[64],name[64];
unsigned int id,max=0;
FILE *fd;

	if(ndpi_last_proto != 0) return;

	fd = fopen("/proc/net/xt_ndpi/proto","r");

	if(!fd) {
		ndpi_last_proto = -1;
		return;
	}

	do {
		if(!fgets(lbuf,sizeof(lbuf)-1,fd)) break;
		if(lbuf[0] != '#') {
			ndpi_last_proto = -1;
			fclose(fd);
			return;
		}
		while(fgets(lbuf,sizeof(lbuf)-1,fd)) {
			if(sscanf(lbuf,"%x %s %s",&id,mark,name) == 3) {
				if(id >= MAX_PROTO_NAMES) continue;
				if(!strncmp(mark,"disabled",8) ||
				   !strcmp(mark,"Free"))  {
					snprintf(name,sizeof(name)-1,"proto%d",id);
					proto_name[id] = strdup(name);
					continue;
				}
				proto_name[id] = strdup(name);
				if(verbose > 2) fprintf(stderr,"%d %s\n",id,name);
				if(id > max) max = id;
			}
		}
	} while(0);
	fclose(fd);
	if(max != 0) ndpi_last_proto = max;
	if(verbose)
		fprintf(stderr,"Last ID %d\n",ndpi_last_proto);
}

static const char *ndpi_proto_name(unsigned int id) {
	return (id < MAX_PROTO_NAMES && proto_name[id]) ? proto_name[id] : "Bad";
}

static int decode_flow(int fd,struct dump_data *dump) {
struct flow_data_common *c;
struct flow_data_v4 *v4;
struct flow_data_v6 *v6;
char *data,buff[512],
     a1[64],a2[64],a3[32],a4[32],
     p1[8],p2[8],p3[8],p4[8],
     pn[64],*tp;
int offs,l,rl;
uint16_t id;

	data = (char*)&dump->data[dump->offs];
	offs = 0;
	while(offs < dump->len-4) {
		c = (struct flow_data_common *)&data[offs];
		switch(c->rec_type) {
		case 0:
			if( !c->host_len ) return -1;
			rl = 4 + c->host_len;
			if(offs+rl > dump->len) return -1;
			id = *(uint16_t *)&c->proto;
			tp = malloc(rl);
			if(tp) {
				memcpy(tp,&data[offs+4],c->host_len);
				tp[c->host_len] = '\0';
				if(proto_name[id]) free(proto_name[id]);
				proto_name[id] = tp;
				if(verbose > 2)
					fprintf(stderr,"proto %d '%s'\n",id,tp);
			}
			offs += rl;
			break;
		case 1:
			rl = 8;
			if(offs+rl > dump->len) return -1;
			l = snprintf(buff,sizeof(buff)-1,"TIME %u\n",c->time_start);
			(void)write(fd,buff,l);
			offs += rl;
			break;
		case 3:
			rl = sizeof(struct flow_data_common);
			if(offs+rl > dump->len) return -1;
			l = snprintf(buff,sizeof(buff)-1,"LOST_TRAFFIC %u %u %" PRIu64 " %" PRIu64 "\n",
				c->p[0],c->p[1],c->b[0],c->b[1]);
			(void)write(fd,buff,l);
			offs += rl;
			break;
		case 2:
			rl = sizeof(struct flow_data_common) + 
				( c->family ? sizeof(struct flow_data_v6) :
				  	      sizeof(struct flow_data_v4));
			if(offs+rl+c->opt_len+c->host_len > dump->len) return -1;
			if(c->family) {
				v6 = (struct flow_data_v6 *)&data[offs+sizeof(struct flow_data_common)];
				inet_ntop(AF_INET6,&v6->ip_s,a1,sizeof(a1)-1);
				inet_ntop(AF_INET6,&v6->ip_d,a2,sizeof(a2)-1);
				a3[0] = '\0';
				a4[0] = '\0';
				snprintf(p1,sizeof(p1)-1,"%d",htons(v6->sport));
				snprintf(p2,sizeof(p2)-1,"%d",htons(v6->dport));
				p3[0] = '\0';
				p4[0] = '\0';
			} else {
				v4 = (struct flow_data_v4 *)&data[offs+sizeof(struct flow_data_common)];
				if(c->nat_flags & 0x5) { // snat || userid
					inet_ntop(AF_INET,&v4->ip_s,a1,sizeof(a1)-1);
					snprintf(p1,sizeof(p1)-1,"%d",htons(v4->sport));
					inet_ntop(AF_INET,&v4->ip_snat,a3,sizeof(a3)-1);
					snprintf(p3,sizeof(p3)-1,"%d",htons(v4->sport_nat));
					
				} else {
					inet_ntop(AF_INET,&v4->ip_s,a1,sizeof(a1)-1);
					snprintf(p1,sizeof(p1)-1,"%d",htons(v4->sport));
					a3[0] = '\0';
					p3[0] = '\0';
				}
				if(c->nat_flags & 2) { // dnat
					inet_ntop(AF_INET,&v4->ip_dnat,a2,sizeof(a2)-1);
					snprintf(p2,sizeof(p2)-1,"%d",htons(v4->dport_nat));
					inet_ntop(AF_INET,&v4->ip_d,a4,sizeof(a4)-1);
					snprintf(p4,sizeof(p4)-1,"%d",htons(v4->dport));
				} else {
					inet_ntop(AF_INET,&v4->ip_d,a2,sizeof(a2)-1);
					snprintf(p2,sizeof(p2)-1,"%d",htons(v4->dport));
					a4[0] = '\0';
					p4[0] = '\0';
				}
			}

			l = snprintf(buff,sizeof(buff)-1,"%u %u %c %d %s %s %s %s %" PRIu64 " %" PRIu64 " %u %u",
				c->time_start, c->time_end,
				c->family ? '6':'4', c->proto, a1, p1, a2, p2,
				c->b[0], c->b[1], c->p[0], c->p[1]);

			pn[0] = '\0';
			if(!ndpi_last_proto)
				ndpi_get_proto_names();
			if(c->proto_app) {
				if(c->proto_master && c->proto_master != c->proto_app) {
					snprintf(pn,sizeof(pn)-1,"%s,%s",
						ndpi_proto_name(c->proto_app),
						ndpi_proto_name(c->proto_master));
				} else
					strncpy(pn,ndpi_proto_name(c->proto_app),sizeof(pn)-1);
			} else if(c->proto_master)
					strncpy(pn,ndpi_proto_name(c->proto_master),sizeof(pn)-1);

			if(c->ifidx != c->ofidx)
				l += snprintf(&buff[l],sizeof(buff)-1-l," I=%d,%d",c->ifidx,c->ofidx);
			  else
				l += snprintf(&buff[l],sizeof(buff)-1-l," I=%d",c->ifidx);

			if(c->nat_flags) {
				if(c->nat_flags & 5)
					l += snprintf(&buff[l],sizeof(buff)-1-l," %s=%s,%s",
							c->nat_flags & 1 ? "SN":"UI", a3,p3);
				if(c->nat_flags & 2)
					l += snprintf(&buff[l],sizeof(buff)-1-l," DN=%s,%s",a4,p4);
			}
			l += snprintf(&buff[l],sizeof(buff)-1-l," P=%s",pn[0] ? pn : "Unknown");

			if(!c->extflag) {
			    if(c->host_len)
				l += snprintf(&buff[l],sizeof(buff)-1-l,
						" H=%.*s",c->host_len,&data[offs+rl+c->opt_len]);
			    if(c->opt_len)
				l += snprintf(&buff[l],sizeof(buff)-1-l,
						" J=%.*s",c->opt_len,&data[offs+rl]);
			} else {
			    if(c->host_len + c->opt_len)
				l += snprintf(&buff[l],sizeof(buff)-1-l,
						" %s%.*s", c->host_len ? "H=":"",
						c->host_len+c->opt_len,&data[offs+rl]);
			}
			buff[l++] = '\n';
			buff[l] = '\0';
			(void)write(fd,buff,l);
			offs += rl + c->opt_len + c->host_len;
			break;
		}
	}
	return offs != dump->len ? -1:0;
}

void help(void) {
	fprintf(stderr,"ndpi_flow_dump [-v] [-m mode] [-i input_binary_file] [-s] [-S output_biary_file]\n"
	"  -v             Verbose + 1\n"
	"  -m closed|flows Set read mode. Default 'read_all'\n"
	"  -s             Human readable output (stdout)\n"
	"  -S file        Write binary data to 'file'\n"
	"  -i file        Read binary data from the 'file' instead /proc/net/xt_ndpi/flows\n"
	);
	exit(1);
}

int main(int argc,char **argv) {
	int fd,e,n;
	size_t blk_size;
	long long int r;
	struct dump_data *c;
	char *src_file= NULL;
	char *bin_file= NULL;
	char *read_mode="read_all_bin\n";
	int text_dump = 0;
	struct stat src_st;
	struct timeval tv1,tv2;
	long int delta;
	int flow_flags = 0;

	while((n=getopt(argc,argv,"vsS:i:m:")) != -1) {
	  switch(n) {
	      case 'v': verbose++; break;
	      case 's': text_dump = 1; break;
	      case 'S': bin_file  = strdup(optarg); break;
	      case 'i': src_file  = strdup(optarg); break;
	      case 'm':
			if(!strcmp(optarg,"closed"))
				read_mode="read_closed_bin\n";
			else if(!strcmp(optarg,"flows"))
				read_mode="read_flows_bin\n";
			else help();
			break;
	      default: help();
	  }
	}
	if(!text_dump && !bin_file) {
		fprintf(stderr,"-s or -S required!\n");
		exit(1);
	}
	if(!src_file) {
		ndpi_get_proto_names();
		fd = open("/proc/net/xt_ndpi/flows",O_RDWR);
		if(fd < 0) {
			perror("open /proc/net/xt_ndpi/flows");
			exit(1);
		}
		if(write(fd,read_mode,strlen(read_mode)) != strlen(read_mode)) {
			perror("Set mode failed");
			close(fd);
			exit(1);
			exit(1);
		}
	} else {
		fd = open(src_file ,O_RDONLY);
		if(fd < 0) {
			perror("open flows");
			exit(1);
		}
		if(fstat(fd,&src_st) < 0) {
			perror("stat");
			exit(1);
		}
		if(!src_st.st_size) {
			exit(0);
		}
	}
	blk_size = FLOW_READ_COUNT;
	c = NULL;
	n = 0;
	e = 0;
	r = 0;
	gettimeofday(&tv1,NULL);
	while(1) {
		if(!c) 
			c = calloc(1,sizeof(struct dump_data)+blk_size+FLOW_PRE_HDR);
		if(!c) {
			perror("malloc");
			break;
		}
		c->next = NULL;
		c->offs = FLOW_PRE_HDR;
		e = read(fd,&c->data[FLOW_PRE_HDR],blk_size);
		if(e < 0) {
			if(errno == EINTR) continue;
			perror("read error");
			break;
		}
		if(e == 0) {
			free(c);
			break;
		}
		if(verbose)
			fprintf(stderr,"Block %d read %d bytes\n",n,e);
		r += e;
		c->len = e;
		n++;
		c->num = n;
		if(!head) {
			head = tail = c;
		} else {
			tail->next = c;
			tail = c;
		}
		c = NULL;
	}
	close(fd);
	gettimeofday(&tv2,NULL);
	tv2.tv_sec -= tv1.tv_sec;
	delta = tv2.tv_sec*1000000 + tv2.tv_usec - tv1.tv_usec;
	if(!delta) delta=1;

	if(verbose)
		fprintf(stderr,"read %llu bytes %ld ms, speed %d MB/s \n",r,delta/1000,(int)(r/delta));

	for(flow_flags = 0,n=0,c = head; c; n++,c = c->next ) {
		int e = check_flow_data(c);
		if(verbose > 1)
			fprintf(stderr,"part %d %s\n",n,e < 0 ? "BAD":"OK");
		if(e < 0) {
			fprintf(stderr,"Decode error.\n");
			exit(1);
		}
		flow_flags |= e;
	}

	if(bin_file && head) {
		struct stat st;
		if(stat(bin_file,&st) == 0 &&
			st.st_dev == src_st.st_dev &&
			st.st_ino == src_st.st_ino) {
			fprintf(stderr,"The input and output files are identical. Do not rewrited.\n");
		} else {
			fd = open(bin_file,O_CREAT|O_WRONLY,0644);
			if(fd < 0) {
				perror("create");
				exit(1);
			}

			if(!(flow_flags & REC_PROTO) && !ndpi_last_proto)
				ndpi_get_proto_names();
			if(!(flow_flags & REC_PROTO) && ndpi_last_proto > 0)
				write_proto_name(fd);
			for(c = head; c; c = c->next ) {
				e = write(fd,&c->data[c->offs],c->len);
				if(e != c->len) {
					perror("write");
					exit(1);
				}
			}
			close(fd);
		}
	}
	if(text_dump && head)
		for(c = head; c; c = c->next ) {
			if(decode_flow(1,c) < 0) {
				fprintf(stderr,"Decode error.\n");
				exit(1);
			}
		}

	for(c = head; c; c = head ) {
		head = c->next;
		free(c);
	}

	exit(0);
}
