/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014, 2024
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
"$Id: privileges.c,v 1.64.4.2.14.2 2024/11/20 22:05:39 karls Exp $";

static privilege_t lastprivelege = SOCKD_PRIV_NOTSET;

int
sockd_initprivs(void)
{
   const char *function = "sockd_initprivs()";

#if HAVE_SOLARIS_PRIVS
   char *privstr;
   priv_set_t *privset;
   const char *extra_privs[] = {
      PRIV_FILE_DAC_READ,    /* password file, and pam? */
      PRIV_FILE_DAC_SEARCH,  /* password file, and pam? */
      PRIV_FILE_DAC_WRITE,   /* writing pidfile.        */
      PRIV_NET_PRIVADDR,     /*
                              * binding ports < 1024 on behalf of the client,
                              * if so configured.
                              */
      PRIV_PROC_LOCK_MEMORY, /* shmem; want it paged in as locks are used.    */
#if HAVE_UDP_SUPPORT
      PRIV_NET_ICMPACCESS,   /*
                              * sending/receiving icmp errors related to sent
                              * udp packets.
                              */
#endif /* HAVE_UDP_SUPPORT */
   };
   size_t i;

   if ((sockscf.privileges.privileged   = priv_allocset()) == NULL
   ||  (sockscf.privileges.unprivileged = priv_allocset()) == NULL) {
      swarn("%s: priv_allocset()", function);
      return -1;
   }

   if ((privset = priv_str_to_set ("basic", ",", NULL)) == NULL) {
      swarn("%s: priv_str_to_set failed", function);
      return -1;
   }

   /*
    * First add/remove what we need from the basic set and save it as the
    * unprivileged set. The unprivileged set is also the set used by libwrap.
    */

   /* add ... Nothing.  */

#if 0
   /* ... and remove. */

   /*
    * removing this would mean libwraps exec statement won't work, but
    * probably nobody uses that from sockd anyway.  Could it be needed
    * by pam, though?  Leave it in for now.
    */
   if (priv_delset(privset, PRIV_PROC_EXEC) != 0) {
      swarn("%s: cannot remove %s privilege", function, PRIV_PROC_EXEC);
      return -1;
   }
#endif

   priv_copyset(privset, sockscf.privileges.unprivileged);

   /*
    * Then add the extra privileges we need.
    */

   for (i = 0; i < ELEMENTS(extra_privs); ++i)
      if (priv_addset(privset, extra_privs[i]) != 0) {
         swarn("%s: cannot add %s privilege", function, extra_privs[i]);
         return -1;
      }
      else
         slog(LOG_DEBUG, "%s: added privilege %s to the privileged set",
         function, extra_privs[i]);

   /*
    * any privileges we may ever need.
    */

   priv_copyset(privset, sockscf.privileges.privileged);
   priv_freeset(privset);

   if ((privstr = priv_set_to_str(sockscf.privileges.privileged,
                                  ',',
                                  PRIV_STR_LIT)) == NULL)
      swarn("%s: priv_set_to_str(sockscf.privileges.privileged) failed",
            function);

   if (setppriv(PRIV_SET, PRIV_PERMITTED, sockscf.privileges.privileged) == -1){
      swarn("%s: cannot set the PRIV_PERMITTED privilege set (%s)",
            function, privstr == NULL ? "" : privstr);

      free(privstr);
      return -1;
   }

   slog(LOG_DEBUG, "%s: using the following privileges for PRIV_PERMITTED: %s",
        function,  privstr == NULL ? "<error>" : privstr);

   free(privstr);

   /*
    * unprivileged is what we'll be running with normally.
    */

   if ((privstr = priv_set_to_str(sockscf.privileges.unprivileged,
                                  ',',
                                  PRIV_STR_LIT)) == NULL)
      swarn("%s: priv_set_to_str(sockscf.privileges.unprivileged) failed",
            function);

   if (setppriv(PRIV_SET, PRIV_EFFECTIVE, sockscf.privileges.unprivileged)
   == -1) {
      swarn("%s: cannot set the PRIV_EFFECTIVE privilege set (%s)",
            function, privstr == NULL ? "" : privstr);

      free(privstr);
      return -1;
   }

   /*
    * Same for inherited.  Only applies to libwrap's exec statement, and
    * PAM?
    */
   if (setppriv(PRIV_SET, PRIV_INHERITABLE, sockscf.privileges.unprivileged)
   == -1) {
      swarn("%s: cannot set PRIV_INHERITABLE privilege set (%s)",
            function, privstr == NULL ? "" : privstr);

      free(privstr);
      return -1;
   }

   slog(LOG_DEBUG, "%s: using the following privileges for PRIV_EFFECTIVE "
                   "and PRIV_INHERITABLE: %s",
                  function,  privstr == NULL ? "<error>" : privstr);

   free(privstr);

   setreuid(getuid(), getuid());
   setregid(getgid(), getgid());

   slog(LOG_DEBUG, "%s: privileges relinquished successfully", function);

   /* should be able to use special privileges. */
   sockscf.state.haveprivs = 1;

#else /* !HAVE_SOLARIS_PRIVS */

   if (geteuid() == 0)
      /* should be able to use special privileges. */
      sockscf.state.haveprivs = 1;

   if (setegid(sockscf.uid.unprivileged_gid) != 0) {
      swarn("%s: setegid(2) to unprivileged gid %lu failed",
            function, (unsigned long)sockscf.uid.unprivileged_gid);

      sockscf.state.haveprivs = 0;
      return -1;
   }
   sockscf.state.egid = sockscf.uid.unprivileged_gid;

   if (seteuid(sockscf.uid.unprivileged_uid) != 0) {
      swarn("%s: seteuid(2) to unprivileged uid %lu failed",
           function, (unsigned long)sockscf.uid.unprivileged_uid);

      sockscf.state.haveprivs = 0;
      return -1;
   }
   sockscf.state.euid = sockscf.uid.unprivileged_uid;

   slog(LOG_DEBUG, "%s: will use euid/egid %lu/%lu normally",
        function,
        (unsigned long)sockscf.uid.unprivileged_uid,
        (unsigned long)sockscf.uid.unprivileged_gid);

#endif /* !HAVE_SOLARIS_PRIVS */

   return 0;
}

