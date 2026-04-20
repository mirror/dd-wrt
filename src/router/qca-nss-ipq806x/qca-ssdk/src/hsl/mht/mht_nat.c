/*
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
 * @defgroup mht_nat MHT_NAT
 * @{
 */

#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_port_prop.h"
#include "mht_nat.h"
#include "isisc_reg.h"
#include "mht_reg.h"
#if defined(MHT_NAT_HELPER)
#include "dess_nat_helper.h"
#endif

#define MHT_HOST_ENTRY_DATA0_ADDR              0x0e80
#define MHT_HOST_ENTRY_DATA7_ADDR              0x0e58

#define MHT_HOST_ENTRY_REG_NUM                 8

#define MHT_NAT_ENTRY_FLUSH                    1
#define MHT_NAT_ENTRY_ADD                      2
#define MHT_NAT_ENTRY_DEL                      3
#define MHT_NAT_ENTRY_NEXT                     4
#define MHT_NAT_ENTRY_SEARCH                   5

#define MHT_ENTRY_NAPT                         0
#define MHT_ENTRY_FLOW                         1

#define MHT_PUB_ADDR_NUM                       16
#define MHT_PUB_ADDR_TBL0_ADDR                 0x5aa00
#define MHT_PUB_ADDR_TBL1_ADDR                 0x5aa04

#define MHT_NAPT_ENTRY_NUM                     1024

#define MHT_NAT_COUTER_ADDR                    0x2b000

aos_mutex_lock_t mht_nat_lock;

#if defined(MHT_NAT_HELPER)
extern void nat_helper_cookie_del(a_uint32_t hw_index);
#endif

static sw_error_t
_mht_nat_feature_check(a_uint32_t dev_id)
{
    sw_error_t rv;
    a_uint32_t entry = 0;

    HSL_REG_FIELD_GET(rv, dev_id, MASK_CTL, 0, DEVICE_ID,
                      (a_uint8_t *) (&entry), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (MHT_DEVICE_ID == entry)
    {
        return SW_OK;
    }
    else
    {
        return SW_NOT_SUPPORTED;
    }
}

static sw_error_t
_mht_ip_prvaddr_sw_to_hw(a_uint32_t dev_id, fal_ip4_addr_t sw_addr,
                          a_uint32_t * hw_addr)
{
    /*
        sw_error_t rv;
        a_uint32_t data;

        HSL_REG_FIELD_GET(rv, dev_id, PRVIP_CTL, 0, BASEADDR_SEL,
                          (a_uint8_t *) (&data), sizeof (a_uint32_t));
        SW_RTN_ON_ERROR(rv);

        if (data) {
            *hw_addr = (sw_addr & 0xff) | (((sw_addr >> 16) & 0xf) << 8);
        } else {
            *hw_addr = sw_addr & 0xfff;
        }
    */
    *hw_addr = sw_addr;
    return SW_OK;
}

static sw_error_t
_mht_ip_prvaddr_hw_to_sw(a_uint32_t dev_id, a_uint32_t hw_addr,
                          fal_ip4_addr_t * sw_addr)
{
    /*
        sw_error_t rv;
        a_uint32_t data, addr;

        HSL_REG_FIELD_GET(rv, dev_id, PRVIP_CTL, 0, BASEADDR_SEL,
                          (a_uint8_t *) (&data), sizeof (a_uint32_t));
        SW_RTN_ON_ERROR(rv);

        HSL_REG_FIELD_GET(rv, dev_id, PRVIP_CTL, 0, IP4_BASEADDR,
                          (a_uint8_t *) (&addr), sizeof (a_uint32_t));
        SW_RTN_ON_ERROR(rv);

        if (data) {
            *sw_addr = ((addr & 0xff) << 8) | (((addr >> 8) & 0xfff) << 8)
                | (hw_addr & 0xff) | (((hw_addr >> 8) & 0xf) << 16);
        } else {
            *sw_addr = (addr << 12) | (hw_addr & 0xfff);
        }
        */
    *sw_addr = hw_addr;

    return SW_OK;
}

static sw_error_t
_mht_nat_counter_get(a_uint32_t dev_id, a_uint32_t cnt_id,
                      a_uint32_t counter[4])
{
    sw_error_t rv;
    a_uint32_t i, addr;

    addr = MHT_NAT_COUTER_ADDR + (cnt_id << 4);
    for (i = 0; i < 4; i++)
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
_mht_nat_entry_commit(a_uint32_t dev_id, a_uint32_t entry_type, a_uint32_t op)
{
    a_uint32_t busy = 1, i = 0x9000000, entry = 0;
    sw_error_t rv;


    while (busy && --i)
    {
        HSL_REG_ENTRY_GET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&entry),
                          sizeof (a_uint32_t));
        SW_RTN_ON_ERROR(rv);
        SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_BUSY, busy, entry);
    }

    if (i == 0)
    {
	printk("busy 1\n");
        return SW_BUSY;
    }

    SW_SET_REG_BY_FIELD(HOST_ENTRY7, TBL_BUSY, 1, entry);
    SW_SET_REG_BY_FIELD(HOST_ENTRY7, TBL_SEL, entry_type, entry);
    SW_SET_REG_BY_FIELD(HOST_ENTRY7, ENTRY_FUNC, op, entry);

    HSL_REG_ENTRY_SET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&entry),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    busy = 1;
    i = 0x90000000;
    while (busy && --i)
    {
        HSL_REG_ENTRY_GET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&entry),
                          sizeof (a_uint32_t));
        SW_RTN_ON_ERROR(rv);
        SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_BUSY, busy, entry);
#if 1
        if(MHT_NAT_ENTRY_SEARCH == op &&  busy) break;
