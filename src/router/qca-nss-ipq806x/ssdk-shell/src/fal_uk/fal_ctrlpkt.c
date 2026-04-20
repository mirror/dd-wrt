/*
 * Copyright (c) 2016-2017, 2021, The Linux Foundation. All rights reserved.
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



#include "sw.h"
#include "sw_ioctl.h"
#include "fal_ctrlpkt.h"
#include "fal_uk_if.h"

sw_error_t
fal_mgmtctrl_ethtype_profile_set(a_uint32_t dev_id, a_uint32_t profile_id, a_uint32_t ethtype)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_ETHTYPE_PROFILE_SET, dev_id, profile_id,
                    (a_uint32_t) ethtype);
    return rv;
}

sw_error_t
fal_mgmtctrl_ethtype_profile_get(a_uint32_t dev_id, a_uint32_t profile_id, a_uint32_t * ethtype)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_ETHTYPE_PROFILE_GET, dev_id, profile_id,
                    ethtype);
    return rv;
}

sw_error_t
fal_mgmtctrl_rfdb_profile_set(a_uint32_t dev_id, a_uint32_t profile_id, fal_mac_addr_t *addr)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_RFDB_PROFILE_SET, dev_id, profile_id,
                    addr);
    return rv;
}

sw_error_t
fal_mgmtctrl_rfdb_profile_get(a_uint32_t dev_id, a_uint32_t profile_id, fal_mac_addr_t *addr)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_RFDB_PROFILE_GET, dev_id, profile_id,
                    addr);
    return rv;
}

sw_error_t
fal_mgmtctrl_ctrlpkt_profile_add(a_uint32_t dev_id, fal_ctrlpkt_profile_t *ctrlpkt)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_CTRLPKT_PROFILE_ADD, dev_id, ctrlpkt);
    return rv;
}

sw_error_t
fal_mgmtctrl_ctrlpkt_profile_del(a_uint32_t dev_id, fal_ctrlpkt_profile_t *ctrlpkt)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_CTRLPKT_PROFILE_DEL, dev_id, ctrlpkt);
    return rv;
}

sw_error_t
fal_mgmtctrl_ctrlpkt_profile_getfirst(a_uint32_t dev_id, fal_ctrlpkt_profile_t *ctrlpkt)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_CTRLPKT_PROFILE_GETFIRST, dev_id, ctrlpkt);
    return rv;
}

sw_error_t
fal_mgmtctrl_ctrlpkt_profile_getnext(a_uint32_t dev_id, fal_ctrlpkt_profile_t *ctrlpkt)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_CTRLPKT_PROFILE_GETNEXT, dev_id, ctrlpkt);
    return rv;
}

sw_error_t
fal_mgmtctrl_vpgroup_set(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t vpgroup_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_VPGROUP_SET, dev_id, port_id, vpgroup_id);
    return rv;
}

sw_error_t
fal_mgmtctrl_vpgroup_get(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t *vpgroup_id)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_VPGROUP_GET, dev_id, port_id, vpgroup_id);
    return rv;
}

sw_error_t
fal_mgmtctrl_tunnel_decap_set(a_uint32_t dev_id, a_uint32_t cpu_code_id, a_bool_t enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_TUNNEL_DECAP_SET, dev_id, cpu_code_id, enable);
    return rv;
}

sw_error_t
fal_mgmtctrl_tunnel_decap_get(a_uint32_t dev_id, a_uint32_t cpu_code_id, a_bool_t *enable)
{
    sw_error_t rv;

    rv = sw_uk_exec(SW_API_MGMTCTRL_TUNNEL_DECAP_GET, dev_id, cpu_code_id, enable);
    return rv;
}
