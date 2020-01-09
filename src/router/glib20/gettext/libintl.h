/*
 * libintl.h - libintl declarations
 *
 * This file is part of the gettext-dummy library (the "Library").
 *
 * The Library is written by Aleksey Demakov and placed in the
 * public domain 2008-01-31. The Library is provided "as is",
 * with NO WARRANTY, either expressed or implied.
 */

#ifndef _LIBINTL_H
#define _LIBINTL_H

#define gettext(msg) (msg)
#define dgettext(domain,msg) (msg)
#define dcgettext(domain,msg,category) (msg)

#define ngettext(msg,pmsg,n) ((n == 1) ? (msg) : (pmsg))
#define dngettext(domain,msg,pmsg,n) ((n == 1) ? (msg) : (pmsg))
#define dcngettext(domain,msg,pmsg,n,category) ((n == 1) ? (msg) : (pmsg))

char *textdomain(const char *domain);
char *bindtextdomain(const char *domain, const char *dir);
char *bind_textdomain_codeset(const char *domain, const char *codeset);

#endif