#endif
    }

    if (i == 0)
    {
	printk("busy 2\n");
        return SW_BUSY;
    }

    /* hardware requirement, we should delay... */
    if ((MHT_NAT_ENTRY_FLUSH == op) && ((MHT_ENTRY_NAPT == entry_type) ||
		(MHT_ENTRY_FLOW == entry_type)))
    {
        aos_mdelay(10);
    }

    /* hardware requirement, we should read again... */
    HSL_REG_ENTRY_GET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&entry),
                      sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_STAUS, busy, entry);
    if (!busy)
    {
        if (MHT_NAT_ENTRY_NEXT == op)
        {
            return SW_NO_MORE;
        }
        else if (MHT_NAT_ENTRY_SEARCH == op)
        {
            return SW_NOT_FOUND;
        }
        else
        {
            return SW_FAIL;
        }
    }

    return SW_OK;
}

static sw_error_t
_mht_napt_sw_to_hw(a_uint32_t dev_id, fal_napt_entry_t * entry,
                    a_uint32_t reg[])
{
    sw_error_t rv;
    a_uint32_t data;

    reg[0] = entry->dst_addr;

    SW_SET_REG_BY_FIELD(NAPT_ENTRY1, DST_PORT, entry->dst_port, reg[1]);
    SW_SET_REG_BY_FIELD(NAPT_ENTRY1, SRC_PORT, entry->src_port, reg[1]);
    SW_SET_REG_BY_FIELD(NAPT_ENTRY2, TRANS_PORT, entry->trans_port, reg[2]);

    rv = _mht_ip_prvaddr_sw_to_hw(dev_id, entry->src_addr, &data);
    SW_RTN_ON_ERROR(rv);

    SW_SET_REG_BY_FIELD(NAPT_ENTRY2, SRC_IPADDR0, (data & 0xfff), reg[2]);
    SW_SET_REG_BY_FIELD(NAPT_ENTRY3, SRC_IPADDR1, (data >> 12), reg[3]);

    SW_SET_REG_BY_FIELD(NAPT_ENTRY2, TRANS_IPADDR, entry->trans_addr, reg[2]);

    if (FAL_MAC_FRWRD == entry->action)
    {
        if (A_TRUE == entry->mirror_en)
        {
            SW_SET_REG_BY_FIELD(NAPT_ENTRY3, ACTION, 0, reg[3]);
        }
        else
        {
            SW_SET_REG_BY_FIELD(NAPT_ENTRY3, ACTION, 3, reg[3]);
        }
    }
    else if (FAL_MAC_CPY_TO_CPU == entry->action)
    {
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, ACTION, 2, reg[3]);
    }
    else if (FAL_MAC_RDT_TO_CPU == entry->action)
    {
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, ACTION, 1, reg[3]);
    }
    else
    {
        return SW_NOT_SUPPORTED;
    }

    if (A_TRUE == entry->counter_en)
    {
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, CNT_EN, 1, reg[3]);
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, CNT_IDX, entry->counter_id, reg[3]);
    }

	if (A_TRUE == entry->priority_en)
    {
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, PRIORITY_EN, 1, reg[3]);
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, PRIORITY_VAL, entry->priority_val, reg[3]);
    }

    data = 2;
    if (FAL_NAT_ENTRY_PROTOCOL_TCP & entry->flags)
    {
        data = 0;
    }
    else if (FAL_NAT_ENTRY_PROTOCOL_UDP & entry->flags)
    {
        data = 1;
    }
    else if (FAL_NAT_ENTRY_PROTOCOL_PPTP & entry->flags)
    {
        data = 3;
    }
    SW_SET_REG_BY_FIELD(NAPT_ENTRY3, PROT_TYP, data, reg[3]);

    SW_SET_REG_BY_FIELD(NAPT_ENTRY4, AGE_FLAG, entry->status, reg[4]);
	SW_SET_REG_BY_FIELD(NAPT_ENTRY4, AGE_SYNC, entry->aging_sync, reg[4]);
	SW_SET_REG_BY_FIELD(NAPT_ENTRY4, VRF_ID, entry->vrf_id, reg[4]);
	SW_SET_REG_BY_FIELD(NAPT_ENTRY4, FLOW_COOKIE, entry->flow_cookie, reg[4]);
	SW_SET_REG_BY_FIELD(NAPT_ENTRY4, LOAD_BALANCE, entry->load_balance, reg[4]);
    return SW_OK;
}

