/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2002-2013 Sourcefire, Inc.
 * Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
 *
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
 */

/* Snort Preprocessor for Telnet Negotiation Normalization*/
/* $Id$ */

/* pp_telnet.c
 *
 * Purpose:  Telnet sessions can contain telnet negotiation strings
 *           that can disrupt pattern matching.  This plugin detects
 *           negotiation strings in stream and "normalizes" them much like
 *           the http_decode preprocessor normalizes encoded URLs
 *
 *
 * official registry of options
 * http://www.iana.org/assignments/telnet-options
 *
 * Arguments:  None
 *
 * Effect:  The telnet nogiation data is removed from the payload
 *
 * Comments:
 *
 */

/* your preprocessor header file goes here */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <sys/types.h>

#include "ftpp_eo_log.h"
#include "pp_telnet.h"
#include "ftpp_return_codes.h"
#include "snort_debug.h"
#include "stream_api.h"

#define NUL 0x00
#define CR 0x0d
#define LF 0x0a

/* This is the allowable number of 8 bit characters,
 * ie, non-ASCII, before we declare this packet/stream
 * as encrypted.
 */
#define CONSECUTIVE_8BIT_THRESHOLD 3

/*
 * Function: normalize_telnet(Packet *)
 *
 * Purpose: Perform the preprocessor's intended function.  This can be
 *          simple (statistics collection) or complex (IP defragmentation)
 *          as you like.  Try not to destroy the performance of the whole
 *          system by trying to do too much....
 *
 * Arguments: p => pointer to the current packet data struct
 *
 * Returns: void function
 *
 */
