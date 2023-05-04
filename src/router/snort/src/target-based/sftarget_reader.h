/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2006-2013 Sourcefire, Inc.
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

/*
 * Author: Steven Sturges
 * sftarget_reader.c
 */

#ifndef SF_TARGET_READER_H_
#define SF_TARGET_READER_H_

#include "snort.h"
#ifdef REG_TEST
#include "reg_test.h"
#endif

#define SFAT_OK 0
#define SFAT_ERROR -1

#define SFAT_CHECKHOST \
    if (!current_host) return SFAT_ERROR;
#define SFAT_CHECKAPP \
    if (!current_app) return SFAT_ERROR;

void SigAttributeTableReloadHandler(int signal);

typedef enum
{
    ATTRIBUTE_NAME,
    ATTRIBUTE_ID
} AttributeTypes;

typedef enum
{
    ATTRIBUTE_SERVICE,
    ATTRIBUTE_CLIENT
} ServiceClient;

typedef struct _MapData
{
    char s_mapvalue[STD_BUF];
    uint32_t l_mapid;
} MapData;

typedef MapData MapEntry;

typedef struct _AttributeData
{
    AttributeTypes type;
    union
    {
        char s_value[STD_BUF];
        uint32_t l_value;
    } value;
    int confidence;
    int16_t attributeOrdinal;
} AttributeData;

#define APPLICATION_ENTRY_PORT 0x01
#define APPLICATION_ENTRY_IPPROTO 0x02
#define APPLICATION_ENTRY_PROTO 0x04
#define APPLICATION_ENTRY_APPLICATION 0x08
#define APPLICATION_ENTRY_VERSION 0x10

typedef struct _ApplicationEntry
{
    struct _ApplicationEntry *next;

    uint16_t port;
    uint16_t ipproto;
    uint16_t protocol;

    uint8_t fields;
} ApplicationEntry;

typedef ApplicationEntry ApplicationList;

#define HOST_INFO_OS 1
#define HOST_INFO_VENDOR 2
#define HOST_INFO_VERSION 3
#define HOST_INFO_FRAG_POLICY 4
#define HOST_INFO_STREAM_POLICY 5
#define POLICY_SET 1
#define POLICY_NOT_SET 0
typedef struct _HostInfo
{
    char streamPolicyName[16];
    char fragPolicyName[16];

    uint16_t streamPolicy;
    uint16_t fragPolicy;

    char streamPolicySet;
    char fragPolicySet;
} HostInfo;

#define SFAT_SERVICE 1
#define SFAT_CLIENT 2
typedef struct _HostAttributeEntry
{
    sfcidr_t ipAddr;

    HostInfo hostInfo;
    ApplicationList *services;
    ApplicationList *clients;
} HostAttributeEntry;

/* Callback Functions from YACC */
int SFAT_AddMapEntry(MapEntry *);
char *SFAT_LookupAttributeNameById(int id);
HostAttributeEntry * SFAT_CreateHostEntry(void);
int SFAT_AddHostEntryToMap(void);
int SFAT_SetHostIp(const char *);
int SFAT_SetOSAttribute(AttributeData *data, int attribute);
int SFAT_SetOSPolicy(char *policy_name, int attribute);
ApplicationEntry * SFAT_CreateApplicationEntry(void);
int SFAT_AddApplicationData(void);
int SFAT_SetApplicationAttribute(AttributeData *data, int attribute);
void PrintAttributeData(char *prefix, AttributeData *data);

/* Callback to set frag & stream policy IDs */
typedef int (*GetPolicyIdFunc)(HostAttributeEntry *);
typedef struct _GetPolicyIdsCallbackList
{
    GetPolicyIdFunc policyCallback;
    struct _GetPolicyIdsCallbackList *next;
} GetPolicyIdsCallbackList;
void SFAT_SetPolicyIds(GetPolicyIdFunc policyCallback, int snortPolicyId);

/* Cleanup Functions, called by Snort shutdown */
void SFAT_Cleanup(void);
void FreeHostEntry(HostAttributeEntry *host);

/* Parsing Functions -- to be called by Snort parser */
int SFAT_ParseAttributeTable(char *args, SnortConfig *sc);

/* Function to swap out new table */
void AttributeTableReloadCheck(void);

/* Status functions */
uint32_t SFAT_NumberOfHosts(void);

/* API Lookup functions, to be called by Stream & Frag */
HostAttributeEntry *SFAT_LookupHostEntryByIP(sfaddr_t *ipAddr);
HostAttributeEntry *SFAT_LookupHostEntryBySrc(Packet *p);
HostAttributeEntry *SFAT_LookupHostEntryByDst(Packet *p);
void SFAT_UpdateApplicationProtocol(sfaddr_t *ipAddr, uint16_t port, uint16_t protocol, uint16_t id);

/* Returns whether this has been configured */
int IsAdaptiveConfigured( void );
int IsAdaptiveConfiguredForSnortConfig(struct _SnortConfig *);

void SFAT_StartReloadThread(void);

void SFLAT_init(void);
void SFLAT_fini(void);
int  SFLAT_isEnabled(tSfPolicyId id, int parsing);

#ifdef SNORT_RELOAD
void SFAT_ReloadCheck(struct _SnortConfig *);
void ReloadAttributeThreadStop(void);
void SFAT_CleanPrevConfig(void);
#endif

#endif /* SF_TARGET_READER_H_ */
