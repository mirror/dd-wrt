#ifndef SRC_MOD_COMMON_ERROR_POOL_H_
#define SRC_MOD_COMMON_ERROR_POOL_H_

#include <linux/types.h>

/* TODO (fine) This is nonsense; find a better way to send messages to userspace. */

void error_pool_setup(void);
void error_pool_teardown(void);

void error_pool_activate(void);
void error_pool_add_message(char *msg);
int error_pool_has_errors(void);
int error_pool_get_message(char **out_message, size_t *msg_len);
void error_pool_deactivate(void);

#endif /* SRC_MOD_COMMON_ERROR_POOL_H_ */
