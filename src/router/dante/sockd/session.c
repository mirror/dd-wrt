/*
 * Copyright (c) 2005, 2008, 2009, 2010, 2011, 2012, 2013, 2024
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
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

#include "common.h"

static const char rcsid[] =
"$Id: session.c,v 1.102.4.2.14.2 2024/11/20 22:05:40 karls Exp $";

static int
session_isavailable(shmem_object_t *ss, const clientinfo_t *cinfo,
                    const int lock, const int mapisopen,
                    char *emsg, size_t emsglen);
/*
 * Returns true if there is at least one session free for use in the
 * "ss" object, assuming the client is "cinfo".
 * Returns false otherwise.
 *
 * Might also remove expired keystate entries from ss->keystate.

 * If "mapisopen" is true, any keymap in shmem to be used for keystate
 * is already open.  Upon function return, the open/close state of the
 * keystate map should remain as it was.
 */

static int
throttlepermits(const sessionthrottle_t *throttle,
                const struct timeval *starttime,
                const size_t client_since_starttime,
                const struct timeval *timenow);
/*
 * Checks whether the throttle "throttle" permits one more client
 * at time "timenow".  "clientsnow" is the current number of clients
 * related to the "throttle".
 *
 * Returns true if the throttle permits, false if not.
 */

static char *
sessionlimitstring(const size_t inuse, const size_t max,
                   char *buf, const size_t buflen);
/*
 * Return error string corresponding to throttle limit having been reached.
 */

static char *
throttlelimitstring(const struct timeval *starttime,
                    const struct timeval *timenow,
                    const size_t newclients, const size_t currentclients,
                    char *buf, const size_t buflen);

/*
 * Return error string corresponding to ratelimit having been reached.
 */



int
session_use(shmem, cinfo, lock, emsg, emsglen)
   shmem_object_t *shmem;
   const clientinfo_t *cinfo;
   const int lock;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "session_use()";
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
   shmem_object_t _shmem = *shmem;
#endif /* DEBUG */
   session_t *ss = &shmem->object.ss;
   struct timeval timenow = { 0, 0 };
   ssize_t index = -1;
   size_t mappedsize;

   SASSERTX(shmem != NULL);

   slog(LOG_DEBUG, "%s: shmid %lu, cinfo = %s, lock = %d",
        function, shmem->mstate.shmid, clientinfo2string(cinfo, NULL, 0), lock);

#if HAVE_SOCKS_HOSTID
   if (shmem->keystate.key == key_hostid)
      if (shmem->keystate.keyinfo.hostindex > cinfo->hostid.addrc) {
         snprintf(emsg, emsglen,
                  "session.state.hostindex specifies using hostindex #%u, but "
                  "client connection provides %u hostindex%s.  "
                  "Network/%s misconfiguration?",
                  (unsigned)shmem->keystate.keyinfo.hostindex,
                  (unsigned)cinfo->hostid.addrc,
                  (unsigned)cinfo->hostid.addrc == 1 ? "" : "es",
                  PRODUCT);

         slog(LOG_WARNING, "%s: problem related to %s #%lu detected: %s",
              function,
              objecttype2string(shmem->mstate.parenttype),
              (unsigned long)shmem->mstate.number,
              emsg);

         return 0;
      }
