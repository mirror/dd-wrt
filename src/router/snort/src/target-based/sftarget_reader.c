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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GETTID
#define _GNU_SOURCE
#endif

#ifdef TARGET_BASED

#include <stdio.h>
#include "mstring.h"
#include "util.h"
#include "parser.h"
#include "sftarget_reader.h"
#include "sftarget_protocol_reference.h"
#include "sfutil/sfrt.h"
#include "sfutil/sfxhash.h"
#include "sfutil/util_net.h"
#include "sftarget_hostentry.h"

#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "snort.h"

#include "snort_debug.h"
#include "sfPolicy.h"

typedef struct
{
    table_t *lookupTable;
    SFXHASH *mapTable;
} tTargetBasedConfig;

typedef struct
{
    /**current configuration. */
    tTargetBasedConfig curr;

    /**previous configuration. */
    tTargetBasedConfig prev;

    /**next configuration. */
    tTargetBasedConfig next;

    //XXX recheck this flag usage
    //char reload_attribute_table_flags;

} tTargetBasedPolicyConfig;


static tTargetBasedPolicyConfig targetBasedPolicyConfig;

static HostAttributeEntry *current_host = NULL;
static ApplicationEntry *current_app = NULL;

//static MapData *current_map_entry = NULL;
ServiceClient sfat_client_or_service;

extern char sfat_error_message[STD_BUF];
extern char sfat_grammar_error_printed;
extern char sfat_insufficient_space_logged;
extern char sfat_fatal_error;
int ParseTargetMap(char *filename);
void DestroyBufferStack(void);

extern char *sfat_saved_file;

extern pthread_t attribute_reload_thread_id;
extern pid_t attribute_reload_thread_pid;
extern volatile int attribute_reload_thread_running;
extern volatile int attribute_reload_thread_stop;
extern int reload_attribute_table_flags;
extern const struct timespec thread_sleep;

/*****TODO: cleanup to use config directive *******/
#define ATTRIBUTE_MAP_MAX_ROWS 1024
uint32_t SFAT_NumberOfHosts(void)
{
    tTargetBasedPolicyConfig *pConfig = &targetBasedPolicyConfig;
    if (pConfig->curr.lookupTable)
    {
        return sfrt_num_entries(pConfig->curr.lookupTable);
    }

    return 0;
}

int SFAT_AddMapEntry(MapEntry *entry)
{
    tTargetBasedPolicyConfig *pConfig = &targetBasedPolicyConfig;

    if (!pConfig->next.mapTable)
    {
        /* Attribute Table node includes memory for each entry,
         * as defined by sizeof(MapEntry).
         */
        pConfig->next.mapTable = sfxhash_new(ATTRIBUTE_MAP_MAX_ROWS,
                                          sizeof(int),
                                          sizeof(MapEntry),
                                          0,
                                          1,
                                          NULL,
                                          NULL,
                                          1);
        if (!pConfig->next.mapTable)
            FatalError("Failed to allocate attribute map table\n");
    }

    /* Memcopy MapEntry to newly allocated one and store in
     * a hash table based on entry->id for easy lookup.
     */

    DEBUG_WRAP(
        DebugMessage(DEBUG_ATTRIBUTE, "Adding Map Entry: %d %s\n",
            entry->l_mapid, entry->s_mapvalue););

    /* Data from entry will be copied into new node */
    sfxhash_add(pConfig->next.mapTable, &entry->l_mapid, entry);

    return SFAT_OK;
}

char *SFAT_LookupAttributeNameById(int id)
{
    MapEntry *entry;
    tTargetBasedPolicyConfig *pConfig = &targetBasedPolicyConfig;

    if (!pConfig->next.mapTable)
        return NULL;

    entry = sfxhash_find(pConfig->next.mapTable, &id);

    if (entry)
    {
        DEBUG_WRAP(
            DebugMessage(DEBUG_ATTRIBUTE, "Found Attribute Name %s for Id %d\n",
                entry->s_mapvalue, id););
        return entry->s_mapvalue;
    }
    DEBUG_WRAP(
        DebugMessage(DEBUG_ATTRIBUTE, "No Attribute Name for Id %d\n", id););

    return NULL;
}

void FreeApplicationEntry(ApplicationEntry *app)
{
    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Freeing ApplicationEntry: 0x%x\n",
        app););
    free(app);
}

ApplicationEntry * SFAT_CreateApplicationEntry(void)
{
    if (current_app)
    {
        /* Something went wrong */
        FreeApplicationEntry(current_app);
        current_app = NULL;
    }

    current_app = SnortAlloc(sizeof(ApplicationEntry));

    return current_app;
}

HostAttributeEntry * SFAT_CreateHostEntry(void)
{
    if (current_host)
    {
        /* Something went wrong */
        FreeHostEntry(current_host);
        current_host = NULL;
    }

    current_host = SnortAlloc(sizeof(HostAttributeEntry));

    return current_host;
}

