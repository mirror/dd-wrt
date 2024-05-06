#ifndef SRC_USR_ARGP_LOG_H_
#define SRC_USR_ARGP_LOG_H_

#include "usr/util/result.h"

#if __GNUC__
#define CHECK_FORMAT(str, args) __attribute__((format(printf, str, args)))
#else
/*
 * No idea how this looks in other compilers.
 * It's safe to obviate since we're bound to see the warnings every time we use
 * GCC anyway.
 */
#define CHECK_FORMAT(str, args) /* Nothing */
#endif

/**
 * Wrappers for fprintf(stderr), intended for full lines.
 * Why do we need them? Because I fucking hate to have to append a newline at
 * the end of every fprintf.
 */
void pr_warn(const char *fmt, ...) CHECK_FORMAT(1, 2);
void pr_err(const char *fmt, ...) CHECK_FORMAT(1, 2);
int pr_result(struct jool_result *result);

#endif /* SRC_USR_ARGP_LOG_H_ */