#endif /* HAVE_SOCKS_HOSTID */

   socks_lock(lock, (off_t)shmem->mstate.shmid, 1, 1, 1);

   if (shmem->keystate.key == key_unset)
      mappedsize = 0;
   else {
      int rc;

      MUNPROTECT_SHMEMHEADER(shmem);
      rc = keystate_openmap(shmem->mstate.shmid, &shmem->keystate, &mappedsize);
      MPROTECT_SHMEMHEADER(shmem);

      if (rc == -1) {
         socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);

         slog(LOG_DEBUG,
              "%s: keystate_openmap() of shmid %ld/key %d, failed: %s",
              function,
              shmem->mstate.shmid,
              (int)shmem->keystate.key,
              strerror(errno));

         return 0;
      }
      else
         slog(LOG_DEBUG,
              "%s: opened mmap(2) of size %lu at address %p for shmid %lu",
              function,
              (unsigned long)mappedsize,
              shmem->keystate.keyv,
              (unsigned long)shmem->mstate.shmid);
   }

   if (mappedsize != 0)
      SASSERTX(shmem->keystate.keyv != NULL);

   if (!session_isavailable(shmem, cinfo, -1, 1, emsg, emsglen)) {
      SASSERTX(mappedsize ==   shmem->keystate.keyc
                             * sizeof(*shmem->keystate.keyv));

      MUNPROTECT_SHMEMHEADER(shmem);

      keystate_closemap(shmem->mstate.shmid, &shmem->keystate, mappedsize, -1);

      MPROTECT_SHMEMHEADER(shmem);

      /*
       * *must* be after keystate_closemap() since keystate_closemap also
       * NULL's the map and this will affect other processes who open it.
       */
      socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);

      slog(LOG_DEBUG, "%s: no free session slots available: %s",
           function, emsglen == 0 ? "" : emsg);

      return 0;
   }

   if (shmem_use(shmem, cinfo, -1, 1) == -1) {
      MUNPROTECT_SHMEMHEADER(shmem);

      keystate_closemap(shmem->mstate.shmid,
                        &shmem->keystate,
                        mappedsize,
                        -1);

      MUNPROTECT_SHMEMHEADER(shmem);

      socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
      return 0;
   }

   /* may have changed if shmem_use() had to expand the array. */
   mappedsize = shmem->keystate.keyc * sizeof(*shmem->keystate.keyv);

   if (ss->throttle_isset) {
      struct timeval tdiff;

      if (!timerisset(&timenow))
         gettimeofday_monotonic(&timenow);

      timersub(&timenow, &ss->throttle.starttime, &tdiff);
      if (tdiff.tv_sec >= ss->throttle.limit.seconds)
         RESET_THROTTLE(&ss->throttle, timenow);

      ++ss->throttle.newclients;
   }

   if (ss->throttle_perstate_isset || ss->max_perstate_isset) {
      /*
       * shmem_use() should make sure an entry is allocated for this
       * address by now.
       */

      SASSERTX(mappedsize != 0);

      index = keystate_index(shmem, cinfo, 0);
      SASSERTX(index >= 0);
      SASSERTX((size_t)index < shmem->keystate.keyc);
   }

   if (ss->throttle_perstate_isset) {
      if (!timerisset(&timenow))
         gettimeofday_monotonic(&timenow);

      if (keystate_hasexpired(shmem, (size_t)index, &timenow))
         RESET_THROTTLE(&shmem->keystate.keyv[index].info.ss, timenow);

      ++shmem->keystate.keyv[index].info.ss.newclients;
   }

   MUNPROTECT_SHMEMHEADER(shmem);

   keystate_closemap(shmem->mstate.shmid, &shmem->keystate, mappedsize, index);

   MUNPROTECT_SHMEMHEADER(shmem);

   socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
   return 1;
}

void
session_unuse(ss, cinfo, lock)
   shmem_object_t *ss;
   const clientinfo_t *cinfo;
   const int lock;
{
   const char *function = "session_unuse()";
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
   shmem_object_t _shmem = *ss;
#endif /* DEBUG */

   SASSERTX(ss != NULL);

   slog(LOG_DEBUG, "%s: shmid = %ld, key = %d, cinfo = %s, lock = %d",
                   function,
                   ss->mstate.shmid,
                   (int)ss->keystate.key,
                   clientinfo2string(cinfo, NULL, 0),
                   lock);

   socks_lock(lock, (off_t)ss->mstate.shmid, 1, 1, 1);

   shmem_unuse(ss, cinfo, -1);

   if (sockscf.option.debug)
      slog(LOG_DEBUG, "%s: sessions left after unuse on shmid %lu, key %d: %s",
                      function,
                      (unsigned long)ss->mstate.shmid,
                      (int)ss->keystate.key,
                      session_isavailable(ss, cinfo, -1, 0, NULL, 0) ?
                           "yes" : "no");

   socks_unlock(lock, (off_t)ss->mstate.shmid, 1);
}

