/*
 * Copyright (c) 2012,2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*qca808x_start*/
#ifndef _HSL_H
#define _HSL_H

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "ssdk_init.h"
#include "ssdk_plat.h"
    typedef sw_error_t
    (*hsl_acl_rule_copy) (a_uint32_t dev_id, a_uint32_t src_addr,
                          a_uint32_t dest_addr, a_uint32_t size);

    typedef sw_error_t
    (*hsl_acl_rule_invalid) (a_uint32_t dev_id, a_uint32_t addr,
                             a_uint32_t size);

    typedef sw_error_t
    (*hsl_acl_addr_update) (a_uint32_t dev_id, a_uint32_t old_addr,
                            a_uint32_t new_addr, a_uint32_t info);

    typedef struct
    {
        hsl_acl_rule_copy     acl_rule_copy;
        hsl_acl_rule_invalid  acl_rule_invalid;
        hsl_acl_addr_update   acl_addr_update;
    } hsl_acl_func_t;
/*qca808x_end*/

#if 1
/*APIs without mdio bus lock*/
#define __HSL_REG_ENTRY_GET(rv, dev, reg, index, value, val_len) \
	rv = __qca_mii_reg_get(dev,reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,value,val_len);

#define __HSL_REG_ENTRY_SET(rv, dev, reg, index, value, val_len) \
	rv = __qca_mii_reg_set(dev,reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,value,val_len);

#define __HSL_REG_FIELD_GET(rv, dev, reg, index, field, value, val_len) \
	rv = __qca_mii_field_get(dev,reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
	reg##_##field##_BOFFSET, \
	reg##_##field##_BLEN,value,val_len);

#define __HSL_REG_FIELD_SET(rv, dev, reg, index, field, value, val_len) \
	rv = __qca_mii_field_set(dev,reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
	reg##_##field##_BOFFSET, \
	reg##_##field##_BLEN,value,val_len);

#define __HSL_REG_ENTRY_GEN_GET(rv, dev, addr, reg_len, value, val_len)\
	rv = __qca_mii_reg_get(dev,addr,(a_uint8_t*)value,val_len);

#define __HSL_REG_ENTRY_GEN_SET(rv, dev, addr, reg_len, value, val_len)\
	rv = __qca_mii_reg_set(dev,addr,(a_uint8_t*)value,val_len);

#define __HSL_REG_FIELD_GEN_GET(rv, dev, regaddr, bitlength, bitoffset, value, val_len)\
	rv = __qca_mii_field_get(dev, regaddr, bitoffset, bitlength, (a_uint8_t*)value, val_len);

#define __HSL_REG_FIELD_GEN_SET(rv, dev, regaddr, bitlength, bitoffset, value, val_len)\
	rv = __qca_mii_field_set(dev,regaddr,bitoffset,bitlength, (a_uint8_t*)value,val_len);

/*APIs with mdio bus lock*/
#define HSL_REG_ENTRY_GET(rv, dev, reg, index, value, val_len) \
	rv = qca_mii_reg_get(dev,reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,value,val_len);
#define HSL_REG_ENTRY_SET(rv, dev, reg, index, value, val_len) \
	rv = qca_mii_reg_set(dev,reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,value,val_len);
#define HSL_REG_FIELD_GET(rv, dev, reg, index, field, value, val_len) \
	rv = qca_mii_field_get(dev,reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
	reg##_##field##_BOFFSET, \
	reg##_##field##_BLEN,value,val_len);
#define HSL_REG_FIELD_SET(rv, dev, reg, index, field, value, val_len) \
	rv = qca_mii_field_set(dev,reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
	reg##_##field##_BOFFSET, \
	reg##_##field##_BLEN,value,val_len);

#define HSL_REG_ENTRY_GEN_GET(rv, dev, addr, reg_len, value, val_len)\
	rv = qca_mii_reg_get(dev,addr,(a_uint8_t*)value,val_len);
#define HSL_REG_ENTRY_GEN_SET(rv, dev, addr, reg_len, value, val_len)\
	rv = qca_mii_reg_set(dev,addr,(a_uint8_t*)value,val_len);
#define HSL_REG_FIELD_GEN_GET(rv, dev, regaddr, bitlength, bitoffset, value, val_len)\
	rv = qca_mii_field_get(dev, regaddr, bitoffset, bitlength, (a_uint8_t*)value, val_len);
#define HSL_REG_FIELD_GEN_SET(rv, dev, regaddr, bitlength, bitoffset, value, val_len)\
	rv = qca_mii_field_set(dev,regaddr,bitoffset,bitlength, (a_uint8_t*)value,val_len);
#else
#define HSL_REG_ENTRY_GET(rv, dev, reg, index, value, val_len) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api) { \
        rv = p_api->reg_get(dev, reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
                                   (a_uint8_t*)value, (a_uint8_t)val_len); \
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);

#define HSL_REG_ENTRY_SET(rv, dev, reg, index, value, val_len) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api) { \
        rv = p_api->reg_set (dev, reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
                                   (a_uint8_t*)value, (a_uint8_t)val_len); \
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);

#define HSL_REG_FIELD_GET(rv, dev, reg, index, field, value, val_len) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api) { \
        rv = p_api->reg_field_get(dev, reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
                                  reg##_##field##_BOFFSET, \
                                  reg##_##field##_BLEN, (a_uint8_t*)value, val_len);\
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);

#define HSL_REG_FIELD_SET(rv, dev, reg, index, field, value, val_len) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api){ \
        rv = p_api->reg_field_set(dev, reg##_OFFSET + ((a_uint32_t)index) * reg##_E_OFFSET,\
                                  reg##_##field##_BOFFSET, \
                                  reg##_##field##_BLEN, (a_uint8_t*)value, val_len);\
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);

#define HSL_REG_ENTRY_GEN_GET(rv, dev, addr, reg_len, value, val_len) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api) { \
        rv = p_api->reg_get(dev, addr, (a_uint8_t*)value, val_len);\
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);

#define HSL_REG_ENTRY_GEN_SET(rv, dev, addr, reg_len, value, val_len) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api) { \
        rv = p_api->reg_set(dev, addr, (a_uint8_t*)value, val_len); \
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);

