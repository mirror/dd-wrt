#ifndef IPTRAF_NG_DDWATCH_H
#define IPTRAF_NG_DDWATCH_H

/***

serv.h  - TCP/UDP port statistics header file

***/

struct addtab {
  struct sockaddr_storage addr;
  in_port_t port_min;
  in_port_t port_max;
  struct addtab *prev_entry;
  struct addtab *next_entry;
};

void ddloadaddports(struct addtab **table);
void ddmon(char *iface, time_t facilitytime);
void destroyaddtab(struct addtab *table);
#endif	/* IPTRAF_NG_DDWATCH_H */