void
sockd_priv(privilege, op)
   const privilege_t privilege;
   const priv_op_t op;
{
   const char *function = "sockd_priv()";
#if HAVE_PRIVILEGES
   static priv_set_t *lastprivset;

#else /* !HAVE_PRIVILEGES */
   static uid_t lasteuid;
   static gid_t lastegid;
   int p;

#endif /* !HAVE_PRIVILEGES */

   if (!sockscf.state.haveprivs)
      return;

   slog(LOG_DEBUG, "%s: switching privilege %d %s",
        function, (int)privilege, privop2string(op));

#define FULLSETS                          \
         SOCKD_PRIV_LIBWRAP:              \
         case SOCKD_PRIV_PRIVILEGED:      \
         case SOCKD_PRIV_UNPRIVILEGED:    \
         case SOCKD_PRIV_PAM:             \
         case SOCKD_PRIV_BSDAUTH

#if HAVE_PRIVILEGES
   if (lastprivset == NULL)
      if ((lastprivset = priv_allocset()) == NULL) {
          serr("%s: priv_allocset()", function);
      }
#endif /* HAVE_PRIVILEGES */

   /*
    * these asserts are only valid as long as we never turn more than
    * one privilege on/off at a time.  If that ever changes, we need
    * to remove these asserts, but until then, they are useful.
    */
   if (op == PRIV_ON) {
      SASSERTX(lastprivelege == SOCKD_PRIV_NOTSET);
      lastprivelege = privilege;

#if HAVE_PRIVILEGES
      switch (privilege) {
         case FULLSETS:
            /*
             * needs to be handled special as it's not a single privilege
             * we turn on/off, but a full set we PRIV_SET.
             */
            if (getppriv(PRIV_EFFECTIVE, lastprivset) != 0) {
               swarn("%s: very strange ...  getppriv(PRIV_EFFECTIVE) failed.  "
                     "This might not work out too well ...",
                     function);
               SWARN(errno);
            }
            break;

         default:
            break;
      }
#endif /* HAVE_PRIVILEGES */
   }
   else {
      SASSERTX(op == PRIV_OFF);
      SASSERTX(lastprivelege == privilege);
      lastprivelege = SOCKD_PRIV_NOTSET;
   }

   switch (privilege) {
      case FULLSETS: {
#if HAVE_PRIVILEGES
         priv_set_t *privtoset;

#else /* !HAVE_PRIVILEGES */
         uid_t neweuid;
         gid_t newegid;

         if (op == PRIV_ON) {
            lasteuid = sockscf.state.euid;
            lastegid = sockscf.state.egid;
         }

#endif /* HAVE_PRIVILEGES */

         if (op == PRIV_ON) {
            switch (privilege) {
               case SOCKD_PRIV_PRIVILEGED:
               case SOCKD_PRIV_PAM:
               case SOCKD_PRIV_BSDAUTH:
#if HAVE_PRIVILEGES

                  privtoset = sockscf.privileges.privileged;

#else /* !HAVE_PRIVILEGES */

                  neweuid  = sockscf.uid.privileged_uid;
                  newegid  = sockscf.uid.privileged_gid;
#endif /* HAVE_PRIVILEGES */

                  break;

               case SOCKD_PRIV_UNPRIVILEGED:
#if HAVE_PRIVILEGES

                  privtoset = sockscf.privileges.unprivileged;

#else /* !HAVE_PRIVILEGES */

                  neweuid  = sockscf.uid.unprivileged_uid;
                  newegid  = sockscf.uid.unprivileged_gid;
#endif /* HAVE_PRIVILEGES */

                  break;

               case SOCKD_PRIV_LIBWRAP:
#if HAVE_PRIVILEGES

                  privtoset = sockscf.privileges.unprivileged;/* same for now */

#else /* !HAVE_PRIVILEGES */

                  neweuid  = sockscf.uid.libwrap_uid;
                  newegid  = sockscf.uid.libwrap_gid;
#endif /* HAVE_PRIVILEGES */

                  break;

               default:
                  SERRX(privilege);
            }
         }
         else {
#if HAVE_PRIVILEGES

            privtoset = lastprivset;

#else /* !HAVE_PRIVILEGES */

            neweuid  = lasteuid;
            newegid  = lastegid;
#endif /* HAVE_PRIVILEGES */
         }

#if HAVE_PRIVILEGES
         if (setppriv(PRIV_SET, PRIV_EFFECTIVE, privtoset) != 0)
            serr("%s: switching privilege level %d %s failed",
                 function, (int)privilege, privop2string(op));

#else /* !HAVE_PRIVILEGES */
         if (sockd_seteugid(neweuid, newegid) != 0)
            serr("%s: switching to euid/egid %u/%u failed",
                 function,
                 op == PRIV_ON ? (unsigned )neweuid : (unsigned )lasteuid,
                 op == PRIV_ON ? (unsigned )newegid : (unsigned )lastegid);
#endif /* HAVE_PRIVILEGES */

         break;
      }

      case SOCKD_PRIV_FILE_READ:
      case SOCKD_PRIV_GSSAPI:
#if HAVE_PRIVILEGES
         if (priv_set(op, PRIV_EFFECTIVE, PRIV_FILE_DAC_SEARCH, NULL) != 0)
            serr("%s: switching PRIV_FILE_DAC_SEARCH %s failed",
                 function, privop2string(op));

         if (priv_set(op, PRIV_EFFECTIVE, PRIV_FILE_DAC_READ, NULL) != 0)
            serr("%s: switching PRIV_FILE_DAC_READ %s failed",
                 function, privop2string(op));

#else /* !HAVE_PRIVILEGES */
         if (op == PRIV_ON)
            p = sockd_seteugid(sockscf.uid.privileged_uid,
                               sockscf.uid.privileged_gid);
         else
            p = sockd_seteugid(lasteuid, lastegid);

         if (p != 0)
            serr("%s: switching to euid/egid %u/%u failed",
                 function,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_uid : (unsigned)lasteuid,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_gid : (unsigned)lastegid);
#endif /* !HAVE_PRIVILEGES */

         break;

      case SOCKD_PRIV_FILE_WRITE:
#if HAVE_PRIVILEGES
         if (priv_set(op, PRIV_EFFECTIVE, PRIV_FILE_DAC_SEARCH, NULL) != 0)
            serr("%s: switching PRIV_FILE_DAC_SEARCH %s failed",
                 function, privop2string(op));

         if (priv_set(op, PRIV_EFFECTIVE, PRIV_FILE_DAC_READ, NULL) != 0)
            serr("%s: switching PRIV_FILE_DAC_READ %s failed",
                 function, privop2string(op));

         if (priv_set(op, PRIV_EFFECTIVE, PRIV_FILE_DAC_WRITE, NULL) != 0)
            serr("%s: switching PRIV_FILE_DAC_WRITE %s failed",
                 function, privop2string(op));

#else /* !HAVE_PRIVILEGES */
         if (op == PRIV_ON)
            p = sockd_seteugid(sockscf.uid.privileged_uid,
                               sockscf.uid.privileged_gid);
         else
            p = sockd_seteugid(lasteuid, lastegid);

         if (p != 0)
            serr("%s: switching to euid/egid %u/%u failed",
                 function,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_uid : (unsigned)lasteuid,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_gid : (unsigned)lastegid);
#endif /* !HAVE_PRIVILEGES */

         break;

      case SOCKD_PRIV_NET_ADDR:
#if HAVE_PRIVILEGES
         if (priv_set(op, PRIV_EFFECTIVE, PRIV_NET_PRIVADDR, NULL) != 0)
            serr("%s: switching PRIV_NET_PRIVADDR %s failed",
                 function, privop2string(op));

#else /* !HAVE_PRIVILEGES */
         if (op == PRIV_ON)
            p = sockd_seteugid(sockscf.uid.privileged_uid,
                               sockscf.uid.privileged_uid);
         else
            p = sockd_seteugid(lasteuid, lastegid);

         if (p != 0)
            serr("%s: switching to euid/egid %u/%u failed",
                 function,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_uid : (unsigned)lasteuid,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_gid : (unsigned)lastegid);
#endif /* !HAVE_PRIVILEGES */

         break;

      case SOCKD_PRIV_NET_ICMPACCESS:
#if HAVE_PRIVILEGES
         if (priv_set(op, PRIV_EFFECTIVE, PRIV_NET_ICMPACCESS, NULL) != 0)
            serr("%s: switching PRIV_NET_ICMPACCESS %s failed",
                 function, privop2string(op));

#else /* !HAVE_PRIVILEGES */
         if (op == PRIV_ON)
            p = sockd_seteugid(sockscf.uid.privileged_uid,
                               sockscf.uid.privileged_uid);
         else
            p = sockd_seteugid(lasteuid, lastegid);

         if (p != 0)
            serr("%s: switching to euid/egid %u/%u failed",
                 function,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_uid : (unsigned)lasteuid,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_gid : (unsigned)lastegid);
#endif /* !HAVE_PRIVILEGES */

         break;

      case SOCKD_PRIV_NET_ROUTESOCKET:
#if HAVE_PRIVILEGES
         /* nothing special required on Solaris apparently. */

#else /* !HAVE_PRIVILEGES */
         if (op == PRIV_ON)
            p = sockd_seteugid(sockscf.uid.privileged_uid,
                               sockscf.uid.privileged_gid);
         else
            p = sockd_seteugid(lasteuid, lastegid);

         if (p != 0)
            serr("%s: switching to euid/egid %u/%u failed",
                 function,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_uid : (unsigned)lasteuid,
                 op == PRIV_ON ?
                     (unsigned)sockscf.uid.privileged_gid : (unsigned)lastegid);
#endif /* !HAVE_PRIVILEGES */

         break;

      default:
         SERRX(privilege);
   }
}

