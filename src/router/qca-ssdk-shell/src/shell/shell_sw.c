/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

#include <stdio.h>
#include "shell.h"
#include "fal.h"

static int sw_devid = 0;

sw_error_t
cmd_set_devid(a_uint32_t *arg_val)
{
    if (arg_val[1] >= SW_MAX_NR_DEV)
    {
        dprintf("dev_id should be less than <%d>\n", SW_MAX_NR_DEV);
        return SW_FAIL;
    }
    sw_devid = arg_val[1];

    return SW_OK;
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

sw_error_t
cmd_show_fdb(a_uint32_t *arg_val)
{
    if (ssdk_cfg.init_cfg.chip_type == CHIP_ISIS) {
	    sw_error_t rtn;
	    a_uint32_t cnt = 0;
	    fal_fdb_op_t    *fdb_op    = (fal_fdb_op_t *)    (ioctl_buf + sizeof(sw_error_t) / 4);
	    fal_fdb_entry_t *fdb_entry = (fal_fdb_entry_t *) (ioctl_buf + sizeof(sw_error_t) / 4 + sizeof(fal_fdb_op_t) / 4);

	    aos_mem_zero(fdb_op,    sizeof (fal_fdb_op_t));
	    aos_mem_zero(fdb_entry, sizeof (fal_fdb_entry_t));
	    arg_val[0] = SW_API_FDB_EXTEND_FIRST;

	    while (1)
	    {
	        arg_val[1] = (a_uint32_t) ioctl_buf;
	        arg_val[2] = get_devid();
	        arg_val[3] = (a_uint32_t) fdb_op;
	        arg_val[4] = (a_uint32_t) fdb_entry;

	        rtn = cmd_exec_api(arg_val);
	        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
	        {
	            break;
	        }
	        arg_val[0] = SW_API_FDB_EXTEND_NEXT;
	        cnt++;
	    }

	    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
	        cmd_print_error(rtn);
	    else
	        dprintf("\ntotal %d entries\n", cnt);
    }else if ((ssdk_cfg.init_cfg.chip_type == CHIP_ISISC) ||
		(ssdk_cfg.init_cfg.chip_type == CHIP_DESS)) {
	    sw_error_t rtn;
	    a_uint32_t cnt = 0;
	    fal_fdb_op_t    *fdb_op    = (fal_fdb_op_t *)    (ioctl_buf + sizeof(sw_error_t) / 4);
	    fal_fdb_entry_t *fdb_entry = (fal_fdb_entry_t *) (ioctl_buf + sizeof(sw_error_t) / 4 + sizeof(fal_fdb_op_t) / 4);

	    aos_mem_zero(fdb_op,    sizeof (fal_fdb_op_t));
	    aos_mem_zero(fdb_entry, sizeof (fal_fdb_entry_t));
	    arg_val[0] = SW_API_FDB_EXTEND_FIRST;

	    while (1)
	    {
	        arg_val[1] = (a_uint32_t) ioctl_buf;
	        arg_val[2] = get_devid();
	        arg_val[3] = (a_uint32_t) fdb_op;
	        arg_val[4] = (a_uint32_t) fdb_entry;

	        rtn = cmd_exec_api(arg_val);
	        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
	        {
	            break;
	        }
	        arg_val[0] = SW_API_FDB_EXTEND_NEXT;
	        cnt++;
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
	        arg_val[1] = (a_uint32_t) ioctl_buf;
	        arg_val[2] = get_devid();
	        arg_val[3] = (a_uint32_t) (ioctl_buf + 1);
	        arg_val[4] = (a_uint32_t) fdb_entry;

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
	    a_uint32_t rtn_size = 1, cnt = 0;
	    fal_fdb_entry_t *fdb_entry = (fal_fdb_entry_t *) (ioctl_buf + rtn_size);

	    memset(fdb_entry, 0, sizeof (fal_fdb_entry_t));
	    arg_val[0] = SW_API_FDB_FIRST;

	    while (1)
	    {
	        arg_val[1] = (a_uint32_t) ioctl_buf;
	        arg_val[2] = get_devid();
	        arg_val[3] = (a_uint32_t) fdb_entry;

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
cmd_show_vlan(a_uint32_t *arg_val)
{
    if (ssdk_cfg.init_cfg.chip_type == CHIP_ISIS) {
	    sw_error_t rtn;
	    a_uint32_t rtn_size = 1 ,tmp_vid = FAL_NEXT_ENTRY_FIRST_ID, cnt = 0;
	    fal_vlan_t *vlan_entry = (fal_vlan_t *) (ioctl_buf + rtn_size);

	    while (1)
	    {
	        arg_val[0] = SW_API_VLAN_NEXT;
	        arg_val[1] = (a_uint32_t) ioctl_buf;
	        arg_val[2] = get_devid();
	        arg_val[3] = tmp_vid;
	        arg_val[4] = (a_uint32_t) vlan_entry;

	        rtn = cmd_exec_api(arg_val);
	        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
	        {
	            break;
	        }

	        tmp_vid = vlan_entry->vid;
	        cnt++;
	    }

	    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
	        cmd_print_error(rtn);
	    else
	        dprintf("\ntotal %d entries\n", cnt);
    }else if ((ssdk_cfg.init_cfg.chip_type == CHIP_ISISC) ||
		(ssdk_cfg.init_cfg.chip_type == CHIP_DESS)) {
	    sw_error_t rtn;
	    a_uint32_t rtn_size = 1 ,tmp_vid = FAL_NEXT_ENTRY_FIRST_ID, cnt = 0;
	    fal_vlan_t *vlan_entry = (fal_vlan_t *) (ioctl_buf + rtn_size);

	    while (1)
	    {
	        arg_val[0] = SW_API_VLAN_NEXT;
	        arg_val[1] = (a_uint32_t) ioctl_buf;
	        arg_val[2] = get_devid();
	        arg_val[3] = tmp_vid;
	        arg_val[4] = (a_uint32_t) vlan_entry;

	        rtn = cmd_exec_api(arg_val);
	        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
	        {
	            break;
	        }

	        tmp_vid = vlan_entry->vid;
	        cnt++;
	    }

	    if((rtn != SW_OK) && (rtn != SW_NO_MORE))
	        cmd_print_error(rtn);
	    else
	        dprintf("\ntotal %d entries\n", cnt);
    } else {
	    sw_error_t rtn;
	    a_uint32_t rtn_size = 1 ,tmp_vid = 0, cnt = 0;
	    fal_vlan_t *vlan_entry = (fal_vlan_t *) (ioctl_buf + rtn_size);

	    while (1)
	    {
	        arg_val[0] = SW_API_VLAN_NEXT;
	        arg_val[1] = (a_uint32_t) ioctl_buf;
	        arg_val[2] = get_devid();
	        arg_val[3] = tmp_vid;
	        arg_val[4] = (a_uint32_t) vlan_entry;

	        rtn = cmd_exec_api(arg_val);
	        if ((SW_OK != rtn)  || (SW_OK != (sw_error_t) (*ioctl_buf)))
	        {
	            break;
	        }

	        tmp_vid = vlan_entry->vid;
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
cmd_show_resv_fdb(a_uint32_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    a_uint32_t  *iterator  = ioctl_buf + 1;
    fal_fdb_entry_t *entry = (fal_fdb_entry_t *) (ioctl_buf + 2);

    *iterator = 0;
    while (1)
    {
        arg_val[0] = SW_API_FDB_RESV_ITERATE;
        arg_val[1] = (a_uint32_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = (a_uint32_t) iterator;
        arg_val[4] = (a_uint32_t) entry;

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
cmd_show_host(a_uint32_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    fal_host_entry_t *host_entry = (fal_host_entry_t *) (ioctl_buf + sizeof(sw_error_t) / 4);

    aos_mem_zero(host_entry, sizeof (fal_host_entry_t));
    host_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_IP_HOST_NEXT;

    while (1)
    {
        arg_val[1] = (a_uint32_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_uint32_t) host_entry;

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
cmd_show_intfmac(a_uint32_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    fal_intf_mac_entry_t *intfmac_entry = (fal_intf_mac_entry_t *) (ioctl_buf + sizeof(sw_error_t) / 4);

    aos_mem_zero(intfmac_entry, sizeof (fal_intf_mac_entry_t));
    intfmac_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_IP_INTF_ENTRY_NEXT;

    while (1)
    {
        arg_val[1] = (a_uint32_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_uint32_t) intfmac_entry;

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
cmd_show_pubaddr(a_uint32_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    fal_nat_pub_addr_t *pubaddr_entry = (fal_nat_pub_addr_t *) (ioctl_buf + sizeof(sw_error_t) / 4);

    aos_mem_zero(pubaddr_entry, sizeof (fal_nat_pub_addr_t));
    pubaddr_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_PUB_ADDR_ENTRY_NEXT;

    while (1)
    {
        arg_val[1] = (a_uint32_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_uint32_t) pubaddr_entry;

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
cmd_show_nat(a_uint32_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    fal_nat_entry_t *nat_entry = (fal_nat_entry_t *) (ioctl_buf + sizeof(sw_error_t) / 4);

    aos_mem_zero(nat_entry, sizeof (fal_nat_entry_t));
    nat_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_NAT_NEXT;

    while (1)
    {
        arg_val[1] = (a_uint32_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_uint32_t) nat_entry;

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
cmd_show_napt(a_uint32_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    fal_napt_entry_t *napt_entry = (fal_napt_entry_t *) (ioctl_buf + sizeof(sw_error_t) / 4);

    aos_mem_zero(napt_entry, sizeof (fal_napt_entry_t));
    napt_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_NAPT_NEXT;

    while (1)
    {
        arg_val[1] = (a_uint32_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_uint32_t) napt_entry;

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
cmd_show_flow(a_uint32_t *arg_val)
{
    sw_error_t rtn;
    a_uint32_t cnt = 0;
    fal_napt_entry_t *napt_entry = (fal_napt_entry_t *) (ioctl_buf + sizeof(sw_error_t) / 4);

    aos_mem_zero(napt_entry, sizeof (fal_napt_entry_t));
    napt_entry->entry_id = FAL_NEXT_ENTRY_FIRST_ID;
    arg_val[0] = SW_API_FLOW_NEXT;

    while (1)
    {
        arg_val[1] = (a_uint32_t) ioctl_buf;
        arg_val[2] = get_devid();
        arg_val[3] = 0;
        arg_val[4] = (a_uint32_t) napt_entry;

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

