/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2005, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2017
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

#if HAVE_SHADOW_H && HAVE_GETSPNAM
#include <shadow.h>
#endif /* HAVE_SHADOW_H && HAVE_GETSPNAM */

static const char rcsid[] =
"$Id: auth_password.c,v 1.41.6.2 2017/01/31 08:17:38 karls Exp $";

static const char *
sockd_getpasswordhash(const char *login, char *pw, const size_t pwsize,
                      char *emsg, const size_t emsglen);
/*
 * Fetches the password hash for the username "login".
 * The returned hash is stored in "pw", which is of size "pwsize".
 *
 * Returns the password hash on success, or NULL on failure.  On failure,
 * emsg, which must be of size emsglen, contains the reason for the error.
 */

int
passwordcheck(name, cleartextpw, emsg, emsglen)
   const char *name;
   const char *cleartextpw;
   char *emsg;
   size_t emsglen;
{
   const char *function = "passwordcheck()";
   const char *p;
   char visstring[MAXNAMELEN * 4], pwhash[MAXPWLEN],  *crypted;
   int rc;

   slog(LOG_DEBUG, "%s: name = %s, password = %s",
        function,
         str2vis(name,
                 strlen(name),
                 visstring,
                 sizeof(visstring)),
        cleartextpw == NULL ? "<empty>" : "<cleartextpw>");

   if (cleartextpw == NULL) {
      /*
       * No password to check.  I.e. the authmethod used does not care
       * about passwords, only whether the user exists or not. E.g.
       * rfc931/ident.
       */
      if (getpwnam(name) == NULL) {
         snprintf(emsg, emsglen, "no user \"%s\" found in system password file",
                  str2vis(name,
                          strlen(name),
                          visstring,
                          sizeof(visstring)));
         return -1;
      }
      else
         /*
          * User is in the passwordfile, and that is all we care about.
          */
         return 0;
   }

   /*
    * Else: the authmethod used requires us to match the password also.
    */

   /* usually need privileges to look up the password. */
   sockd_priv(SOCKD_PRIV_FILE_READ, PRIV_ON);
   p = sockd_getpasswordhash(name,
                             pwhash,
                             sizeof(pwhash),
                             emsg,
                             emsglen);
   sockd_priv(SOCKD_PRIV_FILE_READ, PRIV_OFF);

   if (p == NULL)
      return -1;

   /*
    * Have the passwordhash for the user.  Does it match the provided password?
    */

   crypted = crypt(cleartextpw, pwhash);

   if (crypted == NULL) { /* strange. */
      snprintf(emsg, emsglen,
               "system password crypt(3) failed for user \"%s\": %s",
               str2vis(name,
                       strlen(name),
                       visstring,
                       sizeof(visstring)),
               strerror(errno));

      swarnx("%s: Strange.  This should not happen: %s", function, emsg);
      rc = -1;
   }
   else {
      if (strcmp(crypted, pwhash) == 0)
         rc = 0;
      else {
         snprintf(emsg, emsglen,
                  "system password authentication failed for user \"%s\"",
                  str2vis(name,
                          strlen(name),
                          visstring,
                          sizeof(visstring)));
         rc = -1;
      }
   }

   bzero(pwhash, sizeof(pwhash));
   return rc;
}

static const char *
sockd_getpasswordhash(login, pw, pwsize, emsg, emsglen)
   const char *login;
   char *pw;
   const size_t pwsize;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_getencrypedpassword()";
   const char *pw_db = NULL;
   const int errno_s = errno;
   char visstring[MAXNAMELEN * 4];

#if HAVE_GETSPNAM /* sysv stuff. */
   struct spwd *spwd;

   if ((spwd = getspnam(login)) != NULL)
      pw_db = spwd->sp_pwdp;

#elif HAVE_GETPRPWNAM /* some other broken stuff. */
   /*
    * don't know how this looks and don't know anybody using it.
    */

#error "getprpwnam() not supported yet.  Please contact Inferno Nettverk A/S "
       "if you would like to see support for it."

#elif HAVE_GETPWNAM_SHADOW /* OpenBSD 5.9 and later */

   struct passwd *pwd;

   if ((pwd = getpwnam_shadow(login)) != NULL)
      pw_db = pwd->pw_passwd;

#else /* normal BSD stuff. */
   struct passwd *pwd;

   if ((pwd = getpwnam(login)) != NULL)
      pw_db = pwd->pw_passwd;
#endif /* normal BSD stuff. */

   if (pw_db == NULL) {
      snprintf(emsg, emsglen,
               "could not access user \"%s\"'s records in the system "
               "password file: %s",
               str2vis(login, strlen(login), visstring, sizeof(visstring)),
               strerror(errno));

      return NULL;
   }

   if (strlen(pw_db) + 1 /* NUL */ > pwsize) {
      snprintf(emsg, emsglen,
               "%s: password set for user \"%s\" in the system password file "
               "is too long.  The maximal supported length is %lu, but the "
               "length of the password is %lu characters",
               function,
               str2vis(login,
                      strlen(login),
                      visstring,
                      sizeof(visstring)),
               (unsigned long)(pwsize - 1),
               (unsigned long)strlen(pw_db));

      swarnx("%s: %s", function, emsg);
      return NULL;
   }

   strcpy(pw, pw_db);

   /*
    * some systems can set errno even on success. :-/
    * E.g. OpenBSD 4.4. seems to do this.  Looks like it tries
    * /etc/spwd.db first, and if that fails, /etc/pwd.db, but it
    * forgets to reset errno.
    */
   errno = errno_s;

   return pw;
}
