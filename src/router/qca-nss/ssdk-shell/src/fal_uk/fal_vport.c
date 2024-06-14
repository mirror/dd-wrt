/*
* Copyright (c) 2021, The Linux Foundation. All rights reserved.
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


#include "sw.h"
#include "sw_ioctl.h"
#include "fal_vport.h"
#include "fal_uk_if.h"

sw_error_t
fal_vport_physical_port_id_set(a_uint32_t dev_id,
		fal_port_t vport_id, fal_port_t physical_port_id)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_VPORT_PHYSICAL_PORT_SET, dev_id, vport_id, physical_port_id);

	return rv;
}

sw_error_t
fal_vport_physical_port_id_get(a_uint32_t dev_id,
		fal_port_t vport_id, fal_port_t *physical_port_id)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_VPORT_PHYSICAL_PORT_GET, dev_id, vport_id, physical_port_id);

	return rv;
}

sw_error_t
fal_vport_state_check_set(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_VPORT_STATE_CHECK_SET, dev_id, port_id, vp_state);

	return rv;
}

sw_error_t
fal_vport_state_check_get(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_VPORT_STATE_CHECK_GET, dev_id, port_id, vp_state);

	return rv;
}
