#ifndef IPTRAF_NG_REVNAME_H
#define IPTRAF_NG_REVNAME_H

/***

revname.h - public declarations related to reverse name resolution

***/

int rvnamedactive(void);
int killrvnamed(void);
void open_rvn_socket(int *fd);
void close_rvn_socket(int fd);

int revname(int *lookup, struct in_addr *saddr, struct in6_addr *s6addr,
	    char *target, int rvnfd);

#endif	/* IPTRAF_NG_REVNAME_H */
