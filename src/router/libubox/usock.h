#ifndef USOCK_H_
#define USOCK_H_

#define USOCK_TCP 0
#define USOCK_UDP 1

#define USOCK_SERVER		0x0100
#define USOCK_NOCLOEXEC	0x0200
#define USOCK_NONBLOCK		0x0400
#define USOCK_NUMERIC		0x0800
#define USOCK_IPV6ONLY		0x2000
#define USOCK_IPV4ONLY		0x4000
#define USOCK_UNIX			0x8000

int usock(int type, const char *host, const char *service);

#endif /* USOCK_H_ */
