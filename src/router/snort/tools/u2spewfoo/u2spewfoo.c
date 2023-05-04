/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2002-2013 Sourcefire, Inc.
 * Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
 * Author: Adam Keeton
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#ifndef WIN32
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#ifdef HAVE_UUID_UUID_H
#include<uuid/uuid.h>
#endif

#include "Unified2_common.h"

#define SUCCESS 314159265
#define STEVE -1
#define FAILURE STEVE

#ifndef WIN32
#ifndef uint32_t
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#endif
#else
static void inet_ntop(int family, const void *ip_raw, char *buf, int bufsize) {
    int i;

    if(!ip_raw || !buf || !bufsize ||
       (family != AF_INET && family != AF_INET6) ||
       /* Make sure if it's IPv6 that the buf is large enough. */
       /* Need atleast a max of 8 fields of 4 bytes plus 7 for colons in
        * between.  Need 1 more byte for null. */
       (family == AF_INET6 && bufsize < 8*4 + 7 + 1) ||
       /* Make sure if it's IPv4 that the buf is large enough. */
       /* 4 fields of 3 numbers, plus 3 dots and a null byte */
       (family == AF_INET && bufsize < 3*4 + 4) )
    {
        if(buf && bufsize > 0) buf[0] = 0;
        return;
    }

    /* 4 fields of at most 3 characters each */
    if(family == AF_INET) {
        u_int8_t *p = (u_int8_t*)ip_raw;

        for(i=0; p < ((u_int8_t*)ip_raw) + 4; p++) {
            i += sprintf(&buf[i], "%d", *p);

            /* If this is the last iteration, this could technically cause one
             *  extra byte to be written past the end. */
            if(i < bufsize && ((p + 1) < ((u_int8_t*)ip_raw+4)))
                buf[i] = '.';

            i++;
        }

    /* Check if this is really just an IPv4 address represented as 6,
     * in compatible format */
#if 0
    }
    else if(!field[0] && !field[1] && !field[2]) {
        unsigned char *p = (unsigned char *)(&ip->ip[12]);

        for(i=0; p < &ip->ip[16]; p++)
             i += sprintf(&buf[i], "%d.", *p);
#endif
    }
    else {
        u_int16_t *p = (u_int16_t*)ip_raw;

        for(i=0; p < ((u_int16_t*)ip_raw) + 8; p++) {
            i += sprintf(&buf[i], "%04x", ntohs(*p));

            /* If this is the last iteration, this could technically cause one
             *  extra byte to be written past the end. */
            if(i < bufsize && ((p + 1) < ((u_int16_t*)ip_raw) + 8))
                buf[i] = ':';

            i++;
        }
    }
}
#endif

typedef struct _record {
    uint32_t type;
    uint32_t length;
    uint8_t *data;
} u2record;

typedef struct _u2iterator {
    FILE *file;
    char *filename;
    u2record current;
} u2iterator;

static long s_pos = 0, s_off = 0;

#define TO_IP(x) x >> 24, (x >> 16) & 0xff, (x >> 8) & 0xff, x & 0xff

static u2iterator *new_iterator(char *filename) {
    FILE *f = fopen(filename, "rb");
    u2iterator *ret;

    if(!f) {
        printf("new_iterator: Failed to open file: %s\n\tErrno: %s\n",
          filename, strerror(errno));
        return NULL;
    }

    ret = (u2iterator*)malloc(sizeof(u2iterator));

    if(!ret) {
        printf("new_iterator: Failed to malloc %lu bytes.\n", (unsigned long)sizeof(u2iterator));
        fclose(f);
        return NULL;
    }

    ret->file = f;
    ret->filename = strdup(filename);
    return ret;
}

static inline void free_iterator(u2iterator *it) {
    if(it->file) fclose(it->file);
    if(it->filename) free(it->filename);
    if(it) free(it);
}

