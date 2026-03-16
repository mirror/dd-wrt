/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2004, 2005, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2014, 2016, 2017, 2020
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

/*
 * getaddrinfo() contributed by Motoyuki Kasahara <m-kasahr@sra.co.jp>
 * getipnodebyname() contributed by Lennart Dahlström <lennart@appgate.com>
 */

#include "common.h"

static const char rcsid[] =
"$Id: Rgethostbyname.c,v 1.107.4.8.2.4.4.2.4.1 2024/11/21 10:22:42 michaels Exp $";

struct hostent *
Rgethostbyname2(name, af)
   const char *name;
   int af;
{
   const char *function = "Rgethostbyname2()";
   static struct hostent hostentmem;
   static char *aliases[] = { NULL };
   struct in_addr ipindex;
   struct hostent *hostent;

   clientinit();

   slog(LOG_DEBUG, "%s: %s", function, name);

   switch (sockscf.resolveprotocol) {
      case RESOLVEPROTOCOL_TCP:
      case RESOLVEPROTOCOL_UDP:
         if ((hostent = gethostbyname(name)) != NULL)
            return hostent;
         break;

      case RESOLVEPROTOCOL_FAKE:
         hostent = NULL;
         break;

      default:
         SERRX(sockscf.resolveprotocol);
   }

   if (hostent != NULL)
      return hostent;

   /*
    * continue as if resolveprotocol is set to fake and hope that works.
    */

   if (sockscf.resolveprotocol != RESOLVEPROTOCOL_FAKE)
      slog(LOG_DEBUG, "%s: gethostbyname(%s) failed: %s.  Will try to fake it",
           function, name, hstrerror(h_errno));

   hostent = &hostentmem;

   /*
    * anything that fails from here for fake should be due to
    * resource shortage.
    */
   h_errno = TRY_AGAIN;

   free(hostent->h_name);
   if ((hostent->h_name = strdup(name)) == NULL)
      return NULL;

   hostent->h_aliases  = aliases;
   hostent->h_addrtype = af;

   if (hostent->h_addr_list == NULL) {
      /* x 2 because NULL terminated and always only one valid entry (fake). */
      if ((hostent->h_addr_list = malloc(sizeof(*hostent->h_addr_list) * 2))
      == NULL)
         return NULL;
      hostent->h_addr_list[1] = NULL;
   }

   switch (af) {
      case AF_INET: {
         static char ipv4[sizeof(struct in_addr)];

         hostent->h_length       = sizeof(ipv4);
         hostent->h_addr_list[0] = ipv4;
         break;
      }

      case AF_INET6: {
         static char ipv6[sizeof(struct in6_addr)];

         hostent->h_length       = sizeof(ipv6);
         hostent->h_addr_list[0] = ipv6;
         break;
      }

      default:
         errno = ENOPROTOOPT;
         return NULL;
   }

   if ((ipindex.s_addr = socks_addfakeip(name)) == htonl(INADDR_NONE))
      return NULL;

   if (socks_inet_pton(af, inet_ntoa(ipindex), *hostent->h_addr_list, NULL)
   != 1)
      return NULL;

   slog(LOG_INFO, "%s: added fake ip %s for hostname %s",
        function, inet_ntoa(ipindex), name);

   return hostent;
}

struct hostent *
Rgethostbyname(name)
   const char *name;
{

   return Rgethostbyname2(name, AF_INET);
}

