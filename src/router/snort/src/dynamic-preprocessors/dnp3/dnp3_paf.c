/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * Author: Ryan Jordan
 *
 * Protocol Aware Flushing (PAF) code for DNP3 preprocessor.
 *
 */

#include "spp_dnp3.h"
#include "dnp3_paf.h"
#include "sf_dynamic_preprocessor.h"

/* Forward declarations */
static PAF_Status DNP3Paf(void *ssn, void **user, const uint8_t *data,
                   uint32_t len, uint64_t *flags, uint32_t *fp, uint32_t *fp_eoh);

/* State-tracking structs */
typedef enum _dnp3_paf_state
{
    DNP3_PAF_STATE__START_1 = 0,
    DNP3_PAF_STATE__START_2,
    DNP3_PAF_STATE__LENGTH,
    DNP3_PAF_STATE__SET_FLUSH
} dnp3_paf_state_t;

typedef struct _dnp3_paf_data
{
    dnp3_paf_state_t state;
    uint8_t dnp3_length;
    uint16_t real_length;
} dnp3_paf_data_t;

static uint8_t dnp3_paf_id = 0;

static int DNP3PafRegisterPort (struct _SnortConfig *sc, uint16_t port, tSfPolicyId policy_id)
{
    if (!_dpd.isPafEnabled())
        return 0;

    dnp3_paf_id = _dpd.streamAPI->register_paf_port(sc, policy_id, port, 0, DNP3Paf, true);
    dnp3_paf_id = _dpd.streamAPI->register_paf_port(sc, policy_id, port, 1, DNP3Paf, true);

    return 0;
}

#ifdef TARGET_BASED
int DNP3AddServiceToPaf (struct _SnortConfig *sc, uint16_t service, tSfPolicyId policy_id)
{
    if (!_dpd.isPafEnabled())
        return 0;

    dnp3_paf_id = _dpd.streamAPI->register_paf_service(sc, policy_id, service, 0, DNP3Paf, true);
    dnp3_paf_id = _dpd.streamAPI->register_paf_service(sc, policy_id, service, 1, DNP3Paf, true);

    return 0;
}
#endif

/* Function: DNP3Paf()

   Purpose: DNP3 PAF callback.
            Statefully inspects DNP3 traffic from the start of a session,
            Reads up until the length octet is found, then sets a flush point.
            The flushed PDU is a DNP3 Link Layer frame, the preprocessor
            handles reassembly of frames into Application Layer messages.

   Arguments:
     void * - stream5 session pointer
     void ** - DNP3 state tracking structure
     const uint8_t * - payload data to inspect
     uint32_t - length of payload data
     uint32_t - flags to check whether client or server
     uint32_t * - pointer to set flush point
     uint32_t * - pointer to set header flush point

   Returns:
    PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
*/

static PAF_Status DNP3Paf(void *ssn, void **user, const uint8_t *data,
                     uint32_t len, uint64_t *flags, uint32_t *fp, uint32_t *fp_eoh)
{
    dnp3_paf_data_t *pafdata = *(dnp3_paf_data_t **)user;
    uint32_t bytes_processed = 0;

    /* Allocate state object if it doesn't exist yet. */
    if (pafdata == NULL)
    {
        pafdata = calloc(1, sizeof(dnp3_paf_data_t));
        if (pafdata == NULL)
            return PAF_ABORT;

        *user = pafdata;
    }

    /* Process this packet 1 byte at a time */
    while (bytes_processed < len)
    {
        uint16_t user_data = 0;
        uint16_t num_crcs = 0;

        switch (pafdata->state)
        {
            /* Check the Start bytes. If they are not \x05\x64, don't advance state.
               Could be out of sync, junk data between frames, mid-stream pickup, etc. */
            case DNP3_PAF_STATE__START_1:
                if (((uint8_t) *(data + bytes_processed)) == DNP3_START_BYTE_1)
                    pafdata->state++;
                else
                    return PAF_ABORT;
                break;

            case DNP3_PAF_STATE__START_2:
                if (((uint8_t) *(data + bytes_processed)) == DNP3_START_BYTE_2)
                    pafdata->state++;
                else
                    return PAF_ABORT;
                break;

            /* Read the length. */
            case DNP3_PAF_STATE__LENGTH:
                pafdata->dnp3_length = (uint8_t) *(data + bytes_processed);

                /* DNP3 length only counts non-CRC octets following the
                   length field itself. Each CRC is two octets. One follows
                   the headers, a CRC is inserted for every 16 octets of user data,
                   plus a CRC for the last bit of user data (< 16 octets) */

                if (pafdata->dnp3_length < DNP3_HEADER_REMAINDER_LEN)
                {
                    /* XXX: Can we go about raising decoder alerts & dropping
                            packets within PAF? */
                    return PAF_ABORT;
                }

                user_data = pafdata->dnp3_length - DNP3_HEADER_REMAINDER_LEN;
                num_crcs = 1 + (user_data/DNP3_CHUNK_SIZE) + (user_data % DNP3_CHUNK_SIZE? 1 : 0);
                pafdata->real_length = pafdata->dnp3_length + (DNP3_CRC_SIZE*num_crcs);

                pafdata->state++;
                break;

            /* Set the flush point. */
            case DNP3_PAF_STATE__SET_FLUSH:
                *fp = pafdata->real_length + bytes_processed;
                pafdata->state = DNP3_PAF_STATE__START_1;
                return PAF_FLUSH;
        }

        bytes_processed++;
    }

    return PAF_SEARCH;
}

/* Take a DNP3 config + Snort policy, iterate through ports, register PAF callback. */
int DNP3AddPortsToPaf(struct _SnortConfig *sc, dnp3_config_t *config, tSfPolicyId policy_id)
{
    unsigned int i;

    for (i = 0; i < MAX_PORTS; i++)
    {
        if (config->ports[PORT_INDEX(i)] & CONV_PORT(i))
        {
            DNP3PafRegisterPort(sc, (uint16_t) i, policy_id);
        }
    }

    return DNP3_OK;
}
