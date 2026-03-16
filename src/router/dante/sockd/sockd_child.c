/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2008, 2009,
 *               2010, 2011, 2019, 2020
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
"$Id: sockd_child.c,v 1.454.4.7.6.5.4.1 2024/11/21 10:22:43 michaels Exp $";

#define MOTHER  (0)  /* descriptor mother reads/writes on.   */
#define CHILD   (1)  /* descriptor child reads/writes on.    */

static int
setchildtype(int type, sockd_child_t ***childv, size_t **childc,
             void (**function)(void));
/*
 * Sets "childv", "childc" and "function" to the correct value depending
 * on "type".
 */

static ssize_t
findchild(pid_t pid, size_t childc, const sockd_child_t *childv);
/*
 * Finds the child with pid "pid" in the array "childv".  Searching
 * Elements in "childv" is given by "childc".
 * Returns:
 *      On success: the index of the child in "childv".
 *      On failure: -1.
 */

static sockd_child_t *
addchild(const int type);
/*
 * Adds a new child that can accept objects of type "type" from mother.
 * Returns:
 *    On success: a pointer to the added child.
 *    On failure: NULL.  (resource shortage.)
 */


static sockd_child_t *monitorv;          /* all our monitorchildren.          */
static size_t monitorc;

static sockd_child_t *negchildv;         /* all our negotiatorchildren        */
static size_t negchildc;

static sockd_child_t *reqchildv;         /* all our requestchildren           */
static size_t reqchildc;

static sockd_child_t *iochildv;          /* all our iochildren                */
static size_t iochildc;

void
enable_childcreate(void)
{
   const char *function = "enable_childcreate()";

   if (sockscf.child.noaddchild) {
      static int firsttime = 1;

      if (firsttime)
         firsttime = 0;
      else
         slog(LOG_INFO, "%s: creation of new child processes enabled again",
              function);

      sockscf.child.noaddchild = 0;
   }

   sockscf.child.noaddchild_errno = 0;
}

void
disable_childcreate(err, reason)
   const int err;
   const char *reason;
{
   const char *function = "disable_childcreate()";

   if (!sockscf.child.noaddchild) {
      slog(LOG_INFO,
           "%s: disabling creation of new child processes%s%s (%s)",
           function,
           reason == NULL ? "" : ": ",
           reason == NULL ? "" : reason,
           strerror(errno));

      sockscf.child.noaddchild = 1;
   }

   sockscf.child.noaddchild_errno  = err;
   sockscf.child.noaddchild_reason = reason;
}

size_t
childcheck(type)
   int type;
{
   const char *function = "childcheck()";
   static time_t lastaddchildfailure_time;
   const int errno_s = errno;
   sockd_child_t **childv, *idlechild;
   size_t child, *childc, minfreeslots, maxslotsperproc, proxyc,
          minclientshandled;
   time_t  minlifetime;
#if BAREFOOTD
   pid_t hasfreeudpslot = (pid_t)-1;
#endif /* BAREFOOTD */

   switch (type) {
      /*
       * Special one-child-only processes.
       */
      case -PROC_MONITOR:
      case PROC_MONITOR:
         childc            = &monitorc;
         childv            = &monitorv;
         minfreeslots      = 1;
         minclientshandled = 1;
         maxslotsperproc   = 1;
         break;


      case -PROC_NEGOTIATE:
      case PROC_NEGOTIATE:
         childc            = &negchildc;
         childv            = &negchildv;
         minfreeslots      = SOCKD_FREESLOTS_NEGOTIATE;
         minclientshandled = SOCKD_MIN_CLIENTS_HANDLED_NEGOTIATE;
         maxslotsperproc   = SOCKD_NEGOTIATEMAX;
         break;

      case -PROC_REQUEST:
      case PROC_REQUEST:
         childc            = &reqchildc;
         childv            = &reqchildv;
         minfreeslots      = SOCKD_FREESLOTS_REQUEST;
         minclientshandled = SOCKD_MIN_CLIENTS_HANDLED_REQUEST;
         maxslotsperproc   = SOCKD_REQUESTMAX;
         break;

      case -PROC_IO:
      case PROC_IO:
         childc            = &iochildc;
         childv            = &iochildv;
         minfreeslots      = SOCKD_FREESLOTS_IO;
         minclientshandled = SOCKD_MIN_CLIENTS_HANDLED_IO;
         maxslotsperproc   = SOCKD_IOMAX;
         break;

      default:
         SERRX(type);
   }

   if (sockscf.child.maxrequests != 0)
      minclientshandled = MIN(minclientshandled, sockscf.child.maxrequests);

   /* minlifetime is the same for all. */
   if (sockscf.child.maxlifetime != 0)
      minlifetime = MIN(SOCKD_MIN_LIFETIME_SECONDS, sockscf.child.maxlifetime);
   else
      minlifetime = SOCKD_MIN_LIFETIME_SECONDS;


   /*
    * get an estimate over how many (new or in total) clients our children are
    * able to accept, so we know if we need to create more children, or if we
    * can remove some.
    */
   idlechild = NULL;
   for (child = proxyc = 0; child < *childc; ++child) {
      SASSERTX((*childv)[child].freec <= maxslotsperproc);

      if ((*childv)[child].waitingforexit)
         continue;

      if (child_should_retire(&(*childv)[child])) {
         slog(LOG_DEBUG,
              "%s: not counting %s %ld.  Should be retired when "
              "possible.  Currently has %lu/%lu slots free.",
              function,
              childtype2string((*childv)[child].type),
              (long)(*childv)[child].pid,
              (unsigned long)(*childv)[child].freec,
              (unsigned long)maxfreeslots((*childv)[child].type));

         if ((*childv)[child].freec == maxfreeslots((*childv)[child].type))
            closechild((*childv)[child].pid, 1);

         continue;
      }

#if BAREFOOTD
      if (type == PROC_IO) {
         if (!(*childv)[child].hasudpsession)
            hasfreeudpslot = (*childv)[child].pid;
         else
            slog(LOG_DEBUG, "%s: not counting process %lu: no free udp slots",
                 function, (unsigned long)(*childv)[child].pid);
      }
#endif /* BAREFOOTD */

      proxyc += type < 0 ? maxslotsperproc :
#if BAREFOOTD
                           /*
                            * Don't know what the next client for this
                            * child will be (udp or tcp), so safer to assume
                            * that if it can not handle any more udp clients,
                            * it has no free slots.  Means we will get more
                            * i/o processes than might be required, but
                            * better that than too few.
                            */
                           ((type == PROC_IO
                           && (*childv)[child].hasudpsession) ?
                              0 : (*childv)[child].freec);
#else /* !BAREFOOTD */

                           (*childv)[child].freec;

#endif /* !BAREFOOTD */

      if ((*childv)[child].freec == maxslotsperproc) {
#if BAREFOOTD
         if (type == PROC_IO) {
            if (hasfreeudpslot != (pid_t)-1
            && hasfreeudpslot  != (*childv)[child].pid)
               ;
            else {
               /*
                * Want to keep this regardless, as we have not seen any
                * other child with a free udp slot so far, so don't
                * include it in the idle count where it could possibly be
                * shut down by maxidle/maxreq code.
                */
               hasfreeudpslot = (*childv)[child].pid;
               continue;
            }
         }
#endif /* BAREFOOTD */

         /*
          * all slots in this child are idle.  See later if we can remove an
          * idle child.  Shouldn't matter much which, but choose the one that
          * has handled most clients; if we are using buggy libraries, that
          * increases the chance of some leaked memory being freed too.
          *
          * Ignore children that have only handled a few clients to prevent
          * removing and then re-creating immediately after as the current
          * free slots count goes down by one when a slot is freed.
          * Should work as a cheap form of hysteresis.
          */
         if (sockscf.child.noaddchild     /* in trouble.  Remove what we can. */
         || (*childv)[child].sentc >= minclientshandled
         || socks_difftime(time_monotonic(NULL), (*childv)[child].created)
             >= (time_t)minlifetime) {
            if (idlechild == NULL || (*childv)[child].sentc > idlechild->sentc)
               idlechild = &(*childv)[child];
         }
      }
   }

   if (sockscf.child.noaddchild) {
      if (socks_difftime(time_monotonic(NULL), lastaddchildfailure_time)
      >= (time_t)MAX_ADDCHILD_SECONDS
      &&  socks_difftime(time_monotonic(NULL), sockscf.state.firstdeath_time)
          >= (time_t)MAX_ADDCHILD_SECONDS)
         enable_childcreate();
   }

   if (type >= 0) {
      /*
       * Idle child to remove?
       */
      if (idlechild != NULL
      && (proxyc - maxslotsperproc) >= minfreeslots) {
         slog(LOG_DEBUG,
              "%s: counted %lu free %s slots.  Removing pid %ld which has "
              "handled %lu client%s during %lds",
              function,
              (unsigned long)(proxyc - maxslotsperproc),
              childtype2string(idlechild->type),
              (long)(idlechild->pid),
              (unsigned long)(idlechild->sentc),
              idlechild->sentc == 1 ? "" : "s",
              (long)socks_difftime(time_monotonic(NULL), idlechild->created));

         /*
          * will remove this now, no longer part of free slots pool.
          */
         SASSERTX(idlechild->freec == maxslotsperproc);
         proxyc -= idlechild->freec;
         closechild(idlechild->pid, 1);
      }

      /*
       * Should we create an additional child?
       */
      if (!sockscf.child.noaddchild
      && ((proxyc < minfreeslots)
#if BAREFOOTD
            ||    (type == PROC_IO && hasfreeudpslot == (pid_t)-1)
#endif /* BAREFOOTD */
         )) {
         int reservedv[  MAX(FDPASS_MAX, /* max we can receive from children  */
                             1)          /* need a socket for accept(2).      */

                         + 1             /* to reopen-sockd.conf              */

                         + 1             /*
                                          * if no syslog socket is open, so
                                          * a new sockd.conf is able to open
                                          * one if necessary.
                                          */

                         + 1             /* things we don't know about.       */
                      ];

         size_t i, freec;

         /*
          * It is better to reserve some descriptors for temporary use
          * than to get errors when receiving from a child and lose clients
          * that way.  Make sure we always have some descriptors available,
          * and don't try to add a child if we don't.
          * If we can add a child after reserving the below number of
          * descriptors, things are ok.  If not, it means we have to few
          * fds available.
          */
         for (i = 0, freec = 0; i < ELEMENTS(reservedv); ++i)
            if ((reservedv[i] = makedummyfd(0, 0)) != -1)
               ++freec;

         if (freec < ELEMENTS(reservedv)) {
            swarn("%s: not enough free sockets/file descriptors to add any "
                  "new child.  Need at least %lu, but have only %lu",
                  function,
                  (unsigned long)ELEMENTS(reservedv),
                  (unsigned long)freec);

            disable_childcreate(errno,
                                "not enough free sockets/file descriptors");

            /* don't retry until a child exits, or enough time has passed. */
            time_monotonic(&lastaddchildfailure_time);
         }

         while (!sockscf.child.noaddchild
         &&       ((proxyc < minfreeslots)
#if BAREFOOTD
            ||    (type == PROC_IO && hasfreeudpslot == (pid_t)-1)
#endif /* BAREFOOTD */
         )) {
            sockd_child_t *addedchild;

#if BAREFOOTD
            if (type == PROC_IO && hasfreeudpslot == (pid_t)-1)
               slog(LOG_DEBUG, "%s: no free udp slots: need to add more %sren",
                    function, childtype2string(type));
            else
#endif /* BAREFOOTD */
            slog(LOG_DEBUG,
                  "%s: current # of free %s slots is %lu, configured minimum "
                  "is %lu: need to add more %sren",
                  function,
                  childtype2string(type),
                  (unsigned long)proxyc,
                  (unsigned long)minfreeslots,
                  childtype2string(type));

            if ((addedchild = addchild(type)) != NULL) {
               slog(LOG_DEBUG, "%s: added child, pid %lu",
                    function, (unsigned long)addedchild->pid);

               proxyc += maxslotsperproc;

#if BAREFOOTD
               if (type == PROC_IO)
                 hasfreeudpslot = addedchild->pid;
#endif /* BAREFOOTD */

            }
            else {
               log_addchild_failed();

               disable_childcreate(errno, NULL);

               /* don't retry until a child exits, or enough time has passed. */
               time_monotonic(&lastaddchildfailure_time);
            }
         }

         closev(ELEMENTS(reservedv), reservedv);
      }
   }

   /* if errno was set, it was also logged.  Don't drag it with us. */
   errno = errno_s;

   return proxyc;
}

