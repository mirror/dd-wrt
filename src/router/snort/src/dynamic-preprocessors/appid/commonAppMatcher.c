/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
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


#include <glob.h>

#include "commonAppMatcher.h"
#include "common_util.h"
#include "httpCommon.h"
#include "service_state.h"
#include "service_base.h"
#include "service_api.h"
#include "client_app_base.h"
#include "client_app_api.h"
#include "detector_base.h"
#include "detector_api.h"
#include "detector_http.h"
#include "detector_cip.h"
#include "service_ssl.h"
#include "luaDetectorApi.h"
#include "luaDetectorModule.h"
#include "fw_appid.h"
#include "hostPortAppCache.h"
#include "appInfoTable.h"
#include "appIdStats.h"
#include "appIdConfig.h"
#include "ip_funcs.h"
#include "sfutil.h"
#include "app_forecast.h"
#include "detector_dns.h"

unsigned appIdPolicyId;
tAppIdConfig appIdConfig;
tAppIdConfig        *pAppidActiveConfig;
tAppIdConfig        *pAppidPassiveConfig;
tAppidStaticConfig* appidStaticConfig;
uint32_t app_id_netmasks[33];

/*static const char * const MODULE_NAME = "AppMatcher"; */
#define MAX_DISPLAY_SIZE   65536

/*#define DEBUG_APP_COMMON*/

static void DisplayPortExclusionList(SF_LIST *pe_list, uint16_t port)
{
    const char *p;
    const char *p2;
    char inet_buffer[INET6_ADDRSTRLEN];
    char inet_buffer2[INET6_ADDRSTRLEN];
    PortExclusion *pe;

    if (!pe_list) return;

    for (pe = (PortExclusion *)sflist_first(pe_list);
            pe;
            pe = (PortExclusion *)sflist_next(pe_list))
    {
        p = inet_ntop(pe->family, &pe->ip, inet_buffer, sizeof(inet_buffer));
        p2 = inet_ntop(pe->family, &pe->netmask, inet_buffer2, sizeof(inet_buffer2));
        _dpd.logMsg("        %d on %s/%s\n", port, p ? p:"ERROR", p2 ? p2:"ERROR");
    }
}
static void DisplayConfig(tAppidStaticConfig* appidSC, tAppIdConfig *aic)
{
    unsigned i;
    int j;
    struct in_addr ia;
    char inet_buffer[INET6_ADDRSTRLEN];
    char inet_buffer2[INET6_ADDRSTRLEN];
    NSIPv6Addr six;
    const char *p;
    const char *p2;
    NetworkSet *net_list;

    if (appidSC->appid_thirdparty_dir)
        _dpd.logMsg("    3rd Party Dir: %s\n", appidSC->appid_thirdparty_dir);
    if (appidSC->tp_config_path)
        _dpd.logMsg("    3rd Party Conf: %s\n", appidSC->tp_config_path);

    net_list = aic->net_list;
    _dpd.logMsg("    Monitoring Networks for any zone:\n");
    for (i = 0; i < net_list->count; i++)
    {
        ia.s_addr = htonl(net_list->pnetwork[i]->range_min);
        p = inet_ntop(AF_INET, &ia, inet_buffer, sizeof(inet_buffer));
        ia.s_addr = htonl(net_list->pnetwork[i]->range_max);
        p2 = inet_ntop(AF_INET, &ia, inet_buffer2, sizeof(inet_buffer2));
        _dpd.logMsg("        %s%s-%s %04X\n", (net_list->pnetwork[i]->info.ip_not) ? "!":"", p ? p:"ERROR",
                    p2 ? p2:"ERROR", net_list->pnetwork[i]->info.type);
    }
    for (i = 0; i < net_list->count6; i++)
    {
        six = net_list->pnetwork6[i]->range_min;
        NSIPv6AddrHtoN(&six);
        p = inet_ntop(AF_INET6, (struct in6_addr *)&six, inet_buffer, sizeof(inet_buffer));
        six = net_list->pnetwork6[i]->range_max;
        NSIPv6AddrHtoN(&six);
        p2 = inet_ntop(AF_INET6, (struct in6_addr *)&six, inet_buffer2, sizeof(inet_buffer2));
        _dpd.logMsg("        %s%s-%s %04X\n", (net_list->pnetwork6[i]->info.ip_not) ? "!":"", p ? p:"ERROR",
                    p2 ? p2:"ERROR", net_list->pnetwork6[i]->info.type);
    }

    for (j=0; j < MAX_ZONES; j++)
    {
        if (!(net_list = aic->net_list_by_zone[j]))
            continue;
        _dpd.logMsg("    Monitoring Networks for zone %d:\n", j);
        for (i = 0; i < net_list->count; i++)
        {
            ia.s_addr = htonl(net_list->pnetwork[i]->range_min);
            p = inet_ntop(AF_INET, &ia, inet_buffer, sizeof(inet_buffer));
            ia.s_addr = htonl(net_list->pnetwork[i]->range_max);
            p2 = inet_ntop(AF_INET, &ia, inet_buffer2, sizeof(inet_buffer2));
            _dpd.logMsg("        %s%s-%s %04X\n", (net_list->pnetwork[i]->info.ip_not) ? "!":"", p ? p:"ERROR",
                        p2 ? p2:"ERROR", net_list->pnetwork[i]->info.type);
        }
        for (i = 0; i < net_list->count6; i++)
        {
            six = net_list->pnetwork6[i]->range_min;
            NSIPv6AddrHtoN(&six);
            p = inet_ntop(AF_INET6, (struct in6_addr *)&six, inet_buffer, sizeof(inet_buffer));
            six = net_list->pnetwork6[i]->range_max;
            NSIPv6AddrHtoN(&six);
            p2 = inet_ntop(AF_INET6, (struct in6_addr *)&six, inet_buffer2, sizeof(inet_buffer2));
            _dpd.logMsg("        %s%s-%s %04X\n", (net_list->pnetwork6[i]->info.ip_not) ? "!":"", p ? p:"ERROR",
                        p2 ? p2:"ERROR", net_list->pnetwork6[i]->info.type);
        }
    }

    _dpd.logMsg("    Excluded TCP Ports for Src:\n");
    for (i = 0; i < APP_ID_PORT_ARRAY_SIZE; i++)
        DisplayPortExclusionList(aic->tcp_port_exclusions_src[i], i);

    _dpd.logMsg("    Excluded TCP Ports for Dst:\n");
    for (i = 0; i < APP_ID_PORT_ARRAY_SIZE; i++)
        DisplayPortExclusionList(aic->tcp_port_exclusions_dst[i], i);

    _dpd.logMsg("    Excluded UDP Ports Src:\n");
    for (i = 0; i < APP_ID_PORT_ARRAY_SIZE; i++)
        DisplayPortExclusionList(aic->udp_port_exclusions_src[i], i);

    _dpd.logMsg("    Excluded UDP Ports Dst:\n");
    for (i = 0; i < APP_ID_PORT_ARRAY_SIZE; i++)
        DisplayPortExclusionList(aic->udp_port_exclusions_dst[i], i);
}