static int get_record(u2iterator *it, u2record *record) {
    uint32_t bytes_read;
    uint8_t *tmp;

    if(!it || !it->file) return FAILURE;

    /* check if the log was rotated */
    if(feof(it->file)) {
        /* Get next timestamped file? */
        puts("Hit the EOF .. and this is not being handled yet.");
        return FAILURE;
    }

    if ( s_off )
    {
        if (fseek(it->file, s_pos+s_off, SEEK_SET)) 
        {
            puts("Unable to SEEK on current file .. and this is not being handled yet.");
            return FAILURE;
        }
        s_off = 0;
    }

    /* read type and length */
    bytes_read = fread(record, 1, sizeof(uint32_t) * 2, it->file);
    /* But they're in network order! */
    record->type= ntohl(record->type);
    record->length= ntohl(record->length);

    //if(record->type == UNIFIED2_PACKET) record->length+=4;

    if(bytes_read == 0)
       /* EOF */
       return FAILURE;

    if(bytes_read != sizeof(uint32_t)*2) {
        puts("get_record: (1) Failed to read all of record.");
        printf("\tRead %u of %lu bytes\n", bytes_read, (unsigned long)sizeof(uint32_t)*2);
        return FAILURE;
    }

    s_pos = ftell(it->file);

    tmp = (uint8_t *)realloc(record->data, record->length);
    
    if (!tmp)
    {
        puts("get_record: (2) Failed to allocate memory.");
        free(record->data);
        return FAILURE;
    }

    record->data = tmp;

    bytes_read = fread(record->data, 1, record->length, it->file);

    if(bytes_read != record->length) {
        puts("get_record: (3) Failed to read all of record data.");
        printf("\tRead %u of %u bytes\n", bytes_read, record->length);

        if ( record->type != UNIFIED2_PACKET || 
            bytes_read < ntohl(((Serial_Unified2Packet*)record->data)->packet_length)
        )
            return FAILURE;

        clearerr(it->file);
    }

    return SUCCESS;
}

static void extradata_dump(u2record *record) {
    uint8_t *field, *data;
    int i;
    int len = 0;
    SerialUnified2ExtraData event;
    Unified2ExtraDataHdr eventHdr;
    uint32_t ip;
    char ip6buf[INET6_ADDRSTRLEN+1];
    struct in6_addr ipAddr;

    memcpy(&eventHdr, record->data, sizeof(Unified2ExtraDataHdr));

    memcpy(&event, record->data + sizeof(Unified2ExtraDataHdr) , sizeof(SerialUnified2ExtraData));

    /* network to host ordering */
    field = (uint8_t*)&eventHdr;
    for(i=0; i<2; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }

    field = (uint8_t*)&event;
    for(i=0; i<6; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }



    printf("\n(ExtraDataHdr)\n"
            "\tevent type: %u\tevent length: %u\n",
            eventHdr.event_type, eventHdr.event_length);

    printf("\n(ExtraData)\n"
            "\tsensor id: %u\tevent id: %u\tevent second: %u\n"
            "\ttype: %u\tdatatype: %u\tbloblength: %u\t",
             event.sensor_id, event.event_id,
             event.event_second, event.type,
             event.data_type, event.blob_length);

    len = event.blob_length - sizeof(event.blob_length) - sizeof(event.data_type);

    switch(event.type)
    {
        case EVENT_INFO_XFF_IPV4:
            memcpy(&ip, record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData), sizeof(uint32_t));
            ip = ntohl(ip);
            printf("Original Client IP: %u.%u.%u.%u\n",
                    TO_IP(ip));
            break;

        case EVENT_INFO_XFF_IPV6:
            memcpy(&ipAddr, record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData), sizeof(struct in6_addr));
            inet_ntop(AF_INET6, &ipAddr, ip6buf, INET6_ADDRSTRLEN);
            printf("Original Client IP: %s\n",
                    ip6buf);
            break;

        case EVENT_INFO_GZIP_DATA:
            printf("GZIP Decompressed Data: %.*s\n",
                len, record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData));
            break;

        case EVENT_INFO_JSNORM_DATA:
            printf("Normalized JavaScript Data: %.*s\n",
                len, record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData));
            break;

        case EVENT_INFO_SMTP_FILENAME:
            printf("SMTP Attachment Filename: %.*s\n",
                len,record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData));
            break;

        case EVENT_INFO_SMTP_MAILFROM:
            printf("SMTP MAIL FROM Addresses: %.*s\n",
                    len,record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData));
            break;

        case EVENT_INFO_SMTP_RCPTTO:
            printf("SMTP RCPT TO Addresses: %.*s\n",
                len, record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData));
            break;

        case EVENT_INFO_SMTP_EMAIL_HDRS:
            printf("SMTP EMAIL HEADERS: \n%.*s\n",
                len, record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData));
            break;

        case EVENT_INFO_HTTP_URI:
            printf("HTTP URI: %.*s\n",
                len, record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData));
            break;

        case EVENT_INFO_HTTP_HOSTNAME:
            printf("HTTP Hostname: ");
            data = record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData);
            for(i=0; i < len; i++)
            {
                if(iscntrl(data[i]))
                    printf("%c",'.');
                else
                    printf("%c",data[i]);
            }
            printf("\n");
            break;

        case EVENT_INFO_IPV6_SRC:
            memcpy(&ipAddr, record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData), sizeof(struct in6_addr));
            inet_ntop(AF_INET6, &ipAddr, ip6buf, INET6_ADDRSTRLEN);
            printf("IPv6 Source Address: %s\n",
                    ip6buf);
            break;

        case EVENT_INFO_IPV6_DST:
            memcpy(&ipAddr, record->data + sizeof(Unified2ExtraDataHdr) + sizeof(SerialUnified2ExtraData), sizeof(struct in6_addr));
            inet_ntop(AF_INET6, &ipAddr, ip6buf, INET6_ADDRSTRLEN);
            printf("IPv6 Destination Address: %s\n",
                    ip6buf);
            break;

        default :
            break;
    }

}

