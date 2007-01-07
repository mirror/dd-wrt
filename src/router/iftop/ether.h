#ifndef __ETHER_H_
#define __ETHER_H_

#define	ETHERTYPE_PUP		0x0200   
#define	ETHERTYPE_IP		0x0800
#define	ETHERTYPE_ARP		0x0806
#define	ETHERTYPE_REVARP	0x8035

#define	ETHER_ADDR_LEN		6

struct	ether_header {
	u_int8_t	ether_dhost[ETHER_ADDR_LEN];
	u_int8_t	ether_shost[ETHER_ADDR_LEN];
	u_int16_t	ether_type;
};

struct vlan_8021q_header {
	u_int16_t	priority_cfi_vid;
	u_int16_t	ether_type;
};

#endif 
