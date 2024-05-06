#ifndef SRC_MOD_COMMON_TRANSLATION_STATE_H_
#define SRC_MOD_COMMON_TRANSLATION_STATE_H_

#include "mod/common/icmp_wrapper.h"
#include "mod/common/packet.h"
#include "mod/common/xlator.h"
#include "mod/common/db/bib/entry.h"

/*
 * Fields that need to be translated prematurely because the routing functions
 * need them as input.
 *
 * I named it "flowix" because it's more or less the same as a flowi (routing
 * arguments). But it might be a misnomer because I don't exactly know what
 * "flowi" stands for. I'm assuming it's "IP flow." The added "x" stands for
 * "xlat." Hence "IP translation flow."
 */
union flowix {
	struct {
		struct flowi4 flowi;
		struct in_addr inner_src;
		struct in_addr inner_dst;
	} v4;
	struct {
		struct flowi6 flowi;
		struct in6_addr inner_src;
		struct in6_addr inner_dst;
	} v6;
};

struct xlation_result {
	enum icmp_errcode icmp;
	__u32 info;
};

/**
 * State of the current translation.
 */
struct xlation {
	/**
	 * The instance of Jool that's in charge of carrying out this
	 * translation.
	 */
	struct xlator jool;

	/** The original packet. */
	struct packet in;
	/** The translated version of @in. */
	struct packet out;

	/**
	 * Routing arguments and result.
	 * Needed as part of this structure because the packet sometimes needs
	 * to be routed prematurely.
	 */
	bool flowx_set;
	union flowix flowx;
	struct dst_entry *dst;

	/**
	 * Convenient accesor to the BIB and session entries that correspond
	 * to the packet being translated, so you don't have to find them again.
	 */
	struct bib_session entries;

	/**
	 * Intrinsic hairpin?
	 * Intrinsic EAM hairpinning only. RFC6052 hairpin and Simple EAM
	 * hairpin don't need any flags.
	 */
	bool is_hairpin;

	struct xlation_result result;
};

int xlation_setup(void);
void xlation_teardown(void);

struct xlation *xlation_create(struct xlator *jool);
void xlation_init(struct xlation *state, struct xlator *jool);
/* xlation_cleanup() not needed. */
void xlation_destroy(struct xlation *state);

verdict untranslatable(struct xlation *state, enum jool_stat_id stat);
verdict untranslatable_icmp(struct xlation *state, enum jool_stat_id stat,
		enum icmp_errcode icmp, __u32 info);
verdict drop(struct xlation *state, enum jool_stat_id stat);
verdict drop_icmp(struct xlation *state, enum jool_stat_id stat,
		enum icmp_errcode icmp, __u32 info);
verdict stolen(struct xlation *state, enum jool_stat_id stat);

#define xlation_is_siit(state) xlator_is_siit(&(state)->jool)
#define xlation_is_nat64(state) xlator_is_nat64(&(state)->jool)

#endif /* SRC_MOD_COMMON_TRANSLATION_STATE_H_ */