int
fillset(set, negc, reqc, ioc)
   fd_set *set;
   size_t *negc;
   size_t *reqc;
   size_t *ioc;
{
/*   const char *function = "fillset()"; */
   size_t i;
   int dbits;

   /*
    * There is no point in setting data descriptor of child type N unless
    * child type N+1 is able to accept the data from child N.  So find
    * out if we have slots of the various types available before setting
    * the descriptor.  The same goes for the ack descriptor; we don't
    * want to think the process has a lot of free slots because we have
    * read the ack, but are unable to read the data.
    */

   *negc = childcheck(PROC_NEGOTIATE);
   *reqc = childcheck(PROC_REQUEST);
   *ioc  = childcheck(PROC_IO);

   FD_ZERO(set);
   dbits = -1;

   for (i = 0; i < sockscf.internal.addrc; ++i) {
#if BAREFOOTD
      if (sockscf.internal.addrv[i].protocol != SOCKS_TCP)
         continue; /* udp handled by io children. */
#endif /* BAREFOOTD */

      /*
       * Before we checked whether we had available negotiate slots
       * before accept(2)'ing a new client, but if we do not have
       * negotiate slots available, it will look like we have hung
       * because we are not accepting any new clients.
       * Also, if we get into the situation where we have no negotiate slots
       * and are unable to fork new negotiate processes, things are probably
       * pretty bad, so it might be better to drop new clients
       * and log a warning about it.
       */

      SASSERTX(sockscf.internal.addrv[i].s >= 0);
      FD_SET(sockscf.internal.addrv[i].s, set);
      dbits = MAX(dbits, sockscf.internal.addrv[i].s);
   }

   /* negotiator children. */
   for (i = 0; i < negchildc; ++i) {
      if (negchildv[i].waitingforexit)
         continue;

      if (*reqc > 0) {
         SASSERTX(negchildv[i].s >= 0);
         FD_SET(negchildv[i].s, set);
         dbits = MAX(dbits, negchildv[i].s);

         SASSERTX(negchildv[i].ack >= 0);
         FD_SET(negchildv[i].ack, set);
         dbits = MAX(dbits, negchildv[i].ack);
      }
   }

   /* request children. */
   for (i = 0; i < reqchildc; ++i) {
      if (reqchildv[i].waitingforexit)
         continue;

      if (*ioc > 0) {
         SASSERTX(reqchildv[i].s >= 0);
         FD_SET(reqchildv[i].s, set);
         dbits = MAX(dbits, reqchildv[i].s);

         SASSERTX(reqchildv[i].ack >= 0);
         FD_SET(reqchildv[i].ack, set);
         dbits = MAX(dbits, reqchildv[i].ack);
      }
   }

   /*
    * io children are last in chain, unless we are covenant, which may
    * need to send a client object back from  i/o child to a negotiate
    * child.
    */
   for (i = 0; i < iochildc; ++i) {
      if (iochildv[i].waitingforexit)
         continue;

#if COVENANT
      if (*negc > 0) {
         SASSERTX(iochildv[i].s >= 0);
         FD_SET(iochildv[i].s, set);
         dbits = MAX(dbits, iochildv[i].s);
#endif /* COVENANT */

         SASSERTX(iochildv[i].ack >= 0);
         FD_SET(iochildv[i].ack, set);
         dbits = MAX(dbits, iochildv[i].ack);
#if COVENANT
      }
#endif /* COVENANT */

   }

   return dbits;
}


void
clearchildtype(childtype, pipetype, nfds, set)
   const int childtype;
   whichpipe_t pipetype;
   const int nfds;
   fd_set *set;
{
   const char *function = "clearchildtype()";
   sockd_child_t **childv;
   size_t i, *childc;


   slog(LOG_DEBUG, "%s: clearing all childs of type %s from set",
        function, childtype2string(childtype));

   setchildtype(childtype, &childv, &childc, NULL);

   for (i = 0; i < *childc; ++i)
      clearset(pipetype, &(*childv)[i], set);
}

void
clearset(type, child, set)
   whichpipe_t type;
   const sockd_child_t *child;
   fd_set *set;
{
#if DEBUG
   const char *function = "clearset()";
   char buf[10240];
#endif /* DEBUG */
   int fdtoclear;

   switch (type) {
      case ACKPIPE:
         fdtoclear = child->ack;
         break;

      case DATAPIPE:
         fdtoclear = child->s;
         break;

      default:
         SERRX(type);
   }

   if (fdtoclear <= 0) {
      SASSERTX(child->waitingforexit);
      return;
   }

#if DEBUG
   if (sockscf.option.debug >= DEBUG_DEBUG)
      slog(LOG_DEBUG, "%s: will clear fd %d from the fd-set containing: %s",
           function,
           fdtoclear,
          fdset2string(sockscf.state.highestfdinuse, set, 1, buf, sizeof(buf)));
#endif

   FD_CLR(fdtoclear, set);
}

