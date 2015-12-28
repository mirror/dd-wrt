/*
 * Copyright (c) 2008, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include "ag7240_phy.h"
#include "ag7240.h"
#include "ar7240_s26_phy.h"
#include "eth_diag.h"

#define TRUE    1
#define FALSE   0

#define PYTHON_OK   0
#define PYTHON_ERR  1
#define PYTHON_FULL 2

#define PYTHON_MAX_PORT_ID 4 /* lan ports and cpu port, exclude cpu port */
#define PYTHON_MAX_PORTS_ID 31 /* The maxsize is 0x11111 */

#define PYTHON_PORT_ID_CHECK(port_id) \
do { \
    if (PYTHON_MAX_PORT_ID < port_id) \
        return PYTHON_ERR; \
} while (0)

#define PYTHON_PORTS_ID_CHECK(port_id) \
do { \
    if (PYTHON_MAX_PORTS_ID < port_id) \
        return PYTHON_ERR; \
} while (0)


#define PORT_CONTROL_REG_ADDR(port_id) (0x104 + 0x100 * port_id)

static void
athrs26_reg_write(unsigned int s26_addr, unsigned int s26_write_data);

uint32_t
python_port_igmps_status_set(uint32_t port_id, BOOL enable)
{
    uint32_t data;
    uint32_t addr;

    PYTHON_PORT_ID_CHECK(port_id);

    addr = PORT_CONTROL_REG_ADDR(port_id);

    data = athrs26_reg_read(addr);
    if (TRUE == enable) {
        data |= (0x1 << 10);
    } else if (FALSE == enable) {
        data &= (~(0x1 << 10));
    } else {
        return PYTHON_ERR;
    }
    athrs26_reg_write(addr, data);

    return PYTHON_OK;
}




#define PYTHON_VLAN_ID_CHECK(vlan_id) \
do { \
    if ((4095 < vlan_id) || (0 > vlan_id)) \
        return PYTHON_ERR; \
} while (0)

#define VLAN_LOAD_ENTRY     2
#define VLAN_PURGE_ENTRY    3
#define VLAN_FIND_ENTRY     6

#define PYTHON_VTF0_ADDR    0x40
#define PYTHON_VTF1_ADDR    0x44

static uint32_t
_python_vlan_commit(uint32_t op)
{
    uint32_t vt_busy = 1, i = 0x1000, vt_full, reg0;

    reg0  = athrs26_reg_read(PYTHON_VTF0_ADDR);
    reg0 |= ((0x7 & op) | 0x8);
    athrs26_reg_write(PYTHON_VTF0_ADDR, reg0);

    while (vt_busy && --i) {
        reg0    = athrs26_reg_read(PYTHON_VTF0_ADDR);
        vt_busy = reg0 & 0x8;
        udelay(5);
    }

    /* hardware can't complete the operation */
    if (0 == i) {
        return PYTHON_ERR;
    }

    vt_full = reg0 & 0x10;
    if (vt_full) {
        /* write one to clear full voilation bit */
        reg0 = 0x10;
        athrs26_reg_write(PYTHON_VTF0_ADDR, reg0);
        return PYTHON_FULL;
    }

    return PYTHON_OK;
}

static uint32_t
_python_vlan_read(uint32_t vlan_id, uint32_t * port_bmap)
{
    uint32_t reg0 = 0, reg1 = 0, ret;

    PYTHON_VLAN_ID_CHECK(vlan_id);

    reg0 |= ((0xfff & vlan_id) << 16);
    athrs26_reg_write(PYTHON_VTF0_ADDR, reg0);

    ret = _python_vlan_commit(VLAN_FIND_ENTRY);
    if (PYTHON_OK != ret) {
        return PYTHON_ERR;
    }

    reg0 = athrs26_reg_read(PYTHON_VTF0_ADDR);
    reg1 = athrs26_reg_read(PYTHON_VTF1_ADDR);

    if (reg1 & 0x800) {
        *port_bmap = reg1 & 0x3f;
        return PYTHON_OK;
    } else {
        return PYTHON_ERR;
    }
}

