#ifndef IPTRAF_NG_SOCKADDR_H
#define IPTRAF_NG_SOCKADDR_H

void sockaddr_make_ipv4(struct sockaddr_storage *sockaddr,
			u_int32_t addr);
void sockaddr_make_ipv6(struct sockaddr_storage *sockaddr,
			struct in6_addr *addr);
in_port_t sockaddr_get_port(struct sockaddr_storage *sockaddr);
void sockaddr_set_port(struct sockaddr_storage *sockaddr, in_port_t port);
bool sockaddr_is_equal(struct sockaddr_storage const *addr1,
		       struct sockaddr_storage const *addr2);
bool sockaddr_addr_is_equal(struct sockaddr_storage const *addr1,
			    struct sockaddr_storage const *addr2);
void sockaddr_ntop(const struct sockaddr_storage *addr, char *buf, size_t buflen);
void sockaddr_gethostbyaddr(const struct sockaddr_storage *addr,
			    char *buffer, size_t buflen);
void sockaddr_copy(struct sockaddr_storage *dest, struct sockaddr_storage *src);

#endif	/* IPTRAF_NG_SOCKADDR_H */
