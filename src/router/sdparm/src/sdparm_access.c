/*
 * Copyright (c) 2005-2021, Douglas Gilbert
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sdparm.h"
#include "sg_lib.h"
#include "sg_unaligned.h"
#include "sg_pr2serr.h"

/* sdparm_access.c : helpers for sdparm to access tables in
 * sdparm_data.c
 */

/* Returns true if left argument is "equal" to the right argument. l_pdt_s
 * is a compound PDT (SCSI Peripheral Device Type) or a negative number
 * which represents a wildcard (i.e. match anything). r_pdt_s has a similar
 * form. PDT values are 5 bits long (0 to 31) and a compound pdt_s is
 * formed by shifting the second (upper) PDT by eight bits to the left and
 * OR-ing it with the first PDT. The pdt_s values must be defined so
 * PDT_DISK (0) is _not_ the upper value in a compound pdt_s. */
bool
pdt_s_eq(int l_pdt_s, int r_pdt_s)
{
    bool upper_l = !!(l_pdt_s & PDT_UPPER_MASK);
    bool upper_r = !!(r_pdt_s & PDT_UPPER_MASK);

    if ((l_pdt_s < 0) || (r_pdt_s < 0))
        return true;
    if (!upper_l && !upper_r)
        return l_pdt_s == r_pdt_s;
    else if (upper_l && upper_r)
        return (((PDT_UPPER_MASK & l_pdt_s) == (PDT_UPPER_MASK & r_pdt_s)) ||
                ((PDT_LOWER_MASK & l_pdt_s) == (PDT_LOWER_MASK & r_pdt_s)));
    else if (upper_l)
        return (((PDT_LOWER_MASK & l_pdt_s) == r_pdt_s) ||
                ((PDT_UPPER_MASK & l_pdt_s) >> 8) == r_pdt_s);
    return (((PDT_LOWER_MASK & r_pdt_s) == l_pdt_s) ||
            ((PDT_UPPER_MASK & r_pdt_s) >> 8) == l_pdt_s);
}

/* Returns 1 if strings equal (same length, characters same or only differ
 * by case), else returns 0. Assumes 7 bit ASCII (English alphabet). */
int
sdp_strcase_eq(const char * s1p, const char * s2p)
{
    int c1, c2;

    do {
        c1 = *s1p++;
        c2 = *s2p++;
        if (c1 != c2) {
            if (c2 >= 'a')
                c2 = toupper(c2);
            else if (c1 >= 'a')
                c1 = toupper(c1);
            else
                return 0;
            if (c1 != c2)
                return 0;
        }
    } while (c1);
    return 1;
}

/* Returns 1 if strings equal up to the nth character (characters same or only
 * differ by case), else returns 0. Assumes 7 bit ASCII (English alphabet). */
int
sdp_strcase_eq_upto(const char * s1p, const char * s2p, int n)
{
    int k, c1, c2;

    for (k = 0; k < n; ++k) {
        c1 = *s1p++;
        c2 = *s2p++;
        if (c1 != c2) {
            if (c2 >= 'a')
                c2 = toupper(c2);
            else if (c1 >= 'a')
                c1 = toupper(c1);
            else
                return 0;
            if (c1 != c2)
                return 0;
        }
        if (0 == c1)
            break;
    }
    return 1;
}


/* Returns length of mode page. Assumes mp pointing at start of a mode
 * page (not the start of a MODE SENSE response). */
int
sdp_mpage_len(const uint8_t * mp)
{
    /* if SPF (byte 0, bit 6) is set then 4 byte header, else 2 byte header */
    return (mp[0] & 0x40) ? (sg_get_unaligned_be16(mp + 2) + 4) : (mp[1] + 2);
}

