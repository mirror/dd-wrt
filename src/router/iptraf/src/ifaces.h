#ifndef IPTRAF_NG_IFACES_H
#define IPTRAF_NG_IFACES_H

/***

ifaces.h - prototype declaration for interface support determination
		routine.

***/

FILE *open_procnetdev(void);
int get_next_iface(FILE * fd, char *ifname, int n);
int dev_up(char *iface);
void err_iface_down(void);
int dev_get_ifindex(const char *iface);
int dev_get_mtu(const char *iface);
int dev_get_flags(const char *iface);
int dev_set_flags(const char *iface, int flags);
int dev_clear_flags(const char *iface, int flags);
int dev_set_promisc(char *ifname);
int dev_clear_promisc(char *ifname);
int dev_get_ifname(int ifindex, char *ifname);
int dev_bind_ifindex(const int fd, const int ifindex);
int dev_bind_ifname(const int fd, const char const *ifname);
void isdn_iface_check(int *fd, char *ifname);
char *gen_iface_msg(char *ifptr);

#endif	/* IPTRAF_NG_IFACES_H */
