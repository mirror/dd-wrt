#ifndef __BACKPORT_LINUX_STDDEF_H
#define __BACKPORT_LINUX_STDDEF_H
#include_next <linux/stddef.h>

#ifndef sizeof_field
/**
 * sizeof_field(TYPE, MEMBER)
 *
 * @TYPE: The structure containing the field of interest
 * @MEMBER: The field to return the size of
 */
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))
#endif

#ifndef offsetofend
/**
 * offsetofend(TYPE, MEMBER)
 *
 * @TYPE: The type of the structure
 * @MEMBER: The member within the structure to get the end offset of
 */
#define offsetofend(TYPE, MEMBER) \
	(offsetof(TYPE, MEMBER)	+ sizeof_field(TYPE, MEMBER))
#endif

#ifndef __DECLARE_FLEX_ARRAY
#ifdef __cplusplus
/* sizeof(struct{}) is 1 in C++, not 0, can't use C version of the macro. */
#define __DECLARE_FLEX_ARRAY(T, member)	\
	T member[0]
#else
/**
 * __DECLARE_FLEX_ARRAY() - Declare a flexible array usable in a union
 *
 * @TYPE: The type of each flexible array element
 * @NAME: The name of the flexible array member
 *
 * In order to have a flexible array member in a union or alone in a
 * struct, it needs to be wrapped in an anonymous struct with at least 1
 * named member, but that member can be empty.
 */
#define __DECLARE_FLEX_ARRAY(TYPE, NAME)	\
	struct { \
		struct { } __empty_ ## NAME; \
		TYPE NAME[]; \
	}
#endif
#define DECLARE_FLEX_ARRAY(TYPE, NAME) \
	__DECLARE_FLEX_ARRAY(TYPE, NAME)

#endif
#endif /* __BACKPORT_LINUX_STDDEF_H */