const struct sdparm_mode_page_t *
sdp_get_mpage_t(int page_num, int subpage_num, int pdt, int transp_proto,
                int vendor_id)
{
    const struct sdparm_mode_page_t * mpp;

    if (vendor_id >= 0) {
        const struct sdparm_vendor_pair * vpp;

        vpp = sdp_get_vendor_pair(vendor_id);
        mpp = (vpp ? vpp->mpage : NULL);
    } else if ((transp_proto >= 0) && (transp_proto < 16))
        mpp = sdparm_transport_mp[transp_proto].mpage;
    else
        mpp = sdparm_gen_mode_pg;
    if (NULL == mpp)
        return NULL;

    for ( ; mpp->acron; ++mpp) {
        if ((page_num == mpp->page) && (subpage_num == mpp->subpage)) {
            if ((pdt < 0) || (mpp->pdt_s < 0) || pdt_s_eq(mpp->pdt_s, pdt))
                return mpp;
        }
    }
    return NULL;
}

const struct sdparm_mode_page_t *
sdp_get_mpt_with_str(int page_num, int subpage_num, int pdt, int transp_proto,
                     int vendor_id, bool plus_acron, bool hex, int b_len,
                     char * bp)
{
    int len = b_len - 1;
    const struct sdparm_mode_page_t * mpp = NULL;
    const char * cp;

    if (len < 0)
        return mpp;
    bp[len] = '\0';
    /* first try to match given pdt */
    mpp = sdp_get_mpage_t(page_num, subpage_num, pdt, transp_proto,
                          vendor_id);
    if (NULL == mpp) /* didn't match specific pdt so try -1 (ie. SPC) */
        mpp = sdp_get_mpage_t(page_num, subpage_num, -1, transp_proto,
                              vendor_id);
    if (mpp && mpp->name) {
        cp = mpp->acron;
        if (NULL == cp)
            cp = "";
        if (hex) {
            if (0 == subpage_num) {
                if (plus_acron)
                    snprintf(bp, len, "%s [%s: 0x%x]", mpp->name, cp,
                             page_num);
                else
                    snprintf(bp, len, "%s [0x%x]", mpp->name, page_num);
            } else {
                if (plus_acron)
                    snprintf(bp, len, "%s [%s: 0x%x,0x%x]", mpp->name, cp,
                             page_num, subpage_num);
                else
                    snprintf(bp, len, "%s [0x%x,0x%x]", mpp->name, page_num,
                             subpage_num);
            }
        } else {
            if (plus_acron)
                snprintf(bp, len, "%s [%s]", mpp->name, cp);
            else
                snprintf(bp, len, "%s", mpp->name);
        }
    } else {
        if (0 == subpage_num)
            snprintf(bp, len, "[0x%x]", page_num);
        else
            snprintf(bp, len, "[0x%x,0x%x]", page_num, subpage_num);
    }
    return mpp;
}

const struct sdparm_mode_page_t *
sdp_find_mpt_by_acron(const char * ap, int transp_proto, int vendor_id)
{
    const struct sdparm_mode_page_t * mpp;

    if (vendor_id >= 0) {
        const struct sdparm_vendor_pair * vpp;

        vpp = sdp_get_vendor_pair(vendor_id);
        mpp = (vpp ? vpp->mpage : NULL);
    } else if ((transp_proto >= 0) && (transp_proto < 16))
        mpp = sdparm_transport_mp[transp_proto].mpage;
    else
        mpp = sdparm_gen_mode_pg;
    if (NULL == mpp)
        return NULL;

    for ( ; mpp->acron; ++mpp) {
        if (sdp_strcase_eq(mpp->acron, ap))
            return mpp;
    }
    return NULL;
}

const struct sdparm_vpd_page_t *
sdp_get_vpd_detail(int page_num, int subvalue, int pdt)
{
    const struct sdparm_vpd_page_t * vpp;
    int sv, ty;

    sv = (subvalue < 0) ? 1 : 0;
    ty = (pdt < 0) ? 1 : 0;
    for (vpp = sdparm_vpd_pg; vpp->acron; ++vpp) {
        if ((page_num == vpp->vpd_num) &&
            (sv || (subvalue == vpp->subvalue)) &&
            (ty || (pdt == vpp->pdt_s)))
            return vpp;
    }
    if (! ty)
        return sdp_get_vpd_detail(page_num, subvalue, -1);
    if (! sv)
        return sdp_get_vpd_detail(page_num, -1, -1);
    return NULL;
}

