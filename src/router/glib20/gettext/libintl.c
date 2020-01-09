/*
 * libintl.c - libintl definitions
 *
 * This file is part of the gettext-dummy library (the "Library").
 *
 * The Library is written by Aleksey Demakov and placed in the
 * public domain 2008-01-31. The Library is provided "as is",
 * with NO WARRANTY, either expressed or implied.
 */

char *
gettext(char *msg)
{
	return msg;
}

char *
dgettext(const char *domain, char *msg)
{
	return msg;
}

char *
dcgettext(const char *domain, char *msg, int category)
{
	return msg;
}

char *
ngettext(char *msg, char *msg_plural, unsigned long int n)
{
	return n == 1 ? msg : msg_plural;
}

char *
dngettext(const char *domain, char *msg, char *msg_plural, unsigned long int n)
{
	return n == 1 ? msg : msg_plural;
}

char *
dcngettext(const char *domain, char *msg, char *msg_plural, unsigned long int n, int category)
{
	return n == 1 ? msg : msg_plural;
}

char *
textdomain(const char *domain)
{
	return "dummy";
}

char *
bindtextdomain(const char *domain, const char *dir)
{
	return "/usr/share";
}

char *
bind_textdomain_codeset(const char *domain, const char *codeset)
{
	return (char*)codeset;
}
