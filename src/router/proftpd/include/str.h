/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2008-2013 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* String manipulation functions
 * $Id: str.h,v 1.11 2013/11/24 00:45:29 castaglia Exp $
 */

#ifndef PR_STR_H
#define PR_STR_H

/* Default maximum number of replacements that will do in a given string. */
#define PR_STR_MAX_REPLACEMENTS                 8

char *sstrcat(char *, const char *, size_t);
char *sreplace(pool *, char *, ...);

char *pdircat(pool *, ...);
char *pstrcat(pool *, ...);
char *pstrdup(pool *, const char *);
char *pstrndup(pool *, const char *, size_t);

/* Returns TRUE if the string `s' ends with given `suffix', FALSE if the string
 * does not end with the given suffix, and -1 if there was an error (errno
 * will be set appropriately).
 *
 * The `flags' value consisted of OR'ing the following:
 *
 *  PR_STR_FL_IGNORE_CASE
 *    Request a case-insensitive comparison
 */
int pr_strnrstr(const char *s, size_t slen, const char *suffix,
  size_t suffixlen, int flags);

/* Newer version of sreplace(), with more control and better error reporting. */
char *pr_str_replace(pool *, unsigned int, char *, ...);
char *pr_str_strip(pool *, char *);
char *pr_str_strip_end(char *, char *);
int pr_str_get_nbytes(const char *, const char *, off_t *);
char *pr_str_get_word(char **, int);

/* Parses a "time string" into its duration, in seconds.  Returns the number
 * of seconds obtained via the `duration' pointer, or -1 (with errno) if
 * there was a problem parsing the provided string.
 *
 * A "time string" is formatted as "hh:mm:ss".
 */
int pr_str_get_duration(const char *str, int *duration);

#define PR_STR_FL_PRESERVE_COMMENTS		0x0001
#define PR_STR_FL_PRESERVE_WHITESPACE		0x0002
#define PR_STR_FL_IGNORE_CASE			0x0004

char *pr_str_get_token(char **, char *);
char *pr_str_get_token2(char **, char *, size_t *);

/* Returns TRUE if the given string is "on", "yes", "true", or "1"; returns
 * FALSE if the string is "off", "false", "no", or "0".  Otherwise, -1
 * is returned, with errno set to EINVAL.
 */
int pr_str_is_boolean(const char *);

int pr_str_is_fnmatch(const char *);

#define CHOP(s)		pr_str_strip_end((s), "\r\n")

#endif /* PR_STR_H */
