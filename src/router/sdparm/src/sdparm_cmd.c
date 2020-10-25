/*
 * Copyright (c) 2005-2018, Douglas Gilbert
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
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_lib.h"
#include "sg_cmds_basic.h"
#include "sg_cmds_mmc.h"
#include "sdparm.h"
#include "sg_unaligned.h"
#include "sg_pr2serr.h"

/* sdparm_cmd.c : contains code to implement commands
 * (i.e "--command=<cmd>") in sdparm.
 */


#define RCAP_REPLY_LEN 8
#define RCAP16_REPLY_LEN 32

static uint8_t * aligned_buff;
static uint8_t * free_aligned_buff;
static int aligned_buff_sz;

static uint8_t * allocate_if_needed(void)
{
    if (NULL == aligned_buff) {
        aligned_buff = sg_memalign(0, 0, &free_aligned_buff, false);
        if (NULL == aligned_buff) {
            pr2serr("Unable to allocate aligned_buff\n");
            return NULL;
        }
        aligned_buff_sz = sg_get_page_size();
    }
    return aligned_buff;
}

static int
deallocate_res(int res)
{
    if (free_aligned_buff) {
        free(free_aligned_buff);
        free_aligned_buff = NULL;
        aligned_buff = NULL;
    }
    return res;
}

static int
do_cmd_read_capacity(int sg_fd, bool do_long, int verbose)
{
    bool do16;
    int res;
    unsigned int last_blk_addr, block_size;
    uint64_t llast_blk_addr;
    double sz_mib;
    uint8_t * resp_buff = aligned_buff;

    do16 = do_long;
    if (! do16) {
        res = sg_ll_readcap_10(sg_fd, false /* pmi */, 0 /* lba */,
                               resp_buff, RCAP_REPLY_LEN, true, verbose);
        if (0 == res) {
            last_blk_addr = sg_get_unaligned_be32(resp_buff + 0);
            if (0xffffffff != last_blk_addr) {
                block_size = sg_get_unaligned_be32(resp_buff + 4);
                printf("blocks: %u\n", last_blk_addr + 1);
                printf("block_length: %u\n", block_size);
                sz_mib = ((double)(last_blk_addr + 1) * block_size) /
                          (double)(1048576);
#ifdef SG3_UTILS_MINGW
                printf("capacity_mib: %g\n", sz_mib);
#else
                printf("capacity_mib: %.1f\n", sz_mib);
#endif
            } else
                do16 = true;
        } else
            return res;
    }
    if (do16) {
        /* within SERVICE ACTION IN. May need RW or root permissions. */
        res = sg_ll_readcap_16(sg_fd, false /* pmi */, false /* llba */,
                               resp_buff, RCAP16_REPLY_LEN, true, verbose);
        if (0 == res) {
            llast_blk_addr = sg_get_unaligned_be64(resp_buff + 0);
            block_size = sg_get_unaligned_be32(resp_buff + 8);
            printf("blocks: %" PRIu64 "\n", llast_blk_addr + 1);
            printf("block_length: %u\n", block_size);
            sz_mib = ((double)(llast_blk_addr + 1) * block_size) /
                      (double)(1048576);
#ifdef SG3_UTILS_MINGW
            printf("capacity_mib: %g\n", sz_mib);
#else
            printf("capacity_mib: %.1f\n", sz_mib);
#endif
            if (do_long) {
                uint8_t u[2];

                printf("ZBC_rc_basis: %d\n", ((resp_buff[12] & 0x30) >> 4));
                printf("p_type: %d\n", ((resp_buff[12] & 0xe) >> 1));
                printf("prot_en: %d\n", (resp_buff[12] & 0x1));
                printf("p_i_exponent: %d\n", ((resp_buff[13] & 0xf0) >> 4));
                printf("lbs_per_physical_block_exponent: %d\n",
                       (resp_buff[13] & 0xf));
                printf("lbpme: %d\n", !!(resp_buff[14] & 0x80));
                printf("lbprz: %d\n", !!(resp_buff[14] & 0x40));
                memcpy(u, resp_buff + 14, 2);
                u[0] &= 0x3f;
                printf("lowest_aligned_lba: %u\n", sg_get_unaligned_be16(u));
            }
        } else
            return res;
    }
    return 0;
}