sockd_child_t *
getset(type, set)
   whichpipe_t type;
   fd_set *set;
{
/*   const char *function = "getset()"; */
   size_t i;

   /*
    * check negotiator children for match.
    */
   for (i = 0; i < negchildc; ++i) {
      if (negchildv[i].waitingforexit)
         continue;

      switch (type) {
         case DATAPIPE:
#if BAREFOOTD
            if (!ALL_UDP_BOUNCED()) { /* have some left to fake. */
               static fd_set *zero;

               if (zero == NULL) {
                  zero = allocate_maxsize_fdset();
                  FD_ZERO(zero);
               }

               if (FD_CMP(zero, set) == 0) {
                  slog(LOG_DEBUG,
                       "no fds set in set, but have not yet bounced all "
                       "udp sessions, so faking it for negchild %lu",
                       (unsigned long)negchildv[i].pid);

                  return &negchildv[i];
               }
            }
#endif /* BAREFOOTD */

            if (FD_ISSET(negchildv[i].s, set))
               return &negchildv[i];
            break;

         case ACKPIPE:
            if (FD_ISSET(negchildv[i].ack, set))
               return &negchildv[i];
            break;
      }
   }

   /*
    * check request children for match.
    */
   for (i = 0; i < reqchildc; ++i) {
      if (reqchildv[i].waitingforexit)
         continue;

      switch (type) {
         case DATAPIPE:
            if (FD_ISSET(reqchildv[i].s, set))
               return &reqchildv[i];
            break;

         case ACKPIPE:
            if (FD_ISSET(reqchildv[i].ack, set))
               return &reqchildv[i];
            break;
      }
   }

   /*
    * check io children for match.
    */
   for (i = 0; i < iochildc; ++i) {
      if (iochildv[i].waitingforexit)
         continue;

      switch (type) {
         case DATAPIPE:
            if (FD_ISSET(iochildv[i].s, set))
               return &iochildv[i];
            break;

         case ACKPIPE:
            if (FD_ISSET(iochildv[i].ack, set))
               return &iochildv[i];
            break;
      }
   }

   return NULL;
}

void
removechild(pid)
   const pid_t pid;
{
   const char *function = "removechild()";
   sockd_child_t **childv;
   ssize_t child;
   size_t *childc;

   slog(LOG_DEBUG, "%s: pid = %lu", function, (unsigned long)pid);

   if (pid == 0) { /* remove all children. */
      int childtypev[] = {PROC_MONITOR, PROC_NEGOTIATE, PROC_REQUEST, PROC_IO};
      size_t i;

      for (i = 0; i < ELEMENTS(childtypev); ++i) {
         if (childtypev[i]                 == PROC_MONITOR
         && pidismother(sockscf.state.pid) != 1)
            continue; /* only main mother controls the monitor child. */

         while (1) { /* removechild() will remove it from our child array. */
            setchildtype(childtypev[i], &childv, &childc, NULL);
            if (*childc == 0)
               break;

            SASSERTX((*childv)[0].pid != 0);
            removechild((*childv)[0].pid);
         }
      }

      return;
   }

   setchildtype(childtype(pid), &childv, &childc, NULL);
   child = findchild(pid, *childc, *childv);
   if (child < 0) {
      SWARNX(child);
      return;
   }

   /* shift all following one down */
   while ((size_t)child < *childc - 1) {
      (*childv)[child] = (*childv)[child + 1];
      ++child;
   }
   --(*childc);

   /*
    * Don't bother with realloc(3) when reducing size.
    */
}

void
closechild(pid, isnormalexit)
   pid_t pid;
   const int isnormalexit;
{
   const char *function = "closechild()";
   const char cmd = SOCKD_EXITNORMALLY;
   sockd_child_t **childv;
   ssize_t child;
   size_t *childc;

   slog(LOG_DEBUG, "%s: pid = %lu, isnormalexit = %d",
        function, (unsigned long)pid, isnormalexit);

   if (pid == 0) {
      int childtypev[] = {PROC_MONITOR, PROC_NEGOTIATE, PROC_REQUEST, PROC_IO};
      size_t childtypec;

      for (childtypec = 0; childtypec < ELEMENTS(childtypev); ++childtypec) {
         size_t i;

         if (childtypev[childtypec]        == PROC_MONITOR
         && pidismother(sockscf.state.pid) != 1)
            continue; /* only main mother controls the monitor child. */

         setchildtype(childtypev[childtypec], &childv, &childc, NULL);

         if (*childc == 0)
            continue;

         for (i = 0; i < *childc; ++i) {
            SASSERTX((*childv)[i].pid != 0);
            closechild((*childv)[i].pid, isnormalexit);
         }
      }

      return;
   }

   setchildtype(childtype(pid), &childv, &childc, NULL);
   child = findchild(pid, *childc, *childv);
   if (child < 0) {
      SWARNX(child);
      return;
   }

   if (isnormalexit && (*childv)[child].ack != -1) /* notify child. */
      if (write((*childv)[child].ack, &cmd, sizeof(cmd)) != sizeof(cmd))
         swarn("%s: failed to notify %s %ld it should exit normally",
               function,
               childtype2string((*childv)[child].type),
               (long)(*childv)[child].pid);

   if (((*childv)[child].s) != -1) {
      close((*childv)[child].s);
      (*childv)[child].s = -1;
   }

   if (((*childv)[child].ack) != -1) {
      close((*childv)[child].ack);
      (*childv)[child].ack = -1;
   }

   (*childv)[child].waitingforexit  = 1;
   (*childv)[child].exitingnormally = isnormalexit;
}

sockd_child_t *
nextchild(type, protocol)
   const int type;
   const int protocol;
{
   const char *function = "nextchild()";
   sockd_child_t **childv, *mostbusy;
   size_t i, *childc;
   int triedagain = 0;

tryagain:

   setchildtype(type, &childv, &childc, NULL);

   /*
    * Try to find the child that is most busy, so that we converge to
    * filling up slots in existing children and removing idle ones.
    */
   mostbusy = NULL;

   for (i = 0; i < *childc; ++i) {
      if ((*childv)[i].freec > 0) {
#if BAREFOOTD
         if (protocol == SOCKS_UDP && (*childv)[i].hasudpsession) {
            slog(LOG_DEBUG,
                 "%s: skipping child %ld.  Has %lu free slot%s, but also "
                 "has an udp sessions already",
                 function,
                 (long)(*childv)[i].pid,
                 (unsigned long)(*childv)[i].freec,
                 (unsigned long)(*childv)[i].freec == 1 ? "" : "s");

            continue;
         }
#endif /* BAREFOOTD */

         if (child_should_retire(&(*childv)[i]))
            continue;

         if ((*childv)[i].waitingforexit)
            continue;

         /*
          * Child has at least one free slot.
          * We want to find the child with the fewest free slots, to avoid
          * processes with only a few clients that can not be shut down by
          * child.maxidle.  Trying to fit as many clients as possible
          * into each processes allows us to reduce the process count.
          */
         if (mostbusy == NULL
         || (*childv)[i].freec < mostbusy->freec)
            mostbusy = &(*childv)[i];

         if (mostbusy->freec == 1)
            break; /* no need to look further, got the busiest we can get. */
      }
   }

   if (mostbusy != NULL)
      return mostbusy;

   slog(LOG_DEBUG, "%s: no free %s slots for protocol %s, triedagain = %d",
        function,
        childtype2string(type),
        protocol2string(protocol),
        triedagain);

   if (!triedagain) {
      slog(LOG_DEBUG, "%s: calling childcheck() and trying again", function);

      if (childcheck(type) > 0) {
         triedagain = 1;
         goto tryagain;
      }
   }

   return NULL;
}

int
childtype(pid)
   const pid_t pid;
{

   if (findchild(pid, monitorc, monitorv) >= 0)
      return PROC_MONITOR;

   if (findchild(pid, negchildc, negchildv) >= 0)
      return PROC_NEGOTIATE;

   if (findchild(pid, reqchildc, reqchildv) >= 0)
      return PROC_REQUEST;

   if (findchild(pid, iochildc, iochildv) >= 0)
      return PROC_IO;

   if (pidismother(pid))
      return PROC_MOTHER;

   return PROC_NOTOURS;
}

sockd_child_t *
getchild(pid)
   pid_t pid;
{
   ssize_t child;
   size_t *childc;
   sockd_child_t **childv;
   int type;

   switch (type = childtype(pid)) {
      case PROC_MONITOR:
      case PROC_NEGOTIATE:
      case PROC_REQUEST:
      case PROC_IO:
         break;

      case PROC_MOTHER: /* XXX should have an array for mothers also. */
         return NULL;

      case PROC_NOTOURS:
         return NULL;

      default:
         SERRX(type);
   }

   setchildtype(type, &childv, &childc, NULL);

   if ((child = findchild(pid, *childc, *childv)) >= 0)
      return &(*childv)[child];

   SERRX(pid);
   /* NOTREACHED */
}

