#ifndef IPTRAF_NG_IFACES_H
#define IPTRAF_NG_IFACES_H

#define dev_set_promisc(dev)   dev_set_flags((dev), IFF_PROMISC)
#define dev_clr_promisc(dev)   dev_clear_flags((dev), IFF_PROMISC)

FILE *open_procnetdev(void);
int get_next_iface(FILE * fd, char *ifname, int n);
int dev_up(char *iface);
void err_iface_down(void);
int dev_get_ifindex(const char *iface);
int dev_get_mtu(const char *iface);
int dev_get_flags(const char *iface);
int dev_set_flags(const char *iface, int flags);
int dev_clear_flags(const char *iface, int flags);
int dev_get_ifname(int ifindex, char *ifname);
int dev_bind_ifindex(const int fd, const int ifindex);
int dev_bind_ifname(const int fd, const char const *ifname);
char *gen_iface_msg(char *ifptr);
int dev_promisc_flag(const char *dev_name);

#endif	/* IPTRAF_NG_IFACES_H */
