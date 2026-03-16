/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011,
 *               2012, 2013, 2014, 2016, 2024
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
"$Id: shmem.c,v 1.238.4.6.2.2.8.5 2024/11/21 22:39:03 michaels Exp $";


#define FIRST_SHMEMID  (0)

typedef enum { keytype_ipv4 = 1, keytype_ipv6 } keystate_data_type;

typedef struct {
   keystate_data_type type;

   union {
      struct in_addr  ipv4;
      struct in6_addr ipv6;
   } data;
} keystate_data_t;

static unsigned long
mem2shmem(struct config *config, const unsigned long firstid);
/*
 * Deallocates from all rules starting with "firstrule" ordinary memory for
 * objects that should be in shared memory and allocates shared memory
 * for it instead.
 * "firstid" is the id to use for the first allocation.  The id of
 * subsequent allocations is incremented by one.
 *
 * Returns the id of the last id used.
 */

static void
keystate_removeindex(keystate_t *keystate, const size_t index);
/*
 * Removes the keystate entry with index "index".
 */

static keystate_data_t *
keystate_data(const keystate_t *keystate, const size_t index,
              keystate_data_t *key);
/*
 * Returns the keystate data at index "index" corresponding to the key
 * set in keystate.
 */

static const char *
keydata2string(const keystate_data_t *keydata, char *buf, size_t buflen);
/*
 * Stores a string representation of keydata "keydata" in "buf" and
 * returns a pointer to buf.
 *
 * If "buf" is NULL, the representation is written to a local static buffer
 * and a pointer to it is returned.
 */

static size_t
keystate_clientcount(const keystate_t *keystate, const size_t index);
/*
 * Returns the current clientcount for the keystate entry with index "index".
 */

static ssize_t
keystate_timer(const shmem_object_t *shmem);
/*
 * is there a relevant timer we can use for calculating how often to
 * scan for old keystate entries to expire and remove in "shmem"?
 *
 * Returns the timer as number of seconds if so, or -1 if not.
 */

static int
should_do_expirescan(const shmem_object_t *shmem);
/*
 * Should we do a scan for old keyentries in shmem at this time?
 * Returns true if so, false otherwise.
 */

void
shmem_setup(void)
{
   const char *function = "shmem_setup()";
   char *p;

   /*
    * Main shmem-stuff.  sockd.conf and shmemfd used for locking access
    * to shmem-stuff.
    */

   SASSERTX(sockscf.shmemfd == -1);
   SASSERTX(pidismother(sockscf.state.pid) == 1);

   STRCPY_ASSERTSIZE(sockscf.shmem_fnamebase, SOCKD_SHMEMFILE);

   /*
    * First check that this works ok.  If e.g., user has changed some
    * config-files and made strings too long, it might fail.
    */
   if ((sockscf.shmemfd = socks_mklock(sockscf.shmem_fnamebase,
                                       sockscf.shmem_fnamebase,
                                       sizeof(sockscf.shmem_fnamebase))) == -1)
      serr("%s: socks_mklock() failed to create shmemfile using base %s",
           function, sockscf.shmem_fnamebase);

   if ((p = sockd_getshmemname((unsigned long)FIRST_SHMEMID, key_from)) == NULL)
      serrx("%s: failed to generate shmem filename based on fnamebase "
            "\"%s\" and id %d",
            function, sockscf.shmem_fnamebase, FIRST_SHMEMID);

   if (strlen(p) >= sizeof(sockscf.shmem_fnamebase))
      serrx("%s: shmem filename is %ld bytes too long for this system.  Please "
            "reduce the length of the SOCKD_SHMEMFILE define and recompile",
            function,
            (unsigned long)(strlen(p) + 1) - sizeof(sockscf.shmem_fnamebase));

   if ((sockscf.shmeminfo = sockd_mmap(NULL,
                                       sizeof(*sockscf.shmeminfo),
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED,
                                       sockscf.shmemfd,
                                       1)) == MAP_FAILED)
      serr("%s: failed to mmap shmeminfo", function);

   /* can unlink this file; all children will inherit the fd. */
   if (unlink(sockscf.shmem_fnamebase) != 0)
      serr("%s: failed to unlink shmemfile %s",
           function, sockscf.shmem_fnamebase);

   SASSERTX(sockscf.shmemconfigfd == -1);
   if ((sockscf.shmemconfigfd
   = socks_mklock(SOCKD_SHMEMFILE, NULL, 0)) == -1)
      serr("%s: socks_mklock() failed to create shmemfile using base %s",
           function, SOCKD_SHMEMFILE);

   /*
    * Hostcache also uses shmem, but has it's own area.
    */
   hostcachesetup();

#if HAVE_LDAP
   /*
    * And so does the LDAP module.
    */
   ldapcachesetup();
#endif /* HAVE_LDAP */

}

void
shmem_idupdate(config)
   struct config *config;
{
   static size_t lastshmid = FIRST_SHMEMID + 1;

   lastshmid = mem2shmem(config, lastshmid);
   ++lastshmid;
}

void
sockd_shmdt(rule, objects)
   rule_t *rule;
   const int objects;
{
   const char *function = "sockd_shmdt()";

   if ((objects & SHMEM_BW) && rule->bw_shmid)
      HANDLE_SHMDT(rule, bw, bw_shmid);

   if ((objects & SHMEM_MONITOR) && rule->mstats_shmid)
      HANDLE_SHMDT(rule, mstats, mstats_shmid);

   if ((objects & SHMEM_SS) && rule->ss_shmid)
      HANDLE_SHMDT(rule, ss, ss_shmid);
}