int
send_io(s, io)
   int s;
   sockd_io_t *io;
{
   const char *function = "send_io()";
   struct iovec iov[2];
   struct msghdr msg;
   int ioc, fdtosend, length;
#if HAVE_GSSAPI
   gss_buffer_desc gssapistate = { 0, NULL };
   gss_ctx_id_t original_gssidvalue, *gssid = NULL;
   char gssapistatemem[MAXGSSAPITOKENLEN];
#endif /* HAVE_GSSAPI */
   CMSG_AALLOC(cmsg, sizeof(int) * FDPASS_MAX);

   SASSERTX(io->allocated == 0);

   log_ruleinfo_shmid(CRULE_OR_HRULE(io), function, NULL);

   if (io->srule.type == object_none) {
      SASSERTX(io->state.protocol == SOCKS_UDP);
      SASSERTX(!HAVE_SOCKS_RULES);
   }
   else
      log_ruleinfo_shmid(&io->srule, function, NULL);

   SASSERTX(!(SHMID_ISSET(CRULE_OR_HRULE(io)) && SHMID_ISSET(&io->srule)));

#if HAVE_SOCKS_RULES
   SASSERTX(!SHMID_ISSET(CRULE_OR_HRULE(io)));

#else /* !HAVE_SOCKS_RULES */
   SASSERTX(!SHMID_ISSET(&io->srule));

#endif /* !HAVE_SOCKS_RULES */

   SASSERTX(!SHMID_ISATTACHED(CRULE_OR_HRULE(io)));
   SASSERTX(!SHMID_ISATTACHED(&io->srule));

   bzero(iov, sizeof(iov));
   length = 0;
   ioc    = 0;

   iov[ioc].iov_base  = io;
   iov[ioc].iov_len   = sizeof(*io);
   length            += iov[ioc].iov_len;
   ++ioc;

   fdtosend = 0;
   CMSG_ADDOBJECT(io->src.s, cmsg, sizeof(io->src.s) * fdtosend++);

   switch (io->state.command) {
      case SOCKS_BIND:
         CMSG_ADDOBJECT(io->dst.s, cmsg, sizeof(io->dst.s) * fdtosend++);

#if HAVE_GSSAPI
         if (io->src.auth.method == AUTHMETHOD_GSSAPI)
            gssid = &io->src.auth.mdata.gssapi.state.id;
#endif /* HAVE_GSSAPI */

         if (io->state.extension.bind)
            CMSG_ADDOBJECT(io->control.s,
                           cmsg,
                           sizeof(io->control.s) * fdtosend++);
         break;

      case SOCKS_BINDREPLY:
         CMSG_ADDOBJECT(io->dst.s, cmsg, sizeof(io->dst.s) * fdtosend++);

#if HAVE_GSSAPI
         if (io->dst.auth.method == AUTHMETHOD_GSSAPI)
            gssid = &io->dst.auth.mdata.gssapi.state.id;
#endif /* HAVE_GSSAPI */

         if (io->state.extension.bind)
            CMSG_ADDOBJECT(io->control.s, cmsg,
            sizeof(io->control.s) * fdtosend++);
         break;

      case SOCKS_UDPASSOCIATE:
         SASSERTX(io->dst.s == -1);

#if HAVE_GSSAPI
         if (io->src.auth.method == AUTHMETHOD_GSSAPI)
            gssid = &io->src.auth.mdata.gssapi.state.id;
#endif /* HAVE_GSSAPI */

#if !BAREFOOTD /* no control. */
         CMSG_ADDOBJECT(io->control.s, cmsg,
                       sizeof(io->control.s) * fdtosend++);
#endif /* !BAREFOOTD */
         break;

      case SOCKS_CONNECT:
         CMSG_ADDOBJECT(io->dst.s, cmsg, sizeof(io->dst.s) * fdtosend++);

#if HAVE_GSSAPI
         if (io->src.auth.method == AUTHMETHOD_GSSAPI)
            gssid = &io->src.auth.mdata.gssapi.state.id;
#endif /* HAVE_GSSAPI */
         break;

      default:
         SERRX(io->state.command);
   }

#if HAVE_GSSAPI
   if (gssid == NULL) {
      gssapistate.value  = NULL;
      gssapistate.length = 0;
   }
   else {
      original_gssidvalue = *gssid;

      gssapistate.value  = gssapistatemem;
      gssapistate.length = sizeof(gssapistatemem);

      if (gssapi_export_state(gssid, &gssapistate) != 0)
         return -1;

      iov[ioc].iov_base = gssapistate.value;
      iov[ioc].iov_len  = gssapistate.length;
      length           += gssapistate.length;
      ++ioc;

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: gssapistate has length %lu",
              function, (long unsigned)gssapistate.length);
   }
#endif /* HAVE_GSSAPI */

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iov;
   msg.msg_iovlen  = ioc;
   msg.msg_name    = NULL;

   CMSG_SETHDR_SEND(msg, cmsg, sizeof(int) * fdtosend);

   if (sockscf.option.debug >= DEBUG_VERBOSE) {
      char ctrlbuf[MAXSOCKADDRSTRING * 2 + 64],
           srcbuf[sizeof(ctrlbuf)], dstbuf[sizeof(ctrlbuf)];

      slog(LOG_DEBUG,
           "%s: sending %d descriptors for command %d on fd %d.  "
           "bw_shmid %lu, mstats_shmid %lu, ss_shmid %lu\n"
           "Control: %d (%s)\n"
           "Src    : %d (%s)\n"
           "Dst    : %d (%s)",
           function,
           fdtosend,
           io->state.command,
           s,
           (unsigned long)io->srule.bw_shmid,
           (unsigned long)io->srule.mstats_shmid,
           (unsigned long)io->srule.ss_shmid,
           io->control.s,
           socket2string(io->control.s, ctrlbuf, sizeof(ctrlbuf)),
           io->src.s,
           socket2string(io->src.s, srcbuf, sizeof(srcbuf)),
           io->dst.s,
           io->dst.s == -1 ?
               "N/A" : socket2string(io->dst.s, dstbuf, sizeof(dstbuf)));
   }

   /*
    * if not mother, request child.  Since that child only handles one
    * client at a time, it's safe to block as long as it takes.  Mother
    * on the other hand can not block.
    */
   if (sendmsgn(s, &msg, 0, sockscf.state.type == PROC_MOTHER ? 0 : -1)
   != length) {
#if HAVE_GSSAPI
      if (gssapistate.value != NULL) {
         *gssid = original_gssidvalue;

         if (gssapi_import_state(gssid, &gssapistate) != 0)
            swarnx("%s: could not re-import gssapi state", function);
      }
#endif /* HAVE_GSSAPI */

      slog(LOG_DEBUG,
           "%s: sending client %s to %s failed: %s",
           function,
           sockaddr2string(&CONTROLIO(io)->raddr, NULL, 0),
           sockscf.state.type == PROC_MOTHER ? "child" : "mother",
           strerror(errno));

      return -1;
   }

   return 0;
}

int
send_client(s, _client, buf, buflen)
   int s;
   const sockd_client_t *_client;
   const char *buf;
   const size_t buflen;
{
   const char *function = "send_client()";
   sockd_client_t client = *_client;
   struct iovec iovec[1];
   struct msghdr msg;
   CMSG_AALLOC(cmsg, sizeof(int));
   int fdtosend;

   slog(LOG_DEBUG, "%s: buflen = %lu", function, (unsigned long)buflen);

#if COVENANT
   if (buflen > 0) {
      memcpy(client.clientdata, buf, buflen);
      client.clientdatalen = buflen;
   }
#endif /* COVENANT */

   bzero(iovec, sizeof(iovec));
   iovec[0].iov_base = &client;
   iovec[0].iov_len  = sizeof(client);

   fdtosend = 0;
   CMSG_ADDOBJECT(client.s, cmsg, sizeof(client.s) * fdtosend++);

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iovec;
   msg.msg_iovlen  = ELEMENTS(iovec);
   msg.msg_name    = NULL;

   CMSG_SETHDR_SEND(msg, cmsg, sizeof(int) * fdtosend);

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: sending fd %d (%s) on fd %d ...",
           function, client.s, socket2string(client.s, NULL, 0), s);

   if (sendmsgn(s, &msg, 0, sockscf.state.type == PROC_MOTHER ? 0 : -1)
   != (ssize_t)sizeof(client)) {
      slog(LOG_DEBUG, "%s: sending client to mother on fd %d failed: %s",
           function, s, strerror(errno));

      return -1;
   }

   return 0;
}

