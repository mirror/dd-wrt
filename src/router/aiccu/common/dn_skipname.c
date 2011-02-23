#include <errno.h>
#include <resolv.h>

/* Ripped from glibc 2.4 sources. */

/*
 * ns_name_skip(ptrptr, eom)
 *      Advance *ptrptr to skip over the compressed name it points at.
 * return:
 *      0 on success, -1 (with errno set) on failure.
 */
int ns_name_skip(const u_char **ptrptr, const u_char *eom)
{
	const u_char *cp;
	u_int n;

	cp = *ptrptr;
	while (cp < eom && (n = *cp++) != 0)
	{
		/* Check for indirection. */
		switch (n & NS_CMPRSFLGS) {
		case 0:                 /* normal case, n == len */
			cp += n;
			continue;
		case NS_CMPRSFLGS:      /* indirection */
			cp++;
			break;
		default:                /* illegal type */
			errno = EMSGSIZE;
			return (-1);
		}
		break;
	}
	if (cp > eom)
	{
		errno = EMSGSIZE;
		return (-1);
	}
	*ptrptr = cp;
	return (0);
}

int dn_skipname(const u_char *ptr, const u_char *eom)
{
	const u_char *saveptr = ptr;

	if(ns_name_skip(&ptr, eom) == -1)
		return (-1);
	return (ptr - saveptr);
}

