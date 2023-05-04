/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2004-2013 Sourcefire, Inc.
** Copyright (C) 2001-2004 Jeff Nathan <jeff@snort.org>
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

/* Snort ARPspoof Preprocessor Plugin
 *   by Jeff Nathan <jeff@snort.org>
 *   Version 0.1.4
 *
 * Purpose:
 *
 * This preprocessor looks for anomalies in ARP traffic and attempts to
 * maliciously overwrite  ARP cache information on hosts.
 *
 * Arguments:
 *
 * To check for unicast ARP requests use:
 * arpspoof: -unicast
 *
 * WARNING: this can generate false positives as Linux systems send unicast
 * ARP requests repetatively for entries in their cache.
 *
 * This plugin also takes a list of IP addresses and MAC address in the form:
 * arpspoof_detect_host: 10.10.10.10 29:a2:9a:29:a2:9a
 * arpspoof_detect_host: 192.168.40.1 f0:0f:00:f0:0f:00
 * and so forth...
 *
 * Effect:
 * By comparing information in the Ethernet header to the ARP frame, obvious
 * anomalies are detected.  Also, utilizing a user supplied list of IP
 * addresses and MAC addresses, ARP traffic appearing to have originated from
 * any IP in that list is carefully examined by comparing the source hardware
 * address to the user supplied hardware address.  If there is a mismatch, an
 * alert is generated as either an ARP request or REPLY can be used to
 * overwrite cache information on a remote host.  This should only be used for
 * hosts/devices on the **same layer 2 segment** !!
 *
 * Bugs:
 * This is a proof of concept ONLY.  It is clearly not complete.  Also, the
 * lookup function LookupIPMacEntryByIP is in need of optimization.  The
 * arpspoof_detect_host functionality may false alarm in redundant environments. * Also, see the comment above pertaining to Linux systems.
 *
 * Thanks:
 *
 * First and foremost Patrick Mullen who sat beside me and helped every step of
 * the way.  Andrew Baker for graciously supplying the tougher parts of this
 * code.  W. Richard Stevens for readable documentation and finally
 * Marty for being a badass.  All your packets are belong to Marty.
 *
 */

/*  I N C L U D E S  ************************************************/
#include <assert.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef WIN32
# include <sys/time.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#else
# include <time.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "generators.h"
#include "log.h"
#include "detect.h"
#include "decode.h"
#include "event.h"
#include "plugbase.h"
#include "parser.h"
#include "mstring.h"
#include "snort_debug.h"
#include "util.h"
#include "event_queue.h"

#include "snort.h"
#include "profiler.h"
#include "sfPolicy.h"
#include "session_api.h"

/*  D E F I N E S  **************************************************/
#define MODNAME "spp_arpspoof"
#define WITHUNICAST "-unicast"


/*  D A T A   S T R U C T U R E S  **********************************/
typedef struct _IPMacEntry
{
    uint32_t ipv4_addr;
    uint8_t  mac_addr[6];
    uint8_t  pad[2];
} IPMacEntry;

typedef struct _IPMacEntryListNode
{
    IPMacEntry *ip_mac_entry;
    struct _IPMacEntryListNode *next;
} IPMacEntryListNode;

typedef struct _IPMacEntryList
{
    int size;
    IPMacEntryListNode *head;
    IPMacEntryListNode *tail;
} IPMacEntryList;

typedef struct _ArpSpoofConfig
{
    int check_unicast_arp;
    int check_overwrite;
    IPMacEntryList *ipmel;

} ArpSpoofConfig;


/*  G L O B A L S  **************************************************/
static tSfPolicyUserContextId arp_spoof_config = NULL;

static const uint8_t bcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#ifdef PERF_PROFILING
PreprocStats arpPerfStats;
#endif


/*  P R O T O T Y P E S  ********************************************/
static void ARPspoofInit(struct _SnortConfig *, char *args);
static void ARPspoofHostInit(struct _SnortConfig *, char *args);
static void ParseARPspoofArgs(ArpSpoofConfig *, char *);
static void ParseARPspoofHostArgs(IPMacEntryList *, char *);
static void DetectARPattacks(Packet *p, void *context);
static void ARPspoofCleanExit(int signal, void *unused);
static void FreeIPMacEntryList(IPMacEntryList *ip_mac_entry_list);
static int AddIPMacEntryToList(IPMacEntryList *ip_mac_entry_list,
                               IPMacEntry *ip_mac_entry);