#ifdef DEBUG_APP_COMMON
static void DisplayPortConfig(tAppIdConfig *aic)
{
    unsigned i;
    int first;

    first = 1;
    for (i=0; i<sizeof(aic->tcp_port_only)/sizeof(*aic->tcp_port_only); i++)
    {
        if (aic->tcp_port_only[i])
        {
            if (first)
            {
                _dpd.logMsg("    TCP Port-Only Services\n");
                first = 0;
            }
            _dpd.logMsg("        %5u - %u\n", i, aic->tcp_port_only[i]);
        }
    }

    first = 1;
    for (i=0; i<sizeof(aic->udp_port_only)/sizeof(*aic->udp_port_only); i++)
    {
        if (aic->udp_port_only[i])
        {
            if (first)
            {
                _dpd.logMsg("    UDP Port-Only Services\n");
                first = 0;
            }
            _dpd.logMsg("        %5u - %u\n", i, aic->udp_port_only[i]);
        }
    }
}
#endif

typedef struct _PORT
{
    struct _PORT *next;
    uint16_t port;
} Port;

static void ReadPortDetectors(tAppidStaticConfig* appidSC, tAppIdConfig *aic, const char *files)
{
    int rval;
    glob_t globs;
    char pattern[PATH_MAX];
    uint32_t n;

    snprintf(pattern, sizeof(pattern), "%s/%s", appidSC->app_id_detector_path, files);

    memset(&globs, 0, sizeof(globs));
    rval = glob(pattern, 0, NULL, &globs);
    if (rval != 0 && rval != GLOB_NOMATCH)
    {
        _dpd.errMsg("Unable to read directory '%s'\n",pattern);
        return;
    }

    for(n = 0; n < globs.gl_pathc; n++)
    {
        FILE *file;
        unsigned proto = 0;
        tAppId appId = APP_ID_NONE;
        char line[1024];
        Port *port = NULL;
        Port *tmp_port;

        if ((file = fopen(globs.gl_pathv[n], "r")) == NULL)
        {
            _dpd.errMsg("Unable to read port service '%s'\n",globs.gl_pathv[n]);
            continue;
        }

        while (fgets(line, sizeof(line), file))
        {
            char *key, *value, *p;
            size_t len;

            len = strlen(line);
            for (; len && (line[len - 1] == '\n' || line[len - 1] == '\r'); len--)
                line[len - 1] = 0;

            /* find key/value for lines of the format "key: value\n" */
            if ((value = strchr(line, ':')))
            {
                key = line;
                *value = '\0';
                value++;
                for (; *value && *value == ' '; value++);

                if (strcasecmp(key, "ports") == 0)
                {
                    char *context = NULL;
                    char *ptr;
                    unsigned long tmp;

                    for (ptr = strtok_r(value, ",", &context); ptr; ptr = strtok_r(NULL, ",", &context))
                    {
                        for (; *ptr && *ptr == ' '; ptr++);
                        len = strlen(ptr);
                        for (; len && ptr[len - 1] == ' '; len--)
                            ptr[len - 1] = 0;
                        tmp = strtoul(ptr, &p, 10);
                        if (!*ptr || *p || !tmp || tmp > 65535)
                        {
                            _dpd.errMsg("Invalid port, '%s', in lua detector '%s'\n",ptr, globs.gl_pathv[n]);
                            goto next;
                        }
                        if ((tmp_port = calloc(1, sizeof(*tmp_port))) == NULL)
                        {
                            _dpd.errMsg( "Failed to allocate a port struct");
                            goto next;
                        }
                        tmp_port->port = (uint16_t)tmp;
                        tmp_port->next = port;
                        port = tmp_port;
                    }
                }
                else if (strcasecmp(key, "protocol") == 0)
                {
                    if (strcasecmp(value, "tcp") == 0)
                        proto = 1;
                    else if (strcasecmp(value, "udp") == 0)
                        proto = 2;
                    else if (strcasecmp(value, "tcp/udp") == 0)
                        proto = 3;
                    else
                    {
                        _dpd.errMsg("Invalid protocol, '%s', in port service '%s'\n",value, globs.gl_pathv[n]);
                        goto next;
                    }
                }
                else if (strcasecmp(key, "appId") == 0)
                {
                    appId = (tAppId)strtoul(value, &p, 10);
                    if (!*value || *p || appId <= APP_ID_NONE)
                    {
                        _dpd.errMsg("Invalid app ID, '%s', in port service '%s'\n",value, globs.gl_pathv[n]);
                        goto next;
                    }
                }
            }
        }
        if (port && proto && appId > APP_ID_NONE)
        {
            while ((tmp_port = port))
            {
                port = tmp_port->next;
                if (proto & 1)
                    aic->tcp_port_only[tmp_port->port] = appId;
                if (proto & 2)
                    aic->udp_port_only[tmp_port->port] = appId;

                free(tmp_port);
                appInfoSetActive(appId, true);
            }
            appInfoSetActive(appId, true);
        }
        else
            _dpd.errMsg("Missing parameter(s) in port service '%s'\n",globs.gl_pathv[n]);

next:;
        while ((tmp_port = port))
        {
            port = tmp_port->next;
            free(tmp_port);
        }
        fclose(file);
    }

    globfree(&globs);
}

