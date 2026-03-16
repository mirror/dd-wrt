/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2008, 2009, 2010, 2011, 2012,
 *               2013, 2016, 2017, 2019, 2020
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

#define DISABLE_GETHOSTBYNAME_CACHE (1)
#include "common.h"

static const char rcsid[] =
"$Id: hostcache.c,v 1.172.4.9.2.4.4.3 2020/11/11 16:11:54 karls Exp $";

#if 0
#warning "XXX change back to LOG_DEBUG"
#define  LOG_DEBUG LOG_INFO
#endif

#if !SOCKS_CLIENT

#define MIN_HASH_SIZE (10240)
/*
 * Assume this is so small a cache that it's better to just scan it
 * sequentially rather than expire DNS-entries that we could otherwise
 * have continued to keep in the cache.
 */

static size_t
hosthash(const char *name, const size_t size);
/*
 * Calculates a hash value for "name" and returns it's value.
 * Size of hash table is given by "size".
*/

static hostentry_t *
hostentcopy(hostentry_t *to, const struct hostent *from);
/*
 * Copies all the values in "from" into "to", or as much as there is room
 * for.
 *
 * The only reason this function may fail is if "from" is too big, i.e.
 * has names that are too long (according to the dns spec) or similar.
 *
 * Note that the caller must set to->h_name or to->ipv4 when creating the
 * original entry based on the result of a gethostby*() call.  This is because
 * the result returned by the gethostby*() call may not be the address
 * used when gethostby*() was called, but the latter is what we want to use
 * in our cache.
 *
 * Returns "to" on success, NULL on failure.
 */



static size_t
addrhash(const struct sockaddr_storage *addr, const size_t size);
/*
 * Calculates a hash value for the IP-address "addr" and returns it's value.
 * Size of hash table is given by "size".
*/

static int
addrisinlist(const struct sockaddr *addr, struct addrinfo *ailist,
             struct addrinfo *ailist_last);
/*
 * "ailist_last" is the last member in the "ailist" that is valid.
 *
 * Returns true if the address "addr" is in the list "ailist".
 * Returns false otherwise.
 */

dnsinfo_t *
getoldest( const size_t starti);
/*
 * Returns the oldest entry at index "starti" or later.
 */

static int
name_matches_cache(const dnsinfo_t *cacheentry,
                   const char *name, const char *service,
                   const struct addrinfo *hints);
static int
addr_matches_cache(const dnsinfo_t *cacheentry, const struct sockaddr *addr,
                   const int flags);

/*
 * If the passed arguments match the cache entry "cacheentry",
 * the functions returns true.
 * Otherwise they returns false.
 */



#if STANDALONE_UNIT_TEST
size_t cbyname_hit, cbyname_miss;
size_t cbyaddr_hit, cbyaddr_miss;
#endif /* !STANDALONE_UNIT_TEST */

/*
 * hostname/ipaddress cache.  Shared among all processes.
 */
UNIT_TEST_STATIC_SCOPE dnsinfo_t *hostcache;


static int
gai2h_errno(const int gai_rc);
/*
 * Returns the h_errno version of the getaddrinfo/getnameinfo error "gai_rc".
 */

#undef cgetaddrinfo

#endif /* !SOCKS_CLIENT */

UNIT_TEST_STATIC_SCOPE int
addrinfocopy(dnsinfo_t *to, const struct addrinfo *from,
             const struct addrinfo *hints);
/*
 * Copies all the values in "from" into "to", or as much as there is room
 * for.  "hints" is taken into consideration concerning what to copy.
 *
 * The only reason this function may fail is if "from" is too big, i.e.
 * has names that are too long (according to the dns spec) or similar,
 * *OR* if "hints" precludes some entries from being copied.
 *
 * Returns an errocode of the same type as getaddrinfo(3).
 */


#if SOCKSLIBRARY_DYNAMIC

#define gethostbyaddr(addr, len, type)   sys_gethostbyaddr(addr, len, type)

#if SOCKS_CLIENT

#undef  gethostbyname
#define gethostbyname(name)              sys_gethostbyname(name)

#endif /* SOCKS_CLIENT */

#endif /* SOCKSLIBRARY_DYNAMIC */

#define GAI_RC_IS_SYSTEMPROBLEM(gai_rc)    \
      ((gai_rc) == EAI_MEMORY              \
   ||  (gai_rc) == EAI_SYSTEM)

#define GAI_RC_IS_EAGAIN(gai_rc)           \
   ((gai_rc) == EAI_AGAIN)

#if HAVE_ERR_EAI_OVERFLOW
#define GAI_RC_IS_INTERNALERROR(gai_rc)    \
   (  ((gai_rc) == EAI_BADFLAGS)           \
   || ((gai_rc) == EAI_FAMILY)             \
   || ((gai_rc) == EAI_SOCKTYPE)           \
   || ((gai_rc) == EAI_OVERFLOW))
#else
#define GAI_RC_IS_INTERNALERROR(gai_rc)    \
   (  ((gai_rc) == EAI_BADFLAGS)           \
   || ((gai_rc) == EAI_FAMILY)             \
   || ((gai_rc) == EAI_SOCKTYPE))
#endif /* HAVE_ERR_EAI_OVERFLOW */