static IPMacEntry *LookupIPMacEntryByIP(IPMacEntryList *ip_mac_entry_list,
                                        uint32_t ipv4_addr);
static void ArpSpoofFreeConfig(tSfPolicyUserContextId config);

#ifdef DEBUG
static void PrintIPMacEntryList(IPMacEntryList *ip_mac_entry_list);
#endif

#ifdef SNORT_RELOAD
static void ARPspoofReload(struct _SnortConfig *, char *, void **);
static void ARPspoofReloadHost(struct _SnortConfig *, char *, void **);
static void * ARPspoofReloadSwap(struct _SnortConfig *, void *);
static void ARPspoofReloadSwapFree(void *);
#endif


void SetupARPspoof(void)
{
#ifndef SNORT_RELOAD
    RegisterPreprocessor("arpspoof", ARPspoofInit);
    RegisterPreprocessor("arpspoof_detect_host", ARPspoofHostInit);
#else
    RegisterPreprocessor("arpspoof", ARPspoofInit, ARPspoofReload, NULL,
                         ARPspoofReloadSwap, ARPspoofReloadSwapFree);
    RegisterPreprocessor("arpspoof_detect_host", ARPspoofHostInit,
                         ARPspoofReloadHost, NULL, NULL, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,
            "Preprocessor: ARPspoof is setup...\n"););
}


static void ARPspoofInit(struct _SnortConfig *sc, char *args)
{
    int policy_id = (int)getParserPolicy(sc);
    ArpSpoofConfig *pDefaultPolicyConfig = NULL;
    ArpSpoofConfig *pCurrentPolicyConfig = NULL;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,
            "Preprocessor: ARPspoof Initialized\n"););

    if (arp_spoof_config == NULL)
    {
       arp_spoof_config = sfPolicyConfigCreate();

#ifdef PERF_PROFILING
        RegisterPreprocessorProfile("arpspoof", &arpPerfStats, 0, &totalPerfStats, NULL);
#endif

        AddFuncToPreprocCleanExitList(ARPspoofCleanExit, NULL, PRIORITY_LAST, PP_ARPSPOOF);
    }

    sfPolicyUserPolicySet (arp_spoof_config, policy_id);
    pDefaultPolicyConfig = (ArpSpoofConfig *)sfPolicyUserDataGetDefault(arp_spoof_config);
    pCurrentPolicyConfig = (ArpSpoofConfig *)sfPolicyUserDataGetCurrent(arp_spoof_config);

    if ((policy_id != 0) && (pDefaultPolicyConfig == NULL))
    {
        ParseError("Arpspoof configuration: Must configure default policy "
                   "if other policies are to be configured.");
    }

    if (pCurrentPolicyConfig)
    {
        ParseError("Arpspoof can only be configured once.\n");
    }

    pCurrentPolicyConfig = (ArpSpoofConfig *)SnortAlloc(sizeof(ArpSpoofConfig));
    if (!pCurrentPolicyConfig)
    {
        ParseError("Arpspoof preprocessor: memory allocate failed.\n");
    }

    sfPolicyUserDataSetCurrent(arp_spoof_config, pCurrentPolicyConfig);
    /* Add arpspoof to the preprocessor function list */
    AddFuncToPreprocList(sc, DetectARPattacks, PRIORITY_NETWORK, PP_ARPSPOOF, PROTO_BIT__ARP);
    session_api->enable_preproc_all_ports( sc, PP_ARPSPOOF, PROTO_BIT__ARP );

    //policy independent configuration. First policy defines actual values.
    if (policy_id != 0)
    {
       pCurrentPolicyConfig->check_unicast_arp =
           ((ArpSpoofConfig *)sfPolicyUserDataGetDefault(arp_spoof_config))->check_unicast_arp;
        return;
    }

    /* Parse the arpspoof arguments from snort.conf */
    ParseARPspoofArgs(pCurrentPolicyConfig, args);
}


/**
 * Parse arguments passed to the arpspoof keyword.
 *
 * @param args preprocessor argument string
 *
 * @return void function
 */
static void ParseARPspoofArgs(ArpSpoofConfig *config, char *args)
{
    if ((config == NULL) || (args == NULL))
        return;

    if (strcasecmp(WITHUNICAST, args) == 0)
        config->check_unicast_arp = 1;
    else
        ParseError("Invalid option to arpspoof configuration");
}


