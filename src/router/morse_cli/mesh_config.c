/*
 * Copyright 2023 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#ifndef MORSE_WIN_BUILD
#include <net/if.h>
#endif
#include "portable_endian.h"

#include "command.h"
#include "utilities.h"

static struct
{
    struct arg_str *mesh_id;
    struct arg_int *beaconless;
    struct arg_int *peer_links;
} args;


int mesh_config_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Set Mesh configuration parameters",
        args.mesh_id = arg_str1("m", NULL, "<mesh id>", "Mesh ID as a hex string"),
        args.beaconless = arg_rint0("b", NULL, "<mode>", MORSE_CMD_MESH_BEACONLESS_MODE_DISABLE,
            MORSE_CMD_MESH_BEACONLESS_MODE_ENABLE, "Mesh beaconless mode, "
            STR(MORSE_CMD_MESH_BEACONLESS_MODE_ENABLE)": enable, "
        STR(MORSE_CMD_MESH_BEACONLESS_MODE_DISABLE)": disable"),
        args.peer_links = arg_rint1("p", NULL, "<max peer links>", MORSE_CMD_MESH_PEER_LINKS_MIN,
            MORSE_CMD_MESH_PEER_LINKS_MAX,
            "Maximum number of peer links. (" STR(MORSE_CMD_MESH_PEER_LINKS_MIN) "-"
            STR(MORSE_CMD_MESH_PEER_LINKS_MAX) ")"));
    return 0;
}

int mesh_config(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    size_t length = 0;
    struct morse_cmd_req_set_mesh_config *req;
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, 0);

    if (!req_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_set_mesh_config);

    memset(req, 0, sizeof(*req));

    length = strlen(args.mesh_id->sval[0]);
    if (!length || (length & 1))
    {
        mctrl_err("Invalid Mesh ID hex string length\n");
        ret = -1;
        goto exit;
    }
    length = length / 2;

    if (length > sizeof(req->mesh_id))
    {
        mctrl_err("Mesh ID invalid length:%zu, max allowed length is:%zu\n",
                length, sizeof(req->mesh_id));
        ret = -1;
        goto exit;
    }

    if (hexstr2bin(args.mesh_id->sval[0], req->mesh_id, length))
    {
        mctrl_err("Invalid Mesh ID hex string\n");
        ret = -1;
        goto exit;
    }
    req->mesh_id_len = length;

    if (args.beaconless->count > 0)
    {
        req->mesh_beaconless_mode = args.beaconless->ival[0];
    }

    req->max_plinks = args.peer_links->ival[0];
    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_SET_MESH_CONFIG,
                                 req_tbuff, rsp_tbuff);
exit:
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(mesh_config, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
