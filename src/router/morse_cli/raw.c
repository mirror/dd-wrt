/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "portable_endian.h"

#include "command.h"
#include "stats_format.h"
#include "transport/transport.h"
#include "utilities.h"
#define RAW_CMD_MAX_3BIT_SLOTS          (0b111)
#define RAW_CMD_MIN_SLOT_DUR_US         (500)
#define RAW_CMD_MAX_SLOT_DUR_US         (RAW_CMD_MIN_SLOT_DUR_US + (200 * (1 << 11) - 1))
#define RAW_CMD_MAX_START_TIME_US       (UINT8_MAX * 2 * 1024)
#define RAW_CMD_MAX_AID                 (2007) /* set by linux */

static struct {
    struct arg_csi *slot_def;
    struct arg_lit *cross_slot;
    struct arg_csi *aid_group;
    struct arg_int *start_time;
    struct arg_csi *bcn_spread;
    struct arg_csi *praw;
    struct arg_rex *enable;
    struct arg_int *id;
} args;

int raw_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Configure Restricted Access Window parameters",
        args.slot_def = arg_csi0("s", "slot_def", "<RAW duration (usec)>,<number of slots>", 2,
            "Slot definition of RAW assignment. Required for new configs."),
        args.cross_slot = arg_lit0("x", "cross_slot",
            "Enable cross slot bleed (requires --slot_def)"),
        args.aid_group = arg_csi0("a", "aid_group", "<start AID>,<end AID>", 2,
            "AID range for the config"),
        args.start_time = arg_int0("t", "start_time", "<start time (usec)>",
            "Start time for the RAW window from the end of the frame"),
        args.bcn_spread = arg_csi0("b", "bcn_spread",
            "<max beacons to spread over>,<nominal STAs per beacon>", 2, "Use beacon spreading"),
        args.praw = arg_csi0("p", "praw", "<periodicity>,<validity (-1 for persistent)>,<offset>",
            3, "Use Periodic RAW"),
        args.enable = arg_rex1(NULL, NULL, "(enable|disable|delete)", "{enable|disable|delete}", 0,
            "enable/disable or delete RAW configs. If <id> is 0, globally enable/disable/delete"),
        args.id = arg_int1(NULL, NULL, "<id>", "ID for the RAW config. 0 is reserved as 'global'"));
    return 0;
}

