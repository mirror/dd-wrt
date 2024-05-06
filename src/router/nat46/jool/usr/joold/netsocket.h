#ifndef SRC_USR_JOOLD_NETSOCKET_H_
#define SRC_USR_JOOLD_NETSOCKET_H_

/**
 * This is the socket we use to talk to other joold instances in the network.
 */

#include <stddef.h>

int netsocket_setup(int argc, char **argv);
void netsocket_teardown(void);

void *netsocket_listen(void *arg);
void netsocket_send(void *buffer, size_t size);

#endif /* SRC_USR_JOOLD_NETSOCKET_H_ */
