#ifndef _RTL_IOCTL_H
#define _RTL_IOCTL_H

#define RTL_SMI_INIT		0x6b01
#define RTL_VLAN_INIT		0x6b02
#define RTL_REG_READ		0x6b11
#define RTL_REG_WRITE		0x6b12
#define RTL_SET_VLAN		0x6b21
#define RTL_GET_VLAN		0x6b22
#define RTL_SET_VLAN_PVID	0x6b23
#define RTL_GET_VLAN_PVID	0x6b24
#define RTL_QOS_INIT		0x6b31
#define RTL_SET_FLOW		0x6b32
#define RTL_GET_FLOW		0x6b33
#define RTL_SET_GREEN_ETHERNET 0x6b34
#define RTL_GET_GREEN_ETHERNET 0x6b35
#define RTL_GET_ASIC_REG 0x6b36
#define RTL_SET_ASIC_REG 0x6b37

typedef struct rtl_sw_reg {
	unsigned int off;
	unsigned int val;
} rtl_reg;

typedef struct rtl_sw_vlan {
	unsigned int vid;
	unsigned int map;
	unsigned int utag;
} rtl_vlan;

typedef struct rtl_sw_vlan_pvid {
	unsigned int port;
	unsigned int pvid;
	unsigned int pri;
} rtl_vlan_pvid;

typedef struct rtl_sw_set_reg {
	unsigned int reg_addr;
	unsigned int reg_val;
} rtl_set_reg;
#endif