static int
session_isavailable(shmem, cinfo, lock, mapisopen, emsg, emsglen)
   shmem_object_t *shmem;
   const clientinfo_t *cinfo;
   const int lock;
   const int mapisopen;
   char *emsg;
   size_t emsglen;
{
   const char *function   = "session_isavailable()";
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
   shmem_object_t _shmem = *shmem;
#endif /* DEBUG */
   const session_t *ss    = &shmem->object.ss;
   const int doclosemap   = (mapisopen ? 0 : 1);
   struct timeval timenow = { 0, 0 };
   size_t mappedsize, inuse;
   ssize_t i;
   char buf[256], backupemsg[256];

   if (emsg == NULL || emsglen == 0) {
      emsg    = backupemsg;
      emsglen = sizeof(backupemsg);
   }

   SASSERTX(ss->max_isset
   ||       ss->throttle_isset
   ||       shmem->keystate.key != key_unset);

   socks_lock(lock, (off_t)shmem->mstate.shmid, 1, 1, 1);

   /*
    * Check not-keystate limits first.
    */

   if (ss->throttle_isset) {
      if (!timerisset(&timenow))
         gettimeofday_monotonic(&timenow);

      if (!throttlepermits(&ss->throttle.limit,
                           &ss->throttle.starttime,
                           ss->throttle.newclients,
                           &timenow)) {
         throttlelimitstring(&ss->throttle.starttime,
                             &timenow,
                             ss->throttle.newclients,
                             shmem->mstate.clients,
                             emsg,
                             emsglen);

         socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);

         slog(LOG_DEBUG, "%s: throttlelimit reached for shmid %lu: %s",
              function, (unsigned long)shmem->mstate.shmid, emsg);

         return 0;
      }
   }

   if (ss->max_isset) {
      const size_t left = (unsigned long)(ss->max - shmem->mstate.clients);

      SASSERTX(shmem->mstate.clients <= ss->max);

      slog(LOG_DEBUG, "%s: sessions in use for shmid %lu: %lu, max: %lu",
           function,
           (unsigned long)shmem->mstate.shmid,
           (unsigned long)shmem->mstate.clients,
           (unsigned long)ss->max);

      if (left <= 0) {
         sessionlimitstring(shmem->mstate.clients, ss->max, emsg, emsglen);

         socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
         slog(LOG_DEBUG, "%s: sessionlimit reached for shmid %lu: %s",
              function, (unsigned long)shmem->mstate.shmid, emsg);

         return 0;
      }
   }

   /*
    * not-keystate limits are ok.  Now check keystate-based limits, if any.
    */

   if (shmem->keystate.key == key_unset
   || shmem->keystate.keyc == 0 /* limit can't have been reached. */) {
      socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
      return 1;
   }

   if (mapisopen)
      SASSERTX(shmem->keystate.keyv != NULL);
   else {
      int rc;

      MUNPROTECT_SHMEMHEADER(shmem);

      rc = keystate_openmap(shmem->mstate.shmid, &shmem->keystate, &mappedsize);

      MPROTECT_SHMEMHEADER(shmem);

      if (rc == -1) {
         socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);

         slog(LOG_DEBUG,
              "%s: keystate_openmap() of shmid %lu/key %d failed: %s",
              function,
              shmem->mstate.shmid,
              (int)shmem->keystate.key,
              strerror(errno));

         return 0;
      }
   }

   SASSERTX(shmem->keystate.keyv != NULL);

   if ((i = keystate_index(shmem, cinfo, 0)) != -1) {
      SASSERTX(i >= 0);
      SASSERTX((size_t)i < shmem->keystate.keyc);

      switch (shmem->keystate.key) {
         case key_from:
            inuse = shmem->keystate.keyv[i].data.from.addrc;
            break;

#if HAVE_SOCKS_HOSTID
         case key_hostid:
            inuse = shmem->keystate.keyv[i].data.hostid.addrc;
            break;
#endif /* HAVE_SOCKS_HOSTID */

         default:
            SERRX(shmem->keystate.key);
      }

      if (ss->max_perstate_isset) {
         slog(LOG_DEBUG,
              "%s: per-key %s sessions in use for shmid %lu: %ld, max: %lu",
              function,
              statekey2string(shmem->keystate.key),
              (unsigned long)shmem->mstate.shmid,
              (long)inuse,
              (unsigned long)ss->max_perstate);

         SASSERTX(inuse <= ss->max_perstate);

         if (ss->max_perstate - inuse <= 0) {
            snprintf(emsg, emsglen, "per-key %s",
                     sessionlimitstring((size_t)inuse,
                                        ss->max_perstate,
                                        buf,
                                        sizeof(buf)));
            slog(LOG_DEBUG,
                 "%s: per-key sessionlimit reached for shmid %lu: %s",
                 function, (unsigned long)shmem->mstate.shmid, emsg);

            if (doclosemap) {
               MUNPROTECT_SHMEMHEADER(shmem);

               keystate_closemap(shmem->mstate.shmid,
                                 &shmem->keystate,
                                 mappedsize,
                                 -1);

               MUNPROTECT_SHMEMHEADER(shmem);
            }

            socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
            return 0;
         }
      }

      if (ss->throttle_perstate_isset) {
         SASSERTX(ss->throttle_perstate.limit.clients != 0);
         SASSERTX(ss->throttle_perstate.limit.seconds != 0);

         if (!timerisset(&timenow))
            gettimeofday_monotonic(&timenow);

         if (!throttlepermits(&ss->throttle_perstate.limit,
                              &shmem->keystate.keyv[i].info.ss.starttime,
                              shmem->keystate.keyv[i].info.ss.newclients,
                              &timenow)) {
            snprintf(emsg, emsglen, "per-key %s",
                     throttlelimitstring(
                                     &shmem->keystate.keyv[i].info.ss.starttime,
                                     &timenow,
                                     shmem->keystate.keyv[i].info.ss.newclients,
                                     inuse,
                                     buf,
                                     sizeof(buf)));

            slog(LOG_DEBUG,
                 "%s: per-key throttlelimit reached for shmid %lu: %s",
                 function, (unsigned long)shmem->mstate.shmid, emsg);

            if (doclosemap) {
               MUNPROTECT_SHMEMHEADER(shmem);

               keystate_closemap(shmem->mstate.shmid,
                                 &shmem->keystate,
                                 mappedsize,
                                 -1);

               MPROTECT_SHMEMHEADER(shmem);
            }

            socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
            return 0;
         }
      }
   }

   if (doclosemap) {
      MUNPROTECT_SHMEMHEADER(shmem);

      keystate_closemap(shmem->mstate.shmid, &shmem->keystate, mappedsize, -1);

      MPROTECT_SHMEMHEADER(shmem);
   }

   socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
   return 1;
}

