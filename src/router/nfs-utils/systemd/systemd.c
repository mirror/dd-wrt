/*
 * Helper functions for systemd generators in nfs-utils.
 *
 * Currently just systemd_escape().
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "systemd.h"

static const char hex[16] =
{
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

/*
 * determine length of the string that systemd_escape() needs to allocate
 */
static int systemd_len(char *path)
{
	char *p;
	int len = 0;

	p = path;
	while (*p == '/')
		/* multiple leading "/" are ignored */
		p++;

	if (!*p)
		/* root directory "/" becomes is encoded as a single "-" */
		return 1;

	if (*p == '.')
		/*
		 * replace "." with "\x2d" escape sequence if
		 * it's the first character in escaped path
		 * */
		len += 4;

	while (*p) {
		unsigned char c = *p++;

		if (c == '/') {
			/* multiple non-trailing slashes become '-' */
			while (*p == '/')
				p++;
			if (*p)
				len++;
		} else if (isalnum(c) || c == ':' || c == '.' || c == '_')
			/* these characters are not replaced */
			len++;
		else
			/* replace with "\x2d" escape sequence */
			len += 4;
	}

	return len;
}

/*
 * convert c to "\x2d" escape sequence and append to string
 * at position p, advancing p
 */
static char *hexify(unsigned char c, char *p)
{
	*p++ = '\\';
	*p++ = 'x';
	*p++ = hex[c >> 4];
	*p++ = hex[c & 0xf];
	return p;
}

/*
 * convert a path to a unit name according to the logic in systemd.unit(5):
 *
 *     Basically, given a path, "/" is replaced by "-", and all other
 *     characters which are not ASCII alphanumerics are replaced by C-style
 *     "\x2d" escapes (except that "_" is never replaced and "." is only
 *     replaced when it would be the first character in the escaped path).
 *     The root directory "/" is encoded as single dash, while otherwise the
 *     initial and ending "/" are removed from all paths during
 *     transformation.
 *
 * NB: Although the systemd.unit(5) doesn't mention it, the ':' character
 * is not escaped.
 */
char *systemd_escape(char *path, char *suffix)
{
	char *result;
	char *p;
	int len;

	len = systemd_len(path);
	result = malloc(len + strlen(suffix) + 1);
	p = result;
	while (*path == '/')
		/* multiple leading "/" are ignored */
		path++;
	if (!*path) {
		/* root directory "/" becomes is encoded as a single "-" */
		*p++ = '-';
		goto out;
	}
	if (*path == '.')
		/*
		 * replace "." with "\x2d" escape sequence if
		 * it's the first character in escaped path
		 * */
		p = hexify(*path++, p);

	while (*path) {
		unsigned char c = *path++;

		if (c == '/') {
			/* multiple non-trailing slashes become '-' */
			while (*path == '/')
				path++;
			if (*path)
				*p++ = '-';
		} else if (isalnum(c) || c == ':' || c == '.' || c == '_')
			/* these characters are not replaced */
			*p++ = c;
		else
			/* replace with "\x2d" escape sequence */
			p = hexify(c, p);
	}

out:
	sprintf(p, "%s", suffix);
	return result;
}
