#ifndef SRC_MOD_COMMON_STEPS_HANDLING_HAIRPINNING_H_
#define SRC_MOD_COMMON_STEPS_HANDLING_HAIRPINNING_H_

#include "mod/common/translation_state.h"

bool is_hairpin_siit(struct xlation *state);
verdict handling_hairpinning_siit(struct xlation *old);

#endif /* SRC_MOD_COMMON_STEPS_HANDLING_HAIRPINNING_H_ */
