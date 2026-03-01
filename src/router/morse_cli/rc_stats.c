/*
 * Copyright 2024 Morse Micro
* SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdint.h>

#include "command.h"
#include "mm_argtable3.h"
#include "portable_endian.h"
#include "transport/transport.h"
#include "utilities.h"

static const char *bw_mhz_from_rate_info(uint32_t rate_info)
{
    switch (rate_info & MORSE_CMD_RATE_INFO_BW_MASK)
    {
        case 0: return "1MHz";
        case 1: return "2MHz";
        case 2: return "4MHz";
        case 3: return "8MHz";
    }
    MCTRL_ASSERT(false, "not reached");
}

static unsigned int mcs_from_rate_info(uint32_t rate_info)
{
    return (rate_info & MORSE_CMD_RATE_INFO_MCS_MASK) >> MORSE_CMD_RATE_INFO_MCS_SHIFT;
}

static const char *guard_interval_from_rate_info(uint32_t rate_info)
{
    switch ((rate_info & MORSE_CMD_RATE_INFO_GUARD_MASK) >> MORSE_CMD_RATE_INFO_GUARD_SHIFT)
    {
        case 0: return "LGI";
        case 1: return "SGI";
    }
    MCTRL_ASSERT(false, "not reached");
}

static void print_rc_stats(struct morse_cmd_resp_get_rc_stats *rc_stats)
{
    /* This output format vaguely matches mmrc_debugfs.c in the Linux driver, but
     * fullmac rate control does not give as much detail about each rate so
     * there are fewer columns here. */
    mctrl_print("             -----Rate----- ---------Total---------\n");
    mctrl_print(" BW   Guard  MCS   SS Index     Success     Attempt\n");
    for (size_t i = 0; i < le32toh(rc_stats->n_entries); i++)
    {
        uint32_t rate_info = le32toh(rc_stats->entries[i].rate_info);
        mctrl_print("%5s %5s  MCS%-2u  1  %4zu %11u %11u\n",
                bw_mhz_from_rate_info(rate_info),
                guard_interval_from_rate_info(rate_info),
                mcs_from_rate_info(rate_info),
                i,
                rc_stats->entries[i].total_success,
                rc_stats->entries[i].total_sent);
    }
}

int rc_stats_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Read rate control statistics from the chip (fullmac only)");
    return 0;
}

int rc_stats(struct morsectrl *mors, int argc, char *argv[])
{
    const size_t rsp_bufsize = MORSE_CMD_CFM_LEN;
    struct morsectrl_transport_buff *cmd_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    struct morse_cmd_resp_get_rc_stats *rc_stats;
    int ret;

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, 0);
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, rsp_bufsize);
    if (!cmd_tbuff || !rsp_tbuff)
    {
        ret = -ENOMEM;
        goto exit;
    }

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_GET_RC_STATS, cmd_tbuff, rsp_tbuff);
    if (ret)
    {
        goto exit;
    }

    rc_stats = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_get_rc_stats);

    if (le32toh(rc_stats->n_entries) * sizeof(rc_stats->entries[0]) >
                rsp_bufsize - sizeof(*rc_stats)) {
        mctrl_err("Number of rate control entries too large for buffer: %u\n",
                  rc_stats->n_entries);
        ret = -EINVAL;
        goto exit;
    }

    print_rc_stats(rc_stats);
    ret = 0;

exit:
    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(rc_stats, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