static void ARPspoofHostInit(struct _SnortConfig *sc, char *args)
{
    tSfPolicyId policy_id = getParserPolicy(sc);
    ArpSpoofConfig *pPolicyConfig = NULL;

    if (arp_spoof_config == NULL)
    {
        ParseError("Please activate arpspoof before trying to "
                   "use arpspoof_detect_host.");
    }

    sfPolicyUserPolicySet(arp_spoof_config, policy_id);
    pPolicyConfig = (ArpSpoofConfig *)sfPolicyUserDataGetCurrent(arp_spoof_config);

    if (pPolicyConfig == NULL)
    {
        ParseError("Please activate arpspoof before trying to "
                   "use arpspoof_detect_host.");
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,
            "Preprocessor: ARPspoof (overwrite list) Initialized\n"););

    if (pPolicyConfig->ipmel == NULL)
    {
        pPolicyConfig->ipmel = (IPMacEntryList *)SnortAlloc(sizeof(IPMacEntryList));
    }

    /* Add MAC/IP pairs to ipmel */
    ParseARPspoofHostArgs(pPolicyConfig->ipmel, args);

    if (pPolicyConfig->check_overwrite == 0)
        pPolicyConfig->check_overwrite = 1;
}


/**
 * Parse arguments passed to the arpspoof_detect_host keyword.
 *
 * @param args preprocessor argument string
 *
 * @return void function
 */
static void ParseARPspoofHostArgs(IPMacEntryList *ipmel, char *args)
{
    char **toks;
    char **macbytes;
    int num_toks, num_macbytes;
    int i;
    struct in_addr IP_struct;
    IPMacEntry *ipme = NULL;

    if (ipmel == NULL)
        return;

    toks = mSplit(args, " ", 0, &num_toks, '\\');

    if (num_toks != 2)
        ParseError("Invalid arguments to arpspoof_detect_host.");

    /* Add entries */
    ipme = (IPMacEntry *)SnortAlloc(sizeof(IPMacEntry));

    if ((IP_struct.s_addr = inet_addr(toks[0])) == INADDR_NONE)
    {
        ParseError("Invalid IP address as first argument of "
                   "IP/MAC pair to arpspoof_detect_host.");
    }

    ipme->ipv4_addr = (uint32_t)IP_struct.s_addr;

    macbytes = mSplit(toks[1], ":", 6, &num_macbytes, '\\');

    if (num_macbytes < 6)
    {
        ParseError("Invalid MAC address as second argument of IP/MAC "
                   "pair to arpspoof_detect_host.");
    }
    else
    {
        for (i = 0; i < 6; i++)
            ipme->mac_addr[i] = (uint8_t) strtoul(macbytes[i], NULL, 16);
    }

    AddIPMacEntryToList(ipmel, ipme);

    mSplitFree(&toks, num_toks);
    mSplitFree(&macbytes, num_macbytes);

#if defined(DEBUG)
    PrintIPMacEntryList(ipmel);
#endif
}


/**
 * Detect ARP anomalies and overwrite attacks.
 *
 * @param p packet to detect anomalies and overwrite attacks on
 * @param context unused
 *
 * @return void function
 */
