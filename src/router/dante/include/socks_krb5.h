/*
 * Copyright (c) 2014, 2019
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

#if HAVE_KRB5

int
krb5_err_isset(krb5_context context, char *buf, size_t buflen,
               krb5_error_code code);

#if HAVE_KRB5_MEMORY_KEYTAB
typedef struct _krb5_kt_list {
   struct _krb5_kt_list *next;
   krb5_keytab_entry *entry;
} *krb5_kt_list;

krb5_error_code
krb5_free_kt_list(krb5_context context, krb5_kt_list list);
/*
 *  Free a kt_list
 */

krb5_error_code
krb5_read_keytab(krb5_context context, char *name, krb5_kt_list *list);
/*
 * Read in a keytab and append it to list.  If list starts as NULL,
 * allocate a new one if necessary.
 */

krb5_error_code
krb5_write_keytab(krb5_context context, krb5_kt_list list, char *name);
/*
 * Takes a kt_list and writes it to the named keytab.
 */
#endif /* HAVE_KRB5_MEMORY_KEYTAB */

#if !HAVE_COM_ERR_IN_KRB5
#if HAVE_COM_ERR_H
#include <com_err.h>
#elif HAVE_ET_COM_ERR_H
#include <et/com_err.h>
#endif /* HAVE_ET_COM_ERR_H */
#endif /* !HAVE_COM_ERR_IN_KRB5 */

#if !HAVE_ERROR_MESSAGE && HAVE_KRB5_GET_ERR_TEXT
#define error_message(code) krb5_get_err_text(kparam.context, code)
#elif !HAVE_ERROR_MESSAGE && HAVE_KRB5_GET_ERROR_MESSAGE
#define error_message(code) krb5_get_error_message(kparam.context, code)
#elif !HAVE_ERROR_MESSAGE
   static char err_code[17];
   const char *KRB5_CALLCONV
   error_message(long code) {
      snprintf(err_code, 16, "%ld", code);
      return err_code;
   }
#endif /* !HAVE_ERROR_MESSAGE */

#if HAVE_PAC
int
get_sids(unsigned char *sids, krb5_context k5_context, krb5_pac pac,
             negotiate_state_t *state);
#endif /* HAVE_PAC */

#endif /* HAVE_KRB5 */
