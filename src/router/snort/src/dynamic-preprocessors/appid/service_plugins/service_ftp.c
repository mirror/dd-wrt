/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "flow.h"
#include "service_api.h"
#include "service_util.h"

#define FTP_PORT    21
/*#define RNA_FTP_EXPECTED_ON_PORT    1 */

typedef enum
{
    FTP_STATE_CONNECTION,
    FTP_STATE_LOGIN,
    FTP_STATE_PASSWORD,
    FTP_STATE_ACCOUNT,
    FTP_STATE_CONNECTION_ERROR,
    FTP_STATE_MONITOR
} FTPState;

typedef enum
{
    FTP_REPLY_BEGIN,
    FTP_REPLY_MULTI,
    FTP_REPLY_MID
} FTPReplyState;

typedef enum
{
    FTP_CMD_NONE,
    FTP_CMD_PORT
} FTPCmd;

#define MAX_STRING_SIZE 64
typedef struct _SERVICE_FTP_DATA
{
    FTPState state;
    FTPReplyState rstate;
    int code;
    char vendor[MAX_STRING_SIZE];
    char version[MAX_STRING_SIZE];
    FTPCmd cmd;
    uint32_t address;
    uint16_t port;
} ServiceFTPData;

#pragma pack(1)

typedef struct _SERVICE_FTP_CODE
{
    uint8_t code[3];
    uint8_t sp;
} ServiceFTPCode;

#pragma pack()

static int ftp_init(const InitServiceAPI * const init_api);
MakeRNAServiceValidationPrototype(ftp_validate);

static RNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &ftp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "ftp",
    .ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&ftp_validate, FTP_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

RNAServiceValidationModule ftp_service_mod =
{
    .name = "FTP",
    .init = &ftp_init,
    .pp = pp,
};

#define FTP_PATTERN1 "220 "
#define FTP_PATTERN2 "220-"
#define FTP_PATTERN3 "FTP"
#define FTP_PATTERN4 "ftp"

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_FTP_CONTROL, APPINFO_FLAG_SERVICE_ADDITIONAL}
};

static int16_t ftp_data_app_id = 0;

static int ftp_init(const InitServiceAPI * const init_api)
{
    ftp_data_app_id = init_api->dpd->addProtocolReference("ftp-data");

    init_api->RegisterPattern(&ftp_validate, IPPROTO_TCP, (uint8_t *)FTP_PATTERN1, sizeof(FTP_PATTERN1)-1, 0, "ftp");
    init_api->RegisterPattern(&ftp_validate, IPPROTO_TCP, (uint8_t *)FTP_PATTERN2, sizeof(FTP_PATTERN2)-1, 0, "ftp");
    init_api->RegisterPattern(&ftp_validate, IPPROTO_TCP, (uint8_t *)FTP_PATTERN3, sizeof(FTP_PATTERN3)-1, -1, "ftp");
    init_api->RegisterPattern(&ftp_validate, IPPROTO_TCP, (uint8_t *)FTP_PATTERN4, sizeof(FTP_PATTERN4)-1, -1, "ftp");
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&ftp_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, NULL);
	}

    return 0;
}