void FreeHostEntry(HostAttributeEntry *host)
{
    ApplicationEntry *app = NULL, *tmp_app;

    if (!host)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_ATTRIBUTE, "Freeing HostEntry: 0x%x\n",
        host););

    /* Free the service list */
    if (host->services)
    {
        do
        {
            tmp_app = host->services;
            app = tmp_app->next;
            FreeApplicationEntry(tmp_app);
            host->services = app;
        } while (app);
    }

    /* Free the client list */
    if (host->clients)
    {
        do
        {
            tmp_app = host->clients;
            app = tmp_app->next;
            FreeApplicationEntry(tmp_app);
            host->clients = app;
        } while (app);
    }

    free(host);
}

void PrintAttributeData(char *prefix, AttributeData *data)
{
#ifdef DEBUG_MSGS
    DebugMessage(DEBUG_ATTRIBUTE, "AttributeData for %s\n", prefix);
    if (data->type == ATTRIBUTE_NAME)
    {
        DebugMessage(DEBUG_ATTRIBUTE, "\ttype: %s\tname: %s\t confidence %d\n",
                "Name", data->value.s_value, data->confidence);
    }
    else
    {
        DebugMessage(DEBUG_ATTRIBUTE, "\ttype: %s\tid: %s\t confidence %d",
                "Id", data->value.l_value, data->confidence);
    }
#endif
}

int SFAT_SetHostIp(const char *ip)
{
    static HostAttributeEntry *tmp_host = NULL;
    sfcidr_t ipAddr;
    tTargetBasedPolicyConfig *pConfig = &targetBasedPolicyConfig;
    SFAT_CHECKHOST;

    if (sfip_pton(ip, &ipAddr) != SFIP_SUCCESS)
    {
        return SFAT_ERROR;
    }

    tmp_host = sfrt_lookup(&ipAddr.addr, pConfig->next.lookupTable);

    if (tmp_host &&
        sfip_equals(tmp_host->ipAddr.addr, ipAddr.addr))
    {
        /* Exact match. */
        FreeHostEntry(current_host);
        current_host = tmp_host;
    }
    else
    {
        /* New entry for this host/CIDR */
        sfip_set_ip(&current_host->ipAddr, &ipAddr);
    }
    return SFAT_OK;
}

int SFAT_SetOSPolicy(char *policy_name, int attribute)
{
    SFAT_CHECKHOST;

    switch (attribute)
    {
        case HOST_INFO_FRAG_POLICY:
            SnortStrncpy(current_host->hostInfo.fragPolicyName, policy_name,
                sizeof(current_host->hostInfo.fragPolicyName));
            break;
        case HOST_INFO_STREAM_POLICY:
            SnortStrncpy(current_host->hostInfo.streamPolicyName, policy_name,
                sizeof(current_host->hostInfo.streamPolicyName));
            break;
    }
    return SFAT_OK;
}

int SFAT_SetOSAttribute(AttributeData *data, int attribute)
{
    SFAT_CHECKHOST;
    // currently not using os, vendor, or version
    return SFAT_OK;
}

static void AppendApplicationData(ApplicationList **list)
{
    if (!list)
        return;

    if (*list)
    {
        current_app->next = *list;
    }
    *list = current_app;
    current_app = NULL;
}


int SFAT_AddApplicationData(void)
{
    uint8_t required_fields;
    SFAT_CHECKAPP;
    SFAT_CHECKHOST;

    if (sfat_client_or_service == ATTRIBUTE_SERVICE)
    {
        required_fields = (APPLICATION_ENTRY_PORT |
                          APPLICATION_ENTRY_IPPROTO |
                          APPLICATION_ENTRY_PROTO);
        if ((current_app->fields & required_fields) != required_fields)
        {
            FatalError("%s(%d): Missing required field in Service attribute table for host %s\n",
                file_name, file_line,
                sfip_to_str(&current_host->ipAddr.addr)
                );
        }

        AppendApplicationData(&current_host->services);
    }
    else
    {
        required_fields = (APPLICATION_ENTRY_PROTO);
        /* Currently, client data only includes PROTO, not IPPROTO */
        if ((current_app->fields & required_fields) != required_fields)
        {
            FatalError("%s(%d): Missing required field in Client attribute table for host %s\n",
                file_name, file_line,
                sfip_to_str(&current_host->ipAddr.addr)
                );
        }

        AppendApplicationData(&current_host->clients);
    }
    return SFAT_OK;
}