#if HAVE_GETADDRINFO
int
Rgetaddrinfo(nodename, servname, hints, res)
   const char *nodename;
   const char *servname;
   const struct addrinfo *hints;
   struct addrinfo **res;
{
   const char *function = "Rgetaddrinfo()";
   struct addrinfo fakehints;
   struct in_addr ipindex;
   char addrstr[MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)],
        addrbuf[sizeof(struct in6_addr)],
        vbuf_nodename[MAXHOSTNAMELEN * 4], vbuf_servname[MAXSERVICELEN * 4];
   int fakeip_cantry, gaierr, isipaddr;

   clientinit();

   if (nodename == NULL || *nodename == NUL)
      STRCPY_ASSERTSIZE(vbuf_nodename, "<null>");
   else
      str2vis(nodename, strlen(nodename), vbuf_nodename, sizeof(vbuf_nodename));

   if (servname == NULL || *servname == NUL)
      STRCPY_ASSERTSIZE(vbuf_servname, "<null>");
   else
      str2vis(servname, strlen(servname), vbuf_servname, sizeof(vbuf_servname));

   slog(LOG_DEBUG,
        "%s: resolveprotocol = %s, nodename = \"%s\", servname = \"%s\", "
        "hints = %p (ai_family: %d)",
        function,
        resolveprotocol2string(sockscf.resolveprotocol),
        vbuf_nodename,
        vbuf_servname,
        hints,
        hints == NULL ? 0 : hints->ai_family);

   if ((nodename == NULL || *nodename == NUL)
   || (hints != NULL && (hints->ai_flags & AI_NUMERICHOST))) {
      /*
       * either no hostname or a hostname that should be an ipaddress-string.
       * No need to fake anything then, and getaddrinfo(3) should work.
       */

      gaierr = getaddrinfo(nodename, servname, hints, res);

      slog(LOG_DEBUG, "%s: getaddrinfo(%s, %s) returned %d (%s)",
           function,
           vbuf_nodename,
           vbuf_servname,
           gaierr,
           gai_strerror(gaierr));

      return gaierr;
   }

   /*
    * Check if name passed us is actually an ipaddress on string form.
    * If so, we can not fake a different ipaddress, regardless of what
    * resolvprotocol is set to.  inet_pton(3) should give the answer
    * to whether that is the case or not.
    */

   SASSERTX(nodename != NULL);

   if (hints == NULL || hints->ai_protocol == PF_UNSPEC) {
      if (socks_inet_pton(AF_INET,  nodename, addrbuf, NULL) == 1
      ||  socks_inet_pton(AF_INET6, nodename, addrbuf, NULL) == 1)
         isipaddr = 1;
      else
         isipaddr = 0;
   }
   else {
      SASSERTX(hints != NULL);

      if (socks_inet_pton(hints->ai_family, nodename, addrbuf, NULL) == 1)
         isipaddr = 1;
      else
         isipaddr = 0;
   }

   if (isipaddr) {
      fakeip_cantry = 0;

      slog(LOG_DEBUG, "%s: nodename passed (%s) is an ipaddress",
           function, vbuf_nodename);
   }
   else
      fakeip_cantry = 1;


   bzero(&fakehints, sizeof(fakehints));
   if (hints == NULL) {
      fakehints.ai_flags     = AI_NUMERICHOST;
      fakehints.ai_socktype  = 0;
      fakehints.ai_protocol  = 0;
   }
   else {
      fakehints = *hints;
      fakehints.ai_flags  = hints->ai_flags | AI_NUMERICHOST;
   }

   fakehints.ai_family    = AF_INET; /* only ipv4-support in client. */
   fakehints.ai_addrlen   = 0;
   fakehints.ai_canonname = NULL;
   fakehints.ai_addr      = NULL;
   fakehints.ai_next      = NULL;

   switch (sockscf.resolveprotocol) {
      case RESOLVEPROTOCOL_FAKE:
         if (isipaddr) {
            SASSERTX(fakehints.ai_flags & AI_NUMERICHOST);

            /*
             * With AI_NUMERICHOST this should not result in any DNS lookup.
             */
            gaierr = getaddrinfo(nodename, servname, &fakehints, res);

            slog(LOG_DEBUG, "%s: getaddrinfo(%s, %s) returned %d (%s)",
                 function,
                 vbuf_nodename,
                 vbuf_servname,
                 gaierr,
                 gai_strerror(gaierr));

            return gaierr;
         }

         /*
          * else: should try to fake it resolving.
          */
         SASSERTX(fakeip_cantry);
         break;

      case RESOLVEPROTOCOL_TCP:
      case RESOLVEPROTOCOL_UDP: {
         gaierr = getaddrinfo(nodename, servname, hints, res);

         slog(LOG_DEBUG, "%s: getaddrinfo(%s, %s) returned %d (%s)",
              function,
              vbuf_nodename,
              vbuf_servname,
              gaierr,
              gai_strerror(gaierr));

         if (gaierr == 0)
            return gaierr;

         if (!fakeip_cantry)
            return gaierr; /* failed, but nothing we can do about that. */

         SASSERTX(fakeip_cantry);
         break;
      }

      default:
         SERRX(sockscf.resolveprotocol);
   }

   SASSERTX(fakeip_cantry);
   SASSERTX(nodename != NULL);

   /*
    * Set up things for a call to the proper getaddrinfo(3) function.
    *
    * What wo do here is to make sure AI_NUMERICHOST is set, and
    * then we call the real getaddrinfo() with our faked ip.
    * This should return us a addrinfo struct on the proper format,
    * and using our faked ip as address.  This should also allow the
    * system freeaddrinfo(3) work as normal, which it may not have done
    * if we were to malloc(3) the memory ourselves.
    *
    * Kudos to Motoyuki Kasahara for what is a pretty nifty idea.
    */

   switch (fakehints.ai_family) {
      /*
       * Ok.
       */
      case 0:
      case AF_INET:
         break;

      /*
       * Not ok.
       */
      case AF_INET6:
         default:
         swarnx("%s: %s not supported for resolveprotocol %s yet",
                function,
                safamily2string((sa_family_t)hints->ai_family),
                resolveprotocol2string(sockscf.resolveprotocol));

         return EAI_FAIL;
   }

   if ((ipindex.s_addr = socks_addfakeip(nodename)) == htonl(INADDR_NONE))
      return EAI_MEMORY;

   STRCPY_ASSERTLEN(addrstr, inet_ntoa(ipindex));

   slog(LOG_INFO, "%s: faking ip address %s for host \"%s\", service \"%s\"",
        function, addrstr, vbuf_nodename, vbuf_servname);

   gaierr = getaddrinfo(addrstr, servname, &fakehints, res);

   slog(gaierr == 0 ? LOG_DEBUG : LOG_WARNING,
        "%s: getaddrinfo(%s, %s) returned: %d (%s)",
        function,
        vbuf_nodename,
        vbuf_servname,
        gaierr,
        gai_strerror(gaierr));

   return gaierr;
}
#endif /* HAVE_GETADDRINFO */