#define CISCO_PORT_DETECTORS "odp/port/*"
#define CUSTOM_PORT_DETECTORS "custom/port/*"

static void AppIdConfigureAnalyze(char *toklist[], uint32_t flag, tAppIdConfig *pConfig)
{
    int zone;
    NetworkSet *net_list;
    RNAIpAddrSet *ias;
    RNAIpv6AddrSet *ias6;
    char *p;
    long tmp;

    if (toklist[0])
    {
        if (strchr(toklist[0], ':'))
        {
            ias6 = ParseIpv6Cidr(toklist[0]);
            if (ias6)
            {
                NSIPv6Addr six;
                char min_ip[INET6_ADDRSTRLEN];
                char max_ip[INET6_ADDRSTRLEN];

                if (toklist[1])
                {
                    tmp = strtol(toklist[1], &p, 10);
                    if (!*toklist[1] || *p != 0 || tmp >= MAX_ZONES || tmp < -1)
                    {
                        _dpd.errMsg("Invalid Analyze: %s '%s'", toklist[0], toklist[1]);
                        zone = -1;
                    }
                    else
                        zone = (int)tmp;
                }
                else
                    zone = -1;
                ias6->addr_flags |= flag;
                six = ias6->range_min;
                NSIPv6AddrHtoN(&six);
                inet_ntop(AF_INET6, (struct in6_addr *)&six, min_ip, sizeof(min_ip));
                six = ias6->range_max;
                NSIPv6AddrHtoN(&six);
                inet_ntop(AF_INET6, (struct in6_addr *)&six, max_ip, sizeof(max_ip));
                _dpd.logMsg("Adding %s-%s (0x%08X) with zone %d\n", min_ip, max_ip,
                        ias6->addr_flags, zone);
                if (zone >= 0)
                {
                    if (!(net_list = pConfig->net_list_by_zone[zone]))
                    {
                        if (NetworkSet_New(&net_list))
                            _dpd.errMsg("%s", "Failed to create a network set");
                        else
                        {
                            net_list->next = pConfig->net_list_list;
                            pConfig->net_list_list = net_list;
                        }
                        pConfig->net_list_by_zone[zone] = net_list;
                    }
                }
                else
                    net_list = pConfig->net_list;
                if (net_list && NetworkSet_AddCidrBlock6Ex(net_list, &ias6->range_min, ias6->netmask,
                                                           ias6->addr_flags & IPFUNCS_EXCEPT_IP, 0,
                                                           ias6->addr_flags & (~IPFUNCS_EXCEPT_IP)))
                {
                    _dpd.errMsg("Failed to add an IP address set to the list of monitored networks");
                }
                free(ias6);
            }
            else
                _dpd.errMsg("Invalid analysis parameter: %s", toklist[0]);
        }
        else
        {
            ias = ParseIpCidr(toklist[0], app_id_netmasks);
            if (ias)
            {
                if (toklist[1])
                {
                    tmp = strtol(toklist[1], &p, 10);
                    if (!*toklist[1] || *p != 0 || tmp >= MAX_ZONES || tmp < -1)
                    {
                        _dpd.errMsg("Invalid Analyze: %s '%s'", toklist[0], toklist[1]);
                        zone = -1;
                    }
                    else
                        zone = (int)tmp;
                }
                else
                    zone = -1;
                ias->addr_flags |= flag;
                _dpd.logMsg("Adding 0x%08X-0x%08X (0x%08X) with zone %d\n", ias->range_min, ias->range_max,
                        ias->addr_flags, zone);
                if (zone >= 0)
                {
                    if (!(net_list = pConfig->net_list_by_zone[zone]))
                    {
                        if (NetworkSet_New(&net_list))
                            _dpd.errMsg("%s", "Failed to create a network set");
                        else
                        {
                            net_list->next = pConfig->net_list_list;
                            pConfig->net_list_list = net_list;
                        }
                        pConfig->net_list_by_zone[zone] = net_list;
                    }
                }
                else
                    net_list = pConfig->net_list;
                if (net_list && NetworkSet_AddCidrBlockEx(net_list, ias->range_min, ias->netmask,
                                                          ias->addr_flags & IPFUNCS_EXCEPT_IP, 0,
                                                          ias->addr_flags & (~IPFUNCS_EXCEPT_IP)))
                {
                    _dpd.errMsg("Failed to add an IP address set to the list of monitored networks");
                }
                free(ias);
            }
            else
                _dpd.errMsg("Invalid analysis parameter: %s", toklist[0]);
        }
    }
}