#define MAX_REQ_SENSE_SZ 64

static int
do_cmd_sense(int sg_fd, bool hex, bool do_quiet, int verbose)
{
    bool something;
    int res, resp_len, sk, asc, ascq, progress, pr, rem;
    uint8_t * buff = aligned_buff;
    char b[128];

    res = sg_ll_request_sense(sg_fd, false /* DESC so fixed format */,
                              buff, MAX_REQ_SENSE_SZ, true, verbose);
    if (0 == res) {
        resp_len = buff[7] + 8;
        if (resp_len > aligned_buff_sz)
            resp_len = aligned_buff_sz;
        sk = (0xf & buff[2]);
        if (hex) {
            hex2stdout(buff, resp_len, 1);
            return 0;
        }
        something = false;
        if (verbose) {
            pr2serr("Decode response as sense data:\n");
            sg_print_sense(NULL, buff, resp_len, 0);
            if (verbose > 1) {
                pr2serr("\nOutput response in hex\n");
                hex2stderr(buff, resp_len, 1);
            }
            something = true;
        }
        asc = (resp_len > 12) ? buff[12] : 0;
        ascq = (resp_len > 13) ? buff[13] : 0;
        if (sg_get_sense_progress_fld(buff, resp_len, &progress)) {
            pr = (progress * 100) / 65536;
            rem = ((progress * 100) % 65536) / 656;
            printf("Operation in progress: %d.%d%% done\n", pr, rem);
            something = true;
        }
        if (0 == sk) {  /* NO SENSE */
            /* check for hardware threshold exceeded or warning */
            if ((0xb == asc) || (0x5d == asc))
                printf("%s\n", sg_get_asc_ascq_str(asc, ascq,
                                                   (int)sizeof(b), b));
            /* check for low power conditions */
            if (0x5e == asc)
                printf("%s\n", sg_get_asc_ascq_str(asc, ascq,
                                                   (int)sizeof(b), b));
            return 0;
        } else {
            if (! (something || verbose || do_quiet)) {
                pr2serr("Decode response as sense data:\n");
                sg_print_sense(NULL, buff, resp_len, 0);
            }
            return 0;
        }
    } else {
        sg_get_category_sense_str(res, sizeof(b), b, verbose);
        pr2serr("Request Sense command: %s\n", b);
        if (0 == verbose)
            pr2serr("    try the '-v' option for more information\n");
    }
    return res;
}

#define PERF_DESC_SZ 28

/* cmd_arg is kBytes/sec if given (i.e. 1000 bytes per second */
static int
do_cmd_speed(int sg_fd, int cmd_arg, const struct sdparm_opt_coll * op)
{
    int res;
    unsigned int u;
    unsigned int last_lba = 0xfffffffe;
    unsigned int rw_time = 1000;

    if (cmd_arg >= 0) {
        uint8_t * perf_desc = aligned_buff;
#if 0
        int kbps;

        if (0 == cmd_arg)
            kbps = 0xffff;
        else
            kbps = cmd_arg;
        res = sg_ll_set_cd_speed(sg_fd, 0 /* rot_control */, kbps,
                                 0 /* drv_write_speed */, true, op->verbose);
#else
        if (0 == cmd_arg)
            perf_desc[0] |= 0x4;  /* set RDD bit: restore drive defaults */
        else {
            sg_put_unaligned_be32((uint32_t)last_lba, perf_desc + 8);
            sg_put_unaligned_be32((uint32_t)cmd_arg, perf_desc + 12);
            sg_put_unaligned_be32((uint32_t)rw_time, perf_desc + 16);
            sg_put_unaligned_be32((uint32_t)cmd_arg, perf_desc + 20);
            sg_put_unaligned_be32((uint32_t)rw_time, perf_desc + 24);
        }
        /* performance (type=0), tolerance 10% nominal, read speed */
        res = sg_ll_set_streaming(sg_fd, 0x0 /* type */, perf_desc,
                                  PERF_DESC_SZ, true, op->verbose);
        if (res) {
            if (SG_LIB_CAT_NOT_READY == res)
                pr2serr("Set Streaming failed, device not ready\n");
            else
                pr2serr("Set Streaming failed, add '-v' for more "
                        "information\n");
        }
    } else {
        const int max_num_desc = 16;
        uint8_t * buff = aligned_buff;
        int buff_sz = 8 + (16 * 16);
        unsigned int lba;

        /* performance (type=0), tolerance 10% nominal, read speed */
        res = sg_ll_get_performance(sg_fd, 0x10 /* data_type */,
                                    0 /* starting_lba */,
                                    max_num_desc, 0 /* type */, buff,
                                    buff_sz, true, op->verbose);
        if (0 == res) {
            if (op->verbose) {
                lba = sg_get_unaligned_be32(buff + 8);
                printf("starting LBA: %u\n", lba);
            }
            u = sg_get_unaligned_be32(buff + 12);
            if (op->do_quiet)
                printf("%u\n", u);
            else
                printf("Nominal speed at starting LBA: %u kiloBytes/sec\n",
                       u);
            if (op->verbose) {
                lba = sg_get_unaligned_be32(buff + 16);
                printf("ending LBA: %u\n", lba);
            }
            u = sg_get_unaligned_be32(buff + 20);
            if (1 == op->do_quiet)
                printf("%u\n", u);
            else if (0 == op->do_quiet)
                printf("Nominal speed at ending LBA: %u kiloBytes/sec\n",
                       u);
        }
    }
    return res;
#endif
}