uint32_t
python_vlan_create(uint32_t vlan_id)
{
    uint32_t reg0 = 0, reg1 = 0;
    uint32_t ret;

    PYTHON_VLAN_ID_CHECK(vlan_id);

    reg0 |= (vlan_id << 16);
    reg1 |= (0x1 << 11);
    athrs26_reg_write(PYTHON_VTF0_ADDR, reg0);
    athrs26_reg_write(PYTHON_VTF1_ADDR, reg1);

    ret = _python_vlan_commit(VLAN_LOAD_ENTRY);
    if (PYTHON_OK != ret) {
        return PYTHON_ERR;
    }

    return PYTHON_OK;
}

uint32_t
python_vlan_delete(uint32_t vlan_id)
{
    uint32_t reg0 = 0;
    uint32_t ret;

    PYTHON_VLAN_ID_CHECK(vlan_id);

    reg0 |= (vlan_id << 16);
    athrs26_reg_write(PYTHON_VTF0_ADDR, reg0);

    ret = _python_vlan_commit(VLAN_PURGE_ENTRY);
    if (PYTHON_OK != ret) {
        return PYTHON_ERR;
    }

    return PYTHON_OK;
}

uint32_t
python_vlan_query(uint32_t vlan_id, uint32_t * port_bmap)
{
    PYTHON_VLAN_ID_CHECK(vlan_id);

    if (!port_bmap) {
        return PYTHON_ERR;
    }

    return _python_vlan_read(vlan_id, port_bmap);
}

uint32_t
python_vlan_member_add(uint32_t vlan_id, uint32_t port_id)
{
    uint32_t port_bmap, reg0 = 0, reg1 = 0;
    uint32_t ret;

    PYTHON_VLAN_ID_CHECK(vlan_id);
    PYTHON_PORT_ID_CHECK(port_id);

    ret = _python_vlan_read(vlan_id, &port_bmap);
    if (PYTHON_OK != ret) {
        /* the vlan entry has not been created */
        return PYTHON_ERR;
    }

    port_bmap |= (0x1 << port_id);
    reg0 |= (vlan_id << 16);
    reg1 |= ((0x1 << 11) | port_bmap);
    athrs26_reg_write(PYTHON_VTF0_ADDR, reg0);
    athrs26_reg_write(PYTHON_VTF1_ADDR, reg1);

    ret = _python_vlan_commit(VLAN_LOAD_ENTRY);
    if (PYTHON_FULL == ret) {
        /* hardware will set full bit when update vlan entry */
        return PYTHON_OK;
    }

    return ret;
}

uint32_t
python_vlan_member_del(uint32_t vlan_id, uint32_t port_id)
{
    uint32_t port_bmap, reg0 = 0, reg1 = 0;
    uint32_t ret;

    PYTHON_VLAN_ID_CHECK(vlan_id);
    PYTHON_PORT_ID_CHECK(port_id);

    ret = _python_vlan_read(vlan_id, &port_bmap);
    if (PYTHON_OK != ret) {
        /* the vlan entry has not been created */
        return PYTHON_ERR;
    }

    port_bmap &= (~(0x1 << port_id));
    reg0 |= (vlan_id << 16);
    reg1 |= ((0x1 << 11) | port_bmap);
    athrs26_reg_write(PYTHON_VTF0_ADDR, reg0);
    athrs26_reg_write(PYTHON_VTF1_ADDR, reg1);

    ret = _python_vlan_commit(VLAN_LOAD_ENTRY);
    if (PYTHON_FULL == ret) {
        /* hardware will set full bit when update vlan entry */
        return PYTHON_OK;
    }

    return ret;
}

#define PORT_BASE_VLAN_ADDR(port_id) (0x108 + 0x100 * port_id)

uint32_t
python_port_default_vid_set(uint32_t port_id, uint32_t vlan_id)
{
    uint32_t addr, data;

    PYTHON_PORT_ID_CHECK(port_id);
    PYTHON_VLAN_ID_CHECK(vlan_id);

    addr = PORT_BASE_VLAN_ADDR(port_id);
    data = athrs26_reg_read(addr);
    data &= 0xfffff000;
    data |= vlan_id;
    athrs26_reg_write(addr, data);

    return PYTHON_OK;
}

typedef enum {
    PORT_EG_UNMODIFIED = 0,  /**<  egress transmit packets unmodified */
    PORT_EG_UNTAGGED,        /**<  egress transmit packets without vlan tag */
    PORT_EG_TAGGED,          /**<  egress transmit packets with vlan tag */
} port_1q_egmode_t;

