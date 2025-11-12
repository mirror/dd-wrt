#ifndef NFTABLES_FIB_H
#define NFTABLES_FIB_H

#include <linux/netfilter/nf_tables.h>

extern const char *fib_result_str(const struct expr *expr, bool check);
extern struct expr *fib_expr_alloc(const struct location *loc,
				   unsigned int flags,
				   unsigned int result);
extern const struct datatype fib_addr_type;

#endif /* NFTABLES_FIB_H */
