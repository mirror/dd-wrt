/*
 * Copyright (c) 2014, 2016-2018, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*qca808x_start*/
#include <stdio.h>
#include "shell.h"
#include "fal.h"

static int sw_devid = 0;

sw_error_t
cmd_set_devid(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    ssdk_cfg_t ssdk_cfg_new;

    if (arg_val[1] >= SW_MAX_NR_DEV)
    {
        dprintf("dev_id should be less than <%d>\n", SW_MAX_NR_DEV);
        return SW_FAIL;
    }
    sw_devid = arg_val[1];

    rtn = fal_ssdk_cfg(sw_devid, &ssdk_cfg_new);
    if (rtn == SW_OK)
    {
	    ssdk_cfg = ssdk_cfg_new;
    }

    return rtn;
}

int
get_devid(void)
{
    return sw_devid;
}

int
set_devid(int dev_id)
{
	sw_devid = dev_id;
	return SW_OK;
}
/*qca808x_end*/
sw_error_t
cmd_show_fdb(a_ulong_t *arg_val)
{
	if ((ssdk_cfg.init_cfg.chip_type == CHIP_ISISC) ||
			(ssdk_cfg.init_cfg.chip_type == CHIP_ISIS) ||
			(ssdk_cfg.init_cfg.chip_type == CHIP_MHT) ||
			(ssdk_cfg.init_cfg.chip_type == CHIP_DESS) ||
			(ssdk_cfg.init_cfg.chip_type == CHIP_APPE) ||
		    (ssdk_cfg.init_cfg.chip_type == CHIP_HPPE) ||
		    (ssdk_cfg.init_cfg.chip_type == CHIP_MRPPE)) {
		sw_error_t rtn;
		a_uint32_t cnt = 0, type = 0;
		a_uint16_t rtn_size = sizeof(sw_error_t);
		a_uint16_t p_size = sizeof(a_ulong_t);

		fal_fdb_op_t *fdb_op = (fal_fdb_op_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);
		fal_fdb_entry_t *fdb_entry = (fal_fdb_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/
			p_size + (sizeof(fal_fdb_op_t)+p_size-1)/p_size);

		aos_mem_zero(fdb_op, sizeof (fal_fdb_op_t));
		aos_mem_zero(fdb_entry, sizeof (fal_fdb_entry_t));
		if(arg_val[1] != HW_ENTRY && arg_val[1] != SW_ENTRY)
			return SW_NOT_SUPPORTED;

		if((arg_val[1] == SW_ENTRY) && (ssdk_cfg.init_cfg.chip_type != CHIP_MHT))
			return SW_NOT_SUPPORTED;

		type = arg_val[1];
		arg_val[0] = SW_API_FDB_EXTEND_FIRST;

		while (1)
		{
			fdb_entry->type = type;
			arg_val[1] = (a_ulong_t) ioctl_buf;
			arg_val[2] = get_devid();
			arg_val[3] = (a_ulong_t) fdb_op;
			arg_val[4] = (a_ulong_t) fdb_entry;

			rtn = cmd_exec_api(arg_val);
			if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
			{
				break;
			}
			arg_val[0] = SW_API_FDB_EXTEND_NEXT;
			cnt++;
			if (cnt > 2048) {
				dprintf("\n Query FDB entry exception!\n");
				rtn = SW_ABORTED;
				break;
			}
		}

		if((rtn != SW_OK) && (rtn != SW_NO_MORE))
			cmd_print_error(rtn);
		else
			dprintf("\ntotal %d entries\n", cnt);
	}else if (ssdk_cfg.init_cfg.chip_type == CHIP_SHIVA) {
		sw_error_t rtn;
		a_uint32_t cnt = 0;
		fal_fdb_entry_t *fdb_entry = (fal_fdb_entry_t *) (ioctl_buf + 2);

		memset(fdb_entry, 0, sizeof (fal_fdb_entry_t));
		arg_val[0] = SW_API_FDB_ITERATE;
		*(ioctl_buf + 1) = 0;

		while (1)
		{
			arg_val[1] = (a_ulong_t) ioctl_buf;
			arg_val[2] = get_devid();
			arg_val[3] = (a_ulong_t) (ioctl_buf + 1);
			arg_val[4] = (a_ulong_t) fdb_entry;

			rtn = cmd_exec_api(arg_val);
			if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
			{
				break;
			}
			cnt++;
		}

		if((rtn != SW_OK) && (rtn != SW_NO_MORE))
			cmd_print_error(rtn);
		else
			dprintf("\ntotal %d entries\n", cnt);
	}else {
		sw_error_t rtn;
		a_uint32_t cnt = 0;
		a_uint16_t rtn_size = sizeof(sw_error_t);
		a_uint16_t p_size = sizeof(a_ulong_t);
		fal_fdb_entry_t *fdb_entry = (fal_fdb_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/
			p_size);

		memset(fdb_entry, 0, sizeof (fal_fdb_entry_t));
		arg_val[0] = SW_API_FDB_FIRST;

		while (1)
		{
			arg_val[1] = (a_ulong_t) ioctl_buf;
			arg_val[2] = get_devid();
			arg_val[3] = (a_ulong_t) fdb_entry;

			rtn = cmd_exec_api(arg_val);
			if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
			{
				break;
			}
			arg_val[0] = SW_API_FDB_NEXT;
			cnt++;
		}

		if((rtn != SW_OK) && (rtn != SW_NO_MORE))
			cmd_print_error(rtn);
		else
			dprintf("\ntotal %d entries\n", cnt);
    }

    return SW_OK;
}