int SFAT_SetApplicationAttribute(AttributeData *data, int attribute)
{
    SFAT_CHECKAPP;

    switch(attribute)
    {
        case APPLICATION_ENTRY_PORT:
            /* Convert the port to a integer */
            if (data->type == ATTRIBUTE_NAME)
            {
                char *endPtr = NULL;
                unsigned long value = SnortStrtoul(data->value.s_value, &endPtr, 10);
                if ((endPtr == &data->value.s_value[0]) ||
                    (errno == ERANGE) || value > UINT16_MAX)
                {
                    current_app->port = 0;
                    return SFAT_ERROR;
                }

                current_app->port = (uint16_t)value;
            }
            else
            {
                if (data->value.l_value > UINT16_MAX)
                {
                    current_app->port = 0;
                    return SFAT_ERROR;
                }

                current_app->port = (uint16_t)data->value.l_value;
            }
            break;
        case APPLICATION_ENTRY_IPPROTO:
            /* Add IP Protocol to the reference list */
            current_app->ipproto = AddProtocolReference(data->value.s_value);
            break;
        case APPLICATION_ENTRY_PROTO:
            /* Add Application Protocol to the reference list */
            current_app->protocol = AddProtocolReference(data->value.s_value);
            break;
        // currently not using application or version
        default:
            attribute = 0;
    }
    current_app->fields |= attribute;

    return SFAT_OK;
}

#ifdef DEBUG_MSGS
void PrintHostAttributeEntry(HostAttributeEntry *host)
{
    ApplicationEntry *app;
    int i = 0;

    if (!host)
        return;

    DebugMessage(DEBUG_ATTRIBUTE, "Host IP: %s/%d\n",
            sfip_to_str(&host->ipAddr.addr),
            (sfip_family(&host->ipAddr) == AF_INET) ? ((host->ipAddr.bits >= 96) ? (host->ipAddr.bits - 96) : -1) : host->ipAddr.bits
            );
    DebugMessage(DEBUG_ATTRIBUTE, "\tPolicy Information: frag:%s (%s %u) stream: %s (%s %u)\n",
            host->hostInfo.fragPolicyName, host->hostInfo.fragPolicySet ? "set":"unset", host->hostInfo.fragPolicy,
            host->hostInfo.fragPolicyName, host->hostInfo.streamPolicySet ? "set":"unset", host->hostInfo.streamPolicy);
    DebugMessage(DEBUG_ATTRIBUTE, "\tServices:\n");
    for (i=0, app = host->services; app; app = app->next,i++)
    {
        DebugMessage(DEBUG_ATTRIBUTE, "\tService #%d:\n", i);
        DebugMessage(DEBUG_ATTRIBUTE, "\t\tIPProtocol: %s\tPort: %s\tProtocol %s\n",
                app->ipproto, app->port, app->protocol);
    }
    if (i==0)
        DebugMessage(DEBUG_ATTRIBUTE, "\t\tNone\n");

    DebugMessage(DEBUG_ATTRIBUTE, "\tClients:\n");
    for (i=0, app = host->clients; app; app = app->next,i++)
    {
        DebugMessage(DEBUG_ATTRIBUTE, "\tClient #%d:\n", i);
        DebugMessage(DEBUG_ATTRIBUTE, "\t\tIPProtocol: %s\tProtocol %s\n",
                app->ipproto, app->protocol);

        if (app->fields & APPLICATION_ENTRY_PORT)
        {
            DebugMessage(DEBUG_ATTRIBUTE, "\t\tPort: %s\n", app->port);
        }
    }
    if (i==0)
    {
        DebugMessage(DEBUG_ATTRIBUTE, "\t\tNone\n");
    }
}
#endif

int SFAT_AddHostEntryToMap(void)
{
    HostAttributeEntry *host = current_host;
    int ret;
    sfcidr_t *ipAddr;
    tTargetBasedPolicyConfig *pConfig = &targetBasedPolicyConfig;

    SFAT_CHECKHOST;

    DEBUG_WRAP(PrintHostAttributeEntry(host););

    ipAddr = &host->ipAddr;

    ret = sfrt_insert(ipAddr, (unsigned char)ipAddr->bits, host,
                        RT_FAVOR_SPECIFIC, pConfig->next.lookupTable);

    if (ret != RT_SUCCESS)
    {
        if (ret == RT_POLICY_TABLE_EXCEEDED)
        {
            if (!sfat_insufficient_space_logged)
            {
                SnortSnprintf(sfat_error_message, STD_BUF,
                    "AttributeTable insertion failed: %d Insufficient "
                    "space in attribute table, only configured to store %d hosts\n",
                    ret, ScMaxAttrHosts(snort_conf));
               sfat_grammar_error_printed = 1;
                sfat_insufficient_space_logged = 1;
                sfat_fatal_error = 0;
            }
            /* Reset return value and continue w/ only snort_conf->max_attribute_hosts */
            ret = RT_SUCCESS;
        }
        else
        {
            SnortSnprintf(sfat_error_message, STD_BUF,
                "AttributeTable insertion failed: %d '%s'\n",
                ret, rt_error_messages[ret]);
            sfat_grammar_error_printed = 1;
        }

        FreeHostEntry(host);
    }

    current_host = NULL;

    return ret == RT_SUCCESS ? SFAT_OK : SFAT_ERROR;
}

