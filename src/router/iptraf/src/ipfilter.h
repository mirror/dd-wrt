void gethostparams(struct hostparams *data, char *init_saddr,
                   char *init_smask, char *init_sport1, char *init_sport2,
                   char *init_daddr, char *init_dmask,
                   char *init_dport1, char *init_dport2,
                   char *initinex, char *initmatchop, int *aborted);

void ipfilterselect(struct filterlist *fl,
                    char *filename, int *fltcode, int *faborted);
int ipfilter(unsigned long saddr, unsigned long daddr,
             unsigned int sport, unsigned int dport,
             unsigned int protocol, int match_opp_mode,
             struct filterlist *fl);