static sw_error_t
_mht_napt_hw_to_sw(a_uint32_t dev_id, a_uint32_t reg[],
                    fal_napt_entry_t * entry)
{
    sw_error_t rv;
    a_uint32_t data, cnt[4] = {0};

    entry->dst_addr = reg[0];

    SW_GET_FIELD_BY_REG(NAPT_ENTRY1, DST_PORT, entry->dst_port, reg[1]);
    SW_GET_FIELD_BY_REG(NAPT_ENTRY1, SRC_PORT, entry->src_port, reg[1]);
    SW_GET_FIELD_BY_REG(NAPT_ENTRY2, TRANS_PORT, entry->trans_port, reg[2]);

    SW_GET_FIELD_BY_REG(NAPT_ENTRY2, SRC_IPADDR0, data, reg[2]);
    entry->src_addr = data;
    SW_GET_FIELD_BY_REG(NAPT_ENTRY3, SRC_IPADDR1, data, reg[3]);
    data =  (entry->src_addr & 0xfff) | (data << 12);
    rv = _mht_ip_prvaddr_hw_to_sw(dev_id, data, &(entry->src_addr));
    SW_RTN_ON_ERROR(rv);

    entry->flags |= FAL_NAT_ENTRY_TRANS_IPADDR_INDEX;
    SW_GET_FIELD_BY_REG(NAPT_ENTRY2, TRANS_IPADDR, entry->trans_addr, reg[2]);

    SW_GET_FIELD_BY_REG(NAPT_ENTRY3, ACTION, data, reg[3]);
    entry->action = FAL_MAC_FRWRD;
    if (0 == data)
    {
        entry->mirror_en = A_TRUE;
    }
    else if (2 == data)
    {
        entry->action = FAL_MAC_CPY_TO_CPU;
    }
    else if (1 == data)
    {
        entry->action = FAL_MAC_RDT_TO_CPU;
    }

    SW_GET_FIELD_BY_REG(NAPT_ENTRY3, CNT_EN, data, reg[3]);
    if (data)
    {
        entry->counter_en = A_TRUE;
        SW_GET_FIELD_BY_REG(NAPT_ENTRY3, CNT_IDX, entry->counter_id, reg[3]);

        rv = _mht_nat_counter_get(dev_id, entry->counter_id, cnt);
        SW_RTN_ON_ERROR(rv);

        entry->ingress_packet = cnt[0];
        entry->ingress_byte = cnt[1];
        entry->egress_packet = cnt[2];
        entry->egress_byte = cnt[3];
    }
    else
    {
        entry->counter_en = A_FALSE;
    }

	SW_GET_FIELD_BY_REG(NAPT_ENTRY3, PRIORITY_EN, data, reg[3]);
	if (data)
	{
		entry->priority_en = A_TRUE;
		SW_GET_FIELD_BY_REG(NAPT_ENTRY3, PRIORITY_VAL, entry->priority_val, reg[3]);
	}
	else
	{
		entry->priority_en = A_FALSE;
	}

    SW_GET_FIELD_BY_REG(NAPT_ENTRY3, PROT_TYP, data, reg[3]);
    if (0 == data)
    {
        entry->flags |= FAL_NAT_ENTRY_PROTOCOL_TCP;
    }
    else if (1 == data)
    {
        entry->flags |= FAL_NAT_ENTRY_PROTOCOL_UDP;
    }
    else if (3 == data)
    {
        entry->flags |= FAL_NAT_ENTRY_PROTOCOL_PPTP;
    }

    SW_GET_FIELD_BY_REG(NAPT_ENTRY4, AGE_FLAG, entry->status, reg[4]);
	SW_GET_FIELD_BY_REG(NAPT_ENTRY4, AGE_SYNC, entry->aging_sync, reg[4]);
	SW_GET_FIELD_BY_REG(NAPT_ENTRY4, VRF_ID, entry->vrf_id, reg[4]);
	SW_GET_FIELD_BY_REG(NAPT_ENTRY4, FLOW_COOKIE, entry->flow_cookie, reg[4]);
	SW_GET_FIELD_BY_REG(NAPT_ENTRY4, LOAD_BALANCE, entry->load_balance, reg[4]);
    return SW_OK;
}

static sw_error_t
_mht_nat_down_to_hw(a_uint32_t dev_id, a_uint32_t reg[])
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
_mht_nat_up_to_sw(a_uint32_t dev_id, a_uint32_t reg[])
{
    sw_error_t rv;
    a_uint32_t i, addr;

    for (i = 0; i < MHT_HOST_ENTRY_REG_NUM; i++)
    {
        if((MHT_HOST_ENTRY_REG_NUM -1) == i)
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
_mht_napt_add(a_uint32_t dev_id, fal_napt_entry_t * entry)
{
    sw_error_t rv;
    a_uint32_t reg[MHT_HOST_ENTRY_REG_NUM] = { 0 };

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_napt_sw_to_hw(dev_id, entry, reg);
    SW_RTN_ON_ERROR(rv);

	aos_mutex_lock(&mht_nat_lock);
    rv = _mht_nat_down_to_hw(dev_id, reg);
    	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
    	}

    rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_NAPT, MHT_NAT_ENTRY_ADD);
    	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
    	}

    rv = _mht_nat_up_to_sw(dev_id, reg);
    	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
    	}

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX, entry->entry_id, reg[7]);
	aos_mutex_unlock(&mht_nat_lock);
    return SW_OK;
}

static sw_error_t
_mht_napt_del(a_uint32_t dev_id, a_uint32_t del_mode, fal_napt_entry_t * entry)
{
    sw_error_t rv;
    a_uint32_t data, reg[MHT_HOST_ENTRY_REG_NUM] = { 0 };

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

    if (FAL_NAT_ENTRY_ID_EN & del_mode)
    {
        return SW_NOT_SUPPORTED;
    }

    if (FAL_NAT_ENTRY_KEY_EN & del_mode)
    {
        rv = _mht_napt_sw_to_hw(dev_id, entry, reg);
        SW_RTN_ON_ERROR(rv);

	aos_mutex_lock(&mht_nat_lock);
        rv = _mht_nat_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

        rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_NAPT, MHT_NAT_ENTRY_DEL);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

        rv = _mht_nat_up_to_sw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

        SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX, entry->entry_id, reg[7]);
	aos_mutex_unlock(&mht_nat_lock);
        return SW_OK;
    }
    else
    {
        if (FAL_NAT_ENTRY_PUBLIC_IP_EN & del_mode)
        {
            SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_PIP, 1, reg[7]);
            SW_SET_REG_BY_FIELD(NAPT_ENTRY2, TRANS_IPADDR, entry->trans_addr, reg[2]);
        }

        if (FAL_NAT_ENTRY_SOURCE_IP_EN & del_mode)
        {
            SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_SIP, 1, reg[7]);
            rv = _mht_ip_prvaddr_sw_to_hw(dev_id, entry->src_addr, &data);
            SW_RTN_ON_ERROR(rv);
            SW_SET_REG_BY_FIELD(NAPT_ENTRY2, SRC_IPADDR0, (data & 0xfff), reg[2]);
            SW_SET_REG_BY_FIELD(NAPT_ENTRY3, SRC_IPADDR1, (data >> 12), reg[3]);
        }

        if (FAL_NAT_ENTRY_AGE_EN & del_mode)
        {
            SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_STATUS, 1, reg[7]);
            SW_SET_REG_BY_FIELD(NAPT_ENTRY4, AGE_FLAG, entry->status, reg[4]);
        }

	aos_mutex_lock(&mht_nat_lock);
        rv = _mht_nat_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}
        rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_NAPT, MHT_NAT_ENTRY_FLUSH);
	aos_mutex_unlock(&mht_nat_lock);
        return rv;
    }
}