static void DetectARPattacks(Packet *p, void *context)
{
    IPMacEntry *ipme;
    ArpSpoofConfig *aconfig = NULL;
    const uint8_t *dst_mac_addr = NULL, *src_mac_addr = NULL;
    PROFILE_VARS;
    sfPolicyUserPolicySet (arp_spoof_config, getNapRuntimePolicy());
    aconfig = (ArpSpoofConfig *)sfPolicyUserDataGetCurrent(arp_spoof_config);
    uint32_t *arp_spa;

    /* is the packet valid? */
    if ( aconfig == NULL || p->ah == NULL)
        return;

    // preconditions - what we registered for
    if( p->eh != NULL)
    {
        src_mac_addr = p->eh->ether_src;
        dst_mac_addr = p->eh->ether_dst;
    }
#ifndef NO_NON_ETHER_DECODER
    else if( p->wifih != NULL )
    {
         if ((p->wifih->frame_control & WLAN_FLAG_TODS) &&
             (p->wifih->frame_control & WLAN_FLAG_FROMDS))
         {
             dst_mac_addr = p->wifih->addr3;
             src_mac_addr = p->wifih->addr4;
         }
         else if (p->wifih->frame_control & WLAN_FLAG_TODS)
         {
             src_mac_addr = p->wifih->addr2;
             dst_mac_addr = p->wifih->addr3;
         }
         else if (p->wifih->frame_control & WLAN_FLAG_FROMDS)
         {
             dst_mac_addr = p->wifih->addr1;
             src_mac_addr = p->wifih->addr3;
         }
         else
         {
             dst_mac_addr = p->wifih->addr1;
             src_mac_addr = p->wifih->addr2;
         }
    }
#endif
    else
    {
         return;
    }

    /* is the ARP protocol type IP and the ARP hardware type Ethernet? */
    if ((ntohs(p->ah->ea_hdr.ar_hrd) != 0x0001) ||
            (ntohs(p->ah->ea_hdr.ar_pro) != ETHERNET_TYPE_IP))
        return;

    PREPROC_PROFILE_START(arpPerfStats);

    switch(ntohs(p->ah->ea_hdr.ar_op))
    {
        case ARPOP_REQUEST:
            if (aconfig->check_unicast_arp)
            {
                if (memcmp((u_char *)dst_mac_addr, (u_char *)bcast, 6) != 0)
                {
                    SnortEventqAdd(GENERATOR_SPP_ARPSPOOF,
                            ARPSPOOF_UNICAST_ARP_REQUEST, 1, 0, 3,
                            ARPSPOOF_UNICAST_ARP_REQUEST_STR, 0);

                    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                            "MODNAME: Unicast request\n"););
                }
            }
            else if (memcmp((u_char *)src_mac_addr,
                    (u_char *)p->ah->arp_sha, 6) != 0)
            {
                SnortEventqAdd(GENERATOR_SPP_ARPSPOOF,
                        ARPSPOOF_ETHERFRAME_ARP_MISMATCH_SRC, 1, 0, 3,
                        ARPSPOOF_ETHERFRAME_ARP_MISMATCH_SRC_STR, 0);

                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                            "MODNAME: Ethernet/ARP mismatch request\n"););
            }
            break;
        case ARPOP_REPLY:
            if (memcmp((u_char *)src_mac_addr,
                    (u_char *)p->ah->arp_sha, 6) != 0)
            {
                SnortEventqAdd(GENERATOR_SPP_ARPSPOOF,
                        ARPSPOOF_ETHERFRAME_ARP_MISMATCH_SRC, 1, 0, 3,
                        ARPSPOOF_ETHERFRAME_ARP_MISMATCH_SRC_STR, 0);

                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                        "MODNAME: Ethernet/ARP mismatch reply src\n"););
            }
            else if (memcmp((u_char *)dst_mac_addr,
                    (u_char *)p->ah->arp_tha, 6) != 0)
            {
                SnortEventqAdd(GENERATOR_SPP_ARPSPOOF,
                        ARPSPOOF_ETHERFRAME_ARP_MISMATCH_DST, 1, 0, 3,
                        ARPSPOOF_ETHERFRAME_ARP_MISMATCH_DST_STR, 0);

                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                        "MODNAME: Ethernet/ARP mismatch reply dst\n"););
            }
            break;
    }
    PREPROC_PROFILE_END(arpPerfStats);

    /* return if the overwrite list hasn't been initialized */
    if (!aconfig->check_overwrite)
        return;

    /* LookupIPMacEntryByIP() is too slow, will be fixed later */
    arp_spa = (uint32_t *)&p->ah->arp_spa[0];
    if ((ipme = LookupIPMacEntryByIP(aconfig->ipmel,
                                     *arp_spa)) == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                "MODNAME: LookupIPMacEntryByIp returned NULL\n"););
        return;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                "MODNAME: LookupIPMacEntryByIP returned %p\n", ipme););

        /* If the Ethernet source address or the ARP source hardware address
         * in p doesn't match the MAC address in ipme, then generate an alert
         */
        if ((memcmp((uint8_t *)src_mac_addr,
                (uint8_t *)ipme->mac_addr, 6)) ||
                (memcmp((uint8_t *)p->ah->arp_sha,
                (uint8_t *)ipme->mac_addr, 6)))
        {
            SnortEventqAdd(GENERATOR_SPP_ARPSPOOF,
                    ARPSPOOF_ARP_CACHE_OVERWRITE_ATTACK, 1, 0, 3,
                    ARPSPOOF_ARP_CACHE_OVERWRITE_ATTACK_STR, 0);

            DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                    "MODNAME: Attempted ARP cache overwrite attack\n"););

            return;
        }
    }
}


/**
 * Add IP/MAC pair to a linked list.
 *
 * @param ip_mac_entry_list pointer to the list structure
 * @param ip_mac_entry linked list structure node
 *
 * @return 0 if the node is added successfully, 1 otherwise
 */