int
cgetaddrinfo(name, service, hints, res, resmem)
   const char *name;
   const char *service;
   const struct addrinfo *hints;
   struct addrinfo **res;
   dnsinfo_t *resmem;
{
   const char *function = "cgetaddrinfo()";
   char namebuf[MAXHOSTNAMELEN * 4], servicebuf[MAXSERVICELEN * 4];
   int gai_rc;

#if SOCKS_CLIENT
   /*
    * No cache for client.  Just a slightly different interface, mainly
    * to limit differences in code called by both client and server, as
    * the server caches the result of getaddrinfo(3).
    */

   SASSERTX(res    != NULL);
   SASSERTX(resmem != NULL);

   if ((gai_rc = getaddrinfo(name, service, hints, res)) != 0)
      return gai_rc;

   SASSERTX(*res != NULL);

   /*
    * have resmem.  Copy into that and don't use the one returned from
    * getaddrinfo(3).
    */
   gai_rc = addrinfocopy(resmem, *res, hints);

   freeaddrinfo(*res);

   if (gai_rc != 0) {
      if (GAI_RC_IS_SYSTEMPROBLEM(gai_rc))
         swarnx("%s: addrinfocopy() failed for hostname \"%s\", service \"%s\"",
                function,
                str2vis(name, strlen(name), namebuf, sizeof(namebuf)),
                service == NULL ?
                "<NULL>" : str2vis(service,
                                   strlen(service),
                                   servicebuf,
                                   sizeof(servicebuf)));


      return gai_rc;
   }

   *res = &resmem->data.getaddr.addrinfo;

   return gai_rc;

#else /* !SOCKS_CLIENT */

   static size_t i;
   static int count;
   const time_t timenow = time_monotonic(NULL);
#if !STANDALONE_UNIT_TEST
   static size_t cbyname_hit, cbyname_miss;
#endif /* !STANDALONE_UNIT_TEST */
   dnsinfo_t *freehost;
   size_t hashi;
   int have_oldres = 0;

   SASSERTX(res    != NULL);
   SASSERTX(resmem != NULL);

   if (count++ % SOCKD_CACHESTAT == 0)
      slog(LOG_DEBUG, "%s, hit: %lu, miss: %lu",
           function, (unsigned long)cbyname_hit, (unsigned long)cbyname_miss);

   if (strlen(name) >= sizeof(freehost->id.name)) {
      swarnx("%s: hostname \"%s\" is too long.  Max length is %lu",
              function,
              str2vis(name, strlen(name), namebuf, sizeof(namebuf)),
              (unsigned long)sizeof(freehost->id.name) - 1);

      return EAI_MEMORY;
   }

   if (service != NULL && strlen(service) >= sizeof(freehost->service)) {
      swarn("max length of servicename was at compiletime set to %lu, but "
            "now being called with servicename of length %lu.  Recompile "
            "required to support a servicename this long.  "
            "Servicename: \"%s\"",
            (unsigned long)sizeof(freehost->service) - 1,
            (unsigned long)strlen(service) + 1,
            str2vis(service, strlen(service), servicebuf, sizeof(servicebuf)));

      return EAI_MEMORY;
   }

   socks_lock(sockscf.hostfd, 0, 0, 0, 1);

   /*
    * First check if the desired name is the same as last time.
    */
   if (i < SOCKD_HOSTCACHE
   &&  name_matches_cache(&hostcache[i], name, service, hints)) {
      if (socks_difftime(timenow, hostcache[i].written) < SOCKD_CACHETIMEOUT) {
         ++cbyname_hit;

         if (hostcache[i].gai_rc == 0) {
            gai_rc = addrinfocopy(resmem,
                                  &hostcache[i].data.getaddr.addrinfo,
                                  hints);
            SASSERTX(gai_rc == 0);

            *res = &resmem->data.getaddr.addrinfo;
         }

         socks_unlock(sockscf.hostfd, 0, 0);
         return hostcache[i].gai_rc;
      }

      /*
       * Else; in cache, but expired already.
       */
      freehost = &hostcache[i];
      hashi    = hosthash(name, SOCKD_HOSTCACHE);

      if (freehost->gai_rc == 0) {
         gai_rc = addrinfocopy(resmem,
                               &freehost->data.getaddr.addrinfo,
                               hints);

         SASSERTX(gai_rc == 0);
         have_oldres = 1;
      }
   }
   else {
      /*
       * Go through the entire in the cache, looking for a match.
       */

      hashi = hosthash(name, SOCKD_HOSTCACHE);
      for (i = hashi, freehost = NULL; i < SOCKD_HOSTCACHE; ++i) {
         if (!hostcache[i].allocated) {
            if (freehost == NULL)
               freehost = &hostcache[i];

            continue;
         }

         if (name_matches_cache(&hostcache[i], name, service, hints)) {
            if (socks_difftime(timenow, hostcache[i].written)
            >= SOCKD_CACHETIMEOUT) {
               freehost = &hostcache[i];

               if (freehost->gai_rc == 0) {
                  gai_rc = addrinfocopy(resmem,
                                        &freehost->data.getaddr.addrinfo,
                                        hints);

                  SASSERTX(gai_rc == 0);
                  have_oldres = 1;
               }

               break;
            }

            ++cbyname_hit;

            if (hostcache[i].gai_rc != 0) {
               socks_unlock(sockscf.hostfd, 0, 0);
               return hostcache[i].gai_rc;
            }

            gai_rc = addrinfocopy(resmem,
                                  &hostcache[i].data.getaddr.addrinfo,
                                  hints);

            SASSERTX(gai_rc == 0);

            socks_unlock(sockscf.hostfd, 0, 0);

            *res = &resmem->data.getaddr.addrinfo;
            return 0;
         }
      }
   }

   /*
    * Nope, this name is not in the cache.  Have to resolve.
    */

   socks_unlock(sockscf.hostfd, 0, 0);

   ++cbyname_miss;

   gai_rc = getaddrinfo(name, service, hints, res);

   slog(LOG_DEBUG,
        "%s: getaddrinfo(%s, %s, { %s }) returned %d and %p: %s",
        function,
        str2vis(name, strlen(name), namebuf, sizeof(namebuf)),
        service == NULL ? "<NULL>" : str2vis(service,
                                             strlen(service),
                                             servicebuf,
                                             sizeof(servicebuf)),
        hints == NULL ? "<NULL>" : aihints2string(hints, NULL, 0),
        gai_rc,
        gai_rc == 0 ? *res       : NULL,
        gai_rc == 0 ? "no error" : gai_strerror(gai_rc));

   if (gai_rc == EAI_SYSTEM && errno == EMFILE) {
      if (sockscf.state.reservedfdv[0] != -1) {
         close(sockscf.state.reservedfdv[0]);

         gai_rc = getaddrinfo(name, service, hints, res);

         sockscf.state.reservedfdv[0] = makedummyfd(0, 0);

         if (gai_rc != 0)
            slog(LOG_DEBUG, "%s: getaddrinfo(%s, %s) failed again: %s",
                 function,
                 str2vis(name, strlen(name), namebuf, sizeof(namebuf)),
                 service == NULL ?
                     "<NULL>" : str2vis(service,
                                        strlen(service),
                                        servicebuf,
                                        sizeof(servicebuf)),
                 gai_strerror(gai_rc));
      }
   }

#if HAVE_LINUX_BUGS
   /*
    * glibc calls connect(2) to something that fails from __GI_getaddrinfo(),
    * and then does not clear the errno, so even though the above getaddrinfo()
    * call succeeded, errno is now set, at least on the glibc used by
    * 3.7.3-101.fc17.x86_64.
    */
   if (gai_rc == 0)
      errno = 0;
#endif /* HAVE_LINUX_BUGS */

   if (gai_rc != 0)  {
      /*
       * check if the problem is ours, or the DNS's.
       */

      if (GAI_RC_IS_SYSTEMPROBLEM(gai_rc)) {
         swarn("%s: getaddrinfo(%s, %s) failed again: %s",
               function,
               str2vis(name, strlen(name), namebuf, sizeof(namebuf)),
               service == NULL ?
                   "<NULL>" : str2vis(service,
                                      strlen(service),
                                      servicebuf,
                                      sizeof(servicebuf)),
               gai_strerror(gai_rc));

      }

      if (have_oldres) {
         slog(LOG_DEBUG,
              "%s: resolve of %s failed.  Returning stale cache entry",
              function, str2vis(name, strlen(name), namebuf, sizeof(namebuf))),

         *res = &resmem->data.getaddr.addrinfo;
         return 0;
      }

      if (GAI_RC_IS_SYSTEMPROBLEM(gai_rc))
         return gai_rc;
      else if (GAI_RC_IS_EAGAIN(gai_rc))
         return gai_rc;
      else if (GAI_RC_IS_INTERNALERROR(gai_rc))
         SERR(gai_rc);

      /* else; assume problem is not ours. */
   }

   /*
    * Either things resolved ok, or the reason for failure is not our own
    * (i.e., not lack of memory, free fds, etc.).
    */

   if (gai_rc == 0)
      SASSERTX(*res != NULL);
   else {
      /*
       * else; assume host does not exist at the moment and cache that result.
       */

      *res = NULL;
      res  = NULL;
   }

   socks_lock(sockscf.hostfd, 0, 0, 1, 1);

   if (freehost == NULL)
      freehost = getoldest(hashi);
   /*
    * else: contents pointed to can have changed, in which case we may now
    * be overwriting one of the most recent entries, but never mind that;
    * that unlikely case is not important enough to warrant locking and
    * scanning for the oldest entry now.
    */

   SASSERTX(freehost != NULL);

   if (gai_rc == 0) {
      gai_rc = addrinfocopy(freehost, *res, hints);

      /*
       * have resmem.  Don't use the mem returned from getaddrinfo(3).
       */
      freeaddrinfo(*res);

      if (gai_rc != 0) {
         if (GAI_RC_IS_SYSTEMPROBLEM(gai_rc))
            swarnx("%s: addrinfocopy() failed for \"%s\"/service \"%s\": %s",
                   function,
                   str2vis(name, strlen(name), namebuf, sizeof(namebuf)),
                   service == NULL ?
                    "<NULL>" : str2vis(service,
                                       strlen(service),
                                       servicebuf,
                                       sizeof(servicebuf)),
                   gai_strerror(gai_rc));

         socks_unlock(sockscf.hostfd, 0, 0);
         return gai_rc;
      }
   }

   if (hints == NULL)
      freehost->data.getaddr.hints = NULL;
   else {
      freehost->data.getaddr.hints  = &freehost->data.getaddr.hints_mem;
      *freehost->data.getaddr.hints = *hints;
   }

   freehost->key = id_name;
   STRCPY_ASSERTLEN(freehost->id.name, name);

   if (service == NULL)
      *freehost->service = NUL;
   else
      STRCPY_ASSERTLEN(freehost->service, service);

   freehost->written   = timenow;
   freehost->gai_rc    = gai_rc;
   freehost->allocated = 1;

   SASSERTX(freehost->key == id_name);

   if (gai_rc == 0) {
      gai_rc = addrinfocopy(resmem, &freehost->data.getaddr.addrinfo, hints);
      SASSERTX(gai_rc == 0);
   }

   socks_unlock(sockscf.hostfd, 0, 0);

   resmem->gai_rc = gai_rc;
   if (resmem->gai_rc == 0)
      *res = &resmem->data.getaddr.addrinfo;

   return resmem->gai_rc;
#endif /* !SOCKS_CLIENT */
}