int raw(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morse_cmd_req_config_raw *req;
    union morse_cmd_raw_tlvs *tlv;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    uint16_t aid_start;
    uint16_t aid_end;

    static const size_t cmd_max_size = sizeof(*req) + (sizeof(*tlv) * MORSE_CMD_RAW_TLV_TAG_LAST);

    /* allocate for largest possible size */
    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, cmd_max_size);
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_config_raw);
    tlv = (union morse_cmd_raw_tlvs *) &req->variable[0];

    memset(req, 0 , cmd_max_size);

    if (args.id->count != 0)
    {
        if (args.id->ival[0] > UINT16_MAX)
        {
            mctrl_err("Invalid RAW ID, must be 1 - %u (or 0 for global)\n", UINT16_MAX);
            ret = -1;
            goto exit;
        }
        req->id = htole16(args.id->ival[0]);
    }

    if (args.enable->count)
    {
        if (strcmp("enable", args.enable->sval[0]) == 0)
        {
            req->flags |= htole32(MORSE_CMD_CFG_RAW_FLAG_ENABLE);
        }
        if (strcmp("delete", args.enable->sval[0]) == 0)
        {
            req->flags |= htole32(MORSE_CMD_CFG_RAW_FLAG_DELETE);
            goto done;
        }
        /* else, disable */
    }

    if (args.slot_def->count)
    {
        uint8_t num_slots = args.slot_def->ival[0][1];

        tlv->slot_def.tag = MORSE_CMD_RAW_TLV_TAG_SLOT_DEF;
        tlv->slot_def.raw_duration_us = htole32(args.slot_def->ival[0][0]);
        tlv->slot_def.num_slots = num_slots;
        tlv->slot_def.cross_slot_bleed = (args.cross_slot->count != 0);

        if (num_slots < 1 || num_slots > 63)
        {
            mctrl_err("Invalid number of slots, must be 1-63\n");
            ret = -1;
            goto exit;
        }


        if (le32toh(tlv->slot_def.raw_duration_us) < (num_slots * RAW_CMD_MIN_SLOT_DUR_US) ||
                le32toh(tlv->slot_def.raw_duration_us) > (num_slots * RAW_CMD_MAX_SLOT_DUR_US))
        {
            mctrl_err("Invalid RAW duration. min: %u, max: %u\n"
                    "Try reducing the number of slots\n",
                    num_slots * RAW_CMD_MIN_SLOT_DUR_US,
                    num_slots * RAW_CMD_MAX_SLOT_DUR_US);
            ret = -1;
            goto exit;
        }

        tlv = (union morse_cmd_raw_tlvs *)(((uint8_t*)tlv) + sizeof(tlv->slot_def));
    }
    else
    {
        if (args.cross_slot->count)
        {
            mctrl_err("Cross slot is ignored without a slot_def\n");
            /* dont need to exit here. Warning the user is enough */
        }
    }

    if (args.aid_group->count)
    {
        tlv->group.tag = MORSE_CMD_RAW_TLV_TAG_GROUP;
        aid_start = args.aid_group->ival[0][0];
        aid_end = args.aid_group->ival[0][1];

        if (aid_start > aid_end)
        {
            mctrl_err("AID start (%u) should be less than AID end (%u)\n",
                    aid_start, aid_end);
            ret = -1;
            goto exit;
        }

        if (aid_start < 1 || aid_end > RAW_CMD_MAX_AID)
        {
            mctrl_err("AID range is invalid (min: 1, max: %u)\n", RAW_CMD_MAX_AID);
            ret = -1;
            goto exit;
        }

        tlv->group.aid_start = htole16(aid_start);
        tlv->group.aid_end = htole16(aid_end);

        tlv = (union morse_cmd_raw_tlvs *)(((uint8_t*)tlv) + sizeof(tlv->group));
    }

    if (args.start_time->count)
    {
        tlv->start_time.tag = MORSE_CMD_RAW_TLV_TAG_START_TIME;
        tlv->start_time.start_time_us = htole32(args.start_time->ival[0]);

        if (le32toh(tlv->start_time.start_time_us) > RAW_CMD_MAX_START_TIME_US) {
            mctrl_err("Invalid start time, must be 0-%u\n", RAW_CMD_MAX_START_TIME_US);
            ret = -1;
            goto exit;
        }

        tlv = (union morse_cmd_raw_tlvs *)(((uint8_t*)tlv) + sizeof(tlv->start_time));
    }

    if (args.praw->count && args.bcn_spread->count)
    {
        mctrl_err("Beacon spreading and PRAW are not supported together\n");
        ret = -1;
        goto exit;
    }

    if (args.praw->count)
    {
        tlv->praw.tag = MORSE_CMD_RAW_TLV_TAG_PRAW;

        if (args.praw->ival[0][0] > UINT8_MAX || args.praw->ival[0][0] < 1) {
            mctrl_err("Invalid periodicity, must be 1-%u\n", UINT8_MAX);
            ret = -1;
            goto exit;
        }
        tlv->praw.periodicity = args.praw->ival[0][0];

        if (args.praw->ival[0][1] != -1 &&
                (args.praw->ival[0][1] > UINT8_MAX || args.praw->ival[0][0] < 1)) {
            mctrl_err("Invalid validity, must be 1-%u, or -1 for persistent\n", UINT8_MAX);
            ret = -1;
            goto exit;
        }
        if (args.praw->ival[0][1] == -1) {
            tlv->praw.validity = 255;
            tlv->praw.refresh_on_expiry = 1;
        } else {
            tlv->praw.validity = args.praw->ival[0][1];
        }

        if (args.praw->ival[0][2] > UINT8_MAX || args.praw->ival[0][2] < 0) {
            mctrl_err("Invalid start offset, must be 0-%u\n", UINT8_MAX);
            ret = -1;
            goto exit;
        }
        tlv->praw.start_offset = args.praw->ival[0][2];

        if (tlv->praw.start_offset >= tlv->praw.periodicity) {
            mctrl_err("Start offset (%u) must be less than periodicity (%u)\n",
                    tlv->praw.start_offset, tlv->praw.periodicity);
            ret = -1;
            goto exit;
        }

        tlv = (union morse_cmd_raw_tlvs *)(((uint8_t*)tlv) + sizeof(tlv->praw));
    }

    if (args.bcn_spread->count)
    {
        tlv->bcn_spread.tag = MORSE_CMD_RAW_TLV_TAG_BCN_SPREAD;
        tlv->bcn_spread.max_spread = htole16(args.bcn_spread->ival[0][0]);
        tlv->bcn_spread.nominal_sta_per_bcn = htole16(args.bcn_spread->ival[0][1]);
        tlv = (union morse_cmd_raw_tlvs *)(((uint8_t*)tlv) + sizeof(tlv->bcn_spread));
    }

    if ((uint8_t*)tlv > req->variable)
    {
        if (req->id == 0) {
            mctrl_err("Can't set options when configuring global RAW\n");
            ret = -1;
            goto exit;
        }
        req->flags |= htole32(MORSE_CMD_CFG_RAW_FLAG_UPDATE);
    }

    morsectrl_transport_set_cmd_data_length(req_tbuff, (uint8_t*)tlv - (uint8_t*)req);

done:
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_CONFIG_RAW,
                                 req_tbuff, rsp_tbuff);

exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER_DEPRECATED(raw, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