static const char *
get_profile_str(int profile_num, char * buff)
{
    const struct sdparm_val_desc_t * pdp;

    for (pdp = sdparm_profile_arr; pdp->desc; ++pdp) {
        if (pdp->val == profile_num) {
            strcpy(buff, pdp->desc);
            return buff;
        }
    }
    snprintf(buff, 64, "0x%x", profile_num);
    return buff;
}

static void
decode_get_config_feature(int feature, uint8_t * bp, int len)
{
    int k, profile;
    char buff[128];

    switch (feature) {
    case 0:     /* Profile list */
        printf("Available profiles, profile of current media marked "
               "with * \n");
        for (k = 4; k < len; k += 4) {
            profile = sg_get_unaligned_be16(bp + k);
            printf("    %s   %s\n", get_profile_str(profile, buff),
                   ((bp[k + 2] & 1) ? "*" : ""));
        }
        break;
    default:
        /* ignore other features */
        break;
    }
}


static void
decode_get_config(uint8_t * resp, int max_resp_len, int len)
{
    int k, extra, feature;
    uint8_t * bp;

    if (max_resp_len < len) {
        printf("get_config: response to long for buffer, resp_len=%d>>>\n",
               len);
            len = max_resp_len;
    }
    if (len < 8) {
        printf("get_config: response length too short: %d\n", len);
        return;
    }
    bp = resp + 8;
    len -= 8;
    for (k = 0; k < len; k += extra, bp += extra) {
        extra = 4 + bp[3];
        feature = sg_get_unaligned_be16(bp + 0);
        if (0 != (extra % 4))
            printf("    get_config: additional length [%d] not a multiple "
                   "of 4, ignore\n", extra - 4);
        else
            decode_get_config_feature(feature, bp, extra);
    }
}

#define MAX_CONFIG_RESPLEN 2048

static int
do_cmd_profile(int sg_fd, const struct sdparm_opt_coll * op)
{
    int res, len;
    uint8_t * resp = aligned_buff;

    /* performance (type=0), tolerance 10% nominal, read speed */
    res = sg_ll_get_config(sg_fd, 0x0 /* rt */, 0 /* starting_lba */,
                           resp, MAX_CONFIG_RESPLEN, true, op->verbose);
    if (0 == res) {
        len = sg_get_unaligned_be32(resp + 0);
        decode_get_config(resp, MAX_CONFIG_RESPLEN, len);
    }
    return res;
}