const struct sdparm_vpd_page_t *
sdp_find_vpd_by_acron(const char * ap)
{
    const struct sdparm_vpd_page_t * vpp;

    for (vpp = sdparm_vpd_pg; vpp->acron; ++vpp) {
        if (sdp_strcase_eq(vpp->acron, ap))
            return vpp;
    }
    return NULL;
}

char *
sdp_get_transport_name(int proto_num, int b_len, char * b)
{
    char d[128];

    if (NULL == b)
        ;
    else if (b_len < 2) {
        if (1 == b_len)
            b[0] = '\0';
    } else
        snprintf(b, b_len, "%s", sg_get_trans_proto_str(proto_num, sizeof(d),
                 d));
    return b;
}

int
sdp_find_transport_id_by_acron(const char * ap)
{
    const struct sdparm_val_desc_t * t_vdp;
    const struct sdparm_val_desc_t * t_addp;

    for (t_vdp = sdparm_transport_id; t_vdp->desc; ++t_vdp) {
        if (sdp_strcase_eq(t_vdp->desc, ap))
            return t_vdp->val;
    }
    /* No match ... try additional transport acronyms */
    for (t_addp = sdparm_add_transport_acron; t_addp->desc; ++t_addp) {
        if (sdp_strcase_eq(t_addp->desc, ap))
            return t_addp->val;
    }
    return -1;
}

const char *
sdp_get_vendor_name(int vendor_id)
{
    const struct sdparm_vendor_name_t * vnp;

    for (vnp = sdparm_vendor_id; vnp->acron; ++vnp) {
        if (vendor_id == vnp->vendor_id)
            return vnp->name;
    }
    return NULL;
}

const struct sdparm_vendor_name_t *
sdp_find_vendor_by_acron(const char * ap)
{
    const struct sdparm_vendor_name_t * vnp;

    for (vnp = sdparm_vendor_id; vnp->acron; ++vnp) {
        if (sdp_strcase_eq_upto(vnp->acron, ap, strlen(vnp->acron)))
            return vnp;
    }
    return NULL;
}

const struct sdparm_vendor_pair *
sdp_get_vendor_pair(int vendor_id)
{
     return ((vendor_id >= 0) && (vendor_id < sdparm_vendor_mp_len))
            ? (sdparm_vendor_mp + vendor_id) : NULL;
}

/* Searches mpage items table from (and including) the current position
 * looking for the first match on 'ap' (pointer to acromym). Checks
 * against the inbuilt table (in sdparm_data.c) of generic (when both
 * transp_proto and vendor_id are -1), transport (when transp_proto is
 * >= 0) or vendor (when vendor_id is >= 0) mode page items (fields).
 * If found a pointer to that mitem is returned and *from_p is set to
 * the offset after the match. If not found then NULL is returned and
 * *from_p is set to the offset of the sentinel at the end of the
 * selected mitem array. Start iteration by setting from_p to NULL or
 * point it at -1. */
const struct sdparm_mode_page_item *
sdp_find_mitem_by_acron(const char * ap, int * from_p, int transp_proto,
                        int vendor_id)
{
    int k = 0;
    const struct sdparm_mode_page_item * mpi;

    if (from_p) {
        k = *from_p;
        if (k < 0)
            k = 0;
    }
    if (vendor_id >= 0) {
        const struct sdparm_vendor_pair * vpp;

        vpp = sdp_get_vendor_pair(vendor_id);
        mpi = (vpp ? vpp->mitem : NULL);
    } else if ((transp_proto >= 0) && (transp_proto < 16))
        mpi = sdparm_transport_mp[transp_proto].mitem;
    else
        mpi = sdparm_mitem_arr;
    if (NULL == mpi)
        return NULL;

    for (mpi += k; mpi->acron; ++k, ++mpi) {
        if (sdp_strcase_eq(mpi->acron, ap))
            break;
    }
    if (NULL == mpi->acron)
        mpi = NULL;
    if (from_p)
        *from_p = (mpi ? (k + 1) : k);
    return mpi;
}

