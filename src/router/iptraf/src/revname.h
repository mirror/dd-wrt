#ifndef IPTRAF_NG_REVNAME_H
#define IPTRAF_NG_REVNAME_H

/***

revname.h - public declarations related to reverse name resolution

***/

struct resolver {
	bool lookup;
	int sock;
	pid_t server;
};

void resolver_init(struct resolver *r, bool lookup);
void resolver_destroy(struct resolver *r);

int revname(struct resolver *res, struct sockaddr_storage *addr,
	    char *target, size_t target_size);

#endif	/* IPTRAF_NG_REVNAME_H */