int
send_req(s, req)
   int s;
   sockd_request_t *req;
{
   const char *function = "send_req()";
   struct iovec iov[2];
   struct msghdr msg;
   int fdtosend, ioc, length;
#if HAVE_GSSAPI
   gss_buffer_desc gssapistate;
   char gssapistatemem[MAXGSSAPITOKENLEN];
#endif /* HAVE_GSSAPI */
   CMSG_AALLOC(cmsg, sizeof(int));

   ioc    = 0;
   length = 0;

   bzero(iov, sizeof(iov));
   iov[ioc].iov_base = req;
   iov[ioc].iov_len  = sizeof(*req);
   length           += iov[ioc].iov_len;
   ++ioc;

#if HAVE_GSSAPI
   if (req->sauth.method == AUTHMETHOD_GSSAPI) {
      gssapistate.value   = gssapistatemem;
      gssapistate.length  = sizeof(gssapistatemem);

      if (gssapi_export_state(&req->sauth.mdata.gssapi.state.id, &gssapistate)
      != 0)
         return 1;

      iov[ioc].iov_base = gssapistate.value;
      iov[ioc].iov_len  = gssapistate.length;
      length += iov[ioc].iov_len;
      ++ioc;

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: gssapistate length is %lu",
              function, (long unsigned)gssapistate.length);
   }
   else {
      gssapistate.value   = NULL;
      gssapistate.length  = 0;
   }
#endif /* HAVE_GSSAPI */

   fdtosend = 0;

   if (req->s == -1) {
#if BAREFOOTD
      SASSERT(req->req.command == SOCKS_UDPASSOCIATE);
#else /* !BAREFOOTD */
      SERRX(req->s);
#endif /* !BAREFTOOD */
   }
   else
      CMSG_ADDOBJECT(req->s, cmsg, sizeof(req->s) * fdtosend++);

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iov;
   msg.msg_iovlen  = ELEMENTS(iov);
   msg.msg_name    = NULL;

   CMSG_SETHDR_SEND(msg, cmsg, sizeof(int) * fdtosend);

   if (sockscf.option.debug >= DEBUG_VERBOSE && req->s != -1)
      slog(LOG_DEBUG, "%s: sending fd %d (%s) on fd %d ...",
           function, req->s, socket2string(req->s, NULL, 0), s);

   if (sendmsgn(s, &msg, 0, sockscf.state.type == PROC_MOTHER ? 0 : -1)
   != length) {
      if (sockscf.state.type == PROC_MOTHER)
         swarn("%s: sending client to child failed", function);
      else
         slog(sockd_motherexists() ? LOG_WARNING : LOG_DEBUG,
              "%s: sending client to mother failed: %s",
              function, strerror(errno));

#if HAVE_GSSAPI
      if (gssapistate.value != NULL) {
         /* re-import so it's still here when/if we retry sending client. */
         if (gssapi_import_state(&req->sauth.mdata.gssapi.state.id,
                                 &gssapistate) != 0)
            swarnx("%s: could not re-import gssapi state", function);
      }
#endif /* HAVE_GSSAPI */

      return -1;
   }

   return 0;
}

void
sigchildbroadcast(sig)
   int sig;
{
   const char *function = "sigchildbroadcast()";
   int childtypesv[] =  { PROC_MONITOR, PROC_NEGOTIATE, PROC_REQUEST, PROC_IO };
   size_t *childc, childtypec;
   sockd_child_t **childv;

   for (childtypec = 0; childtypec < ELEMENTS(childtypesv); ++childtypec) {
      size_t i;

      if (childtypesv[childtypec]       == PROC_MONITOR
      && pidismother(sockscf.state.pid) != 1)
         continue; /* only main mother controls the monitor child. */

      setchildtype(childtypesv[childtypec], &childv, &childc, NULL);

      for (i = 0; i < *childc; ++i) {
         slog(LOG_DEBUG, "%s: sending signal %d to %s %ld",
              function,
              sig,
              childtype2string(childtypesv[childtypec]),
              (long)(*childv)[i].pid);

         if (kill((*childv)[i].pid, sig) != 0) {
            if ((*childv)[i].waitingforexit)
               slog(LOG_DEBUG,
                    "%s: could not send signal %d to child process %ld: "
                    "child has exited already",
                    function, sig, (long)(*childv)[i].pid);
            else
               swarn("%s: could not send signal %d to child process %ld",
                     function, sig, (long)(*childv)[i].pid);
         }
      }
   }
}

size_t
maxfreeslots(childtype)
   const int childtype;
{

   switch (childtype) {
      case PROC_MONITOR:
         return 1;

      case PROC_NEGOTIATE:
         return SOCKD_NEGOTIATEMAX;

      case PROC_REQUEST:
         return SOCKD_REQUESTMAX;

      case PROC_IO:
         return SOCKD_IOMAX;

      default:
         SERRX(childtype);
   }

   return 0; /* NOTREACHED */
}

int
child_should_retire(child)
   const sockd_child_t *child;
{
   const char *function = "child_should_retire()";

   if (sockscf.child.maxrequests != 0
   &&  child->sentc >= sockscf.child.maxrequests) {
      slog(LOG_DEBUG,
           "%s: %s %ld has served %lu requests, while the max is %lu.  "
           "It should retire as soon as it has finished serving its %d "
           "remaining client%s%s",
           function,
           childtype2string(child->type),
           (long)child->pid,
           (unsigned long)child->sentc,
           (unsigned long)sockscf.child.maxrequests,
           (int)(maxfreeslots(child->type) - child->freec),
           maxfreeslots(child->type) - child->freec == 1 ? "" : "s",
           maxfreeslots(child->type) - child->freec == 0 ?
                                                         ", meaning now" : "");

      return 1;
   }

   if (sockscf.child.maxlifetime != 0) {
      time_t tnow;

      if (socks_difftime(time_monotonic(&tnow), child->created)
      >=  (time_t)sockscf.child.maxlifetime) {
         slog(LOG_DEBUG,
              "%s: %s %d has served for %ld seconds, while the max is %ld.  "
              "It should retire as soon as it has finished serving its %d "
              "remaining client%s%s",
              function,
              childtype2string(child->type),
              (int)child->pid,
              socks_difftime(tnow, child->created),
              (long)sockscf.child.maxlifetime,
              (int)(maxfreeslots(child->type) - child->freec),
              maxfreeslots(child->type) - child->freec == 1 ? "" : "s",
              maxfreeslots(child->type) - child->freec == 0 ?
                                                         ", meaning now" : "");

         return 1;
      }
   }

   return 0;
}



void
sockd_print_child_ready_message(freefds)
   const size_t freefds;
{
   const char *function = "sockd_print_child_ready_message()";

   slog(LOG_DEBUG,
        "%s: I'm %s and ready to serve with %lu free fd%s and %lu free slot%s",
        function,
        childtype2string(sockscf.state.type),
        (unsigned long)freefds, freefds == 1 ? "" : "s",
        (unsigned long)maxfreeslots(sockscf.state.type),
        maxfreeslots(sockscf.state.type) == 1 ? "" : "s");
}