sw_error_t
cmd_show_ctrlpkt(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t cnt = 0;
	a_uint16_t rtn_size = sizeof(sw_error_t);
	a_uint16_t p_size = sizeof(a_ulong_t);
	fal_ctrlpkt_profile_t *ctrlpkt =
		(fal_ctrlpkt_profile_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

	aos_mem_zero(ctrlpkt, sizeof (fal_ctrlpkt_profile_t));
	arg_val[0] = SW_API_MGMTCTRL_CTRLPKT_PROFILE_GETFIRST;

	while (1)
	{
		arg_val[1] = (a_ulong_t) ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = (a_ulong_t) ctrlpkt;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
		{
			break;
		}
		arg_val[0] = SW_API_MGMTCTRL_CTRLPKT_PROFILE_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NO_MORE))
		cmd_print_error(rtn);
	else
		dprintf("\ntotal %d entries\n", cnt);

	return SW_OK;
}

sw_error_t
cmd_show_vlan(a_ulong_t *arg_val)
{
	sw_error_t rtn = SW_OK;
	a_uint32_t tmp_vid = FAL_NEXT_ENTRY_FIRST_ID, cnt = 0;
	a_uint16_t rtn_size = sizeof(sw_error_t);
	a_uint16_t p_size = sizeof(a_ulong_t);
	fal_vlan_t *vlan_entry = (fal_vlan_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

	switch (ssdk_cfg.init_cfg.chip_type) {
		case CHIP_ISIS:
		case CHIP_ISISC:
		case CHIP_MHT:
		case CHIP_DESS:
			tmp_vid = FAL_NEXT_ENTRY_FIRST_ID;
			break;
		case CHIP_ATHENA:
		case CHIP_GARUDA:
		case CHIP_SHIVA:
		case CHIP_HORUS:
			tmp_vid = 0;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	while (1)
	{
		arg_val[0] = SW_API_VLAN_NEXT;
		arg_val[1] = (a_ulong_t) ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = tmp_vid;
		arg_val[4] = (a_ulong_t) vlan_entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
		{
			break;
		}

		tmp_vid = vlan_entry->vid;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NO_MORE)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\ntotal %d entries\n", cnt);
	}

	return rtn;
}

sw_error_t
cmd_show_resv_fdb(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    a_ulong_t  *iterator  = (ioctl_buf+(rtn_size+p_size-1)/p_size);
    fal_fdb_entry_t *entry = (fal_fdb_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size +
		    (sizeof(*iterator)+p_size-1)/p_size);

    *iterator = 0;
    while (1)
    {
        arg_val[0] = SW_API_FDB_RESV_ITERATE;
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = (a_ulong_t) iterator;
        arg_val[4] = (a_ulong_t) entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
        dprintf("\n");
    }

    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
        cmd_print_error(rtn);
    else
        dprintf("\ntotal %d entries\n", cnt);

    return SW_OK;
}


sw_error_t
cmd_show_host(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_host_entry_t *host_entry = (fal_host_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(host_entry, sizeof (fal_host_entry_t));
    host_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_IP_HOST_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_ulong_t) host_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }

    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
        cmd_print_error(rtn);
    else
        dprintf("\ntotal %d entries\n", cnt);

    return SW_OK;
}

