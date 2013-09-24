/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <vitas@nppfactor.kiev.ua>

******************************************************************************/

/* #define MISC_TEST_FUNCS */

#include <unistd.h>
#include <syslog.h>

#include "epoll_loop.h"
#include "bridge_ctl.h"
#include "netif_utils.h"
#include "packet.h"
#include "log.h"
#include "mstp.h"
#include "ctl_socket_server.h"

#define APP_NAME    "mstpd"

static int become_daemon = 1;
static int is_daemon = 0;
int log_level = LOG_LEVEL_DEFAULT;

#ifdef MISC_TEST_FUNCS
static bool test_ports_trees_mesh(void);
#endif /* MISC_TEST_FUNCS */

int main(int argc, char *argv[])
{
    int c;

    /* Sanity check */
    {
        bridge_identifier_t BridgeIdentifier;
        mst_configuration_identifier_t MST_ConfigurationIdentifier;
        TST(sizeof(BridgeIdentifier) == 8, -1);
        TST(sizeof(BridgeIdentifier.u) == 8, -1);
        TST(sizeof(BridgeIdentifier.s) == 8, -1);
        TST(sizeof(BridgeIdentifier.s.priority) == 2, -1);
        TST(sizeof(BridgeIdentifier.s.mac_address) == 6, -1);
        TST(sizeof(MST_ConfigurationIdentifier) == 51, -1);
        TST(sizeof(MST_ConfigurationIdentifier.a) == 51, -1);
        TST(sizeof(MST_ConfigurationIdentifier.s) == 51, -1);
#ifdef HMAC_MDS_TEST_FUNCTIONS
        TST(MD5TestSuite(), -1);
#endif /* HMAC_MDS_TEST_FUNCTIONS */
#ifdef MISC_TEST_FUNCS
        TST(test_ports_trees_mesh(), -1);
#endif /* MISC_TEST_FUNCS */
        INFO("Sanity checks succeeded");
    }

    while((c = getopt(argc, argv, "dv:")) != -1)
    {
        switch (c)
        {
            case 'd':
                become_daemon = 0;
                break;
            case 'v':
            {
                char *end;
                long l;
                l = strtoul(optarg, &end, 0);
                if(*optarg == 0 || *end != 0 || l > LOG_LEVEL_MAX)
                {
                    ERROR("Invalid loglevel %s", optarg);
                    exit(1);
                }
                log_level = l;
                break;
            }
            default:
                return -1;
        }
    }

    if(become_daemon)
    {
        FILE *f = fopen("/var/run/"APP_NAME".pid", "w");
        if(!f)
        {
            ERROR("can't open /var/run/"APP_NAME".pid");
            return -1;
        }
        openlog(APP_NAME, 0, LOG_DAEMON);
        if(daemon(0, 0))
        {
            ERROR("can't daemonize");
            return -1;
        }
        is_daemon = 1;
        fprintf(f, "%d", getpid());
        fclose(f);
    }

    TST(init_epoll() == 0, -1);
    TST(ctl_socket_init() == 0, -1);
    TST(packet_sock_init() == 0, -1);
    TST(netsock_init() == 0, -1);
    TST(init_bridge_ops() == 0, -1);

    return epoll_main_loop();
}

/*********************** Logging *********************/

#include <stdarg.h>
#include <time.h>

void vDprintf(int level, const char *fmt, va_list ap)
{
    if(level > log_level)
        return;

    if(!is_daemon)
    {
        char logbuf[256];
        logbuf[255] = 0;
        time_t clock;
        struct tm *local_tm;
        time(&clock);
        local_tm = localtime(&clock);
        int l = strftime(logbuf, sizeof(logbuf) - 1, "%F %T ", local_tm);
        vsnprintf(logbuf + l, sizeof(logbuf) - l - 1, fmt, ap);
        printf("%s\n", logbuf);
    }
    else
    {
        vsyslog((level <= LOG_LEVEL_INFO) ? LOG_INFO : LOG_DEBUG, fmt, ap);
    }
}

void Dprintf(int level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vDprintf(level, fmt, ap);
    va_end(ap);
}