HostAttributeEntry *SFAT_LookupHostEntryByIP(sfaddr_t *ipAddr)
{
    tTargetBasedPolicyConfig *pConfig = NULL;
    tSfPolicyId policyId = getNapRuntimePolicy();
    HostAttributeEntry *host = NULL;

    TargetBasedConfig *tbc = &snort_conf->targeted_policies[policyId]->target_based_config;

    if (tbc->args == NULL)
    {
        //this policy didn't specify attribute_table
        return NULL;
    }

    pConfig = &targetBasedPolicyConfig;

    host = sfrt_lookup(ipAddr, pConfig->curr.lookupTable);

    if (host)
    {
        /* Set the policy values for Frag & Stream if not already set */
        //TODO: SetTargetBasedPolicy(host);
    }

    return host;
}

HostAttributeEntry *SFAT_LookupHostEntryBySrc(Packet *p)
{
    if (!p || !p->iph_api)
        return NULL;

    return SFAT_LookupHostEntryByIP(GET_SRC_IP(p));
}

HostAttributeEntry *SFAT_LookupHostEntryByDst(Packet *p)
{
    if (!p || !p->iph_api)
        return NULL;

    return SFAT_LookupHostEntryByIP(GET_DST_IP(p));
}

static GetPolicyIdFunc updatePolicyCallback;
static GetPolicyIdsCallbackList *updatePolicyCallbackList = NULL;
void SFAT_SetPolicyCallback(void *host_attr_ent)
{
    HostAttributeEntry *host_entry = (HostAttributeEntry*)host_attr_ent;

    if (!host_entry)
        return;

    updatePolicyCallback(host_entry);

    return;
}

void SFAT_SetPolicyIds(GetPolicyIdFunc policyCallback, int snortPolicyId)
{
    tTargetBasedPolicyConfig *pConfig = NULL;
    GetPolicyIdsCallbackList *list_entry, *new_list_entry = NULL;

    pConfig = &targetBasedPolicyConfig;

    updatePolicyCallback = policyCallback;

    sfrt_iterate(pConfig->curr.lookupTable, SFAT_SetPolicyCallback);

    if (!updatePolicyCallbackList)
    {
        /* No list present, so no attribute table... bye-bye */
        return;
    }

    /* Look for this callback in the list */
    list_entry = updatePolicyCallbackList;
    while (list_entry)
    {
        if (list_entry->policyCallback == policyCallback)
            return; /* We're done with this one */

        if (list_entry->next)
        {
            list_entry = list_entry->next;
        }
        else
        {
            /* Leave list_entry pointint to last node in list */
            break;
        }
    }

    /* Wasn't there, add it so that when we reload the table,
     * we can set those policy entries on reload. */
    new_list_entry = (GetPolicyIdsCallbackList *)SnortAlloc(sizeof(GetPolicyIdsCallbackList));
    new_list_entry->policyCallback = policyCallback;
    if (list_entry)
    {
        /* list_entry is valid here since there was at least an
         * empty head entry in the list. */
        list_entry->next = new_list_entry;
    }
}

void SFAT_CleanupCallback(void *host_attr_ent)
{
    HostAttributeEntry *host_entry = (HostAttributeEntry*)host_attr_ent;
    FreeHostEntry(host_entry);
}

void SFAT_CleanPrevConfig(void)
{
    tTargetBasedPolicyConfig *pConfig = &targetBasedPolicyConfig;

    if (pConfig->prev.mapTable)
    {
        sfxhash_delete(pConfig->prev.mapTable);
	pConfig->prev.mapTable = NULL;
    }
    if (pConfig->prev.lookupTable)
    {
        sfrt_cleanup(pConfig->prev.lookupTable, SFAT_CleanupCallback);
        sfrt_free(pConfig->prev.lookupTable);
	pConfig->prev.lookupTable = NULL;
    }
    reload_attribute_table_flags = 0;
}