static int ftp_validate_reply(const uint8_t *data, uint16_t *offset,
                              uint16_t size, ServiceFTPData *fd)
{
    const ServiceFTPCode *code_hdr;
    int tmp;
    FTPReplyState tmp_state;

    for (; *offset < size; (*offset)++)
    {
        /* Trim any blank lines (be a little tolerant) */
        for (; *offset<size; (*offset)++)
        {
            if (data[*offset] != 0x0D && data[*offset] != 0x0A) break;
        }

        switch (fd->rstate)
        {
        case FTP_REPLY_BEGIN:
            if (size - (*offset) < (int)sizeof(ServiceFTPCode)) return -1;

            code_hdr = (ServiceFTPCode *)(data + *offset);

            if (code_hdr->sp == '-') fd->rstate = FTP_REPLY_MULTI;
            else if (code_hdr->sp != ' ' && code_hdr->sp != 0x09) return -1;

            if (code_hdr->code[0] < '1' || code_hdr->code[0] > '5') return -1;
            fd->code = (code_hdr->code[0] - '0') * 100;

            if (code_hdr->code[1] < '0' || code_hdr->code[1] > '5') return -1;
            fd->code += (code_hdr->code[1] - '0') * 10;

            if (!isdigit(code_hdr->code[2])) return -1;
            fd->code += code_hdr->code[2] - '0';

            *offset += sizeof(ServiceFTPCode);
            tmp_state = fd->rstate;
            fd->rstate = FTP_REPLY_MID;
            for (; *offset < size; (*offset)++)
            {
                if (data[*offset] == 0x0D)
                {
                    (*offset)++;
                    if (*offset >= size) return -1;
                    if (data[*offset] == 0x0D)
                    {
                        (*offset)++;
                        if (*offset >= size) return -1;
                    }
                    if (data[*offset] != 0x0A) return -1;
                    fd->rstate = tmp_state;
                    break;
                }
                if (data[*offset] == 0x0A)
                {
                    fd->rstate = tmp_state;
                    break;
                }
                else if (!isprint(data[*offset]) && data[*offset] != 0x09) return -1;
            }
            if (fd->rstate == FTP_REPLY_MID) return -1;
            break;
        case FTP_REPLY_MULTI:
            if (size - *offset < (int)sizeof(ServiceFTPCode))
            {
                fd->rstate = FTP_REPLY_MID;
                for (; *offset < size; (*offset)++)
                {
                    if (data[*offset] == 0x0D)
                    {
                        (*offset)++;
                        if (*offset >= size) return -1;
                        if (data[*offset] == 0x0D)
                        {
                            (*offset)++;
                            if (*offset >= size) return -1;
                        }
                        if (data[*offset] != 0x0A) return -1;
                        fd->rstate = FTP_REPLY_MULTI;
                        break;
                    }
                    if (data[*offset] == 0x0A)
                    {
                        fd->rstate = FTP_REPLY_MULTI;
                        break;
                    }
                    if (!isprint(data[*offset]) && data[*offset] != 0x09) return -1;
                }
                if (fd->rstate == FTP_REPLY_MID) return -1;
            }
            else
            {
                code_hdr = (ServiceFTPCode *)(data + *offset);
                if (size - (*offset) >= (int)sizeof(ServiceFTPCode) &&
                    (code_hdr->sp == ' ' || code_hdr->sp == 0x09) &&
                    code_hdr->code[0] >= '1' && code_hdr->code[0] <= '5' &&
                    code_hdr->code[1] >= '1' && code_hdr->code[1] <= '5' &&
                    isdigit(code_hdr->code[2]))
                {
                    tmp = (code_hdr->code[0] - '0') * 100;
                    tmp += (code_hdr->code[1] - '0') * 10;
                    tmp += code_hdr->code[2] - '0';
                    if (tmp == fd->code)
                    {
                        *offset += sizeof(ServiceFTPCode);
                        fd->rstate = FTP_REPLY_BEGIN;
                    }
                }
                tmp_state = fd->rstate;
                fd->rstate = FTP_REPLY_MID;
                for (; *offset < size; (*offset)++)
                {
                    if (data[*offset] == 0x0D)
                    {
                        (*offset)++;
                        if (*offset >= size) return -1;
                        if (data[*offset] == 0x0D)
                        {
                            (*offset)++;
                            if (*offset >= size) return -1;
                        }
                        if (data[*offset] != 0x0A) return -1;
                        fd->rstate = tmp_state;
                        break;
                    }
                    if (data[*offset] == 0x0A)
                    {
                        fd->rstate = tmp_state;
                        break;
                    }
                    if (!isprint(data[*offset]) && data[*offset] != 0x09) return -1;
                }
                if (fd->rstate == FTP_REPLY_MID) return -1;
            }
            break;
        default:
            return -1;
        }
        if (fd->rstate == FTP_REPLY_BEGIN)
        {
            for (; *offset < size; (*offset)++)
            {
                if (data[*offset] == 0x0D)
                {
                    (*offset)++;
                    if (*offset >= size) return -1;
                    if (data[*offset] != 0x0A) return -1;
                }
                else if (!isspace(data[*offset])) break;
            }
            return fd->code;
        }
    }
    return 0;
}