static sfaddr_t* AppIdConfigureDebug(char *toklist[])
{
    if (!toklist[0]) return NULL;

    struct in6_addr tmp;
    uint16_t family;

    if (!strchr(toklist[0], ':'))
    {
        if (inet_pton(AF_INET, toklist[0], &tmp) <= 0)
        {
            _dpd.errMsg("AppId Config: Failed to translate debug host %s", toklist[0]);
            return NULL;
        }
        family = AF_INET;
    }
    else
    {
        if (inet_pton(AF_INET6, toklist[0], &tmp) <= 0)
        {
            _dpd.errMsg("AppId Config: Failed to translate debug host %s", toklist[0]);
            return NULL;
        }
        family = AF_INET6;
    }

    sfaddr_t* ipAddr = malloc(sizeof(*ipAddr));
    if (!ipAddr)
    {
        _dpd.errMsg("AppId Config: Failed to allocate memory");
        return NULL;
    }

    ipAddr->family = family;

    if (ipAddr->family == AF_INET)
        copyIpv4ToIpv6Network(&ipAddr->ip, tmp.s6_addr32[0]);
    else
        memcpy(ipAddr, &tmp, sizeof(tmp));

    _dpd.logMsg("\nAppIdDebugHost: Debugging host IP %s\n", toklist[0]);

    return ipAddr;
}

static inline int AddPortExclusion(SF_LIST *port_exclusions[], const struct in6_addr *ip,
        const struct in6_addr *netmask, int family, uint16_t port)
{
    PortExclusion *port_ex;
    SF_LIST *pe_list;

    if (!(port_ex = calloc(1, sizeof(*port_ex))))
    {
        _dpd.errMsg("Config: Failed to allocate memory for port exclusion entry");
        return -1;
    }

    port_ex->ip = *ip;
    if (family == AF_INET)
    {
        port_ex->netmask.s6_addr32[0] = port_ex->netmask.s6_addr32[1] = port_ex->netmask.s6_addr32[2] = ~0;
        port_ex->netmask.s6_addr32[3] = netmask->s6_addr32[3];
    }
    else
        port_ex->netmask = *netmask;

    if ((pe_list = port_exclusions[port]) == NULL)
    {
        pe_list = port_exclusions[port] = sflist_new();
        if (pe_list == NULL)
        {
            free(port_ex);
            _dpd.errMsg("Config: Failed to allocate memory for port exclusion list");
            return -1;
        }
    }

    /* add this PortExclusion to the sflist for this port */
    if (sflist_add_tail(pe_list, port_ex) )
    {
        free(port_ex);
        _dpd.errMsg("Config: Failed to add an port exclusion to the list");
        return -1;
    }
    return 0;
}

static void ProcessPortExclusion(char *toklist[], tAppIdConfig *aic)
{
    int i = 1;
    char *p;
    RNAIpAddrSet *ias;
    RNAIpv6AddrSet *ias6;
    SF_LIST **port_exclusions;
    unsigned proto;
    unsigned long dir;
    unsigned long port;
    struct in6_addr ip;
    struct in6_addr netmask;
    int family;

    if (!toklist[i])
    {
        _dpd.errMsg("Config: Port exclusion direction omitted");
        return;
    }

    if (strcasecmp(toklist[i], "dst") == 0)
        dir = 2;
    else if (strcasecmp(toklist[i], "src") == 0)
        dir = 1;
    else if (strcasecmp(toklist[i], "both") == 0)
        dir = 3;
    else
    {
        _dpd.errMsg("Config: Invalid port exclusion direction specified");
        return;
    }

    i++;
    if (!toklist[i])
    {
        _dpd.errMsg("Config: Port exclusion protocol omitted");
        return;
    }

    if (strcasecmp(toklist[i], "tcp") == 0)
        proto = IPPROTO_TCP;
    else if (strcasecmp(toklist[i], "udp") == 0)
        proto = IPPROTO_UDP;
    else
    {
        _dpd.errMsg("Config: Invalid port exclusion protocol specified");
        return;
    }

    i++;
    if (!toklist[i])
    {
        _dpd.errMsg("Config: Port exclusion port omitted");
        return;
    }

    port = strtoul(toklist[i], &p, 10);
    if (!*toklist[i] || *p || port >= APP_ID_PORT_ARRAY_SIZE)
    {
        _dpd.errMsg("Config: Invalid port exclusion port specified");
        return;
    }

    i++;
    if (!toklist[i])
    {
        _dpd.errMsg("Config: Port exclusion address omitted");
        return;
    }

    if (strchr(toklist[i], ':'))
    {
        ias6 = ParseIpv6Cidr(toklist[i]);
        if (!ias6 || ias6->addr_flags)
        {
            if (ias6)
                free(ias6);
            _dpd.errMsg("Config: Invalid port exclusion address specified");
            return;
        }
        NSIPv6AddrHtoNConv(&ias6->range_min, &ip);
        NSIPv6AddrHtoNConv(&ias6->netmask_mask, &netmask);
        family = AF_INET6;
        free(ias6);
    }
    else
    {
        ias = ParseIpCidr(toklist[i], app_id_netmasks);
        if (!ias || ias->addr_flags)
        {
            if (ias)
                free(ias);
            _dpd.errMsg("Config: Invalid port exclusion address specified");
            return;
        }
        family = AF_INET;
        copyIpv4ToIpv6Network(&ip, htonl(ias->range_min));
        copyIpv4ToIpv6Network(&netmask, htonl(ias->netmask_mask));
        free(ias);
    }

    if (dir & 1)
    {
        if (proto == IPPROTO_TCP)
            port_exclusions = aic->tcp_port_exclusions_src;
        else
            port_exclusions = aic->udp_port_exclusions_src;
        AddPortExclusion(port_exclusions, &ip, &netmask, family, (uint16_t)port);
    }
    if (dir & 2)
    {
        if (proto == IPPROTO_TCP)
            port_exclusions = aic->tcp_port_exclusions_dst;
        else
            port_exclusions = aic->udp_port_exclusions_dst;
        AddPortExclusion(port_exclusions, &ip, &netmask, family, (uint16_t)port);
    }
}

