#ifndef IPTRAF_NG_SERVNAME_H
#define IPTRAF_NG_SERVNAME_H

/***

servname.h - function prototype for service lookup

***/

void servlook(in_port_t port, unsigned int protocol, char *target, int maxlen);

#endif	/* IPTRAF_NG_SERVNAME_H */
