/*
 * Copyright (c) 2009, 2010, 2011, 2012, 2019
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
  * This code was contributed by
  * Markus Moeller (markus_moeller at compuserve.com).
  */


#include "common.h"

static const char rcsid[] =
"$Id: krb5.c,v 1.6.2.4 2020/11/11 17:02:26 karls Exp $";

#if HAVE_KRB5

#if HAVE_KRB5_MEMORY_KEYTAB
krb5_kt_list ktlist = NULL;
/*
 *  Free a kt_list
 */
krb5_error_code krb5_free_kt_list(context, list)
   krb5_context context;
   krb5_kt_list list;
{
   const char *function = "krb5_free_kt_list()";
   krb5_kt_list lp, prev;
   krb5_error_code retval = 0;

   for (lp = list; lp;) {
      retval = krb5_kt_free_entry(context, lp->entry);
      free(lp->entry);
      if (retval) {
         swarnx("%s: error freeing keytab entry: %s",
                function, error_message(retval));
         break;
      }
      prev = lp;
      lp = lp->next;
      free(prev);
   }
   return retval;
}
/*
 * Read in a keytab and append it to list.  If list starts as NULL,
 * allocate a new one if necessary.
 */
krb5_error_code krb5_read_keytab(context, name, list)
   krb5_context context;
   char *name;
   krb5_kt_list *list;
{
   const char *function = "krb5_read_keytab()";
   krb5_kt_list lp = NULL, tail = NULL, back = NULL;
   krb5_keytab kt;
   krb5_keytab_entry *entry;
   krb5_kt_cursor cursor;
   krb5_error_code retval = 0;

   if (*list) {
      /* point lp at the tail of the list */
      for (lp = *list; lp->next; lp = lp->next);
      back = lp;
   }
   retval = krb5_kt_resolve(context, name, &kt);
   if (retval) {
      swarnx("%s: error resolving keytab: %s",
             function, error_message(retval));
      return retval;
   }
   retval = krb5_kt_start_seq_get(context, kt, &cursor);
   if (retval) {
      swarnx("%s: error starting keytab sequence: %s",
             function, error_message(retval));
      goto close_kt;
   }
   for (;;) {
      entry = (krb5_keytab_entry *)malloc(sizeof (krb5_keytab_entry));
      if (!entry) {
         swarnx("%s: error allocating keytab entry: %s",
                function, error_message(ENOMEM));
         retval = ENOMEM;
         break;
      }
      memset(entry, 0, sizeof (*entry));
      retval = krb5_kt_next_entry(context, kt, entry, &cursor);
      if (retval) {
         if (retval != KRB5_KT_END)
            swarnx("%s: error getting next keytab entry: %s",
                   function, error_message(retval));
         break;
      }

      if (!lp) {              /* if list is empty, start one */
         lp = (krb5_kt_list)malloc(sizeof (*lp));
         if (!lp) {
            swarnx("%s: error allocating keytab list: %s",
                   function, error_message(ENOMEM));
            retval = ENOMEM;
            break;
         }
      } else {
         lp->next = (krb5_kt_list)malloc(sizeof (*lp));
         if (!lp->next) {
            swarnx("%s: error allocating keytab list: %s",
                   function, error_message(ENOMEM));
            retval = ENOMEM;
            break;
         }
         lp = lp->next;
      }
      if (!tail)
         tail = lp;
      lp->next = NULL;
      lp->entry = entry;
   }
   if (entry)
      free(entry);
   if (retval) {
      if (retval == KRB5_KT_END)
         retval = 0;
      else {
         krb5_free_kt_list(context, tail);
         tail = NULL;
         if (back)
            back->next = NULL;
      }
   }
   if (!*list)
      *list = tail;
   krb5_kt_end_seq_get(context, kt, &cursor);
close_kt:
   krb5_kt_close(context, kt);
   return retval;
}

/*
 * Takes a kt_list and writes it to the named keytab.
 */
krb5_error_code krb5_write_keytab(context, list, name)
   krb5_context context;
   krb5_kt_list list;
   char *name;
{
   const char *function = "krb5_write_keytab()";
   krb5_kt_list lp;
   krb5_keytab kt;
   char ktname[PATH_MAX+sizeof("MEMORY:")+1];
   krb5_error_code retval = 0;

   snprintf(ktname, sizeof(ktname), "%s", name);
   retval = krb5_kt_resolve(context, ktname, &kt);
   if (retval) {
      swarnx("%s: error resolving keytab: %s",
             function, error_message(retval));
      return retval;
   }
   for (lp = list; lp; lp = lp->next) {
      retval = krb5_kt_add_entry(context, kt, lp->entry);
      if (retval) {
         swarnx("%s: error resolving keytab: %s",
                function, error_message(retval));
         break;
      }
   }
   /*
    * krb5_kt_close(context, kt);
    */
   return retval;
}
#endif /* HAVE_KRB5_MEMORY_KEYTAB */

#if HAVE_PAC
int
krb5_err_isset(context, buf, buflen, code)
   krb5_context context;
   char *buf;
   size_t buflen;
   krb5_error_code code;
{
   const char *errmsg;
   size_t w;

   if (code) {
       errmsg = krb5_get_error_message(context, code);
       w = snprintf(buf, buflen, "%.*s.  ",
                   (int)strlen(errmsg), (const char *)errmsg);
       buf    += w;
       buflen -= w;
       krb5_free_error_message(context, errmsg);
   }

   return code;
}
#endif /* HAVE_PAC */

#endif /* HAVE_KRB5 */