int
sockd_shmat(rule, objects)
   rule_t *rule;
   int objects;
{
   const char *function = "sockd_shmat()";
   int haveerror = 0;

   if ((objects & SHMEM_BW) && rule->bw_shmid != 0) {
      HANDLE_SHMAT(rule, bw, bw_shmid);

      if (rule->bw_shmid) {
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
         shmem_object_t _shmem = *rule->bw;
#endif /* DEBUG */

         SASSERTX(rule->bw->type == SHMEM_BW);
      }
      else {
         SASSERTX(rule->bw == NULL);

         haveerror = 1;
         objects  &= ~SHMEM_BW;
      }
   }
#if DEBUG
   else
      slog(LOG_DEBUG, "%s: no bw_shmid we need to (re)attach to for rule #%lu",
           function, (unsigned long)rule->number);
#endif /* DEBUG */

   if ((objects & SHMEM_MONITOR) && rule->mstats_shmid != 0) {
      /*
       * monitor-files are deleted and reset on every sighup, so can
       * be deleted even if mother still exists.
       */
      HANDLE_SHMAT(rule, mstats, mstats_shmid);

      if (rule->mstats_shmid) {
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
            shmem_object_t _shmem = *rule->mstats;
#endif /* DEBUG */

         SASSERTX(rule->mstats->type == SHMEM_MONITOR);
      }
      else {
         SASSERTX(rule->mstats == NULL);

         haveerror = 1;
         objects  &= ~SHMEM_MONITOR;
      }
   }
#if DEBUG
   else
      slog(LOG_DEBUG,
           "%s: no mstats_shmid we need to (re)attach to for rule #%lu",
           function, (unsigned long)rule->number);
#endif /* DEBUG */

   if ((objects & SHMEM_SS) && rule->ss_shmid != 0) {
      HANDLE_SHMAT(rule, ss, ss_shmid);

      if (rule->ss_shmid) {
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
         shmem_object_t _shmem = *rule->ss;
#endif /* DEBUG */

         SASSERTX(rule->ss->type == SHMEM_SS);
      }
      else {
         SASSERTX(rule->ss == NULL);

         haveerror = 1;
         objects  &= ~SHMEM_SS;
      }
   }
#if DEBUG
   else
      slog(LOG_DEBUG, "%s: no ss_shmid we need to (re)attach to for rule #%lu",
            function, (unsigned long)rule->number);
#endif /* DEBUG */

   if (haveerror)
      return -1;
   else
      return 0;
}

int
shmem_userule(rule, cinfo, emsg, emsglen)
   rule_t *rule;
   const clientinfo_t *cinfo;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "shmem_userule()";
   size_t attached_to;

   slog(LOG_DEBUG, "%s: cinfo: %s",
        function, clientinfo2string(cinfo, NULL, 0));

   log_ruleinfo_shmid(rule, function, NULL);

   /*
    * Do sessions first, since that's the one that can fail.
    */

   attached_to = 0;

   if (rule->ss_shmid != 0 && rule->ss == NULL)
      if (sockd_shmat(rule, SHMEM_SS) == 0)
         attached_to |= SHMEM_SS;

   if (rule->ss_shmid != 0) {
      if (!session_use(rule->ss,
                       cinfo,
                       sockscf.shmemfd,
                       emsg,
                       emsglen)) {
         SASSERTX(rule->ss_shmid == (rule)->ss->mstate.shmid);

         sockd_shmdt(rule, attached_to);
         return -1;
      }
   }

   if (rule->bw_shmid != 0 && rule->bw == NULL)
      if (sockd_shmat(rule, SHMEM_BW) == 0)
         attached_to |= SHMEM_BW;

   if (rule->bw_shmid != 0)
      bw_use(rule->bw, cinfo, sockscf.shmemfd);

   if (rule->mstats_shmid != 0 && rule->mstats == NULL)
      if (sockd_shmat(rule, SHMEM_MONITOR) == 0)
         attached_to |= SHMEM_MONITOR;

   if (rule->mstats_shmid != 0)
      monitor_use(rule->mstats, cinfo, sockscf.shmemfd);

   sockd_shmdt(rule, attached_to);

   return 0;
}


char *
sockd_getshmemname(id, key)
   const unsigned long id;
   const statekey_t key;
{
/*   const char *function = "sockd_getshmemname()"; */
   static char name[PATH_MAX];

   SASSERTX(*sockscf.shmem_fnamebase != NUL);

   if (key == key_unset)
      snprintf(name, sizeof(name), "%s.%lu", sockscf.shmem_fnamebase, id);
   else
      snprintf(name, sizeof(name), "%s.%lu.%d",
               sockscf.shmem_fnamebase, id, (int)key);

   return name;
}

void *
sockd_mmap(mapping, size, prot, flags, fd, docreate)
   void *mapping;
   size_t size;
   const int prot;
   const int flags;
   const int fd;
   const int docreate;
{
   const char *function = "sockd_mmap()";
   const void *oldmapping = mapping;

   if (size == 0)
      return MAP_FAILED;

   if (docreate) {
      if (ftruncate(fd, (off_t)size) == -1) {
         swarn("%s: ftruncate() to offset %lu failed",
               function, (unsigned long)size);

         return MAP_FAILED;
      }
   }

   slog(LOG_DEBUG, "%s: mmap(2)'ing %lu bytes from fd %d, mapping = %p",
        function, (unsigned long)size, fd, mapping);

   if ((mapping = mmap(mapping,
                       size,
                       prot,
                       flags,
                       fd,
                       (off_t)0)) == MAP_FAILED)
      swarn("%s: %smmap(2) of %lu bytes from fd %d failed",
            function, oldmapping == NULL ? "" : "re-", (unsigned long)size, fd);

   return mapping;
}

