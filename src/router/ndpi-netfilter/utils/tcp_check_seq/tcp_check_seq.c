#define __USE_XOPEN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <stdarg.h>

#include <pcap.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <net/ethernet.h>
#include <netinet/in.h>

int debug = 0;
int non_zero = 0;
int dump_seg = 0;
int info = 0;

struct pcap_pkthdr header; // The header that pcap gives us 

static int c_o[12]= {0,0,0,0,0,0,0,0,0,0,0,0},  c_ob = 0, f_large = 0, c_bad = 0;
static int c_oz[12]= {0,0,0,0,0,0,0,0,0,0,0,0};
static char *c_oo[12] = { "0", "1", "2", "3", "4", "5", "6", "7", "8-15", "16-63","64-255",">256"  };

static size_t pkt_cnt[2] = { 0,0 };
static size_t bytes_cnt[2] = { 0,0 };

#define TCP_BUF_LEN (10*1024*1024)
#define TCP_SG_NUM 4096
struct tseg {
    uint32_t    start,len,set;
};

struct tcp_buf {
    size_t      nseg;
    struct tseg list[TCP_SG_NUM];
    uint8_t     data[TCP_BUF_LEN];
    int         add;
    int         bad;
} tcp_data[2];

#define i_min(a,b) (a) < (b) ? (a):(b)

// hex dump {{{{
static char _bintohex[17]="0123456789abcdef";
static void u16toa(char *buf,uint8_t *payload) {
buf[0] = _bintohex[(payload[0] >> 4) & 0xf];
buf[1] = _bintohex[ payload[0] & 0xf];
buf[2] = _bintohex[(payload[1] >> 4) & 0xf];
buf[3] = _bintohex[ payload[1] & 0xf];
}

static void dump_hex(uint8_t *payload,int payload_len) {
        char buf[80];
        int i,j;
        buf[0] = 0;
        for(j =0, i=0; i < payload_len; i+=2) {
                j = i & 0xf;
                if(!j)
                    snprintf(buf,sizeof(buf),"%04x: %40s | %16s\n",i,"","................");
                u16toa(&buf[6+(j>>1)*5],payload+i);

                if(payload[i]   > ' ' && payload[i]   < 0x80) buf[48+j]=payload[i];
                if(payload[i+1] > ' ' && payload[i+1] < 0x80) buf[48+j+1]=payload[i+1];
                if(j == 0xe) fputs(buf,stdout);
        }
        if(j != 0xe) fputs(buf,stdout);
}
// }}}}

// print_ip4_hdr(),in_cksum(),nextproto4_cksum(), get_l3()  {{{{

static void print_ip4_hdr(uint8_t *pkt_ptr,size_t packet_length,const char *str) {
    char a_src[64],a_dst[64];

	struct ip *ip_hdr = (struct ip *)pkt_ptr; //point to an IP header structure 

	if(ip_hdr->ip_v == 4 && (ip_hdr->ip_p == 17 || ip_hdr->ip_p == 6)) {
		struct udphdr *uh = (void *)(pkt_ptr + ip_hdr->ip_hl*4);
		inet_ntop(AF_INET, &ip_hdr->ip_src, a_src,sizeof(a_src));
		inet_ntop(AF_INET, &ip_hdr->ip_dst, a_dst,sizeof(a_dst));
        if(1) 
		fprintf(stdout,"%ld %s %s:%d  %s:%d len %d %s\n",
                header.ts.tv_sec,
				ip_hdr->ip_p == 17 ? "UDP":"TCP",
				a_src,htons(uh->source),
				a_dst,htons(uh->dest), (int)packet_length, str);
        else
		fprintf(stdout,"%s and src host %s and src port %d and dst host %s and dst port %d len %d\n",
				ip_hdr->ip_p == 17 ? "UDP":"TCP",
				a_src,htons(uh->source),
				a_dst,htons(uh->dest), (int)packet_length);
	}
}

typedef unsigned short u16;
typedef unsigned long u32;
struct cksum_vec {
        const uint8_t   *ptr;
        int             len;
};

#define ADDCARRY(x)  {if ((x) > 65535) (x) -= 65535;}
#define REDUCE {l_util.l = sum; sum = l_util.s[0] + l_util.s[1]; ADDCARRY(sum);}