static sockd_child_t *
addchild(type)
   const int type;
{
   const char *function = "addchild()";
   const char *reason;
   void (*childfunction)(void);
   sigset_t all, oldmask;
   sockd_mother_t mother;
   sockd_child_t **childv;
   pid_t pid;
   socklen_t optlen;
   size_t *childc;
   int min, rcvbuf, sndbuf, rcvbuf_set1, rcvbuf_set2, sndbuf_set1, sndbuf_set2,
       p, optname_sndbuf, optname_rcvbuf, ackpipev[2], datapipev[2];

   slog(LOG_DEBUG, "%s: type is %s", function, childtype2string(type));

   if (sockscf.state.highestfdinuse == 0)
      freedescriptors(NULL, &sockscf.state.highestfdinuse);

   /*
    * create datapipe ...
    */
   if (socketpair(AF_LOCAL, SOCK_DGRAM, 0, datapipev) != 0) {
      swarn("%s: socketpair(AF_LOCAL, SOCK_DGRAM)", function);
      return NULL;
   }

   /* ... and ackpipe. */
   if (socketpair(AF_LOCAL, SOCK_STREAM, 0, ackpipev) != 0) {
      swarn("%s: socketpair(AF_LOCAL, SOCK_STREAM)", function);

      closev(ELEMENTS(datapipev), datapipev);
      return NULL;
   }

   /*
    * Need to set the datapipe non-blocking too, even though it's a
    * DGRAM-based pipe, or we will block on read/write, because it's
    * AF_LOCAL?
    */

#if HAVE_VALGRIND_VALGRIND_H
   if (RUNNING_ON_VALGRIND)
      reason = "pipe between moter and child (probable Valgrind bug)";
   else
      reason = "pipe between moter and child";
#else /* !HAVE_VALGRIND_VALGRIND_H */
   reason = "pipe between moter and child";
#endif /* !HAVE_VALGRIND_VALGRIND_H */

   if (setnonblocking(ackpipev[0],  reason)  == -1
   ||  setnonblocking(ackpipev[1],  reason)  == -1
   ||  setnonblocking(datapipev[0], reason)  == -1
   ||  setnonblocking(datapipev[1], reason)  == -1) {
#if HAVE_VALGRIND_VALGRIND_H
         if (RUNNING_ON_VALGRIND) {
         /*
          * Valgrind seems to have some problems with programs that
          * increase the max number of open files in some cases.
          * Ref http://sourceforge.net/mailarchive/forum.php?thread_name=4F3F7A23.8020808%40gentoo.org&forum_name=valgrind-users
          */

         if (fdisopen(datapipev[0]))
            close(datapipev[0]);
         else
            swarn("%s: probable valgrind bug triggered", function);

         if (fdisopen(datapipev[1]))
            close(datapipev[1]);
         else
            slog(LOG_NOTICE, "%s: probable valgrind bug triggered", function);

         if (fdisopen(ackpipev[0]))
            close(ackpipev[0]);
         else
            slog(LOG_NOTICE, "%s: probable valgrind bug triggered", function);

         if (fdisopen(ackpipev[1]))
            close(ackpipev[1]);
         else
            slog(LOG_NOTICE, "%s: probable valgrind bug triggered", function);

         return NULL;
      }
#endif /* HAVE_VALGRIND_VALGRIND_H */

      closev(ELEMENTS(datapipev), datapipev);
      closev(ELEMENTS(ackpipev), ackpipev);

      return NULL;
   }

   /*
    * Try to set socket buffers to a optimal size depending on the size of
    * the data that passes over the pipes.  Could fine tune this further by
    * differentiating between snd/rcv-sizes for mother/child, but not
    * bothering with that at the moment.
    */
   switch (setchildtype(type, &childv, &childc, &childfunction)) {
      case PROC_MONITOR:
         /* only exit message is expected, so set to some small size. */
         sndbuf = sizeof(SOCKD_EXITNORMALLY);
         rcvbuf = sizeof(SOCKD_EXITNORMALLY);
         break;

      case PROC_NEGOTIATE:
         /*
          * A negotiator child receives a sockd_client_t struct,
          * and sends back a sockd_request_t struct.
          */
         rcvbuf = (MAX(sizeof(sockd_client_t), sizeof(sockd_request_t))
                + sizeof(struct msghdr)
                + CMSG_SPACE(sizeof(int)) * FDPASS_MAX);

         rcvbuf += SENDMSG_PADBYTES;

#if HAVE_GSSAPI
         rcvbuf += (MAX_GSS_STATE + sizeof(struct iovec));
#endif /* HAVE_GSSAPI */

         sndbuf = rcvbuf * SOCKD_NEGOTIATEMAX;
         break;

      case PROC_REQUEST:
         /*
          * A request child receives a sockd_request_t structure,
          * it sends back a sockd_io_t structure.
          */
         rcvbuf = (MAX(sizeof(sockd_request_t), sizeof(sockd_io_t))
                + sizeof(struct msghdr)
                + CMSG_SPACE(sizeof(int)) * FDPASS_MAX);

         rcvbuf += SENDMSG_PADBYTES;

#if HAVE_GSSAPI
         rcvbuf += (MAX_GSS_STATE + sizeof(struct iovec));
#endif /* HAVE_GSSAPI */

         sndbuf = rcvbuf * SOCKD_REQUESTMAX;
         break;

      case PROC_IO:
         /*
          * A io child receives a sockd_io_t structure,
          * it sends back only an ack-byte.
          * XXX that is not true in COVENANT's case.
          */
         rcvbuf = (sizeof(sockd_io_t)
                +  sizeof(struct msghdr)
                +  CMSG_SPACE(sizeof(int)) * FDPASS_MAX);

         rcvbuf += SENDMSG_PADBYTES;

#if HAVE_GSSAPI
         rcvbuf += (MAX_GSS_STATE + sizeof(struct iovec));
#endif /* HAVE_GSSAPI */

         sndbuf = rcvbuf * SOCKD_IOMAX;
         break;

      default:
         SERRX(type);
   }

   min = rcvbuf;

   if (HAVE_PIPEBUFFER_SEND_BASED)
      ; /* as expected. */
   else if (HAVE_PIPEBUFFER_RECV_BASED) {
      /*
       * reverse of our assumption that how much we can write to the pipe
       * depends on the pipe's sndbuf.  On this platform it instead depends
       * on the pipe's rcvbuf.
       */
      const size_t tmp    = sndbuf;
                   sndbuf = rcvbuf;
                   rcvbuf = tmp;
   }
   else if (HAVE_PIPEBUFFER_UNKNOWN) { /* wastes a lot of memory. */
      rcvbuf = MAX(sndbuf, rcvbuf);
      sndbuf = MAX(sndbuf, rcvbuf);
   }

#ifdef SO_RCVBUFFORCE
   if (sockscf.state.haveprivs) {
      optname_rcvbuf = SO_RCVBUFFORCE;
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);
   }
   else
#endif /* !SO_RCVBUFFORCE */
      optname_rcvbuf = SO_RCVBUF;

   p = rcvbuf;
   do {
      if (setsockopt(datapipev[MOTHER],
                     SOL_SOCKET,
                     optname_rcvbuf,
                     &p,
                     sizeof(p)) != 0
      ||  setsockopt(datapipev[CHILD],
                     SOL_SOCKET,
                     optname_rcvbuf,
                     &p,
                     sizeof(p)) != 0) {
         slog(LOG_DEBUG, "%s: could not set SO_RCVBUF to %d: %s",
              function, p, strerror(errno));

         p -= min;
      }
      else
         break;
   } while (p > min);

#ifdef SO_RCVBUFFORCE
   if (sockscf.state.haveprivs)
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);
#endif /* !SO_RCVBUFFORCE */

   optlen = sizeof(rcvbuf_set1);
   if (getsockopt(datapipev[MOTHER],
                  SOL_SOCKET,
                  SO_RCVBUF,
                  &rcvbuf_set1,
                  &optlen) != 0
   ||  getsockopt(datapipev[CHILD],
                  SOL_SOCKET,
                  SO_RCVBUF,
                  &rcvbuf_set2,
                  &optlen) != 0) {
      swarn("%s: could not get size of SO_RCVBUF", function);

      closev(ELEMENTS(datapipev), datapipev);
      closev(ELEMENTS(ackpipev), ackpipev);

      return NULL;
   }

#ifdef SO_SNDBUFFORCE
   if (sockscf.state.haveprivs) {
      optname_sndbuf = SO_SNDBUFFORCE;
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);
   }
   else
#endif /* !SO_SNDBUFFORCE */
      optname_sndbuf = SO_SNDBUF;

   p = sndbuf;
   do {
      if (setsockopt(datapipev[MOTHER],
                     SOL_SOCKET,
                     optname_sndbuf,
                     &p,
                     sizeof(p)) != 0
      ||  setsockopt(datapipev[CHILD],
                     SOL_SOCKET,
                     optname_sndbuf,
                     &p,
                     sizeof(p)) != 0) {

         slog(LOG_DEBUG, "%s: could not set SO_SNDBUF to %d: %s",
              function, p, strerror(errno));

         p -= min;
      }
      else
         break;
   } while (p > min);