int
shmem_use(shmem, cinfo, lock, mapisopen)
   shmem_object_t *shmem;
   const clientinfo_t *cinfo;
   const int lock;
   const int mapisopen;
{
   const char *function = "shmem_use()";
   const int doclosemap = (mapisopen ? 0 : 1);
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
   shmem_object_t _shmem = *shmem;
#endif /* DEBUG */
   ssize_t statecount = -1;

   SASSERTX(shmem != NULL);
   SASSERTX(shmem->mstate.number > 0);

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG,
           "%s: pre-use: lock = %d, # of clients = %lu, rule = %lu, "
           "shmem = %p, keystate = %p, mapisopen = %d, cinfo = %s, shmid %ld, "
           "key %d",
           function,
           lock,
           (unsigned long)shmem->mstate.clients,
           (unsigned long)shmem->mstate.number,
           shmem,
           shmem->keystate.keyv,
           mapisopen,
           clientinfo2string(cinfo, NULL, 0),
           shmem->mstate.shmid,
           (int)shmem->keystate.key);

   switch (shmem->type) {
      case SHMEM_BW:
      case SHMEM_MONITOR:
      case SHMEM_SS:
         break;

      default:
         SERRX(shmem->type);
   }

   socks_lock(lock, (off_t)shmem->mstate.shmid, 1, 1, 1);

   MUNPROTECT_SHMEMHEADER(shmem);

   ++shmem->mstate.clients;

   MPROTECT_SHMEMHEADER(shmem);
   if (shmem->keystate.key != key_unset) {
      size_t sizemapped;
      ssize_t i;
      int rc;

      if (!mapisopen) {
         MUNPROTECT_SHMEMHEADER(shmem);

         rc = keystate_openmap(shmem->mstate.shmid,
                               &shmem->keystate,
                               &sizemapped);

         MPROTECT_SHMEMHEADER(shmem);

         if (rc != 0) {
            socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);

            slog(LOG_DEBUG, "%s: keystate_openmap() of shmid %lu failed: %s",
                 function, (unsigned long)shmem->mstate.shmid, strerror(errno));
            return -1;
         }
      }

      /*
       * increment the current count for address, or add an entry for the addr.
       */
      if ((i = keystate_index(shmem, cinfo, should_do_expirescan(shmem))) >= 0)
         slog(LOG_DEBUG, "%s: entry exists at index #%lu",
              function, (unsigned long)i);
      else {
         /*
          * a bit more complex.  Need to expand the array and add the new
          * entry; remap with a larger size first.
          */
         const char *fname = sockd_getshmemname(shmem->mstate.shmid,
                                                shmem->keystate.key);
         void *newmap;
         int fd;

         slog(LOG_DEBUG,
              "%s: need to expand keyv array to %lu entries for cinfo %s",
              function,
              (unsigned long)shmem->keystate.keyc + 1,
              clientinfo2string(cinfo, NULL, 0));

         if ((fd = open(fname, O_RDWR)) == -1) {
            swarn("%s: could not open shmemfile %s", function, fname);

            if (doclosemap) {
               MUNPROTECT_SHMEMHEADER(shmem);

               keystate_closemap(shmem->mstate.shmid,
                                 &shmem->keystate,
                                 sizemapped,
                                 -1);

               MPROTECT_SHMEMHEADER(shmem);
            }

            socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);

            return -1;
         }

         sizemapped
         = (shmem->keystate.keyc + 1) * sizeof(*shmem->keystate.keyv);

         MUNPROTECT_SHMEMHEADER(shmem);

         newmap = sockd_mmap(shmem->keystate.keyv,
                             sizemapped,
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED,
                             fd,
                             1);

         MPROTECT_SHMEMHEADER(shmem);

         close(fd);

         if (newmap == MAP_FAILED) {
            swarn("%s: failed to mmap(2) shmeminfo of size %lu from "
                  "file %s on fd %d",
                  function, (unsigned long)sizemapped, fname, fd);

            socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
            return -1;
         }

         shmem->keystate.keyv = newmap;

         i = shmem->keystate.keyc;

         MUNPROTECT_SHMEMHEADER(shmem);

         ++shmem->keystate.keyc;

         MPROTECT_SHMEMHEADER(shmem);
      }

      SASSERTX((size_t)i < shmem->keystate.keyc);

      MUNPROTECT_SHMEMHEADER(shmem);
      switch (shmem->keystate.key) {
         case key_from: {
            void *dst;

            switch (cinfo->from.ss_family) {
               case AF_INET:
                  dst = &shmem->keystate.keyv[i].data.from.addr.ipv4;
                  break;

               case AF_INET6:
                  dst = &shmem->keystate.keyv[i].data.from.addr.ipv6;
                  break;

               default:
                  SERRX(cinfo->from.ss_family);
            }

            shmem->keystate.keyv[i].data.from.safamily  = cinfo->from.ss_family;
            memcpy(dst,
                   GET_SOCKADDRADDR(&cinfo->from),
                   inaddrlen(cinfo->from.ss_family));
            ++shmem->keystate.keyv[i].data.from.addrc;

            statecount = (ssize_t)shmem->keystate.keyv[i].data.from.addrc;

            slog(LOG_DEBUG, "%s: updated entry for address %s at index #%lu",
                 function,
                 sockaddr2string2(&cinfo->from, 0, NULL, 0),
                 (unsigned long)i);

            break;
         }

#if HAVE_SOCKS_HOSTID

         case key_hostid:
            SASSERTX(shmem->keystate.keyinfo.hostindex <= cinfo->hostid.addrc);

            shmem->keystate.keyv[i].data.hostid.ipv4
            = *gethostidip(&cinfo->hostid, 
                           shmem->keystate.keyinfo.hostindex - 1);

            ++shmem->keystate.keyv[i].data.hostid.addrc;

            statecount = (ssize_t)shmem->keystate.keyv[i].data.hostid.addrc;

            break;

#endif /* HAVE_SOCKS_HOSTID */

         default:
            SERRX(shmem->keystate.key);
      }

      if (doclosemap) {
         MUNPROTECT_SHMEMHEADER(shmem);

         keystate_closemap(shmem->mstate.shmid,
                           &shmem->keystate,
                           sizemapped,
                           i);

         MPROTECT_SHMEMHEADER(shmem);
      }
   }
   MUNPROTECT_SHMEMHEADER(shmem);

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG,
           "%s: post-use: lock = %d, clients = %lu (%ld), shmem = %p, "
           "shmid %ld, cinfo = %s",
           function,
           lock,
           (unsigned long)shmem->mstate.clients,
           (long)statecount,
           shmem,
           shmem->mstate.shmid,
           clientinfo2string(cinfo, NULL, 0));

   socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
   return 0;
}

