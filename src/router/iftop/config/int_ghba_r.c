/*
 * int_ghba_r.c:
 * Test program to see whether gethostbyaddr_r takes 8 arguments and returns
 * int.
 */

static const char rcsid[] = "$Id: int_ghba_r.c,v 1.1 2002/11/04 12:27:35 chris Exp $";

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
    int res, herr;
    size_t buflen = 1024;
    
    localhost.s_addr = htonl(INADDR_LOOPBACK);
    buf = malloc(buflen);
    while ((res = gethostbyaddr_r((char*)&localhost, sizeof localhost, AF_INET,
                                  &hostbuf, buf, buflen, &hp, &herr))
                    == ERANGE)
        buf = (char*)realloc(buf, buflen *= 2);

    /* We assume that the loopback address can always be resolved if
     * gethostbyaddr_r is actually working. */
    if (res || hp == NULL) {
        fprintf(stderr, "errno = %d, herr = %d, res = %d\n", errno, herr, res);
        return -1;
    } else
        return 0;
}