uint64_t
sdp_mitem_get_value(const struct sdparm_mode_page_item *mpi,
                    const uint8_t * mp)
{
    return sg_get_big_endian(mp + mpi->start_byte, mpi->start_bit,
                              mpi->num_bits);
}

/* Gets a mode page item's value given a pointer to the mode page response
 * (mp). If all_setp is non-NULL then checks 8, 16, 24, 32, 48 and 64 bit
 * quantities for all bits set (e.g. for 8 bits that would be 0xff) and if
 * so sets the bool addressed by all_setp to true. Otherwise if all_setp
 * is non-NULL then it sets the bool addressed by all_setp to false.
 * Returns the value in an unsigned 64 bit integer. To print a value as a
 * signed quantity use sdp_print_signed_decimal(). */
uint64_t
sdp_mitem_get_value_check(const struct sdparm_mode_page_item *mpi,
                          const uint8_t * mp, bool * all_setp)
{
    uint64_t res;

    res = sg_get_big_endian(mp + mpi->start_byte, mpi->start_bit,
                             mpi->num_bits);
    if (all_setp) {
        switch (mpi->num_bits) {
        case 8:
            if (0xff == res)
                *all_setp = true;
            break;
        case 16:
            if (0xffff == res)
                *all_setp = true;
            break;
        case 24:
            if (0xffffff == res)
                *all_setp = true;
            break;
        case 32:
            if (0xffffffff == res)
                *all_setp = true;
            break;
        case 48:
            if (0xffffffffffffULL == res)
                *all_setp = true;
            break;
        case 64:
            if (0xffffffffffffffffULL == res)
                *all_setp = true;
            break;
        default:
            *all_setp = false;
            break;
        }
    }
    return res;
}

void
sdp_print_signed_decimal(uint64_t u, int num_bits, bool leading_zeros)
{
    unsigned int ui;
    uint8_t uc;

    switch (num_bits) {
    /* could support other num_bits, as required */
    case 4:     /* -8 to 7 */
        uc = u & 0xf;
        if (0x8 & uc)
            uc |= 0xf0;         /* sign extend */
        if (leading_zeros)
            printf("%02hhd", (signed char)uc);
        else
            printf("%hhd", (signed char)uc);
        break;
    case 8:     /* -128 to 127 */
        if (leading_zeros)
            printf("%02hhd", (signed char)u);
        else
            printf("%hhd", (signed char)u);
        break;
    case 16:    /* -32768 to 32767 */
        if (leading_zeros)
            printf("%02hd", (short int)u);
        else
            printf("%hd", (short int)u);
        break;
    case 24:
        ui = 0xffffff & u;
        if (0x800000 & ui)
            ui |= 0xff000000;
        if (leading_zeros)
            printf("%02d", (int)ui);
        else
            printf("%d", (int)ui);
        break;
    case 32:
        if (leading_zeros)
            printf("%02d", (int)u);
        else
            printf("%d", (int)u);
        break;
    case 64:
    default:
        if (leading_zeros)
            printf("%02" PRId64 , (int64_t)u);
        else
            printf("%" PRId64 , (int64_t)u);
        break;
    }
}

/* Place 'val' at an offset to 'mp' as indicated by mpi. */
void
sdp_mitem_set_value(uint64_t val, const struct sdparm_mode_page_item * mpi,
                    uint8_t * mp)
{
    sg_set_big_endian(val, mp + mpi->start_byte, mpi->start_bit,
                      mpi->num_bits);
}

char *
sdp_get_ansi_version_str(int version, int buff_len, char * buff)
{
    version &= 0x7;
    buff[buff_len - 1] = '\0';
    strncpy(buff, sdparm_ansi_version_arr[version], buff_len - 1);
    return buff;
}

int
sdp_get_desc_id(int flags)
{
    return (MF_DESC_ID_MASK & flags) >> MF_DESC_ID_SHIFT;
}
