/*
 * Copyright 2023 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "portable_endian.h"
#include "command.h"
#include "utilities.h"

#define MBCA_CONFIG_MIN 1
#define MBCA_CONFIG_MAX 3
#define MIN_BEACON_GAP_MIN 5
#define MIN_BEACON_GAP_MAX 100
#define TBTT_ADJ_INT_MIN 30
#define TBTT_ADJ_INT_MAX 65
#define BEACON_TIMING_REP_INT_MIN 1
#define BEACON_TIMING_REP_INT_MAX 255
#define MBSS_SCAN_DURATION_MIN 2048
#define MBSS_SCAN_DURATION_MAX 10240

static struct {
    struct arg_int *mbca_config;
    struct arg_int *scan_duration;
    struct arg_int *beacon_interval;
    struct arg_int *beacon_gap;
    struct arg_int *tbtt_int;
} args;

int mbca_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args,
    "Configure Mesh beacon collision avoidance (do not use - for internal use by wpa_supplicant)",
    args.mbca_config = arg_rint1("m", NULL, "<MBCA config>", MBCA_CONFIG_MIN, MBCA_CONFIG_MAX,
        "1: enable TBTT selection, 3: enable TBTT selection and adjustment"),
    args.scan_duration = arg_rint1("s", NULL, "<scan duration>",
        MBSS_SCAN_DURATION_MIN, MBSS_SCAN_DURATION_MAX,
        "Initial scan duration in msecs to find peers ("
        STR(MBSS_SCAN_DURATION_MIN) "-" STR(MBSS_SCAN_DURATION_MAX) ")"),
    args.beacon_interval = arg_rint1("r", NULL, "<interval>",
        BEACON_TIMING_REP_INT_MIN, BEACON_TIMING_REP_INT_MAX,
        "Beacon Timing Report interval ("
        STR(BEACON_TIMING_REP_INT_MIN) "-" STR(BEACON_TIMING_REP_INT_MAX) ")"),
    args.beacon_gap = arg_rint1("g", NULL, "<min beacon gap>",
        MIN_BEACON_GAP_MIN, MIN_BEACON_GAP_MAX,
        "Minimum gap in msecs between our and neighbor's beacons ("
        STR(MIN_BEACON_GAP_MIN) "-" STR(MIN_BEACON_GAP_MAX) ")"),
    args.tbtt_int = arg_rint1("i", NULL, "<interval>",
        TBTT_ADJ_INT_MIN, TBTT_ADJ_INT_MAX,
        "TBTT adjustment timer interval in secs ("
        STR(TBTT_ADJ_INT_MIN) "-" STR(TBTT_ADJ_INT_MAX) ")"));
    return 0;
}

int mbca(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morse_cmd_req_set_mcba_conf *mbca_req = NULL;
    struct morsectrl_transport_buff *req_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*mbca_req));
    if (!req_tbuff)
    {
        goto exit;
    }

    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);
    if (!rsp_tbuff)
    {
        goto exit;
    }

    mbca_req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_mcba_conf);
    memset(mbca_req, 0, sizeof(*mbca_req));

    mbca_req->mbca_config = args.mbca_config->ival[0];
    mbca_req->mbss_start_scan_duration_ms = htole16(args.scan_duration->ival[0]);
    mbca_req->beacon_timing_report_interval = args.beacon_interval->ival[0];
    mbca_req->min_beacon_gap_ms = args.beacon_gap->ival[0];
    mbca_req->tbtt_adj_interval_ms = htole16(SECS_TO_MSECS(args.tbtt_int->ival[0]));

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_MCBA_CONF, req_tbuff,
            rsp_tbuff);

exit:
    if (req_tbuff)
    {
        morsectrl_transport_buff_free(req_tbuff);
    }

    if (rsp_tbuff)
    {
        morsectrl_transport_buff_free(rsp_tbuff);
    }

    return ret;
}

MM_CLI_HANDLER(mbca, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
