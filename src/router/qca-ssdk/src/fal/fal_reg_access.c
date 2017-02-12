/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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


/**
 * @defgroup fal_reg_access FAL_REG_ACCESS
 * @{
 */
#include "sw.h"
#include "fal_reg_access.h"
#include "hsl_api.h"

static sw_error_t
_fal_phy_get(a_uint32_t dev_id, a_uint32_t phy_addr,
             a_uint32_t reg, a_uint16_t * value)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->phy_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->phy_get(dev_id, phy_addr, reg, value);
    return rv;
}

static sw_error_t
_fal_phy_set(a_uint32_t dev_id, a_uint32_t phy_addr,
             a_uint32_t reg, a_uint16_t value)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->phy_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->phy_set(dev_id, phy_addr, reg, value);
    return rv;
}

static sw_error_t
_fal_reg_get(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
             a_uint32_t value_len)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->reg_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->reg_get(dev_id, reg_addr, value, value_len);
    return rv;
}

static sw_error_t
_fal_reg_set(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
             a_uint32_t value_len)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->reg_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->reg_set(dev_id, reg_addr, value, value_len);
    return rv;
}


static sw_error_t
_fal_psgmii_reg_get(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
             a_uint32_t value_len)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->psgmii_reg_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->psgmii_reg_get(dev_id, reg_addr, value, value_len);
    return rv;
}

static sw_error_t
_fal_psgmii_reg_set(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
             a_uint32_t value_len)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->psgmii_reg_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->psgmii_reg_set(dev_id, reg_addr, value, value_len);
    return rv;
}

static sw_error_t
_fal_reg_field_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint32_t bit_offset, a_uint32_t field_len,
                   a_uint8_t value[], a_uint32_t value_len)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->reg_field_get)
        return SW_NOT_SUPPORTED;

    rv = p_api->reg_field_get(dev_id, reg_addr, bit_offset, field_len, value, value_len);
    return rv;
}

static sw_error_t
_fal_reg_field_set(a_uint32_t dev_id, a_uint32_t reg_addr,
                   a_uint32_t bit_offset, a_uint32_t field_len,
                   const a_uint8_t value[], a_uint32_t value_len)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->reg_field_set)
        return SW_NOT_SUPPORTED;

    rv = p_api->reg_field_set(dev_id, reg_addr, bit_offset, field_len, value, value_len);
    return rv;
}

static sw_error_t
_fal_reg_dump(a_uint32_t dev_id, a_uint32_t reg_idx,fal_reg_dump_t *reg_dump)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->register_dump)
        return SW_NOT_SUPPORTED;

    rv = p_api->register_dump(dev_id, reg_idx,reg_dump);
    return rv;
}


static sw_error_t
_fal_debug_reg_dump(a_uint32_t dev_id, fal_debug_reg_dump_t *reg_dump)
{
    sw_error_t rv;
    hsl_api_t *p_api;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    if (NULL == p_api->debug_register_dump)
        return SW_NOT_SUPPORTED;

    rv = p_api->debug_register_dump(dev_id, reg_dump);
    return rv;
}



/**
  * fal_phy_get - get value of specific phy device
  * @phy_addr: id of the phy device
  * @reg: register id of phy device
  * @value: pointer to the memory storing the value.
  * @return SW_OK or error code
  */
sw_error_t
fal_phy_get(a_uint32_t dev_id, a_uint32_t phy_addr,
            a_uint32_t reg, a_uint16_t * value)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_phy_get(dev_id, phy_addr, reg, value);
    FAL_API_UNLOCK;
    return rv;
}

/**
  * fal_phy_set - set value of specific phy device
  * @phy_addr: id of the phy device
  * @reg: register id of phy device
  * @value: register value.
  * @return SW_OK or error code
  */
sw_error_t
fal_phy_set(a_uint32_t dev_id, a_uint32_t phy_addr,
            a_uint32_t reg, a_uint16_t value)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_phy_set(dev_id, phy_addr, reg, value);
    FAL_API_UNLOCK;
    return rv;
}