#ifdef SO_SNDBUFFORCE
   if (sockscf.state.haveprivs)
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);
#endif /* !SO_SNDBUFFORCE */

   optlen = sizeof(sndbuf_set1);
   if (getsockopt(datapipev[MOTHER],
                  SOL_SOCKET,
                  SO_SNDBUF,
                  &sndbuf_set1,
                  &optlen) != 0
   ||  getsockopt(datapipev[CHILD],
                  SOL_SOCKET,
                  SO_SNDBUF,
                  &sndbuf_set2,
                  &optlen) != 0) {
      swarn("%s: could not get size of SO_SNDBUF", function);

      closev(ELEMENTS(datapipev), datapipev);
      closev(ELEMENTS(ackpipev), ackpipev);

      return NULL;
   }


   if (rcvbuf_set1 < rcvbuf || rcvbuf_set2 < rcvbuf
   ||  sndbuf_set1 < sndbuf || sndbuf_set2 < sndbuf) {
      const int isfatal = (rcvbuf_set1 < min || rcvbuf_set2 < min
                        || sndbuf_set1 < min || sndbuf_set2 < min);
      const char *value;
      int loggedalready;

      switch (type) {
         case PROC_NEGOTIATE: {
             static int local_loggedalready;

             loggedalready       = local_loggedalready;
             local_loggedalready = 1;

             value = "SOCKD_NEGOTIATEMAX";
             break;
         }

         case PROC_REQUEST: {
             static int local_loggedalready;

             loggedalready       = local_loggedalready;
             local_loggedalready = 1;

            value = "SOCKD_REQUESTMAX";
            break;
         }

         case PROC_IO: {
             static int local_loggedalready;

             loggedalready       = local_loggedalready;
             local_loggedalready = 1;

            value = "SOCKD_IOMAX";
            break;
         }

         case PROC_MONITOR: {
             /*
              * Very strange.  Our requested size should be minimal.
              */
             static int local_loggedalready;

             loggedalready       = local_loggedalready;
             local_loggedalready = 1;

            value = "N/A";
            break;
         }

         default:
            SERRX(type);
      }

      /*
       * Ideally we should shrink the size of the objects we IPC around.
       * Keep this message to remind ourselves.
       */
#if PRERELEASE
      slog(isfatal ? LOG_WARNING : (loggedalready ?  LOG_DEBUG : LOG_NOTICE),
#else /* !PRERELEASE */
      slog(isfatal ? LOG_WARNING : (loggedalready ?  LOG_DEBUG : LOG_DEBUG),
#endif /* !PRERELEASE */

           "%s: kernel did not honour requested size concerning send/receive "
           "buffers for IPC between mother and child processes.  "
           "Requested send size: %d, returned: %d and %d.  "
           "Requested receive size: %d, returned: %d and %d. "
           "This probably means %s's %s was at compile-time set to a value "
           "too large for the current kernel settings.  "
           "To avoid this warning you will need to either increase the "
           "kernel's max-size socket buffers somehow, or decrease the %s "
           "value in %s and recompile.  "
           "%s",
           function,
           sndbuf, sndbuf_set1, sndbuf_set2,
           rcvbuf, rcvbuf_set1, rcvbuf_set2,
           PRODUCT,
           value,
           value,
           PRODUCT,
           isfatal ? "" : "We will continue with the smaller buffersize, as "
                          "if our internal maxvalue was reduced, but "
                          "performance might be degraded");

      if (isfatal) {
         closev(ELEMENTS(datapipev), datapipev);
         closev(ELEMENTS(ackpipev), ackpipev);

         return NULL;
      }
   }

   /*
    * All set to create the new child.  Update highestfdinuse now.
    * Never mind that it will not be exactly the same in child and
    * mother.  The important thing is that the value saved is not lower
    * than what the value really is.
    */
   sockscf.state.highestfdinuse = MAX(sockscf.state.highestfdinuse,
                                      MAX(datapipev[0], datapipev[1]));

   sockscf.state.highestfdinuse = MAX(sockscf.state.highestfdinuse,
                                      MAX(ackpipev[0], ackpipev[1]));

   slog(LOG_DEBUG, "%s: highest fd in use at the moment: %d",
        function, sockscf.state.highestfdinuse);

   /*
    * Temporarily block signals to avoid mixing up signals to us
    * and to child created as well as any races between child receiving
    * signal and child having finished setting up signalhandlers.
    */
   (void)sigfillset(&all);
   if (sigprocmask(SIG_SETMASK, &all, &oldmask) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK)", function);

   switch ((pid = fork())) {
      case -1:
         if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
            swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);

         swarn("%s: fork()", function);

         closev(ELEMENTS(datapipev), datapipev);
         closev(ELEMENTS(ackpipev), ackpipev);

         return NULL;

      case 0: {
         struct sigaction sigact;
         size_t i, maxfd;

#if HAVE_PROFILING && HAVE_MONCONTROL /* XXX is this only needed on Linux? */
         moncontrol(1);
#endif /* HAVE_PROFILING && HAVE_MONCONTROL */

         bzero(&sigact, sizeof(sigact));

         /*
          * signals mother has set up but which we need ignore at this
          * point, lest we accidentally run mothers signal handler if the
          * child does not install it's own signal handler for the
          * particular signal.
          * Later on, the child sets up its own signal handlers.
          */
#if HAVE_BSDAUTH
         /*
          * For BSD authentication on OpenBSD, libc forks sub process,
          * so SIG_DFL is needed for waitpid() to work.
          */
         sigact.sa_handler = SIG_DFL;
#else /* !HAVE_BSDAUTH */
         sigact.sa_handler = SIG_IGN;
#endif /* !HAVE_BSDAUTH */

         if (sigaction(SIGCHLD, &sigact, NULL) != 0)
            swarn("%s: sigaction(SIGCHLD)", function);

         sigact.sa_handler = SIG_IGN;
	
#if HAVE_SIGNAL_SIGINFO
         if (sigaction(SIGINFO, &sigact, NULL) != 0)
            swarn("%s: sigaction(SIGINFO)", function);
#endif /* HAVE_SIGNAL_SIGINFO */

         if (sigaction(SIGUSR1, &sigact, NULL) != 0)
            swarn("%s: sigaction(SIGUSR1)", function);

         /*
          * Next install a SIGHUP handler.  Same for all children and
          * different from the one mother uses.
          */
         sigact.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_SIGINFO;

         sigact.sa_sigaction = sighup_child;
         if (sigaction(SIGHUP, &sigact, NULL) != 0)
            serr("%s: could not install signalhandler for SIGHUP", function);

         mother.s           = datapipev[CHILD];
         mother.ack         = ackpipev[CHILD];
         sockscf.state.type = type;

         newprocinit();
         postconfigloadinit(); /* may be something special for this process. */

#if 0
         slog(LOG_DEBUG, "sleeping...");
         sleep(20);
#endif

         /*
          * It would be nice to be able to lose all root privileges here
          * but unfortunately we can't.  All children may need it for
          * re-opening logfiles or sockd.conf on SIGHUP.  In addition:
          *
          * monitor children:
          *      - Nothing known.
          *
          * negotiation children:
          *      - could need privileges to check password.
          *
          * request children:
          *      - could need privileges to bind port.
          *      - could need privileges to check password.
          *
          * io children:
          *      - could need privileges to create a raw socket to listen
          *        for icmp errors.
          *      - could need privileges to bind port if using the
          *        redirect() module.
          *
          * Also may need it to write to the logfiles.
          * If we have privilege-support, give up what we can though.
          */

         switch (type) {
            case PROC_MONITOR:
               break;

            case PROC_NEGOTIATE:
#if HAVE_LIBWRAP
#if SOCKD_NEGOTIATEMAX > 1
               resident = 1;
#endif /* SOCKD_NEGOTIATEMAX > 1 */
#endif /* HAVE_LIBWRAP */

#if HAVE_PRIVILEGES
               if (sockscf.state.haveprivs) {
                  /* don't need this privilege so permanently loose it. */
                  priv_delset(sockscf.privileges.privileged, PRIV_NET_PRIVADDR);

                  if (setppriv(PRIV_SET,
                               PRIV_PERMITTED,
                               sockscf.privileges.privileged) != 0)
                     swarn("%s: setppriv() failed to relinquish "
                           "PRIV_NET_PRIVADDR",
                           function);
               }
#endif /* HAVE_PRIVILEGES */

               break;

            case PROC_REQUEST:
#if HAVE_LIBWRAP
#if SOCKD_REQUESTMAX > 1
               resident = 1;
#endif /* SOCKD_REQUESTMAX > 1 */
#endif /* HAVE_LIBWRAP */
               break;

            case PROC_IO:
#if HAVE_LIBWRAP
#if SOCKD_IOMAX > 1
               resident = 1;
#endif /* SOCKD_IOMAX > 1 */
#endif /* HAVE_LIBWRAP */
               break;

            default:
               SERRX(type);
         }

         /*
          * delete all fd's  we got from parent, with a few exceptions.
          */

         maxfd = (size_t)sockscf.state.highestfdinuse;
         SASSERTX(maxfd != 0);

         for (i = 0; i <= maxfd; ++i) {
            /*
             * exceptions.
             */

            if (i == (size_t)mother.s || i == (size_t)mother.ack)
               continue;

            if (descriptorisreserved((int)i))
               continue;

            (void)close((int)i);
         }

         /*
          * Needs to be called again after closing, since if using syslog we
          * don't know what descriptor that uses, so it will have been closed
          * in the above close(2) loop.
          *
          * This must happen as the first thing after the above loop as
          * newprocinit() will close the old syslog descriptor, if any,
          * before opening a new one.  If we have started to use the
          * descriptor for something else already (e.g. due to dup(2)),
          * newprocinit(), will still close the old descriptor, even
          * though it's no longer a syslog descriptor.
          */
         newprocinit();

         /*
          * This is minor optimization to make things faster for select(2)
          * by avoiding having two increasingly high-numbered descriptors
          * to check for, with most of the other descriptors in the lower-end.
          */

          datapipev[0] = mother.s;
          datapipev[1] = mother.ack;

          if ((mother.s   = dup(mother.s))   == -1
          ||  (mother.ack = dup(mother.ack)) == -1)
            serr("%s: failed to dup(2) pipe to mother", function);

          close(datapipev[0]);
          close(datapipev[1]);

         if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
            swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);

         /*
          * Ok, all set for this process.
          */

         slog(LOG_DEBUG, "%s: I am a new %s with data-pipe %d and ack-pipe %d",
              function, childtype2string(type), mother.s, mother.ack);

         sockscf.state.mother = mother;
         time_monotonic(&sockscf.stat.boot);

         errno = 0;
         childfunction();
         /* NOTREACHED */
         break;
      }

      default: {
         sockd_child_t *p;

         if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
            swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);

         if ((p = realloc(*childv, sizeof(**childv) * (*childc + 1))) == NULL) {
            swarn("%s: %s", function, NOMEM);

            closev(ELEMENTS(datapipev), datapipev);
            closev(ELEMENTS(ackpipev), ackpipev);

            return NULL;
         }
         *childv = p;

         bzero(&((*childv)[*childc]), sizeof((*childv)[*childc]));

         time_monotonic(&(*childv)[*childc].created);
         (*childv)[*childc].type           = type;
         (*childv)[*childc].pid            = pid;
         (*childv)[*childc].s              = datapipev[MOTHER];
         (*childv)[*childc].ack            = ackpipev[MOTHER];

         close(datapipev[CHILD]);
         close(ackpipev[CHILD]);

         slog(LOG_DEBUG,
              "%s: created new %s with pid %ld, data-pipe %d and ack-pipe %d.  "
              "Minimum rcvbuf: %d, set: %d and %d.  "
              "Minimum sndbuf: %d, set: %d and %d",
              function,
              childtype2string((*childv)[*childc].type),
              (long)(*childv)[*childc].pid,
              (*childv)[*childc].s,
              (*childv)[*childc].ack,
              rcvbuf, rcvbuf_set1, rcvbuf_set2,
              sndbuf, sndbuf_set1, sndbuf_set2);

         switch ((*childv)[*childc].type) {
            case PROC_MONITOR:
               /*
                * Doesn't really have slots, so just set 1.  Will remain
                * at one for the entire lifespans, as there are no slots.
                */
               (*childv)[*childc].freec = 1;
               break;

            case PROC_NEGOTIATE:
               (*childv)[*childc].freec = SOCKD_NEGOTIATEMAX;
               break;

            case PROC_REQUEST:
               (*childv)[*childc].freec = SOCKD_REQUESTMAX;
               break;

            case PROC_IO:
               (*childv)[*childc].freec = SOCKD_IOMAX;
               break;

            default:
               SERRX((*childv)[*childc].type);
         }
      }
   }

   return &(*childv)[(*childc)++];
}