#define HSL_REG_FIELD_GEN_GET(rv, dev, regaddr, bitlength, bitoffset, value, val_len) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api) { \
        rv = p_api->reg_field_get(dev, regaddr, bitoffset, bitlength, \
                                   (a_uint8_t *) value, val_len);\
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);

#define HSL_REG_FIELD_GEN_SET(rv, dev, regaddr, bitlength, bitoffset, value, val_len) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api) {\
        rv = p_api->reg_field_set(dev, regaddr, bitoffset, bitlength, \
                                   (a_uint8_t *) value, val_len);\
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);

#define HSL_PHY_GET(rv, dev, phy_addr, reg, value) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api) { \
        rv = p_api->phy_get(dev, phy_addr, reg, value); \
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);

#define HSL_PHY_SET(rv, dev, phy_addr, reg, value) \
do { \
    hsl_api_t *p_api = hsl_api_ptr_get(dev); \
    if (p_api) { \
        rv = p_api->phy_set(dev, phy_addr, reg, value); \
    } else { \
        rv = SW_NOT_INITIALIZED; \
    } \
} while (0);
#endif
/*qca808x_start*/
#if (defined(API_LOCK) \
&& (defined(HSL_STANDALONG) || (defined(KERNEL_MODULE) && defined(USER_MODE))))
    extern  aos_lock_t sw_hsl_api_lock;
#define HSL_API_LOCK    aos_lock(&sw_hsl_api_lock)
#define HSL_API_UNLOCK  aos_unlock(&sw_hsl_api_lock)
#else
#define HSL_API_LOCK
#define HSL_API_UNLOCK
#endif

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _HSL_H */
/*qca808x_end*/