uint16_t
in_cksum(const struct cksum_vec *vec, int veclen)
{
	register const uint16_t *w;
	register int sum = 0;
	register int mlen = 0;
	int byte_swapped = 0;

	union {
		uint8_t		c[2];
		uint16_t	s;
	} s_util;
	union {
		uint16_t	s[2];
		uint32_t	l;
	} l_util;

	for (; veclen != 0; vec++, veclen--) {
		if (vec->len == 0)
			continue;
		w = (const uint16_t *)(void *)vec->ptr;
		if (mlen == -1) {
			/*
			 * The first byte of this chunk is the continuation
			 * of a word spanning between this chunk and the
			 * last chunk.
			 *
			 * s_util.c[0] is already saved when scanning previous
			 * chunk.
			 */
			s_util.c[1] = *(const uint8_t *)w;
			sum += s_util.s;
			w = (const uint16_t *)(void *)((const uint8_t *)w + 1);
			mlen = vec->len - 1;
		} else
			mlen = vec->len;
		/*
		 * Force to even boundary.
		 */
		if ((1 & (unsigned long) w) && (mlen > 0)) {
			REDUCE;
			sum <<= 8;
			s_util.c[0] = *(const uint8_t *)w;
			w = (const uint16_t *)(void *)((const uint8_t *)w + 1);
			mlen--;
			byte_swapped = 1;
		}
		/*
		 * Unroll the loop to make overhead from
		 * branches &c small.
		 */
		while ((mlen -= 32) >= 0) {
			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
			sum += w[4]; sum += w[5]; sum += w[6]; sum += w[7];
			sum += w[8]; sum += w[9]; sum += w[10]; sum += w[11];
			sum += w[12]; sum += w[13]; sum += w[14]; sum += w[15];
			w += 16;
		}
		mlen += 32;
		while ((mlen -= 8) >= 0) {
			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
			w += 4;
		}
		mlen += 8;
		if (mlen == 0 && byte_swapped == 0)
			continue;
		REDUCE;
		while ((mlen -= 2) >= 0) {
			sum += *w++;
		}
		if (byte_swapped) {
			REDUCE;
			sum <<= 8;
			byte_swapped = 0;
			if (mlen == -1) {
				s_util.c[1] = *(const uint8_t *)w;
				sum += s_util.s;
				mlen = 0;
			} else
				mlen = -1;
		} else if (mlen == -1)
			s_util.c[0] = *(const uint8_t *)w;
	}
	if (mlen == -1) {
		/* The last mbuf has odd # of bytes. Follow the
		   standard (the odd byte may be shifted left by 8 bits
		   or not as determined by endian-ness of the machine) */
		s_util.c[1] = 0;
		sum += s_util.s;
	}
	REDUCE;
	return (~sum & 0xffff);
}

uint16_t nextproto4_cksum(
                 const struct ip *ip, const uint8_t *data,
                 u_int len, u_int next_proto)
{
        struct phdr {
                uint32_t src;
                uint32_t dst;
                u_char mbz;
                u_char proto;
                uint16_t len;
        } ph;
        struct cksum_vec vec[2];

        /* pseudo-header.. */
        ph.len = htons((uint16_t)len);
        ph.mbz = 0;
        ph.proto = next_proto;
        memcpy(&ph.src, &ip->ip_src.s_addr, sizeof(uint32_t));
        memcpy(&ph.dst, &ip->ip_dst.s_addr, sizeof(uint32_t));

        vec[0].ptr = (const uint8_t *)(void *)&ph;
        vec[0].len = sizeof(ph);
        vec[1].ptr = data;
        vec[1].len = len;
        return (in_cksum(vec, 2));
}

uint16_t
in_cksum_shouldbe(uint16_t sum, uint16_t computed_sum)
{
        uint32_t shouldbe;

        shouldbe = sum;
        shouldbe += ntohs(computed_sum);
        shouldbe = (shouldbe & 0xFFFF) + (shouldbe >> 16);
        shouldbe = (shouldbe & 0xFFFF) + (shouldbe >> 16);
        return shouldbe;
}

uint8_t *get_l3(uint8_t *pkt_ptr) {
    int ether_offset;
    int ether_type;

    ether_offset = 12;

    while(1) {
        ether_type = ntohs( *(u_int16_t*)&pkt_ptr[ether_offset] );
        switch(ether_type) {
          case ETHERTYPE_ARP:
          case ETHERTYPE_LOOPBACK:
            return NULL;

          case ETHERTYPE_IP:
            ether_offset +=2; //skip past the Ethernet II header
            return pkt_ptr + ether_offset;
          case ETHERTYPE_IPV6:
            ether_offset +=2; //skip past the Ethernet II header
            return pkt_ptr + ether_offset;
          case ETHERTYPE_VLAN:
            ether_offset += 4;
            break;

          default:
            //fprintf(stderr, "Unknown ethernet type, %04X, skipping...\n", ether_type);
          return NULL;
        }
    }
    return NULL;
}