static int
setchildtype(type, childv, childc, function)
   int type;
   sockd_child_t ***childv;
   size_t **childc;
   void (**function)(void);
{
   size_t *c;
   sockd_child_t **v;
   void (*f)(void);

   switch (type) {
      case PROC_MONITOR:
         c = &monitorc;
         v = &monitorv;
         f = &run_monitor;
         break;

      case PROC_NEGOTIATE:
         c = &negchildc;
         v = &negchildv;
         f = &run_negotiate;
         break;

      case PROC_IO:
         c = &iochildc;
         v = &iochildv;
         f = &run_io;
         break;

      case PROC_REQUEST:
         c = &reqchildc;
         v = &reqchildv;
         f = &run_request;
         break;

      default:
         SERRX(type);
   }

   if (childv != NULL)
      *childv = v;

   if (childc != NULL)
      *childc = c;

   if (function != NULL)
      *function = f;

   SASSERTX(c != NULL);
   SASSERTX(v != NULL);
   SASSERTX(f != NULL);

   return type;
}

static ssize_t
findchild(pid, childc, childv)
   pid_t pid;
   size_t childc;
   const sockd_child_t *childv;
{
   static size_t i;

   if (i < childc && childv[i].pid == pid)
      return i;

   for (i = 0; i < childc; ++i)
      if (childv[i].pid == pid)
         return i;

   return -1;
}


/* ARGSUSED */
void
sighup_child(sig, si, sc)
   int sig;
   siginfo_t *si;
   void *sc;
{
   const char *function = "sighup_child()";
   const int errno_s = errno;
   struct config *shmemconfig, config;
   ptrdiff_t offset;
#if DIAGNOSTIC
   size_t i;
#endif /* DIAGNOSTIC */

   SIGNAL_PROLOGUE(sig, si, errno_s);

   /*
    * we are not main mother.  Can we assume the signal is from mother?
    * If not, ignore it.
    * Some systems don't set si_pid correctly, so check if the value is set
    * first.
    */
   if (si->si_pid != 0  /* some systems don't fill this in. */
   && !pidismother(si->si_pid)) {
      swarnx("%s: received SIGHUP from pid %ld, but only expecting it from "
             "mother (pid %ld).  Ignored",
             function, (long)si->si_pid, (long)getppid());

      return;
   }

   if (sockscf.shmeminfo->configsize == 0) {
      swarnx("%s: Strange.  Received a SIGHUP signal, but the size of "
             "mother's shmemconfig is zero, which does not make sense.  "
             "Ignoring it",
             function);
      return;
   }

   slog(LOG_INFO, "%s: received SIGHUP: reloading config", function);

   socks_lock(sockscf.shmemconfigfd, 0, 0, 0, 1);

#if 1
   shmemconfig = sockd_mmap(NULL,
                            sockscf.shmeminfo->configsize,
                            /*
                             * Need PROT_WRITE also as we may need to adjust
                             * the pointer addresses before accessing.
                             */
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE,
                            sockscf.shmemconfigfd,
                            0);

#else /* just for testing */
   shmemconfig = mmap((void *)((uintptr_t)sockscf.shmeminfo->config
                                          + sockscf.state.pagesize),
                      sockscf.shmeminfo->configsize,
                      /*
                       * Need PROT_WRITE also as we update
                       * the pointer addresses as we go.
                       */
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_FIXED,
                      sockscf.shmemconfigfd,
                      0);
#endif /* just for testing. */

   if (shmemconfig  == MAP_FAILED) {
      swarn("%s: could not mmap(2) %lu bytes of memory for config reload",
            function, (unsigned long)sockscf.shmeminfo->configsize);

      socks_unlock(sockscf.shmemconfigfd, 0, 0);
      return;
   }

   offset = (uintptr_t)shmemconfig - (uintptr_t)sockscf.shmeminfo->config;

   slog(LOG_DEBUG,
        "%s: mmap(2)-ed %lu bytes of memory at offset %p.  "
        "Mothers offset is at %p, %ld bytes away",
        function,
        (unsigned long)sockscf.shmeminfo->configsize,
        shmemconfig,
        sockscf.shmeminfo->config,
        (long)offset);

   /*
    * shallow copy what we can.
    */

   config = *shmemconfig;

   if (pointer_copy(shmemconfig,  offset, &config, NULL, 0) != 0) {
      swarnx("%s: could not update pointers in config-object after reload",
             function);

      if (munmap(shmemconfig, sockscf.shmeminfo->configsize) != 0)
         swarn("%s: munmap(%p, %lu) failed",
               function,
               shmemconfig,
               (unsigned long)sockscf.shmeminfo->configsize);

      socks_unlock(sockscf.shmemconfigfd, 0, 0);
      return;
   }


#if DIAGNOSTIC
   slog(LOG_DEBUG, "%s: updated config, doing compare test ...", function);
   i = compareconfigs(shmemconfig, &config);
#endif /* DIAGNOSTIC */

   if (munmap(shmemconfig, sockscf.shmeminfo->configsize) != 0)
      swarn("%s: munmap(%p, %lu) failed",
            function,
            shmemconfig,
            (unsigned long)sockscf.shmeminfo->configsize);

#if DIAGNOSTIC
   if (i == 0) {
      swarnx("%s: created config not identical to config in shmem", function);
      return;
   }
   else
      slog(LOG_DEBUG,
           "%s: created config identical to shmem config.  %lu bytes compared",
           function, (unsigned long)i);
#endif /* DIAGNOSTIC */

   socks_unlock(sockscf.shmemconfigfd, 0, 0);

   SASSERTX(!pidismainmother(sockscf.state.pid));

   SASSERTX(config.state.type == sockscf.state.type);

   resetconfig(&sockscf, 0); /* reset old before loading new. */

   SASSERTX(config.state.type == sockscf.state.type);

   /*
    * needs to be handled specially.
    */

   sockd_freelogobject(&sockscf.errlog, 1);
   sockd_reopenlogfiles(&config.errlog, 0);

   sockd_freelogobject(&sockscf.log, 1);
   sockd_reopenlogfiles(&config.log, 0);

   sockscf = config;

#if DIAGNOSTIC
   /*
    * compareconfig() will dereference pointers.  If there's something left
    * pointing to the shmem by mistake, things will hopefully crash now.
    */
   SASSERTX(compareconfigs(&sockscf, &config) != 0);
#endif /* DIAGNOSTIC */

   postconfigloadinit();

   if (pidismother(sockscf.state.pid))
      sigchildbroadcast(sig); /* each mother process has its own children. */

   resetprivileges();

   time_monotonic(&sockscf.stat.configload);

   SIGNAL_EPILOGUE(sig, si, errno_s);
}
