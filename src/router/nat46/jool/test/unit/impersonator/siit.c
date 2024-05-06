#include "common/types.h"
#include "mod/common/address_xlat.h"
#include "mod/common/packet.h"
#include "mod/common/db/denylist4.h"
#include "mod/common/db/eam.h"
#include "mod/common/db/rfc6791v4.h"
#include "mod/common/db/rfc6791v6.h"

/**
 * @file
 * SIIT-specific functions, as linked by NAT64 code.
 *
 * These are all supposed to be unreachable code, so they're very noisy on the
 * kernel log.
 */

static int fail(const char *function_name)
{
	WARN(true, "%s() was called from NAT64 code.", function_name);
	return -EINVAL;
}

struct addr4_pool *denylist4_alloc(void)
{
	fail(__func__);
	return NULL;
}

void denylist4_get(struct addr4_pool *pool)
{
	fail(__func__);
}

void denylist4_put(struct addr4_pool *pool)
{
	fail(__func__);
}

int rfc6791v4_find(struct xlation *state, struct in_addr *result)
{
	return fail(__func__);
}

int rfc6791v6_find(struct xlation *state, struct in6_addr *result)
{
	return fail(__func__);
}

struct eam_table *eamt_alloc(void)
{
	fail(__func__);
	return NULL;
}

void eamt_get(struct eam_table *eamt)
{
	fail(__func__);
}

void eamt_put(struct eam_table *eamt)
{
	fail(__func__);
}

bool eamt_contains4(struct eam_table *eamt, __be32 addr)
{
	fail(__func__);
	return false;
}

static struct addrxlat_result fail_addr(void)
{
	static const struct addrxlat_result result = {
		.verdict = ADDRXLAT_DROP,
		.reason = "Stateful NAT64 doesn't do stateless address translation.",
	};

	fail(__func__);
	return result;
}

struct addrxlat_result addrxlat_siit64(struct xlator *instance,
		struct in6_addr *in, struct result_addrxlat64 *out,
		bool enable_denylists)
{
	return fail_addr();
}

struct addrxlat_result addrxlat_siit46(struct xlator *instance,
		__be32 in, struct result_addrxlat46 *out,
		bool enable_eam, bool enable_denylists)
{
	return fail_addr();
}

verdict translate_addrs64_siit(struct xlation *state)
{
	fail(__func__);
	return VERDICT_DROP;
}

verdict translate_addrs46_siit(struct xlation *state)
{
	fail(__func__);
	return VERDICT_DROP;
}

bool is_hairpin_siit(struct xlation *state)
{
	fail(__func__);
	return false;
}

verdict handling_hairpinning_siit(struct xlation *old)
{
	fail(__func__);
	return VERDICT_DROP;
}
