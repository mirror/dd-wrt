/***

ifaces.h - prototype declaration for interface support determination
		routine.
		
***/

FILE *open_procnetdev(void);
void get_next_iface(FILE * fd, char *ifname);
int iface_supported(char *iface);
int iface_up(char *iface);
void err_iface_unsupported(void);
void err_iface_down(void);
void isdn_iface_check(int *fd, char *ifname);
char *gen_iface_msg(char *ifptr);