static sw_error_t
_mht_napt_get(a_uint32_t dev_id, a_uint32_t get_mode, fal_napt_entry_t * entry)
{
    sw_error_t rv;
    a_uint32_t found, age, reg[MHT_HOST_ENTRY_REG_NUM] = { 0 };

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

#if 0
    if (FAL_NAT_ENTRY_ID_EN != get_mode)
    {
        return SW_NOT_SUPPORTED;
    }

    SW_SET_REG_BY_FIELD(HOST_ENTRY7, TBL_IDX, entry->entry_id, reg[7]);
#else
    rv = _mht_napt_sw_to_hw(dev_id, entry, reg);
    SW_RTN_ON_ERROR(rv);
#endif

	aos_mutex_lock(&mht_nat_lock);
    rv = _mht_nat_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

    rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_NAPT, MHT_NAT_ENTRY_SEARCH);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

    rv = _mht_nat_up_to_sw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}
	aos_mutex_unlock(&mht_nat_lock);

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_STAUS, found, reg[7]);
    SW_GET_FIELD_BY_REG(NAPT_ENTRY4, AGE_FLAG,  age, reg[4]);
    if (found && age)
    {
        found = 1;
    }
    else
    {
        found = 0;
    }

    rv = _mht_napt_hw_to_sw(dev_id, reg, entry);
    SW_RTN_ON_ERROR(rv);

    if (!found)
    {
        return SW_NOT_FOUND;
    }

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX,   entry->entry_id, reg[7]);
    return SW_OK;
}

static sw_error_t
_mht_flow_add(a_uint32_t dev_id, fal_napt_entry_t * entry)
{
    sw_error_t rv;
    a_uint32_t reg[MHT_HOST_ENTRY_REG_NUM] = { 0 };

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_napt_sw_to_hw(dev_id, entry, reg);
    SW_RTN_ON_ERROR(rv);

	aos_mutex_lock(&mht_nat_lock);
    rv = _mht_nat_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

    rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_FLOW, MHT_NAT_ENTRY_ADD);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

    rv = _mht_nat_up_to_sw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX, entry->entry_id, reg[7]);
	aos_mutex_unlock(&mht_nat_lock);
    return SW_OK;
}

static sw_error_t
_mht_flow_del(a_uint32_t dev_id, a_uint32_t del_mode, fal_napt_entry_t * entry)
{
    sw_error_t rv;
    a_uint32_t data, reg[MHT_HOST_ENTRY_REG_NUM] = { 0 };

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

    if (FAL_NAT_ENTRY_ID_EN & del_mode)
    {
        return SW_NOT_SUPPORTED;
    }

    if (FAL_NAT_ENTRY_KEY_EN & del_mode)
    {
        rv = _mht_napt_sw_to_hw(dev_id, entry, reg);
        SW_RTN_ON_ERROR(rv);

	aos_mutex_lock(&mht_nat_lock);
        rv = _mht_nat_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

        rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_FLOW, MHT_NAT_ENTRY_DEL);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

        rv = _mht_nat_up_to_sw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

        SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX, entry->entry_id, reg[7]);
	aos_mutex_unlock(&mht_nat_lock);
        return SW_OK;
    }
    else
    {
        if (FAL_NAT_ENTRY_PUBLIC_IP_EN & del_mode)
        {
            SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_PIP, 1, reg[7]);
            SW_SET_REG_BY_FIELD(NAPT_ENTRY2, TRANS_IPADDR, entry->trans_addr, reg[2]);
        }

        if (FAL_NAT_ENTRY_SOURCE_IP_EN & del_mode)
        {
            SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_SIP, 1, reg[7]);
            rv = _mht_ip_prvaddr_sw_to_hw(dev_id, entry->src_addr, &data);
            SW_RTN_ON_ERROR(rv);
            SW_SET_REG_BY_FIELD(NAPT_ENTRY2, SRC_IPADDR0, (data & 0xfff), reg[2]);
            SW_SET_REG_BY_FIELD(NAPT_ENTRY3, SRC_IPADDR1, (data >> 12), reg[3]);
        }

        if (FAL_NAT_ENTRY_AGE_EN & del_mode)
        {
            SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_STATUS, 1, reg[7]);
            SW_SET_REG_BY_FIELD(NAPT_ENTRY4, AGE_FLAG, entry->status, reg[4]);
        }

	aos_mutex_lock(&mht_nat_lock);
        rv = _mht_nat_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

        rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_FLOW, MHT_NAT_ENTRY_FLUSH);
	aos_mutex_unlock(&mht_nat_lock);
        return rv;
    }
}

static sw_error_t
_mht_flow_get(a_uint32_t dev_id, a_uint32_t get_mode, fal_napt_entry_t * entry)
{
    sw_error_t rv;
    a_uint32_t found, age, reg[MHT_HOST_ENTRY_REG_NUM] = { 0 };

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

#if 0
    if (FAL_NAT_ENTRY_ID_EN != get_mode)
    {
        return SW_NOT_SUPPORTED;
    }

    SW_SET_REG_BY_FIELD(HOST_ENTRY7, TBL_IDX, entry->entry_id, reg[7]);
#else
    rv = _mht_napt_sw_to_hw(dev_id, entry, reg);
    SW_RTN_ON_ERROR(rv);
#endif

	aos_mutex_lock(&mht_nat_lock);
    rv = _mht_nat_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

    rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_FLOW, MHT_NAT_ENTRY_SEARCH);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

    rv = _mht_nat_up_to_sw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}
	aos_mutex_unlock(&mht_nat_lock);

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_STAUS, found, reg[7]);
    SW_GET_FIELD_BY_REG(NAPT_ENTRY4, AGE_FLAG,  age, reg[4]);
    if (found && age)
    {
        found = 1;
    }
    else
    {
        found = 0;
    }

    rv = _mht_napt_hw_to_sw(dev_id, reg, entry);
    SW_RTN_ON_ERROR(rv);

    if (!found)
    {
        return SW_NOT_FOUND;
    }

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX,   entry->entry_id, reg[7]);
    return SW_OK;
}