void SFAT_Cleanup(void)
{
    GetPolicyIdsCallbackList *list_entry, *tmp_list_entry = NULL;

    tTargetBasedPolicyConfig *pConfig = &targetBasedPolicyConfig;

    if (pConfig->curr.mapTable)
    {
        sfxhash_delete(pConfig->curr.mapTable);
	pConfig->curr.mapTable = NULL;
    }

    if (pConfig->prev.mapTable)
    {
        sfxhash_delete(pConfig->prev.mapTable);
	pConfig->prev.mapTable = NULL;
    }

    if (pConfig->next.mapTable)
    {
        sfxhash_delete(pConfig->next.mapTable);
	pConfig->next.mapTable = NULL;
    }

    if (pConfig->curr.lookupTable)
    {
        sfrt_cleanup(pConfig->curr.lookupTable, SFAT_CleanupCallback);
        sfrt_free(pConfig->curr.lookupTable);
	pConfig->curr.lookupTable = NULL;
    }

    if (pConfig->prev.lookupTable)
    {
        sfrt_cleanup(pConfig->prev.lookupTable, SFAT_CleanupCallback);
        sfrt_free(pConfig->prev.lookupTable);
	pConfig->prev.lookupTable = NULL;
    }

    if (pConfig->next.lookupTable)
    {
        sfrt_cleanup(pConfig->next.lookupTable, SFAT_CleanupCallback);
        sfrt_free(pConfig->next.lookupTable);
	pConfig->next.lookupTable = NULL;
    }

    if (sfat_saved_file)
    {
        free(sfat_saved_file);
        sfat_saved_file = NULL;
    }

    if (updatePolicyCallbackList)
    {
        list_entry = updatePolicyCallbackList;
        while (list_entry)
        {
            tmp_list_entry = list_entry->next;
            free(list_entry);
            list_entry = tmp_list_entry;
        }
        updatePolicyCallbackList = NULL;
    }

    DestroyBufferStack();
}

#define set_attribute_table_flag(flag) \
    reload_attribute_table_flags |= flag;
#define clear_attribute_table_flag(flag) \
    reload_attribute_table_flags &= ~flag;
#define check_attribute_table_flag(flag) \
    (reload_attribute_table_flags & flag)

void SigAttributeTableReloadHandler(int signal)
{
    /* If we're already reloading, don't do anything. */
    if (check_attribute_table_flag(ATTRIBUTE_TABLE_RELOADING_FLAG))
        return;

    /* Set flag to reload attribute table */
    set_attribute_table_flag(ATTRIBUTE_TABLE_RELOAD_FLAG);
}

void SFAT_VTAlrmHandler(int signal)
{
    /* Do nothing, just used to wake the sleeping dog... */
    return;
}

