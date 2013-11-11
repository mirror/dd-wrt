#ifndef IPTRAF_NG_IFSTATS_H
#define IPTRAF_NG_IFSTATS_H

void selectiface(char *ifname, int withall, int *aborted);
void ifstats(time_t facilitytime);

#endif /* IPTRAF_NG_IFSTATS_H */
