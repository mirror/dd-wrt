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

 /* Version 1.15 20210320  */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "sdparm.h"
#include "sg_lib.h"

/*
 * This is a maintenance program for checking the integrity of
 * data in the sdparm_data.c tables.
 *
 * Build with a line like:
 *      gcc -I ../include -Wall -o chk_sdparm_data sdparm_data.o
 *               sg_lib.o sg_lib_data.o sdparm_data_vendor.o
 *               chk_sdparm_data.c
 *
 */

#define MAX_MP_LEN 1024
#define MAX_PDT  0x1f

static uint8_t cl_pdt_arr[MAX_PDT + 1][MAX_MP_LEN];
static uint8_t cl_common_arr[MAX_MP_LEN];

static void clear_cl()
{
    memset(cl_pdt_arr, 0, sizeof(cl_pdt_arr));
    memset(cl_common_arr, 0, sizeof(cl_common_arr));
}

/* result: 0 -> good, 1 -> clash at given pdt, 2 -> clash
 *         other than given pdt, -1 -> bad input */
static int check_cl(int off, int pdt_s, uint8_t mask)
{
    int k;
    int a_pdt = (pdt_s & 0xff);

    if (off >= MAX_MP_LEN)
        return -1;
    if (pdt_s < 0) {
        if (cl_common_arr[off] & mask)
            return 1;
        for (k = 0; k <= MAX_PDT; ++k) {
            if (cl_pdt_arr[k][off] & mask)
                return 2;
        }
        return 0;
    } else if (a_pdt <= MAX_PDT) {
        if (cl_pdt_arr[a_pdt][off] & mask)
            return 1;
        if (cl_common_arr[off] & mask)
            return 2;
        if (pdt_s & ~0xff)
            return check_cl(off, (pdt_s >> 8), mask);
        return 0;
    }
    return -1;
}

static void set_cl(int off, int pdt_s, uint8_t mask)
{
    int a_pdt = (pdt_s & 0xff);

    if (off < MAX_MP_LEN) {
        if (pdt_s < 0)
            cl_common_arr[off] |= mask;
        else if (a_pdt <= MAX_PDT) {
            cl_pdt_arr[a_pdt][off] |= mask;
            if (pdt_s & ~0xff)
                return set_cl(off, (pdt_s >> 8), mask);
        }
    }
}

static int check(const struct sdparm_mode_page_item * mpi,
                 const struct sdparm_mode_page_item * mpi_b)
{
    bool second_k = false;
    bool second_j = false;
    bool k_clash_ok;
    int res, prev_mp, prev_msp, prev_pdt, sbyte, sbit, nbits;
    int bad_count = 0;
    uint8_t mask;
    const struct sdparm_mode_page_item * kp = mpi;
    const struct sdparm_mode_page_item * jp = mpi;
    const char * acron;

    clear_cl();
    for (prev_mp = 0, prev_msp = 0, prev_pdt = -1; ; ++kp) {
        if (NULL == kp->acron) {
            if ((NULL == mpi_b) || second_k)
                break;
            prev_mp = 0;
            prev_msp = 0;
            kp = mpi_b;
            second_k = true;
        }
        k_clash_ok = !! (kp->flags & MF_CLASH_OK);
        acron = kp->acron ? kp->acron : "?";
        if ((prev_mp != kp->pg_num) || (prev_msp != kp->subpg_num)) {
            if (prev_mp > kp->pg_num) {
                printf("  mode page 0x%x,0x%x out of order\n", kp->pg_num,
                        kp->subpg_num);
                ++bad_count;
            }
            if ((prev_mp == kp->pg_num) && (prev_msp > kp->subpg_num)) {
                printf("  mode subpage 0x%x,0x%x out of order, previous msp "
                       "was 0x%x\n", kp->pg_num, kp->subpg_num, prev_msp);
                ++bad_count;
            }
            prev_mp = kp->pg_num;
            prev_msp = kp->subpg_num;
            prev_pdt = kp->pdt_s;
            clear_cl();
        } else if ((prev_pdt >= 0) && (prev_pdt != kp->pdt_s)) {
            if (prev_pdt > kp->pdt_s) {
                printf("  mode page 0x%x,0x%x pdt out of order, pdt was "
                       "%d, now %d\n", kp->pg_num, kp->subpg_num,
                       prev_pdt, kp->pdt_s);
                ++bad_count;
            }
            prev_pdt = kp->pdt_s;
        }
        for (jp = kp + 1, second_j = second_k; ; ++jp) {
            if (NULL == jp->acron) {
                if ((NULL == mpi_b) || second_j)
                    break;
                jp = mpi_b;
                second_j = true;
            }
            if ((0 == strcmp(acron, jp->acron)) &&
                (! ((jp->flags & MF_CLASH_OK) || k_clash_ok))) {
                printf("  acronym '%s' with this description: '%s'\n    "
                       "clashes with '%s'\n", acron, kp->description,
                       jp->description);
                ++bad_count;
            }
        }
        sbyte = kp->start_byte;
        if ((unsigned)sbyte + 8 > MAX_MP_LEN) {
            printf("  acronym: %s  start byte too large: %d\n", kp->acron,
                   sbyte);
            ++bad_count;
            continue;
        }
        sbit = kp->start_bit;
        if ((unsigned)sbit > 7) {
            printf("  acronym: %s  start bit too large: %d\n", kp->acron,
                   sbit);
            ++bad_count;
            continue;
        }
        nbits = kp->num_bits;
        if (nbits > 64) {
            printf("  acronym: %s  number of bits too large: %d\n",
                   kp->acron, nbits);
            ++bad_count;
            continue;
        }
        if (nbits < 1) {
            printf("  acronym: %s  number of bits too small: %d\n",
                   kp->acron, nbits);
            ++bad_count;
            continue;
        }

        /* now mark up bit map and check for overwrites */
        if (kp->flags & MF_CLASH_OK)
            continue;   /* MF_CLASH_OK flag implies clash expected */
        mask = (1 << (sbit + 1)) - 1;
        if ((nbits - 1) < sbit)
            mask &= ~((1 << (sbit + 1 - nbits)) - 1);
        res = check_cl(sbyte, kp->pdt_s, mask);
        if (res) {
            if (1 == res) {
                printf("  0x%x,0x%x: clash at start_byte: %d, bit: %d "
                       "[latest acron: %s, this pdt]\n", prev_mp, prev_msp,
                       sbyte, sbit, acron);
                ++bad_count;
           } else if (2 == res) {
                printf("  0x%x,0x%x: clash at start_byte: %d, bit: %d "
                       "[latest acron: %s, another pdt]\n", prev_mp,
                       prev_msp, sbyte, sbit, acron);
                ++bad_count;
            } else {
                printf("  0x%x,0x%x: clash, bad data at start_byte: %d, "
                       "bit: %d [latest acron: %s]\n", prev_mp,
                       prev_msp, sbyte, sbit, acron);
                ++bad_count;
            }
        }
        set_cl(sbyte, kp->pdt_s, mask);
        if ((nbits - 1) > sbit) {
            nbits -= (sbit + 1);
            if ((nbits > 7) && (0 != (nbits % 8))) {
                printf("  0x%x,0x%x: check nbits: %d, start_byte: %d, bit: "
                       "%d [acron: %s]\n", prev_mp, prev_msp, kp->num_bits,
                       sbyte, sbit, acron);
                ++bad_count;
            }
            do {
                ++sbyte;
                mask = 0xff;
                if (nbits > 7)
                    nbits -= 8;
                else {
                    mask &= ~((1 << (8 - nbits)) - 1);
                    nbits = 0;
                }
                res = check_cl(sbyte, kp->pdt_s, mask);
                if (res) {
                    if (1 == res) {
                        printf("   0x%x,0x%x: clash at start_byte: %d, "
                               "bit: %d [latest acron: %s, this pdt]\n",
                               prev_mp, prev_msp, sbyte, sbit, acron);
                        ++bad_count;
                    } else if (2 == res) {
                        printf("   0x%x,0x%x: clash at start_byte: %d, "
                               "bit: %d [latest acron: %s, another pdt]\n",
                               prev_mp, prev_msp, sbyte, sbit, acron);
                        ++bad_count;
                    } else {
                        printf("   0x%x,0x%x: clash, bad at start_byte: "
                               "%d, bit: %d [latest acron: %s]\n",
                               prev_mp, prev_msp, sbyte, sbit, acron);
                        ++bad_count;
                    }
                }
                set_cl(sbyte, kp->pdt_s, mask);
            } while (nbits > 0);
        }
    }
    return bad_count;
}

