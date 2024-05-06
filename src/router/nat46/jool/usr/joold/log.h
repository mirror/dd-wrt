#ifndef SRC_USR_JOOLD_LOG_H_
#define SRC_USR_JOOLD_LOG_H_

#include "usr/util/result.h"

int pr_result(struct jool_result *result);

/**
 * Prints @prefix, then a colon, then @error's standard error message, then a
 * newline.
 *
 * Replaces perror(). (Because perror print into standard error.)
 * perror() should not be used anywhere in joold!
 */
void pr_perror(char *prefix, int error);

#endif /* SRC_USR_JOOLD_LOG_H_ */