void *SFAT_ReloadAttributeTableThread(void *arg)
{
#ifndef WIN32
    sigset_t mtmask, oldmask;
    int ret;
    int reloads = 0;
    tTargetBasedPolicyConfig *pConfig = NULL;

    pConfig = &targetBasedPolicyConfig;

    sigemptyset(&mtmask);
    attribute_reload_thread_pid = gettid();

    /* Get the current set of signals inherited from main thread.*/
    pthread_sigmask(SIG_UNBLOCK, &mtmask, &oldmask);

    /* Now block those signals from being delivered to this thread.
     * now Main receives all signals. */
    pthread_sigmask(SIG_BLOCK, &oldmask, NULL);

    /* And allow SIGVTALRM through */
    signal (SIGVTALRM, SFAT_VTAlrmHandler);  if(errno!=0) errno=0;
    sigemptyset(&mtmask);
    sigaddset(&mtmask, SIGVTALRM);
    pthread_sigmask(SIG_UNBLOCK, &mtmask, NULL);

    attribute_reload_thread_running = 1;
    attribute_reload_thread_stop = 0;

    /* Checks the flag and terminates the attribute reload thread.
     *
     * Receipt of VTALRM signal pulls it out of the idle sleep (at
     * bottom of while().  Thread exits normally on next iteration
     * through its loop because stop flag is set.
     */
    while (!attribute_reload_thread_stop)
    {
#ifdef DEBUG_MSGS
        DebugMessage(DEBUG_ATTRIBUTE,
            "AttrReloadThread: Checking for new attr table...\n");
#endif
        ret = SFAT_ERROR;

        /* Is there an old table waiting to be cleaned up? */
        if (check_attribute_table_flag(ATTRIBUTE_TABLE_TAKEN_FLAG))
        {
            if (check_attribute_table_flag(ATTRIBUTE_TABLE_AVAILABLE_FLAG))
            {
#ifdef DEBUG_MSGS
                DebugMessage(DEBUG_ATTRIBUTE,
                    "AttrReloadThread: Freeing old attr table...\n");
#endif
                /* Free the map and attribute tables that are stored in
                 * prev.mapTable and prev.lookupTable */
                sfxhash_delete(pConfig->prev.mapTable);
                pConfig->prev.mapTable = NULL;

                sfrt_cleanup(pConfig->prev.lookupTable, SFAT_CleanupCallback);
                sfrt_free(pConfig->prev.lookupTable);
                pConfig->prev.lookupTable = NULL;
                clear_attribute_table_flag(ATTRIBUTE_TABLE_AVAILABLE_FLAG);
            }
            clear_attribute_table_flag(ATTRIBUTE_TABLE_PARSE_FAILED_FLAG);
            clear_attribute_table_flag(ATTRIBUTE_TABLE_TAKEN_FLAG);
            continue;
        }
        else if (check_attribute_table_flag(ATTRIBUTE_TABLE_RELOAD_FLAG) &&
                 !check_attribute_table_flag(ATTRIBUTE_TABLE_AVAILABLE_FLAG) &&
                 !check_attribute_table_flag(ATTRIBUTE_TABLE_PARSE_FAILED_FLAG))
        {
            /* Is there an new table ready? */
            set_attribute_table_flag(ATTRIBUTE_TABLE_RELOADING_FLAG);
#ifdef DEBUG_MSGS
            DebugMessage(DEBUG_ATTRIBUTE,
                "AttrReloadThread: loading new attr table.\n");
#endif
            reloads++;
            if (sfat_saved_file)
            {
                /* Initialize a new lookup table */
                if (!pConfig->next.lookupTable)
                {
                    /* Add 1 to max for table purposes
                     * We use max_hosts to limit memcap, assume 16k per entry costs*/
                    pConfig->next.lookupTable =
                        sfrt_new(DIR_8x16, IPv6, ScMaxAttrHosts(snort_conf) + 1,
                                ((ScMaxAttrHosts(snort_conf))>>6) + 1);
                    if (!pConfig->next.lookupTable)
                    {
                        SnortSnprintf(sfat_error_message, STD_BUF,
                            "Failed to initialize memory for new attribute table\n");
                        clear_attribute_table_flag(ATTRIBUTE_TABLE_RELOAD_FLAG);
                        clear_attribute_table_flag(ATTRIBUTE_TABLE_RELOADING_FLAG);
                        set_attribute_table_flag(ATTRIBUTE_TABLE_PARSE_FAILED_FLAG);
                        continue;
                    }
                }
                ret = ParseTargetMap(sfat_saved_file);
                if (ret == SFAT_OK)
                {
                    GetPolicyIdsCallbackList *list_entry = NULL;
                    /* Set flag that a new table is available.  Main
                     * process will check that flag and do the swap.
                     * After the new table is in use, the available
                     * flag should be cleared, the taken flag gets set
                     * and we'll go off and free the old one.
                     */

                    /* Set the policy IDs in the new table... */
                    list_entry = (GetPolicyIdsCallbackList *)arg;
                    while (list_entry)
                    {
                        if (list_entry->policyCallback)
                        {
                            sfrt_iterate(pConfig->next.lookupTable,
                                (sfrt_iterator_callback)list_entry->policyCallback);
                        }
                        list_entry = list_entry->next;
                    }

                    set_attribute_table_flag(ATTRIBUTE_TABLE_AVAILABLE_FLAG);
                }
                else
                {
                    /* Failed to parse, clean it up */
                    if (pConfig->next.mapTable)
                        sfxhash_delete(pConfig->next.mapTable);
                    pConfig->next.mapTable = NULL;

                    sfrt_cleanup(pConfig->next.lookupTable, SFAT_CleanupCallback);
                    sfrt_free(pConfig->next.lookupTable);
                    pConfig->next.lookupTable = NULL;

                    set_attribute_table_flag(ATTRIBUTE_TABLE_PARSE_FAILED_FLAG);
                }
            }
            clear_attribute_table_flag(ATTRIBUTE_TABLE_RELOAD_FLAG);
            clear_attribute_table_flag(ATTRIBUTE_TABLE_RELOADING_FLAG);
        }
        else
        {
            /* Sleep for 60 seconds */
#ifdef DEBUG_MSGS
            DebugMessage(DEBUG_ATTRIBUTE,
                "AttrReloadThread: Checked for new attr table... sleeping.\n");
#endif
            sleep(60);
        }
    }
#ifdef DEBUG_MSGS
    DebugMessage(DEBUG_ATTRIBUTE,
        "AttrReloadThread: exiting... Handled %d reloads\n", reloads);
#endif

    attribute_reload_thread_running = 0;
    pthread_exit(NULL);
#endif /* !Win32 */
    return NULL;
}

