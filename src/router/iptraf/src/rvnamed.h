#ifndef IPTRAF_NG_RVNAMED_H
#define IPTRAF_NG_RVNAMED_H

#include <netinet/in.h>
#include <arpa/inet.h>

#define CHILDSOCKNAME "/tmp/rvndcldcomsk"
#define PARENTSOCKNAME "/tmp/rvndpntcomsk"
#define IPTSOCKNAME "/tmp/rvndiptcomsk"

#define SOCKET_PREFIX	"isock"

#define NOTRESOLVED 0
#define RESOLVING 1
#define RESOLVED 2

#define RVN_HELLO 0
#define RVN_REQUEST 1
#define RVN_REPLY 2
#define RVN_QUIT 3

#define MAX_RVNAMED_CHILDREN	200

struct rvn {
	int type;
	int ready;
	struct sockaddr_storage addr;
	char fqdn[45];
};

#endif	/* IPTRAF_NG_RVNAMED_H */
