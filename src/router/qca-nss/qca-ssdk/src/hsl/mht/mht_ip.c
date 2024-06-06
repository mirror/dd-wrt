/*
 * Copyright (c) 2012, 2016, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021,2023 Qualcomm Innovation Center, Inc. All rights reserved.
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


/**
 * @defgroup mht_ip MHT_IP
 * @{
 */

#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_port_prop.h"
#include "mht_ip.h"
#include "isisc_reg.h"
#include "mht_reg.h"


#define MHT_HOST_ENTRY_DATA0_ADDR              0x0e80
#define MHT_HOST_ENTRY_DATA7_ADDR              0x0e58

#define MHT_HOST_ENTRY_REG_NUM                 8

#define MHT_HOST_ENTRY_FLUSH                   1
#define MHT_HOST_ENTRY_ADD                     2
#define MHT_HOST_ENTRY_DEL                     3
#define MHT_HOST_ENTRY_NEXT                    4
#define MHT_HOST_ENTRY_SEARCH                  5

#define MHT_ENTRY_ARP                          3

#define MHT_INTF_MAC_ADDR_NUM                  8
#define MHT_INTF_MAC_TBL0_ADDR                 0x5a900
#define MHT_INTF_MAC_EDIT0_ADDR                0x02000

#define MHT_IP_COUTER_ADDR                     0x2b000

#define MHT_IP4_DEFAULT_ROUTE_TBL_ADDR                 0x004c4
#define MHT_IP6_DEFAULT_ROUTE_TBL_ADDR                 0x004e4
#define MHT_IP4_HOST_ROUTE_TBL0_ADDR                 0x5b000
#define MHT_IP6_HOST_ROUTE_TBL0_ADDR                 0x5b100

#if defined(IN_RFS)
extern aos_mutex_lock_t mht_nat_lock;
static a_uint32_t mht_mac_snap[SW_MAX_NR_DEV] = { 0 };
static fal_intf_mac_entry_t mht_intf_snap[SW_MAX_NR_DEV][MHT_INTF_MAC_ADDR_NUM];
#endif

static sw_error_t
_mht_ip_feature_check(a_uint32_t dev_id)
{
	sw_error_t rv;
	a_uint32_t entry = 0;

	HSL_REG_FIELD_GET(rv, dev_id, MASK_CTL, 0, DEVICE_ID,
			(a_uint8_t *) (&entry), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	switch (entry) {
		case MHT_DEVICE_ID:
			return SW_OK;
		default:
			return SW_NOT_SUPPORTED;
	}
}

#if defined(IN_RFS)
static void
_mht_ip_pt_learn_save(a_uint32_t dev_id, a_uint32_t * status)
{
	sw_error_t rv;
	a_uint32_t data = 0;

	HSL_REG_ENTRY_GET(rv, dev_id, ROUTER_PTCTRL2, 0,
			(a_uint8_t *) (&data), sizeof (a_uint32_t));
	if (SW_OK != rv)
	{
		return;
	}

	*status = (data & 0x7f7f);

	data &= 0xffff8080;
	HSL_REG_ENTRY_SET(rv, dev_id, ROUTER_PTCTRL2, 0,
			(a_uint8_t *) (&data), sizeof (a_uint32_t));
	return;
}

static void
_mht_ip_pt_learn_restore(a_uint32_t dev_id, a_uint32_t status)
{
	sw_error_t rv;
	a_uint32_t data = 0;

	HSL_REG_ENTRY_GET(rv, dev_id, ROUTER_PTCTRL2, 0,
			(a_uint8_t *) (&data), sizeof (a_uint32_t));
	if (SW_OK != rv)
	{
		return;
	}

	data &= 0xffff8080;
	data |= (status & 0x7f7f);

	HSL_REG_ENTRY_SET(rv, dev_id, ROUTER_PTCTRL2, 0,
			(a_uint8_t *) (&data), sizeof (a_uint32_t));
	return;
}

static sw_error_t
_mht_ip_counter_get(a_uint32_t dev_id, a_uint32_t cnt_id,
		a_uint32_t counter[2])
{
	sw_error_t rv;
	a_uint32_t i, addr;

	addr = MHT_IP_COUTER_ADDR + (cnt_id << 3);
	for (i = 0; i < 2; i++)
	{
		HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
				(a_uint8_t *) (&(counter[i])),
				sizeof (a_uint32_t));
		SW_RTN_ON_ERROR(rv);

		addr += 4;
	}

	return SW_OK;
}

static sw_error_t
_mht_host_entry_commit(a_uint32_t dev_id, a_uint32_t entry_type, a_uint32_t op)
{
	a_uint32_t busy = 1, i = 0x9000000, entry = 0, j, try_num;
	a_uint32_t learn_status = 0, data = 0;
	sw_error_t rv;

	while (busy && --i)
	{
		HSL_REG_ENTRY_GET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&entry),
				sizeof (a_uint32_t));
		SW_RTN_ON_ERROR(rv);
		//printk("IP first entry is 0x%x\r\n", entry);
		SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_BUSY, busy, entry);
	}

	if (i == 0)
	{
		printk("host entry busy!\n");
		return SW_BUSY;
	}


	/* hardware requirements, we should disable ARP learn at first */
	/* and maybe we should try several times... */
	_mht_ip_pt_learn_save(dev_id, &learn_status);

	//printk("data0=0x%x\n", data);
	if(learn_status) {
		aos_mdelay(800);
		HSL_REG_ENTRY_GET(rv, dev_id, ROUTER_PTCTRL2, 0,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	//printk("data1=0x%x\n", data);


	SW_SET_REG_BY_FIELD(HOST_ENTRY7, TBL_BUSY, 1, entry);
	SW_SET_REG_BY_FIELD(HOST_ENTRY7, TBL_SEL, entry_type, entry);
	SW_SET_REG_BY_FIELD(HOST_ENTRY7, ENTRY_FUNC, op, entry);

	HSL_REG_ENTRY_SET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&entry),
			sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	if (learn_status)
	{
		try_num = 300;
	}
	else
	{
		try_num = 1;
	}

	for (j = 0; j < try_num; j++)
	{
		busy = 1;
		i = 0x9000000;
		while (busy && --i)
		{
			HSL_REG_ENTRY_GET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&entry),
					sizeof (a_uint32_t));
			//printk("IP second entry is 0x%x\r\n", entry);
			if (SW_OK != rv)
			{
				_mht_ip_pt_learn_restore(dev_id, learn_status);
				return rv;
			}
			SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_BUSY, busy, entry);
		}

		if (i == 0)
		{
			_mht_ip_pt_learn_restore(dev_id, learn_status);
			return SW_BUSY;
		}

		/* hardware requirement, we should read again... */
		HSL_REG_ENTRY_GET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&entry),
				sizeof (a_uint32_t));
		if (SW_OK != rv)
		{
			_mht_ip_pt_learn_restore(dev_id, learn_status);
			return rv;
		}

		/* operation success...... */
		SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_STAUS, busy, entry);
		if (busy)
		{
			_mht_ip_pt_learn_restore(dev_id, learn_status);
			return SW_OK;
		}
	}

	_mht_ip_pt_learn_restore(dev_id, learn_status);
	if (MHT_HOST_ENTRY_NEXT == op)
	{
		return SW_NO_MORE;
	}
	else if (MHT_HOST_ENTRY_SEARCH == op)
	{
		return SW_NOT_FOUND;
	}
	else if (MHT_HOST_ENTRY_DEL == op)
	{
		return SW_NOT_FOUND;
	}
	else
	{
		return SW_FAIL;
	}
}