void AttributeTableReloadCheck(void)
{
    tTargetBasedPolicyConfig *pConfig = NULL;

    pConfig = &targetBasedPolicyConfig;

    if (check_attribute_table_flag(ATTRIBUTE_TABLE_TAKEN_FLAG))
    {
        return; /* Nothing to do, waiting for thread to clear this
                 * flag... */
    }
    /* Swap the attribute table pointers. */
    else if ((pConfig != NULL) &&
            check_attribute_table_flag(ATTRIBUTE_TABLE_AVAILABLE_FLAG))
    {
        LogMessage("Swapping Attribute Tables.\n");
        /***Do this on receipt of new packet ****/
        /***Avoids need for mutex****/
        pConfig->prev.lookupTable = pConfig->curr.lookupTable;
        pConfig->curr.lookupTable = pConfig->next.lookupTable;
        pConfig->next.lookupTable = NULL;

        pConfig->prev.mapTable = pConfig->curr.mapTable;
        pConfig->curr.mapTable = pConfig->next.mapTable;
        pConfig->next.mapTable = NULL;

        /* Set taken to indicate we've taken the new table */
        set_attribute_table_flag(ATTRIBUTE_TABLE_TAKEN_FLAG);

        sfBase.iAttributeHosts = SFAT_NumberOfHosts();
        sfBase.iAttributeReloads++;
        pc.attribute_table_reloads++;
    }
    else if (check_attribute_table_flag(ATTRIBUTE_TABLE_PARSE_FAILED_FLAG))
    {
        LogMessage("%s", sfat_error_message);
        /* Set taken to indicate we've taken the error message */
        set_attribute_table_flag(ATTRIBUTE_TABLE_TAKEN_FLAG);
    }
}

/**called once during initialization. Reads attribute table for the first time.*/
int SFAT_ParseAttributeTable(char *args, SnortConfig *sc)
{
    char **toks;
    int num_toks;
    int ret;
    tTargetBasedPolicyConfig *pConfig = &targetBasedPolicyConfig;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"AttributeTable\n"););


    /* Initialize lookup table */
    if (!pConfig->next.lookupTable)
    {
        /* Add 1 to max for table purposes
         * We use max_hosts to limit memcap, assume 16k per entry costs*/
        pConfig->next.lookupTable =
            sfrt_new(DIR_8x16, IPv6, ScMaxAttrHosts(sc) + 1,
                    ((ScMaxAttrHosts(sc))>>6)+ 1);
        if (!pConfig->next.lookupTable)
        {
            FatalError("Failed to initialize attribute table memory\n");
        }
    }

    /* Parse filename */
    toks = mSplit(args, " \t", 0, &num_toks, 0);

    if (num_toks != 2)
    {
        FatalError("%s(%d) ==> attribute_table must have 2 parameters\n",
                file_name, file_line);
    }

    if (!(strcasecmp(toks[0], "filename") == 0))
    {
        FatalError("%s(%d) ==> attribute_table must have 2 arguments, the 1st "
                "is 'filename'\n",
                file_name, file_line);
    }

    /* Reset... */
    sfat_insufficient_space_logged = 0;
    sfat_fatal_error = 1;

    ret = ParseTargetMap(toks[1]);

    if (ret == SFAT_OK)
    {
        pConfig->curr.lookupTable = pConfig->next.lookupTable;
        pConfig->next.lookupTable = NULL;
        pConfig->curr.mapTable = pConfig->next.mapTable;
        pConfig->next.mapTable = NULL;
        if (sfat_insufficient_space_logged)
            LogMessage("%s", sfat_error_message);
    }
    else
    {
        LogMessage("%s", sfat_error_message);
        if (sfat_fatal_error)
            FatalError("%s(%d) ==> failed to load attribute table from %s\n",
                file_name, file_line, toks[1]);
    }
    mSplitFree(&toks, num_toks);

    /* Create Thread to handle reparsing stuff... */
    sfBase.iAttributeHosts = SFAT_NumberOfHosts();
    LogMessage("Attribute Table Loaded with " STDu64 " hosts\n", sfBase.iAttributeHosts);

    /* Set up the head (empty) node in the policy callback list to
     * pass to thread.*/
    updatePolicyCallbackList = (GetPolicyIdsCallbackList *)SnortAlloc(sizeof(GetPolicyIdsCallbackList));

    return SFAT_OK;
}

void SFAT_StartReloadThread(void)
{
#ifndef WIN32

    LogMessage("Attribute Table Reload Thread Starting...\n");

    if (pthread_create(&attribute_reload_thread_id, NULL,
                       SFAT_ReloadAttributeTableThread, updatePolicyCallbackList))
    {
        FatalError("Failed to start thread to handle reloading attribute table\n");
    }

    while (!attribute_reload_thread_running)
        nanosleep(&thread_sleep, NULL);

    LogMessage("Attribute Table Reload Thread Started, thread %p (%u)\n",
               (void*)attribute_reload_thread_id, attribute_reload_thread_pid);
#endif
}

int IsAdaptiveConfigured( void )
{
    SnortConfig *sc = snort_conf;
    tSfPolicyId id = getDefaultPolicy( );

    if (id >= sc->num_policies_allocated)
    {
        ErrorMessage("%s(%d) Policy id is greater than the number of policies "
                     "allocated.\n", __FILE__, __LINE__);
        return 0;
    }

    if ((sc->targeted_policies[id] == NULL) ||
        (sc->targeted_policies[id]->target_based_config.args == NULL))
    {
        return 0;
    }

    return 1;
}