int
shmem_unuse(shmem, cinfo, lock)
   shmem_object_t *shmem;
   const clientinfo_t *cinfo;
   int lock;
{
   const char *function = "shmem_unuse()";
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
   shmem_object_t _shmem = *shmem;
#endif /* DEBUG */
   ssize_t statecount = -1;

   SASSERTX(shmem != NULL);
   SASSERTX(shmem->mstate.number > 0);

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: pre-use: lock = %d, # of clients = %lu, rule = %lu, "
                      "shmem = %p, cinfo = %s, shmid %ld, key %d",
                      function,
                      lock,
                      (unsigned long)shmem->mstate.clients,
                      (unsigned long)shmem->mstate.number,
                      shmem,
                      clientinfo2string(cinfo, NULL, 0),
                      shmem->mstate.shmid,
                      (int)shmem->keystate.key);

   switch (shmem->type) {
      case SHMEM_BW:
      case SHMEM_MONITOR:
      case SHMEM_SS:
         break;

      default:
         SERRX(shmem->type);
   }

   socks_lock(lock, (off_t)shmem->mstate.shmid, 1, 1, 1);

   if (shmem->keystate.key != key_unset) {
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
      keystate_t _keystate;
#endif /* DEBUG */
      ssize_t i;
      size_t sizemapped;
      int rc;

      SASSERTX(shmem->keystate.keyv == NULL);
      SASSERTX(shmem->keystate.keyc > 0);

      MUNPROTECT_SHMEMHEADER(shmem);

      rc = keystate_openmap(shmem->mstate.shmid, &shmem->keystate, &sizemapped);

      MPROTECT_SHMEMHEADER(shmem);

      if (rc != 0) {
         socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);

         slog(LOG_DEBUG, "%s: keystate_openmap() of shmid %lu failed: %s",
              function, (unsigned long)shmem->mstate.shmid, strerror(errno));

         return -1;
      }

#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
      _keystate = shmem->keystate;
#endif /* DEBUG */

      SASSERTX(sizemapped > 0);
      SASSERTX(shmem->keystate.keyv != NULL);
      SASSERTX(shmem->keystate.keyc > 0);

      i = keystate_index(shmem, cinfo, should_do_expirescan(shmem));
      SASSERTX(i >= 0);
      SASSERTX((size_t)i < shmem->keystate.keyc);

      MUNPROTECT_SHMEMHEADER(shmem);

      switch (shmem->keystate.key) {
         case key_from:
            SASSERTX(shmem->keystate.keyv[i].data.from.addrc > 0);
            --shmem->keystate.keyv[i].data.from.addrc;
            statecount = (ssize_t)shmem->keystate.keyv[i].data.from.addrc;
            break;

#if HAVE_SOCKS_HOSTID
         case key_hostid:
            SASSERTX(shmem->keystate.keyv[i].data.hostid.addrc > 0);
            --shmem->keystate.keyv[i].data.hostid.addrc;
            statecount = (ssize_t)shmem->keystate.keyv[i].data.hostid.addrc;
            break;
#endif /* HAVE_SOCKS_HOSTID */

         default:
            SERRX(shmem->keystate.key);
      }

      keystate_closemap(shmem->mstate.shmid, &shmem->keystate, sizemapped, i);

      MPROTECT_SHMEMHEADER(shmem);
   }

   MUNPROTECT_SHMEMHEADER(shmem);

   SASSERTX(shmem->mstate.clients > 0);
   --shmem->mstate.clients;

   MPROTECT_SHMEMHEADER(shmem);

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG,
           "%s: post-use: lock = %d, clients = %lu (%ld), shmem = %p, "
           "shmid = %ld, cinfo = %s",
           function,
           lock,
           (unsigned long)shmem->mstate.clients,
           (long)statecount,
           shmem,
           shmem->mstate.shmid,
           clientinfo2string(cinfo, NULL, 0));

   socks_unlock(lock, (off_t)shmem->mstate.shmid, 1);
   return 0;
}


int
keystate_hasexpired(shmem, keyindex, timenow)
   const shmem_object_t *shmem;
   const size_t keyindex;
   const struct timeval *timenow;
{
   const char *function = "keystate_hasexpired()";
   struct timeval tdiff;
   char buf[64];
   int expired = 0;

   SASSERTX(keyindex < shmem->keystate.keyc);

   switch (shmem->type) {
      case SHMEM_SS: {

         if (!shmem->object.ss.throttle_perstate_isset)
            break; /* only throttle-limit can expire. */

         timersub(timenow,
                  &shmem->keystate.keyv[keyindex].info.ss.starttime,
                  &tdiff);

         if (tdiff.tv_sec >= shmem->object.ss.throttle_perstate.limit.seconds)
            expired = 1;

         break;
      }

      default:
         break;
   }

   if (expired) {
      if (!timerisset(&shmem->keystate.keyv[keyindex].info.ss.starttime))
         snprintf(buf, sizeof(buf), "yes (init)");
      else
         snprintf(buf, sizeof(buf), "yes, %ld.%06lds ago",
                  (long)tdiff.tv_sec, (long)tdiff.tv_usec);
   }

   if (sockscf.option.debug) {
      const char *keystring;
      keystate_data_t key;

      keystate_data(&shmem->keystate, keyindex, &key);
      keystring = keydata2string(&key, NULL, 0);

      slog(LOG_DEBUG,
           "%s: keystate address %s, index %lu, starttime = %ld.%06ld, "
           "clients = %lu, expired: %s",
           function,
           keystring,
           (unsigned long)keyindex,
           (long)shmem->keystate.keyv[keyindex].info.ss.starttime.tv_sec,
           (long)shmem->keystate.keyv[keyindex].info.ss.starttime.tv_usec,
           (unsigned long)keystate_clientcount(&shmem->keystate, keyindex),
           expired ? buf : "no");
   }

   return expired;
}