#if HAVE_GETIPNODEBYNAME
/*
 * Solaris implements getaddrinfo() by calling getipnodebyname(),
 * so need this too.
 */

struct hostent *
Rgetipnodebyname2(name, af, flags, error_num)
   const char *name;
   int af;
   int flags;
   int *error_num;
{
   const char *function = "Rgetipnodebyname2()";
   struct in_addr ipindex;
   struct hostent *hostent;
   char **addrlist, vbuf_name[MAXHOSTNAMELEN * 4];
   int rc;

   /* needs to be done before getipnodebyname() calls. */
   clientinit();

   str2vis(name, strlen(name), vbuf_name, sizeof(vbuf_name));

   slog(LOG_DEBUG, "%s: %s: %s", function, safamily2string(af), vbuf_name);

   /*
    * On Solaris, where getaddrinfo(3) calls getipnodename(3), note that
    * "name" may already be a faked ip if resolveprotocol is set to fake.
    */
   if (socks_inet_pton(af, name, &ipindex, NULL) == 1) {
      hostent = getipnodebyname(name, af, flags, error_num);

      slog(LOG_DEBUG,
           "%s: since name \"%s\" is already on numeric form, assumed nothing "
           "special needs to be done.  getipnodebyname2() returned %p/%d",
           function, vbuf_name, hostent, *error_num);

      return hostent;
   }

   switch (sockscf.resolveprotocol) {
      case RESOLVEPROTOCOL_TCP:
      case RESOLVEPROTOCOL_UDP:
         slog(LOG_INFO,
              "%s: configured for using %s for resolving hostnames",
              function, protocol2string(sockscf.resolveprotocol));

         if ((hostent = getipnodebyname(name, af, flags, error_num)) != NULL)
            return hostent;

         break;

      case RESOLVEPROTOCOL_FAKE:
         slog(LOG_INFO,
              "%s: configured for faking resolve of hostnames", function);

         hostent = NULL;
         h_errno = NO_RECOVERY;
         break;

      default:
         SERRX(sockscf.resolveprotocol);
   }

   if (h_errno != NO_RECOVERY)
      return hostent;

   if ((hostent = malloc(sizeof(struct hostent))) == NULL) {
      swarnx("%s: malloc(3) failed", function);
      return NULL;
   }

   /*
    * anything that fails from here is due to resource shortage.
    */
   h_errno = TRY_AGAIN;

   if ((hostent->h_name = strdup(name)) == NULL) {
      free(hostent);
      return NULL;
   }

   hostent->h_aliases  = NULL;
   hostent->h_addrtype = af;

   /* * 2; NULL terminated. */
   if ((addrlist = malloc(sizeof(*addrlist) * 2)) == NULL) {
      swarnx("%s: malloc(3) failed", function);

      free(hostent->h_name);
      free(hostent);

      return NULL;
   }

   switch (af) {
      case AF_INET: {
         static char ipv4[sizeof(struct in_addr)];

         hostent->h_length = sizeof(ipv4);
         *addrlist         = ipv4;

         break;
      }

      case AF_INET6: {
         static char ipv6[sizeof(struct in6_addr)];

         hostent->h_length = sizeof(ipv6);
         *addrlist         = ipv6;

         break;
      }

      default:
         swarnx("%s: unsupported address family: %d", function, af);


         free(hostent->h_name);
         free(hostent);

         errno = ENOPROTOOPT;
         return NULL;
   }

   if ((ipindex.s_addr = socks_addfakeip(name)) == htonl(INADDR_NONE)) {
      free(hostent->h_name);
      free(hostent);
      free(addrlist);

      return NULL;
   }

   switch (af) {
      case AF_INET: {
         memcpy(*addrlist, &ipindex.s_addr, sizeof(struct in_addr));
         break;
      }

      case AF_INET6: {
         unsigned char ff[] = {0xff,0xff};

         memset(*addrlist, 0, 10);
         memcpy(*addrlist + 10, ff, 2);
         memcpy(*addrlist + 12, &ipindex.s_addr, sizeof(struct in_addr));
         break;
      }

      default:
         SERRX(af);
   }

   hostent->h_addr_list = addrlist++;
   *addrlist            = NULL;

   return hostent;
}

