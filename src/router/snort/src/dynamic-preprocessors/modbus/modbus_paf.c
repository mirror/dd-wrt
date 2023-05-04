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
 * Protocol-Aware Flushing (PAF) code for the Modbus preprocessor.
 *
 */

#include "spp_modbus.h"
#include "modbus_decode.h"
#include "modbus_paf.h"
#include "sf_dynamic_preprocessor.h"

/* Defines */
#define MODBUS_MIN_HDR_LEN 2        /* Enough for Unit ID + Function */
#define MODBUS_MAX_HDR_LEN 254      /* Max PDU size is 260, 6 bytes already seen */


int ModbusPafRegisterPort (struct _SnortConfig *sc, uint16_t port, tSfPolicyId policy_id)
{
    if (!_dpd.isPafEnabled())
        return 0;

    _dpd.streamAPI->register_paf_port(sc, policy_id, port, 0, ModbusPaf, true);
    _dpd.streamAPI->register_paf_port(sc, policy_id, port, 1, ModbusPaf, true);

    return 0;
}

#ifdef TARGET_BASED
int ModbusAddServiceToPaf (struct _SnortConfig *sc, uint16_t service, tSfPolicyId policy_id)
{
    if (!_dpd.isPafEnabled())
        return 0;

    _dpd.streamAPI->register_paf_service(sc, policy_id, service, 0, ModbusPaf, true);
    _dpd.streamAPI->register_paf_service(sc, policy_id, service, 1, ModbusPaf, true);

    return 0;
}
#endif

/* Function: ModbusPaf()

   Purpose: Modbus/TCP PAF callback.
            Statefully inspects Modbus traffic from the start of a session,
            Reads up until the length octet is found, then sets a flush point.

   Arguments:
     void * - stream5 session pointer
     void ** - Modbus state tracking structure
     const uint8_t * - payload data to inspect
     uint32_t - length of payload data
     uint32_t - flags to check whether client or server
     uint32_t * - pointer to set flush point
     uint32_t * - pointer to set header flush point

   Returns:
    PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
*/

PAF_Status ModbusPaf(void *ssn, void **user, const uint8_t *data,
                     uint32_t len, uint64_t *flags, uint32_t *fp, uint32_t *fp_eoh)
{
    modbus_paf_data_t *pafdata = *(modbus_paf_data_t **)user;
    uint32_t bytes_processed = 0;

    /* Allocate state object if it doesn't exist yet. */
    if (pafdata == NULL)
    {
        pafdata = calloc(1, sizeof(modbus_paf_data_t));
        if (pafdata == NULL)
            return PAF_ABORT;

        *user = pafdata;
    }

    /* Process this packet 1 byte at a time */
    while (bytes_processed < len)
    {
        switch (pafdata->state)
        {
            /* Skip the Transaction & Protocol IDs */
            case MODBUS_PAF_STATE__TRANS_ID_1:
            case MODBUS_PAF_STATE__TRANS_ID_2:
            case MODBUS_PAF_STATE__PROTO_ID_1:
            case MODBUS_PAF_STATE__PROTO_ID_2:
                pafdata->state++;
                break;

            /* Read length 1 byte at a time, in case a TCP segment is sent
             * with only 5 bytes from the MBAP header */
            case MODBUS_PAF_STATE__LENGTH_1:
                pafdata->modbus_length |= ( *(data + bytes_processed) << 8 );
                pafdata->state++;
                break;

            case MODBUS_PAF_STATE__LENGTH_2:
                pafdata->modbus_length |= *(data + bytes_processed);
                pafdata->state++;
                break;

            case MODBUS_PAF_STATE__SET_FLUSH:
                if ((pafdata->modbus_length < MODBUS_MIN_HDR_LEN) ||
                    (pafdata->modbus_length > MODBUS_MAX_HDR_LEN))
                {
                    _dpd.alertAdd(GENERATOR_SPP_MODBUS, MODBUS_BAD_LENGTH, 1, 0, 3,
                                  MODBUS_BAD_LENGTH_STR, 0);
                }

                *fp = pafdata->modbus_length + bytes_processed;
                pafdata->state = MODBUS_PAF_STATE__TRANS_ID_1;
                pafdata->modbus_length = 0;
                return PAF_FLUSH;
        }

        bytes_processed++;
    }

    return PAF_SEARCH;
}

/* Take a Modbus config + Snort policy, iterate through ports, register PAF callback */
void ModbusAddPortsToPaf(struct _SnortConfig *sc, modbus_config_t *config, tSfPolicyId policy_id)
{
    unsigned int i;

    for (i = 0; i < MAX_PORTS; i++)
    {
        if (config->ports[PORT_INDEX(i)] & CONV_PORT(i))
        {
            ModbusPafRegisterPort(sc, (uint16_t) i, policy_id);
        }
    }
}