static unsigned long
mem2shmem(config, firstid)
   struct config *config;
   const unsigned long firstid;
{
   const char *function = "mem2shmem()";
   monitor_t *monitor;
   rule_t *rule;
   rule_t *rulev[]    = { config->crule,

#if HAVE_SOCKS_HOSTID
                          config->hrule,
#endif /* HAVE_SOCKS_HOSTID */

#if HAVE_SOCKS_RULES
                          config->srule
#endif /* HAVE_SOCKS_RULES */
                               };
   unsigned long nextid;
   size_t i;

   /*
    * Only main mother allocates the memory.  Children just set the
    * shmid value and use it to attach to the memory as needed later on.
    * Mother makes sure all the ids are in consecutive order starting
    * from the passed "firstid" argument, so children just need to
    * increment it to get the shmid of the next object.
    */
   SASSERTX(pidismother(sockscf.state.pid) == 1);

/*
 * Mother needs to create and fill in the correct contents initially,
 * but after that, the child-processes attach and update the segments
 * as needed.
 */
#define HANDLE_SHMCR(object, memfield, idfield, id)                            \
do {                                                                           \
   const statekey_t key = (object)->memfield->keystate.key;                    \
   const char *fname    = sockd_getshmemname(id, key_unset);                   \
   const int flags      = O_RDWR  | O_CREAT | O_EXCL;                          \
   const mode_t mode    = S_IRUSR | S_IWUSR;                                   \
   shmem_object_t *mem;                                                        \
   int fd;                                                                     \
                                                                               \
   SASSERTX(id != 0);                                                          \
   SASSERTX(fname != NULL && *fname != NUL);                                   \
                                                                               \
   SASSERTX((object)->memfield->mstate.number     > 0);                        \
   SASSERTX((object)->memfield->mstate.parenttype != object_none);             \
   SASSERTX((object)->type                        != object_none);             \
   SASSERTX((object)->memfield->type              != SHMEM_NONE);              \
                                                                               \
   if ((fd = open(fname, flags, mode)) == -1)                                  \
      serr("%s: failed to create shmemfile \"%s\"", function, fname);          \
                                                                               \
   slog(LOG_DEBUG,                                                             \
        "%s: will use filename %s for shmid %ld when creating shmem segment "  \
        "for " #memfield " in %s #%lu",                                        \
        function,                                                              \
        fname,                                                                 \
        (id),                                                                  \
        objecttype2string(object->type),                                       \
        (unsigned long)(object)->number);                                      \
                                                                               \
   if ((mem = sockd_mmap(NULL,                                                 \
                         sizeof(*(object)->memfield),                          \
                         PROT_READ | PROT_WRITE,                               \
                         MAP_SHARED,                                           \
                         fd,                                                   \
                         1)) == MAP_FAILED)                                    \
      serr("%s: sockd_mmap of size %lu failed",                                \
           function, (unsigned long)sizeof(*(object)->memfield));              \
                                                                               \
                                                                               \
   /*                                                                          \
    * replace the ordinary memory with shared memory.                          \
    */                                                                         \
   *mem               = *(object)->memfield;                                   \
   free((object)->memfield);                                                   \
   (object)->memfield = mem;                                                   \
                                                                               \
   close(fd);                                                                  \
                                                                               \
   if (key != key_unset) {                                                     \
      const char *fname = sockd_getshmemname(id, key);                         \
                                                                               \
      SASSERTX(fname != NULL && *fname != NUL);                                \
                                                                               \
      if ((fd = open(fname, flags, mode)) == -1)                               \
         serr("%s: failed to create file %s", function, fname);                \
                                                                               \
      /*                                                                       \
       * Just create the file for now.  Nothing to init.                       \
       */                                                                      \
                                                                               \
      close(fd);                                                               \
                                                                               \
      slog(LOG_DEBUG,                                                          \
           "%s: will use filename %s for shmid %lu/key %lu when creating "     \
           "shmem segment for " #memfield " in %s #%lu",                       \
           function,                                                           \
           fname,                                                              \
           (unsigned long)(id),                                                \
           (unsigned long)(key),                                               \
           objecttype2string((object)->type),                                  \
           (unsigned long)(object)->number);                                   \
   }                                                                           \
                                                                               \
   SASSERTX((object)->memfield->mstate.shmid == 0);                            \
   (object)->memfield->mstate.shmid = (id);                                    \
                                                                               \
   SASSERTX((object)->idfield == 0);                                           \
   (object)->idfield = (id);                                                   \
                                                                               \
   slog(LOG_DEBUG, "%s: allocated " #idfield " %ld for object #%lu",           \
        function, (object)->idfield, (unsigned long)(object)->number);         \
                                                                               \
   SASSERTX(munmap((object)->memfield, sizeof(*(object)->memfield)) == 0);     \
   (object)->memfield = NULL;                                                  \
} while (/* CONSTCOND */ 0)


   nextid = firstid;

   /*
    * Shmem for rules.
    */
   for (i = 0; i < ELEMENTS(rulev); ++i) {
      rule = rulev[i];

      while (rule != NULL) {
         if (rule->bw != NULL) {
            SASSERTX(rule->bw->type == SHMEM_BW);
            HANDLE_SHMCR(rule, bw, bw_shmid, nextid);
            ++nextid;
         }

         if (rule->ss != NULL) {
            SASSERTX(rule->ss->type == SHMEM_SS);
            HANDLE_SHMCR(rule, ss, ss_shmid, nextid);
            ++nextid;
         }

         rule = rule->next;
      }
   }

   /*
    * Shmem for monitors.
    */
   for (monitor = config->monitor; monitor != NULL; monitor = monitor->next) {
      if (monitor->mstats != NULL) {
         SASSERTX(monitor->mstats->type == SHMEM_MONITOR);
         HANDLE_SHMCR(monitor, mstats, mstats_shmid, nextid);
         ++nextid;
      }
   }

   slog(LOG_DEBUG,
        "%s: ok, allocated %ld shared memory id%s, first id is %lu, "
        "last id is %lu",
        function,
        (long)(nextid - firstid),
        nextid - firstid == 1 ? "" : "s",
        firstid,
        nextid);

   return nextid;
}

