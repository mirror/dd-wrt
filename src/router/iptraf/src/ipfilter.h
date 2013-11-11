#ifndef IPTRAF_NG_IPFILTER_H
#define IPTRAF_NG_IPFILTER_H

void gethostparams(struct hostparams *data, char *init_saddr, char *init_smask,
		   char *init_sport1, char *init_sport2, char *init_daddr,
		   char *init_dmask, char *init_dport1, char *init_dport2,
		   char *initinex, char *initmatchop, int *aborted);
void ipfilterselect(int *faborted);
int ipfilter(unsigned long saddr, unsigned long daddr, in_port_t sport,
	     in_port_t dport, unsigned int protocol, int match_opp_mode);

#endif	/* IPTRAF_NG_IPFILTER_H */
