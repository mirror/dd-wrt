#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

static char test_buf[36] = {
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35
};

#define	DST_IP		"127.0.0.1"
#define	DST_PORT	35000
#define	SRC_IP1		"10.0.0.1"
#define	SRC_IP2		"10.0.0.2"
#define	SRC_PORT	DST_PORT
#define	NPACKETS	100000

int
main(int argc, char **argv)
{
    int fd1, fd2, i, n, f;
    struct addrinfo hints, *res;
    struct sockaddr_in ia1, ia2, dia;

    fd1 = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd1 == -1)
        err(1, "can't create socket");
    fd2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd2 == -1)
        err(1, "can't create socket");
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    n = getaddrinfo(SRC_IP1, NULL, &hints, &res);
    if (n != 0)
        errx(1, "getaddrinfo failed: %s", gai_strerror(n));
    memcpy((void *)&ia1, res->ai_addr, res->ai_addrlen);
    ia1.sin_port = htons(SRC_PORT);
    freeaddrinfo(res);
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    n = getaddrinfo(SRC_IP2, NULL, &hints, &res);
    if (n != 0)
        errx(1, "getaddrinfo failed: %s", gai_strerror(n));
    memcpy((void *)&ia2, res->ai_addr, res->ai_addrlen);
    ia2.sin_port = htons(SRC_PORT);
    freeaddrinfo(res);
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    n = getaddrinfo(DST_IP, NULL, &hints, &res);
    if (n != 0)
        errx(1, "getaddrinfo failed: %s", gai_strerror(n));
    memcpy((void *)&dia, res->ai_addr, res->ai_addrlen);
    dia.sin_port = htons(DST_PORT);
    freeaddrinfo(res);

    if (bind(fd1, (struct sockaddr *)&ia1, sizeof(ia1)) != 0)
        err(1, "can't bind a socket");
    if (bind(fd2, (struct sockaddr *)&ia2, sizeof(ia2)) != 0)
        err(1, "can't bind a socket");

    for (i = 0; i < NPACKETS; i++) {
        sendto(fd1, test_buf, sizeof(test_buf), 0,
          (struct sockaddr *)&dia, sizeof(dia));
        sendto(fd2, test_buf, sizeof(test_buf), 0,
          (struct sockaddr *)&dia, sizeof(dia));
	for (f = 0; f < 10000; f++);
    }

    exit(0);
}