static void
keystate_removeindex(keystate, index)
   keystate_t *keystate;
   const size_t index;
{
   const char *function = "keystate_removeindex()";
   sa_family_t safamily;
   void *addr;

   switch (keystate->key) {
      case key_from:
         switch (keystate->keyv[index].data.from.safamily) {
            case AF_INET:
               safamily = AF_INET;
               addr     = &keystate->keyv[index].data.from.addr.ipv4;
               break;

            case AF_INET6:
               safamily = AF_INET6;
               addr     = &keystate->keyv[index].data.from.addr.ipv6;
               break;

            default:
               SERRX(keystate->keyv[index].data.from.safamily);
         }

         break;


#if HAVE_SOCKS_HOSTID
      case key_hostid:
         safamily = AF_INET6;
         addr     = &keystate->keyv[index].data.hostid.ipv4;
         break;
#endif /* HAVE_SOCKS_HOSTID */

      default:
         SERRX(keystate->key);
   }

   if (sockscf.option.debug) {
      char ntop[MAXSOCKADDRSTRING];

      if (inet_ntop(safamily, addr, ntop, sizeof(ntop)) == NULL) {
         swarn("%s: inet_ntop(3) failed on safamily %s, addr %s",
              function,
              safamily2string(safamily),
              addr2hexstring(addr, safamily, NULL, 0));
      }

      slog(LOG_DEBUG,
           "%s: removing entry for address %s (key: %s) at index #%lu "
           "from keystate. Will have %lu entries in keyv afterwards",
           function,
           ntop,
           statekey2string(keystate->key),
           (unsigned long)index,
           (unsigned long)keystate->keyc - 1);
   }


   /*
    * bzero the removed entry, so that if somebody overwrites it before
    * we close and reopen the mmap(2)-ed file, it will be zero.
    */
   bzero(&keystate->keyv[index], sizeof(*keystate->keyv));

   if (keystate->keyc > 1)
      memmove(&keystate->keyv[index],
              &keystate->keyv[index + 1],
              sizeof(*keystate->keyv) * (keystate->keyc - (index + 1)));

   --keystate->keyc;

   if (sockscf.option.debug) {
      size_t i;

      for (i = 0; i < keystate->keyc; ++i) {
         const char *keystring;
         keystate_data_t key;

         keystate_data(keystate, i, &key);
         keystring = keydata2string(&key, NULL, 0);

         slog(LOG_DEBUG, "%s: entry #%lu: %s",
              function, (unsigned long)i, keystring);
      }
   }
}

ssize_t
keystate_index(shmem, cinfo, doexpirescan)
   shmem_object_t *shmem;
   const clientinfo_t *cinfo;
   const int doexpirescan;
{
   const char *function = "keystate_index()";
   const void *datatomatch;
   static ssize_t lastmatched = -1;
   keystate_data_t keydata;
   keystate_data_type datatype;
   struct timeval timenow;
   ssize_t matchedi;
   size_t i, datalen;
   char extrainfo[128] = { NUL };

   if (doexpirescan)
      gettimeofday_monotonic(&timenow);

   SASSERTX(shmem->keystate.key != key_unset);

   switch (shmem->keystate.key) {
      case key_from:
         switch (cinfo->from.ss_family) {
            case AF_INET:
               datatype = keytype_ipv4;
               break;

            case AF_INET6:
               datatype = keytype_ipv6;
               break;

            default:
               SERRX(cinfo->from.ss_family);
         }

         datatomatch = GET_SOCKADDRADDR(&cinfo->from);
         datalen     = inaddrlen(cinfo->from.ss_family);
         break;

#if HAVE_SOCKS_HOSTID

      case key_hostid:
         SASSERTX(shmem->keystate.keyinfo.hostindex <= cinfo->hostid.addrc);

         datatomatch = gethostidip(&cinfo->hostid,
                                   shmem->keystate.keyinfo.hostindex - 1);

         datatype    = keytype_ipv4;
         datalen     = sizeof(keydata.data.ipv4);

         snprintf(extrainfo, sizeof(extrainfo), "(hostindex = %u)",
                  (unsigned)shmem->keystate.keyinfo.hostindex);

         break;

#endif /* HAVE_SOCKS_HOSTID */

      default:
         SERRX(shmem->keystate.key);
   }

   if (lastmatched != -1 && lastmatched < (ssize_t)shmem->keystate.keyc) {
      keystate_data(&shmem->keystate, lastmatched, &keydata);

      if (keydata.type == datatype) {
         switch (keydata.type) {
            case keytype_ipv4:
               if (memcmp(&keydata.data.ipv4, datatomatch, datalen) == 0)
                  matchedi = lastmatched;
               else
                  matchedi = -1;
               break;

            case keytype_ipv6:
               if (memcmp(&keydata.data.ipv6, datatomatch, datalen) == 0)
                  matchedi = lastmatched;
               else
                  matchedi = -1;
               break;

            default:
               SERRX(keydata.type);
         }
      }
      else {
         slog(LOG_DEBUG, "%s: keydata.type (%u) != datatype (%u)",
              function, (unsigned)keydata.type, (unsigned)datatype);

         matchedi = -1;
      }
   }
   else
      matchedi = -1;

   if (sockscf.option.debug) {
      char ntop[MAXSOCKADDRSTRING];

      switch (datatype) {
         case keytype_ipv4:
            if (inet_ntop(AF_INET, datatomatch, ntop, sizeof(ntop)) == NULL) {
               swarn("%s: inet_ntop(3) failed on %s %x",
                    function,
                    atype2string(AF_INET),
                    ((const struct in_addr *)datatomatch)->s_addr);

               snprintf(ntop, sizeof(ntop), "<unknown>");
            }
            break;

         case keytype_ipv6:
            if (inet_ntop(AF_INET6, datatomatch, ntop, sizeof(ntop)) == NULL) {
               swarn("%s: inet_ntop(3) failed on %s " IP6_FMTSTR,
                    function,
                    atype2string(AF_INET6),
                    IP6_ELEMENTS((const struct in6_addr *)datatomatch));

               snprintf(ntop, sizeof(ntop), "<unknown>");
            }
            break;

         default:
            SERRX(datatype);
      }

      slog(LOG_DEBUG,
           "%s: trying to find a match for key %s, address %s%s%s in keyv "
           "array with %lu entries.  Doexpirescan = %d, matchedi = %ld%s",
           function,
           statekey2string(shmem->keystate.key),
           ntop,
           *extrainfo == NUL ? "" : " ",
           *extrainfo == NUL ? "" : extrainfo,
           (unsigned long)shmem->keystate.keyc,
           doexpirescan,
           (long)matchedi,
           matchedi == -1 ?
               "" : doexpirescan ?
                 ".  Not scanning since index from last time matches" :  "");
   }

   if (matchedi != -1)
      return matchedi;

   i  = 0;
   while (i < shmem->keystate.keyc) {
      keystate_data_t keydata;
      int matches;

      if (doexpirescan && keystate_hasexpired(shmem, i, &timenow)) {
         MUNPROTECT_SHMEMHEADER(shmem);

         if (keystate_clientcount(&shmem->keystate, i) == 0) {
            keystate_removeindex(&shmem->keystate, i);
            continue;
         }
         else /* can't remove as long as long as entry is in use; just reset. */
            RESET_THROTTLE(&shmem->object.ss.throttle, timenow);

         MPROTECT_SHMEMHEADER(shmem);
      }

      keystate_data(&shmem->keystate, i, &keydata);

      if (keydata.type == datatype) {
         switch (keydata.type) {
            case keytype_ipv4:
               matches
               = (memcmp(&keydata.data.ipv4, datatomatch, datalen) == 0);
               break;

            case keytype_ipv6:
               matches
               = (memcmp(&keydata.data.ipv6, datatomatch, datalen) == 0);
               break;

            default:
               SERRX(keydata.type);
         }

         if (sockscf.option.debug)
            slog(LOG_DEBUG,
                 "%s: compared against %s at index #%lu using %lu bytes: %s",
                 function,
                 keydata2string(&keydata, NULL, 0),
                 (unsigned long)i,
                 (unsigned long)datalen,
                 matches ? "matches" : "no match");
      }
      else /* can't possibly match. */
         matches = 0;

      if (matches) {
         SASSERTX(matchedi == -1);
         matchedi = lastmatched = (ssize_t)i;

         if (!doexpirescan)
            break;
      }

      ++i;
   }

   if (doexpirescan) {
      MUNPROTECT_SHMEMHEADER(shmem);

      shmem->keystate.lastexpirescan = timenow.tv_sec;

      MPROTECT_SHMEMHEADER(shmem);
   }
   return matchedi;
}

