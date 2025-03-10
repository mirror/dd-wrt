#ifndef _LINUX_NETFILTER_XT_TARPIT_H
#define _LINUX_NETFILTER_XT_TARPIT_H 1

enum xt_tarpit_target_variant {
	XTTARPIT_TARPIT,
	XTTARPIT_HONEYPOT,
	XTTARPIT_RESET,
};

struct xt_tarpit_tginfo {
	uint8_t variant;
};

#endif /* _LINUX_NETFILTER_XT_TARPIT_H */