static int ftp_decode_number(const uint8_t * *data, const uint8_t *end, uint8_t delimiter, uint32_t *number)
{
    *number = 0;
    for (; *data < end && **data == ' '; (*data)++);
    if (*data < end && **data == delimiter) return -1;
    while (*data < end && **data != delimiter)
    {
        if (!isdigit(**data)) return -1;
        *number *= 10;
        *number += **data - '0';
        (*data)++;
    }
    if (*data >= end || **data != delimiter || *number > 255)
    {
        *number = 0;
        return -1;
    }
    (*data)++;
    return 0;
}

static int ftp_validate_pasv(const uint8_t *data, uint16_t size,
                             uint32_t *address, uint16_t *port)
{
    const uint8_t *end;
    uint32_t tmp;

    *address = 0;
    *port = 0;

    end = data + size;
    data += sizeof(ServiceFTPCode);

    for (; data<end && *data!='('; data++);
    data++;
    if (data >= end) return 1;

    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *address = tmp << 24;
    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *address += tmp << 16;
    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *address += tmp << 8;
    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *address += tmp;
    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *port = (uint16_t)(tmp << 8);
    if (ftp_decode_number(&data, end, ')', &tmp)) return -1;
    *port += tmp;
    return 0;
}

static int ftp_validate_epsv(const uint8_t *data, uint16_t size,
                             uint16_t *port)
{
    const uint8_t *end;

    *port = 0;

    end = data + size;
    data += sizeof(ServiceFTPCode);

    for (; data<end && *data!='('; data++);
	data++;
	if (data >= end) return 1;

	for (; data<end && *data!='|'; data++);
	data++;
	if (data >= end) return 1;

	for (; data<end && *data!='|'; data++);
	data++;
	if (data >= end) return 1;

	for (; data<end && *data!='|'; data++);
    data++;
    if (data >= end) return 1;

	while (data < end && *data != '|')
    {
        if (!isdigit(*data)) return -1;
        *port *= 10;
        *port += *data - '0';
        data++;
    }

    return 0;
}

static int ftp_validate_port(const uint8_t *data, uint16_t size,
                             uint32_t *address, uint16_t *port)
{
    const uint8_t *end;
    const uint8_t *p;
    uint32_t tmp;

    *address = 0;
    *port = 0;

    end = data + size;

    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *address = tmp << 24;
    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *address += tmp << 16;
    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *address += tmp << 8;
    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *address += tmp;
    if (ftp_decode_number(&data, end, ',', &tmp)) return -1;
    *port = (uint16_t)(tmp << 8);
    p = end - 1;
    if (p > data)
    {
        if (*p == 0x0a)
        {
            p--;
            if (*p == 0x0d)
            {
                if (ftp_decode_number(&data, end, 0x0d, &tmp)) return -1;
                *port += tmp;
                return 0;
            }
        }
    }
    if (ftp_decode_number(&data, end, 0x0a, &tmp)) return -1;
    *port += tmp;
    return 0;
}

