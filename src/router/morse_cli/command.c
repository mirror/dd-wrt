/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "portable_endian.h"
#include "utilities.h"

#include "command.h"

int morsectrl_send_command(struct morsectrl_transport *transport,
                           int message_id,
                           struct morsectrl_transport_buff *req,
                           struct morsectrl_transport_buff *resp)
{
    int ret = 0;
    struct request *request;
    struct response *response;

    if (!req || !resp)
    {
        ret = -ENOMEM;
        goto exit;
    }

    request = (struct request *)req->data;
    memset(&request->hdr, 0, sizeof(request->hdr));
    request->hdr.message_id = htole16(message_id);
    request->hdr.len = htole16(req->data_len - sizeof(struct request));
    request->hdr.flags = htole16(MORSE_CMD_TYPE_REQ);
    response = (struct response *)resp->data;

    ret = morsectrl_transport_send(transport, req, resp);

    if (ret < 0)
    {
        morsectrl_transport_debug(transport, "Message failed %d\n", ret);
        goto exit;
    }

    ret = response->status;
    if (ret)
    {
        if (ret != 110)
        {
            morsectrl_transport_debug(transport, "Command failed\n");
        }

        goto exit;
    }

exit:
    return ret;
} // NOLINT - checkstyle.py seems to think this brace is in the wrong place.