static sw_error_t
_mht_flow_next(a_uint32_t dev_id, a_uint32_t next_mode,
                fal_napt_entry_t * napt_entry)
{
    a_uint32_t data, idx, reg[MHT_HOST_ENTRY_REG_NUM] = { 0 };
    sw_error_t rv;

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

    if (FAL_NEXT_ENTRY_FIRST_ID == napt_entry->entry_id)
    {
        idx = MHT_NAPT_ENTRY_NUM - 1;
    }
    else
    {
        if ((MHT_NAPT_ENTRY_NUM - 1) == napt_entry->entry_id)
        {
            return SW_NO_MORE;
        }
        else
        {
            idx = napt_entry->entry_id;
        }
    }

    if (FAL_NAT_ENTRY_PUBLIC_IP_EN & next_mode)
    {
        SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_PIP, 1, reg[7]);
        SW_SET_REG_BY_FIELD(NAPT_ENTRY2, TRANS_IPADDR, napt_entry->trans_addr, reg[2]);
    }

    if (FAL_NAT_ENTRY_SOURCE_IP_EN & next_mode)
    {
        SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_SIP, 1, reg[7]);
        rv = _mht_ip_prvaddr_sw_to_hw(dev_id, napt_entry->src_addr, &data);
        SW_RTN_ON_ERROR(rv);
        SW_SET_REG_BY_FIELD(NAPT_ENTRY2, SRC_IPADDR0, (data & 0xfff), reg[2]);
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, SRC_IPADDR1, (data >> 12), reg[3]);
    }

    if (FAL_NAT_ENTRY_AGE_EN & next_mode)
    {
        SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_STATUS, 1, reg[7]);
        SW_SET_REG_BY_FIELD(NAPT_ENTRY4, AGE_FLAG, napt_entry->status, reg[4]);
    }

	if (FAL_NAT_ENTRY_SYNC_EN & next_mode)
    {
        SW_SET_REG_BY_FIELD(HOST_ENTRY7, SPEC_SYNC, 1, reg[7]);
        SW_SET_REG_BY_FIELD(NAPT_ENTRY4, AGE_SYNC, napt_entry->status, reg[4]);
    }

    SW_SET_REG_BY_FIELD(HOST_ENTRY7, TBL_IDX, idx, reg[7]);

	aos_mutex_lock(&mht_nat_lock);
    rv = _mht_nat_down_to_hw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

    rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_FLOW, MHT_NAT_ENTRY_NEXT);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}

    rv = _mht_nat_up_to_sw(dev_id, reg);
	if (rv != SW_OK) {
		aos_mutex_unlock(&mht_nat_lock);
		return rv;
	}
	aos_mutex_unlock(&mht_nat_lock);

    aos_mem_zero(napt_entry, sizeof (fal_nat_entry_t));

    rv = _mht_napt_hw_to_sw(dev_id, reg, napt_entry);
    SW_RTN_ON_ERROR(rv);

#if 0
    a_uint32_t temp=0, complete=0;

    HSL_REG_ENTRY_GET(rv, dev_id, HOST_ENTRY7, 0, (a_uint8_t *) (&temp),
                      sizeof (a_uint32_t));

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_STAUS, complete, temp);

    if (!complete)
    {
        return SW_NO_MORE;
    }
#endif

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX, napt_entry->entry_id, reg[7]);
    return SW_OK;
}

static sw_error_t
_mht_flow_counter_bind(a_uint32_t dev_id, a_uint32_t entry_id,
                        a_uint32_t cnt_id, a_bool_t enable)
{
    sw_error_t rv;
    a_uint32_t reg[MHT_HOST_ENTRY_REG_NUM] = { 0 }, tbl_idx;

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

    tbl_idx = (entry_id - 1) & 0x3ff;
    SW_SET_REG_BY_FIELD(HOST_ENTRY7, TBL_IDX, tbl_idx, reg[7]);

    rv = _mht_nat_down_to_hw(dev_id, reg);
    SW_RTN_ON_ERROR(rv);

    rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_FLOW, MHT_NAT_ENTRY_NEXT);
    if (SW_OK != rv)
    {
        return SW_NOT_FOUND;
    }

    rv = _mht_nat_up_to_sw(dev_id, reg);
    SW_RTN_ON_ERROR(rv);

    SW_GET_FIELD_BY_REG(HOST_ENTRY7, TBL_IDX, tbl_idx, reg[7]);
    if (entry_id != tbl_idx)
    {
        return SW_NOT_FOUND;
    }

    if (A_FALSE == enable)
    {
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, CNT_EN, 0, reg[3]);
    }
    else if (A_TRUE == enable)
    {
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, CNT_EN, 1, reg[3]);
        SW_SET_REG_BY_FIELD(NAPT_ENTRY3, CNT_IDX, cnt_id, reg[3]);
    }
    else
    {
        return SW_BAD_PARAM;
    }

    rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_FLOW, MHT_NAT_ENTRY_DEL);
    SW_RTN_ON_ERROR(rv);

    rv = _mht_nat_down_to_hw(dev_id, reg);
    SW_RTN_ON_ERROR(rv);

    rv = _mht_nat_entry_commit(dev_id, MHT_ENTRY_FLOW, MHT_NAT_ENTRY_ADD);
    return rv;
}

