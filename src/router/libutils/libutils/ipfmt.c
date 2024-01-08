#include <stdio.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

/* inet_netaddr_of - returns network address
 *
 * addr     : Address as 32 bit value
 * msk      : Netmask as 32 bit value 
 */
struct in_addr inet_netaddr_of(struct in_addr addr, struct in_addr msk)
{
	struct in_addr netaddr;

	netaddr.s_addr = addr.s_addr & msk.s_addr;
	return netaddr;
}

/* inet_bcastaddr_of - returns broadcast address
 *
 * addr     : Address as 32 bit value
 * msk      : Netmask as 32 bit value 
 */
struct in_addr inet_bcastaddr_of(struct in_addr net, struct in_addr msk)
{
	struct in_addr bcast;

	bcast.s_addr = net.s_addr | ~msk.s_addr;
	return bcast;
}

/*
 * net_addr_to_cidr - build cidr string from 32bit address and netmask
 *
 * addr     : Address as 32 bit value
 * msk      : Netmask as 32 bit value 
 * cidr_buf : created cdir string
 *
 */
void inet_addr_to_cidr(struct in_addr addr, struct in_addr msk, char *cidr_buf)
{
	sprintf(cidr_buf, "%s/%i", inet_ntoa(addr), __builtin_popcount(msk.s_addr));
}

/* 
 * inet_cidr_to_addr - converts cidr string to address and netmask
 *
 * cidr_str : Address string in cidr notation (exmpl: 192.168.1.1/24)
 * addr     : Address as 32 bit value
 * msk      : Netmask as 32 bit value 
 *
 * returns -EINVAL  on invlaid cidr string, 0 on success.
 *
 */
int inet_cidr_to_addr(char *cidr_str, struct in_addr *addr, struct in_addr *msk)
{
	char *addr_end = strstr(cidr_str, "/");
	int sldr;

	if (addr_end == NULL)
		return -EINVAL;

	addr_end[0] = 0;
	if (inet_aton(cidr_str, addr) == 0)
		return -EINVAL;

	sldr = atoi(&addr_end[1]);
	if (sldr < 0 || sldr > 32)
		return -EINVAL;

	msk->s_addr = htonl(~(0xffffffff >> sldr));
	return 0;
}
