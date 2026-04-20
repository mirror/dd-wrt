/*
 * Copyright (c) 2016-2017, 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hppe_reg_access.h"
#include "hppe_acl_reg.h"
#include "hppe_acl.h"

sw_error_t
hppe_non_ip_udf0_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + NON_IP_UDF0_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_non_ip_udf0_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + NON_IP_UDF0_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_non_ip_udf1_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + NON_IP_UDF1_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_non_ip_udf1_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + NON_IP_UDF1_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_non_ip_udf2_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + NON_IP_UDF2_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_non_ip_udf2_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + NON_IP_UDF2_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_non_ip_udf3_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + NON_IP_UDF3_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_non_ip_udf3_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + NON_IP_UDF3_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipv4_udf0_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV4_UDF0_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipv4_udf0_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV4_UDF0_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipv4_udf1_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV4_UDF1_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipv4_udf1_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV4_UDF1_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipv4_udf2_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV4_UDF2_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipv4_udf2_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV4_UDF2_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipv4_udf3_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV4_UDF3_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipv4_udf3_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV4_UDF3_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipv6_udf0_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV6_UDF0_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipv6_udf0_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV6_UDF0_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipv6_udf1_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV6_UDF1_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipv6_udf1_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV6_UDF1_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipv6_udf2_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV6_UDF2_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipv6_udf2_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV6_UDF2_CTRL_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipv6_udf3_ctrl_reg_get(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV6_UDF3_CTRL_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipv6_udf3_ctrl_reg_set(
		a_uint32_t dev_id,
		union udf_ctrl_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + IPV6_UDF3_CTRL_REG_ADDRESS,
				value->val);
}

#if 0
sw_error_t
hppe_non_ip_udf0_ctrl_reg_udf0_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf0_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_non_ip_udf0_ctrl_reg_udf0_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf0_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_non_ip_udf0_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_non_ip_udf0_ctrl_reg_udf0_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf0_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_non_ip_udf0_ctrl_reg_udf0_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf0_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_non_ip_udf0_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_non_ip_udf1_ctrl_reg_udf1_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf1_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_non_ip_udf1_ctrl_reg_udf1_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf1_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_non_ip_udf1_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_non_ip_udf1_ctrl_reg_udf1_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf1_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_non_ip_udf1_ctrl_reg_udf1_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf1_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_non_ip_udf1_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_non_ip_udf2_ctrl_reg_udf2_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf2_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_non_ip_udf2_ctrl_reg_udf2_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf2_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_non_ip_udf2_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_non_ip_udf2_ctrl_reg_udf2_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf2_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_non_ip_udf2_ctrl_reg_udf2_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf2_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_non_ip_udf2_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_non_ip_udf3_ctrl_reg_udf3_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf3_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_non_ip_udf3_ctrl_reg_udf3_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf3_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_non_ip_udf3_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_non_ip_udf3_ctrl_reg_udf3_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf3_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_non_ip_udf3_ctrl_reg_udf3_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_non_ip_udf3_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_non_ip_udf3_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv4_udf0_ctrl_reg_udf0_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf0_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_ipv4_udf0_ctrl_reg_udf0_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf0_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_ipv4_udf0_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv4_udf0_ctrl_reg_udf0_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf0_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_ipv4_udf0_ctrl_reg_udf0_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf0_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_ipv4_udf0_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv4_udf1_ctrl_reg_udf1_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf1_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_ipv4_udf1_ctrl_reg_udf1_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf1_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_ipv4_udf1_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv4_udf1_ctrl_reg_udf1_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf1_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_ipv4_udf1_ctrl_reg_udf1_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf1_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_ipv4_udf1_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv4_udf2_ctrl_reg_udf2_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf2_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_ipv4_udf2_ctrl_reg_udf2_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf2_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_ipv4_udf2_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv4_udf2_ctrl_reg_udf2_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf2_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_ipv4_udf2_ctrl_reg_udf2_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf2_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_ipv4_udf2_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv4_udf3_ctrl_reg_udf3_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf3_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_ipv4_udf3_ctrl_reg_udf3_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf3_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_ipv4_udf3_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv4_udf3_ctrl_reg_udf3_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf3_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_ipv4_udf3_ctrl_reg_udf3_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv4_udf3_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_ipv4_udf3_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv6_udf0_ctrl_reg_udf0_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf0_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_ipv6_udf0_ctrl_reg_udf0_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf0_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_ipv6_udf0_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv6_udf0_ctrl_reg_udf0_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf0_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_ipv6_udf0_ctrl_reg_udf0_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf0_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_ipv6_udf0_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv6_udf1_ctrl_reg_udf1_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf1_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_ipv6_udf1_ctrl_reg_udf1_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf1_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_ipv6_udf1_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv6_udf1_ctrl_reg_udf1_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf1_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_ipv6_udf1_ctrl_reg_udf1_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf1_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_ipv6_udf1_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv6_udf2_ctrl_reg_udf2_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf2_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_ipv6_udf2_ctrl_reg_udf2_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf2_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_ipv6_udf2_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv6_udf2_ctrl_reg_udf2_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf2_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_ipv6_udf2_ctrl_reg_udf2_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf2_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_ipv6_udf2_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv6_udf3_ctrl_reg_udf3_base_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf3_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_base;
	return ret;
}

sw_error_t
hppe_ipv6_udf3_ctrl_reg_udf3_base_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf3_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_base = value;
	ret = hppe_ipv6_udf3_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_ipv6_udf3_ctrl_reg_udf3_offset_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf3_ctrl_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.udf_offset;
	return ret;
}

sw_error_t
hppe_ipv6_udf3_ctrl_reg_udf3_offset_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union udf_ctrl_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipv6_udf3_ctrl_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf_offset = value;
	ret = hppe_ipv6_udf3_ctrl_reg_set(dev_id, &reg_val);
	return ret;
}
#endif

sw_error_t
hppe_ipo_rule_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_rule_reg_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_RULE_REG_ADDRESS + \
				index * IPO_RULE_REG_INC,
				value->val,
				3);
}

sw_error_t
hppe_ipo_rule_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_rule_reg_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_RULE_REG_ADDRESS + \
				index * IPO_RULE_REG_INC,
				value->val,
				3);
}

sw_error_t
hppe_ipo_mask_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_mask_reg_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_MASK_REG_ADDRESS + \
				index * IPO_MASK_REG_INC,
				value->val,
				2);
}

sw_error_t
hppe_ipo_mask_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_mask_reg_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_MASK_REG_ADDRESS + \
				index * IPO_MASK_REG_INC,
				value->val,
				2);
}

sw_error_t
hppe_rule_ext_1_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_1_reg_u *value)
{
	if (index >= RULE_EXT_1_REG_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPO_CSR_BASE_ADDR + RULE_EXT_1_REG_ADDRESS + \
				index * RULE_EXT_1_REG_INC,
				&value->val);
}

sw_error_t
hppe_rule_ext_1_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_1_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPO_CSR_BASE_ADDR + RULE_EXT_1_REG_ADDRESS + \
				index * RULE_EXT_1_REG_INC,
				value->val);
}

sw_error_t
hppe_rule_ext_2_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_2_reg_u *value)
{
	if (index >= RULE_EXT_2_REG_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPO_CSR_BASE_ADDR + RULE_EXT_2_REG_ADDRESS + \
				index * RULE_EXT_2_REG_INC,
				&value->val);
}

sw_error_t
hppe_rule_ext_2_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_2_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPO_CSR_BASE_ADDR + RULE_EXT_2_REG_ADDRESS + \
				index * RULE_EXT_2_REG_INC,
				value->val);
}

sw_error_t
hppe_rule_ext_4_reg_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_4_reg_u *value)
{
	if (index >= RULE_EXT_4_REG_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPO_CSR_BASE_ADDR + RULE_EXT_4_REG_ADDRESS + \
				index * RULE_EXT_4_REG_INC,
				&value->val);
}

sw_error_t
hppe_rule_ext_4_reg_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union rule_ext_4_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPO_CSR_BASE_ADDR + RULE_EXT_4_REG_ADDRESS + \
				index * RULE_EXT_4_REG_INC,
				value->val);
}

#if 0
sw_error_t
hppe_ipo_dbg_addr_reg_get(
		a_uint32_t dev_id,
		union ipo_dbg_addr_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_DBG_ADDR_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipo_dbg_addr_reg_set(
		a_uint32_t dev_id,
		union ipo_dbg_addr_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_DBG_ADDR_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipo_dbg_data_reg_get(
		a_uint32_t dev_id,
		union ipo_dbg_data_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_DBG_DATA_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipo_dbg_data_reg_set(
		a_uint32_t dev_id,
		union ipo_dbg_data_reg_u *value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_ipo_spare_reg_reg_get(
		a_uint32_t dev_id,
		union ipo_spare_reg_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_SPARE_REG_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipo_spare_reg_reg_set(
		a_uint32_t dev_id,
		union ipo_spare_reg_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_SPARE_REG_REG_ADDRESS,
				value->val);
}

sw_error_t
hppe_ipo_glb_hit_counter_reg_get(
		a_uint32_t dev_id,
		union ipo_glb_hit_counter_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_GLB_HIT_COUNTER_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipo_glb_hit_counter_reg_set(
		a_uint32_t dev_id,
		union ipo_glb_hit_counter_reg_u *value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_ipo_glb_miss_counter_reg_get(
		a_uint32_t dev_id,
		union ipo_glb_miss_counter_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_GLB_MISS_COUNTER_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipo_glb_miss_counter_reg_set(
		a_uint32_t dev_id,
		union ipo_glb_miss_counter_reg_u *value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_ipo_glb_bypass_counter_reg_get(
		a_uint32_t dev_id,
		union ipo_glb_bypass_counter_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPO_CSR_BASE_ADDR + IPO_GLB_BYPASS_COUNTER_REG_ADDRESS,
				&value->val);
}

sw_error_t
hppe_ipo_glb_bypass_counter_reg_set(
		a_uint32_t dev_id,
		union ipo_glb_bypass_counter_reg_u *value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_rule_ext_1_reg_ext2_2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union rule_ext_1_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_1_reg_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ext2_2;
	return ret;
}

sw_error_t
hppe_rule_ext_1_reg_ext2_2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union rule_ext_1_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_1_reg_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ext2_2 = value;
	ret = hppe_rule_ext_1_reg_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_rule_ext_1_reg_ext2_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union rule_ext_1_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_1_reg_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ext2_0;
	return ret;
}

sw_error_t
hppe_rule_ext_1_reg_ext2_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union rule_ext_1_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_1_reg_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ext2_0 = value;
	ret = hppe_rule_ext_1_reg_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_rule_ext_1_reg_ext2_3_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union rule_ext_1_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_1_reg_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ext2_3;
	return ret;
}

sw_error_t
hppe_rule_ext_1_reg_ext2_3_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union rule_ext_1_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_1_reg_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ext2_3 = value;
	ret = hppe_rule_ext_1_reg_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_rule_ext_1_reg_ext2_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union rule_ext_1_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_1_reg_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ext2_1;
	return ret;
}

sw_error_t
hppe_rule_ext_1_reg_ext2_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union rule_ext_1_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_1_reg_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ext2_1 = value;
	ret = hppe_rule_ext_1_reg_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_rule_ext_2_reg_ext4_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union rule_ext_2_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_2_reg_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ext4_0;
	return ret;
}

sw_error_t
hppe_rule_ext_2_reg_ext4_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union rule_ext_2_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_2_reg_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ext4_0 = value;
	ret = hppe_rule_ext_2_reg_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_rule_ext_2_reg_ext4_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union rule_ext_2_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_2_reg_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ext4_1;
	return ret;
}

sw_error_t
hppe_rule_ext_2_reg_ext4_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union rule_ext_2_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_2_reg_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ext4_1 = value;
	ret = hppe_rule_ext_2_reg_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_rule_ext_4_reg_ext8_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union rule_ext_4_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_4_reg_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ext8;
	return ret;
}

sw_error_t
hppe_rule_ext_4_reg_ext8_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union rule_ext_4_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_rule_ext_4_reg_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ext8 = value;
	ret = hppe_rule_ext_4_reg_set(dev_id, index, &reg_val);
	return ret;
}
#endif

sw_error_t
hppe_ipo_cnt_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_cnt_tbl_u *value)
{
	if (index >= IPO_CNT_TBL_NUM)
		return SW_OUT_OF_RANGE;
	return hppe_reg_tbl_get(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + IPO_CNT_TBL_ADDRESS + \
				index * IPO_CNT_TBL_INC,
				value->val,
				3);
}

sw_error_t
hppe_ipo_cnt_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_cnt_tbl_u *value)
{
	if (index >= IPO_CNT_TBL_NUM)
		return SW_OUT_OF_RANGE;
	return hppe_reg_tbl_set(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + IPO_CNT_TBL_ADDRESS + \
				index * IPO_CNT_TBL_INC,
				value->val,
				3);
}

#if 0
sw_error_t
hppe_ipo_cnt_tbl_hit_byte_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value)
{
	union ipo_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipo_cnt_tbl_get(dev_id, index, &reg_val);
	*value = (a_uint64_t)reg_val.bf.hit_byte_cnt_1 << 32 | \
		reg_val.bf.hit_byte_cnt_0;
	return ret;
}

sw_error_t
hppe_ipo_cnt_tbl_hit_byte_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value)
{
	union ipo_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipo_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hit_byte_cnt_1 = value >> 32;
	reg_val.bf.hit_byte_cnt_0 = value & (((a_uint64_t)1<<32)-1);
	ret = hppe_ipo_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_ipo_cnt_tbl_hit_pkt_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union ipo_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipo_cnt_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.hit_pkt_cnt;
	return ret;
}

sw_error_t
hppe_ipo_cnt_tbl_hit_pkt_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union ipo_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_ipo_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hit_pkt_cnt = value;
	ret = hppe_ipo_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif

sw_error_t
hppe_ipo_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_action_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				IPE_L2_BASE_ADDR + IPO_ACTION_ADDRESS + \
				index * IPO_ACTION_INC,
				value->val,
				sizeof(union ipo_action_u)/sizeof(a_uint32_t));
}

sw_error_t
hppe_ipo_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipo_action_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				IPE_L2_BASE_ADDR + IPO_ACTION_ADDRESS + \
				index * IPO_ACTION_INC,
				value->val,
				sizeof(union ipo_action_u)/sizeof(a_uint32_t));
}