/* Copy from https://github.com/jafingerhut/p4-guide/tree/master/tcp-options-parser/look-for-ts-tcp-option */

#define TCP_OPTION_END_OF_OPTIONS 0    // End of Option List - RFC 793
#define TCP_OPTION_NOP            1    // No-Operation - RFC 793
#define TCP_OPTION_MSS            2    // Maximum Segment Size - RFC 793
#define TCP_OPTION_WINDOW_SCALE   3    // Window Scale - RFC 7323
#define TCP_OPTION_SACK_PERMITTED 4    // SACK Permitted - RFC 2018
#define TCP_OPTION_SACK           5    // SACK - RFC 2018
#define TCP_OPTION_TIMESTAMPS     8    // Timestamps - RFC 7323

typedef uint8_t bool;

#define TRUE (1)
#define FALSE (0)

static int tcp_st = 0;
static uint32_t src = 0; // IPv4 src
static uint32_t seq_start[2];
static uint32_t seq_next[2];


uint32_t get_uint32_at_byte_offset(uint8_t in_tcp_options[],
                                   int in_byte_offset)
{
    int i = in_byte_offset;
    uint32_t ret;

    ret = ((((uint32_t) in_tcp_options[i+0]) << 24) |
           (((uint32_t) in_tcp_options[i+1]) << 16) |
           (((uint32_t) in_tcp_options[i+2]) <<  8) |
           (((uint32_t) in_tcp_options[i+3]) <<  0));
    return ret;
}
uint16_t get_uint16_at_byte_offset(uint8_t in_tcp_options[],
                                   int in_byte_offset)
{
    int i = in_byte_offset;
    uint32_t ret;

    ret = ((((uint16_t) in_tcp_options[i+0]) <<  8) |
           (((uint16_t) in_tcp_options[i+1]) <<  0));
    return ret;
}
static char topt_txt[256];

void get_option_kind_info (uint8_t in_option_kind,
                      bool *out_known_option,
                      bool *out_kind_followed_by_length,
                      bool *out_fixed_length,
                      uint8_t *out_expected_fixed_length)
{
    *out_known_option = TRUE;
    switch (in_option_kind) {
    case TCP_OPTION_END_OF_OPTIONS:
        //strcat(topt_txt,"END ");
        *out_kind_followed_by_length = FALSE;
        break;
    case TCP_OPTION_NOP:
        //strcat(topt_txt,"NOOP ");
        *out_kind_followed_by_length = FALSE;
        break;
    case TCP_OPTION_MSS:
        strcat(topt_txt,"MSS");
        *out_kind_followed_by_length = TRUE;
        *out_fixed_length = TRUE;
        *out_expected_fixed_length = 4;
        break;
    case TCP_OPTION_WINDOW_SCALE:
        // strcat(topt_txt,"WS ");
        *out_kind_followed_by_length = TRUE;
        *out_fixed_length = TRUE;
        *out_expected_fixed_length = 4;
        break;
    case TCP_OPTION_SACK_PERMITTED:
        //strcat(topt_txt,"SACK_PERM ");
        *out_kind_followed_by_length = TRUE;
        *out_fixed_length = TRUE;
        *out_expected_fixed_length = 2;
        break;
    case TCP_OPTION_SACK:
        strcat(topt_txt,"SACK");
        *out_kind_followed_by_length = TRUE;
        *out_fixed_length = FALSE;
        break;
    case TCP_OPTION_TIMESTAMPS:
        //strcat(topt_txt,"TS ");
        *out_kind_followed_by_length = TRUE;
        *out_fixed_length = TRUE;
        *out_expected_fixed_length = 10;
        break;
    default:
        *out_known_option = FALSE;
        break;
    }
}

