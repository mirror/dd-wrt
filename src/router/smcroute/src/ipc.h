/* Daemon IPC API */
#ifndef SMCROUTE_IPC_H_
#define SMCROUTE_IPC_H_

#include "config.h"

int   ipc_init    (char *path);
void  ipc_exit    (void);

int     ipc_send   (int sd, const char *buf, size_t len);
ssize_t ipc_receive(int sd, char *buf, size_t len, int first_call);
int     ipc_parse  (const char *buf, size_t sz, void *msg_buf);

#endif /* SMCROUTE_IPC_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