struct hostent *
Rgetipnodebyname(name, af, flags, error_num)
   const char *name;
   int af;
   int flags;
   int *error_num;
{
   struct hostent *hent;
   const char *function = "Rgetipnodebyname()";

   slog(LOG_DEBUG, "%s: %s, %d", function, name, af);

   if ((hent = Rgetipnodebyname2(name, af, flags, error_num)) == NULL)
       *error_num = h_errno;

   return hent;
}

void
Rfreehostent(ptr)
   struct hostent *ptr;
{
   struct in_addr addr;

   const char *function = "Rfreehostent()";

   slog(LOG_DEBUG, "%s", function);

   if (socks_getfakeip(ptr->h_name, &addr)) {
      free(ptr->h_name);
      free(*(ptr->h_addr_list));
      free(ptr->h_addr_list);
      free(ptr);
   }
   else
      freehostent(ptr);
}
#endif /* HAVE_GETIPNODEBYNAME */

#if HAVE_GETNAMEINFO

int
Rgetnameinfo(sa, salen, host, hostlen, serv, servlen, flags)
   const struct sockaddr *sa;
   socklen_t salen;
   char *host;
   socklen_t hostlen;
   char *serv;
   socklen_t servlen;
   int flags;
{
   const char *function = "getnameinfo()";

   if (sockscf.resolveprotocol == RESOLVEPROTOCOL_FAKE) {
      char vbuf_name[MAXHOSTNAMELEN * 4];

      if (host == NULL || *host == NUL)
         STRCPY_ASSERTSIZE(vbuf_name, "<null>");
      else
         str2vis(host, strlen(host), vbuf_name, sizeof(vbuf_name));

      slog(LOG_WARNING,
           "%s: getnameinfo(3) (%s) is not yet supported", function, vbuf_name);
   }

   return getnameinfo(sa, salen, host, hostlen, serv, servlen, flags);
}

#endif /* HAVE_GETNAMEINFO  */