static sw_error_t
_mht_ip_intf_sw_to_hw(a_uint32_t dev_id, fal_host_entry_t * entry,
		a_uint32_t * hw_intf)
{
	sw_error_t rv;
	a_uint32_t addr, lvid, hvid, tbl[3] = {0}, i;
	a_uint32_t sw_intf = entry->intf_id;
	a_uint32_t vid_offset;

	for (i = 0; i < MHT_INTF_MAC_ADDR_NUM; i++)
	{
		if (mht_mac_snap[dev_id] & (0x1 << i))
		{
			addr = MHT_INTF_MAC_TBL0_ADDR + (i << 4) + 4;
			HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
					(a_uint8_t *) (&(tbl[1])),
					sizeof (a_uint32_t));
			SW_RTN_ON_ERROR(rv);

			addr = MHT_INTF_MAC_TBL0_ADDR + (i << 4) + 8;
			HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
					(a_uint8_t *) (&(tbl[2])),
					sizeof (a_uint32_t));
			SW_RTN_ON_ERROR(rv);

			SW_GET_FIELD_BY_REG(INTF_ADDR_ENTRY1, VID_HIGH0, hvid, tbl[1]);
			SW_GET_FIELD_BY_REG(INTF_ADDR_ENTRY2, VID_HIGH1, lvid, tbl[2]);
			hvid |= ((lvid & 0xff) << 4);

			SW_GET_FIELD_BY_REG(INTF_ADDR_ENTRY1, VID_LOW, lvid, tbl[1]);

			if ((lvid <= sw_intf) && (hvid >= sw_intf))
			{
				vid_offset = entry->expect_vid ? (entry->expect_vid - lvid) : (sw_intf - lvid);
				*hw_intf = (vid_offset << 3) | i;
				return SW_OK;
			}
		}
	}

	return SW_BAD_PARAM;
}

static sw_error_t
_mht_ip_intf_hw_to_sw(a_uint32_t dev_id, a_uint32_t hw_intf,
		a_uint32_t * sw_intf)
{
	sw_error_t rv;
	a_uint32_t addr, lvid, tbl = 0, i;

	i = hw_intf & 0x7;

	addr = MHT_INTF_MAC_TBL0_ADDR + (i << 4) + 4;
	HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
			(a_uint8_t *) (&tbl), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	SW_GET_FIELD_BY_REG(INTF_ADDR_ENTRY1, VID_LOW, lvid, tbl);
	*sw_intf = lvid + (hw_intf >> 3);

	return SW_OK;
}

static sw_error_t
_mht_host_sw_to_hw(a_uint32_t dev_id, fal_host_entry_t * entry,
		a_uint32_t reg[])
{
	sw_error_t rv;
	a_uint32_t data;

	if (FAL_IP_IP4_ADDR & entry->flags)
	{
		reg[0] = entry->ip4_addr;
		SW_SET_REG_BY_FIELD(HOST_ENTRY6, IP_VER, 0, reg[6]);
	}

	if (FAL_IP_IP6_ADDR & entry->flags)
	{
		reg[0] = entry->ip6_addr.ul[3];
		reg[1] = entry->ip6_addr.ul[2];
		reg[2] = entry->ip6_addr.ul[1];
		reg[3] = entry->ip6_addr.ul[0];
		SW_SET_REG_BY_FIELD(HOST_ENTRY6, IP_VER, 1, reg[6]);
	}

	SW_SET_REG_BY_FIELD(HOST_ENTRY4, MAC_ADDR2, entry->mac_addr.uc[2], reg[4]);
	SW_SET_REG_BY_FIELD(HOST_ENTRY4, MAC_ADDR3, entry->mac_addr.uc[3], reg[4]);
	SW_SET_REG_BY_FIELD(HOST_ENTRY4, MAC_ADDR4, entry->mac_addr.uc[4], reg[4]);
	SW_SET_REG_BY_FIELD(HOST_ENTRY4, MAC_ADDR5, entry->mac_addr.uc[5], reg[4]);
	SW_SET_REG_BY_FIELD(HOST_ENTRY5, MAC_ADDR0, entry->mac_addr.uc[0], reg[5]);
	SW_SET_REG_BY_FIELD(HOST_ENTRY5, MAC_ADDR1, entry->mac_addr.uc[1], reg[5]);

	rv = _mht_ip_intf_sw_to_hw(dev_id, entry/*was:->intf_id*/, &data);

	SW_RTN_ON_ERROR(rv);
	SW_SET_REG_BY_FIELD(HOST_ENTRY5, INTF_ID, data, reg[5]);

#if 0
	if (A_TRUE != hsl_port_prop_check(dev_id, entry->port_id, HSL_PP_INCL_CPU))
	{
		return SW_BAD_PARAM;
	}
#endif

	SW_SET_REG_BY_FIELD(HOST_ENTRY5, SRC_PORT, entry->port_id, reg[5]);

	if (FAL_IP_CPU_ADDR & entry->flags)
	{
		SW_SET_REG_BY_FIELD(HOST_ENTRY5, CPU_ADDR, 1, reg[5]);
	}

	SW_SET_REG_BY_FIELD(HOST_ENTRY6, AGE_FLAG, entry->status, reg[6]);
	SW_SET_REG_BY_FIELD(HOST_ENTRY6, VRF_ID, entry->vrf_id, reg[6]);
	SW_SET_REG_BY_FIELD(HOST_ENTRY6, LB_BIT, entry->lb_num, reg[6]);

	if ((A_TRUE == entry->mirror_en) && (FAL_MAC_FRWRD != entry->action))
	{
		return SW_NOT_SUPPORTED;
	}

	if (A_TRUE == entry->counter_en)
	{
		SW_SET_REG_BY_FIELD(HOST_ENTRY6, CNT_EN, 1, reg[6]);
		SW_SET_REG_BY_FIELD(HOST_ENTRY6, CNT_IDX, entry->counter_id, reg[6]);
	}

	if (FAL_MAC_DROP == entry->action)
	{
		SW_SET_REG_BY_FIELD(HOST_ENTRY5, SRC_PORT, 7, reg[5]);
		SW_SET_REG_BY_FIELD(HOST_ENTRY6, ACTION, 3, reg[6]);
	}
	else if (FAL_MAC_RDT_TO_CPU == entry->action)
	{
		SW_SET_REG_BY_FIELD(HOST_ENTRY6, ACTION, 1, reg[6]);
	}
	else if (FAL_MAC_CPY_TO_CPU == entry->action)
	{
		SW_SET_REG_BY_FIELD(HOST_ENTRY6, ACTION, 2, reg[6]);
	}
	else
	{
		if (A_TRUE == entry->mirror_en)
		{
			SW_SET_REG_BY_FIELD(HOST_ENTRY6, ACTION, 0, reg[6]);
		}
		else
		{
			SW_SET_REG_BY_FIELD(HOST_ENTRY6, ACTION, 3, reg[6]);
		}
	}

	return SW_OK;
}

static sw_error_t
_mht_host_hw_to_sw(a_uint32_t dev_id, a_uint32_t reg[],
                    fal_host_entry_t * entry)
{
	sw_error_t rv;
	a_uint32_t data = 0, cnt[2] = {0};

	SW_GET_FIELD_BY_REG(HOST_ENTRY6, IP_VER, data, reg[6]);
	if (data)
	{
		entry->ip6_addr.ul[0] = reg[3];
		entry->ip6_addr.ul[1] = reg[2];
		entry->ip6_addr.ul[2] = reg[1];
		entry->ip6_addr.ul[3] = reg[0];
		entry->flags |= FAL_IP_IP6_ADDR;
	}
	else
	{
		entry->ip4_addr = reg[0];
		entry->flags |= FAL_IP_IP4_ADDR;
	}

	SW_GET_FIELD_BY_REG(HOST_ENTRY4, MAC_ADDR2, entry->mac_addr.uc[2], reg[4]);
	SW_GET_FIELD_BY_REG(HOST_ENTRY4, MAC_ADDR3, entry->mac_addr.uc[3], reg[4]);
	SW_GET_FIELD_BY_REG(HOST_ENTRY4, MAC_ADDR4, entry->mac_addr.uc[4], reg[4]);
	SW_GET_FIELD_BY_REG(HOST_ENTRY4, MAC_ADDR5, entry->mac_addr.uc[5], reg[4]);
	SW_GET_FIELD_BY_REG(HOST_ENTRY5, MAC_ADDR0, entry->mac_addr.uc[0], reg[5]);
	SW_GET_FIELD_BY_REG(HOST_ENTRY5, MAC_ADDR1, entry->mac_addr.uc[1], reg[5]);

	SW_GET_FIELD_BY_REG(HOST_ENTRY5, INTF_ID, data, reg[5]);
	rv = _mht_ip_intf_hw_to_sw(dev_id, data, &(entry->intf_id));
	SW_RTN_ON_ERROR(rv);

	SW_GET_FIELD_BY_REG(HOST_ENTRY5, SRC_PORT, entry->port_id, reg[5]);

	SW_GET_FIELD_BY_REG(HOST_ENTRY5, CPU_ADDR, data, reg[5]);
	if (data)
	{
		entry->flags |= FAL_IP_CPU_ADDR;
	}

	SW_GET_FIELD_BY_REG(HOST_ENTRY6, AGE_FLAG, entry->status, reg[6]);
	SW_GET_FIELD_BY_REG(HOST_ENTRY6, VRF_ID, entry->vrf_id, reg[6]);
	SW_GET_FIELD_BY_REG(HOST_ENTRY6, LB_BIT, entry->lb_num, reg[6]);

	SW_GET_FIELD_BY_REG(HOST_ENTRY6, CNT_EN, data, reg[6]);
	if (data)
	{
		entry->counter_en = A_TRUE;
		SW_GET_FIELD_BY_REG(HOST_ENTRY6, CNT_IDX, entry->counter_id, reg[6]);

		rv = _mht_ip_counter_get(dev_id, entry->counter_id, cnt);
		SW_RTN_ON_ERROR(rv);

		entry->packet = cnt[0];
		entry->byte = cnt[1];
	}
	else
	{
		entry->counter_en = A_FALSE;
	}

	SW_GET_FIELD_BY_REG(HOST_ENTRY6, PPPOE_EN, data, reg[6]);
	if (data)
	{
		entry->pppoe_en = A_TRUE;
		SW_GET_FIELD_BY_REG(HOST_ENTRY6, PPPOE_IDX, data, reg[6]);
		entry->pppoe_id = data;
	}
	else
	{
		entry->pppoe_en = A_FALSE;
	}

	if (7 == entry->port_id)
	{
		entry->port_id = 0;
		entry->action = FAL_MAC_DROP;
	}
	else
	{
		SW_GET_FIELD_BY_REG(HOST_ENTRY6, ACTION, data, reg[6]);
		entry->action = FAL_MAC_FRWRD;
		if (0 == data)
		{
			entry->mirror_en = A_TRUE;
		}
		else if (1 == data)
		{
			entry->action = FAL_MAC_RDT_TO_CPU;
		}
		else if (2 == data)
		{
			entry->action = FAL_MAC_CPY_TO_CPU;
		}
	}

	return SW_OK;
}

