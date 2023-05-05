/* $Id$ */
/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2002-2013 Sourcefire, Inc.
 *
 * Author(s):  Andrew R. Baker <andrewb@snort.org>
 *             Martin Roesch   <roesch@sourcefire.com>
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
 *
 */

/* includes */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef WIN32
#include <netdb.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "util.h"
#include "mstring.h"
#include "parser.h"
#include "snort_debug.h"
#include "snort.h"
#include "sfPolicy.h"

#include "IpAddrSet.h"

# include "ipv6_port.h"

extern char *file_name;     /* current rules file being processed */
extern int line_num;        /* current rules file line */



IpAddrSet *IpAddrSetParse(SnortConfig *sc, char *addr)
{
    IpAddrSet *ret;
    int ret_code;
    vartable_t *ip_vartable;

    if ((sc == NULL) || (sc->targeted_policies[getParserPolicy(sc)] == NULL))
    {
        FatalError("%s(%d) Snort conf for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    ip_vartable = sc->targeted_policies[getParserPolicy(sc)]->ip_vartable;

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Got address string: %s\n",
                addr););

    ret = (IpAddrSet*)SnortAlloc(sizeof(IpAddrSet));

    if((ret_code = sfvt_add_to_var(ip_vartable, ret, addr)) != SFIP_SUCCESS)
    {
        if(ret_code == SFIP_LOOKUP_FAILURE)
            FatalError("%s(%d) => Undefined variable in the string: %s\n",
                file_name, file_line, addr);
        else if(ret_code == SFIP_CONFLICT)
            FatalError("%s(%d) => Negated IP ranges that equal to or are"
                " more-specific than non-negated ranges are not allowed."
                " Consider inverting the logic: %s.\n",
                file_name, file_line, addr);
        else
            FatalError("%s(%d) => Unable to process the IP address: %s\n",
                file_name, file_line, addr);
    }

    return ret;
}

void IpAddrSetDestroy(IpAddrSet *ipAddrSet)
{

    if(!ipAddrSet)
        return;

    sfvar_free(ipAddrSet);
}