/*********************** Testing *********************/
#ifdef MISC_TEST_FUNCS

#include <string.h>
#include <asm/byteorder.h>

static void printout_mesh(bridge_t *br)
{
    tree_t *tree;
    port_t *prt;
    per_tree_port_t *ptp;

    printf("Ports:\n");
    list_for_each_entry(prt, &br->ports, br_list)
    {
        printf(" %s(%03hX)", prt->sysdeps.name, __be16_to_cpu(prt->port_number));
        list_for_each_entry(ptp, &prt->trees, port_list)
            printf("->%03hX", __be16_to_cpu(ptp->MSTID));
        printf("\n");
    }

    printf("\nTrees:\n");
    list_for_each_entry(tree, &br->trees, bridge_list)
    {
        printf(" %03hX", __be16_to_cpu(tree->MSTID));
        list_for_each_entry(ptp, &tree->ports, tree_list)
            printf("->%s", ptp->port->sysdeps.name);
        printf("\n");
    }
    printf("\n");
}

static bool test_ports_trees_mesh(void)
{
    bridge_t *br = calloc(1, sizeof(*br));
    if(!br)
        return false;
    strcpy(br->sysdeps.name, "BR_TEST");
    br->sysdeps.macaddr[5] = 0xED;
    if(!MSTP_IN_bridge_create(br, br->sysdeps.macaddr))
    {
        free(br);
        return false;
    }

    port_t *prt[5];
    int i;

    for(i = 0; i < 5; ++i)
    {
        if(!(prt[i] = calloc(1, sizeof(port_t))))
            return false;
        prt[i]->bridge = br;
    }

    if(!MSTP_IN_create_msti(br, 0xF91))
    {
error_exit:
        MSTP_IN_delete_bridge(br);
        free(br);
        return false;
    }
    if(!MSTP_IN_create_msti(br, 0xE10))
        goto error_exit;

    strcpy(prt[0]->sysdeps.name, "PRT_10C");
    if(!MSTP_IN_port_create_and_add_tail(prt[0], 0x10C))
        goto error_exit;

    strcpy(prt[1]->sysdeps.name, "PRT_001");
    if(!MSTP_IN_port_create_and_add_tail(prt[1], 0x001))
        goto error_exit;

    strcpy(prt[2]->sysdeps.name, "PRT_C01");
    if(!MSTP_IN_port_create_and_add_tail(prt[2], 0xC01))
        goto error_exit;

    if(!MSTP_IN_create_msti(br, 0xE12))
        goto error_exit;
    if(!MSTP_IN_create_msti(br, 0x001))
        goto error_exit;

    strcpy(prt[3]->sysdeps.name, "PRT_002");
    if(!MSTP_IN_port_create_and_add_tail(prt[3], 0x002))
        goto error_exit;
    strcpy(prt[4]->sysdeps.name, "PRT_003");
    if(!MSTP_IN_port_create_and_add_tail(prt[4], 0x003))
        goto error_exit;

    if(!MSTP_IN_create_msti(br, 0x005))
        goto error_exit;

    printout_mesh(br);

    MSTP_IN_delete_port(prt[1]);
    if(!MSTP_IN_delete_msti(br, 0xE12))
        goto error_exit;
    MSTP_IN_delete_port(prt[3]);
    if(!MSTP_IN_delete_msti(br, 0x005))
        goto error_exit;

    printout_mesh(br);

    if(!MSTP_IN_create_msti(br, 0x102))
        goto error_exit;
    strcpy(prt[1]->sysdeps.name, "PRT_504");
    if(!MSTP_IN_port_create_and_add_tail(prt[1], 0x504))
        goto error_exit;

    if(!MSTP_IN_create_msti(br, 0x105))
        goto error_exit;
    strcpy(prt[3]->sysdeps.name, "PRT_777");
    if(!MSTP_IN_port_create_and_add_tail(prt[3], 0x777))
        goto error_exit;

    printout_mesh(br);

    MSTP_IN_delete_bridge(br);
    free(br);
    return true;
}
#endif /* MISC_TEST_FUNCS */
