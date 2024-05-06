#ifndef SRC_MOD_NAT64_FILTERING_AND_UPDATING_H_
#define SRC_MOD_NAT64_FILTERING_AND_UPDATING_H_

/**
 * @file
 * Second step of the stateful NAT64 translation algorithm: "Filtering and Updating Binding and
 * Session Information", as defined in RFC6146 section 3.5.
 */

#include "mod/common/translation_state.h"
#include "mod/common/db/bib/entry.h"

verdict filtering_and_updating(struct xlation *state);
enum session_fate tcp_est_expire_cb(struct session_entry *session, void *arg);

#endif /* SRC_MOD_NAT64_FILTERING_AND_UPDATING_H_ */
