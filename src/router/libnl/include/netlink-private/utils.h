/*
 * netlink-private/utils.h	Local Utility Functions
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_UTILS_PRIV_H_
#define NETLINK_UTILS_PRIV_H_

#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define ntohll(x) (x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define ntohll(x) bswap_64((x))
#endif
#define htonll(x) ntohll(x)

/*****************************************************************************/

#define _NL_STRINGIFY_ARG(contents)       #contents
#define _NL_STRINGIFY(macro_or_string)    _NL_STRINGIFY_ARG (macro_or_string)

/*****************************************************************************/

#if defined (__GNUC__)
#define _NL_PRAGMA_WARNING_DO(warning)       _NL_STRINGIFY(GCC diagnostic ignored warning)
#elif defined (__clang__)
#define _NL_PRAGMA_WARNING_DO(warning)       _NL_STRINGIFY(clang diagnostic ignored warning)
#endif

/* you can only suppress a specific warning that the compiler
 * understands. Otherwise you will get another compiler warning
 * about invalid pragma option.
 * It's not that bad however, because gcc and clang often have the
 * same name for the same warning. */

#if defined (__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#define _NL_PRAGMA_WARNING_DISABLE(warning) \
        _Pragma("GCC diagnostic push") \
        _Pragma(_NL_PRAGMA_WARNING_DO("-Wpragmas")) \
        _Pragma(_NL_PRAGMA_WARNING_DO(warning))
#elif defined (__clang__)
#define _NL_PRAGMA_WARNING_DISABLE(warning) \
        _Pragma("clang diagnostic push") \
        _Pragma(_NL_PRAGMA_WARNING_DO("-Wunknown-warning-option")) \
        _Pragma(_NL_PRAGMA_WARNING_DO(warning))
#else
#define _NL_PRAGMA_WARNING_DISABLE(warning)
#endif

#if defined (__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#define _NL_PRAGMA_WARNING_REENABLE \
    _Pragma("GCC diagnostic pop")
#elif defined (__clang__)
#define _NL_PRAGMA_WARNING_REENABLE \
    _Pragma("clang diagnostic pop")
#else
#define _NL_PRAGMA_WARNING_REENABLE
#endif

/*****************************************************************************/

#define _nl_unused                  __attribute__ ((__unused__))
#define _nl_auto(fcn)               __attribute__ ((__cleanup__(fcn)))

/*****************************************************************************/

#define _NL_STATIC_ASSERT(cond) ((void) sizeof (char[(cond) ? 1 : -1]))

/*****************************************************************************/

#if defined(NL_MORE_ASSERTS) && NL_MORE_ASSERTS > 0
#define _nl_assert(cond) assert(cond)
#else
#define _nl_assert(cond) do { if (0) { assert(cond); } } while (0)
#endif

/*****************************************************************************/

#define _NL_AUTO_DEFINE_FCN_VOID0(CastType, name, func) \
static inline void name (void *v) \
{ \
	if (*((CastType *) v)) \
		func (*((CastType *) v)); \
}

#define _nl_auto_free _nl_auto(_nl_auto_free_fcn)
_NL_AUTO_DEFINE_FCN_VOID0 (void *, _nl_auto_free_fcn, free)

/*****************************************************************************/

extern const char *nl_strerror_l(int err);

/*****************************************************************************/

/* internal macro to calculate the size of a struct @type up to (and including) @field.
 * this will be used for .minlen policy fields, so that we require only a field of up
 * to the given size. */
#define _nl_offsetofend(type, field) (offsetof (type, field) + sizeof (((type *) NULL)->field))

/*****************************************************************************/

#define _nl_clear_pointer(pp, destroy) \
	({ \
		__typeof__ (*(pp)) *_pp = (pp); \
		__typeof__ (*_pp) _p; \
		int _changed = 0; \
		\
		if (   _pp \
			&& (_p = *_pp)) { \
			_nl_unused const void *const _p_check_is_pointer = _p; \
			\
			*_pp = NULL; \
			\
			(destroy) (_p); \
			\
			_changed = 1; \
		} \
		_changed; \
	})

#define _nl_clear_free(pp) _nl_clear_pointer (pp, free)

#define _nl_steal_pointer(pp) \
	({ \
		__typeof__ (*(pp)) *const _pp = (pp); \
		__typeof__ (*_pp) _p = NULL; \
		\
		if (   _pp \
		    && (_p = *_pp)) { \
			*_pp = NULL; \
		} \
		\
		_p; \
	})

/*****************************************************************************/

#define _nl_malloc_maybe_a(alloca_maxlen, bytes, to_free) \
	({ \
		const size_t _bytes = (bytes); \
		__typeof__ (to_free) _to_free = (to_free); \
		__typeof__ (*_to_free) _ptr; \
		\
		_NL_STATIC_ASSERT ((alloca_maxlen) <= 500); \
		_nl_assert (_to_free && !*_to_free); \
		\
		if (_bytes <= (alloca_maxlen)) { \
			_ptr = alloca (_bytes); \
		} else { \
			_ptr = malloc (_bytes); \
			*_to_free = _ptr; \
		}; \
		\
		_ptr; \
	})

/*****************************************************************************/

static inline char *
_nl_strncpy_trunc(char *dst, const char *src, size_t len)
{
	/* we don't use/reimplement strlcpy(), because we want the fill-all-with-NUL
	 * behavior of strncpy(). This is just strncpy() with gracefully handling trunction
	 * (and disabling the "-Wstringop-truncation" warning).
	 *
	 * Note that truncation is silently accepted.
	 */

	_NL_PRAGMA_WARNING_DISABLE ("-Wstringop-truncation");
	_NL_PRAGMA_WARNING_DISABLE ("-Wstringop-overflow");

	if (len > 0) {
		_nl_assert(dst);
		_nl_assert(src);

		strncpy(dst, src, len);

		dst[len - 1] = '\0';
	}

	_NL_PRAGMA_WARNING_REENABLE;
	_NL_PRAGMA_WARNING_REENABLE;

	return dst;
}

static inline char *
_nl_strncpy(char *dst, const char *src, size_t len)
{
	/* we don't use/reimplement strlcpy(), because we want the fill-all-with-NUL
	 * behavior of strncpy(). This is just strncpy() with gracefully handling trunction
	 * (and disabling the "-Wstringop-truncation" warning).
	 *
	 * Note that truncation is still a bug and there is an _nl_assert()
	 * against that.
	 */

	if (len > 0) {
		_nl_assert(dst);
		_nl_assert(src);

		strncpy(dst, src, len);

		/* Truncation is a bug and we assert against it. But note that this
		 * assertion is disabled by default because we cannot be sure that
		 * there are not wrong uses of _nl_strncpy() where truncation might
		 * happen (wrongly!!). */
		_nl_assert (memchr(dst, '\0', len));

		dst[len - 1] = '\0';
	}

	return dst;
}

#endif