static void ProcessConfigDirective(char *toklist[], tAppIdConfig *aic, int reload)
{
    char *curtok;
    int i;

    /* the first tok is "config" or we wouldn't be here now */
    i = 1;
    curtok = toklist[i];
    i++;

    if (!strcasecmp(curtok, "Analyze"))
    {
        AppIdConfigureAnalyze(&toklist[i], IPFUNCS_HOSTS_IP | IPFUNCS_APPLICATION, aic);
    }
    else if (!strcasecmp(curtok, "AnalyzeHost"))
    {
        AppIdConfigureAnalyze(&toklist[i], IPFUNCS_HOSTS_IP | IPFUNCS_APPLICATION, aic);
    }
    else if (!strcasecmp(curtok, "AnalyzeUser"))
    {
        AppIdConfigureAnalyze(&toklist[i], IPFUNCS_USER_IP | IPFUNCS_APPLICATION, aic);
    }
    else if (!strcasecmp(curtok, "AnalyzeHostUser"))
    {
        AppIdConfigureAnalyze(&toklist[i], IPFUNCS_HOSTS_IP | IPFUNCS_USER_IP | IPFUNCS_APPLICATION, aic);
    }
    else if (!strcasecmp(curtok, "AnalyzeApplication"))
    {
        AppIdConfigureAnalyze(&toklist[i], IPFUNCS_APPLICATION, aic);
    }
    else if (!strcasecmp(curtok, "DebugHost"))
    {
        aic->debugHostIp = AppIdConfigureDebug(&toklist[i]);
    }
}

static int AppIdLoadConfigFile(tAppidStaticConfig* appidSC, int reload, int instance_id, tAppIdConfig *pConfig)
{
    FILE *fp;
    char linebuffer[MAX_LINE];
    char *cptr;
    char *toklist[MAX_TOKS];
    int num_toks;
    unsigned line = 0;
    NetworkSet *net_list;

    if (NetworkSet_New(&pConfig->net_list))
    {
        _dpd.fatalMsg("Failed to allocate a network set");
        exit(1);
    }
    pConfig->net_list_list = pConfig->net_list;

    if (!appidSC->conf_file || (!appidSC->conf_file[0]))
    {
        char addrString[sizeof("0.0.0.0/0")];
        _dpd.logMsg("Defaulting to monitoring all Snort traffic for AppID.\n");
        toklist[1] = NULL;
        toklist[0] = addrString;
        strcpy(addrString,"0.0.0.0/0");
        AppIdConfigureAnalyze(toklist, IPFUNCS_HOSTS_IP | IPFUNCS_USER_IP | IPFUNCS_APPLICATION, pConfig);
        strcpy(addrString,"::/0");
        AppIdConfigureAnalyze(toklist, IPFUNCS_HOSTS_IP | IPFUNCS_USER_IP | IPFUNCS_APPLICATION, pConfig);
        toklist[0] = NULL;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_APPID, "Loading configuration file: %s", appidSC->conf_file););

        if (!(fp = fopen(appidSC->conf_file, "r")))
        {
            _dpd.errMsg("Unable to open %s", appidSC->conf_file);
            return -1;
        }

        while (fgets(linebuffer, MAX_LINE, fp) != NULL)
        {
            line++;

            strip(linebuffer);

            cptr = linebuffer;

            while (isspace((int)*cptr))
                cptr++;

            if (*cptr && (*cptr != '#') && (*cptr != 0x0a))
            {
                memset(toklist, 0, sizeof(toklist));
                /* tokenize the line */
                num_toks = Tokenize(cptr, toklist);
                if (num_toks < 2)
                {
                    fclose(fp);
                    _dpd.errMsg("Invalid configuration file line %u", line);
                    return -1;
                }
                if (!(strcasecmp(toklist[0], "config")))
                {
                    ProcessConfigDirective(toklist, pConfig, reload);
                }
                else if (!(strcasecmp(toklist[0], "portexclusion")))
                {
                    ProcessPortExclusion(toklist, pConfig);
                }
            }
        }

        fclose(fp);
    }

