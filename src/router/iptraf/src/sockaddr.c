/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"

void sockaddr_make_ipv4(struct sockaddr_storage *sockaddr,
			u_int32_t addr)
{
	if (!sockaddr)
		die("%s(): sockaddr == NULL", __FUNCTION__);

	memset(sockaddr, 0, sizeof(*sockaddr));
	struct sockaddr_in *sockaddr_in = (struct sockaddr_in *)sockaddr;
	sockaddr_in->sin_family = AF_INET;
	sockaddr_in->sin_port = 0;
	sockaddr_in->sin_addr.s_addr = addr;
}

void sockaddr_make_ipv6(struct sockaddr_storage *sockaddr,
			struct in6_addr *addr)
{
	if (!sockaddr)
		die("%s(): sockaddr == NULL", __FUNCTION__);
	if (!addr)
		die("%s(): addr == NULL", __FUNCTION__);

	memset(sockaddr, 0, sizeof(*sockaddr));
	struct sockaddr_in6 *sockaddr_in6 = (struct sockaddr_in6 *)sockaddr;
	sockaddr_in6->sin6_family = AF_INET6;
	sockaddr_in6->sin6_port = 0;
	sockaddr_in6->sin6_addr = *addr;
	sockaddr_in6->sin6_flowinfo = 0;
	sockaddr_in6->sin6_scope_id = 0;
}

in_port_t sockaddr_get_port(struct sockaddr_storage *sockaddr)
{
	if (!sockaddr)
		die("%s(): sockaddr == NULL", __FUNCTION__);

	switch (sockaddr->ss_family) {
	case AF_INET:
		return ((struct sockaddr_in *)sockaddr)->sin_port;
	case AF_INET6:
		return ((struct sockaddr_in6 *)sockaddr)->sin6_port;
	default:
		die("%s(): Unknown address family", __FUNCTION__);
	}
}

void sockaddr_set_port(struct sockaddr_storage *sockaddr, in_port_t port)
{
	if (!sockaddr)
		die("%s(): sockaddr == NULL", __FUNCTION__);

	switch (sockaddr->ss_family) {
	case AF_INET:
		((struct sockaddr_in *)sockaddr)->sin_port = port;
		break;
	case AF_INET6:
		((struct sockaddr_in6 *)sockaddr)->sin6_port = port;
		break;
	default:
		die("%s(): Unknown address family", __FUNCTION__);
	}
}

static bool _sockaddr_is_equal(struct sockaddr_storage const *addr1,
			       struct sockaddr_storage const *addr2,
			       bool check_address_only)
{
	if (!addr1)
		die("%s(): addr1 == NULL", __FUNCTION__);
	if (!addr2)
		die("%s(): addr2 == NULL", __FUNCTION__);

	if (addr1->ss_family != addr2->ss_family)
		return false;

	switch (addr1->ss_family) {
	case AF_INET: {
		struct sockaddr_in *sa1 = (struct sockaddr_in *)addr1;
		struct sockaddr_in *sa2 = (struct sockaddr_in *)addr2;

		if (sa1->sin_addr.s_addr != sa2->sin_addr.s_addr)
			return false;
		if (check_address_only)
			return true;
		if (sa1->sin_port != sa2->sin_port)
			return false;

		/* all equal */
		return true;
		}
	case AF_INET6: {
		struct sockaddr_in6 *sa1 = (struct sockaddr_in6 *)addr1;
		struct sockaddr_in6 *sa2 = (struct sockaddr_in6 *)addr2;

		if (memcmp(&sa1->sin6_addr, &sa2->sin6_addr, sizeof(sa1->sin6_addr)) != 0)
			return false;
		if (check_address_only)
			return true;
		if (sa1->sin6_port != sa2->sin6_port)
			return false;
		if (sa1->sin6_flowinfo != sa2->sin6_flowinfo)
			return false;
		if (sa1->sin6_scope_id != sa2->sin6_scope_id)
			return false;

		/* all equal */
		return true;
	       }
	default:
		die("%s(): Unknown address family", __FUNCTION__);
	}
}

bool sockaddr_is_equal(struct sockaddr_storage const *addr1,
		       struct sockaddr_storage const *addr2)
{
	return _sockaddr_is_equal(addr1, addr2, false);
}

bool sockaddr_addr_is_equal(struct sockaddr_storage const *addr1,
			    struct sockaddr_storage const *addr2)
{
	return _sockaddr_is_equal(addr1, addr2, true);
}

void sockaddr_ntop(const struct sockaddr_storage *addr, char *buf, size_t buflen)
{
	if(!addr)
		die("%s(): addr == NULL", __FUNCTION__);

	const char *ret;
	size_t minlen;

	memset(buf, 0, buflen);
	switch (addr->ss_family) {
	case AF_INET:
		minlen = INET_ADDRSTRLEN;
		ret = inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr, buf, buflen - 1);
		break;
	case AF_INET6:
		minlen = INET6_ADDRSTRLEN;
		ret = inet_ntop(AF_INET6, &((struct sockaddr_in6 *)addr)->sin6_addr, buf, buflen - 1);
		break;
	default:
		die("%s(): Unknown address family", __FUNCTION__);
	}
	if (ret == NULL) {
		switch (errno) {
		case ENOSPC:
			die("%s(): buffer too small (must be at least %zu bytes)", __FUNCTION__, minlen);
		case EAFNOSUPPORT:
			die("%s(): Unknown address family", __FUNCTION__);
		}
	}
}

void sockaddr_gethostbyaddr(const struct sockaddr_storage *addr,
			    char *buffer, size_t buflen)
{
	char hostbuf[NI_MAXHOST];

	int res = getnameinfo((struct sockaddr *)addr, sizeof(*addr),
			      hostbuf, sizeof(hostbuf),
			      NULL, 0,
			      0 /* flags */);
	if (res == 0) {
		snprintf(buffer, buflen - 1, "%s", hostbuf);
		buffer[buflen - 1] = '\0';
	} else
		sockaddr_ntop(addr, buffer, buflen);
}

void sockaddr_copy(struct sockaddr_storage *dest, struct sockaddr_storage *src)
{
	if (!src)
		die("%s(): src == NULL", __FUNCTION__);
	if (!dest)
		die("%s(): dest == NULL", __FUNCTION__);

	memcpy(dest, src, sizeof(struct sockaddr_storage));
}