sw_error_t
cmd_show_host_ipv4(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_host_entry_t *host_entry = (fal_host_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(host_entry, sizeof (fal_host_entry_t));
    host_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_IP_HOST_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = FAL_IP_IP4_ADDR;
        arg_val[4] = (a_ulong_t) host_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }


    dprintf("\nipv4 total %d entries\n", cnt);

    return SW_OK;
}

sw_error_t
cmd_show_host_ipv6(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_host_entry_t *host_entry = (fal_host_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(host_entry, sizeof (fal_host_entry_t));
    host_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_IP_HOST_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = FAL_IP_IP6_ADDR;
        arg_val[4] = (a_ulong_t) host_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }


    dprintf("\nipv6 total %d entries\n", cnt);

    return SW_OK;
}

sw_error_t
cmd_show_host_ipv4M(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_host_entry_t *host_entry = (fal_host_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(host_entry, sizeof (fal_host_entry_t));
    host_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_IP_HOST_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = FAL_IP_IP4_ADDR_MCAST;
        arg_val[4] = (a_ulong_t) host_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }


    dprintf("\nipv4 multicast total %d entries\n", cnt);

    return SW_OK;
}

sw_error_t
cmd_show_host_ipv6M(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_host_entry_t *host_entry = (fal_host_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(host_entry, sizeof (fal_host_entry_t));
    host_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_IP_HOST_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = FAL_IP_IP6_ADDR_MCAST;
        arg_val[4] = (a_ulong_t) host_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }


    dprintf("\nipv6 multicast total %d entries\n", cnt);

    return SW_OK;
}

sw_error_t
cmd_show_flow_entry(a_ulong_t *arg_val, a_uint32_t type)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_flow_entry_t *flow_entry = (fal_flow_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(flow_entry, sizeof (fal_flow_entry_t));
    flow_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_FLOWENTRY_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = type;
        arg_val[4] = (a_ulong_t) flow_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }

    dprintf("\nflow total %d entries\n", cnt);

    return SW_OK;
}

sw_error_t
cmd_show_flow_ipv4_3tuple(a_ulong_t *arg_val)
{
    return cmd_show_flow_entry(arg_val, FAL_FLOW_IP4_3TUPLE_ADDR);
}

sw_error_t
cmd_show_flow_ipv4_5tuple(a_ulong_t *arg_val)
{
    return cmd_show_flow_entry(arg_val, FAL_FLOW_IP4_5TUPLE_ADDR);
}

sw_error_t
cmd_show_flow_ipv6_3tuple(a_ulong_t *arg_val)
{
    return cmd_show_flow_entry(arg_val, FAL_FLOW_IP6_3TUPLE_ADDR);
}

sw_error_t
cmd_show_flow_ipv6_5tuple(a_ulong_t *arg_val)
{
    return cmd_show_flow_entry(arg_val, FAL_FLOW_IP6_5TUPLE_ADDR);
}


sw_error_t
cmd_show_intfmac(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_intf_mac_entry_t *intfmac_entry =
	    (fal_intf_mac_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(intfmac_entry, sizeof (fal_intf_mac_entry_t));
    intfmac_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_IP_INTF_ENTRY_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_ulong_t) intfmac_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }

    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
        cmd_print_error(rtn);
    else
        dprintf("\ntotal %d entries\n", cnt);

    return SW_OK;
}

sw_error_t
cmd_show_pubaddr(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_nat_pub_addr_t *pubaddr_entry =
	    (fal_nat_pub_addr_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(pubaddr_entry, sizeof (fal_nat_pub_addr_t));
    pubaddr_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_PUB_ADDR_ENTRY_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_ulong_t) pubaddr_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }

    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
        cmd_print_error(rtn);
    else
        dprintf("\ntotal %d entries\n", cnt);

    return SW_OK;
}