#define DEFAULT_THIRDPARTY_PATH "/usr/local/lib/thirdparty"
    if (!appidSC->appid_thirdparty_dir)
    {
        if (!(appidSC->appid_thirdparty_dir = strdup(DEFAULT_THIRDPARTY_PATH)))
        {
            _dpd.errMsg("Failed to allocate a module directory");
            return -1;
        }
    }

    if (instance_id)
    {
        char *instance_toklist[2];
        char addrString[sizeof("0.0.0.0/0")];
        _dpd.logMsg("Defaulting to monitoring all Snort traffic for AppID.\n");
        instance_toklist[0] = addrString;
        instance_toklist[1] = NULL;
        strcpy(addrString,"0.0.0.0/0");
        AppIdConfigureAnalyze(instance_toklist, IPFUNCS_APPLICATION, pConfig);
        strcpy(addrString,"::/0");
        AppIdConfigureAnalyze(instance_toklist, IPFUNCS_APPLICATION, pConfig);
    }

    for (net_list = pConfig->net_list_list; net_list; net_list = net_list->next)
    {
        if (net_list != pConfig->net_list)
        {
            if (NetworkSet_AddSet(net_list, pConfig->net_list))
                _dpd.errMsg("Failed to add any network list to a zone network list");
        }
    }
    pConfig->net_list_count = 0;
    for (net_list = pConfig->net_list_list; net_list; net_list = net_list->next)
    {
        if (NetworkSet_Reduce(net_list))
            _dpd.errMsg("Failed to reduce the IP address sets");
        pConfig->net_list_count += NetworkSet_CountEx(net_list) + NetworkSet_Count6Ex(net_list);
    }

    return 0;
}

static void ReadPorts(tAppidStaticConfig* appidSC, tAppIdConfig *aic)
{
    ReadPortDetectors(appidSC, aic, CISCO_PORT_DETECTORS);
    ReadPortDetectors(appidSC, aic, CUSTOM_PORT_DETECTORS);
}

static void LoadModules(uint32_t instance_id, tAppIdConfig *pConfig)
{
    if (LoadServiceModules(NULL, instance_id, pConfig))
        exit(-1);

    if (LoadClientAppModules(NULL, pConfig))
        exit(-1);

    if (LoadDetectorModules(NULL))
        exit(-1);
}

static void FinalizePatternModules(tAppIdConfig *pConfig)
{
    int                  i;
    tServicePatternData *curr;
    tServicePatternData *lists[] = { pConfig->serviceConfig.tcp_pattern_data,
                                     pConfig->serviceConfig.udp_pattern_data };
    for (i = 0; i < (sizeof(lists) / sizeof(*lists)); i++)
    {
        curr = lists[i];
        while (curr != NULL)
        {
            if (curr->svc != NULL)
            {
                bool isActive = true;
                if (curr->svc->userdata && !curr->svc->userdata->isActive)
                {
                    /* C detectors don't have userdata here, but they're always
                     * active.  So, this check is really just for Lua
                     * detectors. */
                    isActive = false;
                }
                if (isActive)
                {
                    curr->svc->current_ref_count = curr->svc->ref_count;
                }
            }
            curr = curr->next;
        }
    }
}

/**
 * \brief Reload C modules - both static and dynamic.
 *
 * This function is called during reload/reconfiguration. Nothing needs to be done for client
 * app modules. In addition to the C service modules, \e ReloadServiceModules() also takes care
 * of loading services associated with detectors.
 *
 * @param pConfig - AppId context in which the modules need to be reloaded
 * @return None
 */
static void ReloadModules(tAppIdConfig *pConfig)
{
    if (ReloadServiceModules(pConfig))
         exit(-1);
 }

typedef enum
{

    RNA_FW_CONFIG_STATE_UNINIT,
    RNA_FW_CONFIG_STATE_INIT,
    RNA_FW_CONFIG_STATE_PENDING,

} tRnaFwConfigState;

static tRnaFwConfigState rnaFwConfigState = RNA_FW_CONFIG_STATE_UNINIT;

static void ConfigItemFree(ConfigItem *ci)
{
    if (ci)
    {
        if (ci->name)
            free(ci->name);
        if (ci->value)
            free(ci->value);
        free(ci);
    }
}

static void AppIdCleanupConfig(tAppIdConfig *pConfig)
{
    NetworkSet  *net_list;         ///< list of network sets
    unsigned int i;
    while ((net_list = pConfig->net_list_list))
    {
        pConfig->net_list_list = net_list->next;
        NetworkSet_Destroy(net_list);
    }

    /* clean up any port exclusions that have been allocated */
    for( i=0; i<APP_ID_PORT_ARRAY_SIZE; i++ )
    {
        if( pConfig->tcp_port_exclusions_src[i] != NULL )
        {
            sflist_free_all(pConfig->tcp_port_exclusions_src[i], &free);
            pConfig->tcp_port_exclusions_src[i] = NULL;
        }
        if( pConfig->tcp_port_exclusions_dst[i] != NULL )
        {
            sflist_free_all(pConfig->tcp_port_exclusions_dst[i], &free);
            pConfig->tcp_port_exclusions_dst[i] = NULL;
        }
        if( pConfig->udp_port_exclusions_src[i] != NULL )
        {
            sflist_free_all(pConfig->udp_port_exclusions_src[i], &free);
            pConfig->udp_port_exclusions_src[i] = NULL;
        }
        if( pConfig->udp_port_exclusions_dst[i] != NULL )
        {
            sflist_free_all(pConfig->udp_port_exclusions_dst[i], &free);
            pConfig->udp_port_exclusions_dst[i] = NULL;
        }
    }

    pConfig->net_list = NULL;

    if (pConfig->CHP_glossary)
    {
        sfxhash_delete(pConfig->CHP_glossary);
        pConfig->CHP_glossary = NULL;
    }

    if (pConfig->AF_indicators)
    {
        sfxhash_delete(pConfig->AF_indicators);
        pConfig->AF_indicators = NULL;
    }

    if (pConfig->AF_actives)
    {
        sfxhash_delete(pConfig->AF_actives);
        pConfig->AF_actives = NULL;
    }

    memset(pConfig->net_list_by_zone, 0, sizeof(pConfig->net_list_by_zone));

    sflist_static_free_all(&pConfig->client_app_args, (void (*)(void*))ConfigItemFree);

    if (pConfig->debugHostIp)
    {
        free(pConfig->debugHostIp);
        pConfig->debugHostIp = NULL;
    }
}

