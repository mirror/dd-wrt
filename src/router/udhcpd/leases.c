/* 
 * leases.c -- tools to manage DHCP leases 
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "arpping.h"
#include "get_time.h"

unsigned char blank_chaddr[] = {[0 ... 15] = 0};

/* check if there is a static lease with chaddr OR yiaddr */
struct dhcpOfferedAddr *check_static_leases(u_int8_t *chaddr, u_int32_t yiaddr) {
	unsigned int i, j;
	
	for (j = 0; j < 16 && !chaddr[j]; j++);
	
	for (i = 0; i < server_config.max_leases; i++)
		if(leases[i].expires == EXPIRES_NEVER) {
			if ((j != 16 && !memcmp(leases[i].chaddr, chaddr, 16) &&
			     // Allow multiple "match-any" MAC addresses
			     memcmp(chaddr, MAC_BCAST_ADDR, strlen(MAC_BCAST_ADDR))) ||
			    (yiaddr && leases[i].yiaddr == yiaddr)) {
				return &leases[i];
			}
		}
	return NULL;
}

/* clear every lease out that chaddr OR yiaddr matches and is nonzero */
void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr)
{
	unsigned int i, j;
	
	for (j = 0; j < 16 && !chaddr[j]; j++);
	
	for (i = 0; i < server_config.max_leases; i++)
		if ((j != 16 && !memcmp(leases[i].chaddr, chaddr, 16) &&
		     // Don't clear "match-any" MAC addresses
		     memcmp(chaddr, MAC_BCAST_ADDR, strlen(MAC_BCAST_ADDR))) ||
		    (yiaddr && leases[i].yiaddr == yiaddr)) {
			memset(&(leases[i]), 0, sizeof(struct dhcpOfferedAddr));
		}
}


/* add a lease into the table, clearing out any old ones */
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease)
{
	struct dhcpOfferedAddr *oldest;
	
	oldest = check_static_leases(chaddr, yiaddr);
	if(oldest) {
		/* static lease exists, these don't want to be changed */
		return oldest;
	}
	
	/* clean out any old ones */
	clear_lease(chaddr, yiaddr);
		
	oldest = oldest_expired_lease();
	
	if (oldest) {
		memcpy(oldest->chaddr, chaddr, 16);
		oldest->yiaddr = yiaddr;
		if(lease == EXPIRES_NEVER) 
			oldest->expires = EXPIRES_NEVER;
		else 	oldest->expires = get_time(0) + lease;
	}
	
	return oldest;
}


/* true if a lease has expired */
int lease_expired(struct dhcpOfferedAddr *lease)
{
	if(lease->expires == EXPIRES_NEVER)
		return 0;
	else	return (lease->expires < (unsigned long) get_time(0));
}	

/* Find the oldest expired lease, NULL if there are no expired leases */
struct dhcpOfferedAddr *oldest_expired_lease(void)
{
	struct dhcpOfferedAddr *oldest = NULL;
	unsigned long oldest_lease = get_time(0);
	unsigned int i;

	
	for (i = 0; i < server_config.max_leases; i++)
		if (lease_expired(&leases[i]) && oldest_lease > leases[i].expires) {
			oldest_lease = leases[i].expires;
			oldest = &(leases[i]);
		}
	return oldest;
		
}


/* Find the first lease that matches chaddr, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (!memcmp(leases[i].chaddr, chaddr, 16)) return &(leases[i]);
	
	return NULL;
}


/* Find the first lease that matches yiaddr, NULL is no match */
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (leases[i].yiaddr == yiaddr) {

			return &(leases[i]);
		}
	
	return NULL;
}


/* Find the first lease that matches hostname, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_hostname(unsigned char *hostname)
{
	unsigned int i;
	for (i = 0; i < server_config.max_leases; i++) {
	      if (leases[i].hostname!=NULL)
		if (!strncmp(leases[i].hostname, hostname, hostname[-1]) &&
		    (strlen(leases[i].hostname) == hostname[-1])) {
			return &(leases[i]);
		}
	}
	
	return NULL;
}


/* find an assignable address, it check_expired is true, we check all the expired leases as well.
 * Maybe this should try expired leases by age... */
u_int32_t find_address(int check_expired) 
{
	u_int32_t addr, ret;
	struct dhcpOfferedAddr *lease = NULL;		

	addr = ntohl(server_config.start); /* addr is in host order here */
	for (;addr <= ntohl(server_config.end); addr++) {

		/* Router IP , by honor*/
		if (ret == server_config.server) continue;
		
		/* ie, 192.168.55.0 */
		if (!(addr & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((addr & 0xFF) == 0xFF) continue;

		/* lease is not taken */
		ret = htonl(addr);
		if ((!(lease = find_lease_by_yiaddr(ret)) ||

		     /* or it expired and we are checking for expired leases */
		     (check_expired  && lease_expired(lease))) &&

		     /* and it isn't on the network */
	    	     !check_ip(ret)) {
			return ret;
			break;
		}
	}
	return 0;
}


/* check is an IP is taken, if it is, add it to the lease table */
int check_ip(u_int32_t addr)
{
	struct in_addr temp;
	
	
	if (arpping(addr, server_config.server, server_config.arp, server_config.interface) == 0) {
		temp.s_addr = addr;
	 	LOG(LOG_INFO, "%s belongs to someone, reserving it for %ld seconds", 
	 		inet_ntoa(temp), server_config.conflict_time);
		add_lease(blank_chaddr, addr, server_config.conflict_time);
		return 1;
	} else return 0;
}

