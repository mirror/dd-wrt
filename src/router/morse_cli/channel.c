/*
 * Copyright 2020 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "portable_endian.h"
#include "command.h"
#include "channel.h"
#include "transport/transport.h"
#include "utilities.h"

static struct
{
    struct arg_lit *all_channels;
    struct arg_int *frequency;
    struct arg_int *operating_bandwidth;
    struct arg_int *primary_bandwidth;
    struct arg_int *primary_idx;
    struct arg_lit *ignore_reg_power;
    struct arg_lit *json_format;
} args;

int channel_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Get (default) or set channel parameters",
                     args.all_channels = arg_lit0("a", NULL, "prints all the channel "
                                                  "information i.e. full, DTIM, and current"),
                     args.frequency = arg_int0("c", NULL, "<freq>",
                                               "channel frequency in kHz"),
                     args.operating_bandwidth = arg_int0("o", NULL, "<operating BW>",
                                                         "operating bandwidth in MHz"),
                     args.primary_bandwidth = arg_int0("p", NULL, "<primary BW>",
                                                       "primary bandwidth in MHz"),
                     args.primary_idx = arg_int0("n", NULL, "<primary chan index>",
                                                 "primary 1 MHz channel index"),
                     #ifndef MORSE_CLIENT
                     args.ignore_reg_power = arg_lit0("r", NULL, "ignores regulatory max tx power"),
                     #endif
                     args.json_format = arg_lit0("j", NULL,
                                                 "prints full channel information in easily "
                                                 "parsable JSON format"));
    return 0;
}

static void channel_set_invalid_channel_handler(struct morsectrl *mors,
    struct morse_cmd_req_set_channel *cmd_set, struct morsectrl_transport_buff *cmd_get_tbuff,
    struct morsectrl_transport_buff *rsp_get_tbuff, struct morse_cmd_resp_get_channel *resp_get)
{
    uint32_t freq_khz = HZ_TO_KHZ(le32toh(cmd_set->op_chan_freq_hz));
    uint8_t op_bw_mhz = cmd_set->op_bw_mhz;
    uint8_t pri_bw_mhz = cmd_set->pri_bw_mhz;
    uint8_t pri_1mhz_chan_idx = cmd_set->pri_1mhz_chan_idx;
    int ret = -1;

    /* Get current configured channel from chip */
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_GET_CHANNEL_FULL,
                                cmd_get_tbuff, rsp_get_tbuff);
    if (ret < 0)
    {
        mctrl_err("Failed to get channel\n");
        return;
    }

    /* Print the invalid channel configuration combined with parameters obtained from chip */
    mctrl_err("Invalid combination of parameters - "
        "freq=%u, bw%s=%u, primary bw%s=%u, primary idx%s=%u\n", freq_khz,
        (op_bw_mhz == MORSE_CMD_CHANNEL_BW_NOT_SET) ? " (from chip)" : "",
        (op_bw_mhz == MORSE_CMD_CHANNEL_BW_NOT_SET) ? resp_get->op_chan_bw_mhz : op_bw_mhz,
        (pri_bw_mhz == MORSE_CMD_CHANNEL_BW_NOT_SET) ? " (from chip)" : "",
        (pri_bw_mhz == MORSE_CMD_CHANNEL_BW_NOT_SET) ? resp_get->pri_chan_bw_mhz : pri_bw_mhz,
        (pri_1mhz_chan_idx == MORSE_CMD_CHANNEL_IDX_NOT_SET) ? " (from chip)" : "",
        (pri_1mhz_chan_idx == MORSE_CMD_CHANNEL_IDX_NOT_SET) ?
            resp_get->pri_1mhz_chan_idx : pri_1mhz_chan_idx);
}

int channel(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    uint32_t freq_khz = 0;
    uint8_t op_channel_bandwidth = MORSE_CMD_CHANNEL_BW_NOT_SET;
    uint8_t primary_channel_bandwidth = MORSE_CMD_CHANNEL_BW_NOT_SET;
    uint8_t primary_1mhz_channel_index = MORSE_CMD_CHANNEL_IDX_NOT_SET;
    struct morse_cmd_req_set_channel *cmd_set;
    struct morse_cmd_resp_get_channel *resp_get;
    bool set_freq = false;
    bool get_all_channels = false;
    bool json = false;
    uint8_t reg_tx_power_set = 1;

    struct morsectrl_transport_buff *cmd_set_tbuff;
    struct morsectrl_transport_buff *rsp_set_tbuff;
    struct morsectrl_transport_buff *cmd_get_tbuff;
    struct morsectrl_transport_buff *rsp_get_tbuff;

    cmd_set_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*cmd_set));
    rsp_set_tbuff = morsectrl_transport_resp_alloc(mors->transport,
                        sizeof(struct morse_cmd_resp_set_channel));
    cmd_get_tbuff = morsectrl_transport_cmd_alloc(mors->transport,
                        sizeof(struct morse_cmd_req_get_channel));
    rsp_get_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*resp_get));

    if (!cmd_set_tbuff || !rsp_set_tbuff || !cmd_get_tbuff || !rsp_get_tbuff)
        goto exit;


    cmd_set = TBUFF_TO_REQ(cmd_set_tbuff, struct morse_cmd_req_set_channel);
    resp_get = TBUFF_TO_RSP(rsp_get_tbuff, struct morse_cmd_resp_get_channel);

    if (args.frequency->count)
    {
        freq_khz = args.frequency->ival[0];
        set_freq = true;
    }

    if (args.operating_bandwidth->count)
    {
        op_channel_bandwidth = args.operating_bandwidth->ival[0];
        set_freq = true;
    }

    if (args.primary_bandwidth->count)
    {
        primary_channel_bandwidth = args.primary_bandwidth->ival[0];
        set_freq = true;
    }

    if (args.primary_idx->count)
    {
        primary_1mhz_channel_index = args.primary_idx->ival[0];
        set_freq = true;
    }

    json = (args.json_format->count > 0);

    get_all_channels = (args.all_channels->count > 0);

