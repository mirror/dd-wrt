#ifndef LIBNFTNL_UTILS_H
#define LIBNFTNL_UTILS_H 1

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <libnftnl/common.h>

#include "config.h"
#ifdef HAVE_VISIBILITY_HIDDEN
#	define __visible	__attribute__((visibility("default")))
#	define EXPORT_SYMBOL(x)	typeof(x) (x) __visible;
#else
#	define __visible
#	define EXPORT_SYMBOL
#endif

#define __noreturn	__attribute__((__noreturn__))

#define xfree(ptr)	free((void *)ptr);

#define div_round_up(n, d)	(((n) + (d) - 1) / (d))

void __noreturn __abi_breakage(const char *file, int line, const char *reason);

#define abi_breakage()	\
	__abi_breakage(__FILE__, __LINE__, strerror(errno));

void __nftnl_assert_fail(uint16_t attr, const char *filename, int line);

#define nftnl_assert(val, attr, expr)			\
  ((!val || expr)					\
   ? (void)0						\
   : __nftnl_assert_fail(attr, __FILE__, __LINE__))

#define nftnl_assert_validate(data, _validate_array, _attr, _data_len)		\
({										\
	if (!data)								\
		__nftnl_assert_fail(_attr, __FILE__, __LINE__);			\
	if (_validate_array[_attr])						\
		nftnl_assert(data, _attr, _validate_array[_attr] == _data_len);	\
})

void __nftnl_assert_attr_exists(uint16_t attr, uint16_t attr_max,
				const char *filename, int line);

#define nftnl_assert_attr_exists(_attr, _attr_max)					\
({											\
	if (_attr > _attr_max)								\
		__nftnl_assert_attr_exists(_attr, _attr_max, __FILE__, __LINE__);	\
})

#define SNPRINTF_BUFFER_SIZE(ret, remain, offset)	\
	if (ret < 0)					\
		ret = 0;				\
	offset += ret;					\
	if (ret > remain)				\
		ret = remain;				\
	remain -= ret;					\


#define BUILD_BUG_ON_ZERO(e)	(sizeof(char[1 - 2 * !!(e)]) - 1)

#define __must_be_array(a) \
	BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))

#define array_size(arr)		(sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

const char *nftnl_family2str(uint32_t family);

const char *nftnl_verdict2str(uint32_t verdict);

enum nftnl_cmd_type nftnl_flag2cmd(uint32_t flags);

int nftnl_fprintf(FILE *fpconst, const void *obj, uint32_t cmd, uint32_t type,
		  uint32_t flags,
		  int (*snprintf_cb)(char *buf, size_t bufsiz, const void *obj,
			  	     uint32_t cmd, uint32_t type,
				     uint32_t flags));

int nftnl_set_str_attr(const char **dptr, uint32_t *flags,
		       uint16_t attr, const void *data, uint32_t data_len);

struct nlattr;

void nftnl_attr_put_ifname(struct nlmsghdr *nlh, const char *ifname);
char *nftnl_attr_get_ifname(const struct nlattr *attr);

int nftnl_parse_str_attr(const struct nlattr *tb, int attr,
			 const char **field, uint32_t *flags);

static inline uint32_t bytesum(uint8_t *buf, size_t buflen)
{
	uint32_t ret = 0;

	while (buflen--)
		ret += buf[buflen];

	return ret;
}

#endif
