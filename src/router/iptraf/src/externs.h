
/***

externs.h - external routines used by the the iptraf module and some
others

***/

void ipmon(const struct OPTIONS *options, struct filterstate *ofilter,
           int facilitytime, char *ifptr);
void selectiface(char *ifname, int withall, int *aborted);
void ifstats(const struct OPTIONS *options, struct filterstate *ofilter,
             int facilitytime);
void detstats(char *iface, const struct OPTIONS *options, int facilitytime,
              struct filterstate *ofilter);
void packet_size_breakdown(struct OPTIONS *options, char *iface,
                           int facilitytime, struct filterstate *ofilter);
void servmon(char *iface, struct porttab *ports,
             const struct OPTIONS *options, int facilitytime,
             struct filterstate *ofilter);
void ip_host_breakdown(struct OPTIONS *options, char *iface);
void hostmon(const struct OPTIONS *options, int facilitytime, char *ifptr,
             struct filterstate *ofilter);
void ethdescmgr(void);
void setoptions(struct OPTIONS *options, struct porttab **ports);
void loadoptions(struct OPTIONS *options);
void saveoptions(struct OPTIONS *options);