static int
throttlepermits(throttle, starttime, newclients, timenow)
   const sessionthrottle_t *throttle;
   const struct timeval *starttime;
   const size_t newclients;
   const struct timeval *timenow;
{
   const char *function = "throttlepermits()";
   struct timeval tdiff;

   slog(LOG_DEBUG,
        "%s: throttle: %lu/%ld, starttime: %ld.%06ld, newclients: %lu, "
        "timenow: %ld.%06ld",
        function,
        (unsigned long)throttle->clients,
        (long)throttle->seconds,
        (long)starttime->tv_sec,
        (long)starttime->tv_usec,
        (unsigned long)newclients,
        (long)timenow->tv_sec,
        (long)timenow->tv_usec);

   if (newclients < throttle->clients)
      return 1;

   timersub(timenow, starttime, &tdiff);

   if (tdiff.tv_sec >= throttle->seconds)
      return 1;

   return 0;
}

static char *
sessionlimitstring(inuse, max, buf, buflen)
   const size_t inuse;
   const size_t max;
   char *buf;
   size_t buflen;
{

   SASSERTX(inuse <= max);

   snprintf(buf, buflen, "session limit reached (%ld/%ld slot%s in use)",
            (long)inuse, (long)max, max == 1 ? "" : "s");

   return buf;
}

static char *
throttlelimitstring(starttime, timenow, newclients, currentclients, buf, buflen)
   const struct timeval *starttime;
   const struct timeval *timenow;
   const size_t newclients;
   const size_t currentclients;
   char *buf;
   const size_t buflen;
{
   struct timeval tdiff;

   timersub(timenow, starttime, &tdiff);
   snprintf(buf, buflen,
            "session rate limit reached (already accepted %lu new client%s "
            "during the last %lds.  Current client count: %lu)",
            (unsigned long)newclients,
            (unsigned long)newclients == 1 ? "" : "s",
            (long)timeval2seconds(&tdiff),
            (unsigned long)currentclients);

   return buf;
}