static sw_error_t
_mht_host_down_to_hw(a_uint32_t dev_id, a_uint32_t reg[])
{
	sw_error_t rv;
	a_uint32_t i, addr;

	for (i = 0; i < MHT_HOST_ENTRY_REG_NUM; i++)
	{
		if((MHT_HOST_ENTRY_REG_NUM - 1) == i)
		{
			addr = MHT_HOST_ENTRY_DATA7_ADDR;
		}
		else
		{
			addr = MHT_HOST_ENTRY_DATA0_ADDR + (i << 2);
		}

		HSL_REG_ENTRY_GEN_SET(rv, dev_id, addr, sizeof (a_uint32_t),
				(a_uint8_t *) (&reg[i]), sizeof (a_uint32_t));
		SW_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

static sw_error_t
_mht_host_up_to_sw(a_uint32_t dev_id, a_uint32_t reg[])
{
	sw_error_t rv;
	a_uint32_t i, addr;

	for (i = 0; i < MHT_HOST_ENTRY_REG_NUM; i++)
	{
		if((MHT_HOST_ENTRY_REG_NUM - 1) == i)
		{
			addr = MHT_HOST_ENTRY_DATA7_ADDR;
		}
		else
		{
			addr = MHT_HOST_ENTRY_DATA0_ADDR + (i << 2);
		}
		HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
				(a_uint8_t *) (&reg[i]), sizeof (a_uint32_t));
		SW_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

static sw_error_t
_mht_ip_host_add(a_uint32_t dev_id, fal_host_entry_t * entry)
{
	sw_error_t rv;
	a_uint32_t reg[MHT_HOST_ENTRY_REG_NUM] = { 0 };

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	rv = _mht_host_sw_to_hw(dev_id, entry, reg);
	SW_RTN_ON_ERROR(rv);

	aos_mutex_lock(&mht_nat_lock);
	rv = _mht_host_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

	rv = _mht_host_entry_commit(dev_id, MHT_ENTRY_ARP, MHT_HOST_ENTRY_ADD);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

	HSL_REG_ENTRY_GET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&reg[7]),
			sizeof (a_uint32_t));
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}
	aos_mutex_unlock(&mht_nat_lock);
	SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX, entry->entry_id, reg[7]);

	return SW_OK;
}

static sw_error_t
_mht_ip_host_del(a_uint32_t dev_id, a_uint32_t del_mode,
		fal_host_entry_t * entry)
{
	sw_error_t rv;
	a_uint32_t data, reg[MHT_HOST_ENTRY_REG_NUM] = { 0 }, op = MHT_HOST_ENTRY_FLUSH;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (FAL_IP_ENTRY_ID_EN & del_mode)
	{
		return SW_NOT_SUPPORTED;
	}

	if (FAL_IP_ENTRY_IPADDR_EN & del_mode)
	{
		op = MHT_HOST_ENTRY_DEL;
		if (FAL_IP_IP4_ADDR & entry->flags)
		{
			reg[0] = entry->ip4_addr;
		}

		if (FAL_IP_IP6_ADDR & entry->flags)
		{
			reg[0] = entry->ip6_addr.ul[3];
			reg[1] = entry->ip6_addr.ul[2];
			reg[2] = entry->ip6_addr.ul[1];
			reg[3] = entry->ip6_addr.ul[0];
			SW_SET_REG_BY_FIELD(HOST_ENTRY6, IP_VER, 1, reg[6]);
		}
	}

	if (FAL_IP_ENTRY_INTF_EN & del_mode)
	{
		rv = _mht_ip_intf_sw_to_hw(dev_id, entry/*was:->intf_id*/, &data);
		SW_RTN_ON_ERROR(rv);

		SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_VID, 1, reg[7]);
		SW_SET_REG_BY_FIELD(HOST_ENTRY5, INTF_ID, data, reg[5]);
	}

	if (FAL_IP_ENTRY_PORT_EN & del_mode)
	{
		SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_SP, 1, reg[7]);
		SW_SET_REG_BY_FIELD(HOST_ENTRY5, SRC_PORT, entry->port_id, reg[5]);
	}

	if (FAL_IP_ENTRY_STATUS_EN & del_mode)
	{
		SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_STATUS, 1, reg[7]);
		SW_SET_REG_BY_FIELD(HOST_ENTRY6, AGE_FLAG, entry->status, reg[6]);
	}
	aos_mutex_lock(&mht_nat_lock);
	rv = _mht_host_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

	rv = _mht_host_entry_commit(dev_id, MHT_ENTRY_ARP, op);
	aos_mutex_unlock(&mht_nat_lock);

	return rv;
}

static sw_error_t
_mht_ip_host_get(a_uint32_t dev_id, a_uint32_t get_mode,
		fal_host_entry_t * entry)
{
	sw_error_t rv;
	a_uint32_t reg[MHT_HOST_ENTRY_REG_NUM] = { 0 };

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (FAL_IP_ENTRY_IPADDR_EN != get_mode)
	{
		return SW_NOT_SUPPORTED;
	}

	if (FAL_IP_IP4_ADDR & entry->flags)
	{
		reg[0] = entry->ip4_addr;
	}
	else
	{
		reg[0] = entry->ip6_addr.ul[3];
		reg[1] = entry->ip6_addr.ul[2];
		reg[2] = entry->ip6_addr.ul[1];
		reg[3] = entry->ip6_addr.ul[0];
		SW_SET_REG_BY_FIELD(HOST_ENTRY6, IP_VER, 1, reg[6]);
	}
	aos_mutex_lock(&mht_nat_lock);
	rv = _mht_host_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

	rv = _mht_host_entry_commit(dev_id, MHT_ENTRY_ARP,
			MHT_HOST_ENTRY_SEARCH);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

	rv = _mht_host_up_to_sw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

	aos_mem_zero(entry, sizeof (fal_host_entry_t));

	rv = _mht_host_hw_to_sw(dev_id, reg, entry);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

	if (!(entry->status))
	{
		aos_mutex_unlock(&mht_nat_lock);
		return SW_NOT_FOUND;
	}

	HSL_REG_ENTRY_GET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&reg[7]),
			sizeof (a_uint32_t));
	aos_mutex_unlock(&mht_nat_lock);
	SW_RTN_ON_ERROR(rv);

	SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX, entry->entry_id, reg[7]);
	return SW_OK;
}

