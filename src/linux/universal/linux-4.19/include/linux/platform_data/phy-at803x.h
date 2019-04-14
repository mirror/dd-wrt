#ifndef _PHY_AT803X_PDATA_H
#define _PHY_AT803X_PDATA_H

struct at803x_platform_data {
	int disable_smarteee:1;
	int enable_rgmii_tx_delay:1;
	int enable_rgmii_rx_delay:1;
	int fixup_rgmii_tx_delay:1;
};

#endif /* _PHY_AT803X_PDATA_H */