int normalize_telnet(FTPTELNET_GLOBAL_CONF *GlobalConf,
                     TELNET_SESSION *tnssn, SFSnortPacket *p,
                     int iMode, char ignoreEraseCmds)
{
    int ret = FTPP_NORMALIZED;
    const unsigned char *read_ptr, *sb_start = NULL;
    int saw_ayt = 0;
    const unsigned char *start = _dpd.altBuffer->data;
    unsigned char *write_ptr;
    const unsigned char *end;
    int normalization_required = 0;
    int consec_8bit_chars = 0;

    /* Telnet commands are handled in here.
    * They can be 2 bytes long -- ie, IAC NOP, IAC AYT, etc.
    * Sub-negotiation strings are at least 4 bytes, IAC SB x IAC SE */
    if(p->payload_size < 2)
    {
        if (tnssn && iMode == FTPP_SI_CLIENT_MODE)
            tnssn->consec_ayt = 0;
        return FTPP_SUCCESS;
    }

    /* setup the pointers */
    read_ptr = p->payload;
    end = p->payload + p->payload_size;

    /* look to see if we have any telnet negotiaion codes in the payload */
    while(!normalization_required && (read_ptr < end))
    {
        /* look for the start of a negotiation string */
        if(*read_ptr == (unsigned char) TNC_IAC)
        {
            /* set a flag for stage 2 normalization */
            normalization_required = 1;
        }
        else
        {
            /* Okay, it wasn't an IAC also its a midstream pickup */
            if (*read_ptr > 0x7F && _dpd.sessionAPI->get_session_flags(p->stream_session) & SSNFLAG_MIDSTREAM)
            {
                consec_8bit_chars++;
                if (consec_8bit_chars > CONSECUTIVE_8BIT_THRESHOLD)
                {
                    /* This data stream had a series of 8 bit characters.
                     * It is very likely encrypted.  This handles the case
                     * where we either missed the option negotiation, or
                     * lost state of an already encrypted telnet session.
                     */
                    if (tnssn)
                    {
                        tnssn->encr_state = 1;
                        if (GlobalConf->encrypted.alert)
                        {
                            /* Alert on encrypted channel */
                            telnet_eo_event_log(tnssn, TELNET_EO_ENCRYPTED,
                                NULL, NULL);
                        }

                        if (!GlobalConf->check_encrypted_data)
                        {
                            /* Mark this session & packet as one to ignore */
                            _dpd.sessionAPI->stop_inspection(p->stream_session, p,
                                                        SSN_DIR_BOTH, -1, 0);
                            /* No point to do further normalization */
                            return FTPP_ALERT;
                        }
                    }
                    break;
                }
            }
            else
            {
                consec_8bit_chars = 0;
            }
        }

        read_ptr++;
    }

    if(!normalization_required)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_FTPTELNET, "Nothing to process!\n"););
        if (tnssn && iMode == FTPP_SI_CLIENT_MODE)
            tnssn->consec_ayt = 0;
        return FTPP_SUCCESS;
    }

    /*
    * if we found telnet negotiation strings OR backspace characters,
    * we're going to have to normalize the data
    *
    * Note that this is always ( now: 2002-08-12 ) done to a
    * alternative data buffer.
    */
    /* rewind the data stream to p->data */
    read_ptr = p->payload;

    /* setup for overwriting the negotaiation strings with
    * the follow-on data
    */
    write_ptr = (unsigned char *) _dpd.altBuffer;

    /* walk thru the remainder of the packet */
    while((read_ptr < end) &&
            (write_ptr < ((unsigned char *) _dpd.altBuffer->data) + sizeof(_dpd.altBuffer->data)))
    {
        saw_ayt = 0;
        /* if the following byte isn't a subnegotiation initialization */
        if(((read_ptr + 1) < end) &&
            (*read_ptr == (unsigned char) TNC_IAC) &&
            (*(read_ptr + 1) != (unsigned char) TNC_SB))
        {
            /* NOPs are two bytes long */
            switch(* ((unsigned char *)(read_ptr + 1)))
            {
            case TNC_NOP:
                read_ptr += 2;
                break;
            case TNC_EAC:
                read_ptr += 2;
                /* wind it back a character? */
                if (ignoreEraseCmds == FTPP_APPLY_TNC_ERASE_CMDS)
                {
                    if(write_ptr  > start)
                    {
                        write_ptr--;
                    }
                }
                break;
            case TNC_EAL:
                read_ptr += 2;
                /* wind it back a line? */
                if (ignoreEraseCmds == FTPP_APPLY_TNC_ERASE_CMDS)
                {
                    /* Go back to previous CR NULL or CR LF? */
                    while (write_ptr > start)
                    {
                        /* Go to previous char */
                        write_ptr--;

                        if ((*write_ptr == CR) &&
                           ((*(write_ptr+1) == NUL) || (*(write_ptr+1) == LF)) )
                        {
                            /* Okay, found the CR NUL or CR LF, move it
                             * forward past those two -- that is the
                             * beginning of this line
                             */
                            write_ptr+=2;
                            break;
                        }
                    }
                }
                break;
                /* These are two bytes long */
            case TNC_AYT:
                saw_ayt = 1;
                if (tnssn)
                {
                    tnssn->consec_ayt++;
                    if ((tnssn->telnet_conf->ayt_threshold > 0) &&
                        (tnssn->consec_ayt >
                            tnssn->telnet_conf->ayt_threshold))
                    {
                        /* Alert on consecutive AYT commands */
                        telnet_eo_event_log(tnssn, TELNET_EO_AYT_OVERFLOW,
                            NULL, NULL);
                        tnssn->consec_ayt = 0;
                        return FTPP_ALERT;
                    }
                }
                /* Fall through */
            case TNC_BRK:
            case TNC_DM:
            case TNC_IP:
            case TNC_AO:
            case TNC_GA:
#ifdef RFC1184
            case TNC_EOF:
            case TNC_SUSP:
            case TNC_ABOR:
#endif
#ifdef RFC885
            case TNC_EOR:
#endif
                read_ptr += 2;
                break;
            case TNC_SE:
                /* Uh, what the heck is a Subnegotiation-end
                 * doing here without SB?.  could generate an alert.
                 * Will just normalize it out since we may have
                 * processed the SB in a previous packet.
                 */
                read_ptr += 2;
                break;
            case TNC_IAC:
                /* IAC IAC -- means the IAC character (0xff) should be
                * in the data stream since it was escaped */
                read_ptr++; /* skip past the first IAC */
                *write_ptr++ = *read_ptr++;
                break;
            case TNC_WILL:
            case TNC_WONT:
            case TNC_DO:
            case TNC_DONT:
                read_ptr += 3;
                break;
            default:
                /* move the read ptr up 2 bytes */
                read_ptr += 2;
            }
            /* If not an AYT, reset it */
            if (!saw_ayt)
            {
                if (tnssn && iMode == FTPP_SI_CLIENT_MODE)
                    tnssn->consec_ayt = 0;
            }
        }
        /* check for subnegotiation */
        else if(((read_ptr + 1) < end) &&
            (*read_ptr == (unsigned char) TNC_IAC) &&
            (*(read_ptr+1) == (unsigned char) TNC_SB))
        {
            sb_start = read_ptr;

            switch (*(read_ptr+2))
            {
            case 0x26: /* Encryption -- RFC 2946 */
                /* printf("Telnet: Saw SB for Encryption\n"); */
                read_ptr += 3;
                switch (*read_ptr)
                {
#ifdef TRACK_ENCRYPTION_NEGOTIATION
                case 0x00:
                    /* Client sending the Encryption IS marker
                     * followed by address. */
                    {
                        read_ptr++;
                        if (*read_ptr != 0x00)
                            /* Encryption type is not NULL */
                        {
                            /* printf("Encryption being negotiated by
                             * telnet client\n"); */
                        }
                    }
                    break;
#endif
                case 0x03:
                    /* Client sending the Encryption START marker
                     * followed by address. */
                    {
                        read_ptr++;
                        /* printf("Encryption started by telnet client\n"); */
                        if (tnssn)
                        {
                            tnssn->encr_state = 1;
                            if (GlobalConf->encrypted.alert)
                            {
                                /* Alert on encrypted channel */
                                telnet_eo_event_log(tnssn, TELNET_EO_ENCRYPTED,
                                    NULL, NULL);
                            }

                            if (!GlobalConf->check_encrypted_data)
                            {
                                /* Mark this session & packet as one to ignore */
                                _dpd.sessionAPI->stop_inspection(p->stream_session, p,
                                                        SSN_DIR_BOTH, -1, 0);
                                /* No point to do further normalization */
                                return FTPP_ALERT;
                            }
                        }
                    }
                    break;
                }
                break;
            }

            /* find the end of the subneg -- this handles when there are
             * embedded IAC IACs within a sub negotiation.  Just looking
             * for the TNC_SE could cause problems.  Similarly, just looking
             * for the TNC_IAC could end it too early. */
            while(read_ptr < end)
            {
                if ((*read_ptr == (unsigned char) TNC_IAC) &&
                    (*(read_ptr+1) == (unsigned char) TNC_SE))
                {
                    sb_start = NULL;
                    break;
                }
                read_ptr++;
            }

            if (sb_start)
            {
                /* Didn't find the IAC SE.  Normalize out the IAC SB
                 * and restart from there. Presumption is this is
                 * just someone trying to fool us, since we usually
                 * see the entire IAC SB ... IAC SE in one packet. */
                read_ptr = sb_start+2;
                if (!tnssn)
                {
                    /* Its an FTP session */
                    ret = FTPP_ALERT;
                }
                else if (GlobalConf->telnet_config->detect_anomalies)
                {
                    /* Alert on SB without SE */
                    telnet_eo_event_log(tnssn, TELNET_EO_SB_NO_SE,
                        NULL, NULL);
                    ret = FTPP_ALERT;
                }

                continue;
            }

            /* Okay, found the IAC SE -- move past it */
            if (read_ptr < end)
            {
                read_ptr += 2;
            }

            if (tnssn && iMode == FTPP_SI_CLIENT_MODE)
                tnssn->consec_ayt = 0;
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FTPTELNET,
                "overwriting %2X(%c) with %2X(%c)\n",
                (unsigned char)(*write_ptr&0xFF), *write_ptr,
                (unsigned char)(*read_ptr & 0xFF), *read_ptr););

            /* overwrite the negotiation bytes with the follow-on bytes */
            switch(* ((unsigned char *)(read_ptr)))
            {
            case 0x7F: /* Delete */
            case 0x08: /* Backspace/Ctrl-H */
                /* wind it back a character */
                if (write_ptr > start)
                {
                    write_ptr--;
                }
                read_ptr++;
                break;
            default:
                *write_ptr++ = *read_ptr++;
                break;
            }

            if (tnssn && iMode == FTPP_SI_CLIENT_MODE)
                tnssn->consec_ayt = 0;
        }
    }

    _dpd.SetAltDecode((uint16_t)(write_ptr - start));

    /* DEBUG_WRAP(DebugMessage(DEBUG_FTPTELNET,
    "Converted buffer after telnet normalization:\n");
    PrintNetData(stdout, (char *) _dpd.altBuffer->data, _dpd.altBuffer->len););
    */
    return ret;
}