int IsAdaptiveConfiguredForSnortConfig(struct _SnortConfig *sc)
{
    tSfPolicyId id = getDefaultPolicy( );

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort conf for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    if (id >= sc->num_policies_allocated)
    {
        ErrorMessage("%s(%d) Policy id is greater than the number of policies "
                     "allocated.\n", __FILE__, __LINE__);
        return 0;
    }

    if ((sc->targeted_policies[id] == NULL) ||
        (sc->targeted_policies[id]->target_based_config.args == NULL))
    {
        return 0;
    }

    return 1;
}

void SFAT_UpdateApplicationProtocol(sfaddr_t *ipAddr, uint16_t port, uint16_t protocol, uint16_t id)
{
    HostAttributeEntry *host_entry;
    ApplicationEntry *service;
    tTargetBasedPolicyConfig *pConfig = &targetBasedPolicyConfig;
    unsigned service_count = 0;
    int rval;

    pConfig = &targetBasedPolicyConfig;

    host_entry = sfrt_lookup(ipAddr, pConfig->curr.lookupTable);

    if (!host_entry)
    {
        GetPolicyIdsCallbackList *list_entry;

        if (sfrt_num_entries(pConfig->curr.lookupTable) >= ScMaxAttrHosts(snort_conf))
            return;

        host_entry = SnortAlloc(sizeof(*host_entry));
        sfip_set_ip(&host_entry->ipAddr.addr, ipAddr);
        host_entry->ipAddr.bits = 128;
        if ((rval = sfrt_insert(&host_entry->ipAddr, (unsigned char)host_entry->ipAddr.bits, host_entry,
                                RT_FAVOR_SPECIFIC, pConfig->curr.lookupTable)) != RT_SUCCESS)
        {
            FreeHostEntry(host_entry);
            return;
        }
        for (list_entry = updatePolicyCallbackList; list_entry; list_entry = list_entry->next)
        {
            if (list_entry->policyCallback)
                list_entry->policyCallback(host_entry);
        }
        service = NULL;
    }
    else
    {
        for (service = host_entry->services; service; service = service->next)
        {
            if (service->ipproto == protocol && (uint16_t)service->port == port)
            {
                break;
            }
            service_count++;
        }
    }
    if (!service)
    {
        if ( service_count >= ScMaxAttrServicesPerHost() )
            return;

        service = SnortAlloc(sizeof(*service));
        service->port = port;
        service->ipproto = protocol;
        service->next = host_entry->services;
        host_entry->services = service;
        service->protocol = id;
    }
    else if (service->protocol != id)
    {
        service->protocol = id;
    }
}

#ifdef SNORT_RELOAD
void SFAT_ReloadCheck(SnortConfig *sc)
{
   /* Adaptive profile has changed from  OFF  ->  ON */
    if(!IsAdaptiveConfiguredForSnortConfig(snort_conf) && IsAdaptiveConfiguredForSnortConfig(sc))
    {
        tSfPolicyId defaultPolicyId = sfGetDefaultPolicy(sc->policy_config);
        TargetBasedConfig *tbc = &sc->targeted_policies[defaultPolicyId]->target_based_config;

        if (tbc && tbc->args)
        {
            SFAT_ParseAttributeTable(tbc->args, sc);
        }

        if(!(sc->run_flags & RUN_FLAG__DISABLE_ATTRIBUTE_RELOAD_THREAD))
            SFAT_StartReloadThread();
#ifdef REG_TEST
        if(REG_TEST_FLAG_RELOAD & getRegTestFlags())
        {
            printf("Adaptive profile enabled, started attribute reload thread.\n");
        }
#endif
    }
    /* Adaptive profile already ON, enabled profile updates */
    else if(IsAdaptiveConfiguredForSnortConfig(snort_conf) && IsAdaptiveConfiguredForSnortConfig(sc))
    {
        if((snort_conf->run_flags & RUN_FLAG__DISABLE_ATTRIBUTE_RELOAD_THREAD) &&
                !(sc->run_flags & RUN_FLAG__DISABLE_ATTRIBUTE_RELOAD_THREAD))
        {
            SFAT_StartReloadThread();
        }
    }
}

void ReloadAttributeThreadStop(void)
{
    // Stop the attribute reload thread
    if (attribute_reload_thread_running)
    {
        /* Doing same stuff that is done in SnortCleanup()
         */
        LogMessage("Terminating attribute reload thread\n");
        attribute_reload_thread_stop = 1;
        pthread_kill(attribute_reload_thread_id, SIGVTALRM);
        pthread_join(attribute_reload_thread_id, NULL);
    }
}
#endif

#endif /* TARGET_BASED */

