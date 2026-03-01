/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "portable_endian.h"
#include "command.h"
#include "utilities.h"

static struct
{
    struct arg_str *add;
    struct arg_lit *clear;
    struct arg_rex *oui;
    struct arg_lit *reset_oui_whitelist;
    struct arg_lit *beacons;
    struct arg_lit *probes;
    struct arg_lit *assoc;
} args;

int vendor_ie_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Manipulate vendor information elements",
        args.add = arg_str0("a", "add", "<bytes>", "Add a vendor element (hex string)"),
        args.clear = arg_lit0("c", "clear", "Clear previously added vendor elements"),
        args.oui = arg_rex0("o", "oui", "[a-z0-9]{6}", "<OUI>", ARG_REX_ICASE,
            "Add an OUI to the vendor IE whitelist (hex string)"),
        args.reset_oui_whitelist = arg_lit0("r", NULL, "Reset configured OUI whitelist"),
        args.beacons = arg_lit0("b", "beacon", "Apply to beacons"),
        args.probes = arg_lit0("p", "probe", "Apply to probe requests/responses"),
        args.assoc = arg_lit0("s", "assoc", "Apply to assoc requests/responses"));
    return 0;
}

int vendor_ie(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = 0;
    size_t length = 0;
    int count;

    struct morse_cmd_req_vendor_ie_config *req_vie;
    struct morse_cmd_resp_vendor_ie_config *rsp_vie;
    struct morsectrl_transport_buff *req_tbuff = NULL;
    struct morsectrl_transport_buff *rsp_tbuff = NULL;

    uint16_t opcode;
    uint16_t mgmt_type_mask;

    count = args.add->count + args.clear->count + args.oui->count + args.reset_oui_whitelist->count;

    if (count > 1)
    {
        mctrl_err("Specify only one of [-a, -o, -r, -c]\n");
        return -1;
    }

    if (count == 0)
    {
        mctrl_err("You must specify one of [-a, -o, -r, -c]\n");
        return -1;
    }

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req_vie));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp_vie));
    if (!req_tbuff || !rsp_tbuff)
    {
        ret = -1;
        goto exit;
    }

    req_vie = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_vendor_ie_config);
    rsp_vie = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_vendor_ie_config);

    if (req_vie == NULL ||
        rsp_vie == NULL)
    {
        ret = -1;
        goto exit;
    }

    memset(req_vie, 0, sizeof(*req_vie));
    opcode = MORSE_CMD_VENDOR_IE_OP_INVALID;
    mgmt_type_mask = 0;

    if (args.probes->count)
    {
        mgmt_type_mask |= (MORSE_CMD_VENDOR_IE_TYPE_FLAG_PROBE_REQ |
                                    MORSE_CMD_VENDOR_IE_TYPE_FLAG_PROBE_RESP);
    }

    if (args.assoc->count)
    {
        mgmt_type_mask |= (MORSE_CMD_VENDOR_IE_TYPE_FLAG_ASSOC_REQ |
                                    MORSE_CMD_VENDOR_IE_TYPE_FLAG_ASSOC_RESP);
    }

    if (args.beacons->count)
    {
        mgmt_type_mask |= MORSE_CMD_VENDOR_IE_TYPE_FLAG_BEACON;
    }

    if (args.add->count)
    {
        const char *ie_str = args.add->sval[0];
        length = strlen(ie_str);
        if (length & 1)
        {
            mctrl_err("Odd number of characters in data bytestring\n");
            ret = -1;
            goto exit;
        }
        length = length / 2;

        if (length > sizeof(req_vie->data))
        {
            mctrl_err("Vendor IE has too many bytes %zu\n", length);
            ret = -1;
            goto exit;
        }

        opcode = MORSE_CMD_VENDOR_IE_OP_ADD_ELEMENT;
        if (hexstr2bin(ie_str, req_vie->data, length))
        {
            mctrl_err("Invalid hex string\n");
            ret = -1;
            goto exit;
        }
    }

    if (args.oui->count)
    {
        const char *oui_str = args.oui->sval[0];
        length = strlen(oui_str) / 2;
        opcode = MORSE_CMD_VENDOR_IE_OP_ADD_FILTER;
        hexstr2bin(oui_str, req_vie->data, length);
    }

    if (args.reset_oui_whitelist->count)
    {
        opcode = MORSE_CMD_VENDOR_IE_OP_CLEAR_FILTERS;
    }

    if (args.clear->count)
    {
        opcode = MORSE_CMD_VENDOR_IE_OP_CLEAR_ELEMENTS;
    }

    req_vie->opcode = htole16(opcode);
    req_vie->mgmt_type_mask = htole16(mgmt_type_mask);
    /* set the length used in command */
    morsectrl_transport_set_cmd_data_length(req_tbuff,
                                length + sizeof(*req_vie) - sizeof(req_vie->data));

    if (mgmt_type_mask == 0)
    {
        mctrl_err("No frame type specified\n");
        goto exit;
    }

    ret = morsectrl_send_command(mors->transport,
                                 MORSE_CMD_ID_VENDOR_IE_CONFIG,
                                 req_tbuff,
                                 rsp_tbuff);
exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);

    return ret;
}

MM_CLI_HANDLER(vendor_ie, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