static void event_dump(u2record *record) {
    uint8_t *field;
    int i;
    Serial_Unified2IDSEvent_legacy event;

    memcpy(&event, record->data, sizeof(Serial_Unified2IDSEvent_legacy));

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first 11 fields need to be convertted */
    field = (uint8_t*)&event;
    for(i=0; i<11; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }

    /* last 3 fields, with the exception of the last most since it's just one byte */
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* sport_itype */
    field += 2;
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* dport_icode */
    /* done changing the network ordering */


    printf("\n(Event)\n"
            "\tsensor id: %u\tevent id: %u\tevent second: %u\tevent microsecond: %u\n"
            "\tsig id: %u\tgen id: %u\trevision: %u\t classification: %u\n"
            "\tpriority: %u\tip source: %u.%u.%u.%u\tip destination: %u.%u.%u.%u\n"
            "\tsrc port: %u\tdest port: %u\tprotocol: %u\timpact_flag: %u\tblocked: %u\n",
             event.sensor_id, event.event_id,
             event.event_second, event.event_microsecond,
             event.signature_id, event.generator_id,
             event.signature_revision, event.classification_id,
             event.priority_id, TO_IP(event.ip_source),
             TO_IP(event.ip_destination), event.sport_itype,
             event.dport_icode, event.protocol,
             event.impact_flag, event.blocked);
}

static void event6_dump(u2record *record) {
    uint8_t *field;
    int i;
    Serial_Unified2IDSEventIPv6_legacy event;
    char ip6buf[INET6_ADDRSTRLEN+1];

    memcpy(&event, record->data, sizeof(Serial_Unified2IDSEventIPv6_legacy));

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first fields need to be convertted */
    field = (uint8_t*)&event;
    for(i=0; i<9; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }

    field = field + 2*sizeof(struct in6_addr);

    /* last 3 fields, with the exception of the last most since it's just one byte */
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* sport_itype */
    field += 2;
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* dport_icode */
    /* done changing the network ordering */

    inet_ntop(AF_INET6, &event.ip_source, ip6buf, INET6_ADDRSTRLEN);

    printf("\n(IPv6 Event)\n"
            "\tsensor id: %u\tevent id: %u\tevent second: %u\tevent microsecond: %u\n"
            "\tsig id: %u\tgen id: %u\trevision: %u\t classification: %u\n"
            "\tpriority: %u\tip source: %s\t",
             event.sensor_id, event.event_id,
             event.event_second, event.event_microsecond,
             event.signature_id, event.generator_id,
             event.signature_revision, event.classification_id,
             event.priority_id, ip6buf);


    inet_ntop(AF_INET6, &event.ip_destination, ip6buf, INET6_ADDRSTRLEN);
    printf("ip destination: %s\n"
            "\tsrc port: %u\tdest port: %u\tprotocol: %u\timpact_flag: %u\tblocked: %u\n",
             ip6buf, event.sport_itype,
             event.dport_icode, event.protocol,
             event.impact_flag, event.blocked);
}