#if !SOCKS_CLIENT
int
cgetnameinfo(addr, addrlen, _host, _hostlen, _service, _servicelen, flags)
   const struct sockaddr *addr;
   const socklen_t addrlen;
   char *_host;
   const socklen_t _hostlen;
   char *_service;
   const socklen_t _servicelen;
   const int flags;
{
   const char *function = "cgetnameinfo()";
   const time_t timenow = time_monotonic(NULL);
#if !STANDALONE_UNIT_TEST
   static size_t cbyaddr_hit, cbyaddr_miss;
#endif /* !STANDALONE_UNIT_TEST */
   static size_t i;
   static int count;
   dnsinfo_t *freehost;
   size_t hashi;
   char host[sizeof(freehost->data.getname.name)],
        service[sizeof(freehost->service)];
   int gai_rc, have_oldres = 0;

#define DOCOPY(_host, _hostlen, _service, _servicelen, cacheentry)             \
do {                                                                           \
   if ((_hostlen) > 0) {                                                       \
      strncpy((_host), (cacheentry)->data.getname.name, (_hostlen) - 1);       \
      (_host)[(_hostlen) - 1] = NUL;                                           \
   }                                                                           \
                                                                               \
   if ((_servicelen) > 0) {                                                    \
      strncpy((_service), (cacheentry)->service, (_servicelen) - 1);           \
      (_service)[(_servicelen) - 1] = NUL;                                     \
   }                                                                           \
} while (/* CONSTCOND */ 0)

   if (count++ % SOCKD_CACHESTAT == 0) {
      slog(LOG_DEBUG, "%s: hit: %lu, miss: %lu",
           function, (unsigned long)cbyaddr_hit, (unsigned long)cbyaddr_miss);
   }

   socks_lock(sockscf.hostfd, 0, 0, 0, 1);

   /*
    * First check if the desired name is the same as last time.
    */
   if (i < SOCKD_HOSTCACHE
   &&  addr_matches_cache(&hostcache[i], addr, flags)) {
      if (socks_difftime(timenow, hostcache[i].written) < SOCKD_CACHETIMEOUT) {
         ++cbyaddr_hit;

         if (hostcache[i].gai_rc == 0)
            DOCOPY(_host, _hostlen, _service, _servicelen, &hostcache[i]);

         socks_unlock(sockscf.hostfd, 0, 0);
         return hostcache[i].gai_rc;
      }

      /*
       * Else; have an entry, but it's expired already.
       */

      freehost = &hostcache[i];
      hashi    = addrhash(TOCSS(addr), SOCKD_HOSTCACHE);

      if (freehost->gai_rc == 0) {
         DOCOPY(_host, _hostlen, _service, _servicelen, freehost);
         have_oldres = 1;
      }

   }
   else {
      /*
       * Go through the cache and see if there is match there.
       */
      hashi = addrhash(TOCSS(addr), SOCKD_HOSTCACHE);
      for (i = hashi, freehost = NULL; i < SOCKD_HOSTCACHE; ++i) {
         if (!hostcache[i].allocated) {
            if (freehost == NULL)
               freehost = &hostcache[i];

            continue;
         }

         if (addr_matches_cache(&hostcache[i], addr, flags)) {
            if (socks_difftime(timenow, hostcache[i].written)
            >= SOCKD_CACHETIMEOUT) {
               freehost = &hostcache[i];

               if (freehost->gai_rc == 0) {
                  DOCOPY(_host, _hostlen, _service, _servicelen, freehost);
                  have_oldres = 1;
               }

               break;
            }

            ++cbyaddr_hit;

            if (hostcache[i].gai_rc != 0) {
               socks_unlock(sockscf.hostfd, 0, 0);
               return hostcache[i].gai_rc;
            }

            DOCOPY(_host, _hostlen, _service, _servicelen, &hostcache[i]);

            socks_unlock(sockscf.hostfd, 0, 0);
            return 0;
         }
      }
   }

   ++cbyaddr_miss;

   socks_unlock(sockscf.hostfd, 0, 0);

#if SOCKSLIBRARY_DYNAMIC
   /*
    * In case getnameinfo(3) resolves to some other libresolv call
    * which we are also interpositioning. E.g. on FreeBSD 9.1 it's
    * getnameinfo(3) -> getipnodebyaddr(3) -> gethostbyaddr(3).
    * We don't want that; when we call getnameinfo(3), we don't want
    * that to resolve to something possibly interpositioned by us.
    */

   socks_markasnative("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

   gai_rc = getnameinfo(addr,
                        addrlen,
                        host,
                        sizeof(host),
                        service,
                        sizeof(service),
                        flags);

#if SOCKSLIBRARY_DYNAMIC
   socks_markasnormal("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

   slog(LOG_DEBUG, "%s: getnameinfo(%s) returned %d: %s",
        function,
        sockaddr2string(TOCSS(addr), NULL, 0),
        gai_rc,
        gai_strerror(gai_rc));

   if (gai_rc != 0 && GAI_RC_IS_SYSTEMPROBLEM(gai_rc)) {
      if (sockscf.state.reservedfdv[0] != -1) {
         close(sockscf.state.reservedfdv[0]);

         gai_rc = getnameinfo(addr,
                              addrlen,
                              host,
                              sizeof(host),
                              service,
                              sizeof(service),
                              flags);

         sockscf.state.reservedfdv[0] = makedummyfd(0, 0);

         if (gai_rc != 0) {
            slog(LOG_DEBUG, "%s: getnameinfo(%s) failed again: %s",
                 function,
                 sockaddr2string(TOCSS(addr), NULL, 0),
                 gai_strerror(gai_rc));
         }
      }
   }

   if (gai_rc != 0) {
      if (have_oldres) {
         slog(LOG_DEBUG,
              "%s: resolve of %s failed.  Returning stale cache entry",
              function, sockaddr2string(TOCSS(addr), NULL, 0));

         /* should be copied into _host and _service already. */
         return 0;
      }

      if (GAI_RC_IS_SYSTEMPROBLEM(gai_rc)) {
         swarn("%s: getnameinfo(%s) failed: %s",
               function,
               sockaddr2string(TOCSS(addr), NULL, 0),
               gai_strerror(gai_rc));

         return gai_rc;
      }
      else if (GAI_RC_IS_EAGAIN(gai_rc))
         return gai_rc;
      else if (GAI_RC_IS_INTERNALERROR(gai_rc))
         SERR(gai_rc);
   }

   socks_lock(sockscf.hostfd, 0, 0, 1, -1);

   if (freehost == NULL)
      freehost = getoldest(hashi);

   SASSERTX(freehost != NULL);

   freehost->gai_rc = gai_rc;

   STRCPY_ASSERTSIZE(freehost->service, service);
   STRCPY_ASSERTSIZE(freehost->data.getname.name, host);

   freehost->data.getname.flags = flags;
   freehost->written            = timenow;
   freehost->key                = id_addr;
   memcpy(&freehost->id.addr, addr, MIN(sizeof(freehost->id.addr), addrlen));

   freehost->allocated = 1;

   socks_unlock(sockscf.hostfd, 0, 0);

   if (freehost->gai_rc == 0)
      DOCOPY(_host, _hostlen, _service, _servicelen, freehost);

   return freehost->gai_rc;
}
#endif /* !SOCKS_CLIENT */


int
addrinfo_issupported(ai)
   const struct addrinfo *ai;
{

   if (!safamily_issupported(ai->ai_family))
      return 0;

   switch (ai->ai_socktype) {
      case 0: /* Solaris. :-(. */
      case SOCK_STREAM:
      case SOCK_DGRAM:
         break;

      default:
         return 0;
   }

   switch (ai->ai_protocol) {
      case 0: /* Solaris. :-(. */
      case IPPROTO_TCP:
      case IPPROTO_UDP:
         break;

      default:
         return 0;
   }

   return 1;
}

UNIT_TEST_STATIC_SCOPE int
addrinfocopy(to, from, hints)
   dnsinfo_t *to;
   const struct addrinfo *from;
   const struct addrinfo *hints;
{
   const char *function = "addrinfocopy()";
   const struct addrinfo *from_ai;
   const size_t maxentries = ELEMENTS(to->data.getaddr.ai_addr_mem);
   struct addrinfo *to_ai, *to_ai_previous;

#if !SOCKS_CLIENT

   struct addrinfo *to_ai_start;

#endif /* !SOCKS_CLIENT */

   size_t i;
   char visbuf[MAXHOSTNAMELEN * 4];

   bzero(to, sizeof(*to));

   from_ai        = from;
   to_ai          = &to->data.getaddr.addrinfo;

#if !SOCKS_CLIENT

   to_ai_start    = to_ai;

#endif /* !SOCKS_CLIENT */

   to_ai_previous = to_ai;
   i              = 0;

   while (i < maxentries && from_ai != NULL) {

#if !SOCKS_CLIENT
      int doskip = 0;

      if (!addrinfo_issupported(from_ai))
         doskip = 1;
      else if (i > 0
      &&       ntohs(GET_SOCKADDRPORT(TOSS(from_ai->ai_addr)))        == 0
      &&       ntohs(GET_SOCKADDRPORT(TOSS(to_ai_previous->ai_addr))) == 0
      &&       addrisinlist(from_ai->ai_addr, to_ai_start, to_ai_previous)) {
         /*
          * same address as before, just different protocol.  We don't
          * care about protocol and don't want to waste memory on that.
          *
          * We compare against 0 since a portnumber of 0 should mean
          * the caller doesn't care either (though this will still not
          * make things work in the generic case, where the caller may
          * be looking for an entry with the appropriate protocol),
          * and makes it easy to set a portnumber (different from 0)
          * when used in a particular unit-test.  Apart from that, it
          * would be more correct to ignore the portnumber of course.
          */
         slog(LOG_DEBUG, "%s: skipping address %s, protocol %d",
              function,
              sockaddr2string(TOSS(from_ai->ai_addr), NULL, 0),
              from_ai->ai_protocol);

         doskip = 1;
      }
      else if (from_ai->ai_addr->sa_family == AF_INET6
      && IN6_IS_ADDR_V4MAPPED(&TOIN6(from_ai->ai_addr)->sin6_addr)) {
         if (hints != NULL
         && hints->ai_family != 0
         && hints->ai_family != AF_INET)
            doskip = 1;
         else if (!external_has_safamily(AF_INET)) {
            /*
             * no point in converting to IPv4 if we don't have any
             * IPv4 address to use.  Lets hope there is another
             * address for this hostname.
             */
            doskip = 1;
         }
         else {
            /*
             * Have IPv4 on external interface and hints does not preclude
             * us from returning IPv4, so can convert the address from
             * IPv4-mapped IPv6 (which we don't want to use), to a regular
             * IPv4 address.
             */

            *to_ai = *from_ai; /* most attributes will remain the same. */

            to_ai->ai_addr = TOSA(&to->data.getaddr.ai_addr_mem[i]);
            bzero(to_ai->ai_addr, salen(AF_INET));
            SET_SOCKADDR(TOSS(to_ai->ai_addr), AF_INET);

            ipv4_mapped_to_regular(&TOIN6(from_ai->ai_addr)->sin6_addr,
                                   &TOIN(to_ai->ai_addr)->sin_addr);

            to_ai->ai_family  = AF_INET;
            to_ai->ai_addrlen = salen(AF_INET);
         }
      }
      else {
         *to_ai          = *from_ai;
         to_ai->ai_addr  = TOSA(&to->data.getaddr.ai_addr_mem[i]);
         memcpy(to_ai->ai_addr, from_ai->ai_addr, from_ai->ai_addrlen);
      }

      if (doskip) {
         slog(LOG_DEBUG, "%s: skipping address family %d",
              function, from_ai->ai_addr->sa_family);

         from_ai = from_ai->ai_next;
         continue;
      }

#else /* SOCKS_CLIENT */

      *to_ai          = *from_ai;
      to_ai->ai_addr  = TOSA(&to->data.getaddr.ai_addr_mem[i]);
      memcpy(to_ai->ai_addr, from_ai->ai_addr, from_ai->ai_addrlen);

#endif /* !SOCKS_CLIENT */

      if (from_ai->ai_canonname != NULL) {
         const size_t len = strlen(from_ai->ai_canonname);

         if (len >= sizeof(to->data.getaddr.ai_canonname_mem)) {
            swarnx("%s: DNS-name %s is %lu bytes long, expected max is %lu",
                   function,
                   str2vis(from_ai->ai_canonname, len, visbuf, sizeof(visbuf)),
                   (unsigned long)len,
                  (unsigned long)sizeof(to->data.getaddr.ai_canonname_mem) - 1);

            return EAI_MEMORY;
         }

         /*
          * Whether any entries but the first entry have an ai_canonname
          * appears to vary.
          *
          * glibc:     no, at least the version on 3.7.3-101.fc17.x86_64.
          * libresolv: yes, at least the version with FreeBSD 9.1-RELEASE.
          *
          * In both cases, all ai_canonnames are the same however, so
          * we do not need to allocate separate memory for them but can
          * let them all use the memory of the first entry.
          */

         if (i == 0) /* first entry; initialize memory. */
            memcpy(to->data.getaddr.ai_canonname_mem,
                   from_ai->ai_canonname,
                   len + 1);

         to_ai->ai_canonname = to->data.getaddr.ai_canonname_mem;
      }
      else
         to_ai->ai_canonname = NULL;

      from_ai        = from_ai->ai_next;

      to_ai_previous = to_ai;
      to_ai->ai_next = &to->data.getaddr.ai_next_mem[i];
      to_ai          = to_ai->ai_next;

      ++i;
   }

   to_ai->ai_next = NULL;

   if (from_ai == NULL || i >= maxentries)
      to_ai_previous->ai_next = NULL;

   if (i == 0) {
      slog(LOG_DEBUG, "%s: strange, no entries copied", function);

      bzero(&to->data.getaddr.addrinfo, sizeof(to->data.getaddr.addrinfo));
      return EAI_FAMILY;
   }

   return 0;
}


#if !SOCKS_CLIENT

void
hostcacheinvalidate(void)
{
   const char *function = "hostcacheinvalidate()";
   size_t i;

   slog(LOG_DEBUG, "%s", function);

   socks_lock(sockscf.hostfd, 0, 0, 0, 1);

   for (i = 0; i < SOCKD_HOSTCACHE; ++i)
      hostcache[i].allocated = 0;

   socks_unlock(sockscf.hostfd, 0, 0);
}

static int
gai2h_errno(gai_rc)
   const int gai_rc;
{

   switch (gai_rc) {
      case EAI_AGAIN:
      case EAI_MEMORY:
         return TRY_AGAIN;

      case EAI_NONAME:
         return HOST_NOT_FOUND;
   }

   return NO_RECOVERY;
}

void
hostcachesetup(void)
{
   const char *function = "hostcachesetup()";

   if ((sockscf.hostfd = socks_mklock(SOCKD_SHMEMFILE, NULL, 0)) == -1)
      serr("%s: socks_mklock() failed to create shmemfile using base %s",
           function, SOCKD_SHMEMFILE);

   if ((hostcache = sockd_mmap(NULL,
                               sizeof(*hostcache) * SOCKD_HOSTCACHE,
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED,
                               sockscf.hostfd,
                               1)) == MAP_FAILED)
      serr("%s: failed to mmap(2) hostcache of size %lu",
           function, (unsigned long)(sizeof(*hostcache) * SOCKD_HOSTCACHE));
}


struct hostent *
cgethostbyname(name)
   const char *name;
{
   const char *function = "cgethostbyname()";
   static hostentry_t hostentry;
   static dnsinfo_t resmem;
   static char *nullist[] = { NULL }, *addrv[HOSTENT_MAX_ALIASES + 1];
   struct addrinfo *ainfo, *next, hints;
   struct hostent he;
   size_t addrc;
   int rc;

   bzero(&hints, sizeof(hints));
   hints.ai_flags    = AI_CANONNAME;
   hints.ai_family   = AF_INET;

   if ((rc = cgetaddrinfo(name, NULL, &hints, &ainfo, &resmem)) != 0) {
      char visbuf[MAXHOSTNAMELEN * 4];

      slog(LOG_DEBUG, "%s: getaddrinfo(%s) failed: %s",
           function,
           str2vis(name, strlen(name), visbuf, sizeof(visbuf)),
           gai_strerror(rc));

      h_errno = gai2h_errno(rc);
      return NULL;
   }

   SASSERTX(ainfo != NULL);

   addrc = 0;
   for (next = ainfo; next != NULL; next = next->ai_next) {
      SASSERTX(addrc < ELEMENTS(addrv));
      SASSERTX(next->ai_addr    != NULL);
      SASSERTX(next->ai_family  == next->ai_addr->sa_family);

      if (next->ai_addr->sa_family == AF_INET) {
         he.h_addrtype     = next->ai_addr->sa_family;
         he.h_length       = sizeof(struct in_addr);
         addrv[addrc++]    = (char *)&TOIN(next->ai_addr)->sin_addr;

         if (addrc + 1 >= ELEMENTS(addrv))
            break;
      }
      else
         slog(LOG_DEBUG,
              "%s: ai_family = %d.  Skipped", function, next->ai_family);
   }

   if (addrc == 0) {
      h_errno = NO_ADDRESS;
      return NULL;
   }

   addrv[addrc] = NULL;

   he.h_name         = ainfo->ai_canonname;
   he.h_aliases      = nullist;
   he.h_addr_list    = addrv;

   hostentcopy(&hostentry, &he);

   return &hostentry.hostent;
}


struct hostent *
cgethostbyaddr(addr, len, type)
   const void *addr;
   socklen_t len;
   int type;
{
   const char *function = "cgethostbyaddr()";
   static hostentry_t hostentry;
   static char *nullist[] = { NULL }, *onelist[2];
   struct sockaddr_storage sa;
   struct hostent he;
   char host[MAXHOSTNAMELEN];
   int rc;

   SASSERTX(type == AF_INET);
   SASSERTX(len  == sizeof(struct in_addr));

   bzero(&sa, sizeof(sa));
   SET_SOCKADDR(&sa, AF_INET);
   TOIN(&sa)->sin_addr = *(const struct in_addr *)addr;

   if ((rc = cgetnameinfo(TOSA(&sa),
                          salen(sa.ss_family),
                          host,
                          sizeof(host),
                          NULL,
                          0,
                          NI_NAMEREQD)) != 0) {
      slog(LOG_DEBUG, "%s: cgetnameinfo(%s) failed: %s",
           function, sockaddr2string(&sa, NULL, 0), gai_strerror(rc));

      h_errno = gai2h_errno(rc);
      return NULL;
   }

   he.h_name         = host;
   he.h_aliases      = nullist;
   he.h_addrtype     = sa.ss_family;
   he.h_length       = sizeof(struct in_addr);
   he.h_addr_list    = onelist;
   he.h_addr_list[0] = (char *)&TOIN(&sa)->sin_addr;
   he.h_addr_list[1] = NULL;

   hostentcopy(&hostentry, &he);

   return &hostentry.hostent;
}

static hostentry_t *
hostentcopy(to, from)
   hostentry_t *to;
   const struct hostent *from;
{
   const char *function = "hostentcopy()";
   const size_t maxaliases = ELEMENTS(to->h_aliases);
   size_t i;

   if ((size_t)from->h_length > sizeof(to->h_addr_listmem[0])) {
      swarnx("%s: h_length of %s is %d bytes long, max expected is %lu",
             function,
             from->h_name,
             from->h_length,
             (unsigned long)sizeof(to->h_addr_listmem[0]));

      return NULL;
   }

   if (strlen(from->h_name) >= sizeof(to->h_name)) {
      swarnx("%s: name %s is %lu bytes long, max expected is %lu",
             function,
             from->h_name,
             (unsigned long)strlen(from->h_name),
             (unsigned long)sizeof(to->h_name) - 1);

      return NULL;
   }

   to->hostent.h_addrtype = from->h_addrtype;
   to->hostent.h_length   = from->h_length;
   to->hostent.h_name     = to->h_name;
   strcpy(to->hostent.h_name, from->h_name);

   for (i = 0; i < (maxaliases - 1) && from->h_aliases[i] != NULL; ++i) {
      if (strlen(from->h_aliases[i]) >= sizeof(to->h_aliasesmem[0])) {
         swarnx("%s: name %s is %lu bytes long, max expected is %lu",
                function,
                from->h_aliases[i],
                (unsigned long)strlen(from->h_aliases[i]),
                (unsigned long)sizeof(to->h_aliases[0]) - 1);

         return NULL;
      }

      to->h_aliases[i] = to->h_aliasesmem[i];
      strcpy(to->h_aliases[i], from->h_aliases[i]);
   }

   to->hostent.h_aliases     = to->h_aliases;
   to->hostent.h_aliases[i]  = NULL;

   for (i = 0; i < (maxaliases - 1) && from->h_addr_list[i] != NULL; ++i) {
      to->h_addr_list[i] = to->h_addr_listmem[i];
      memcpy(to->h_addr_list[i], from->h_addr_list[i], (size_t)from->h_length);
   }

   to->hostent.h_addr_list    = to->h_addr_list;
   to->hostent.h_addr_list[i] = NULL;

   return to;
}

static size_t
hosthash(name, size)
   const char *name;
   const size_t size;
{
   unsigned int value;
   char *end;

   if (size < MIN_HASH_SIZE)
      return 0;

   /* don't bother scanning past second dot. */
   if ((end = strchr(name, '.')) != NULL)
      end = strchr(end + 1, '.');

   if (end == NULL) /* zero or one dot in name. */
      end = strchr(name, NUL);

   SASSERTX(name <= end);

   value = 0;
   while (name != end) {
      value = (value << 5) + *name;   /* MAW - DS&A: Horner's rule. */
      ++name;
   }

   return value % size;
}

static size_t
addrhash(addr, size)
   const struct sockaddr_storage *addr;
   const size_t size;
{
   size_t sum;

   if (size < MIN_HASH_SIZE)
      /*
       * Assume this is so small a cache that it's better to just scan
       * it sequentially, rather than risk premature expiry of cached
       * DNS-entries.
       */
      return 0;

   switch (addr->ss_family) {
      case AF_INET:
         sum = ntohl(TOCIN(addr)->sin_addr.s_addr);
         break;

      case AF_INET6: {
         const unsigned char *byte   = TOCIN6(addr)->sin6_addr.s6_addr;
         const size_t len            = sizeof(TOCIN6(addr)->sin6_addr.s6_addr),
                      digits_wanted  = 4;
         ssize_t i;
         size_t digits_summed;

         sum           = 0;
         digits_summed = 0;
         i             = len - 1;

         while (i >= 0 && digits_summed < digits_wanted) {
            if (byte[i] != 0) {
               sum += byte[i];
               ++digits_summed;
            }

            --i;
         }

         break;
      }

      default:
         SERRX(addr->ss_family);
   }

   return sum % size;
}

dnsinfo_t *
getoldest(starti)
   const size_t starti;
{
   dnsinfo_t *oldest;
   size_t i;

   for (i = starti, oldest = NULL; i < SOCKD_HOSTCACHE; ++i) {
      if (oldest == NULL
      ||  hostcache[i].written < oldest->written)
         oldest = &hostcache[i];
   }

   SASSERTX(oldest != NULL);
   return oldest;
}

static int
name_matches_cache(ce, name, service, hints)
   const dnsinfo_t *ce; /* CacheEntry. */
   const char *name;
   const char *service;
   const struct addrinfo *hints;
{
   if (ce->allocated && ce->key == id_name) {
      if (hints == NULL) {
         if (ce->data.getaddr.hints != NULL)
            return 0;
      }
      else {
         if (ce->data.getaddr.hints == NULL)
            return 0;

         if (memcmp(ce->data.getaddr.hints, hints, sizeof(*hints)) != 0)
            return 0;
      }

      if  (strcasecmp(ce->id.name, name) != 0)
         return 0;

      if (service == NULL) {
         if (*ce->service != NUL)
            return 0;
      }
      else {
         if (*ce->service == NUL)
            return 0;

         if (strcasecmp(ce->service, service) != 0)
            return 0;
      }

      /* all matches */
      return 1;
   }

   return 0;
}

static int
addr_matches_cache(ce, addr, flags)
   const dnsinfo_t *ce; /* cache entry. */
   const struct sockaddr *addr;
   const int flags;
{
   if (ce->allocated
   &&  ce->data.getname.flags == flags
   &&  ce->key                == id_addr
   &&  sockaddrareeq(&ce->id.addr, TOCSS(addr), 0))
      return 1;

   return 0;
}

static int
addrisinlist(addr, ailist, ailist_last)
   const struct sockaddr *addr;
   struct addrinfo *ailist;
   struct addrinfo *ailist_last;
{
   const char *function = "addrisinlist()";
   struct addrinfo *ai_next_original = ailist_last->ai_next;
   int isinlist;

   ailist_last->ai_next = NULL;

   switch(addr->sa_family) {
      case AF_INET: {
         struct in_addr mask;/* not const due to Solaris's "struct in struct" */

         mask.s_addr = htonl(IPV4_FULLNETMASK);

         isinlist = ipv4_addrisinlist(&TOCIN(addr)->sin_addr, &mask, ailist)
                     == NULL ? 0 : 1;
         break;
      }

      case AF_INET6: {
         const unsigned int mask = { IPV6_NETMASKBITS };

         isinlist = ipv6_addrisinlist(&TOCIN6(addr)->sin6_addr, mask, ailist)
                    == NULL ? 0 : 1;
         break;
      }

      default:
         slog(PRERELEASE ? LOG_NOTICE /* does this ever happen? */ : LOG_DEBUG,
              "%s: unknown/unused sa_family %d in addrinfo struct",
              function, addr->sa_family);

         isinlist = 1;
   }

   ailist_last->ai_next = ai_next_original;

   return isinlist;
}
#endif /* !SOCKS_CLIENT */