uint32_t
python_port_egvlanmode_set(uint32_t port_id, port_1q_egmode_t mode)
{
    uint32_t addr, data;

    PYTHON_PORT_ID_CHECK(port_id);

    addr = PORT_CONTROL_REG_ADDR(port_id);

    data = athrs26_reg_read(addr);
    data &= 0xfffffcff;
    if (PORT_EG_UNMODIFIED == mode) {
        /* need't set value */
    } else if (PORT_EG_UNTAGGED == mode) {
        data |= (0x1 << 8);
    } else if (PORT_EG_TAGGED == mode) {
        data |= (0x2 << 8);
    } else {
        /* hardware not support value */
        return PYTHON_ERR;
    }
    athrs26_reg_write(addr, data);

    return PYTHON_OK;
}

/* Don't forget to enable the ports once the driver has completed the startup.*/
uint32_t
python_port_1qmode_set(uint32_t port_id, BOOL enable)
{
    uint32_t addr, data;

    PYTHON_PORT_ID_CHECK(port_id);

    addr = PORT_BASE_VLAN_ADDR(port_id);

    data = athrs26_reg_read(addr);
    data &= 0x3fffffff;
    if (TRUE == enable) {
        data |= (0x3 << 30);
    } else if (FALSE == enable) {
        /* needn't set value */
    } else {
        return PYTHON_ERR;
    }
    athrs26_reg_write(addr, data);

    return PYTHON_OK;
}




#define PYTHON_ATF0_ADDR          0x50
#define PYTHON_ATF1_ADDR          0x54
#define PYTHON_ATF2_ADDR          0x58

#define ARL_FLUSH_ALL             1
#define ARL_LOAD_ENTRY            2
#define ARL_PURGE_ENTRY           3
#define ARL_FLUSH_ALL_UNLOCK      4
#define ARL_LOOKUP_ENTRY          7

static uint32_t
_python_fdb_commit(uint32_t op)
{
    uint32_t busy = 1, i = 0x1000;
    uint32_t reg0, full_vio;

    reg0 = athrs26_reg_read(PYTHON_ATF0_ADDR);
    reg0 |= ((op & 0x7) | 0x8);
    athrs26_reg_write(PYTHON_ATF0_ADDR, reg0);

    while (busy && --i) {
        reg0 = athrs26_reg_read(PYTHON_ATF0_ADDR);
        busy = reg0 & 0x8;
        udelay(5);
    }

    if (0 == i) {
        return PYTHON_ERR;
    }

    full_vio = reg0 & 0x1000; 
    if (full_vio) {
        /* must clear AT_FULL_VOI bit */
        reg0 = 0x1000;
        athrs26_reg_write(PYTHON_ATF0_ADDR, reg0);

        if (ARL_LOAD_ENTRY == op) {
            return PYTHON_FULL;
        } else {
            return PYTHON_ERR;
        }
    }

    return PYTHON_OK;
}

static void
_python_fdb_fill_addr(mac_addr_t addr, uint32_t * reg0, uint32_t * reg1)
{
    *reg1 |= (addr.uc[0] << 24);
    *reg1 |= (addr.uc[1] << 16);
    *reg1 |= (addr.uc[2] << 8);
    *reg1 |= addr.uc[3];

    *reg0 |= (addr.uc[4] << 24);
    *reg0 |= (addr.uc[5] << 16);
}

static BOOL
_python_fdb_is_zeroaddr(mac_addr_t addr)
{
    uint32_t i;

    for (i = 0; i < 6; i++) {
        if (addr.uc[i]) {
            return FALSE;
        }
    }

    return TRUE;
}

uint32_t
python_fdb_add(mac_addr_t addr, uint32_t port_bmap, BOOL sa_drop)
{
    uint32_t reg0 = 0, reg1 = 0, reg2 = 0;

    if (TRUE == _python_fdb_is_zeroaddr(addr)) {
        return PYTHON_ERR;
    }

    _python_fdb_fill_addr(addr, &reg0, &reg1);

    reg2 |= (0xf << 16);
    if (TRUE == sa_drop) {
        reg2 |= (0x1 << 14);
    }
    reg2 |= (port_bmap & 0x3f);

    athrs26_reg_write(PYTHON_ATF0_ADDR, reg0);
    athrs26_reg_write(PYTHON_ATF1_ADDR, reg1);
    athrs26_reg_write(PYTHON_ATF2_ADDR, reg2);

    return _python_fdb_commit(ARL_LOAD_ENTRY);
}