static sw_error_t
_mht_ip_intf_entry_add(a_uint32_t dev_id, fal_intf_mac_entry_t * entry)
{
	sw_error_t rv;
	a_uint32_t i, j, found = 0, addr, tbl[3] = { 0 };
	fal_intf_mac_entry_t * intf_entry = NULL;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	for (i = 0; i < MHT_INTF_MAC_ADDR_NUM; i++)
	{
		if (mht_mac_snap[dev_id] & (0x1 << i))
		{
			intf_entry = &(mht_intf_snap[dev_id][i]);
			if ((entry->vid_low == intf_entry->vid_low)
					&& (entry->vid_high == intf_entry->vid_high))
			{
				/* all same, return OK directly */
				if (!aos_mem_cmp(intf_entry, entry, sizeof(fal_intf_mac_entry_t)))
				{
					return SW_OK;
				}
				else
				{
					/* update entry */
					found = 1;
					break;
				}
			}
			else
			{
				/* entry VID cross border, not support */
				if ((entry->vid_low >= intf_entry->vid_low) && (entry->vid_low <= intf_entry->vid_high))
				{
					return SW_BAD_PARAM;
				}

				/* entry VID cross border, not support */
				if ((entry->vid_high >= intf_entry->vid_low) && (entry->vid_low <= intf_entry->vid_high))
				{
					return SW_BAD_PARAM;
				}
			}
		}
	}

	if (!found)
	{
		for (i = 0; i < MHT_INTF_MAC_ADDR_NUM; i++)
		{
			if (!(mht_mac_snap[dev_id] & (0x1 << i)))
			{
				intf_entry = &(mht_intf_snap[dev_id][i]);
				break;
			}
		}
	}

	if (MHT_INTF_MAC_ADDR_NUM == i)
	{
		return SW_NO_RESOURCE;
	}

	if ((A_FALSE == entry->ip4_route) && (A_FALSE == entry->ip6_route))
	{
		return SW_NOT_SUPPORTED;
	}

	if (512 <= (entry->vid_high - entry->vid_low))
	{
		return SW_BAD_PARAM;
	}

	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY0, MAC_ADDR2, entry->mac_addr.uc[2],
			tbl[0]);
	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY0, MAC_ADDR3, entry->mac_addr.uc[3],
			tbl[0]);
	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY0, MAC_ADDR4, entry->mac_addr.uc[4],
			tbl[0]);
	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY0, MAC_ADDR5, entry->mac_addr.uc[5],
			tbl[0]);
	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY1, MAC_ADDR0, entry->mac_addr.uc[0],
			tbl[1]);
	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY1, MAC_ADDR1, entry->mac_addr.uc[1],
			tbl[1]);

	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY1, VID_LOW, entry->vid_low, tbl[1]);
	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY1, VID_HIGH0, (entry->vid_high & 0xf),
			tbl[1]);
	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY2, VID_HIGH1, (entry->vid_high >> 4),
			tbl[2]);

	if (A_TRUE == entry->ip4_route)
	{
		SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY2, IP4_ROUTE, 1, tbl[2]);
	}

	if (A_TRUE == entry->ip6_route)
	{
		SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY2, IP6_ROUTE, 1, tbl[2]);
	}

	SW_SET_REG_BY_FIELD(INTF_ADDR_ENTRY2, VRF_ID, entry->vrf_id, tbl[2]);

	for (j = 0; j < 2; j++)
	{
		addr = MHT_INTF_MAC_EDIT0_ADDR + (i << 4) + (j << 2);
		HSL_REG_ENTRY_GEN_SET(rv, dev_id, addr, sizeof (a_uint32_t),
				(a_uint8_t *) (&(tbl[j])), sizeof (a_uint32_t));
		SW_RTN_ON_ERROR(rv);
	}

	for (j = 0; j < 3; j++)
	{
		addr = MHT_INTF_MAC_TBL0_ADDR + (i << 4) + (j << 2);
		HSL_REG_ENTRY_GEN_SET(rv, dev_id, addr, sizeof (a_uint32_t),
				(a_uint8_t *) (&(tbl[j])), sizeof (a_uint32_t));
		SW_RTN_ON_ERROR(rv);
	}

	mht_mac_snap[dev_id] |= (0x1 << i);
	*intf_entry = *entry;
	entry->entry_id = i;
	return SW_OK;
}
#endif

#define MHT_WCMP_ENTRY_MAX_ID    3
#define MHT_WCMP_HASH_MAX_NUM    16
#define MHT_IP_ENTRY_MAX_ID      127

#define MHT_WCMP_HASH_TBL_ADDR   0x0e10
#define MHT_WCMP_NHOP_TBL_ADDR   0x0e20

