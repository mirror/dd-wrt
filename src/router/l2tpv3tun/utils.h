#ifndef __UTILS_H__
#define __UTILS_H__ 1

#include <asm/types.h>
#include <resolv.h>
#include <stdlib.h>


#define SPRINT_BSIZE 64
#define SPRINT_BUF(x)	char x[SPRINT_BSIZE]

extern void incomplete_command(void) __attribute__((noreturn));

#define NEXT_ARG() do { argv++; if (--argc <= 0) incomplete_command(); } while(0)
#define NEXT_ARG_OK() (argc - 1 > 0)
#define PREV_ARG() do { argv--; argc++; } while(0)

typedef struct
{
	__u8 family;
	__u8 bytelen;
	__s16 bitlen;
	__u32 flags;
	__u32 data[8];
} inet_prefix;

#define PREFIXLEN_SPECIFIED 1

extern __u32 get_addr32(const char *name);
extern int get_addr_1(inet_prefix *dst, const char *arg, int family);
extern int get_prefix_1(inet_prefix *dst, char *arg, int family);
extern int get_addr(inet_prefix *dst, const char *arg, int family);
extern int get_prefix(inet_prefix *dst, char *arg, int family);
extern int mask2bits(__u32 netmask);

extern int get_integer(int *val, const char *arg, int base);
extern int get_unsigned(unsigned *val, const char *arg, int base);
extern int get_jiffies(unsigned *val, const char *arg, int base, int *raw);
#define get_byte get_u8
#define get_ushort get_u16
#define get_short get_s16
extern int get_u64(__u64 *val, const char *arg, int base);
extern int get_u32(__u32 *val, const char *arg, int base);
extern int get_u16(__u16 *val, const char *arg, int base);
extern int get_s16(__s16 *val, const char *arg, int base);
extern int get_u8(__u8 *val, const char *arg, int base);
extern int get_s8(__s8 *val, const char *arg, int base);

extern char* hexstring_n2a(const __u8 *str, int len, char *buf, int blen);
extern __u8* hexstring_a2n(const char *str, __u8 *buf, int blen);

void missarg(const char *) __attribute__((noreturn));
void invarg(const char *, const char *) __attribute__((noreturn));
void duparg(const char *, const char *) __attribute__((noreturn));
void duparg2(const char *, const char *) __attribute__((noreturn));
int matches(const char *arg, const char *pattern);
extern int inet_addr_match(const inet_prefix *a, const inet_prefix *b, int bits);

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif /* __UTILS_H__ */
