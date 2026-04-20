/*
* Copyright (c) 2021, The Linux Foundation. All rights reserved.
* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "fal_tunnel.h"
#include "fal_mapt.h"
#include "fal_uk_if.h"

sw_error_t
fal_mapt_decap_ctrl_set(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_CTRL_SET, dev_id, decap_ctrl);

	return rv;
}

sw_error_t
fal_mapt_decap_ctrl_get(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_CTRL_GET, dev_id, decap_ctrl);

	return rv;
}

sw_error_t
fal_mapt_decap_rule_entry_set(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_RULE_ENTRY_SET, dev_id, rule_id, mapt_rule_entry);

	return rv;
}

sw_error_t
fal_mapt_decap_rule_entry_get(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_RULE_ENTRY_GET, dev_id, rule_id, mapt_rule_entry);

	return rv;
}

sw_error_t
fal_mapt_decap_rule_entry_del(a_uint32_t dev_id, a_uint32_t rule_id,
		fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_RULE_ENTRY_DEL, dev_id, rule_id, mapt_rule_entry);

	return rv;
}

sw_error_t
fal_mapt_decap_entry_add(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_ENTRY_ADD, dev_id, mapt_entry);

	return rv;
}

sw_error_t
fal_mapt_decap_entry_del(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_ENTRY_DEL, dev_id, mapt_entry);

	return rv;
}

sw_error_t
fal_mapt_decap_entry_getfirst(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_ENTRY_GETFIRST, dev_id, mapt_entry);

	return rv;
}

sw_error_t
fal_mapt_decap_entry_getnext(a_uint32_t dev_id, fal_mapt_decap_entry_t *mapt_entry)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_ENTRY_GETNEXT, dev_id, mapt_entry);

	return rv;
}

sw_error_t
fal_mapt_decap_en_set(a_uint32_t dev_id, a_uint32_t mapt_index, a_bool_t en)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_EN_SET, dev_id, mapt_index, en);

	return rv;
}

sw_error_t
fal_mapt_decap_en_get(a_uint32_t dev_id, a_uint32_t mapt_index, a_bool_t *en)
{
	sw_error_t rv;

	rv = sw_uk_exec(SW_API_MAPT_DECAP_EN_GET, dev_id, mapt_index, en);

	return rv;
}
