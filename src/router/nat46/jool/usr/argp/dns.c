#include "usr/argp/dns.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include "usr/argp/log.h"

void print_addr6(struct ipv6_transport_addr const *addr6, bool numeric,
		char const *separator, __u8 l4_proto)
{
	char hostname[NI_MAXHOST], service[NI_MAXSERV];
	char hostaddr[INET6_ADDRSTRLEN];
	struct sockaddr_in6 sa6;
	int err;

	if (numeric)
		goto print_numeric;

	memset(&sa6, 0, sizeof(struct sockaddr_in6));
	sa6.sin6_family = AF_INET6;
	sa6.sin6_port = htons(addr6->l4);
	sa6.sin6_addr = addr6->l3;

	err = getnameinfo((const struct sockaddr*) &sa6, sizeof(sa6),
			hostname, sizeof(hostname), service, sizeof(service), 0);
	if (err != 0) {
		pr_err("getnameinfo failed: %s", gai_strerror(err));
		goto print_numeric;
	}

	/*
	 * Verification because ICMP doesn't use numeric ports, so it makes no
	 * sense to have a translation of the "ICMP id".
	 */
	if (l4_proto != L4PROTO_ICMP)
		printf("%s%s%s", hostname, separator, service);
	else
		printf("%s%s%u", hostname, separator, addr6->l4);
	return;

print_numeric:
	inet_ntop(AF_INET6, &addr6->l3, hostaddr, sizeof(hostaddr));
	printf("%s%s%u", hostaddr, separator, addr6->l4);
}

void print_addr4(struct ipv4_transport_addr const *addr4, bool numeric,
		char const *separator, __u8 l4_proto)
{
	char hostname[NI_MAXHOST], service[NI_MAXSERV];
	char *hostaddr;
	struct sockaddr_in sa;
	int err;

	if (numeric)
		goto print_numeric;

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(addr4->l4);
	sa.sin_addr = addr4->l3;

	err = getnameinfo((const struct sockaddr*) &sa, sizeof(sa),
			hostname, sizeof(hostname), service, sizeof(service), 0);
	if (err != 0) {
		pr_err("getnameinfo failed: %s", gai_strerror(err));
		goto print_numeric;
	}

	/*
	 * Verification because ICMP doesn't use numeric ports, so it makes no
	 * sense to have a translation of the "ICMP id".
	 */
	if (l4_proto != L4PROTO_ICMP)
		printf("%s%s%s", hostname, separator, service);
	else
		printf("%s%s%u", hostname, separator, addr4->l4);
	return;

print_numeric:
	hostaddr = inet_ntoa(addr4->l3);
	printf("%s%s%u", hostaddr, separator, addr4->l4);
}
