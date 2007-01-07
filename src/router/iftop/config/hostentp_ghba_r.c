/*
 * hostentp_ghba_r.c:
 * Test program to see whether gethostbyaddr_r takes 7 arguments and returns
 * struct hostent*. 
 */

static const char rcsid[] = "$Id: hostentp_ghba_r.c,v 1.1 2002/11/04 12:27:35 chris Exp $";

#include <sys/types.h>

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

int main(void) {
    struct in_addr localhost;
    struct hostent hostbuf, *hp;
    char *buf;
    int herr;
    size_t buflen = 1024;
    
    localhost.s_addr = htonl(INADDR_LOOPBACK);
    buf = malloc(buflen);
    while ((hp = gethostbyaddr_r((char*)&localhost, sizeof(struct in_addr),
                                 AF_INET, &hostbuf, buf, buflen, &herr))
                    == NULL
           && errno == ERANGE)
        buf = (char*)realloc(buf, buflen *= 2);
        
    /* We assume that the loopback address can always be resolved if
     * gethostbyaddr_r is actually working. */
    if (hp == NULL) {
        fprintf(stderr, "errno = %d, herr = %d\n", errno, herr);
        return -1;
    } else
        return 0;
}
