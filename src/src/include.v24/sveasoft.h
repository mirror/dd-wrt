#define MAX_WDS_DEVS 10

int ishexit(char c);
int sv_valid_hwaddr(char *value);
int sv_valid_ipaddr(char *value);
int sv_valid_range(char *value, int low, int high);
int sv_valid_statics(char* value);
void get_network(char *ipaddr, char *netmask);
void get_broadcast(char *ipaddr, char *netmask);
int route_manip(int cmd, char *name, int metric, char *dst, char *gateway, char *genmask);
int route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