static const char * get_vendor_name(int vendor_id)
{
    const struct sdparm_vendor_name_t * vnp;

    for (vnp = sdparm_vendor_id; vnp->acron; ++vnp) {
        if (vendor_id == vnp->vendor_id)
            return vnp->name;
    }
    return NULL;
}

int main(int argc, char ** argv)
{
    int k, bad_count;
    const struct sdparm_transport_pair * tp;
    const struct sdparm_vendor_pair * vp;
    const char * ccp;
    char b[128];

    if (argc) {
        ;       /* suppress unused warning */
    }
    if (argv) {
        ;       /* suppress unused warning */
    }
    printf("    Check integrity of mode page item tables in sdparm\n");
    printf("    ==================================================\n\n");
    printf("Generic (i.e. non-transport specific) mode page items:\n");
    bad_count = check(sdparm_mitem_arr, NULL);
    if (bad_count)
        printf("%d problem%s\n", bad_count, (1 == bad_count) ? "" : "s");
    else
        printf("Pass\n");
    tp = sdparm_transport_mp;
    for (k = 0; k < 16; ++k, ++tp) {
        if (tp->mitem) {
            printf("%s mode page items:\n",
                   sg_get_trans_proto_str(k, sizeof(b), b));
            bad_count = check(tp->mitem, NULL);
            if (bad_count)
                printf("%d problem%s\n", bad_count,
                       (1 == bad_count) ? "" : "s");
            else
                printf("Pass\n");
        }
    }
    vp = sdparm_vendor_mp;
    for (k = 0; k < sdparm_vendor_mp_len; ++k, ++vp) {
        if (vp->mitem) {
            ccp = get_vendor_name(k);
            if (ccp)
                printf("%s mode page items:\n", ccp);
            else
                printf("0x%x mode page items:\n", k);
            bad_count = check(vp->mitem, NULL);
            if (bad_count)
                printf("%d problem%s\n", bad_count,
                       (1 == bad_count) ? "" : "s");
            else
                printf("Pass\n");
        }
    }
    printf("Cross check Generic with SAS mode page items:\n");
    bad_count = check(sdparm_mitem_arr, sdparm_transport_mp[6].mitem);
    if (bad_count)
        printf("%d problem%s\n", bad_count, (1 == bad_count) ? "" : "s");
    else
        printf("Pass\n");
    return 0;
}