static void event2_dump(u2record *record) {
    uint8_t *field;
    int i;

    Serial_Unified2IDSEvent event;

    memcpy(&event, record->data, sizeof(Serial_Unified2IDSEvent));

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first 11 fields need to be convertted */
    field = (uint8_t*)&event;
    for(i=0; i<11; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }

    /* last 3 fields, with the exception of the last most since it's just one byte */
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* sport_itype */
    field += 2;
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* dport_icode */
    field +=6;
    *(uint32_t*)field = ntohl(*(uint32_t*)field); /* mpls_label */
    field += 4;
    /* policy_id and vlanid */
    for(i=0; i<2; i++, field+=2) {
        *(uint16_t*)field = ntohs(*(uint16_t*)field);
    }
    /* done changing the network ordering */


    printf("\n(Event)\n"
            "\tsensor id: %u\tevent id: %u\tevent second: %u\tevent microsecond: %u\n"
            "\tsig id: %u\tgen id: %u\trevision: %u\t classification: %u\n"
            "\tpriority: %u\tip source: %u.%u.%u.%u\tip destination: %u.%u.%u.%u\n"
            "\tsrc port: %u\tdest port: %u\tprotocol: %u\timpact_flag: %u\tblocked: %u\n"
            "\tmpls label: %u\tvland id: %u\tpolicy id: %u\n",
             event.sensor_id, event.event_id,
             event.event_second, event.event_microsecond,
             event.signature_id, event.generator_id,
             event.signature_revision, event.classification_id,
             event.priority_id, TO_IP(event.ip_source),
             TO_IP(event.ip_destination), event.sport_itype,
             event.dport_icode, event.protocol,
             event.impact_flag, event.blocked,
             event.mpls_label, event.vlanId, event.pad2);

}

#if defined(FEAT_OPEN_APPID)
static void event3_dump(u2record *record) 
{
    uint8_t *field;
    int i;

    Serial_Unified2IDSEvent event;

    memcpy(&event, record->data, sizeof(Serial_Unified2IDSEvent));

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first 11 fields need to be convertted */
    field = (uint8_t*)&event;
    for(i=0; i<11; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }

    /* last 3 fields, with the exception of the last most since it's just one byte */
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* sport_itype */
    field += 2;
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* dport_icode */
    field +=6;
    *(uint32_t*)field = ntohl(*(uint32_t*)field); /* mpls_label */
    field += 4;
    /* policy_id and vlanid */
    for(i=0; i<2; i++, field+=2) {
        *(uint16_t*)field = ntohs(*(uint16_t*)field);
    }
    /* done changing the network ordering */


    printf("\n(Event)\n"
            "\tsensor id: %u\tevent id: %u\tevent second: %u\tevent microsecond: %u\n"
            "\tsig id: %u\tgen id: %u\trevision: %u\t classification: %u\n"
            "\tpriority: %u\tip source: %u.%u.%u.%u\tip destination: %u.%u.%u.%u\n"
            "\tsrc port: %u\tdest port: %u\tprotocol: %u\timpact_flag: %u\tblocked: %u\n"
            "\tmpls label: %u\tvland id: %u\tpolicy id: %u\tappid: %s\n",
             event.sensor_id, event.event_id,
             event.event_second, event.event_microsecond,
             event.signature_id, event.generator_id,
             event.signature_revision, event.classification_id,
             event.priority_id, TO_IP(event.ip_source),
             TO_IP(event.ip_destination), event.sport_itype,
             event.dport_icode, event.protocol,
             event.impact_flag, event.blocked,
             event.mpls_label, event.vlanId, event.pad2, event.app_name);

}

