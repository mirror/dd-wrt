/*******************************************************************************
 *
 *
 *   Copyright (c) 2009 Cavium Networks 
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *   more details.
 *
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59
 *   Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *   The full GNU General Public License is included in this distribution in the
 *   file called LICENSE.
 *
 ********************************************************************************/

#ifndef CNS3XXX_TOOL_H
#define CNS3XXX_TOOL_H

#define PRINT_INFO printk

#if defined(__KERNEL__)

#include "cns3xxx.h"
#include <linux/kernel.h> // for printk

#else // u-boot

#endif

#define SHOW_DEBUG_MESSAGE
#ifdef SHOW_DEBUG_MESSAGE

extern int MSG_LEVEL;

#define NO_MSG 0
#define NORMAL_MSG 1
#define WARNING_MSG (1 << 1)
#define CRITICAL_MSG (1 << 2)
#define DUMP_RX_PKT_INFO (1 << 3)
#define DUMP_TX_PKT_INFO (1 << 4)

#define DEBUG_MSG(msg_level, fmt, args...)\
{ \
        int i=0; \
\
        for(i=0 ; i < 3 ; ++i) { \
                if ((MSG_LEVEL & msg_level) >> i) \
                        printk(KERN_INFO "*cns3xxx gsw debug* " fmt, ## args); \
        } \
}

#endif

#define GET_MAC_PORT_CFG(port, cfg) \
{ \
	switch (port) \
	{ \
		case MAC_PORT0: \
		{ \
			cfg = MAC0_CFG_REG; \
			break; \
		} \
		case MAC_PORT1: \
		{ \
			cfg = MAC1_CFG_REG; \
			break; \
		} \
		case MAC_PORT2: \
		{ \
			cfg = MAC2_CFG_REG; \
			break; \
		} \
	} \
}

#define SET_MAC_PORT_CFG(port, cfg) \
{ \
	switch (port) \
	{ \
		case MAC_PORT0: \
		{ \
			MAC0_CFG_REG = cfg; \
			break; \
		} \
		case MAC_PORT1: \
		{ \
			MAC1_CFG_REG = cfg; \
			break; \
		} \
		case MAC_PORT2: \
		{ \
			MAC2_CFG_REG = cfg; \
			break; \
		} \
	} \
}

#define between(x, start, end) ((x)>=(start) && (x)<=(end))
static inline void print_packet(unsigned char *data, int len) 
{
    int i,j;

    printk("packet length: %d%s:\n", len, len>128?"(only show the first 128 bytes)":"");
#if 0
    if(len > 128) {
        len = 128;
    }
#endif
    for(i=0;len;) {
        if(len >=16 ) {
            for(j=0;j<16;j++) {
                printk("%02x ", data[i++]);
            }
            printk("| ");

            i -= 16;
            for(j=0;j<16;j++) {
                if( between(data[i], 0x21, 0x7e) ) {
                    printk("%c", data[i++]);
                }
                else {
                    printk(".");
                    i++;
                }
            }
            printk("\n");

            len -= 16;
        }
        else {
            /* last line */

            for(j=0; j<len; j++) {
                printk("%02x ", data[i++]);
            }
            for(;j<16;j++) {
                printk("   ");
            }
            printk("| ");

            i -= len;
            for(j=0;j<len;j++) {
                if( between(data[i], 0x21, 0x7e) ) {
                    printk("%c", data[i++]);
                }
                else {
                    printk(".");
                    i++;
                }
            }
            for(;j<16;j++) {
                printk(" ");
            }
            printk("\n");

            len = 0;
        }
    }
    return;

}

static inline void cns3xxx_gsw_power_enable(void)
{
        PLL_HM_PD_CTRL_REG &= (~(1 << 2)); // power up PLL_RGMII (for MAC)
        CLK_GATE_REG |= (1 << 11); // enable switch clock
}       

static inline void cns3xxx_gsw_software_reset(void)
{
        SOFT_RST_REG &= (~(1 << 11));
        SOFT_RST_REG |= (1 << 11); 
}




// port:
// 0 : mac port0
// 1 : mac port1
// 2 : mac port2
// 3 : cpu port
static inline void enable_port(u8 port, u8 enable)
{
	switch (port)
	{
		case 0:
		{
			(enable==1) ? (MAC0_CFG_REG &= (~(1 << 18)) ) : (MAC0_CFG_REG |= (1 << 18)) ;

			break;
		}
		case 1:
		{
			(enable==1) ? (MAC1_CFG_REG &= (~(1 << 18)) ) : (MAC1_CFG_REG |= (1 << 18)) ;
			break;
		}
		case 2:
		{
			(enable==1) ? (MAC2_CFG_REG &= (~(1 << 18)) ) : (MAC2_CFG_REG |= (1 << 18)) ;
			break;
		}
		case 3:
		{
			(enable==1) ? (CPU_CFG_REG &= (~(1 << 18)) ) : (CPU_CFG_REG |= (1 << 18)) ;
			break;
		}
	}
}

static inline int cns3xxx_vlan_table_lookup(VLANTableEntry *entry)
{
	VLAN_CTRL2_REG |= entry->vid;
	ARL_VLAN_CMD_REG |= (1 << 8); // look up vlan table command

	// wait for vlan command complete
	while(( (ARL_VLAN_CMD_REG >> 9) & 1) == 0) ;
	
	if (!((ARL_VLAN_CMD_REG >> 10) & 1)) {
		// not found any entry
		return CAVM_NOT_FOUND;
	}

        entry->valid = ((VLAN_CTRL0_REG >> 31) & 0x1);
        entry->vid = ((VLAN_CTRL2_REG >> 31) & 0xfff);
        entry->wan_side = ((VLAN_CTRL0_REG >> 30) & 0x1);
        entry->etag_pmap = ((VLAN_CTRL0_REG >> 25) & 0x1f);
        entry->mb_pmap = ((VLAN_CTRL0_REG >> 9) & 0x1f);

        entry->my_mac[0] = ((VLAN_CTRL1_REG >> 24) & 0xff);
        entry->my_mac[1] = ((VLAN_CTRL1_REG >> 16) & 0xff);
        entry->my_mac[2] = ((VLAN_CTRL1_REG >> 8) & 0xff);
        entry->my_mac[3] = (VLAN_CTRL1_REG & 0xff);

        entry->my_mac[4] = ((VLAN_CTRL2_REG >> 24) & 0xff);
        entry->my_mac[5] = ((VLAN_CTRL2_REG >> 16) & 0xff);

	return CAVM_FOUND;
}

static inline int cns3xxx_vlan_table_read(VLANTableEntry *entry)
{
        //printf("VLAN_CTRL0_REG: %x\n", VLAN_CTRL0_REG);
	ARL_VLAN_CMD_REG &= (~0x3f);
	ARL_VLAN_CMD_REG |= (entry->vlan_index);
	ARL_VLAN_CMD_REG |= (1 << 7); // read vlan table command
        //printf("after read ARL_VLAN_CMD_REG: %x\n", ARL_VLAN_CMD_REG);

	// wait for vlan command complete
	while(( (ARL_VLAN_CMD_REG >> 9) & 1) == 0) ;

        //printf("ARL_VLAN_CMD_REG: %x\n", ARL_VLAN_CMD_REG);
        //printf("VLAN_CTRL0_REG: %x\n", VLAN_CTRL0_REG);

        entry->valid = ((VLAN_CTRL0_REG >> 31) & 0x1);
        entry->vid = ((VLAN_CTRL2_REG) & 0xfff);
        entry->wan_side = ((VLAN_CTRL0_REG >> 30) & 0x1);
        entry->etag_pmap = ((VLAN_CTRL0_REG >> 25) & 0x1f);
        entry->mb_pmap = ((VLAN_CTRL0_REG >> 9) & 0x1f);

        entry->my_mac[0] = ((VLAN_CTRL1_REG >> 24) & 0xff);
        entry->my_mac[1] = ((VLAN_CTRL1_REG >> 16) & 0xff);
        entry->my_mac[2] = ((VLAN_CTRL1_REG >> 8) & 0xff);
        entry->my_mac[3] = (VLAN_CTRL1_REG & 0xff);

        entry->my_mac[4] = ((VLAN_CTRL2_REG >> 24) & 0xff);
        entry->my_mac[5] = ((VLAN_CTRL2_REG >> 16) & 0xff);

	return CAVM_OK;

}


// add a entry in the vlan table
static inline int cns3xxx_vlan_table_add(VLANTableEntry *entry)
{
	VLAN_CTRL0_REG = 0;
	VLAN_CTRL1_REG = 0;
	VLAN_CTRL2_REG = 0;

#if 0
	printk("a [kernel mode] VLAN_CTRL0_REG: %x\n", VLAN_CTRL0_REG);
	printk("a [kernel mode] VLAN_CTRL1_REG: %x\n", VLAN_CTRL1_REG);
	printk("a [kernel mode] VLAN_CTRL2_REG: %x\n", VLAN_CTRL2_REG);
#endif

	//printk("vlan_index: %x\n", entry->vlan_index);
	VLAN_CTRL0_REG |= (entry->valid << 31);
	//DEBUG_MSG(NORMAL_MSG, "1 [kernel mode] VLAN_CTRL0_REG: %x\n", VLAN_CTRL0_REG);
	VLAN_CTRL0_REG |= (entry->wan_side << 30);
	//DEBUG_MSG(NORMAL_MSG, "2 [kernel mode] VLAN_CTRL0_REG: %x\n", VLAN_CTRL0_REG);
	//printk("entry->etag_pmap: %x\n", entry->etag_pmap);
	VLAN_CTRL0_REG |= (entry->etag_pmap << 25);
	//DEBUG_MSG(NORMAL_MSG, "3 [kernel mode] VLAN_CTRL0_REG: %x\n", VLAN_CTRL0_REG);
	//printk("entry->mb_pmap: %x\n", entry->mb_pmap);
	VLAN_CTRL0_REG |= (entry->mb_pmap << 9);
	//DEBUG_MSG(NORMAL_MSG, "4 [kernel mode] VLAN_CTRL0_REG: %x\n", VLAN_CTRL0_REG);
	//printk("bb [kernel mode] VLAN_CTRL0_REG: %x\n", VLAN_CTRL0_REG);

        //printf("vlan index: %d ## add VLAN_CTRL0_REG: %x\n", entry->vlan_index, VLAN_CTRL0_REG);


	VLAN_CTRL1_REG |= (entry->my_mac[0] << 24);
	VLAN_CTRL1_REG |= (entry->my_mac[1] << 16);
	VLAN_CTRL1_REG |= (entry->my_mac[2] << 8);
	VLAN_CTRL1_REG |= (entry->my_mac[3]);

	VLAN_CTRL2_REG |= (entry->my_mac[4] << 24);
	VLAN_CTRL2_REG |= (entry->my_mac[5] << 16);
	VLAN_CTRL2_REG |= entry->vid;

#if 0
	printk("b [kernel mode] VLAN_CTRL0_REG: %x\n", VLAN_CTRL0_REG);
	printk("b [kernel mode] VLAN_CTRL1_REG: %x\n", VLAN_CTRL1_REG);
	printk("b [kernel mode] VLAN_CTRL2_REG: %x\n", VLAN_CTRL2_REG);
#endif

	ARL_VLAN_CMD_REG &= (~0x3f);
	ARL_VLAN_CMD_REG |= (entry->vlan_index);
	ARL_VLAN_CMD_REG |= (1 << 6); // write vlan table command


        //printf("after write ARL_VLAN_CMD_REG: %x\n", ARL_VLAN_CMD_REG);

	// wait for vlan command complete
	while(( (ARL_VLAN_CMD_REG >> 9) & 1) == 0) ;

	return CAVM_OK;
}

static inline void print_arl_table_entry(ARLTableEntry *entry)
{
        printk("vid: %d\n", entry->vid);
        printk("pmap: %#x\n", entry->pmap);
        printk("age_field: %d\n", entry->age_field);
        printk("vlan_mac: %d\n", entry->vlan_mac);
        printk("filter: %d\n", entry->filter);
        printk("mac addr: %x:%x:%x:%x:%x:%x\n", entry->mac[0], entry->mac[1],entry->mac[2],entry->mac[3],entry->mac[4],entry->mac[5]);

}


static inline int cns3xxx_arl_table_lookup(ARLTableEntry *entry)
{
	ARL_CTRL0_REG = 0;
	ARL_CTRL1_REG = 0;
	ARL_CTRL2_REG = 0;

	ARL_CTRL0_REG |= (entry->vid << 16);

	ARL_CTRL1_REG |= (entry->mac[0] << 24);
	ARL_CTRL1_REG |= (entry->mac[1] << 16);
	ARL_CTRL1_REG |= (entry->mac[2] << 8);
	ARL_CTRL1_REG |= entry->mac[3];

	ARL_CTRL2_REG |= (entry->mac[4] << 24);
	ARL_CTRL2_REG |= (entry->mac[5] << 16);

	ARL_VLAN_CMD_REG |= (1 << 18); // arl table lookup command

	// wait arl command complete
	while(( (ARL_VLAN_CMD_REG >> 21) & 1) == 0);

	if (( (ARL_VLAN_CMD_REG >> 23) & 1)) {
		// found

		entry->vid = ((ARL_CTRL0_REG >> 16) & 0xfff);
		entry->pmap = ((ARL_CTRL0_REG >> 9) & 0x1f);

		entry->age_field = ((ARL_CTRL2_REG >> 4 ) & 0x7);
		entry->vlan_mac = ((ARL_CTRL2_REG >> 1 ) & 0x1);
		entry->filter = (ARL_CTRL2_REG & 0x1);
	} else {
		// not found
		return CAVM_NOT_FOUND;
	}
#if 0
	printk("[kernel mode] ARL_VLAN_CMD_REG : %#x\n", ARL_VLAN_CMD_REG);
	printk("[kernel mode] ARL_CTRL0_REG : %#x\n", ARL_CTRL0_REG);
	printk("[kernel mode] ARL_CTRL1_REG : %#x\n", ARL_CTRL1_REG);
	printk("[kernel mode] ARL_CTRL2_REG : %#x\n", ARL_CTRL2_REG);
#endif

	return CAVM_FOUND;
}

static inline int cns3xxx_arl_table_search_again(ARLTableEntry *entry)
{
	ARL_CTRL0_REG = 0;
	ARL_CTRL1_REG = 0;
	ARL_CTRL2_REG = 0;

	ARL_VLAN_CMD_REG |= (1 << 17); // arl table search again command

	// wait arl command complete
	while(( (ARL_VLAN_CMD_REG >> 21) & 1) == 0);

	if ((ARL_VLAN_CMD_REG >> 23) & 1) {

		// found
	#if 0
	printk("[kernel mode] ARL_VLAN_CMD_REG : %#x\n", ARL_VLAN_CMD_REG);
	printk("[kernel mode] ARL_CTRL0_REG : %#x\n", ARL_CTRL0_REG);
	printk("[kernel mode] ARL_CTRL1_REG : %#x\n", ARL_CTRL1_REG);
	printk("[kernel mode] ARL_CTRL2_REG : %#x\n", ARL_CTRL2_REG);
	#endif
		entry->vid = ((ARL_CTRL0_REG >> 16) & 0xfff);
		entry->pmap = ((ARL_CTRL0_REG >> 9) & 0x1f);

		entry->age_field = ((ARL_CTRL2_REG >> 4 ) & 0x7);
		entry->vlan_mac = ((ARL_CTRL2_REG >> 1 ) & 0x1);
		entry->filter = (ARL_CTRL2_REG & 0x1);

		entry->mac[0] = (ARL_CTRL1_REG >> 24);
		entry->mac[1] = (ARL_CTRL1_REG >> 16);
		entry->mac[2] = (ARL_CTRL1_REG >> 8);
		entry->mac[3] = ARL_CTRL1_REG;

		entry->mac[4] = (ARL_CTRL2_REG >> 24);
		entry->mac[5] = (ARL_CTRL2_REG >> 16);

		return CAVM_FOUND;
	} else {
		// not found
		return CAVM_NOT_FOUND;
	}
}

static inline int cns3xxx_is_arl_table_end(void)
{
	ARL_CTRL0_REG = 0;
	ARL_CTRL1_REG = 0;
	ARL_CTRL2_REG = 0;

	if (( (ARL_VLAN_CMD_REG >> 22) & 1)) { // search to table end
		return CAVM_OK;
	} else {
		return CAVM_ERR;
	}
}

static inline int cns3xxx_arl_table_search(ARLTableEntry *entry)
{
	ARL_CTRL0_REG = 0;
	ARL_CTRL1_REG = 0;
	ARL_CTRL2_REG = 0;

#if 0
	ARL_CTRL0_REG |= (entry->vid << 16);

	ARL_CTRL1_REG |= (entry->mac[0] << 24);
	ARL_CTRL1_REG |= (entry->mac[1] << 16);
	ARL_CTRL1_REG |= (entry->mac[2] << 8);
	ARL_CTRL1_REG |= entry->mac[3];

	ARL_CTRL2_REG |= (entry->mac[4] << 24);
	ARL_CTRL2_REG |= (entry->mac[5] << 16);
#endif
	ARL_VLAN_CMD_REG |= (1 << 16); // arl table search start command

	// wait arl command complete
	while(( (ARL_VLAN_CMD_REG >> 21) & 1) == 0);

	if (((ARL_VLAN_CMD_REG >> 23) & 1)) {
		// found
	#if 0
	printk("[kernel mode] ARL_VLAN_CMD_REG : %#x\n", ARL_VLAN_CMD_REG);
	printk("[kernel mode] ARL_CTRL0_REG : %#x\n", ARL_CTRL0_REG);
	printk("[kernel mode] ARL_CTRL1_REG : %#x\n", ARL_CTRL1_REG);
	printk("[kernel mode] ARL_CTRL2_REG : %#x\n", ARL_CTRL2_REG);
	#endif
		entry->vid = ((ARL_CTRL0_REG >> 16) & 0xfff);
		entry->pmap = ((ARL_CTRL0_REG >> 9) & 0x1f);

		entry->age_field = ((ARL_CTRL2_REG >> 4 ) & 0x7);
		entry->vlan_mac = ((ARL_CTRL2_REG >> 1 ) & 0x1);
		entry->filter = (ARL_CTRL2_REG & 0x1);

		entry->mac[0] = (ARL_CTRL1_REG >> 24);
		entry->mac[1] = (ARL_CTRL1_REG >> 16);
		entry->mac[2] = (ARL_CTRL1_REG >> 8);
		entry->mac[3] = ARL_CTRL1_REG;

		entry->mac[4] = (ARL_CTRL2_REG >> 24);
		entry->mac[5] = (ARL_CTRL2_REG >> 16);

		return CAVM_FOUND;
	} else {
		// not found
		return CAVM_NOT_FOUND;
	}
}


// flush all age out entries except static entries
static inline int cns3xxx_arl_table_flush(void)
{
	ARL_VLAN_CMD_REG |= (1 << 20); // flush arl table command

	// wait arl command complete
	while(( (ARL_VLAN_CMD_REG >> 21) & 1) == 0);


	return CAVM_OK;
}


// add a entry in the arl table
static inline int cns3xxx_arl_table_add(ARLTableEntry *entry)
{
	ARL_CTRL0_REG = 0;
	ARL_CTRL1_REG = 0;
	ARL_CTRL2_REG = 0;

	entry->age_field = 7; // static entry
	ARL_CTRL0_REG |= (entry->vid << 16);
	ARL_CTRL0_REG |= (entry->pmap << 9);

	ARL_CTRL1_REG |= (entry->mac[0] << 24);
	ARL_CTRL1_REG |= (entry->mac[1] << 16);
	ARL_CTRL1_REG |= (entry->mac[2] << 8);
	ARL_CTRL1_REG |= entry->mac[3];

	ARL_CTRL2_REG |= (entry->mac[4] << 24);
	ARL_CTRL2_REG |= (entry->mac[5] << 16);

	ARL_CTRL2_REG |= (entry->age_field << 4);
	ARL_CTRL2_REG |= (entry->vlan_mac << 1);
	ARL_CTRL2_REG |= (entry->filter);

	//printk("entry->age_field: %d\n", entry->age_field);
	//printk("ARL_CTRL2_REG: %x\n", ARL_CTRL2_REG);

	ARL_VLAN_CMD_REG |= (1 << 19); // arl table write command

	// wait arl command complete
	while(( (ARL_VLAN_CMD_REG >> 21) & 1) == 0);

	return CAVM_OK;
}

// invalid a entry in the arl table
static inline int cns3xxx_arl_table_invalid(ARLTableEntry *entry)
{
	entry->age_field = 0; // invalid
	return cns3xxx_arl_table_add(entry);
}

// port:
// 0 : mac port0
// 1 : mac port1
// 2 : mac port2
// 3 : cpu port
static inline void cns3xxx_set_pvid(u8 port, u16 pvid)
{
	switch (port)
	{
		case 0:
		{
			MAC1_MAC0_PVID_REG &= (~0x0fff);
			MAC1_MAC0_PVID_REG |= pvid;
			break;
		}
		case 1:
		{
			MAC1_MAC0_PVID_REG &= (~(0x0fff << 16));
			MAC1_MAC0_PVID_REG |= (pvid << 16);
			break;
		}
		case 2:
		{
			MAC2_CPU_PVID_REG &= (~(0x0fff << 16));
			MAC2_CPU_PVID_REG |= (pvid << 16);
			break;
		}
		case 3: // cpu port
		{
			MAC2_CPU_PVID_REG &= (~0x0fff);
			MAC2_CPU_PVID_REG |= pvid;
			break;
		}
	}


}

static inline u16 cns3xxx_get_pvid(u8 port)
{
		        // 0,     1,   2,    cpu port
	u16 port_offset[]={0x9c, 0x9c, 0xa0, 0xa0};
		      // 0, 1,   2,  cpu port
	u16 port_shift[]={0, 16, 16, 0};

	return ((SWITCH_REG_VALUE(port_offset[port]) >> port_shift[port]) & 0xfff);
}

// which : 0 or 1
// enable: 0 or 1
static inline int enable_rx_dma(u8 which, u8 enable)
{
	if (which == 0 ) {
		FS_DMA_CTRL0_REG = enable;
	} else if (which == 1 ) {
		FS_DMA_CTRL1_REG = enable;
	} else {
		return CAVM_ERR;
	}
	return CAVM_OK;
}


// which : 0 or 1
// enable: 0 or 1
static inline int enable_tx_dma(u8 which, u8 enable)
{
	if (which == 0 ) {
		TS_DMA_CTRL0_REG = enable;
	} else if (which == 1 ) {
		TS_DMA_CTRL1_REG = enable;
	} else {
		return CAVM_ERR;
	}
	return CAVM_OK;
}

#define DUMP_TX_DESC_PROC(tx_desc, page, num) \
{ \
	num += sprintf(page + num,"sdp: %x\n", tx_desc->sdp); \
	num += sprintf(page + num,"sdl: %d\n", tx_desc->sdl); \
	num += sprintf(page + num,"tco: %d\n", tx_desc->tco); \
	num += sprintf(page + num,"uco: %d\n", tx_desc->uco); \
	num += sprintf(page + num,"ico: %d\n", tx_desc->ico); \
	num += sprintf(page + num,"pri: %d\n", tx_desc->pri); \
	num += sprintf(page + num,"fp: %d\n", tx_desc->fp); \
	num += sprintf(page + num,"fr: %d\n", tx_desc->fr); \
	num += sprintf(page + num,"interrupt: %d\n", tx_desc->interrupt); \
	num += sprintf(page + num,"lsd: %d\n", tx_desc->lsd); \
	num += sprintf(page + num,"fsd: %d\n", tx_desc->fsd); \
	num += sprintf(page + num,"eor: %d\n", tx_desc->eor); \
	num += sprintf(page + num,"cown: %d\n", tx_desc->cown); \
 \
	num += sprintf(page + num,"ctv: %d\n", tx_desc->ctv); \
	num += sprintf(page + num,"stv: %d\n", tx_desc->stv); \
	num += sprintf(page + num,"sid: %d\n", tx_desc->sid); \
	num += sprintf(page + num,"inss: %d\n", tx_desc->inss); \
	num += sprintf(page + num,"dels: %d\n", tx_desc->dels); \
	num += sprintf(page + num,"pmap: %d\n", tx_desc->pmap); \
	num += sprintf(page + num,"mark: %d\n", tx_desc->mark); \
	num += sprintf(page + num,"ewan: %d\n", tx_desc->ewan); \
	num += sprintf(page + num,"fewan: %d\n", tx_desc->fewan); \
 \
	num += sprintf(page + num,"c_vid: %d\n", tx_desc->c_vid); \
	num += sprintf(page + num,"c_cfs: %d\n", tx_desc->c_cfs); \
	num += sprintf(page + num,"c_pri: %d\n", tx_desc->c_pri); \
	num += sprintf(page + num,"s_vid: %d\n", tx_desc->s_vid); \
	num += sprintf(page + num,"s_dei: %d\n", tx_desc->s_dei); \
	num += sprintf(page + num,"s_pri: %d\n", tx_desc->s_pri); \
}

static inline void dump_tx_desc(TXDesc volatile *tx_desc)
{
	printk("sdp: %x\n", tx_desc->sdp);
	printk("sdl: %d\n", tx_desc->sdl);
	printk("tco: %d\n", tx_desc->tco);
	printk("uco: %d\n", tx_desc->uco);
	printk("ico: %d\n", tx_desc->ico);
	printk("pri: %d\n", tx_desc->pri);
	printk("fp: %d\n", tx_desc->fp);
	printk("fr: %d\n", tx_desc->fr);
	printk("interrupt: %d\n", tx_desc->interrupt);
	printk("lsd: %d\n", tx_desc->lsd);
	printk("fsd: %d\n", tx_desc->fsd);
	printk("eor: %d\n", tx_desc->eor);
	printk("cown: %d\n", tx_desc->cown);

	printk("ctv: %d\n", tx_desc->ctv);
	printk("stv: %d\n", tx_desc->stv);
	printk("sid: %d\n", tx_desc->sid);
	printk("inss: %d\n", tx_desc->inss);
	printk("dels: %d\n", tx_desc->dels);
	printk("pmap: %d\n", tx_desc->pmap);
	printk("mark: %d\n", tx_desc->mark);
	printk("ewan: %d\n", tx_desc->ewan);
	printk("fewan: %d\n", tx_desc->fewan);

	printk("c_vid: %d\n", tx_desc->c_vid);
	printk("c_cfs: %d\n", tx_desc->c_cfs);
	printk("c_pri: %d\n", tx_desc->c_pri);
	printk("s_vid: %d\n", tx_desc->s_vid);
	printk("s_dei: %d\n", tx_desc->s_dei);
	printk("s_pri: %d\n", tx_desc->s_pri);
}

static inline void dump_rx_desc(RXDesc volatile *rx_desc)
{

	printk("rx_desc: %p\n", rx_desc);
	//printk("rx_desc: %p ## cown: %d\n", rx_desc, rx_desc->cown);
	//printk("rx_desc phy addr : %x\n", (u32)page_to_dma(NULL, rx_desc) );
#if 0
	int i=0;
	for (i=0; i < 8 ; ++4) {
		u32 rx_desc_data = *((u32 *)(rx_desc+i));
		printk("%d: %#x\n", i, rx_desc_data);
	}
#endif

	printk("sdp: %x\n", rx_desc->sdp);

	printk("sdl: %d\n", rx_desc->sdl);
#if 1
	printk("l4f: %d\n", rx_desc->l4f);
	printk("ipf: %d\n", rx_desc->ipf);
	printk("prot: %d\n", rx_desc->prot);
	printk("hr: %d\n", rx_desc->hr);
	printk("lsd: %d\n", rx_desc->lsd);
	printk("fsd: %d\n", rx_desc->fsd);
	printk("eor: %d\n", rx_desc->eor);
#endif
	printk("cown: %d\n", rx_desc->cown);

#if 1
	printk("ctv: %d\n", rx_desc->ctv);
	printk("stv: %d\n", rx_desc->stv);
	printk("unv: %d\n", rx_desc->unv);
	printk("iwan: %d\n", rx_desc->iwan);
	printk("exdv: %d\n", rx_desc->exdv);
	printk("sp: %d\n", rx_desc->sp);
	printk("crc_err: %d\n", rx_desc->crc_err);
	printk("un_eth: %d\n", rx_desc->un_eth);
	printk("tc: %d\n", rx_desc->tc);
	printk("ip_offset: %d\n", rx_desc->ip_offset);

	printk("c_vid: %d\n", rx_desc->c_vid);
	printk("c_cfs: %d\n", rx_desc->c_cfs);
	printk("c_pri: %d\n", rx_desc->c_pri);
	printk("s_vid: %d\n", rx_desc->s_vid);
	printk("s_dei: %d\n", rx_desc->s_dei);
	printk("s_pri: %d\n", rx_desc->s_pri);
#endif
}

static inline void dump_all_rx_ring(const RXRing *rx_ring, u8 r_index)
{
	int i=0;

	RXBuffer volatile *rx_buf = get_rx_ring_head(rx_ring);

	printk("all rx ring: %d\n", r_index);
	for (i=0 ; i < get_rx_ring_size(rx_ring) ; ++i) {
		printk("%d ## rx_buf: %p ##  rx_buf->rx_desc: %p\n", i, rx_buf, rx_buf->rx_desc);
		dump_rx_desc(rx_buf->rx_desc);
		++rx_buf;
	}
}

static inline void rx_dma_suspend(u8 enable)
{
#if 1
	DMA_AUTO_POLL_CFG_REG &= (~0x00000001);
	if (enable == 1)
		DMA_AUTO_POLL_CFG_REG |= 1;
#endif
}


// clear: 0 normal
// clear: 1 clear
static inline void clear_fs_dma_state(u8 clear)
{
	DMA_RING_CTRL_REG &= (~(1 << 31));
	if (clear==1) {
		DMA_RING_CTRL_REG |= (1 << 31);
	}
}

// enable: 1 -> IVL
// enable: 0 -> SVL
static inline void cns3xxx_ivl(u8 enable)
{
	// SVL
	MAC_GLOB_CFG_REG &= (~(0x1 << 7));
	if (enable == 1)
		MAC_GLOB_CFG_REG |= (0x1 << 7);
}

static inline void cns3xxx_nic_mode(u8 enable)
{
	VLAN_CFG &= (~(1<<15));
	if (enable == 1) 
		VLAN_CFG |= (1<<15);
}


void gic_mask_irq(unsigned int irq);
void gic_unmask_irq(unsigned int irq);
extern void __iomem *gic_cpu_base_addr;


static inline void cns3xxx_disable_irq(u32 irq)
{
#ifdef CONFIG_SMP
	disable_irq_nosync(irq);
#else
	disable_irq(irq);
#endif
	//gic_mask_irq(irq);
}

static inline void cns3xxx_enable_irq(u32 irq)
{
	enable_irq(irq);
	//gic_unmask_irq(irq);
}

static inline int cns3xxx_get_tx_hw_index(u8 ring_index)
{
	if (ring_index == 0) {
		return (TS_DESC_PTR0_REG - TS_DESC_BASE_ADDR0_REG) / sizeof (TXDesc);
	} else if (ring_index == 1) {
		return (TS_DESC_PTR1_REG - TS_DESC_BASE_ADDR1_REG) / sizeof (TXDesc);
	} else {
		return CAVM_ERR;
	}
}

static inline TXBuffer* get_tx_buffer_by_index(TXRing *tx_ring, int i)
{ 
	int index = i;  

	index = ((index + get_tx_ring_size(tx_ring) )% get_tx_ring_size(tx_ring)); 

	return tx_ring->head + index;
}

static inline int cns3xxx_is_untag_packet(const RXDesc *rx_desc)
{
	return rx_desc->crc_err;
}


#ifdef CONFIG_SWITCH_BIG_ENDIAN
static inline void swap_rx_desc(RXDesc *org_desc, RXDesc *new_desc)
{
         int i=0;
         void *org_p = org_desc;
         void *new_p = new_desc;

        for (i=0; i < 16 ; i+=4) { 
                u32 rx_desc_data = 0; 
                u32 swab_rx_desc_data = 0;

                rx_desc_data = *((volatile u32 *)(org_p+i));
                swab_rx_desc_data = ___swab32(rx_desc_data);

                *((volatile u32 *)(new_p+i)) = swab_rx_desc_data;
        }
}

static inline void swap_tx_desc(TXDesc *org_desc, TXDesc *new_desc)
{
        int i=0;
        void *org_p = org_desc;
        void *new_p = new_desc;

        for (i=0; i < 16 ; i+=4) {
                u32 desc_data = *((volatile u32 *)(org_p+i));
                u32 swab_desc_data = ___swab32(desc_data);

                *((volatile u32 *)(new_p+i)) = swab_desc_data;
        }
}

#endif


static inline int cns3xxx_min_mtu(void)
{
	return 64;
}

static inline int cns3xxx_max_mtu(void)
{
	int max_len[]={1518, 1522, 1536, 9600};

        return max_len[((PHY_AUTO_ADDR_REG >> 30) & 0x3)];
}

#endif // CNS3XXX_TOOL_H
