#ifndef LIBNFTNL_STR_ARRAY_H
#define LIBNFTNL_STR_ARRAY_H 1

#include <stdint.h>

struct nlattr;

struct nftnl_str_array {
	char		**array;
	uint32_t	len;
};

void nftnl_str_array_clear(struct nftnl_str_array *sa);
int nftnl_str_array_set(struct nftnl_str_array *sa, const char * const *array);
int nftnl_parse_devs(struct nftnl_str_array *sa, const struct nlattr *nest);

#define nftnl_str_array_foreach(ptr, sa, idx)	\
	for (idx = 0, ptr = (sa)->array[idx];	\
	     idx < (sa)->len;			\
	     ptr = (sa)->array[++idx])

#endif /* LIBNFTNL_STR_ARRAY_H */