#endif /* defined(FEAT_OPEN_APPID) */
static void event2_6_dump(u2record *record) {
    uint8_t *field;
    int i;
    char ip6buf[INET6_ADDRSTRLEN+1];
    Serial_Unified2IDSEventIPv6 event;

    memcpy(&event, record->data, sizeof(Serial_Unified2IDSEventIPv6));

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first fields need to be convertted */
    field = (uint8_t*)&event;
    for(i=0; i<9; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }

    field = field + 2*sizeof(struct in6_addr);

    /* last 3 fields, with the exception of the last most since it's just one byte */
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* sport_itype */
    field += 2;
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* dport_icode */
    field +=6;
    *(uint32_t*)field = ntohl(*(uint32_t*)field); /* mpls_label */
    field += 4;
    /* policy_id and vlanid */
    for(i=0; i<2; i++, field+=2) {
        *(uint16_t*)field = ntohs(*(uint16_t*)field);
    }
    /* done changing the network ordering */

    inet_ntop(AF_INET6, &event.ip_source, ip6buf, INET6_ADDRSTRLEN);

    printf("\n(IPv6 Event)\n"
            "\tsensor id: %u\tevent id: %u\tevent second: %u\tevent microsecond: %u\n"
            "\tsig id: %u\tgen id: %u\trevision: %u\t classification: %u\n"
            "\tpriority: %u\tip source: %s\t",
             event.sensor_id, event.event_id,
             event.event_second, event.event_microsecond,
             event.signature_id, event.generator_id,
             event.signature_revision, event.classification_id,
             event.priority_id, ip6buf);


    inet_ntop(AF_INET6, &event.ip_destination, ip6buf, INET6_ADDRSTRLEN);
    printf("ip destination: %s\n"
            "\tsrc port: %u\tdest port: %u\tprotocol: %u\timpact_flag: %u\tblocked: %u\n"
            "\tmpls label: %u\tvland id: %u\tpolicy id: %u\n",
             ip6buf, event.sport_itype,
             event.dport_icode, event.protocol,
             event.impact_flag, event.blocked,
             event.mpls_label, event.vlanId,event.pad2);

}

#if defined(FEAT_OPEN_APPID)
static void event3_6_dump(u2record *record) {
    uint8_t *field;
    int i;
    char ip6buf[INET6_ADDRSTRLEN+1];
    Serial_Unified2IDSEventIPv6 event;

    memcpy(&event, record->data, sizeof(Serial_Unified2IDSEventIPv6));

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first fields need to be convertted */
    field = (uint8_t*)&event;
    for(i=0; i<9; i++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }

    field = field + 2*sizeof(struct in6_addr);

    /* last 3 fields, with the exception of the last most since it's just one byte */
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* sport_itype */
    field += 2;
    *(uint16_t*)field = ntohs(*(uint16_t*)field); /* dport_icode */
    field +=6;
    *(uint32_t*)field = ntohl(*(uint32_t*)field); /* mpls_label */
    field += 4;
    /* policy_id and vlanid */
    for(i=0; i<2; i++, field+=2) {
        *(uint16_t*)field = ntohs(*(uint16_t*)field);
    }
    /* done changing the network ordering */

    inet_ntop(AF_INET6, &event.ip_source, ip6buf, INET6_ADDRSTRLEN);

    printf("\n(IPv6 Event)\n"
            "\tsensor id: %u\tevent id: %u\tevent second: %u\tevent microsecond: %u\n"
            "\tsig id: %u\tgen id: %u\trevision: %u\t classification: %u\n"
            "\tpriority: %u\tip source: %s\t",
             event.sensor_id, event.event_id,
             event.event_second, event.event_microsecond,
             event.signature_id, event.generator_id,
             event.signature_revision, event.classification_id,
             event.priority_id, ip6buf);


    inet_ntop(AF_INET6, &event.ip_destination, ip6buf, INET6_ADDRSTRLEN);
    printf("ip destination: %s\n"
            "\tsrc port: %u\tdest port: %u\tprotocol: %u\timpact_flag: %u\tblocked: %u\n"
            "\tmpls label: %u\tvland id: %u\tpolicy id: %u\tappid: %s\n",
             ip6buf, event.sport_itype,
             event.dport_icode, event.protocol,
             event.impact_flag, event.blocked,
             event.mpls_label, event.vlanId,event.pad2, event.app_name);

}

static void appid_dump(u2record *record) {
    uint8_t *field = (uint8_t*)record->data;
    unsigned i;
    unsigned appCnt;
    unsigned statTime;

    /* network to host ordering */
    /* In the event structure, only the last 40 bits are not 32 bit fields */
    /* The first fields need to be convertted */
    statTime = ntohl(*(uint32_t*)field);
    field += 4;
    appCnt = ntohl(*(uint32_t*)field); 
    field += 4;
    
    printf("\n(AppId Stats)\n"
            "    event second: %u\tRecordCount: %u\n",
             statTime, appCnt);
    for(i=0; i<appCnt; i++) 
    {
        char appName[MAX_EVENT_APPNAME_LEN];
        memcpy(appName, field, sizeof(appName));
        field += MAX_EVENT_APPNAME_LEN;

        int txBytes = ntohl(*(uint32_t*)field);
        field += 4;
        int rxBytes = ntohl(*(uint32_t*)field);
        field += 4;

        printf("    -----------\n\tApp:%s\n\tbytes_out: %u\n\tbytes_in: %u\n",
            appName, txBytes, rxBytes);
     }
}

