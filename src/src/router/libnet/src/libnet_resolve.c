/*
 *  $Id: libnet_resolve.c,v 1.1 2004/04/27 01:29:51 dyang Exp $
 *
 *  libnet
 *  libnet_resolve.c - various name resolution type routines
 *
 *  Copyright (c) 1998, 1999 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include "../include/libnet.h"

u_char *
libnet_host_lookup(u_long in, u_short use_name)
{
    static u_char hostname[512], hostname2[512];
    static u_short which;
    u_char *p;

    struct hostent *host_ent = NULL;
    struct in_addr addr;

    /*
     *  Swap to the other buffer.  We swap static buffers to avoid having to
     *  pass in a char *.  This makes the code that calls this function more
     *  intuitive, but makes this function ugly.  This function is seriously
     *  non-reentrant.  For threaded applications (or for signal handler code)
     *  use host_lookup_r().
     */
    which++;
    
    if (use_name == LIBNET_RESOLVE)
    {
        addr.s_addr = in;
        host_ent = gethostbyaddr((char *)&addr, sizeof(struct in_addr), AF_INET);
    }
    if (!host_ent)
    {

        p = (u_char *)&in;
        sprintf(((which % 2) ? hostname : hostname2),  "%d.%d.%d.%d",
                (p[0] & 255), (p[1] & 255), (p[2] & 255), (p[3] & 255));
    }
    else if (use_name == LIBNET_RESOLVE)
    {
        strncpy(((which % 2) ? hostname : hostname2), host_ent->h_name, 
                                                        sizeof(hostname));
    }
    return (which % 2) ? (hostname) : (hostname2);
}


void
libnet_host_lookup_r(u_long in, u_short use_name, u_char *hostname)
{
    u_char *p;
    struct hostent *host_ent = NULL;
    struct in_addr addr;

    if (use_name == LIBNET_RESOLVE)
    {    
        addr.s_addr = in;
        host_ent = gethostbyaddr((char *)&addr, sizeof(struct in_addr), AF_INET);
    }
    if (!host_ent)
    {

        p = (u_char *)&in;
        sprintf(hostname, "%d.%d.%d.%d",
                (p[0] & 255), (p[1] & 255), (p[2] & 255), (p[3] & 255));
    }
    else
    {
        /* XXX - sizeof(hostname) == 4 bytes you moron.  FIX THAT. - r */
        strncpy(hostname, host_ent->h_name, sizeof(hostname));
    }
}


u_long
libnet_name_resolve(u_char *host_name, u_short use_name)
{
    struct in_addr addr;
    struct hostent *host_ent; 
    u_long l;
    u_int val;
    int i;
   
    if (use_name == LIBNET_RESOLVE)
    {
        if ((addr.s_addr = inet_addr(host_name)) == -1)
        {
            if (!(host_ent = gethostbyname(host_name)))
            {
                return (-1);
            }
            memcpy((char *)&addr.s_addr, host_ent->h_addr, host_ent->h_length);
        }
        return (addr.s_addr);
    }
    else
    {
        /*
         *  We only want dots 'n decimals.
         */
        if (!isdigit(host_name[0]))
        {
            return (-1L);
        }

        l = 0;
        for (i = 0; i < 4; i++)
        {
            l <<= 8;
            if (*host_name)
            {
                val = 0;
                while (*host_name && *host_name != '.')
                {   
                    val *= 10;
                    val += *host_name - '0';
                    if (val > 255)
                    {
                        return (-1L);
                    }
                    host_name++;
                }
                l |= val;
                if (*host_name)
                {
                    host_name++;
                }
            }
        }
        return (htonl(l));
    }
}


u_long
libnet_get_ipaddr(struct libnet_link_int *l, const u_char *device, char *ebuf)
{
    struct ifreq ifr;
    register struct sockaddr_in *sin;
    int fd;

    /*
     *  Create dummy socket to perform an ioctl upon.
     */
    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        sprintf(ebuf, "socket: %s", strerror(errno));
        return (0);
    }

    memset(&ifr, 0, sizeof(ifr));
    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));

    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl(fd, SIOCGIFADDR, (char*) &ifr) < 0)
    {
        close(fd);
        return(0);
    }
    close(fd);
    return (ntohl(sin->sin_addr.s_addr));
}


/*
 *  get_hwaddr routine moved to arch specifc files (sockpacket.c, bpf.c, etc)
 */

/* EOF */
