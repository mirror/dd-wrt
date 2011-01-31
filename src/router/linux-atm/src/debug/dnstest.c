#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>

#include <atm.h>


int main(int argc, char **argv) {
    int i;
    unsigned char *pnsap;
    struct sockaddr_atmsvc addr;
    unsigned char buffer[1024];

    if (argc != 2) {
        printf("Usage: %s <name> \n", argv[0]);
        exit(0);
    }
    if (text2atm(argv[1], (struct sockaddr *) &addr, sizeof(addr), T2A_NAME) < 0) {
        perror("text2atm()");
        exit(1);
    }
    if (atm2text(buffer, 1024, (struct sockaddr *) &addr, A2T_NAME) < 0) {
        perror("atm2text()");
	exit(2);
    }
    printf("Looking up %s \n", argv[1]);
    printf("Direct: ");
    pnsap = (unsigned char *) &(addr.sas_addr.prv);
    for (i = 0; i < 20; i++) {
        printf("%02X", pnsap[i]);
    }
    printf("\n");
    printf("Reverse: %s \n", buffer);
    exit(0);
}

