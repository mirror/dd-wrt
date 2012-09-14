#ifndef IPTRAF_NG_SERVNAME_H
#define IPTRAF_NG_SERVNAME_H

/***

servname.h - function prototype for service lookup

***/

void servlook(int servnames, unsigned int port, unsigned int protocol,
	      char *target, int maxlen);

#endif	/* IPTRAF_NG_SERVNAME_H */