uint32_t
python_fdb_del(mac_addr_t addr)
{
    uint32_t reg0 = 0, reg1 = 0, reg2 = 0;

    if (TRUE == _python_fdb_is_zeroaddr(addr)) {
        return PYTHON_ERR;
    }

    _python_fdb_fill_addr(addr, &reg0, &reg1);

    athrs26_reg_write(PYTHON_ATF0_ADDR, reg0);
    athrs26_reg_write(PYTHON_ATF1_ADDR, reg1);
    athrs26_reg_write(PYTHON_ATF2_ADDR, reg2);

    return _python_fdb_commit(ARL_PURGE_ENTRY);
}

uint32_t
python_fdb_flush(BOOL f_static)
{
    if (f_static) {
        return _python_fdb_commit(ARL_FLUSH_ALL);
    } else {
        return _python_fdb_commit(ARL_FLUSH_ALL_UNLOCK);
    }
}

uint32_t 
python_fdb_getentry(mac_addr_t addr)
{
    uint32_t reg0 = 0, reg1 = 0, reg2 = 0,ret = 0;
    int drop=0,valid=0,port= -1;

    if (TRUE == _python_fdb_is_zeroaddr(addr)) {
	printk("ret = %d .\n",ret);
        return 0;
    }

    _python_fdb_fill_addr(addr, &reg0, &reg1);

    athrs26_reg_write(PYTHON_ATF0_ADDR, reg0);
    athrs26_reg_write(PYTHON_ATF1_ADDR, reg1);
    athrs26_reg_write(PYTHON_ATF2_ADDR, reg2);

    ret = _python_fdb_commit(ARL_LOOKUP_ENTRY);
    
    if(ret != PYTHON_OK){
	printk("ret = %d .\n",ret);
	return 0;
    }

    reg0 = athrs26_reg_read(PYTHON_ATF0_ADDR);
    reg1 = athrs26_reg_read(PYTHON_ATF1_ADDR);
    reg2 = athrs26_reg_read(PYTHON_ATF2_ADDR);

    addr.uc[0] = reg1 >> 24;
    addr.uc[1] = (reg1 >> 16)&0xff;
    addr.uc[2] = (reg1 >> 8)&0xff;
    addr.uc[3] = reg1 & 0xff;

    addr.uc[4] = reg0 >> 24;
    addr.uc[5] = (reg0 >> 16)&0xff;    

    port = reg2&0x3f;
    drop = (reg2 >> 14)&1;
    valid = (reg2 >> 16)&0xf;

    printk("Lookup reg %8x %8x %8x mac %x.%x.%x.%x.%x.%x,port %d,drop %d,valid %d.\n",reg0,reg1,reg2,addr.uc[0],addr.uc[1],addr.uc[2],
		addr.uc[3],addr.uc[4],addr.uc[5],port,drop,valid);
    
    if(reg1==0&&(reg0&0xffff0000)==0&&port==0) 
	return 0;
    else
	return 1; 
}

/* We should write high 16bits to phy then low 16bits 
   The reason is that the trigger bit about vlan is located in the low 16bits.
   So we adjust the sequence of writing registers here. Normally we still use the 
   adverse sequence.*/

static void
athrs26_reg_write(unsigned int s26_addr, unsigned int s26_write_data)
{
    unsigned int addr_temp;
    unsigned int data;
    unsigned int phy_address, reg_address;


    addr_temp = (s26_addr &  0xfffffffc) >>2;
    data = addr_temp >> 7;

    phy_address = 0x1f;
    reg_address = 0x10;

    phy_reg_write(0,phy_address, reg_address, data);

    phy_address = (0x17 & ((addr_temp >> 4) | 0x10));

    reg_address = (((addr_temp << 1) & 0x1e) | 0x1);
    data = s26_write_data >> 16;
    phy_reg_write(0,phy_address, reg_address, data);
    
    reg_address = ((addr_temp << 1) & 0x1e);
    data = s26_write_data  & 0xffff;
    phy_reg_write(0,phy_address, reg_address, data);
}
/*============================================================================================= 
 * The following functions are designed for the ioctl calls.
 *============================================================================================*/