const struct sdparm_command_t *
sdp_build_cmd(const char * cmd_str, bool * rwp, int * argp)
{
    int arg = -1;
    int len;
    const struct sdparm_command_t * scmdp;
    const char * eq_cp;
    const char * cp;
    char cbuff[16];

    eq_cp = strchr(cmd_str, '=');
    if (eq_cp) {
        len = eq_cp - cmd_str;
        if (len >= (int)sizeof(cbuff))
            return NULL;
        strncpy(cbuff, cmd_str, len);
        cbuff[len] = '\0';
        if (1 != sscanf(eq_cp + 1, "%d", &arg))
            return NULL;
        cp = cbuff;
    } else
        cp = cmd_str;
    if (argp)
        *argp = arg;

    for (scmdp = sdparm_command_arr; scmdp->name; ++scmdp) {
        if (sdp_strcase_eq(scmdp->name, cp))
            break;
    }
    if ((NULL == scmdp->name) && (strlen(cp) >= 2)) {
        for (scmdp = sdparm_command_arr; scmdp->name; ++scmdp) {
            len = strlen(scmdp->min_abbrev);
            if (0 == memcmp(scmdp->min_abbrev, cp, 2))
                break;
        }
    }
    if (scmdp->name) {
        if (rwp) {
            if ((CMD_READY  == scmdp->cmd_num) ||
                (CMD_SENSE  == scmdp->cmd_num) ||
                (CMD_CAPACITY  == scmdp->cmd_num))
                *rwp = false;
            else
                *rwp = true;
        }
        return scmdp;
    } else
        return NULL;
}

void
sdp_enumerate_commands()
{
    const struct sdparm_command_t * scmdp;

    for (scmdp = sdparm_command_arr; scmdp->name; ++scmdp) {
        if (scmdp->extra_arg)
            printf("  %s[=%s]\n", scmdp->name, scmdp->extra_arg);
        else
            printf("  %s\n", scmdp->name);
    }
}

/* Returns 0 if successful */
int
sdp_process_cmd(int sg_fd, const struct sdparm_command_t * scmdp, int cmd_arg,
                int pdt, const struct sdparm_opt_coll * op)
{
    int res, progress;

    if (! (op->flexible ||
          (CMD_READY == scmdp->cmd_num) ||
          (CMD_SENSE == scmdp->cmd_num) ||
          (0 == pdt) || (5 == pdt)) ) {
        pr2serr("this command only valid on a disk or cd/dvd; use "
                "'--flexible' to override\n");
        return SG_LIB_SYNTAX_ERROR;
    }
    if (NULL == allocate_if_needed())
        return sg_convert_errno(ENOMEM);
    switch (scmdp->cmd_num)
    {
    case CMD_CAPACITY:
        res = do_cmd_read_capacity(sg_fd, op->do_long, op->verbose);
        break;
    case CMD_EJECT:
        res = sg_ll_start_stop_unit(sg_fd, false /* immed */, 0 /* fl_num */,
                                    0 /* power cond. */, false /* fl */,
                                    true /*loej */, false /* start */,
                                    true /* noisy */, op->verbose);
        break;
    case CMD_LOAD:
        res = sg_ll_start_stop_unit(sg_fd, false, 0, 0, false, true, true,
                                    true, op->verbose);
        break;
    case CMD_PROFILE:
        res = do_cmd_profile(sg_fd, op);
        break;
    case CMD_READY:
        progress = -1;
        res = sg_ll_test_unit_ready_progress(sg_fd, false, &progress, false,
                                             op->verbose);
        if (0 == res)
            printf("Ready\n");
        else {
            if (progress >= 0)
                printf("Not ready, progress indication: %d%% done\n",
                       (progress * 100) / 65536);
            else
                printf("Not ready\n");
        }
        break;
    case CMD_SENSE:
        res = do_cmd_sense(sg_fd, (op->do_hex > 0), (op->do_quiet > 0),
                           op->verbose);
        break;
    case CMD_SPEED:
        res = do_cmd_speed(sg_fd, cmd_arg, op);
        break;
    case CMD_START:
        res = sg_ll_start_stop_unit(sg_fd, false, 0, 0, false, false, true,
                                    true, op->verbose);
        break;
    case CMD_STOP:
        res = sg_ll_start_stop_unit(sg_fd, false, 0, 0, false, false, false,
                                    true, op->verbose);
        break;
    case CMD_SYNC:
        res = sg_ll_sync_cache_10(sg_fd, false, false, 0, 0, 0, true,
                                  op->verbose);
        break;
    case CMD_UNLOCK:
        res = sg_ll_prevent_allow(sg_fd, 0, true, op->verbose);
        break;
    default:
        pr2serr("unknown cmd number [%d]\n", scmdp->cmd_num);
        res = SG_LIB_SYNTAX_ERROR;
    }
    return deallocate_res(res);
}