int
keystate_openmap(id, keystate, sizemapped)
   const unsigned long id;
   keystate_t *keystate;
   size_t *sizemapped;
{
   const char *function = "keystate_openmap()";
   const char *fname    = sockd_getshmemname(id, keystate->key);
   size_t _sizemapped;

   SASSERTX(fname != NULL && *fname != NUL);
   SASSERTX(keystate->key != key_unset);

   slog(LOG_DEBUG, "%s: shmid %ld, key %d",
        function, id, (int)keystate->key);

   if (sizemapped == NULL)
      sizemapped = &_sizemapped;

   *sizemapped = keystate->keyc * sizeof(*keystate->keyv);

   if (*sizemapped == 0) {
      SASSERTX(keystate->keyc == 0);
      SASSERTX(keystate->keyv == NULL);
   }
   else {
      const int fd = open(fname, O_RDWR);

      if (fd == -1) {
         slog(sockd_motherexists() ? LOG_WARNING: LOG_DEBUG,
               "%s: could not open shmemfile %s generated from "
               "id %ld, key %d: %s",
               function,
               fname,
               id,
               (int)keystate->key,
               getppid() == 1 ?
                    "probably because mother has exited and deleted it already"
                  : strerror(errno));

         return -1;
      }

      if ((keystate->keyv = sockd_mmap(NULL,
                                       *sizemapped,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED,
                                       fd,
                                       0)) == MAP_FAILED) {
         swarn("%s: could not mmap shmemfile %s of size %lu generated from "
               "id %ld, key %d",
               function,
               fname,
               (unsigned long)*sizemapped,
               id,
               (int)keystate->key);

         keystate->keyv = NULL;
         close(fd);

         return -1;
      }

      close(fd);
   }

   slog(LOG_DEBUG, "%s: mapped %lu bytes for shmid %ld, key %d, at address %p",
                   function,
                   (unsigned long)*sizemapped,
                   id,
                   (int)keystate->key,
                   keystate->keyv);

   return 0;
}

void
keystate_closemap(id, keystate, mappedsize, changedindex)
   const unsigned long id;
   keystate_t *keystate;
   const size_t mappedsize;
   const ssize_t changedindex;
{
   const char *function = "keystate_closemap()";
   const char *fname    = sockd_getshmemname(id, keystate->key);
   size_t newsize;

   newsize = keystate->keyc * sizeof(*keystate->keyv);

   slog(LOG_DEBUG,
        "%s: mapped size for keystate of id %lu is %lu, newsize is %lu, "
        "changedindex: %ld",
        function,
        (unsigned long)id,
        (unsigned long)mappedsize,
        (unsigned long)newsize,
        (long)changedindex);

   if (sockscf.option.debug) {
      keystate_data_t keydata;
      size_t i;

      for (i = 0; i < keystate->keyc; ++i)
         slog(LOG_DEBUG, "%s: entry #%lu: %s",
              function,
              (unsigned long)i,
              keydata2string(keystate_data(keystate, i, &keydata), NULL, 0));
   }

   if (mappedsize == 0)
      SASSERTX(keystate->keyv == NULL);
   else {
#if !HAVE_UNIFIED_BUFFERCACHE
      /*
       * With a unified buffer cache, modifications to the mmap(2)-ed file will
       * be visible by other processes that open(2) and mmap(2) the same file
       * without having to msync(2) the memory to file.  I.e., this will work
       * Process1:
       *    t1: open(2) file F.
       *    t2: mmap(2) file F.
       *    t3: <modify mmap(2)-ed memory>
       *
       * Process2:
       *    t4: open(2) file F.
       *    t5: mmap(2) file F.
       *    t6: <see same thing Process1 sees>
       *
       * This code is for platforms where it does not work.  Currently
       * only OpenBSD AFAIK (5.1 at least).
       */
      void *addr;
      size_t len;
      int needmsync;

      SASSERTX(keystate->keyv != NULL);

      if (mappedsize < newsize) {
         needmsync = 1;

         /*
          * don't know what has changed, have to msync everything.
          */

         addr = keystate->keyv;
         len  = newsize;
      }
      else if (changedindex != -1) {
         needmsync = 1;

         addr = (void *)ROUNDDOWN((uintptr_t)&keystate->keyv[changedindex],
                                  sockscf.state.pagesize);

         SASSERTX((uintptr_t)keystate->keyv <= (uintptr_t)addr);

         len = ((uintptr_t)&keystate->keyv[changedindex] - (uintptr_t)addr)
               + sizeof(keystate->keyv[changedindex]);
      }
      else {
         SASSERTX(mappedsize == newsize);
         needmsync = 0;
      }

      if (needmsync) {
         const int rc = msync(addr, len, MS_SYNC);

         slog(rc == 0 ? LOG_DEBUG : LOG_WARNING,
              "%s: msync(%p, %lu, MS_SYNC), based on mappedsize %lu, "
              "newsize %lu, changedindex %ld, keyv %p %s (%s)",
              function,
              addr,
              len,
              (unsigned long)mappedsize,
              (unsigned long)newsize,
              (long)changedindex,
              keystate->keyv,
              rc == 0 ? "is ok" : "failed",
              strerror(errno));
      }
#endif /* !HAVE_UNIFIED_BUFFERCACHE */

      SASSERTX(keystate->keyv != NULL);

      if (munmap(keystate->keyv, mappedsize) != 0)
         swarn("%s: munmap(2) of keystate.keyv (%p) of size %lu failed",
               function, keystate->keyv, (unsigned long)mappedsize);

      keystate->keyv = NULL;
   }

   if (newsize != mappedsize) {
      if (truncate(fname, (off_t)newsize) == 0)
         slog(LOG_DEBUG, "%s: truncated shmemfile %s to size %lu",
                         function, fname, (unsigned long)newsize);
      else
         swarn("%s: could not truncate shmemfile %s to size %lu",
               function, fname, (unsigned long)newsize);
   }
}

