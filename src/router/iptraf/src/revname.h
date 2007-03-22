/***

revname.h - public declarations related to reverse name resolution

***/

int rvnamedactive();
int killrvnamed();
void open_rvn_socket(int *fd);
void close_rvn_socket(int fd);
int revname(int *lookup, struct in_addr *saddr, char *target, int rvnfd);
