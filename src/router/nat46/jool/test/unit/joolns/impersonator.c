#include "mod/common/nl/global.h"
#include "mod/common/steps/handling_hairpinning_siit.h"
#include "framework/unit_test.h"

int global_update(struct jool_globals *cfg, xlator_type xt, bool force,
		struct global_value *request, size_t request_size)
{
	return -EINVAL;
}

verdict translating_the_packet(struct xlation *state)
{
	return VERDICT_DROP;
}

bool is_hairpin_siit(struct xlation *state)
{
	return false;
}

verdict handling_hairpinning_siit(struct xlation *old)
{
	broken_unit_call(__func__);
	return VERDICT_DROP;
}
