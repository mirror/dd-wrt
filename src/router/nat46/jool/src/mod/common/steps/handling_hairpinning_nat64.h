#ifndef SRC_MOD_COMMON_HANDLING_HARPINNING_H_
#define SRC_MOD_COMMON_HANDLING_HARPINNING_H_

/**
 * @file
 * Fifth and (officially) last step of the Nat64 translation algorithm: "Handling Hairpinning", as
 * defined in RFC6146 section 3.8.
 * Recognizes a packet that should return from the same interface and handles it accordingly.
 */

#include "mod/common/translation_state.h"

bool is_hairpin_nat64(struct xlation *state);
verdict handling_hairpinning_nat64(struct xlation *state);

#endif /* SRC_MOD_COMMON_HANDLING_HARPINNING_H_ */
