#ifndef __UTILS_H__
#define __UTILS_H__ 1

#include <asm/types.h>

extern int preferred_family;
extern int show_stats;
extern int show_details;
extern int show_raw;
extern int resolve_hosts;

#ifndef IPPROTO_ESP
#define IPPROTO_ESP	50
#endif
#ifndef IPPROTO_AH
#define IPPROTO_AH	51
#endif

#define SPRINT_BSIZE 64
#define SPRINT_BUF(x)	char x[SPRINT_BSIZE]


#define NEXT_ARG() \
argv++; \
if (--argc <= 0) \
	usage();

typedef struct
{
	__u8 family;
	__u8 bytelen;
	__s16 bitlen;
	__u32 data[4];
} inet_prefix;

extern __u32 get_addr32(char *name);
extern int get_addr_1(inet_prefix *dst, char *arg, int family);
extern int get_prefix_1(inet_prefix *dst, char *arg, int family);
extern int get_addr(inet_prefix *dst, char *arg, int family);
extern int get_prefix(inet_prefix *dst, char *arg, int family);

extern int scan_number(char *arg, unsigned *val);

extern int get_integer(int *val, char *arg, int base);
extern int get_unsigned(unsigned *val, char *arg, int base);
#define get_byte get_u8
#define get_ushort get_u16
#define get_short get_s16
extern int get_u32(__u32 *val, char *arg, int base);
extern int get_u16(__u16 *val, char *arg, int base);
extern int get_s16(__s16 *val, char *arg, int base);
extern int get_u8(__u8 *val, char *arg, int base);
extern int get_s8(__s8 *val, char *arg, int base);

extern int get_tc_classid(__u32 *h, char *str);
extern int print_tc_classid(char *buf, int len, __u32 h);
extern char * sprint_tc_classid(__u32 h, char *buf);

/* static void usage(void) __attribute__((noreturn)); */
void invarg(char *) __attribute__((noreturn));
int matches(char *arg, char *pattern);
extern int inet_addr_match(inet_prefix *a, inet_prefix *b, int bits);

extern int ipaddr_list(int argc, char **argv);
extern int iproute_monitor(int argc, char **argv);
extern int do_ipaddr(int argc, char **argv);
extern int do_iproute(int argc, char **argv);
extern int do_iprule(int argc, char **argv);
extern int do_ipneigh(int argc, char **argv);
extern int do_iptunnel(int argc, char **argv);
extern int do_iplink(int argc, char **argv);
extern int do_ipmonitor(int argc, char **argv);
extern int do_multiaddr(int argc, char **argv);
extern int do_qdisc(int argc, char **argv);
extern int do_class(int argc, char **argv);
extern int do_filter(int argc, char **argv);

extern const char *format_host(int af, void *addr, __u8 *abuf, int alen);

#endif /* __UTILS_H__ */
