#include "mod/common/address_xlat.h"

#include "mod/common/address.h"
#include "mod/common/rfc6052.h"
#include "mod/common/db/denylist4.h"
#include "mod/common/db/eam.h"

static bool is_illegal_source(struct in6_addr *src)
{
	/*
	 * The RFC does not define "illegal source address"...
	 * TODO (warning) think about this.
	 */
	return (src->s6_addr32[0] == 0)
			&& (src->s6_addr32[1] == 0)
			&& (src->s6_addr32[2] == 0)
			&& (be32_to_cpu(src->s6_addr32[3]) == 1);
}

static bool must_not_translate(struct in_addr *addr, struct net *ns)
{
	return addr4_is_scope_subnet(addr->s_addr)
			|| interface_contains(ns, addr);
}

static struct addrxlat_result programming_error(void)
{
	struct addrxlat_result result;
	result.verdict = ADDRXLAT_DROP;
	result.reason = "Programming error.";
	return result;
}

struct addrxlat_result addrxlat_siit64(struct xlator *instance,
		struct in6_addr *in, struct result_addrxlat64 *out,
		bool enable_denylists)
{
	struct addrxlat_result result;
	int error;

	if (is_illegal_source(in)) {
		result.verdict = ADDRXLAT_ACCEPT;
		result.reason = "IPv6 source address (::1) is illegal (according to RFC 7915).";
		return result;
	}

	error = eamt_xlat_6to4(instance->siit.eamt, in, out);
	if (!error)
		goto success;
	if (unlikely(error != -ESRCH))
		return programming_error();

	if (!instance->globals.pool6.set || rfc6052_6to4(&instance->globals.pool6.prefix, in, out)) {
		result.verdict = ADDRXLAT_TRY_SOMETHING_ELSE;
		result.reason = "Address lacks both pool6 prefix and EAM.";
		return result;
	}

	if (enable_denylists && denylist4_contains(instance->siit.denylist4,
			&out->addr)) {
		result.verdict = ADDRXLAT_ACCEPT;
		/* No, that's not a typo. */
		result.reason = "Address is denylist4ed.";
		return result;
	}

success:
	if (enable_denylists && must_not_translate(&out->addr, instance->ns)) {
		result.verdict = ADDRXLAT_ACCEPT;
		result.reason = "Address is subnet-scoped or belongs to a local interface.";
		return result;
	}

	result.verdict = ADDRXLAT_CONTINUE;
	result.reason = NULL;
	return result;
}

struct addrxlat_result addrxlat_siit46(struct xlator *instance,
		__be32 in, struct result_addrxlat46 *out,
		bool enable_eam, bool enable_denylists)
{
	struct in_addr tmp = { .s_addr = in };
	struct addrxlat_result result;
	int error;

	if (enable_denylists && must_not_translate(&tmp, instance->ns)) {
		result.verdict = ADDRXLAT_ACCEPT;
		result.reason = "Address is subnet-scoped or belongs to a local interface.";
		return result;
	}

	if (enable_eam) {
		error = eamt_xlat_4to6(instance->siit.eamt, &tmp, out);
		if (!error)
			goto success;
		if (error != -ESRCH)
			return programming_error();
	}

	if (denylist4_contains(instance->siit.denylist4, &tmp)) {
		result.verdict = ADDRXLAT_ACCEPT;
		result.reason = "Address lacks EAMT entry and is denylist4ed.";
		return result;
	}

	if (!instance->globals.pool6.set) {
		result.verdict = ADDRXLAT_TRY_SOMETHING_ELSE;
		result.reason = "Address lacks EAMT entry and there's no pool6 prefix.";
		return result;
	}

	error = rfc6052_4to6(&instance->globals.pool6.prefix, &tmp, out);
	if (error)
		return programming_error();

success:
	result.verdict = ADDRXLAT_CONTINUE;
	result.reason = NULL;
	return result;
}