#ifndef MORSE_CLIENT
    reg_tx_power_set = !(args.ignore_reg_power->count > 0);
#endif

    if (set_freq)
    {
        if (!freq_khz)
        {
            mctrl_err("Channel frequency [-c] option must be specified\n");
            goto exit;
        }

        cmd_set->op_chan_freq_hz = htole32(KHZ_TO_HZ(freq_khz));
        cmd_set->op_bw_mhz = op_channel_bandwidth;
        cmd_set->pri_bw_mhz = primary_channel_bandwidth;
        cmd_set->pri_1mhz_chan_idx = primary_1mhz_channel_index;
        cmd_set->dot11_mode = 0; /* TODO */
        cmd_set->reg_tx_power_set = reg_tx_power_set;

        ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_CHANNEL,
                                    cmd_set_tbuff, rsp_set_tbuff);

        if (ret == MORSE_RET_SET_INVALID_CHAN_CONFIG)
        {
            channel_set_invalid_channel_handler(mors, cmd_set, cmd_get_tbuff,
                                                rsp_get_tbuff, resp_get);
            goto exit;
        }
        else if (ret < 0)
        {
            mctrl_err("Failed to set channel\n");
            goto exit;
        }
    }

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_GET_CHANNEL_FULL,
                                 cmd_get_tbuff, rsp_get_tbuff);

    if (ret < 0)
    {
        mctrl_err("Failed to get channel frequency\n");
        goto exit;
    }
    if (json)
    {
        mctrl_print("{\n" \
               "    \"channel_frequency\":%d,\n" \
               "    \"channel_op_bw\":%d,\n" \
               "    \"channel_primary_bw\":%d,\n" \
               "    \"channel_index\":%d,\n" \
               "    \"bw_mhz\":%d\n" \
               "}\n",
               (le32toh(resp_get->op_chan_freq_hz) / 1000),
               resp_get->op_chan_bw_mhz,
               resp_get->pri_chan_bw_mhz,
               resp_get->pri_1mhz_chan_idx,
               resp_get->op_chan_bw_mhz);
    }
    else
    {
        mctrl_print("Full Channel Information\n" \
               "\tOperating Frequency: %d kHz\n" \
               "\tOperating BW: %d MHz\n" \
               "\tPrimary BW: %d MHz\n" \
               "\tPrimary Channel Index: %d\n",
               (le32toh(resp_get->op_chan_freq_hz) / 1000),
               resp_get->op_chan_bw_mhz,
               resp_get->pri_chan_bw_mhz,
               resp_get->pri_1mhz_chan_idx);
    }

    if (get_all_channels)
    {
        ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_GET_CHANNEL_DTIM,
                                     cmd_get_tbuff, rsp_get_tbuff);
        if (ret < 0)
        {
            mctrl_err("Failed to get channel frequency\n");
            goto exit;
        }

        mctrl_print("DTIM Channel Information\n" \
               "\tOperating Frequency: %d kHz\n" \
               "\tOperating BW: %d MHz\n" \
               "\tPrimary BW: %d MHz\n" \
               "\tPrimary Channel Index: %d\n",
               (le32toh(resp_get->op_chan_freq_hz) / 1000),
               resp_get->op_chan_bw_mhz,
               resp_get->pri_chan_bw_mhz,
               resp_get->pri_1mhz_chan_idx);

        ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_GET_CHANNEL,
                                     cmd_get_tbuff, rsp_get_tbuff);
        if (ret < 0)
        {
            mctrl_err("Failed to get channel frequency\n");
            goto exit;
        }

        mctrl_print("Current Channel Information\n" \
               "\tOperating Frequency: %d kHz\n" \
               "\tOperating BW: %d MHz\n" \
               "\tPrimary BW: %d MHz\n" \
               "\tPrimary Channel Index: %d\n",
               (le32toh(resp_get->op_chan_freq_hz) / 1000),
               resp_get->op_chan_bw_mhz,
               resp_get->pri_chan_bw_mhz,
               resp_get->pri_1mhz_chan_idx);
    }

exit:
    morsectrl_transport_buff_free(cmd_set_tbuff);
    morsectrl_transport_buff_free(rsp_set_tbuff);
    morsectrl_transport_buff_free(cmd_get_tbuff);
    morsectrl_transport_buff_free(rsp_get_tbuff);
    return ret;
}

MM_CLI_HANDLER(channel, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