void parse_tcp_options(struct tcphdr *th,int dir) {
    uint8_t *tcp_options = (uint8_t *)th + sizeof(struct tcphdr);
    size_t options_length = th->th_off*4 - sizeof(struct tcphdr);
    size_t offset = 0;
    uint8_t option_kind;
    bool known_option;
    bool kind_followed_by_length;
    bool fixed_length;
    uint8_t expected_fixed_length;
    uint8_t option_len_bytes;

    bzero(topt_txt,sizeof(topt_txt));
    if(!options_length) return;
    while(offset < options_length) {
        option_kind = tcp_options[offset];
        get_option_kind_info(option_kind, &known_option,
                      &kind_followed_by_length, &fixed_length,
                      &expected_fixed_length);
        if (!known_option) break;
        if (kind_followed_by_length) {
            if ((offset + 1) >= options_length) {
                // malformed TCP options - fell off end of TCP options header
                break;
            }
            option_len_bytes = tcp_options[offset+1];
            if (fixed_length && (option_len_bytes != expected_fixed_length)) {
                // malformed TCP options - incorrect length
                break;
            }
            // This code assumes that if fixed_length is FALSE, the
            // length in the packet is correct.  For the SACK option,
            // it is possible to check that the option length is one
            // of a few legal values.  See parser state
            // 'parse_tcp_options_sack' in the program
            // tcp-options-parser.p4 for some details on this.
        } else {
            if(option_kind == TCP_OPTION_SACK) {
                option_len_bytes = tcp_options[offset+1];
            } else {
                option_len_bytes = 1;
            }
        }
        if ((offset + option_len_bytes) > options_length) {
            // option is too long to fit in packet's TCP options
            break;
        }
        if (option_kind == TCP_OPTION_MSS) {
            int l = strlen(topt_txt);
            snprintf(&topt_txt[l],sizeof(topt_txt)-1-l,":%u ",
                get_uint16_at_byte_offset(tcp_options,offset+2)
                );
        }
        if (option_kind == TCP_OPTION_SACK) {
            int l = strlen(topt_txt);
            snprintf(&topt_txt[l],sizeof(topt_txt)-1-l,":{%u:%u} ",
                get_uint32_at_byte_offset(tcp_options,offset+2)   - seq_start[dir^1],
                get_uint32_at_byte_offset(tcp_options,offset+2+4) - seq_start[dir^1]
                );
        }
        if (option_kind == TCP_OPTION_END_OF_OPTIONS) {
            // Stop if End of Options option is encountered.
            break;
        }
        offset = offset + option_len_bytes;
    }
}

char *tcp_flags(uint8_t f) {
    static char *_tcpf2t[7]={ "Fin","Syn","Rst","Push","Ack","Urg",NULL};
    static char _tcpf2tb[64];
	char l = 0;
	for(int i=0; i < 6;i++)
		if(f & (1 << i)) {
            if(l) _tcpf2tb[l++] = ',';
            l += snprintf(&_tcpf2tb[l],sizeof(_tcpf2tb)-l-1,"%s",_tcpf2t[i]);
        }
	return &_tcpf2tb[0];
}

char *print_seq_range(uint32_t seq,uint32_t len) {
    static char buf[24];
    snprintf(buf,sizeof(buf)-1,"seq:%u-%u",seq,seq+len);
    return buf;
}

// }}}}


void tcp_buf_init(void) {
    int i;
    bzero((char *)tcp_data,sizeof(tcp_data));
    for(i=0; i < 2; i++) {
        tcp_data[i].list[0].len = TCP_BUF_LEN;
        tcp_data[i].nseg = 1;
    }
}

struct tseg *get_seg_index(struct tcp_buf *td,size_t pos) {
    size_t i;
    for(i=0; i < td->nseg; i++) {
        if(td->list[i].start <= pos && pos < td->list[i].start+td->list[i].len)
            return &td->list[i];
    }
    return NULL;
}
void tcp_buf_seg_dump(struct tcp_buf *td) {
    size_t i=0;
    printf("Dump S:%d ",(int)td->nseg);
    if(!td->list[i].set && td->list[i].len == 1) i++;
    for(; i < td->nseg; i++)
        if((i < td->nseg - 1) || td->list[i].set)
            printf("%u.%u%c ",td->list[i].start,td->list[i].len,td->list[i].set ? '+':'-');
    i = td->nseg-1;
//    printf(" last %u\n",td->list[i].start+td->list[i].len);
    printf("\n");
    if(td->list[i].start+td->list[i].len != TCP_BUF_LEN) abort();
}
void tcp_buf_seg_split(struct tcp_buf *td,int seg , size_t len) {
    int i;
    if(td->nseg >= TCP_SG_NUM) {
        fprintf(stderr,"Too many segments\n");
        abort();
    }
    if(debug > 2)
        printf("%s seg %d %u.%u len %lu\n",__func__,seg,td->list[seg].start,td->list[seg].len,len);
    for(i=td->nseg; i > seg; i--) {
        td->list[i]=td->list[i-1];
    }
    td->nseg++;
    td->list[seg].len = len;
    td->list[seg+1].start += len;
    td->list[seg+1].len -= len;
    if(debug > 2) tcp_buf_seg_dump(td);
}

