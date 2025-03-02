/* SMCRoute IPC API
 *
 * Multicast routes:
 *
 * add eth0 [1.1.1.1] 239.1.1.1 eth1 eth2
 *
 *  +----+-----+---+--------------------------------------------+
 *  | 42 | 'a' | 5 | "eth0\01.1.1.1\0239.1.1.1\0eth1\0eth2\0\0" |
 *  +----+-----+---+--------------------------------------------+
 *  ^              ^        |-----|
 *  |              |               `---> Second argument is optional => (*,G)
 *  +-----cmd------+
 *
 * del [1.1.1.1] 239.1.1.1
 *
 *  +----+-----+---+--------------------------+
 *  | 27 | 'r' | 2 | "1.1.1.1\0239.1.1.1\0\0" |
 *  +----+-----+---+--------------------------+
 *  ^              ^
 *  |              |
 *  +-----cmd------+
 *
 * Multicast groups:
 *
 * join/leave eth0 [1.1.1.1] 239.1.1.1
 *
 *  +----+-----+---+--------------------------------------------+
 *  | 32 | 'j' | 3 | "eth0\01.1.1.1\0239.1.1.1\0\0"             |
 *  +----+-----+---+--------------------------------------------+
 *                          |-----|
 *                                 `-----> For SSM group join/leave
 */
#ifndef SMCROUTE_MSG_H_
#define SMCROUTE_MSG_H_

#include <paths.h>
#include <stdint.h>
#include <stdlib.h>

#define MX_CMDPKT_SZ 1024	/* command size including appended strings */

struct ipc_msg {
	size_t   len;		/* total size of packet including cmd header */
	uint16_t cmd;		/* 'a'=Add,'r'=Remove,'j'=Join,'l'=Leave,'k'=Kill */
	uint16_t count;		/* command argument count */
	char    *argv[0]; 	/* 'count' * '\0' terminated strings + '\0' */
};

int msg_do(int sd, struct ipc_msg *msg);

#endif /* SMCROUTE_MSG_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
