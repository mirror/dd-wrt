#ifndef __AR531X_PLATFORM_H
#define __AR531X_PLATFORM_H

struct ar531x_eth {
	int phy;
	int mac;
	u32 reset_base;
	u32 reset_mac;
	u32 reset_phy;
	char *board_config;
};

#endif /* __AR531X_PLATFORM_H */