void tcp_buf_seg_join(struct tcp_buf *td,int s1, int s2) {
    int i,n;
    if(debug > 2)
        printf("%s seg %d-%d %u.%u -  %u.%u\n",__func__,s1,s2,
                td->list[s1].start,td->list[s1].len,
                td->list[s2].start,td->list[s2].len);
    n = s2-s1;
    td->list[s1].len = td->list[s2].start + td->list[s2].len - td->list[s1].start;
    for(i=s1+1; i+n < (int)td->nseg; i++) {
        td->list[i]=td->list[i+n];
    }
    td->nseg -= n;
    if(debug > 2) tcp_buf_seg_dump(td);
}

void seg_compact(struct tcp_buf *td) {
    int i,j;
//    if(td->nseg < TCP_SG_NUM*8/10) return;
    if(debug > 1)
        printf("compact start. seg %d\n",(int)td->nseg);
    i=0;j=1;
    while(j < (int)td->nseg) {
        if(td->list[i].set == td->list[j].set) {
            if(debug > 2) printf("compact ok %d:%u.%u%c + %d:%u.%u\n",i,td->list[i].start,td->list[i].len,td->list[i].set ? '+':'-',
                    j,td->list[j].start,td->list[j].len);
            td->list[i].len += td->list[j].len;
        } else {
            if(debug > 2) printf("compact next %d %d\n",i,j);
            i++;
            if(i < j) {
                td->list[i] = td->list[j];
            }
            if(debug > 2) printf("compact next start %d:%u.%u%c\n",i,td->list[i].start,td->list[i].len,td->list[i].set ? '+':'-');
        }
        j++;
    }
    if(i+1 != (int)td->nseg) {
        if(debug > 1)
            printf("compact end %d -> %d\n",(int)td->nseg,i+1);
        td->nseg = i+1;
        if(debug > 1)
            tcp_buf_seg_dump(td);
        td->add = 0;
    } else {
        if(debug > 1)
            printf("compact end\n");
    }
}



int test_in_seg(struct tseg *ts,size_t pos,size_t len) {
    if(pos < ts->start) return 0;
    if(pos > ts->start +ts->len) return 0;
    if(pos + len > ts->start +ts->len) return 0;
    return 1;
}

void tcp_buf_add_seg(struct tcp_buf *td, uint8_t *data, size_t data_len,size_t pos) {
    size_t i;
    if(!data_len) return;
    if(pos >= TCP_BUF_LEN) return;
    if(pos + data_len >= TCP_BUF_LEN) return;
    if(debug > 1) {
        printf("%s pos %lu len %lu\n",__func__,pos,data_len);
        tcp_buf_seg_dump(td);
    }
    for(i=0; i < td->nseg; i++)
        if(td->list[i].start <= pos && pos < td->list[i].start + td->list[i].len) 
            break;

    if(i == td->nseg) return;

    if(pos > td->list[i].start + td->list[i].len) {
        fprintf(stderr,"%s:%d pos > start + len\n",__func__,__LINE__);
        abort();
    }
    if(debug > 2)
        printf("found seg %lu\n",i);
    if(pos > td->list[i].start) {
        tcp_buf_seg_split(td,i,pos - td->list[i].start);
        i++;
    }

    if(pos == td->list[i].start) {
        if(data_len > td->list[i].len) { // overlap severals seg
            size_t j;
            if(debug > 2) printf("Overlap N-seg\n");
            for(j=i+1; j < td->nseg; j++) {
                if(pos+data_len < td->list[j].start) continue;

                if(pos+data_len > td->list[j].start + td->list[j].len) {
                    if(j == td->nseg-1) {
                        printf("BUG %lu > %u max %u\n",pos+data_len,td->list[j].start + td->list[j].len,TCP_BUF_LEN);
                        tcp_buf_seg_dump(td);
                        abort();
                    }
                    continue;
                }
                break;
            }
            if(debug > 1) {
                printf("start seg %lu %u:%u%c\n",i,td->list[i].start,td->list[i].start+td->list[i].len-1,td->list[i].set ? '+':'-');
                printf("next seg %lu %u:%u%c new seg %lu:%lu\n",j,td->list[j].start,td->list[j].start+td->list[j].len-1,td->list[j].set ? '+':'-',
                            pos,pos+data_len);
            }
            // TODO print overlaped segs
            if(pos+data_len  < td->list[j].start+td->list[j].len)
                tcp_buf_seg_split(td,j,pos+data_len - td->list[j].start);

            tcp_buf_seg_join(td,i,j);
            if(data_len > td->list[i].len) {
                fprintf(stderr,"%s:%d pos + data_len not found\n",__func__,__LINE__);
                abort();
            }
        }
        if(data_len < td->list[i].len)
            tcp_buf_seg_split(td,i,data_len);

        if(data_len == td->list[i].len) {
            if(td->list[i].set) { // overlap
                if(memcmp(&td->data[pos],data,data_len) == 0) {
                    if(debug > 1)
                      printf("Retransmit pos %lu len %lu ",pos,data_len);
                } else {
                    if(debug > 1)
                      printf("Overlap pos %lu len %lu ",pos,data_len);
                }
            } else {
                    if(debug > 1)
                       printf("SET DATA seg %ld pos %lu len %lu ",i,pos,data_len);
            }
            memcpy(&td->data[pos],data,data_len);
            td->list[i].set = 1;
            if(debug > 1) 
                tcp_buf_seg_dump(td);
            if(debug > 2)
                dump_hex(data,data_len);
            return;
        }
        fprintf(stderr,"%s:%d data_len != td->list[i].len\n",__func__,__LINE__);
        abort();
    }
    fprintf(stderr,"%s:%d pos != start\n",__func__,__LINE__);
    abort();
}

