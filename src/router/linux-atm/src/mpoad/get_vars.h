#ifndef GET_VARS_H
#define GET_VARS_H

#include <linux/types.h>

int get_ttl(void);
uint32_t get_own_ip_addr(int iface_nmbr);
int get_own_atm_addr(unsigned char *address );

#endif /* GET_VARS_H */