static void CheckVendorVersion(const uint8_t *data, uint16_t init_offset,
                               uint16_t offset, ServiceFTPData *fd)
{
    static const unsigned char ven_hp[] = "Hewlett-Packard FTP Print Server";
    static const unsigned char ver_hp[] = "Version ";
    const unsigned char *p;
    const unsigned char *end;
    const unsigned char *ver;
    char *v;
    char *v_end;

    p = &data[init_offset];
    end = &data[offset-1];
    /* Search for the HP vendor string */
    if ((p=service_strstr(p, end-p, ven_hp, sizeof(ven_hp)-1)))
    {
        /* Found HP vendor string */
        strcpy(fd->vendor, (char *)ven_hp);
        /* Move just past the vendor string */
        p += sizeof(ven_hp) - 1;
        /* Search for the version string */
        if ((p = service_strstr(p, end-p, ver_hp, sizeof(ver_hp)-1)))
        {
            /* Found the version string.  Move just past the version string */
            ver = p + (sizeof(ver_hp) - 1);
            p = ver;
            v = fd->version;
            v_end = v + (MAX_STRING_SIZE - 1);
            while (p < end && *p && (isalnum(*p) || *p == '.'))
            {
                if (v < v_end)
                {
                    *v = *p;
                    v++;
                }
                p++;
            }
            *v = 0;
            /* Don't let the version end in . */
            if (v != fd->version && *(v-1) == '.')
            {
                v--;
                *v = 0;
            }
        }
    }
}