sw_error_t
cmd_show_nat(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_nat_entry_t *nat_entry = (fal_nat_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(nat_entry, sizeof (fal_nat_entry_t));
    nat_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_NAT_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_ulong_t) nat_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }

    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
        cmd_print_error(rtn);
    else
        dprintf("\ntotal %d entries\n", cnt);

    return SW_OK;
}


sw_error_t
cmd_show_napt(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_napt_entry_t *napt_entry = (fal_napt_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(napt_entry, sizeof (fal_napt_entry_t));
    napt_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_NAPT_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_ulong_t) napt_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }

    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
        cmd_print_error(rtn);
    else
        dprintf("\ntotal %d entries\n", cnt);

    return SW_OK;
}

sw_error_t
cmd_show_flow(a_ulong_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint16_t rtn_size = sizeof(sw_error_t);
    a_uint16_t p_size = sizeof(a_ulong_t);
    fal_napt_entry_t *napt_entry = (fal_napt_entry_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

    aos_mem_zero(napt_entry, sizeof (fal_napt_entry_t));
    napt_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_FLOW_NEXT;

    while (1)
    {
        arg_val[1] = (a_ulong_t ) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_ulong_t ) napt_entry;

        rtn = cmd_exec_api(arg_val);
        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
        {
            break;
        }
        cnt++;
    }

    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
        cmd_print_error(rtn);
    else
        dprintf("\ntotal %d entries\n", cnt);

    return SW_OK;
}