static sw_error_t
_mht_ip_wcmp_entry_set(a_uint32_t dev_id, a_uint32_t wcmp_id, fal_ip_wcmp_t * wcmp)
{
	sw_error_t rv;
	a_uint32_t i, j, addr, data;
	a_uint8_t  idx, ptr[4] = { 0 }, pos[16] = { 0 };

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (MHT_WCMP_ENTRY_MAX_ID < wcmp_id)
	{
		return SW_BAD_PARAM;
	}

	if (MHT_WCMP_HASH_MAX_NUM < wcmp->nh_nr)
	{
		return SW_BAD_PARAM;
	}

	for (i = 0; i < wcmp->nh_nr; i++)
	{
		if (MHT_IP_ENTRY_MAX_ID < wcmp->nh_id[i])
		{
			return SW_BAD_PARAM;
		}

		idx = 4;
		for (j = 0; j < 4; j++)
		{
			if (ptr[j] & 0x80)
			{
				if ((ptr[j] & 0x7f) == wcmp->nh_id[i])
				{
					idx = j;
					break;
				}
			}
			else
			{
				idx = j;
			}
		}

		if (4 == idx)
		{
			return SW_BAD_PARAM;
		}
		else
		{
			ptr[idx] = (wcmp->nh_id[i] & 0x7f) | 0x80;
			pos[i]   = idx;
		}
	}

	data = 0;
	for (j = 0; j < 4; j++)
	{
		data |= (ptr[j] << (j << 3));
	}

	addr = MHT_WCMP_NHOP_TBL_ADDR + (wcmp_id << 2);
	HSL_REG_ENTRY_GEN_SET(rv, dev_id, addr, sizeof (a_uint32_t),
			(a_uint8_t *) (&data), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	data = 0;
	for (j = 0; j < 16; j++)
	{
		data |= (pos[j] << (j << 1));
	}

	addr = MHT_WCMP_HASH_TBL_ADDR + (wcmp_id << 2);
	HSL_REG_ENTRY_GEN_SET(rv, dev_id, addr, sizeof (a_uint32_t),
			(a_uint8_t *) (&data), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	return SW_OK;
}

static sw_error_t
_mht_ip_wcmp_entry_get(a_uint32_t dev_id, a_uint32_t wcmp_id, fal_ip_wcmp_t * wcmp)
{
	sw_error_t rv;
	a_uint32_t i, addr, data = 0;
	a_uint8_t  ptr[4] = { 0 }, pos[16] = { 0 };

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (MHT_WCMP_ENTRY_MAX_ID < wcmp_id)
	{
		return SW_BAD_PARAM;
	}

	wcmp->nh_nr = MHT_WCMP_HASH_MAX_NUM;

	addr = MHT_WCMP_NHOP_TBL_ADDR + (wcmp_id << 2);
	HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
			(a_uint8_t *) (&data), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	for (i = 0; i < 4; i++)
	{
		ptr[i] = (data >> (i << 3)) & 0x7f;
	}

	addr = MHT_WCMP_HASH_TBL_ADDR + (wcmp_id << 2);
	HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
			(a_uint8_t *) (&data), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	for (i = 0; i < 16; i++)
	{
		pos[i] = (data >> (i << 1)) & 0x3;
	}

	for (i = 0; i < 16; i++)
	{
		wcmp->nh_id[i] = ptr[pos[i]];
	}

	return SW_OK;
}

#define MHT_VRF_ENTRY_MAX_ID    7
#define MHT_VRF_ENTRY_TBL_ADDR   0x0484
#define MHT_VRF_ENTRY_MASK_ADDR   0x0488

static sw_error_t
_mht_ip_vrf_base_addr_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t addr)
{
	sw_error_t rv;
	a_uint32_t reg_addr;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (MHT_VRF_ENTRY_MAX_ID < vrf_id)
	{
		return SW_BAD_PARAM;
	}

	reg_addr = MHT_VRF_ENTRY_TBL_ADDR + (vrf_id << 3);
	HSL_REG_ENTRY_GEN_SET(rv, dev_id, reg_addr, sizeof (a_uint32_t),
			(a_uint8_t *) (&addr), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	return rv;
}

static sw_error_t
_mht_ip_vrf_base_addr_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t * addr)
{
	sw_error_t rv;
	a_uint32_t reg_addr;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (MHT_VRF_ENTRY_MAX_ID < vrf_id)
	{
		return SW_BAD_PARAM;
	}

	reg_addr = MHT_VRF_ENTRY_TBL_ADDR + (vrf_id << 3);
	HSL_REG_ENTRY_GEN_GET(rv, dev_id, reg_addr, sizeof (a_uint32_t),
			(a_uint8_t *) (addr), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	return SW_OK;
}

static sw_error_t
_mht_ip_vrf_base_mask_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t addr)
{
	sw_error_t rv;
	a_uint32_t reg_addr;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (MHT_VRF_ENTRY_MAX_ID < vrf_id)
	{
		return SW_BAD_PARAM;
	}

	reg_addr = MHT_VRF_ENTRY_MASK_ADDR + (vrf_id << 3);
	HSL_REG_ENTRY_GEN_SET(rv, dev_id, reg_addr, sizeof (a_uint32_t),
			(a_uint8_t *) (&addr), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	return rv;
}

static sw_error_t
_mht_ip_vrf_base_mask_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t * addr)
{
	sw_error_t rv;
	a_uint32_t reg_addr;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (MHT_VRF_ENTRY_MAX_ID < vrf_id)
	{
		return SW_BAD_PARAM;
	}

	reg_addr = MHT_VRF_ENTRY_MASK_ADDR + (vrf_id << 3);
	HSL_REG_ENTRY_GEN_GET(rv, dev_id, reg_addr, sizeof (a_uint32_t),
			(a_uint8_t *) (addr), sizeof (a_uint32_t));
	SW_RTN_ON_ERROR(rv);

	return SW_OK;
}

static sw_error_t
_mht_ip_default_route_set(a_uint32_t dev_id, a_uint32_t droute_id, fal_default_route_t * entry)
{
	sw_error_t rv;
	a_uint32_t data = 0;
	a_uint32_t addr;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (entry->ip_version == FAL_ADDR_IPV4)
	{
		SW_SET_REG_BY_FIELD(IP4_DEFAULT_ROUTE_ENTRY, VALID, entry->valid, data);
		SW_SET_REG_BY_FIELD(IP4_DEFAULT_ROUTE_ENTRY, VRF, entry->vrf_id, data);
		SW_SET_REG_BY_FIELD(IP4_DEFAULT_ROUTE_ENTRY, ARP_WCMP_TYPE, entry->droute_type, data);
		SW_SET_REG_BY_FIELD(IP4_DEFAULT_ROUTE_ENTRY, ARP_WCMP_INDEX, entry->index, data);

		addr = MHT_IP4_DEFAULT_ROUTE_TBL_ADDR + (droute_id << 2);
		HSL_REG_ENTRY_GEN_SET(rv, dev_id, addr, sizeof (a_uint32_t),
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else
	{
		SW_SET_REG_BY_FIELD(IP6_DEFAULT_ROUTE_ENTRY, VALID, entry->valid, data);
		SW_SET_REG_BY_FIELD(IP6_DEFAULT_ROUTE_ENTRY, VRF, entry->vrf_id, data);
		SW_SET_REG_BY_FIELD(IP6_DEFAULT_ROUTE_ENTRY, ARP_WCMP_TYPE, entry->droute_type, data);
		SW_SET_REG_BY_FIELD(IP6_DEFAULT_ROUTE_ENTRY, ARP_WCMP_INDEX, entry->index, data);

		addr = MHT_IP6_DEFAULT_ROUTE_TBL_ADDR + (droute_id << 2);
		HSL_REG_ENTRY_GEN_SET(rv, dev_id, addr, sizeof (a_uint32_t),
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}

	return rv;
}

static sw_error_t
_mht_ip_default_route_get(a_uint32_t dev_id, a_uint32_t droute_id, fal_default_route_t * entry)
{
	sw_error_t rv;
	a_uint32_t data = 0;
	a_uint32_t addr;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (entry->ip_version == FAL_ADDR_IPV4)
	{
		addr = MHT_IP4_DEFAULT_ROUTE_TBL_ADDR + (droute_id << 2);
		HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
				(a_uint8_t *) (&data), sizeof (a_uint32_t));

		SW_GET_FIELD_BY_REG(IP4_DEFAULT_ROUTE_ENTRY, VALID, entry->valid, data);
		SW_GET_FIELD_BY_REG(IP4_DEFAULT_ROUTE_ENTRY, VRF, entry->vrf_id, data);
		SW_GET_FIELD_BY_REG(IP4_DEFAULT_ROUTE_ENTRY, ARP_WCMP_TYPE, entry->droute_type, data);
		SW_GET_FIELD_BY_REG(IP4_DEFAULT_ROUTE_ENTRY, ARP_WCMP_INDEX, entry->index, data);
	}
	else
	{
		addr = MHT_IP6_DEFAULT_ROUTE_TBL_ADDR + (droute_id << 2);
		HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
				(a_uint8_t *) (&data), sizeof (a_uint32_t));

		SW_GET_FIELD_BY_REG(IP6_DEFAULT_ROUTE_ENTRY, VALID, entry->valid, data);
		SW_GET_FIELD_BY_REG(IP6_DEFAULT_ROUTE_ENTRY, VRF, entry->vrf_id, data);
		SW_GET_FIELD_BY_REG(IP6_DEFAULT_ROUTE_ENTRY, ARP_WCMP_TYPE, entry->droute_type, data);
		SW_GET_FIELD_BY_REG(IP6_DEFAULT_ROUTE_ENTRY, ARP_WCMP_INDEX, entry->index, data);
	}

	return SW_OK;
}

static sw_error_t
_mht_ip_host_route_set(a_uint32_t dev_id, a_uint32_t hroute_id, fal_host_route_t * entry)
{
	sw_error_t rv;
	a_uint32_t j, addr, tbl[5] = { 0 };
	a_uint32_t addr0_low, addr0_high, addr1_low, addr1_high,
		   addr2_low, addr2_high, addr3_low, addr3_high;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (entry->ip_version == FAL_ADDR_IPV4)
	{
		addr0_low = entry->route_addr.ip4_addr & 0x7ffffff;
		addr0_high = entry->route_addr.ip4_addr >> 27;
		SW_SET_REG_BY_FIELD(IP4_HOST_ROUTE_ENTRY0, PREFIX_LENGTH, entry->prefix_length,
				tbl[0]);
		SW_SET_REG_BY_FIELD(IP4_HOST_ROUTE_ENTRY0, IP4_ADDRL, addr0_low, tbl[0]);
		SW_SET_REG_BY_FIELD(IP4_HOST_ROUTE_ENTRY1, IP4_ADDRH, addr0_high, tbl[1]);
		SW_SET_REG_BY_FIELD(IP4_HOST_ROUTE_ENTRY1, VALID, entry->valid,
				tbl[1]);
		SW_SET_REG_BY_FIELD(IP4_HOST_ROUTE_ENTRY1, VRF, entry->vrf_id,
				tbl[1]);
		for (j = 0; j < 2; j++)
		{
			addr = MHT_IP4_HOST_ROUTE_TBL0_ADDR + (hroute_id << 4) + (j << 2);
			HSL_REG_ENTRY_GEN_SET(rv, dev_id, addr, sizeof (a_uint32_t),
					(a_uint8_t *) (&(tbl[j])), sizeof (a_uint32_t));
			SW_RTN_ON_ERROR(rv);
		}
	}
	else
	{
		addr0_low = entry->route_addr.ip6_addr.ul[3] & 0x1ffffff;
		addr0_high = (entry->route_addr.ip6_addr.ul[3] >> 25) & 0x7f;
		addr1_low = entry->route_addr.ip6_addr.ul[2] & 0x1ffffff;
		addr1_high = (entry->route_addr.ip6_addr.ul[2] >> 25) & 0x7f;
		addr2_low = entry->route_addr.ip6_addr.ul[1] & 0x1ffffff;
		addr2_high = (entry->route_addr.ip6_addr.ul[1] >> 25) & 0x7f;
		addr3_low = entry->route_addr.ip6_addr.ul[0] & 0x1ffffff;
		addr3_high = (entry->route_addr.ip6_addr.ul[0] >> 25) & 0x7f;

		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY0, IP6_ADDR0L, addr0_low, tbl[0]);
		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY0, PREFIX_LENGTH, entry->prefix_length, tbl[0]);

		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY1, IP6_ADDR1L, addr1_low, tbl[1]);
		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY1, IP6_ADDR0H, addr0_high, tbl[1]);

		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY2, IP6_ADDR2L, addr2_low, tbl[2]);
		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY2, IP6_ADDR1H, addr1_high, tbl[2]);

		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY3, IP6_ADDR3L, addr3_low, tbl[3]);
		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY3, IP6_ADDR2H, addr2_high, tbl[3]);

		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY4, VALID, entry->valid, tbl[4]);
		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY4, VRF, entry->vrf_id, tbl[4]);
		SW_SET_REG_BY_FIELD(IP6_HOST_ROUTE_ENTRY4, IP6_ADDR3H, addr3_high, tbl[4]);

		for (j = 0; j < 5; j++)
		{
			addr = MHT_IP6_HOST_ROUTE_TBL0_ADDR + (hroute_id << 5) + (j << 2);
			HSL_REG_ENTRY_GEN_SET(rv, dev_id, addr, sizeof (a_uint32_t),
					(a_uint8_t *) (&(tbl[j])), sizeof (a_uint32_t));
			SW_RTN_ON_ERROR(rv);
		}    
	}

	return SW_OK;
}