/* [python_ioctl_vlan_addports or python_ioctl_vlan_addports] port_id: bit0--->port0,bit1--->port1.*/
uint32_t
python_ioctl_vlan_addports(uint32_t vlan_id, uint32_t port_id)
{
    uint32_t port_bmap, reg0 = 0, reg1 = 0;
    uint32_t ret;

    PYTHON_VLAN_ID_CHECK(vlan_id);
    PYTHON_PORTS_ID_CHECK(port_id);

    ret = _python_vlan_read(vlan_id, &port_bmap);
    if (PYTHON_OK != ret) {
        /* the vlan entry has not been created */
	ret = python_vlan_create(vlan_id);
	if (PYTHON_OK != ret) return PYTHON_ERR;
    }

    port_bmap |= port_id;
    reg0 |= (vlan_id << 16);
    reg1 |= ((0x1 << 11) | port_bmap);
    athrs26_reg_write(PYTHON_VTF0_ADDR, reg0);
    athrs26_reg_write(PYTHON_VTF1_ADDR, reg1);

    ret = _python_vlan_commit(VLAN_LOAD_ENTRY);
    if (PYTHON_FULL == ret) {
        /* hardware will set full bit when update vlan entry */
        return PYTHON_OK;
    }

    return ret;

}

// port_id: bit0--->port0,bit1--->port1.
uint32_t
python_ioctl_vlan_delports(uint32_t vlan_id, uint32_t port_id)
{
    uint32_t port_bmap, reg0 = 0, reg1 = 0;
    uint32_t ret;

    PYTHON_VLAN_ID_CHECK(vlan_id);
    PYTHON_PORTS_ID_CHECK(port_id);

    ret = _python_vlan_read(vlan_id, &port_bmap);
    if (PYTHON_OK != ret) {
        /* the vlan entry has not been created */
	return PYTHON_ERR;
    }
    
    port_bmap &= (~port_id);
    reg0 |= (vlan_id << 16);
    reg1 |= ((0x1 << 11) | port_bmap);
    athrs26_reg_write(PYTHON_VTF0_ADDR, reg0);
    athrs26_reg_write(PYTHON_VTF1_ADDR, reg1);

    ret = _python_vlan_commit(VLAN_LOAD_ENTRY);
    if (PYTHON_FULL == ret) {
        /* hardware will set full bit when update vlan entry */
        return PYTHON_OK;
    }
   
    // Delete the vlan id if no ports exists in the vlan. 
    if(port_bmap == 0) 
    {
	ret = python_vlan_delete(vlan_id);
	if (PYTHON_OK != ret) return PYTHON_ERR;
    }

    return ret;

}

uint32_t
python_ioctl_set_igmp(uint32_t vlan_id, BOOL enable)
{
    uint32_t port_bmap;
    uint32_t ret;
    uint32_t i = 0;
    uint32_t port = 0;

    PYTHON_VLAN_ID_CHECK(vlan_id);

    ret = _python_vlan_read(vlan_id, &port_bmap);
    if (PYTHON_OK != ret) {
        /* the vlan entry has not been created */
        return PYTHON_ERR;
    }

    for(i=1;i<5;i++)
    {
        port = (port_bmap >>i)&0x1;
	if(port) 
	{	
		ret = python_port_igmps_status_set(i,enable);
		if (PYTHON_OK != ret) return PYTHON_ERR;
	}
    }    
    return PYTHON_OK;
}

uint32_t
python_ioctl_enable_vlan()
{
    int i;
    /* Enable ports [0-4](phy0-3 and cpu port0) */
    for(i=0;i<5;i++)
	python_port_1qmode_set(i,1);

    return PYTHON_OK;
}

uint32_t
python_ioctl_disable_vlan()
{
    int i;
    /* Disable ports [0-4](phy0-3 and cpu port0) */
    for(i=0;i<5;i++)
	python_port_1qmode_set(i,0);

    return PYTHON_OK;
}

void
python_clear_multi()
{
	athrs26_reg_write(0x2c, athrs26_reg_read(0x2c)&0xffc0ffff);	
}

void
python_set_multi()
{
	athrs26_reg_write(0x2c,athrs26_reg_read(0x2c)|0x3f0000);	
}