void
resetprivileges(void)
{
   const char *function = "resetprivileges()";

   slog(LOG_DEBUG, "%s: euid/egid %ld/%ld",
        function, (long)geteuid(), (long)getegid());

#if !HAVE_PRIVILEGES

   if (sockscf.uid.privileged_uid == sockscf.uid.unprivileged_uid
   &&  sockscf.uid.privileged_uid == sockscf.uid.libwrap_uid
   &&  sockscf.uid.privileged_uid != geteuid()) {
      slog(LOG_DEBUG,
           "%s: no alternate userids configured for use. Will use uid %lu "
           "in all contexts and permanently drop all others",
           function, (unsigned long)sockscf.uid.unprivileged_uid);

      (void)seteuid(0);

      /* drop all other groups than the one we will setgid(2) to. */
      if (setgroups(0, NULL) != 0)
         swarn("%s: failed to drop all groups via setgroups(0, NULL)",
               function);

      if (setgid(sockscf.uid.unprivileged_gid) != 0) {
         if (getegid() != sockscf.uid.unprivileged_gid
         ||  getgid()  != sockscf.uid.unprivileged_gid)
            serr("setgid(2) to unprivileged gid %lu failed",
                  (unsigned long)sockscf.uid.unprivileged_gid);
      }

      if (setuid(sockscf.uid.unprivileged_uid) != 0) {
         if (geteuid() != sockscf.uid.unprivileged_uid
         ||  getuid()  != sockscf.uid.unprivileged_uid)
            serr("setuid(2) to unprivileged uid %lu failed",
                 (unsigned long)sockscf.uid.unprivileged_uid);
      }

      sockscf.state.egid = sockscf.uid.unprivileged_gid;
      sockscf.state.euid = sockscf.uid.unprivileged_uid;

      sockscf.state.haveprivs = 0; /* don't have it anymore. */
   }
#endif /* !HAVE_PRIVILEGES */
}