void copy_data(int dir,uint8_t *data, size_t data_len,size_t pos) {
    tcp_buf_add_seg(&tcp_data[dir],data,data_len,pos);
    tcp_data[dir].add++;
    if(tcp_data[dir].add > 20)
        seg_compact(&tcp_data[dir]);
}


int _pkt_data_eq(int dir, uint8_t *pdata, size_t len, size_t pos) {
    size_t i,offs;
    uint8_t *data;
    struct tseg *ts = get_seg_index(&tcp_data[dir],pos);

    if(!ts) return 0;
    
    offs = pos - ts->start;
    data = &tcp_data[dir].data[pos];
    if(debug > 1)
        printf("check start offs %lu pos %u.%u%c\n",offs,ts->start,ts->len,ts->set ? '+':'-');
    if(debug > 3) 
        dump_hex(data,len);
    for(i=0; i < len; i++,offs++) {
        if(offs > ts->len) {
            if(ts->start + ts->len >= TCP_BUF_LEN) return 0;
            ts++;
            offs = 0;
        }
        if(pdata) {
            if(ts->set && data[i] != pdata[i]) {
                if(debug > 3) {
                    printf("not match %lu\n",i);
                    dump_hex(pdata,len-i);
                }
                return 0;
            }
        } else {
            if(ts->set && data[i]) {
                if(debug > 3) {
                    printf("not zero %lu\n",i);
                    dump_hex(pdata,len-1);
                }
                return 0;
            }
        }
    }
    return 1;
}
int pkt_data_eq(int dir, uint8_t *pdata, size_t len, size_t pos) {
    int r = _pkt_data_eq(dir,pdata,len,pos);
    if(debug > 1)
        printf("check %s pos %lu.%lu = %d\n",pdata ? "eq":"zero",pos,len,r);
    return r;
}
void check_tcp_ovarlap(struct ip *ip_hdr, int pkt_len,
		struct tcphdr *th,
		uint8_t *tcp_data, int tcp_data_len) {

	int dir = 0;
	uint32_t th_seq = ntohl(th->th_seq);
	uint32_t th_ack = ntohl(th->th_ack);
	if(!src) 
		src = ip_hdr->ip_src.s_addr;
	dir = src != ip_hdr->ip_src.s_addr;
    parse_tcp_options(th,dir);
    pkt_cnt[dir]++;
    bytes_cnt[dir]+=tcp_data_len;
	if(tcp_st > 1 && debug) {
	  printf("%u %s [%-9s]  L=%-5u %-18s ack:%-8u next:%-8u %s\n",
            (unsigned int)header.ts.tv_sec,
            dir ? "<-":"->",
            tcp_flags(th->th_flags),
            tcp_data_len,
            print_seq_range(th_seq-seq_start[dir],tcp_data_len),
            th_ack-seq_start[dir ^ 1],
            seq_next[dir]- seq_start[dir],
            topt_txt
            );
	}
	switch(tcp_st) {
		case 0:
			if((th->th_flags & (TH_SYN|TH_ACK)) != TH_SYN || dir) break;
			seq_next[dir] = seq_start[dir] = th_seq;
			tcp_st = 1;
            if(debug) {
	            print_ip4_hdr((uint8_t *)ip_hdr,pkt_len,tcp_flags(th->th_flags));
                printf("\t   Seq_start -> %u",seq_start[0]);
                if(tcp_data_len != 0)
                    printf(" tcp_data_len %u\n",tcp_data_len);
                printf("\n");
            }
			break;
		case 1:
			if(th->th_flags != (TH_SYN|TH_ACK) || !dir) break;
			seq_next[dir] = seq_start[dir] = th_seq;
			tcp_st = 2;
            if(debug) {
	            print_ip4_hdr((uint8_t *)ip_hdr,pkt_len,tcp_flags(th->th_flags));
                printf("\t   Seq_start <- %u",seq_start[1]);
                if(tcp_data_len != 0)
                    printf(" tcp_data_len %u\n",tcp_data_len);
                printf("\n");
            }
			break;
		case 2:
			if((th->th_flags & TH_ACK) != TH_ACK || dir) break;
			seq_next[dir] = th_seq + tcp_data_len;
            copy_data(dir,tcp_data, tcp_data_len,th_seq-seq_start[dir]);
			tcp_st = 3;
			break;
		case 3:
			if(th->th_flags & (TH_FIN|TH_RST)) {
				tcp_st = 4;
				break;
			}
			if((th->th_flags & TH_ACK) != TH_ACK) break;

			if(th_seq < seq_next[dir] && tcp_data_len != 0) {
                int is_z = 0;
                int all = th_seq + tcp_data_len == seq_next[dir];
				int s = (int32_t)seq_next[dir]-(int32_t)th_seq;
                do {
    				if(s < 0) {
                        if(debug)
    	    				printf("\t   Overlap %d BUG!\n",s);
		    		    c_ob++;
		    		    c_o[0]++;
                        break;
			    	}
                    if( pkt_data_eq(dir,tcp_data,tcp_data_len,th_seq-seq_start[dir])) {
                        if(!pkt_data_eq(dir,NULL,tcp_data_len,th_seq-seq_start[dir])) {
                            c_oz[0]++;
                            if(debug)
                                printf("\t   Retransmit %d\n",s);
                            break;
                        }
                    }
                    if(s < 8) { 
                        is_z = pkt_data_eq(dir,NULL,s,th_seq-seq_start[dir]);
                        if(is_z) c_oz[s]++;
                          else    c_o[s]++;
                        c_o[0]++;
                        if(debug) {
                            if(s == (i_min(s,tcp_data_len)))
                                printf("\t   Overlaped %d %s bytes\n",s,is_z ? "zero":"nonzero");
                              else
                                printf("\t   Overlaped %d %s bytes offset -%d\n",i_min(s,tcp_data_len),is_z ? "zero":"nonzero",s);
                        }
                        break;
                    }

                    if(s < 16) c_o[8]++;
                    else if(s < 64) c_o[9]++;
                    else if(s < 256) c_o[10]++;
                    else if(s < 1536) c_o[11]++;
                    else { c_o[0]--; }
                    if(debug) {
                        if(all)
                            printf("\t   Overlaped %d bytes\n",i_min(s,tcp_data_len));
                          else
                            printf("\t   Overlaped %d bytes offset -%d\n",tcp_data_len,s);
                    }
                    c_o[0]++;
                } while(0);
			}
			seq_next[dir] = th_seq + tcp_data_len;
            copy_data(dir,tcp_data, tcp_data_len, th_seq-seq_start[dir]);
            if(th_seq-seq_start[dir] >= TCP_BUF_LEN) {
                if(debug)
                    printf("\t   Stream too large (%u bytes). STOP\n",TCP_BUF_LEN);
                tcp_st = 4;
                f_large = 1;
            }
			break;
		case 4:
			break;
	}
}