static sw_error_t
_mht_nat_status_get(a_uint32_t dev_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t data = 0;

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

    HSL_REG_FIELD_GET(rv, dev_id, NAT_CTRL, 0, NAT_EN,
                      (a_uint8_t *) (&data), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (data)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return rv;
}

static sw_error_t
_mht_napt_status_get(a_uint32_t dev_id, a_bool_t * enable)
{
    sw_error_t rv;
    a_uint32_t data = 0;

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

    HSL_REG_FIELD_GET(rv, dev_id, NAT_CTRL, 0, NAPT_EN,
                      (a_uint8_t *) (&data), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    if (data)
    {
        *enable = A_TRUE;
    }
    else
    {
        *enable = A_FALSE;
    }

    return rv;
}

static sw_error_t
_mht_nat_pub_addr_next(a_uint32_t dev_id, a_uint32_t next_mode,
                        fal_nat_pub_addr_t * entry)
{
    sw_error_t rv;
    a_uint32_t data, addr, idx, index, tbl[2] = {0};

    HSL_DEV_ID_CHECK(dev_id);

    rv = _mht_nat_feature_check(dev_id);
    SW_RTN_ON_ERROR(rv);

    if (FAL_NEXT_ENTRY_FIRST_ID == entry->entry_id)
    {
        idx = 0;
    }
    else
    {
        if ((MHT_PUB_ADDR_NUM - 1) == entry->entry_id)
        {
            return SW_NO_MORE;
        }
        else
        {
            idx = entry->entry_id + 1;
        }
    }

    for (index = idx; index < MHT_PUB_ADDR_NUM; index++)
    {
        addr = MHT_PUB_ADDR_TBL1_ADDR + (index << 4);
        HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
                              (a_uint8_t *) (&(tbl[1])), sizeof (a_uint32_t));
        SW_RTN_ON_ERROR(rv);

        SW_GET_FIELD_BY_REG(PUB_ADDR1, ADDR_VALID, data, tbl[1]);
        if (data)
        {
            break;
        }
    }

    if (MHT_PUB_ADDR_NUM == index)
    {
        return SW_NO_MORE;
    }

    addr = MHT_PUB_ADDR_TBL0_ADDR + (index << 4);
    HSL_REG_ENTRY_GEN_GET(rv, dev_id, addr, sizeof (a_uint32_t),
                          (a_uint8_t *) (&(tbl[0])), sizeof (a_uint32_t));
    SW_RTN_ON_ERROR(rv);

    entry->entry_id = index;
    entry->pub_addr = tbl[0];

    return SW_OK;
}

#define MHT_NAT_VRF_ENTRY_TBL_ADDR   0x0484
#define MHT_NAT_VRF_ENTRY_MASK_ADDR   0x0488

a_uint8_t _mht_snat_matched(a_uint32_t dev_id, fal_ip4_addr_t addr)
{
	a_bool_t nat_enable = 0, napt_enable = 0;
	fal_ip4_addr_t mask = 0, base = 0;
	a_uint32_t reg_addr;
	sw_error_t rv;

	_mht_nat_status_get(dev_id, &nat_enable);
	_mht_napt_status_get(dev_id, &napt_enable);
	if(!(nat_enable & napt_enable))
		return 0;

	/*check for private base ip*/
	reg_addr = MHT_NAT_VRF_ENTRY_MASK_ADDR;
	HSL_REG_ENTRY_GEN_GET(rv, dev_id, reg_addr, sizeof (a_uint32_t),
                          (a_uint8_t *) (&mask), sizeof (a_uint32_t));

	reg_addr = MHT_NAT_VRF_ENTRY_TBL_ADDR;
	HSL_REG_ENTRY_GEN_GET(rv, dev_id, reg_addr, sizeof (a_uint32_t),
                          (a_uint8_t *) (&base), sizeof (a_uint32_t));
	if (rv)
		return 0;
	if((mask&addr) == (mask&base)) {
		return 1;
	}

	return 0;
}

a_uint8_t _mht_dnat_matched(
		a_uint32_t dev_id,
		fal_ip4_addr_t addr,
		a_uint8_t *index)
{
	a_bool_t nat_enable = 0, napt_enable = 0;
	fal_nat_pub_addr_t entry;
	sw_error_t ret;

	_mht_nat_status_get(dev_id, &nat_enable);
	_mht_napt_status_get(dev_id, &napt_enable);
	if(!(nat_enable & napt_enable))
		return 0;

	/*check for public ip*/
	memset(&entry, 0, sizeof(entry));
	entry.entry_id = FAL_NEXT_ENTRY_FIRST_ID;
	while(1) {
		ret = _mht_nat_pub_addr_next(dev_id, 0, &entry);
		if(ret) {
			break;
		}
		if(entry.pub_addr == addr) {
			*index = entry.entry_id;
			return 1;
		}
	}
	return 0;
}



static sw_error_t
_mht_flow_cookie_snat_set(a_uint32_t dev_id, fal_flow_cookie_t * flow_cookie)
{
	fal_napt_entry_t entry;
	sw_error_t ret;

	memset(&entry, 0, sizeof(entry));
	entry.flags = FAL_NAT_ENTRY_TRANS_IPADDR_INDEX | flow_cookie->proto;
	entry.status = 0xf;
	entry.src_addr = flow_cookie->src_addr;
	entry.dst_addr = flow_cookie->dst_addr;
	entry.src_port = flow_cookie->src_port;
	entry.dst_port = flow_cookie->dst_port;
	entry.trans_port = flow_cookie->src_port;
	entry.action = FAL_MAC_RDT_TO_CPU;
	ret = _mht_napt_get(dev_id, 0, &entry);
	if(ret) {
		if(flow_cookie->flow_cookie == 0)
			return SW_OK;
	}
	if(flow_cookie->flow_cookie == 0) {
		if(entry.flow_cookie == 0) {
			ret = _mht_napt_del(dev_id, FAL_NAT_ENTRY_KEY_EN, &entry);
			#if defined(MHT_NAT_HELPER)
			#if 0
			napt_cookie[entry.entry_id*2+1] = 0;
			#endif
			if (mht_nat_global_status)
				nat_helper_cookie_del(entry.entry_id);
			#endif
			return ret;
		}
		ret = _mht_napt_del(dev_id, FAL_NAT_ENTRY_KEY_EN, &entry);
	} else {
		entry.flow_cookie = flow_cookie->flow_cookie;
		ret = _mht_napt_add(dev_id, &entry);
	}

	return ret;
}

static sw_error_t
_mht_flow_cookie_dnat_set(
		a_uint32_t dev_id,
		fal_flow_cookie_t * flow_cookie,
		a_uint8_t index)
{
	fal_napt_entry_t entry;
	sw_error_t ret = 0;

	memset(&entry, 0, sizeof(entry));
	entry.flags = FAL_NAT_ENTRY_TRANS_IPADDR_INDEX | flow_cookie->proto;
	entry.status = 0xf;
	entry.trans_addr = index;
	entry.trans_port = flow_cookie->dst_port;
	entry.dst_addr = flow_cookie->src_addr;
	entry.dst_port = flow_cookie->src_port;
	entry.src_port = flow_cookie->dst_port;
	entry.action = FAL_MAC_RDT_TO_CPU;
	ret = _mht_napt_get(dev_id, 0, &entry);
	if(ret) {
		if(flow_cookie->flow_cookie == 0) {
			return SW_OK;
		} else {
			/*add a fresh flowcookie*/
			entry.flow_cookie = flow_cookie->flow_cookie;
			ret = _mht_napt_add(dev_id, &entry);
			return ret;
		}
	}
	if(flow_cookie->flow_cookie == 0) {
		/*del flow cookie*/
		if(entry.flow_cookie == 0) {
			ret = _mht_napt_del(dev_id, FAL_NAT_ENTRY_KEY_EN, &entry);
			#if defined(MHT_NAT_HELPER)
			#if 0
			napt_cookie[entry.entry_id*2] = 0;
			#endif
			if (mht_nat_global_status)
				nat_helper_cookie_del(entry.entry_id);
			#endif
			return ret;
		}
		ret = _mht_napt_del(dev_id, FAL_NAT_ENTRY_KEY_EN, &entry);
		if(entry.load_balance & 4) {
			/*keep rfs*/
			entry.flow_cookie = 0;
			ret = _mht_napt_add(dev_id, &entry);
			return ret;
		}
	} else {
		/*add flow cookie*/
		ret = _mht_napt_del(dev_id, FAL_NAT_ENTRY_KEY_EN, &entry);
		entry.flow_cookie = flow_cookie->flow_cookie;
		ret = _mht_napt_add(dev_id, &entry);
		return ret;
	}
	return ret;

}

static sw_error_t
_mht_flow_rfs_dnat_set(
		a_uint32_t dev_id,
		a_uint8_t action,
		fal_flow_rfs_t * rfs,
		a_uint8_t index)
{
	fal_napt_entry_t entry;
	sw_error_t ret = 0;

	memset(&entry, 0, sizeof(entry));
	entry.flags = FAL_NAT_ENTRY_TRANS_IPADDR_INDEX | rfs->proto;
	entry.status = 0xf;
	entry.trans_addr = index;
	entry.trans_port = rfs->dst_port;
	entry.dst_addr = rfs->src_addr;
	entry.dst_port = rfs->src_port;
	entry.src_port = rfs->dst_port;
	entry.action = FAL_MAC_RDT_TO_CPU;
	ret = _mht_napt_get(dev_id, 0, &entry);
	if(ret) {
		if(action == 0) {
			return SW_FAIL;
		} else {
			/*add a fresh rfs*/
			entry.load_balance = rfs->load_balance | 4;
			ret = _mht_napt_add(dev_id, &entry);
			return ret;
		}
	}
	if(action == 0) {
		/*del flow rfs*/
		ret = _mht_napt_del(dev_id, FAL_NAT_ENTRY_KEY_EN, &entry);
		if(entry.flow_cookie != 0) {
			/*keep cookie*/
			entry.load_balance = 0;
			ret = _mht_napt_add(dev_id, &entry);
			return ret;
		}
	} else {
		/*add flow rfs*/
		ret = _mht_napt_del(dev_id, FAL_NAT_ENTRY_KEY_EN, &entry);
		entry.load_balance = rfs->load_balance | 4;
		ret = _mht_napt_add(dev_id, &entry);
		return ret;
	}
	return ret;

}


static sw_error_t
_mht_flow_cookie_set(a_uint32_t dev_id, fal_flow_cookie_t * flow_cookie)
{
	fal_napt_entry_t entry;
	sw_error_t ret;
	a_uint8_t index;

	if(_mht_dnat_matched(dev_id, flow_cookie->dst_addr, &index))
		return _mht_flow_cookie_dnat_set(dev_id, flow_cookie, index);
	if(_mht_snat_matched(dev_id, flow_cookie->src_addr))
		return _mht_flow_cookie_snat_set(dev_id, flow_cookie);

	/*normal flow*/
	memset(&entry, 0, sizeof(entry));
	entry.flags = flow_cookie->proto;
	entry.src_addr = flow_cookie->src_addr;
	entry.dst_addr = flow_cookie->dst_addr;
	entry.src_port = flow_cookie->src_port;
	entry.dst_port = flow_cookie->dst_port;
    ret = _mht_flow_get(0, 0, &entry);
	if(SW_OK != ret && flow_cookie->flow_cookie == 0)
		return ret;
	if(flow_cookie->flow_cookie == 0) {
		/*del*/
		_mht_flow_del(0, FAL_NAT_ENTRY_KEY_EN, &entry);
		if(entry.load_balance & 4) {
			entry.status = 0xf;
			entry.flow_cookie = 0;
			return _mht_flow_add(0, &entry);
		}
	} else {
		/*add*/
		if(ret == SW_OK)
			_mht_flow_del(0, FAL_NAT_ENTRY_KEY_EN, &entry);
		entry.status = 0xf;
		entry.flow_cookie = flow_cookie->flow_cookie;
		return _mht_flow_add(0, &entry);
	}
    return SW_OK;
}

static sw_error_t
_mht_flow_rfs_set(a_uint32_t dev_id, a_uint8_t action, fal_flow_rfs_t * rfs)
{
	fal_napt_entry_t entry;
	sw_error_t ret;
	a_uint8_t index;

	if(_mht_dnat_matched(dev_id, rfs->dst_addr, &index))
		return _mht_flow_rfs_dnat_set(dev_id, action, rfs, index);

	memset(&entry, 0, sizeof(entry));
	entry.flags = rfs->proto;
	entry.src_addr = rfs->src_addr;
	entry.dst_addr = rfs->dst_addr;
	entry.src_port = rfs->src_port;
	entry.dst_port = rfs->dst_port;
	ret = _mht_flow_get(0, 0, &entry);
	if(SW_OK != ret && action == 0)
		return ret;
	if(action == 0) {
		/*del*/
		_mht_flow_del(0, FAL_NAT_ENTRY_KEY_EN, &entry);
		if(entry.flow_cookie != 0) {
			entry.load_balance = 0;
			return _mht_flow_add(0, &entry);
		}
	} else {
		/*add*/
		if(ret == SW_OK)
			_mht_flow_del(0, FAL_NAT_ENTRY_KEY_EN, &entry);
		entry.status = 0xf;
		entry.load_balance = rfs->load_balance | 0x4;
		return _mht_flow_add(0, &entry);
	}
	return SW_OK;
}

/**
 * @brief Add one FLOW entry to one particular device.
 *   @details Comments:
       Before FLOW entry added related ip4 private base address must be set
       at first.
       In parameter napt_entry related entry flags must be set
       Hardware entry id will be returned.
 * @param[in] dev_id device id
 * @param[in] napt_entry FLOW entry parameter
 * @return SW_OK or error code
 */
sw_error_t
mht_flow_add(a_uint32_t dev_id, fal_napt_entry_t * napt_entry)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_flow_add(dev_id, napt_entry);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Del FLOW entries from one particular device.
 * @param[in] dev_id device id
 * @param[in] del_mode FLOW entry delete operation mode
 * @param[in] napt_entry FLOW entry parameter
 * @return SW_OK or error code
 */
sw_error_t
mht_flow_del(a_uint32_t dev_id, a_uint32_t del_mode,
              fal_napt_entry_t * napt_entry)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_flow_del(dev_id, del_mode, napt_entry);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Get one FLOW entry from one particular device.
 * @param[in] dev_id device id
 * @param[in] get_mode FLOW entry get operation mode
 * @param[in] nat_entry FLOW entry parameter
 * @param[out] nat_entry FLOW entry parameter
 * @return SW_OK or error code
 */
sw_error_t
mht_flow_get(a_uint32_t dev_id, a_uint32_t get_mode,
              fal_napt_entry_t * napt_entry)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_flow_get(dev_id, get_mode, napt_entry);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Next FLOW entries from one particular device.
 * @param[in] dev_id device id
 * @param[in] next_mode FLOW entry next operation mode
 * @param[in] napt_entry FLOW entry parameter
 * @param[out] napt_entry FLOW entry parameter
 * @return SW_OK or error code
 */
sw_error_t
mht_flow_next(a_uint32_t dev_id, a_uint32_t next_mode,
               fal_napt_entry_t * napt_entry)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_flow_next(dev_id, next_mode, napt_entry);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Bind one counter entry to one FLOW entry to one particular device.
 * @param[in] dev_id device id
 * @param[in] entry_id FLOW entry id
 * @param[in] cnt_id counter entry id
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
mht_flow_counter_bind(a_uint32_t dev_id, a_uint32_t entry_id,
                       a_uint32_t cnt_id, a_bool_t enable)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_flow_counter_bind(dev_id, entry_id, cnt_id, enable);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Add/del one FLOW cookie entry to one particular device.
 *   @details Comments:
       Before FLOW entry added related ip4 private base address must be set
       at first.
       In parameter napt_entry related entry flags must be set
       Hardware entry id will be returned.
 * @param[in] dev_id device id
 * @param[in]  FLOW cookie entry parameter
 * @return SW_OK or error code
 */
sw_error_t
mht_flow_cookie_set(a_uint32_t dev_id, fal_flow_cookie_t * flow_cookie)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_flow_cookie_set(dev_id, flow_cookie);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @brief Add/del one FLOW rfs entry to one particular device.
 *   @details Comments:
       Before FLOW entry added related ip4 private base address must be set
       at first.
       In parameter napt_entry related entry flags must be set
       Hardware entry id will be returned.
 * @param[in] dev_id device id
 * @param[in]  FLOW cookie entry parameter
 * @return SW_OK or error code
 */
sw_error_t
mht_flow_rfs_set(a_uint32_t dev_id, a_uint8_t action, fal_flow_rfs_t * rfs)
{
    sw_error_t rv;

    HSL_API_LOCK;
    rv = _mht_flow_rfs_set(dev_id, action, rfs);
    HSL_API_UNLOCK;
    return rv;
}

/**
 * @}
 */