static int AddIPMacEntryToList(IPMacEntryList *ip_mac_entry_list,
                               IPMacEntry *ip_mac_entry)
{
    IPMacEntryListNode *newNode;

    if (ip_mac_entry == NULL || ip_mac_entry_list == NULL)
        return 1;

    newNode = (IPMacEntryListNode *)SnortAlloc(sizeof(IPMacEntryListNode));
    newNode->ip_mac_entry = ip_mac_entry;
    newNode->next = NULL;

    if (ip_mac_entry_list->head == NULL)
    {
        ip_mac_entry_list->head = newNode;
        ip_mac_entry_list->size = 1;
    }
    else
    {
        ip_mac_entry_list->tail->next = newNode;
        ip_mac_entry_list->size += 1;
    }
    ip_mac_entry_list->tail = newNode;
    return 0;
}


/**
 * Locate a linked list structure node by an IP address.
 *
 * @param ip_mac_entry_list pointer to the list structure
 * @param ipv4_addr IPv4 address as an unsigned 32-bit integer
 *
 * @return pointer to a structure node if a match is found, NULL otherwise
 */
static IPMacEntry *LookupIPMacEntryByIP(IPMacEntryList *ip_mac_entry_list,
                                        uint32_t ipv4_addr)
{
    IPMacEntryListNode *current;
#if defined(DEBUG)
    char *cha, *chb;
    sfaddr_t ina, inb;
#endif

    if (ip_mac_entry_list == NULL)
        return NULL;

    for (current = ip_mac_entry_list->head; current != NULL;
            current = current->next)
    {
#if defined(DEBUG)
        sfip_set_raw(&ina, &ipv4_addr, AF_INET);
        sfip_set_raw(&inb, &current->ip_mac_entry->ipv4_addr, AF_INET);
        cha = strdup(inet_ntoa(IP_ARG(ina)));
        chb = strdup(inet_ntoa(IP_ARG(inb)));

        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
            "MODNAME: LookupIPMacEntryByIP() comparing %s to %s\n", cha, chb););
        free(cha);
        free(chb);
#endif
        if (current->ip_mac_entry->ipv4_addr == ipv4_addr)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,
                    "MODNAME: LookupIPMecEntryByIP() match!"););

            return current->ip_mac_entry;
        }
    }
    return NULL;
}


/**
 * Free the linked list of IP/MAC address pairs
 *
 * @param ip_mac_entry_list pointer to the list structure
 *
 * @return void function
 */
static void FreeIPMacEntryList(IPMacEntryList *ip_mac_entry_list)
{
    IPMacEntryListNode *prev;
    IPMacEntryListNode *current;

    if (ip_mac_entry_list == NULL)
        return;

    current = ip_mac_entry_list->head;
    while (current != NULL)
    {
        if (current->ip_mac_entry != NULL)
            free(current->ip_mac_entry);

        prev = current;
        current = current->next;
        free(prev);
    }
    ip_mac_entry_list->head = NULL;
    ip_mac_entry_list->size = 0;

    return;
}


static void ARPspoofCleanExit(int signal, void *unused)
{
    ArpSpoofFreeConfig(arp_spoof_config);
    arp_spoof_config = NULL;
}

static int ArpSpoofFreeConfigPolicy(tSfPolicyUserContextId config,tSfPolicyId policyId, void* pData )
{
    ArpSpoofConfig *pPolicyConfig = (ArpSpoofConfig *)pData;
    if(pPolicyConfig->ipmel != NULL)
    {
         FreeIPMacEntryList(pPolicyConfig->ipmel);
         free(pPolicyConfig->ipmel);
    }
    sfPolicyUserDataClear (config, policyId);
    free(pPolicyConfig);
    return 0;
}

static void ArpSpoofFreeConfig(tSfPolicyUserContextId config)
{

    if (config == NULL)
        return;

    sfPolicyUserDataFreeIterate (config, ArpSpoofFreeConfigPolicy);
    sfPolicyConfigDelete(config);

}


#ifdef DEBUG
/**
 * Print the overwrite list for debugging purposes
 *
 * @param ip_mac_entry_list pointer to the list structure
 *
 * @return void function
 */