MakeRNAServiceValidationPrototype(ftp_validate)
{
    static const char FTP_PORT_CMD[] = "PORT ";
    static const unsigned char ven_ms[] = "Microsoft FTP Service";
    static const unsigned char ver_ms[] = "(Version ";
    static const unsigned char ven_wu[] = "(Version wu-";
    static const unsigned char ven_proftpd[] = "ProFTPD";
    static const unsigned char ven_pureftpd[] = "Pure-FTPd";
    static const unsigned char ven_ncftpd[] = "NcFTPd";
    ServiceFTPData *fd;
    uint16_t offset;
    uint16_t init_offset;
    const unsigned char *p;
    const unsigned char *ven;
    const unsigned char *ver;
    int code;
    int code_index;
    uint32_t address;
    uint16_t port;
    FLOW *fp;
    int retval = SERVICE_INPROCESS;
    char *v;
    char *v_end;
    const unsigned char *begin;
    const unsigned char *end;

    if (!size)
        goto inprocess;

    fd = ftp_service_mod.api->data_get(flowp);
    if (!fd)
    {
        fd = calloc(1, sizeof(*fd));
        if (!fd)
            return SERVICE_ENOMEM;
        if (ftp_service_mod.api->data_add(flowp, fd, &free))
        {
            free(fd);
            return SERVICE_ENOMEM;
        }
        fd->state = FTP_STATE_CONNECTION;
        fd->rstate = FTP_REPLY_BEGIN;
        fd->cmd = FTP_CMD_NONE;
    }

    if (dir != APP_ID_FROM_RESPONDER)
    {
        if (size > sizeof(FTP_PORT_CMD)-1 &&
            strncasecmp((char *)data, FTP_PORT_CMD, sizeof(FTP_PORT_CMD)-1) == 0)
        {
            if (data[size-1] != 0x0a) return SERVICE_INPROCESS;
            if (ftp_validate_port(data+(sizeof(FTP_PORT_CMD)-1),
                                  size-(sizeof(FTP_PORT_CMD)-1),
                                  &fd->address, &fd->port) == 0)
            {
                if (fd->state != FTP_STATE_MONITOR)
                {
                    flow_mark(flowp, FLOW_SERVICEDETECTED | FLOW_CONTINUE);
                    fd->state = FTP_STATE_MONITOR;
                }
#ifdef RNA_FTP_EXPECTED_ON_PORT
                snort_ip ip;

                ip.family = AF_INET;
                ip.bits = 32;
                ip.ip32[0] = htonl(fd->address);
                ip.ip32[1] = 0;
                ip.ip32[2] = 0;
                ip.ip32[3] = 0;
                fp = ftp_service_mod.api->flow_new(pkt, GET_DST_IP(pkt), 0,
                                                   &ip, fd->port, flowp->proto, ftp_data_app_id);
                if (fp)
                {
                    fp->serviceAppId = APP_ID_FTP_DATA;
                    flow_mark(fp, FLOW_SERVICEDETECTED | FLOW_NOT_A_SERVICE | FLOW_PORT_SERVICE_DONE);
                    fp->rnaServiceState = RNA_STATE_FINISHED;
                    fp->rnaClientState = RNA_STATE_FINISHED;
                }
                fd->cmd = FTP_CMD_NONE;
#else
                fd->cmd = FTP_CMD_PORT;
#endif
            }
        }
        goto inprocess;
    }

    v_end = fd->version;
    v_end += MAX_STRING_SIZE - 1;

    offset = 0;
    while (offset < size)
    {
        init_offset = offset;
        if ((code=ftp_validate_reply(data, &offset, size, fd)) < 0) goto fail;
        if (!code) goto inprocess;

        switch (fd->state)
        {
        case FTP_STATE_CONNECTION:
            switch (code)
            {
            case 120: /*system will be ready in nn minutes */
                break;
            case 220: /*service ready for new user */
                fd->state = FTP_STATE_LOGIN;
                begin = &data[init_offset];
                end = &data[offset-1];
                if (service_strstr(begin, end-begin, ven_ms, sizeof(ven_ms)-1))
                {
                    strcpy(fd->vendor, (char *)ven_ms);
                    if ((p = service_strstr(begin, end-begin, ver_ms, sizeof(ver_ms)-1)))
                    {
                        ver = p + (sizeof(ver_ms) - 1);
                        v = fd->version;
                        for (p=ver; p<end && *p && *p != ')'; p++)
                        {
                            if (v < v_end)
                            {
                                *v = *p;
                                v++;
                            }
                        }
                        *v = 0;
                        if (p >= end || !(*p))
                        {
                            /* did not find a closing ), no version */
                            fd->version[0] = 0;
                        }
                    }
                }
                else if ((p=service_strstr(begin, end-begin, ven_wu, sizeof(ven_wu)-1)))
                {
                    strcpy(fd->vendor, "wu");
                    ver = p + (sizeof(ven_wu) - 1);
                    v = fd->version;
                    for (p=ver; p<end && *p && *p != ' '; p++)
                    {
                        if (v < v_end)
                        {
                            *v = *p;
                            v++;
                        }
                    }
                    *v = 0;
                    if (p >= end || !(*p))
                    {
                        /* did not find a space, no version */
                        fd->version[0] = 0;
                    }
                }
                else if ((p=service_strstr(begin, end-begin, ven_proftpd, sizeof(ven_proftpd)-1)))
                {
                    strcpy(fd->vendor, (char *)ven_proftpd);
                    ver = p + (sizeof(ven_proftpd) - 1);
                    if (*ver == ' ')
                    {
                        ver++;
                        v = fd->version;
                        for (p=ver; p<end && *p && *p != ' '; p++)
                        {
                            if (v < v_end)
                            {
                                *v = *p;
                                v++;
                            }
                        }
                        *v = 0;
                        if (p >= end || !(*p))
                        {
                            /* did not find a space, no version */
                            fd->version[0] = 0;
                        }
                    }
                }
                else if (service_strstr(begin, end-begin, ven_pureftpd, sizeof(ven_pureftpd)-1))
                {
                    strcpy(fd->vendor, (char *)ven_pureftpd);
                }
                else if (service_strstr(begin, end-begin, ven_ncftpd, sizeof(ven_ncftpd)-1))
                {
                    strcpy(fd->vendor, (char *)ven_ncftpd);
                }
                else
                {
                    /* Look for (Vendor Version:  or  (Vendor Version) */
                    for (p=begin; p<end && *p && *p!='('; p++);
                    if (p < end)
                    {
                        p++;
                        ven = p;
                        for (; p<end && *p && *p!=' '; p++);
                        if (p < end && *p)
                        {
                            const unsigned char *ven_end;
                            const char *vendor_end;

                            ven_end = p;
                            ver = p + 1;
                            v = fd->vendor;
                            vendor_end = v + (MAX_STRING_SIZE - 1);
                            for (p=ven; p<ven_end; p++)
                            {
                                if (!isprint(*p)) break;
                                if (v < vendor_end)
                                {
                                    *v = *p;
                                    v++;
                                }
                            }
                            if (p >= ven_end)
                            {
                                *v = 0;
                                for (p=ver; p<end && *p && *p!=':'; p++);
                                if (p>=end || !(*p))
                                {
                                    for (p=ver; p<end && *p && *p!=')'; p++);
                                }
                                if (p < end && *p)
                                {
                                    const unsigned char *ver_end;
                                    ver_end = p;
                                    v = fd->version;
                                    for (p=ver; p<ver_end; p++)
                                    {
                                        if (!isprint(*p)) break;
                                        if (v < v_end)
                                        {
                                            *v = *p;
                                            v++;
                                        }
                                    }
                                    if (p >= ver_end)
                                    {
                                        *v = 0;
                                    }
                                    else
                                    {
                                        /* Non-printable characters.  No vendor or version */
                                        fd->vendor[0] = 0;
                                        fd->version[0] = 0;
                                    }
                                }
                                else
                                {
                                    /* No : or ).  No vendor */
                                    fd->vendor[0] = 0;
                                }
                            }
                            else
                            {
                                /* Non-printable characters.  No vendor */
                                fd->vendor[0] = 0;
                            }
                        }
                    }
                }
                break;
            case 110: /* restart mark reply */
            case 125: /* connection is open start transferring file */
            case 150: /* Opening command */
            case 200: /*command ok */
            case 202: /*command not implemented */
            case 211: /* system status */
            case 212: /* directory status */
            case 213: /* file status */
            case 214: /* help message */
            case 215: /* name system type */
            case 225: /* data connection open */
            case 226: /* Transfer complete */
            case 227: /*entering passive mode */
            case 230: /*user loggined */
            case 250: /* CWD command successful */
            case 257: /* PATHNAME created */
            case 331: /* login ok need password */
            case 332: /*new account for login */
            case 350: /*requested file action pending futher information */
            case 450: /*requested file action not taken */
            case 451: /*requested file action aborted */
            case 452: /*requested file action not taken not enough space */
            case 500: /*syntax error */
            case 501: /*not recognozed */
            case 502: /*not recognozed */
            case 503: /*bad sequence of commands */
            case 504: /*command not implemented */
            case 530: /*login incorrect */
            case 532: /*new account for storing file */
            case 550: /*requested action not taken */
            case 551: /*requested action aborted :page type unknown */
            case 552: /*requested action aborted */
            case 553: /*requested action not taken file name is not allowed */
                flow_mark(flowp, FLOW_SERVICEDETECTED | FLOW_CONTINUE);
                fd->state = FTP_STATE_MONITOR;
                break;
            case 221: /*good bye */
            case 421: /*service not available closing connection */
                fd->state = FTP_STATE_CONNECTION_ERROR;
                break;
            default:
                goto fail;
            }
            break;
        case FTP_STATE_LOGIN:
            code_index = code / 100;
            switch (code_index)
            {
            case 2:
                switch (code)
                {
                case 221:
                    fd->state = FTP_STATE_CONNECTION_ERROR;
                    break;
                case 230:
                    if (!fd->vendor[0] && !fd->version[0])
                        CheckVendorVersion(data, init_offset, offset, fd);
                    flow_mark(flowp, FLOW_CONTINUE);
                    fd->state = FTP_STATE_MONITOR;
                    retval = SERVICE_SUCCESS;
                default:
                    break;
                }
                break;
            case 3:
                switch (code)
                {
                case 331:
                    fd->state = FTP_STATE_PASSWORD;
                    break;
                case 332:
                    fd->state = FTP_STATE_ACCOUNT;
                    break;
                default:
                    break;
                }
                break;
            case 4:
                switch (code)
                {
                case 421:
                    fd->state = FTP_STATE_CONNECTION_ERROR;
                    break;
                case 431:
                    break;
                default:
                    goto fail;
                }
                break;
            case 5:
                break;
            default:
                goto fail;
            }
            break;
        case FTP_STATE_PASSWORD:
            code_index = code / 100;
            switch (code_index)
            {
            case 2:
                switch (code)
                {
                case 221:
                    fd->state = FTP_STATE_CONNECTION_ERROR;
                    break;
                case 202:
                case 230:
                    if (!fd->vendor[0] && !fd->version[0])
                        CheckVendorVersion(data, init_offset, offset, fd);
                    flow_mark(flowp, FLOW_CONTINUE);
                    fd->state = FTP_STATE_MONITOR;
                    retval = SERVICE_SUCCESS;
                default:
                    break;
                }
                break;
            case 3:
                switch (code)
                {
                case 332:
                    fd->state = FTP_STATE_ACCOUNT;
                    break;
                default:
                    break;
                }
                break;
            case 4:
                switch (code)
                {
                case 421:
                    fd->state = FTP_STATE_CONNECTION_ERROR;
                    break;
                default:
                    goto fail;
                }
                break;
            case 5:
                switch (code)
                {
                case 500:
                case 501:
                case 503:
                case 530:
                    fd->state = FTP_STATE_LOGIN;
                    break;
                default:
                    goto fail;
                }
                break;
            default:
                goto fail;
            }
            break;
        case FTP_STATE_ACCOUNT:
            code_index = code / 100;
            switch (code_index)
            {
            case 2:
                switch (code)
                {
                case 202:
                case 230:
                    if (!fd->vendor[0] && !fd->version[0])
                        CheckVendorVersion(data, init_offset, offset, fd);
                    flow_mark(flowp, FLOW_CONTINUE);
                    fd->state = FTP_STATE_MONITOR;
                    retval = SERVICE_SUCCESS;
                default:
                    break;
                }
                break;
            case 3:
                switch (code)
                {
                case 332:
                    fd->state = FTP_STATE_ACCOUNT;
                    break;
                default:
                    break;
                }
                break;
            case 4:
                switch (code)
                {
                case 421:
                    fd->state = FTP_STATE_CONNECTION_ERROR;
                    break;
                default:
                    goto fail;
                }
                break;
            case 5:
                switch (code)
                {
                case 500:
                case 501:
                case 503:
                case 530:
                    fd->state = FTP_STATE_LOGIN;
                    break;
                default:
                    goto fail;
                }
                break;
            default:
                goto fail;
            }
            break;
        case FTP_STATE_MONITOR:
            switch (code)
            {
            case 227:
                fd->cmd = FTP_CMD_NONE;
                code = ftp_validate_pasv(data + init_offset,
                                         (uint16_t)(offset-init_offset),
                                         &address, &port);
                if (!code)
                {
                    snort_ip ip;
                    snort_ip *sip;
                    snort_ip *dip;

                    dip = GET_DST_IP(pkt);
                    sip = GET_SRC_IP(pkt);
                    ip.family = AF_INET;
                    ip.bits = 32;
                    ip.ip32[0] = htonl(address);
                    ip.ip32[1] = 0;
                    ip.ip32[2] = 0;
                    ip.ip32[3] = 0;
                    fp = ftp_service_mod.api->flow_new(pkt, dip, 0, &ip, port, flowp->proto, ftp_data_app_id);
                    if (fp)
                    {
                        fp->serviceAppId = APP_ID_FTP_DATA;
                        flow_mark(fp, FLOW_SERVICEDETECTED | FLOW_NOT_A_SERVICE | FLOW_PORT_SERVICE_DONE);
                        fp->rnaServiceState = RNA_STATE_FINISHED;
                        fp->rnaClientState = RNA_STATE_FINISHED;
                    }
                    if (!sfip_fast_equals_raw(&ip, sip))
                    {
                        fp = ftp_service_mod.api->flow_new(pkt, dip, 0, sip, port, flowp->proto, ftp_data_app_id);
                        if (fp)
                        {
                            fp->serviceAppId = APP_ID_FTP_DATA;
                            flow_mark(fp, FLOW_SERVICEDETECTED | FLOW_NOT_A_SERVICE | FLOW_PORT_SERVICE_DONE);
                            fp->rnaServiceState = RNA_STATE_FINISHED;
                            fp->rnaClientState = RNA_STATE_FINISHED;
                        }
                    }
                }
                else if (code < 0)
                {
                    goto fail;
                }
                break;
            case 229:
                fd->cmd = FTP_CMD_NONE;
                code = ftp_validate_epsv(data + init_offset,
                                         (uint16_t)(offset-init_offset),
                                         &port);

                if (!code)
                {
                    snort_ip *sip;
                    snort_ip *dip;

                    dip = GET_DST_IP(pkt);
                    sip = GET_SRC_IP(pkt);
                    fp = ftp_service_mod.api->flow_new(pkt, dip, 0, sip, port, flowp->proto, ftp_data_app_id);
                    if (fp)
                    {
                        fp->serviceAppId = APP_ID_FTP_DATA;
                        flow_mark(fp, FLOW_SERVICEDETECTED | FLOW_NOT_A_SERVICE | FLOW_PORT_SERVICE_DONE);
                        fp->rnaServiceState = RNA_STATE_FINISHED;
                        fp->rnaClientState = RNA_STATE_FINISHED;
                    }
                }
                else if (code < 0)
                {
                    goto fail;
                }
                break;
            case 200:
                if (fd->cmd == FTP_CMD_PORT)
                {
                    snort_ip ip;
                    snort_ip *sip;

                    sip = GET_SRC_IP(pkt);
                    ip.family = AF_INET;
                    ip.bits = 32;
                    ip.ip32[0] = htonl(fd->address);
                    ip.ip32[1] = 0;
                    ip.ip32[2] = 0;
                    ip.ip32[3] = 0;
                    fp = ftp_service_mod.api->flow_new(pkt, sip, 0, &ip, fd->port, flowp->proto, ftp_data_app_id);
                    if (fp)
                    {
                        fp->serviceAppId = APP_ID_FTP_DATA;
                        flow_mark(fp, FLOW_SERVICEDETECTED | FLOW_NOT_A_SERVICE | FLOW_PORT_SERVICE_DONE);
                        fp->rnaServiceState = RNA_STATE_FINISHED;
                        fp->rnaClientState = RNA_STATE_FINISHED;
                    }
                }
                fd->cmd = FTP_CMD_NONE;
                break;
            default:
                fd->cmd = FTP_CMD_NONE;
                break;
            }
            break;
        case FTP_STATE_CONNECTION_ERROR:
        default:
            goto fail;
        }
    }

    switch (retval)
    {
    default:
    case SERVICE_INPROCESS:
inprocess:
        if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
        {
            ftp_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element);
        }
        return SERVICE_INPROCESS;

    case SERVICE_SUCCESS:
        if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
        {
            ftp_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                             APP_ID_FTP_CONTROL, fd->vendor[0] ? fd->vendor:NULL,
                                             fd->version[0] ? fd->version:NULL, NULL);
        }
        return SERVICE_SUCCESS;

    case SERVICE_NOMATCH:
fail:
        if (!flow_checkflag(flowp, FLOW_SERVICEDETECTED))
        {
            ftp_service_mod.api->fail_service(flowp, pkt, dir, &svc_element);
        }
        flow_clear(flowp, FLOW_CONTINUE);
        return SERVICE_NOMATCH;
    }
}