static sw_error_t
_mht_ip_host_route_get(a_uint32_t dev_id, a_uint32_t hroute_id, fal_host_route_t * entry)
{
	sw_error_t rv;
	a_uint32_t j, addr, tbl[5] = { 0 };
	a_uint32_t addr0_low, addr0_high, addr1_low, addr1_high,
		   addr2_low, addr2_high, addr3_low, addr3_high;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (entry->ip_version == FAL_ADDR_IPV4)
	{
		for (j = 0; j < 2; j++)
		{
			addr = MHT_IP4_HOST_ROUTE_TBL0_ADDR + (hroute_id << 4) + (j << 2);
			HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
					(a_uint8_t *) (&(tbl[j])), sizeof (a_uint32_t));
			SW_RTN_ON_ERROR(rv);
		}
		SW_GET_FIELD_BY_REG(IP4_HOST_ROUTE_ENTRY0, PREFIX_LENGTH, entry->prefix_length,
				tbl[0]);
		SW_GET_FIELD_BY_REG(IP4_HOST_ROUTE_ENTRY0, IP4_ADDRL, addr0_low,
				tbl[0]);
		SW_GET_FIELD_BY_REG(IP4_HOST_ROUTE_ENTRY1, IP4_ADDRH, addr0_high,
				tbl[1]);
		SW_GET_FIELD_BY_REG(IP4_HOST_ROUTE_ENTRY1, VALID, entry->valid,
				tbl[1]);
		SW_GET_FIELD_BY_REG(IP4_HOST_ROUTE_ENTRY1, VRF, entry->vrf_id,
				tbl[1]);
		entry->route_addr.ip4_addr = addr0_low | (addr0_high << 27);
	}
	else
	{
		for (j = 0; j < 5; j++)
		{
			addr = MHT_IP6_HOST_ROUTE_TBL0_ADDR + (hroute_id << 5) + (j << 2);
			HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
					(a_uint8_t *) (&(tbl[j])), sizeof (a_uint32_t));
			SW_RTN_ON_ERROR(rv);
		}

		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY0, IP6_ADDR0L, addr0_low, tbl[0]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY0, PREFIX_LENGTH, entry->prefix_length, tbl[0]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY1, IP6_ADDR1L, addr1_low, tbl[1]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY1, IP6_ADDR0H, addr0_high, tbl[1]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY2, IP6_ADDR2L, addr2_low, tbl[2]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY2, IP6_ADDR1H, addr1_high, tbl[2]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY3, IP6_ADDR3L, addr3_low, tbl[3]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY3, IP6_ADDR2H, addr2_high, tbl[3]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY4, VRF, entry->vrf_id, tbl[4]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY4, IP6_ADDR3H, addr3_high, tbl[4]);
		SW_GET_FIELD_BY_REG(IP6_HOST_ROUTE_ENTRY4, VALID, entry->valid, tbl[4]);
		entry->route_addr.ip6_addr.ul[3] = (addr0_high << 25) | addr0_low;
		entry->route_addr.ip6_addr.ul[2] = (addr1_high << 25) | addr1_low;
		entry->route_addr.ip6_addr.ul[1] = (addr2_high << 25) | addr2_low;
		entry->route_addr.ip6_addr.ul[0] = (addr3_high << 25) | addr3_low;
	}

	return SW_OK;
}

#if defined(IN_RFS)

#define RFS_ADD_OP  1
#define RFS_DEL_OP  2
static sw_error_t
_mht_ip_rfs_ip4_update(
		a_uint32_t dev_id,
		fal_host_entry_t *entry,
		fal_ip4_rfs_t * rfs,
		char op)
{
	fal_host_entry_t tmp = *entry;

	_mht_ip_host_del(dev_id, FAL_IP_ENTRY_IPADDR_EN, entry);
	if(RFS_ADD_OP == op) {
		tmp.lb_num = rfs->load_balance | 0x4;
		tmp.status = 0x7;
	}
	else {
		tmp.lb_num = 0;
		tmp.status = 0x6;
	}
	return _mht_ip_host_add(dev_id, &tmp);
}

static sw_error_t
_mht_ip_rfs_ip6_update(
		a_uint32_t dev_id,
		fal_host_entry_t *entry,
		fal_ip6_rfs_t * rfs,
		char op)
{
	fal_host_entry_t tmp = *entry;

	_mht_ip_host_del(dev_id, FAL_IP_ENTRY_IPADDR_EN, entry);
	if(RFS_ADD_OP == op) {
		tmp.lb_num = rfs->load_balance | 0x4;
		tmp.status = 0x7;
	}
	else {
		tmp.lb_num = 0;
		tmp.status = 0x6;
	}
	return _mht_ip_host_add(dev_id, &tmp);
}

static sw_error_t
_mht_ip_rfs_ip4_set(a_uint32_t dev_id, fal_ip4_rfs_t * rfs)
{
	fal_host_entry_t entry;

	memset(&entry, 0, sizeof(entry));
	entry.flags = FAL_IP_IP4_ADDR;
	entry.ip4_addr = rfs->ip4_addr;
	if(SW_OK == _mht_ip_host_get(dev_id, 0x10, &entry)) {
		return _mht_ip_rfs_ip4_update(dev_id, &entry, rfs, RFS_ADD_OP);
	}
	/*add a new one*/
	entry.status = 0x7;
	entry.flags = FAL_IP_IP4_ADDR;
	entry.ip4_addr = rfs->ip4_addr;
	memcpy(&entry.mac_addr, &rfs->mac_addr, 6);
	entry.intf_id = rfs->vid;
	entry.port_id = 0;
	entry.lb_num = rfs->load_balance | 0x4;
	entry.action = FAL_MAC_RDT_TO_CPU;
	return _mht_ip_host_add(dev_id, &entry);
}

static sw_error_t
_mht_ip_rfs_ip6_set(a_uint32_t dev_id, fal_ip6_rfs_t * rfs)
{
	fal_host_entry_t entry;

	memset(&entry, 0, sizeof(entry));
	entry.flags = FAL_IP_IP6_ADDR;
	entry.ip6_addr = rfs->ip6_addr;
	if(SW_OK == _mht_ip_host_get(dev_id, 0x10, &entry)) {
		return _mht_ip_rfs_ip6_update(dev_id, &entry, rfs, RFS_ADD_OP);
	}
	/*add a new one*/
	entry.status = 0x7;
	entry.flags = FAL_IP_IP6_ADDR;
	entry.ip6_addr = rfs->ip6_addr;
	memcpy(&entry.mac_addr, &rfs->mac_addr, 6);
	entry.intf_id = rfs->vid;
	entry.port_id = 0;
	entry.lb_num = rfs->load_balance | 0x4;
	entry.action = FAL_MAC_RDT_TO_CPU;
	return _mht_ip_host_add(dev_id, &entry);
}