#endif /* defined(FEAT_OPEN_APPID) */

#define LOG_CHARS 16

static void LogBuffer (const uint8_t* p, unsigned n)
{
    char hex[(3*LOG_CHARS)+1];
    char txt[LOG_CHARS+1];
    unsigned odx = 0, idx = 0, at = 0;

    for ( idx = 0; idx < n; idx++)
    {
        uint8_t byte = p[idx];
        sprintf(hex + 3*odx, "%2.02X ", byte);
        txt[odx++] = isprint(byte) ? byte : '.';

        if ( odx == LOG_CHARS )
        {
            txt[odx] = hex[3*odx] = '\0';
            printf("[%5u] %s %s\n", at, hex, txt);
            at = idx + 1;
            odx = 0;
        }
    }
    if ( odx )
    {
        txt[odx] = hex[3*odx] = '\0';
        printf("[%5u] %-48.48s %s\n", at, hex, txt);
    }
}

static void packet_dump(u2record *record) {
    uint32_t counter;
    uint8_t *field;

    unsigned offset = sizeof(Serial_Unified2Packet)-4;
    unsigned reclen = record->length - offset;

    Serial_Unified2Packet packet;
    memcpy(&packet, record->data, sizeof(Serial_Unified2Packet));

    /* network to host ordering */
    /* The first 7 fields need to be convertted */
    field = (uint8_t*)&packet;
    for(counter=0; counter<7; counter++, field+=4) {
        *(uint32_t*)field = ntohl(*(uint32_t*)field);
    }
    /* done changing from network ordering */

    printf("\nPacket\n"
            "\tsensor id: %u\tevent id: %u\tevent second: %u\n"
            "\tpacket second: %u\tpacket microsecond: %u\n"
            "\tlinktype: %u\tpacket_length: %u\n",
            packet.sensor_id, packet.event_id, packet.event_second,
            packet.packet_second, packet.packet_microsecond, packet.linktype,
            packet.packet_length);

    
    if ( record->length <= offset )
        return;

    if ( packet.packet_length != reclen )
    {
        printf("ERROR: logged %u but packet_length = %u\n",
            record->length-offset, packet.packet_length);

        if ( packet.packet_length < reclen )
        {
            reclen = packet.packet_length;
            s_off = reclen + offset;
        }
    }
    LogBuffer(record->data+offset, reclen);
}

static int u2dump(char *file) {
    u2record record;
    u2iterator *it = new_iterator(file);

    memset(&record, 0, sizeof(record));

    if(!it) {
        printf("u2dump: Failed to create new iterator with file: %s\n", file);
        return -1;
    }

    while( get_record(it, &record) == SUCCESS ) {
        if(record.type == UNIFIED2_IDS_EVENT) event_dump(&record);
        else if(record.type == UNIFIED2_IDS_EVENT_VLAN) event2_dump(&record);
        else if(record.type == UNIFIED2_PACKET) packet_dump(&record);
        else if(record.type == UNIFIED2_IDS_EVENT_IPV6) event6_dump(&record);
        else if(record.type == UNIFIED2_IDS_EVENT_IPV6_VLAN) event2_6_dump(&record);
        else if(record.type == UNIFIED2_EXTRA_DATA) extradata_dump(&record);
#if defined(FEAT_OPEN_APPID)

        else if(record.type == UNIFIED2_IDS_EVENT_APPID) event3_dump(&record);
        else if(record.type == UNIFIED2_IDS_EVENT_APPID_IPV6) event3_6_dump(&record);
        else if(record.type == UNIFIED2_IDS_EVENT_APPSTAT) appid_dump(&record);
#endif /* defined(FEAT_OPEN_APPID) */
    }

    free_iterator(it);
    if(record.data)
        free(record.data);

    return 0;
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("usage: %s <file>\n",argv[0]);
        return 1;
    }

    return u2dump(argv[1]);
}