static int initAFIndicators(tAppIdConfig *pConfig)
{
    if (!(pConfig->AF_indicators = sfxhash_new(1024,
                                               sizeof(tAppId),
                                               sizeof(AFElement),
                                               0,
                                               0,
                                               NULL,
                                               NULL,
                                               0)))
    {
        _dpd.errMsg("Config: failed to allocate memory for an sfxhash.");
        return 0;
    }
    else
        return 1;
}

static int initAFActives(tAppIdConfig *pConfig)
{
    if (!(pConfig->AF_actives = sfxhash_new(1024,
                                            sizeof(AFActKey),
                                            sizeof(AFActVal),
                                            (sizeof(SFXHASH_NODE)*2048),
                                            1,
                                            NULL,
                                            NULL,
                                            1)))
    {
        _dpd.errMsg("Config: failed to allocate memory for an sfxhash.");
        return 0;
    }
    else
        return 1;
}
static int genericDataFree (void *key, void *data)
{
    if (data)
        free(data);
    return 0;
}

static int initCHPGlossary (tAppIdConfig *pConfig)
{
    if (!(pConfig->CHP_glossary = sfxhash_new(1024,
                                              sizeof(tAppId),
                                              0, 0, 0, NULL,
                                              &genericDataFree, 0)))
    {
        _dpd.errMsg("Config: failed to allocate memory for an sfxhash.");
        return 0;
    }
    else
        return 1;
}

#if 0
if packet thread then
    active
else
    if RNA_FW_CONFIG_STATE_UNINIT
        then active
    else
        passive
#endif

int AppIdCommonInit(tAppidStaticConfig *appidSC)
{
    if (!(pAppidActiveConfig = (tAppIdConfig *)_dpd.snortAlloc(1,
        sizeof(*pAppidActiveConfig), PP_APP_ID, PP_MEM_CATEGORY_CONFIG)))
    {
        _dpd.errMsg("Config: Failed to allocate memory for AppIdConfig");
        return -1;
    }

    fwAppIdInit();

    if (rnaFwConfigState == RNA_FW_CONFIG_STATE_UNINIT)
    {
        appIdPolicyId = 53;
        pAppidPassiveConfig = pAppidActiveConfig;
        rnaFwConfigState = RNA_FW_CONFIG_STATE_PENDING;
        InitNetmasks(app_id_netmasks);
        sflist_init(&pAppidActiveConfig->client_app_args);
        AppIdLoadConfigFile(appidSC, 0, appidSC->instance_id, pAppidActiveConfig);
        if (!initCHPGlossary(pAppidActiveConfig))
            return -1;
        if (!initAFIndicators(pAppidActiveConfig))
            return -1;
        if (!initAFActives(pAppidActiveConfig))
            return -1;
        luaModuleInit();
        appInfoTableInit(appidSC, pAppidActiveConfig);
        ReadPorts(appidSC, pAppidActiveConfig);
        LoadModules(appidSC->instance_id, pAppidActiveConfig);
        hostPortAppCacheDynamicInit();
        hostPortAppCacheInit(pAppidActiveConfig);
        lengthAppCacheInit(pAppidActiveConfig);

        LoadLuaModules(appidSC, pAppidActiveConfig);
        ClientAppInit(appidSC, pAppidActiveConfig);
        ServiceInit(pAppidActiveConfig);
        FinalizeLuaModules(pAppidActiveConfig);

        FinalizePatternModules(pAppidActiveConfig);

        http_detector_finalize(pAppidActiveConfig);
        sipUaFinalize(&pAppidActiveConfig->detectorSipConfig);
        ssl_detector_process_patterns(&pAppidActiveConfig->serviceSslConfig);
        dns_host_detector_process_patterns(&pAppidActiveConfig->serviceDnsConfig);
        portPatternFinalize(pAppidActiveConfig);
        ClientAppFinalize(pAppidActiveConfig);
        ServiceFinalize(pAppidActiveConfig);
        appIdStatsInit(appidSC->app_stats_filename, appidSC->app_stats_period,
                appidSC->app_stats_rollover_size, appidSC->app_stats_rollover_time);
        DisplayConfig(appidSC, pAppidActiveConfig);
#ifdef DEBUG_APP_COMMON
        DisplayPortConfig(pAppidActiveConfig);
#endif
        if (AppIdServiceStateInit(appidSC->memcap))
        {
            _dpd.fatalMsg("AppID failed to create the service state cache with %lu memory\n",
                          appidSC->memcap); 
        }
        rnaFwConfigState = RNA_FW_CONFIG_STATE_INIT;
        return 0;
    }
    return -1;
}