void usage(const char *msg) {
	fprintf(stderr,"%s\ntcp_check_seq [-d] [-n] [-s] pcap_files\n",msg);
	exit(1);
}

int main(int argc, char **argv) 
{ 
    int fnum;
    int c;

    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE]; //not sure what to do with this, oh well 

    const u_char *packet; // The actual packet 
    struct ip *ip_hdr;
    size_t packet_length;

    while((c = getopt(argc,argv,"+dnsi")) != -1) {
        switch(c) {
          case 'd': debug++; break;
          case 'n': non_zero++; break;
          case 's': dump_seg++; break;
          case 'i': info++; break;
	      default: usage("Bad option");
	    }
    }

    for (fnum=optind; fnum < argc; fnum++) {  


        bzero((char *)c_o,sizeof(c_o));
        bzero((char *)c_oz,sizeof(c_oz));
        bzero((char *)pkt_cnt,sizeof(pkt_cnt));
        bzero((char *)bytes_cnt,sizeof(bytes_cnt));
        c_ob = 0;
        c_bad = 0;
        tcp_st = 0;
        f_large = 0;
        src = 0; // IPv4 src
        seq_start[0] = seq_start[1] = 0;
        seq_next[0] = seq_next[1] = 0;
        tcp_buf_init();

        handle = pcap_open_offline(argv[fnum], errbuf);
     
        if (handle == NULL) { 
            fprintf(stderr,"Couldn't open pcap file %s: %s\n", argv[fnum], errbuf); 
            continue;
        } 
        if(pcap_datalink(handle) != DLT_EN10MB) {
            fprintf(stderr,"file %s is not DLT_EN10MB \n", argv[fnum]); 
            continue;
        }

        while ((packet = pcap_next(handle,&header)) != NULL) { 

            u_char *pkt_ptr = get_l3((uint8_t*)packet);

            if(!pkt_ptr) continue;

            ip_hdr = (struct ip *)pkt_ptr;
            if(ip_hdr->ip_v != 4) continue;
            if(ip_hdr->ip_p != 6) continue;
            packet_length = ntohs(ip_hdr->ip_len); 
            if(packet_length > header.caplen) {
                fprintf(stderr,"BUG! ip_len(%lu) > header.caplen(%u)\n",
                        packet_length,(unsigned int)header.caplen);
                continue;
            }
            if(ip_hdr->ip_v != 4) continue;
            if(packet_length < 40) continue;
            {
                struct tcphdr *th = (void *)(pkt_ptr + ip_hdr->ip_hl*4);
                int tcp_len = packet_length - ip_hdr->ip_hl*4;
                int tcp_data_len = tcp_len - th->th_off*4;
                uint8_t *tcp_data = (uint8_t *)th + th->th_off*4;
                check_tcp_ovarlap(ip_hdr,packet_length,th,tcp_data,tcp_data_len);
            }
            if(tcp_st == 4) break;

        }
        {
            struct tcp_buf *td;
            int i;
            for(td = &tcp_data[0],i=0;i < 2; td++,i++) {
                seg_compact(td);
                if(dump_seg) {
                    tcp_buf_seg_dump(td);
                }
                if(td->nseg > 1) {
                if(td->nseg != 3) td->bad = 1;
                if(!td->bad && td->list[0].len != 1) td->bad = 1;
                if(!td->bad && td->list[0].set) td->bad = 1;
                if(!td->bad && !td->list[1].set) td->bad = 1;
                if(td->bad) c_bad = 1;
                }
            }
        }
     
        pcap_close(handle);
        if(non_zero) {
            int i,s;
            for(i=1,s=0; i <= 11; i++) s+=c_o[i];
            if(!s) continue;
        }
        if(c_o[0] || c_oz[0] || c_ob || debug || f_large || c_bad) {
          int i;
          printf("%s ", argv[fnum]);
          printf("%d overlaps. ",c_o[0]);
          printf("%d retrans. ",c_oz[0]);
          for(i=1; i < 8; i++) {
              if(c_o[i]) printf("n%s:%d ",c_oo[i],c_o[i]);
              if(c_oz[i]) printf("z%s:%d ",c_oo[i],c_oz[i]);
          }
          for(i=8; i <= 11; i++) if(c_o[i]) printf("X_%s:%d ",c_oo[i],c_o[i]);
          if(f_large)
              printf("BIGFILE ");
          if(c_bad)
              printf("BADSTREAM ");
          if(info)
              printf("CNT %lu %lu %lu %lu ", bytes_cnt[0],bytes_cnt[1], pkt_cnt[0], pkt_cnt[1]);
          if(c_ob)
              printf("BUG!");
        } else {
          printf("%s OK", argv[fnum]);
          if(info)
              printf(" CNT %lu %lu %lu %lu", bytes_cnt[0],bytes_cnt[1], pkt_cnt[0], pkt_cnt[1]);
        }
        printf("\n");
    } //end for loop through each command line argument 

    return c_bad + c_ob ? 1:0;
}

/* vim: set ts=4 sw=4 et foldmarker={{{{,}}}} foldmethod=marker : */