static void PrintIPMacEntryList(IPMacEntryList *ip_mac_entry_list)
{
    IPMacEntryListNode *current;
    int i;
    sfaddr_t in;

    if (ip_mac_entry_list == NULL)
        return;

    current = ip_mac_entry_list->head;
    printf("Arpspoof IPMacEntry List");
    printf("  Size: %i\n", ip_mac_entry_list->size);
    while (current != NULL)
    {
        sfip_set_raw(&in, &current->ip_mac_entry->ipv4_addr, AF_INET);
        printf("%s -> ", inet_ntoa(IP_ARG(in)));
        for (i = 0; i < 6; i++)
        {
            printf("%02x", current->ip_mac_entry->mac_addr[i]);
            if (i != 5)
                printf(":");
        }
        printf("\n");
        current = current->next;
    }
    return;
}
#endif

#ifdef SNORT_RELOAD
static void ARPspoofReload(struct _SnortConfig *sc, char *args, void **new_config)
{
    tSfPolicyUserContextId arp_spoof_swap_config = (tSfPolicyUserContextId)*new_config;
    int policy_id = (int)getParserPolicy(sc);
    ArpSpoofConfig *pPolicyConfig;

    if (!arp_spoof_swap_config)
    {
        arp_spoof_swap_config = sfPolicyConfigCreate();
        *new_config = (void *)arp_spoof_swap_config;
    }

    sfPolicyUserPolicySet (arp_spoof_swap_config, policy_id);

    pPolicyConfig = (ArpSpoofConfig *)sfPolicyUserDataGetCurrent(arp_spoof_swap_config);
    if (pPolicyConfig)
    {
        FatalError("Arpspoof can only be configured once.\n");
    }

    pPolicyConfig = (ArpSpoofConfig *)SnortAlloc(sizeof(ArpSpoofConfig));
    if (!pPolicyConfig)
    {
        ParseError("ARPSPOOF preprocessor: memory allocate failed.\n");
    }
     sfPolicyUserDataSetCurrent(arp_spoof_swap_config, pPolicyConfig);


    /* Add arpspoof to the preprocessor function list */
    AddFuncToPreprocList(sc, DetectARPattacks, PRIORITY_NETWORK, PP_ARPSPOOF, PROTO_BIT__ARP);

    //policy independent configuration. First policy defines actual values.
    if (policy_id != 0)
    {
       pPolicyConfig->check_unicast_arp = ((ArpSpoofConfig *)sfPolicyUserDataGetDefault(arp_spoof_swap_config))->check_unicast_arp;
        return;
    }

    /* Parse the arpspoof arguments from snort.conf */
    ParseARPspoofArgs(pPolicyConfig, args);

}

static void ARPspoofReloadHost(struct _SnortConfig *sc, char *args, void **new_config)
{
    int policy_id = (int)getParserPolicy(sc);
    ArpSpoofConfig *pPolicyConfig = NULL;
    tSfPolicyUserContextId arp_spoof_swap_config;

    arp_spoof_swap_config = (tSfPolicyUserContextId)GetRelatedReloadData(sc, "arpspoof");

    if ((arp_spoof_swap_config == NULL) ||
        (pPolicyConfig == NULL))
    {
        ParseError("Please activate arpspoof before trying to "
                   "use arpspoof_detect_host.");
    }

    sfPolicyUserPolicySet(arp_spoof_swap_config, policy_id);
    pPolicyConfig = (ArpSpoofConfig *)sfPolicyUserDataGetCurrent(arp_spoof_swap_config);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,
            "Preprocessor: ARPspoof (overwrite list) Initialized\n"););

    if (pPolicyConfig->ipmel == NULL)
    {
        pPolicyConfig->ipmel =
            (IPMacEntryList *)SnortAlloc(sizeof(IPMacEntryList));
    }

    /* Add MAC/IP pairs to ipmel */
    ParseARPspoofHostArgs(pPolicyConfig->ipmel, args);

    if (pPolicyConfig->check_overwrite == 0)
        pPolicyConfig->check_overwrite = 1;
}

static void * ARPspoofReloadSwap(struct _SnortConfig *sc, void *swap_config)
{
    tSfPolicyUserContextId arp_spoof_swap_config = (tSfPolicyUserContextId)swap_config;
    tSfPolicyUserContextId old_config = arp_spoof_config;

    if (arp_spoof_swap_config == NULL)
        return NULL;

    arp_spoof_config = arp_spoof_swap_config;
    arp_spoof_swap_config = NULL;

    return (void *)old_config;
}

static void ARPspoofReloadSwapFree(void *data)
{
    if (data == NULL)
        return;

    ArpSpoofFreeConfig((tSfPolicyUserContextId)data);
}
#endif
