/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <nft.h>

#include "nftutils.h"

#include <netdb.h>

/* Buffer size used for getprotobynumber_r() and similar. The manual comments
 * that a buffer of 1024 should be sufficient "for most applications"(??), so
 * let's double it.  It still fits reasonably on the stack, so no need to
 * choose a smaller one. */
#define NETDB_BUFSIZE 2048

bool nft_getprotobynumber(int proto, char *out_name, size_t name_len)
{
	const struct protoent *result;

#if HAVE_DECL_GETPROTOBYNUMBER_R
	struct protoent result_buf;
	char buf[NETDB_BUFSIZE];
	int r;

	r = getprotobynumber_r(proto,
	                       &result_buf,
	                       buf,
	                       sizeof(buf),
	                       (struct protoent **) &result);
	if (r != 0 || result != &result_buf)
		result = NULL;
#else
	result = getprotobynumber(proto);
#endif

	if (!result)
		return false;

	if (strlen(result->p_name) >= name_len)
		return false;
	strcpy(out_name, result->p_name);
	return true;
}

int nft_getprotobyname(const char *name)
{
	const struct protoent *result;

#if HAVE_DECL_GETPROTOBYNAME_R
	struct protoent result_buf;
	char buf[NETDB_BUFSIZE];
	int r;

	r = getprotobyname_r(name,
	                     &result_buf,
	                     buf,
	                     sizeof(buf),
	                     (struct protoent **) &result);
	if (r != 0 || result != &result_buf)
		result = NULL;
#else
	result = getprotobyname(name);
#endif

	if (!result)
		return -1;

	if (result->p_proto < 0 || result->p_proto > UINT8_MAX)
		return -1;
	return (uint8_t) result->p_proto;
}

bool nft_getservbyport(int port, const char *proto, char *out_name, size_t name_len)
{
	const struct servent *result;

#if HAVE_DECL_GETSERVBYPORT_R
	struct servent result_buf;
	char buf[NETDB_BUFSIZE];
	int r;

	r = getservbyport_r(port,
	                    proto,
	                    &result_buf,
	                    buf,
	                    sizeof(buf),
	                    (struct servent**) &result);
	if (r != 0 || result != &result_buf)
		result = NULL;
#else
	result = getservbyport(port, proto);
#endif

	if (!result)
		return false;

	if (strlen(result->s_name) >= name_len)
		return false;
	strcpy(out_name, result->s_name);
	return true;
}