int AppIdCommonFini(void)
{

    if (rnaFwConfigState == RNA_FW_CONFIG_STATE_INIT)
    {
        rnaFwConfigState = RNA_FW_CONFIG_STATE_PENDING;
        pAppidPassiveConfig = pAppidActiveConfig;
        ThirdPartyAppIDFini();

        AppIdCleanupConfig(pAppidActiveConfig);
        CleanupServices(pAppidActiveConfig);
        CleanupClientApp(pAppidActiveConfig);
        luaModuleFini();
        hostPortAppCacheDynamicFini();
        hostPortAppCacheFini(pAppidActiveConfig);
        AppIdServiceStateCleanup();
        appIdStatsFini();
        fwAppIdFini(pAppidActiveConfig);
        lengthAppCacheFini(pAppidActiveConfig);
        http_detector_clean(&pAppidActiveConfig->detectorHttpConfig);
        service_ssl_clean(&pAppidActiveConfig->serviceSslConfig);
        service_dns_host_clean(&pAppidActiveConfig->serviceDnsConfig);
        CipClean();
        rnaFwConfigState = RNA_FW_CONFIG_STATE_UNINIT;
        _dpd.snortFree(pAppidActiveConfig, sizeof(*pAppidActiveConfig),
                PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
        pAppidActiveConfig = NULL;
        pAppidPassiveConfig = NULL;
        return 0;
    }
    return -1;
}

int AppIdCommonReload(tAppidStaticConfig* appidSC, void **new_context)
{
    tAppIdConfig *pNewConfig = (tAppIdConfig *)_dpd.snortAlloc(1,
                      sizeof(*pNewConfig), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
    if (!pNewConfig)
    {
        _dpd.fatalMsg("AppID failed to allocate memory for reload AppIdConfig");
    }
    pAppidPassiveConfig = pNewConfig;

    // During a reload, C modules are not reloaded. Also, existing Lua modules are not reloaded.
    // New Lua modules that did not exist before the reload are loaded.
    // Data structures related to them such as service ports, patterns, etc. are cleaned up and
    // reloaded. A Lua module that does not exist after the reload is made inactive by not reloading
    // its data structures.

    // Following lists are all linear lists and new items are inserted at the head. During a reload,
    // only new Lua modules can be added to these lists. No items are removed from the lists. Since
    // items are inserted at the head, we just point the lists in the new AppId context to the head
    // of the lists in the current AppId context. If a new item is added to a list, the list in the
    // current context will be unchanged.
    pNewConfig->clientAppConfig.tcp_client_app_list = pAppidActiveConfig->clientAppConfig.tcp_client_app_list;
    pNewConfig->clientAppConfig.udp_client_app_list = pAppidActiveConfig->clientAppConfig.udp_client_app_list;
    pNewConfig->serviceConfig.active_service_list = pAppidActiveConfig->serviceConfig.active_service_list;
    pNewConfig->serviceConfig.tcp_service_list = pAppidActiveConfig->serviceConfig.tcp_service_list;
    pNewConfig->serviceConfig.udp_service_list = pAppidActiveConfig->serviceConfig.udp_service_list;
    pNewConfig->serviceConfig.udp_reversed_service_list = pAppidActiveConfig->serviceConfig.udp_reversed_service_list;

    sflist_init(&pNewConfig->client_app_args);
    AppIdLoadConfigFile(appidSC, 1, 0, pNewConfig);
    if (!initCHPGlossary(pNewConfig))
        return -1;
    if (!initAFIndicators(pNewConfig))
        return -1;
    if (!initAFActives(pNewConfig))
        return -1;
    sflist_init(&pNewConfig->genericConfigList);
    appInfoTableInit(appidSC, pNewConfig);
    ReadPorts(appidSC, pNewConfig);
    ReloadModules(pNewConfig);
    hostPortAppCacheInit(pNewConfig);
    lengthAppCacheInit(pNewConfig);

    LoadLuaModules(appidSC, pNewConfig);
    ClientAppInit(appidSC, pNewConfig);
    ReconfigureServices(pNewConfig);

    http_detector_finalize(pNewConfig);
    sipUaFinalize(&pNewConfig->detectorSipConfig);
    ssl_detector_process_patterns(&pNewConfig->serviceSslConfig);
    dns_host_detector_process_patterns(&pNewConfig->serviceDnsConfig);
    portPatternFinalize(pNewConfig);
    ClientAppFinalize(pNewConfig);
    ServiceFinalize(pNewConfig);

    appIdStatsReinit();
    DisplayConfig(appidSC, pNewConfig);
 #ifdef DEBUG_APP_COMMON
    DisplayPortConfig(pNewConfig);
 #endif

    pAppidPassiveConfig = NULL;
    *new_context = pNewConfig;

    return 0;
}

void *AppIdCommonReloadSwap(void *swap_config)
{
    tAppIdConfig *pAppidNewConfig = (tAppIdConfig *)swap_config;
    tAppIdConfig *pAppidOldConfig = NULL;

    pAppidPassiveConfig = pAppidNewConfig;
    FinalizeLuaModules(pAppidNewConfig);
    FinalizePatternModules(pAppidNewConfig);
    appIdPolicyId++;
    pAppidPassiveConfig = NULL;

    // Return old configuration data structure
    pAppidOldConfig = pAppidActiveConfig;
    // Make new configuration the active configuration
    pAppidActiveConfig = pAppidNewConfig;

    return pAppidOldConfig;;
}

void AppIdCommonUnload(void *old_context)
{
    tAppIdConfig *pOldConfig = (tAppIdConfig *)old_context;

    pAppidPassiveConfig = pOldConfig;
    AppIdCleanupConfig(pOldConfig);
    UnconfigureServices(pOldConfig);
    UnconfigureClientApp(pOldConfig);
    UnloadLuaModules(pOldConfig);
    hostPortAppCacheFini(pOldConfig);
    lengthAppCacheFini(pOldConfig);
    appInfoTableFini(pOldConfig);
    http_detector_clean(&pOldConfig->detectorHttpConfig);
    service_ssl_clean(&pOldConfig->serviceSslConfig);
    service_dns_host_clean(&pOldConfig->serviceDnsConfig);

    _dpd.snortFree(pOldConfig, sizeof(*pOldConfig), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
    pAppidPassiveConfig = NULL;
}