#if DIAGNOSTIC && !SOCKS_CLIENT /* for internal debugging/testing. */
void
shmemcheck(void)
{
   const int errno_s = errno;
   rule_t *rulev[]    = { sockscf.crule,

#if HAVE_SOCKS_HOSTID
                          sockscf.hrule,
#endif /* HAVE_SOCKS_HOSTID */

#if HAVE_SOCKS_RULES
                          sockscf.srule
#endif /* HAVE_SOCKS_RULES */
                               };
   size_t i;

   /*
    * Shmem for existing rules.
    */
   for (i = 0; i < ELEMENTS(rulev); ++i) {
      rule_t *_rule = rulev[i];

      while (_rule != NULL) {
         rule_t rule = *_rule;

         /*
          * sockd_shm{at,dt} will do the checking.
          */
         sockd_shmat(&rule, SHMEM_ALL);
         sockd_shmdt(&rule, SHMEM_ALL);

         _rule = _rule->next;
      }
   }

   if (pidismother(sockscf.state.pid) == 1) {
      /*
       * Shmem for old rules.
       */
      for (i = 0; i < sockscf.oldshmemc; ++i) {
         rule_t rule;

         bzero(&rule, sizeof(rule));

         switch (sockscf.oldshmemv[i].type) {
            case SHMEM_BW:
               rule.bw_shmid = sockscf.oldshmemv[i].id;
               break;

            case SHMEM_MONITOR:
               rule.mstats_shmid = sockscf.oldshmemv[i].id;
               break;

            case SHMEM_SS:
               rule.ss_shmid = sockscf.oldshmemv[i].id;
               break;

            default:
               SERRX(sockscf.oldshmemv[i].type);
         }

         (void)sockd_shmat(&rule, sockscf.oldshmemv[i].type);
         (void)sockd_shmdt(&rule, sockscf.oldshmemv[i].type);
      }
   }

   errno = errno_s;
}
#endif /* DIAGNOSTIC && !SOCKS_CLIENT */

static int
should_do_expirescan(shmem)
   const shmem_object_t *shmem;
{
   ssize_t timer;

   if ((timer = keystate_timer(shmem)) != -1
   && socks_difftime(time_monotonic(NULL),
                     shmem->keystate.lastexpirescan) >= timer)
      return 1;

   return 0;
}

static ssize_t
keystate_timer(shmem)
   const shmem_object_t *shmem;
{

   switch (shmem->type) {
      case SHMEM_SS:
         if (shmem->object.ss.throttle_perstate_isset)
            return (size_t)shmem->object.ss.throttle_perstate.limit.seconds;
         else
            return -1;

      default:
         return -1;
   }
}

static keystate_data_t *
keystate_data(keystate, index, keydata)
   const keystate_t *keystate;
   const size_t index;
   keystate_data_t *keydata;
{

   SASSERTX(index < keystate->keyc);
   switch (keystate->key) {
      case key_from:
         switch (keystate->keyv[index].data.from.safamily) {
            case AF_INET:
               keydata->type      = keytype_ipv4;
               keydata->data.ipv4 = keystate->keyv[index].data.from.addr.ipv4;
               break;

            case AF_INET6:
               keydata->type      = keytype_ipv6;
               keydata->data.ipv6 = keystate->keyv[index].data.from.addr.ipv6;
               break;

            default:
               SERRX(keystate->keyv[index].data.from.safamily);
         }

         break;


#if HAVE_SOCKS_HOSTID
      case key_hostid:
         keydata->type      = keytype_ipv4;
         keydata->data.ipv4 = keystate->keyv[index].data.hostid.ipv4;
         break;
#endif /* HAVE_SOCKS_HOSTID */

      default:
         SERRX(keystate->key);
   }

   return keydata;
}

static const char *
keydata2string(keydata, buf, buflen)
   const keystate_data_t *keydata;
   char *buf;
   size_t buflen;

{
   const char *function = "keydata2string()";
   const void *addr;
   sa_family_t safamily;

   if (buf == NULL) {
      static char _buf[MAXSOCKADDRSTRING];

      buf    = _buf;
      buflen = sizeof(_buf);
   }

   switch (keydata->type) {
      case keytype_ipv4:
         safamily = AF_INET;
         addr     = &keydata->data.ipv4;
         break;

      case keytype_ipv6:
         safamily = AF_INET6;
         addr     = &keydata->data.ipv6;
         break;

      default:
         SERRX(keydata->type);
   }

   if (inet_ntop(safamily, addr, buf, buflen) == NULL) {
      addr2hexstring(addr, safamily, buf, buflen);
      swarn("%s: inet_ntop(3) failed on safamily %s, addr %s",
           function, safamily2string(safamily), buf);
   }

   return buf;
}

static size_t
keystate_clientcount(keystate, index)
   const keystate_t *keystate;
   const size_t index;
{

   SASSERTX(index < keystate->keyc);

   switch (keystate->key) {
      case key_from:
         return keystate->keyv[index].data.from.addrc;

#if HAVE_SOCKS_HOSTID
      case key_hostid:
         return keystate->keyv[index].data.hostid.addrc;
#endif /* HAVE_SOCKS_HOSTID */

      default:
         SERRX(keystate->key);
   }

   /* NOTREACHED */
}
