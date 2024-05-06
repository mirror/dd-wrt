#ifndef SRC_MOD_COMMON_ADDRESS_XLAT_H_
#define SRC_MOD_COMMON_ADDRESS_XLAT_H_

#include "mod/common/xlator.h"

/**
 * The reason why I need to create a new enum (as opposed to adding
 * TRY_SOMETHING_ELSE to verdict) is because VERDICT_CONTINUE is the only of its
 * kind that does not interrupt translation, which allows me to simplify most
 * verdict handling in the rest of the project:
 *
 *	verdict = handle_something(...);
 * 	if (verdict != VERDICT_CONTINUE)
 * 		return verdict; // ie. "interrupt"
 *
 * This would simply not be possible if there were other "possibly continue"
 * verdicts.
 */
typedef enum addrxlat_verdict {
	/** "Ok, address translated. Do something else now." */
	ADDRXLAT_CONTINUE,
	/** "Translation failed but caller might use a fallback method." */
	ADDRXLAT_TRY_SOMETHING_ELSE,
	/** "Translation prohibited. Return VERDICT_ACCEPT and forget it." */
	ADDRXLAT_ACCEPT,
	/** "Translation prohibited. Return VERDICT_DROP and forget it." */
	ADDRXLAT_DROP,
} addrxlat_verdict;

struct addrxlat_result {
	addrxlat_verdict verdict;
	char const *reason;
};

struct addrxlat_result addrxlat_siit64(struct xlator *instance,
		struct in6_addr *in, struct result_addrxlat64 *out,
		bool enable_denylists);
struct addrxlat_result addrxlat_siit46(struct xlator *instance,
		__be32 in, struct result_addrxlat46 *out,
		bool enable_eam, bool enable_denylists);

#endif /* SRC_MOD_COMMON_ADDRESS_XLAT_H_ */