#if !HAVE_PRIVILEGES
int
sockd_seteugid(uid, gid)
   const uid_t uid;
   const gid_t gid;
{
   const char *function = "sockd_setugid()";

   if (sockscf.state.inited && !sockscf.state.haveprivs)
      return -1;

   slog(LOG_DEBUG, "%s: old uid/gid: %lu/%lu, new: %lu/%lu",
        function,
        (unsigned long)sockscf.state.euid,
        (unsigned long)sockscf.state.egid,
        (unsigned long)uid,
        (unsigned long)gid);

#if DIAGNOSTIC
   SASSERTX(geteuid() == sockscf.state.euid);
   SASSERTX(getegid() == sockscf.state.egid);
#endif /* DIAGNOSTIC */

   if (sockscf.state.euid == uid
   &&  sockscf.state.egid == gid)
      return 0;

   if (sockscf.state.euid != 0) {
      /* revert back to original (presumably 0) euid before changing. */
      if (seteuid(sockscf.initial.euid) != 0) {
         swarn("%s: failed revering to original euid %lu",
               function, (unsigned long)sockscf.initial.euid);

         return -1;
      }
   }

   /* first the groupid ... */
   if (setegid(gid) != 0) {
      swarn("%s: setegid(2) to gid %lu from euid/egid %lu/%lu failed",
            function,
            (unsigned long)gid,
            (unsigned long)sockscf.state.euid,
            (unsigned long)sockscf.state.egid);

      return -1;
   }

   sockscf.state.egid = gid;

   /* ... and then the uid. */
   if (seteuid(uid) != 0) {
      swarn("%s: seteuid(2) to uid %lu from euid/egid %lu/%lu failed",
            function,
            (unsigned long)uid,
            (unsigned long)sockscf.state.euid,
            (unsigned long)sockscf.state.egid);

      return -1;
   }

   sockscf.state.euid = uid;

   return 0;
}

#endif /* !HAVE_PRIVILEGES */