sw_error_t
cmd_show_ptvlan_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t port_id, direction, cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_vlan_trans_adv_rule_t *vlan_rule = (fal_vlan_trans_adv_rule_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);
	fal_vlan_trans_adv_action_t *vlan_action = (fal_vlan_trans_adv_action_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size +
			(sizeof(fal_vlan_trans_adv_rule_t) + p_size - 1) / p_size);

	aos_mem_zero(vlan_rule, sizeof(fal_vlan_trans_adv_rule_t));
	aos_mem_zero(vlan_action, sizeof(fal_vlan_trans_adv_action_t));

	port_id = arg_val[1];
	direction = arg_val[2];
	cnt = 0;

	if (direction != FAL_PORT_VLAN_INGRESS &&
			direction != FAL_PORT_VLAN_EGRESS) {
		return SW_BAD_PARAM;
	}

	arg_val[0] = SW_API_PT_VLAN_TRANS_ADV_GETFIRST;

	while (1) {
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = port_id;
		arg_val[4] = direction;
		arg_val[5] = (a_ulong_t)vlan_rule;
		arg_val[6] = (a_ulong_t)vlan_action;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		arg_val[0] = SW_API_PT_VLAN_TRANS_ADV_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\nvlan total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_tunnel_decap_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_tunnel_decap_entry_t *decap_entry = (fal_tunnel_decap_entry_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(decap_entry, sizeof(fal_tunnel_decap_entry_t));

	cnt = 0;
	decap_entry->decap_rule.entry_id = 0;

	while (decap_entry->decap_rule.entry_id < FAL_TUNNEL_DECAP_ENTRY_MAX) {
		arg_val[0] = SW_API_TUNNEL_DECAP_ENTRY_GETNEXT;
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = FAL_TUNNEL_OP_MODE_INDEX;
		arg_val[4] = (a_ulong_t)decap_entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		cnt++;
		/* entry_id will be updated by cmd_exec_api */
		decap_entry->decap_rule.entry_id++;
	}

	if((rtn != SW_OK) && (rtn != SW_OUT_OF_RANGE)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\ndecap total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_tunnel_vlan_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_tunnel_vlan_intf_t *decap_vlan = (fal_tunnel_vlan_intf_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(decap_vlan, sizeof(fal_tunnel_vlan_intf_t));

	cnt = 0;
	arg_val[0] = SW_API_TUNNEL_VLAN_INTF_GETFIRST;

	while (1) {
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = (a_ulong_t)decap_vlan;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		cnt++;
		arg_val[0] = SW_API_TUNNEL_VLAN_INTF_GETNEXT;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\ndecap vlan total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_tunnel_encap_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t cnt, tunnel_id;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_tunnel_encap_cfg_t *encap_entry = (fal_tunnel_encap_cfg_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(encap_entry, sizeof(fal_tunnel_encap_cfg_t));

	cnt = 0;
	tunnel_id = 0;

	while (tunnel_id < FAL_TUNNEL_ENCAP_ENTRY_MAX) {
		arg_val[0] = SW_API_TUNNEL_ENCAP_ENTRY_GETNEXT;
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = tunnel_id;
		arg_val[4] = (a_ulong_t)encap_entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		/* tunnel_id will be updated by cmd_exec_api */
		tunnel_id = encap_entry->rt_tunnel_id;
		dprintf(encap_entry->rt_tlid_type ? "[tunnel_l3_if]:%d" : "[tunnel_vp_id]:%d",
				encap_entry->rt_tlid_index);
		dprintf(" [tunnel_id]:%d\n", tunnel_id);
		tunnel_id++;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_OUT_OF_RANGE)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\nencap total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_vxlan_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t vxlan_type, cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_tunnel_udp_entry_t *entry = (fal_tunnel_udp_entry_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(entry, sizeof(fal_tunnel_udp_entry_t));

	vxlan_type = arg_val[1];
	cnt = 0;

	if (vxlan_type != FAL_VXLAN &&
			vxlan_type != FAL_VXLAN_GPE) {
		return SW_BAD_PARAM;
	}

	arg_val[0] = SW_API_VXLAN_ENTRY_GETFIRST;

	while (1) {
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = vxlan_type;
		arg_val[4] = (a_ulong_t)entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		arg_val[0] = SW_API_VXLAN_ENTRY_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\nvxlan total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_geneve_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_tunnel_udp_entry_t *entry = (fal_tunnel_udp_entry_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(entry, sizeof(fal_tunnel_udp_entry_t));

	cnt = 0;

	arg_val[0] = SW_API_GENEVE_ENTRY_GETFIRST;

	while (1) {
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = (a_ulong_t)entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		arg_val[0] = SW_API_GENEVE_ENTRY_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\ngeneve total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_mapt_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_mapt_decap_entry_t *decap_entry = (fal_mapt_decap_entry_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(decap_entry, sizeof(fal_mapt_decap_entry_t));

	cnt = 0;
	arg_val[0] = SW_API_MAPT_DECAP_ENTRY_GETFIRST;

	while (1) {
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = (a_ulong_t)decap_entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}

		arg_val[0] = SW_API_MAPT_DECAP_ENTRY_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\nmapt total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_tunnel_program_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t program_type, cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_tunnel_program_entry_t *entry = (fal_tunnel_program_entry_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(entry, sizeof(fal_tunnel_program_entry_t));

	program_type = arg_val[1];
	cnt = 0;

	if (program_type > FAL_TUNNEL_PROGRAM_TYPE_5) {
		return SW_BAD_PARAM;
	}

	arg_val[0] = SW_API_TUNNEL_PROGRAM_ENTRY_GETFIRST;

	while (1) {
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = program_type;
		arg_val[4] = (a_ulong_t)entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		arg_val[0] = SW_API_TUNNEL_PROGRAM_ENTRY_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\ntunnel program total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_tunnel_program_udf(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t program_type, cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_tunnel_program_udf_t *entry = (fal_tunnel_program_udf_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(entry, sizeof(fal_tunnel_program_udf_t));

	program_type = arg_val[1];
	cnt = 0;

	if (program_type > FAL_TUNNEL_PROGRAM_TYPE_5) {
		return SW_BAD_PARAM;
	}

	arg_val[0] = SW_API_TUNNEL_PROGRAM_UDF_GETFIRST;

	while (1) {
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = program_type;
		arg_val[4] = (a_ulong_t)entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		arg_val[0] = SW_API_TUNNEL_PROGRAM_UDF_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\ntunnel program udf total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_tunnel_udf_profile_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t profile_id, cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_tunnel_udf_profile_entry_t *entry = (fal_tunnel_udf_profile_entry_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(entry, sizeof(fal_tunnel_udf_profile_entry_t));

	profile_id = arg_val[1];
	cnt = 0;

	arg_val[0] = SW_API_TUNNEL_UDF_PROFILE_ENTRY_GETFIRST;

	while (1) {
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = profile_id;
		arg_val[4] = (a_ulong_t)entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		arg_val[0] = SW_API_TUNNEL_UDF_PROFILE_ENTRY_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\ntunnel udf profile total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_acl_udf_profile_entry(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t profile_id, cnt;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_acl_udf_profile_entry_t *entry = (fal_acl_udf_profile_entry_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);

	aos_mem_zero(entry, sizeof(fal_acl_udf_profile_entry_t));

	profile_id = arg_val[1];
	cnt = 0;

	arg_val[0] = SW_API_ACL_UDF_PROFILE_ENTRY_GETFIRST;

	while (1) {
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = profile_id;
		arg_val[4] = (a_ulong_t)entry;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		arg_val[0] = SW_API_ACL_UDF_PROFILE_ENTRY_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\nacl udf profile total %d entries\n", cnt);
	}

	return SW_OK;
}

sw_error_t
cmd_show_ptp_timestamp(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t port_id, direction, seq_id;
	a_uint32_t p_size = sizeof(a_ulong_t);

	fal_ptp_pkt_info_t *pkt_info = (fal_ptp_pkt_info_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size);
	fal_ptp_time_t *ptp_time = (fal_ptp_time_t *)(ioctl_buf +
			(sizeof(sw_error_t) + p_size - 1) / p_size +
			(sizeof(fal_ptp_pkt_info_t) + p_size - 1) / p_size);

	aos_mem_zero(pkt_info, sizeof(fal_ptp_pkt_info_t));
	aos_mem_zero(ptp_time, sizeof(fal_ptp_time_t));

	port_id = arg_val[1];
	direction = arg_val[2];
	seq_id = 0;

	if (direction != FAL_RX_DIRECTION &&
			direction != FAL_TX_DIRECTION) {
		return SW_BAD_PARAM;
	}

	while (seq_id < 4) {
		pkt_info->msg_type = FAL_PTP_MSG_INVALID;
		pkt_info->sequence_id = seq_id;

		arg_val[0] = SW_API_PTP_TIMESTAMP_GET;
		arg_val[1] = (a_ulong_t)ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = port_id;
		arg_val[4] = direction;
		arg_val[5] = (a_ulong_t)pkt_info;
		arg_val[6] = (a_ulong_t)ptp_time;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t)(*ioctl_buf))) {
			break;
		}
		seq_id++;
	}

	if((rtn != SW_OK) && (rtn != SW_NOT_FOUND)) {
		cmd_print_error(rtn);
	} else {
		dprintf("\ntimestamped total %d entries\n", seq_id);
	}

	return SW_OK;
}

sw_error_t
cmd_show_toeplitz_hash_config(a_ulong_t *arg_val)
{
	sw_error_t rtn;
	a_uint32_t cnt = 0;
	a_uint16_t rtn_size = sizeof(sw_error_t);
	a_uint16_t p_size = sizeof(a_ulong_t);
	fal_toeplitz_hash_config_t *cfg =
		(fal_toeplitz_hash_config_t *)(ioctl_buf+(rtn_size+p_size-1)/p_size);

	aos_mem_zero(cfg, sizeof (fal_toeplitz_hash_config_t));
	arg_val[0] = SW_API_TOEPLITZ_HASH_CONFIG_GETFIRST;

	while (1)
	{
		arg_val[1] = (a_ulong_t) ioctl_buf;
		arg_val[2] = get_devid();
		arg_val[3] = (a_ulong_t) cfg;

		rtn = cmd_exec_api(arg_val);
		if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
		{
			break;
		}
		arg_val[0] = SW_API_TOEPLITZ_HASH_CONFIG_GETNEXT;
		cnt++;
	}

	if((rtn != SW_OK) && (rtn != SW_NO_MORE))
		cmd_print_error(rtn);
	else
		dprintf("\ntotal %d entries\n", cnt);

	return SW_OK;
}