/**
  * fal_reg_get - get value of specific register
  * @reg_addr: address of the register
  * @value: pointer to the memory storing the value.
  * @value_len: length of the value.
  *
  * Get the value of a specific register field with related parameter
  */
sw_error_t
fal_reg_get(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
            a_uint32_t value_len)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_reg_get(dev_id, reg_addr, value, value_len);
    FAL_API_UNLOCK;
    return rv;
}

/**
  * fal_reg_set - set value of specific register
  * @reg_addr: address of the register
  * @value: pointer to the memory storing the value.
  * @value_len: length of the value.
  *
  * Get the value of a specific register field with related parameter
  */
sw_error_t
fal_reg_set(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
            a_uint32_t value_len)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_reg_set(dev_id, reg_addr, value, value_len);
    FAL_API_UNLOCK;
    return rv;
}

/**
  * fal_psgmii_reg_get - get value of specific register in psgmii module
  * @reg_addr: address of the register
  * @value: pointer to the memory storing the value.
  * @value_len: length of the value.
  *
  * Get the value of a specific register field with related parameter
  */
sw_error_t
fal_psgmii_reg_get(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
            a_uint32_t value_len)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_psgmii_reg_get(dev_id, reg_addr, value, value_len);
    FAL_API_UNLOCK;
    return rv;
}

/**
  * fal_psgmii_reg_set - set value of specific register in psgmii module
  * @reg_addr: address of the register
  * @value: pointer to the memory storing the value.
  * @value_len: length of the value.
  *
  * Get the value of a specific register field with related parameter
  */
sw_error_t
fal_psgmii_reg_set(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
            a_uint32_t value_len)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_psgmii_reg_set(dev_id, reg_addr, value, value_len);
    FAL_API_UNLOCK;
    return rv;
}

/**
  * fal_reg_field_get - get value of specific register field
  * @reg_addr: address of the register
  * @bit_offset: position of the field in bit
  * @field_len: length of the field in bit
  * @value: pointer to the memory storing the value.
  * @value_len: length of the value.
  *
  * Get the value of a specific register field with related parameter
  */
sw_error_t
fal_reg_field_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                  a_uint32_t bit_offset, a_uint32_t field_len,
                  a_uint8_t value[], a_uint32_t value_len)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_reg_field_get(dev_id, reg_addr, bit_offset, field_len, value, value_len);
    FAL_API_UNLOCK;
    return rv;
}

/**
  * fal_reg_field_set - set value of specific register field
  * @reg_addr: address of the register
  * @bit_offset: position of the field in bit
  * @field_len: length of the field in bit
  * @value: pointer to the memory storing the value.
  * @value_len: length of the value.
  *
  * Set the value of a specific register field with related parameter
  */
sw_error_t
fal_reg_field_set(a_uint32_t dev_id, a_uint32_t reg_addr,
                  a_uint32_t bit_offset, a_uint32_t field_len,
                  const a_uint8_t value[], a_uint32_t value_len)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_reg_field_set(dev_id, reg_addr, bit_offset, field_len, value, value_len);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief dump device register group
 * @details   Comments:
 *    The unit of packets size is byte.
 * @param[in] dev_id device id
 * @param[out] reg_dump dump out register group
 * @return SW_OK or error code
 */
sw_error_t
fal_reg_dump(a_uint32_t dev_id, a_uint32_t reg_idx,fal_reg_dump_t *reg_dump)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_reg_dump(dev_id, reg_idx,reg_dump);
    FAL_API_UNLOCK;
    return rv;
}

/**
 * @brief dump device debug register
 * @details   Comments:
 *    The unit of packets size is byte.
 * @param[in] dev_id device id
 * @param[out] reg_dump dump out debub register
 * @return SW_OK or error code
 */
sw_error_t
fal_debug_reg_dump(a_uint32_t dev_id, fal_debug_reg_dump_t *reg_dump)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_debug_reg_dump(dev_id,reg_dump);
    FAL_API_UNLOCK;
    return rv;
}




/**
 * @}
 */
