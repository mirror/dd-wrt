/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "offchip_statistics.h"
#include "command.h"


/*
* Get the offchip data for this tag,
* or NULL if none can be found.
*/
struct statistics_offchip_data *get_stats_offchip(const struct morsectrl *mors, stats_tlv_tag_t tag)
{
    struct statistics_offchip_data *res = NULL;
    for (int i = 0; i < mors->n_stats; i++)
    {
        if (mors->stats[i].tag == tag)
        {
            res = mors->stats + i;
            break;
        }
    }
    return res;
}


int64_t get_signed_value_as_int64(const uint8_t *buf, uint32_t size)
{
    int64_t n = 0;
    switch (size)
    {
        case 1:
        {
            int8_t x;
            memcpy(&x, buf, size);
            n = (int8_t)x;
            break;
        }
        case 2:
        {
            __le16 x;
            memcpy(&x, buf, size);
            n = (int16_t)le16toh(x);
            break;
        }
        case 4:
        {
            __le32 x;
            memcpy(&x, buf, size);
            n = (int32_t)le32toh(x);
            break;
        }
        case 8:
        {
            __le64 x;
            memcpy(&x, buf, size);
            n = (int64_t) le64toh(x);
            break;
        }
        default:
            mctrl_err("get_signed_value_as_int64 can't convert %d-byte quantity\n", size);
            break;
    }
    return n;
}

uint64_t get_unsigned_value_as_uint64(const uint8_t *buf, uint32_t size)
{
    uint64_t n = 0;
    switch (size)
    {
        case 1:
        {
            uint8_t x;
            memcpy(&x, buf, size);
            break;
        }
        case 2:
        {
            __le16 x;
            memcpy(&x, buf, size);
            n = (uint64_t) le16toh(x);
            break;
        }
        case 4:
        {
            __le32 x;
            memcpy(&x, buf, size);
            n = (uint64_t) le32toh(x);
            break;
        }
        case 8:
        {
            __le64 x;
            memcpy(&x, buf, size);
            n = (uint64_t) le64toh(x);
            break;
        }
        default:
            mctrl_err("get_unsigned_value_as_uint64 can't convert %d-byte quantity\n", size);
            break;
    }
    return n;
}