static sw_error_t
_mht_ip_rfs_ip4_del(a_uint32_t dev_id, fal_ip4_rfs_t * rfs)
{
	fal_host_entry_t entry;
	sw_error_t ret;

	memset(&entry, 0, sizeof(entry));
	entry.flags = FAL_IP_IP4_ADDR;
	entry.ip4_addr = rfs->ip4_addr;
	if(SW_OK == (ret = _mht_ip_host_get(dev_id, 0x10, &entry))) {
		return _mht_ip_rfs_ip4_update(dev_id, &entry, rfs, RFS_DEL_OP);
	}
	return ret;
}

static sw_error_t
_mht_ip_rfs_ip6_del(a_uint32_t dev_id, fal_ip6_rfs_t * rfs)
{
	fal_host_entry_t entry;
	sw_error_t ret;

	memset(&entry, 0, sizeof(entry));
	entry.flags = FAL_IP_IP6_ADDR;
	entry.ip6_addr = rfs->ip6_addr;
	if(SW_OK == (ret = _mht_ip_host_get(dev_id, 0x10, &entry))) {
		return _mht_ip_rfs_ip6_update(dev_id, &entry, rfs, RFS_DEL_OP);
	}
	return ret;
}
#endif

static sw_error_t
_mht_default_flow_cmd_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_flow_type_t type, fal_default_flow_cmd_t cmd)
{
	sw_error_t rv;
	a_uint32_t data;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (FAL_DEFAULT_FLOW_FORWARD == cmd)
	{
		data = 0;
	}
	else if (FAL_DEFAULT_FLOW_DROP == cmd)
	{
		data = 1;
	}
	else if (FAL_DEFAULT_FLOW_RDT_TO_CPU == cmd)
	{
		data = 2;
	}
	else if (FAL_DEFAULT_FLOW_ADMIT_ALL == cmd)
	{
		data = 3;
	}
	else
	{
		return SW_NOT_SUPPORTED;
	}

	if (FAL_FLOW_LAN_TO_LAN == type)
	{
		HSL_REG_FIELD_SET(rv, dev_id, FlOW_CMD_CTL, vrf_id, LAN_2_LAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_WAN_TO_LAN == type)
	{
		HSL_REG_FIELD_SET(rv, dev_id, FlOW_CMD_CTL, vrf_id, WAN_2_LAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_LAN_TO_WAN == type)
	{
		HSL_REG_FIELD_SET(rv, dev_id, FlOW_CMD_CTL, vrf_id, LAN_2_WAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_WAN_TO_WAN == type)
	{
		HSL_REG_FIELD_SET(rv, dev_id, FlOW_CMD_CTL, vrf_id, WAN_2_WAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else
	{
		return SW_NOT_SUPPORTED;
	}

	return rv;
}

static sw_error_t
_mht_default_flow_cmd_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_flow_type_t type, fal_default_flow_cmd_t * cmd)
{
	sw_error_t rv;
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (FAL_FLOW_LAN_TO_LAN == type)
	{
		HSL_REG_FIELD_GET(rv, dev_id, FlOW_CMD_CTL, vrf_id, LAN_2_LAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_WAN_TO_LAN == type)
	{
		HSL_REG_FIELD_GET(rv, dev_id, FlOW_CMD_CTL, vrf_id, WAN_2_LAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_LAN_TO_WAN == type)
	{
		HSL_REG_FIELD_GET(rv, dev_id, FlOW_CMD_CTL, vrf_id, LAN_2_WAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_WAN_TO_WAN == type)
	{
		HSL_REG_FIELD_GET(rv, dev_id, FlOW_CMD_CTL, vrf_id, WAN_2_WAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else
	{
		return SW_NOT_SUPPORTED;
	}
	SW_RTN_ON_ERROR(rv);

	if (0 == data)
	{
		*cmd = FAL_DEFAULT_FLOW_FORWARD;
	}
	else if (1 == data)
	{
		*cmd = FAL_DEFAULT_FLOW_DROP;
	}
	else if (2 == data)
	{
		*cmd = FAL_DEFAULT_FLOW_RDT_TO_CPU;
	}
	else if (3 == data)
	{
		*cmd = FAL_DEFAULT_FLOW_ADMIT_ALL;
	}

	return SW_OK;
}

static sw_error_t
_mht_default_rt_flow_cmd_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_flow_type_t type, fal_default_flow_cmd_t cmd)
{
	sw_error_t rv;
	a_uint32_t data;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (FAL_DEFAULT_FLOW_FORWARD == cmd)
	{
		data = 0;
	}
	else if (FAL_DEFAULT_FLOW_DROP == cmd)
	{
		data = 1;
	}
	else if (FAL_DEFAULT_FLOW_RDT_TO_CPU == cmd)
	{
		data = 2;
	}
	else if (FAL_DEFAULT_FLOW_ADMIT_ALL == cmd)
	{
		data = 3;
	}
	else
	{
		return SW_NOT_SUPPORTED;
	}

	if (FAL_FLOW_LAN_TO_LAN == type)
	{
		HSL_REG_FIELD_SET(rv, dev_id, FlOW_RT_CMD_CTL, vrf_id, LAN_2_LAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_WAN_TO_LAN == type)
	{
		HSL_REG_FIELD_SET(rv, dev_id, FlOW_RT_CMD_CTL, vrf_id, WAN_2_LAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_LAN_TO_WAN == type)
	{
		HSL_REG_FIELD_SET(rv, dev_id, FlOW_RT_CMD_CTL, vrf_id, LAN_2_WAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_WAN_TO_WAN == type)
	{
		HSL_REG_FIELD_SET(rv, dev_id, FlOW_RT_CMD_CTL, vrf_id, WAN_2_WAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else
	{
		return SW_NOT_SUPPORTED;
	}

	return rv;
}

static sw_error_t
_mht_default_rt_flow_cmd_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_flow_type_t type, fal_default_flow_cmd_t * cmd)
{
	sw_error_t rv;
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);

	rv = _mht_ip_feature_check(dev_id);
	SW_RTN_ON_ERROR(rv);

	if (FAL_FLOW_LAN_TO_LAN == type)
	{
		HSL_REG_FIELD_GET(rv, dev_id, FlOW_RT_CMD_CTL, vrf_id, LAN_2_LAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_WAN_TO_LAN == type)
	{
		HSL_REG_FIELD_GET(rv, dev_id, FlOW_RT_CMD_CTL, vrf_id, WAN_2_LAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_LAN_TO_WAN == type)
	{
		HSL_REG_FIELD_GET(rv, dev_id, FlOW_RT_CMD_CTL, vrf_id, LAN_2_WAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else if (FAL_FLOW_WAN_TO_WAN == type)
	{
		HSL_REG_FIELD_GET(rv, dev_id, FlOW_RT_CMD_CTL, vrf_id, WAN_2_WAN_DEFAULT,
				(a_uint8_t *) (&data), sizeof (a_uint32_t));
	}
	else
	{
		return SW_NOT_SUPPORTED;
	}
	SW_RTN_ON_ERROR(rv);

	if (0 == data)
	{
		*cmd = FAL_DEFAULT_FLOW_FORWARD;
	}
	else if (1 == data)
	{
		*cmd = FAL_DEFAULT_FLOW_DROP;
	}
	else if (2 == data)
	{
		*cmd = FAL_DEFAULT_FLOW_RDT_TO_CPU;
	}
	else if (3 == data)
	{
		*cmd = FAL_DEFAULT_FLOW_ADMIT_ALL;
	}

	return SW_OK;
}

static sw_error_t
_mht_ip_glb_lock_time_set(a_uint32_t dev_id, fal_glb_lock_time_t lock_time)
{
	sw_error_t rv;
	a_uint32_t data = lock_time;

	HSL_REG_FIELD_SET(rv, dev_id, ROUTER_CTRL, 0, GLB_LOCKTIME,
			(a_uint8_t *) (&data), sizeof (a_uint32_t));
	return rv;
}

/**
 * @brief Set IP WCMP table one particular device.
 *   @details Comments:
 *     Hardware only support 0 - 15 hash values and 4 different host tables.
 * @param[in] dev_id device id
 * @param[in] wcmp_id wcmp entry id
 * @param[in] wcmp wcmp entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_wcmp_entry_set(a_uint32_t dev_id, a_uint32_t wcmp_id, fal_ip_wcmp_t * wcmp)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_wcmp_entry_set(dev_id, wcmp_id, wcmp);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get IP WCMP table one particular device.
 *   @details Comments:
 *     Hardware only support 0 - 15 hash values and 4 different host tables.
 * @param[in] dev_id device id
 * @param[in] wcmp_id wcmp entry id
 * @param[out] wcmp wcmp entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_wcmp_entry_get(a_uint32_t dev_id, a_uint32_t wcmp_id, fal_ip_wcmp_t * wcmp)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_wcmp_entry_get(dev_id, wcmp_id, wcmp);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set IP VRF base IP address.
 * @param[in] dev_id device id
 * @param[in] vrf_id VRF route index, from 0~7
 * @param[in] fal_ip4_addr_t IPv4 address for this VRF route
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_vrf_base_addr_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t addr)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_vrf_base_addr_set(dev_id, vrf_id, addr);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get IP VRF base IP address.
 * @param[in] dev_id device id
 * @param[in] vrf_id VRF route index, from 0~7
 * @param[out] fal_ip4_addr_t IPv4 address for this VRF route
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_vrf_base_addr_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t * addr)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_vrf_base_addr_get(dev_id, vrf_id, addr);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set IP VRF base IP address mask.
 * @param[in] dev_id device id
 * @param[in] vrf_id VRF route index, from 0~7
 * @param[in] fal_ip4_addr_t IPv4 address mask for this VRF route
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_vrf_base_mask_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t addr)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_vrf_base_mask_set(dev_id, vrf_id, addr);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get IP VRF base IP address mask.
 * @param[in] dev_id device id
 * @param[in] vrf_id VRF route index, from 0~7
 * @param[out] fal_ip4_addr_t IPv4 address mask for this VRF route
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_vrf_base_mask_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_ip4_addr_t * addr)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_vrf_base_mask_get(dev_id, vrf_id, addr);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set IP default route entry with special default route id.
 * @param[in] dev_id device id
 * @param[in] droute_id default route index, from 0~7
 * @param[in] fal_default_route_t default route entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_default_route_set(a_uint32_t dev_id, a_uint32_t droute_id, fal_default_route_t * entry)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_default_route_set(dev_id, droute_id, entry);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get IP default route entry with special default route id.
 * @param[in] dev_id device id
 * @param[in] droute_id default route index, from 0~7
 * @param[in] fal_default_route_t default route entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_default_route_get(a_uint32_t dev_id, a_uint32_t droute_id, fal_default_route_t * entry)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_default_route_get(dev_id, droute_id, entry);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set IP host route entry with special default route id.
 * @param[in] dev_id device id
 * @param[in] hroute_id default route index, from 0~15
 * @param[in] fal_host_route_t host route entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_host_route_set(a_uint32_t dev_id, a_uint32_t hroute_id, fal_host_route_t * entry)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_host_route_set(dev_id, hroute_id, entry);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get IP host route entry with special default route id.
 * @param[in] dev_id device id
 * @param[in] hroute_id default route index, from 0~15
 * @param[in] fal_host_route_t host route entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_host_route_get(a_uint32_t dev_id, a_uint32_t hroute_id, fal_host_route_t * entry)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_host_route_get(dev_id, hroute_id, entry);
	HSL_API_UNLOCK;
	return rv;
}

#if defined(IN_RFS)
/**
 * @brief Set IP host route load balance.
 * @param[in] dev_id device id
 * @param[in] fal_ip4_rfs_t host route entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_rfs_ip4_set(a_uint32_t dev_id, fal_ip4_rfs_t * rfs)
{
	sw_error_t rv;
	fal_intf_mac_entry_t mac_entry;

	HSL_API_LOCK;
	memset(&mac_entry, 0, sizeof(mac_entry));
	mac_entry.ip4_route = A_TRUE;
	mac_entry.ip6_route = A_TRUE;
	mac_entry.vid_low = rfs->vid;
	mac_entry.vid_high = rfs->vid;
	mac_entry.mac_addr = rfs->mac_addr;
	rv = _mht_ip_intf_entry_add(dev_id, &mac_entry);
	if(!rv)
		rv = _mht_ip_rfs_ip4_set(dev_id, rfs);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set IP6 host route load balance.
 * @param[in] dev_id device id
 * @param[in] fal_ip6_rfs_t host route entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_rfs_ip6_set(a_uint32_t dev_id, fal_ip6_rfs_t * rfs)
{
	sw_error_t rv;
	fal_intf_mac_entry_t mac_entry;

	HSL_API_LOCK;
	memset(&mac_entry, 0, sizeof(mac_entry));
	mac_entry.ip4_route = A_TRUE;
	mac_entry.ip6_route = A_TRUE;
	mac_entry.vid_low = rfs->vid;
	mac_entry.vid_high = rfs->vid;
	mac_entry.mac_addr = rfs->mac_addr;
	rv = _mht_ip_intf_entry_add(dev_id, &mac_entry);
	if(!rv)
		rv = _mht_ip_rfs_ip6_set(dev_id, rfs);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief del IP host route load balance.
 * @param[in] dev_id device id
 * @param[in] fal_ip4_rfs_t host route entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_rfs_ip4_del(a_uint32_t dev_id, fal_ip4_rfs_t * rfs)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_rfs_ip4_del(dev_id, rfs);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief del IP6 host route load balance.
 * @param[in] dev_id device id
 * @param[in] fal_ip6_rfs_t host route entry
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_rfs_ip6_del(a_uint32_t dev_id, fal_ip6_rfs_t * rfs)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_rfs_ip6_del(dev_id, rfs);
	HSL_API_UNLOCK;
	return rv;
}
#endif

/**
 * @brief Set flow type traffic default forward command.
 * @param[in] dev_id device id
 * @param[in] vrf_id VRF route index, from 0~7
 * @param[in] type traffic flow type pass through switch core
 * @param[in] fal_default_flow_cmd_mode_t default flow forward command when flow table mismatch
 * @return SW_OK or error code
 */
sw_error_t
mht_default_flow_cmd_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_flow_type_t type, fal_default_flow_cmd_t cmd)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_default_flow_cmd_set(dev_id, vrf_id, type, cmd);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get flow type traffic default forward command.
 * @param[in] dev_id device id
 * @param[in] vrf_id VRF route index, from 0~7
 * @param[in] type traffic flow type pass through switch core
 * @param[out] fal_default_flow_cmd_mode_t default flow forward command when flow table mismatch
 * @return SW_OK or error code
 */
sw_error_t
mht_default_flow_cmd_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_flow_type_t type, fal_default_flow_cmd_t * cmd)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_default_flow_cmd_get(dev_id, vrf_id, type, cmd);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set flow&route type traffic default forward command.
 * @param[in] dev_id device id
 * @param[in] vrf_id VRF route index, from 0~7
 * @param[in] type traffic flow type pass through switch core
 * @param[in] fal_default_flow_cmd_mode_t default flow&route forward command when route mac match but flow table mismatch
 * @return SW_OK or error code
 */
sw_error_t
mht_default_rt_flow_cmd_set(a_uint32_t dev_id, a_uint32_t vrf_id, fal_flow_type_t type, fal_default_flow_cmd_t cmd)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_default_rt_flow_cmd_set(dev_id, vrf_id, type, cmd);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Get flow&route type traffic default forward command.
 * @param[in] dev_id device id
 * @param[in] vrf_id VRF route index, from 0~7
 * @param[in] type traffic flow type pass through switch core
 * @param[in] fal_default_flow_cmd_mode_t default flow&route forward command when route mac match but flow table mismatch
 * @return SW_OK or error code
 */
sw_error_t
mht_default_rt_flow_cmd_get(a_uint32_t dev_id, a_uint32_t vrf_id, fal_flow_type_t type, fal_default_flow_cmd_t * cmd)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_default_rt_flow_cmd_get(dev_id, vrf_id, type, cmd);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @brief Set blobal lock time.
 * @param[in] dev_id device id
 * @param[in] fal_glb_lock_time_t lock time
 * @return SW_OK or error code
 */
sw_error_t
mht_ip_glb_lock_time_set(a_uint32_t dev_id, fal_glb_lock_time_t lock_time)
{
	sw_error_t rv;

	HSL_API_LOCK;
	rv = _mht_ip_glb_lock_time_set(dev_id, lock_time);
	HSL_API_UNLOCK;
	return rv;
}

/**
 * @}
 */
