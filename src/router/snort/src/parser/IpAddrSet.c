/* $Id$ */
/*
 * Copyright (C) 2002-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
#include "debug.h"
#include "snort.h"
#include "sfPolicy.h"

#include "IpAddrSet.h"

#ifdef SUP_IP6
# include "ipv6_port.h"
#endif

extern SnortConfig *snort_conf_for_parsing;
extern char *file_name;     /* current rules file being processed */
extern int line_num;        /* current rules file line */

#ifndef SUP_IP6
IpAddrSet *IpAddrSetCreate(void)
{
    IpAddrSet *tmp;

    tmp = (IpAddrSet *) SnortAlloc(sizeof(IpAddrSet));

    return tmp;
}

static char buffer[1024];

void IpAddrSetPrint(char *prefix, IpAddrSet *ipAddrSet)
{
    IpAddrNode *iplist, *neglist;
    struct in_addr in;
    int ret;

    if(!ipAddrSet) return;

    iplist = ipAddrSet->iplist;
    neglist = ipAddrSet->neg_iplist;

    while(iplist) 
    {
        buffer[0] = '\0';

        in.s_addr = iplist->ip_addr;
        ret = SnortSnprintfAppend(buffer, sizeof(buffer), "%s/", inet_ntoa(in));
        if (ret != SNORT_SNPRINTF_SUCCESS)
            return;

        in.s_addr = iplist->netmask;
        ret = SnortSnprintfAppend(buffer, sizeof(buffer), "%s", inet_ntoa(in));
        if (ret != SNORT_SNPRINTF_SUCCESS)
            return;

        if (prefix)
            LogMessage("%s%s\n", prefix, buffer);
        else
            LogMessage("%s\n", buffer);

        iplist = iplist->next;
       
    }

    while(neglist) 
    {
        buffer[0] = '\0';

        in.s_addr = neglist->ip_addr;
        ret = SnortSnprintfAppend(buffer, sizeof(buffer), "NOT %s/", inet_ntoa(in));
        if (ret != SNORT_SNPRINTF_SUCCESS)
            return;

        in.s_addr = neglist->netmask;
        ret = SnortSnprintfAppend(buffer, sizeof(buffer), "%s", inet_ntoa(in));
        if (ret != SNORT_SNPRINTF_SUCCESS)
            return;

        if (prefix)
            LogMessage("%s%s\n", prefix, buffer);
        else
            LogMessage("%s\n", buffer);

        neglist = neglist->next;
    }
}

IpAddrSet *IpAddrSetCopy(IpAddrSet *ipAddrSet)
{
    IpAddrSet *newIpAddrSet;
    IpAddrNode *current;
    IpAddrNode *iplist, *neglist;
    IpAddrNode *prev = NULL;

    if(!ipAddrSet) return NULL;

    newIpAddrSet = (IpAddrSet *)calloc(sizeof(IpAddrSet), 1);
    if(!newIpAddrSet) 
    {
        goto failed;
    }

    iplist = ipAddrSet->iplist;
    neglist = ipAddrSet->neg_iplist;

    while(iplist)
    {
        current = (IpAddrNode *)malloc(sizeof(IpAddrNode));
        if (!current)
        {
            goto failed;
        }

        if(!newIpAddrSet->iplist)
            newIpAddrSet->iplist = current;
        
        current->ip_addr = iplist->ip_addr;
        current->netmask = iplist->netmask;
        current->addr_flags = iplist->addr_flags;
        current->next = NULL;

        if(prev)
            prev->next = current;

        prev = current;

        iplist = iplist->next;
    }

    prev = current = NULL;
    while(neglist)
    {
        current = (IpAddrNode *)malloc(sizeof(IpAddrNode));
        if (!current)
        {
            goto failed;
        }
        
        if(!newIpAddrSet->neg_iplist)
            newIpAddrSet->neg_iplist = current;

        current->ip_addr = neglist->ip_addr;
        current->netmask = neglist->netmask;
        current->addr_flags = neglist->addr_flags;
        current->next = NULL;

        if(prev)
            prev->next = current;

        prev = current;

        neglist = neglist->next;
    }

    newIpAddrSet->id = ipAddrSet->id;

    return newIpAddrSet;

failed:
    if(newIpAddrSet)
        IpAddrSetDestroy(newIpAddrSet);
    return NULL; /* XXX ENOMEM */
}


/* XXX: legacy support function */
/*
 * Function: ParseIP(char *, IpAddrSet *)
 *
 * Purpose: Convert a supplied IP address to it's network order 32-bit long
 *          value.  Also convert the CIDR block notation into a real
 *          netmask.
 *
 * Arguments: char *addr  => address string to convert
 *            IpAddrSet * =>
 *            
 *
 * Returns: 0 for normal addresses, 1 for an "any" address
 */
int ParseIP(char *paddr, IpAddrSet *ias, int negate) //, IpAddrNode *node)
{
    char **toks;        /* token dbl buffer */
    int num_toks;       /* number of tokens found by mSplit() */
    int cidr = 1;       /* is network expressed in CIDR format */
    int nmask = -1;     /* netmask temporary storage */
    char *addr;         /* string to parse, eventually a
                         * variable-contents */
    struct hostent *host_info;  /* various struct pointers for stuff */
    struct sockaddr_in sin; /* addr struct */
    char broadcast_addr_set = 0;

    IpAddrNode *address_data = (IpAddrNode*)SnortAlloc(sizeof(IpAddrNode));

    if(!paddr || !ias) 
        return 1;

    addr = paddr;

    if(*addr == '!')
    {
        negate = !negate;
//        address_data->addr_flags |= EXCEPT_IP;

        addr++;  /* inc past the '!' */
    }

    /* check for wildcards */
    if(!strcasecmp(addr, "any"))
    {
        if(negate) 
        {
            FatalError("%s(%d) => !any is not allowed\n", file_name, file_line);
        }
    
        /* Make first node 0, which matches anything */
        if(!ias->iplist) 
        {
            ias->iplist = (IpAddrNode*)SnortAlloc(sizeof(IpAddrNode));
        }
        ias->iplist->ip_addr = 0;
        ias->iplist->netmask = 0;

        free(address_data);

        return 1;
    }
    /* break out the CIDR notation from the IP address */
    toks = mSplit(addr, "/", 2, &num_toks, 0);

    /* "/" was not used as a delimeter, try ":" */
    if(num_toks == 1)
    {
        mSplitFree(&toks, num_toks);
        toks = mSplit(addr, ":", 2, &num_toks, 0);
    }

    /*
     * if we have a mask spec and it is more than two characters long, assume
     * it is netmask format
     */
    if((num_toks > 1) && strlen(toks[1]) > 2)
    {
        cidr = 0;
    }

    switch(num_toks)
    {
        case 1:
            address_data->netmask = netmasks[32];
            break;

        case 2:
            if(cidr)
            {
                /* convert the CIDR notation into a real live netmask */
                nmask = atoi(toks[1]);

                /* it's pain to differ whether toks[1] is correct if netmask */
                /* is /0, so we deploy some sort of evil hack with isdigit */

                if(!isdigit((int) toks[1][0]))
                    nmask = -1;

                /* if second char is != '\0', it must be a digit
                 * by Daniel B. Cid, dcid@sourcefire.com
                 */ 
                if((toks[1][1] != '\0')&&(!isdigit((int) toks[1][1]) ))
                    nmask = -1;
                
                if((nmask > -1) && (nmask < 33))
                {
                    address_data->netmask = netmasks[nmask];
                }
                else
                {
                    FatalError("%s(%d): Invalid CIDR block for IP addr "
                            "%s\n", file_name, file_line, addr);
                           
                }
            }
            else
            {
                /* convert the netmask into its 32-bit value */

                /* broadcast address fix from 
                 * Steve Beaty <beaty@emess.mscd.edu> 
                 */

                /*
                 * if the address is the (v4) broadcast address, inet_addr *
                 * returns -1 which usually signifies an error, but in the *
                 * broadcast address case, is correct.  we'd use inet_aton() *
                 * here, but it's less portable.
                 */
                if(!strncmp(toks[1], "255.255.255.255", 15))
                {
                    address_data->netmask = INADDR_BROADCAST;
                }
                else if((address_data->netmask = inet_addr(toks[1])) == INADDR_NONE)
                {
                    FatalError("%s(%d): Unable to parse rule netmask "
                            "(%s)\n", file_name, file_line, toks[1]);
                }
                /* Set nmask so we don't try to do a host lookup below.
                 * The value of 0 is irrelevant. */
                nmask = 0;
            }
            break;

        default:
            FatalError("%s(%d) => Unrecognized IP address/netmask %s\n",
                    file_name, file_line, addr);
            break;
    }
    sin.sin_addr.s_addr = inet_addr(toks[0]);

#ifndef WORDS_BIGENDIAN
    /*
     * since PC's store things the "wrong" way, shuffle the bytes into the
     * right order.  Non-CIDR netmasks are already correct.
     */
    if(cidr)
    {
        address_data->netmask = htonl(address_data->netmask);
    }
#endif
    /* broadcast address fix from Steve Beaty <beaty@emess.mscd.edu> */
    /* Changed location */
    if(!strncmp(toks[0], "255.255.255.255", 15))
    {
        address_data->ip_addr = INADDR_BROADCAST;
        broadcast_addr_set = 1;
    }
    else if (nmask == -1)
    {
        /* Try to do a host lookup if the address didn't
         * convert to a valid IP and there were not any
         * mask bits specified (CIDR or dot notation). */
        if(sin.sin_addr.s_addr == INADDR_NONE)
        {
            /* get the hostname and fill in the host_info struct */
            host_info = gethostbyname(toks[0]);
            if (host_info)
            {
                /* protecting against malicious DNS servers */
                if(host_info->h_length <= (int)sizeof(sin.sin_addr))
                {
                    bcopy(host_info->h_addr, (char *) &sin.sin_addr, host_info->h_length);
                }
                else
                {
                    bcopy(host_info->h_addr, (char *) &sin.sin_addr, sizeof(sin.sin_addr));
                }
            }
            /* Using h_errno */
            else if(h_errno == HOST_NOT_FOUND)
            /*else if((sin.sin_addr.s_addr = inet_addr(toks[0])) == INADDR_NONE)*/
            {
                FatalError("%s(%d): Couldn't resolve hostname %s\n",
                    file_name, file_line, toks[0]);
            }
        }
        else
        {
            /* It was a valid IP address with no netmask specified. */
            /* Noop */
        }
    }
    else
    {
        if(sin.sin_addr.s_addr == INADDR_NONE)
        {
            /* It was not a valid IP address but had a valid netmask. */
            FatalError("%s(%d): Rule IP addr (%s) didn't translate\n",
                file_name, file_line, toks[0]);
        }
    }

    /* Only set this if we haven't set it above as 255.255.255.255 */
    if (!broadcast_addr_set)
    {
        address_data->ip_addr = ((u_long) (sin.sin_addr.s_addr) &
            (address_data->netmask));
    }
    mSplitFree(&toks, num_toks);

    /* Add new IP address to address set */
    if(!negate) 
    {
        IpAddrNode *idx;

        if(!ias->iplist) 
        {
            ias->iplist = address_data;
        }
        else 
        {
            /* Get to the end of the list */
            for(idx = ias->iplist; idx->next; idx=idx->next) ;

            idx->next = address_data;
        }
    }
    else
    {
        IpAddrNode *idx;

        if(!ias->neg_iplist) 
        {
            ias->neg_iplist = address_data;
        }
        else 
        {
            /* Get to the end of the list */
            for(idx = ias->neg_iplist; idx->next; idx=idx->next) ;

            idx->next = address_data;
        }

        address_data->addr_flags |= EXCEPT_IP;
    }
    
    return 0;
} 


void IpAddrSetBuild(char *addr, IpAddrSet *ret, int neg_list) 
{
    char *tok, *end, *tmp;
    int neg_ip;

    while(*addr) 
    {
        /* Skip whitespace and leading commas */
        for(; *addr && (isspace((int)*addr) || *addr == ','); addr++) ;

        /* Handle multiple negations (such as if someone negates variable that
         * contains a negated IP */
        neg_ip = 0;
        for(; *addr == '!'; addr++) 
             neg_ip = !neg_ip;

        /* Find end of this token */
        for(end = addr+1; 
           *end && !isspace((int)*end) && *end != ']' && *end != ',';
            end++) ;

        tok = SnortStrndup(addr, end - addr);

        if(!tok)    
        {
            FatalError("%s(%d) => Failed to allocate memory for parsing '%s'\n", 
                           file_name, file_line, addr);
        }

        if(*addr == '[') 
        {
            int brack_count = 0;
            char *list_tok;
    
            /* Find corresponding ending bracket */
            for(end = addr; *end; end++) 
            {
                if(*end == '[') 
                    brack_count++;
                else if(*end == ']')
                    brack_count--;
    
                if(!brack_count)
                    break;
            }
    
            if(!*end) 
            {
                FatalError("%s(%d) => Unterminated IP List '%s'\n", 
                           file_name, file_line, addr);
            }
        
            addr++;

            list_tok = SnortStrndup(addr, end - addr);

            if(!list_tok)    
            {
                FatalError("%s(%d) => Failed to allocate memory for parsing '%s'\n", 
                               file_name, file_line, addr);
            }

            IpAddrSetBuild(list_tok, ret, neg_ip ^ neg_list);
            free(list_tok);
        }
        else if(*addr == '$') 
        {
            if((tmp = VarGet(tok + 1)) == NULL)
            {
                FatalError("%s(%d) => Undefined variable %s\n", file_name, 
                        file_line, addr);
            }
            
            IpAddrSetBuild(tmp, ret, neg_list ^ neg_ip); 
        }
        else if(*addr == ']')
        {
            if(!(*(addr+1))) 
            {
                /* Succesfully reached the end of this list */
                free(tok);
                return;
            }

            FatalError("%s(%d) => Mismatched bracket in '%s'\n", 
                           file_name, file_line, addr);
        }
        else 
        {
            /* Skip leading commas */
            for(; *addr && (*addr == ',' || isspace((int)*addr)); addr++) ;

            ParseIP(tok, ret, neg_list ^ neg_ip);

            if(ret->iplist && !ret->iplist->ip_addr && !ret->iplist->netmask) 
                 ret->iplist->addr_flags |= ANY_SRC_IP;
                
            /* Note: the neg_iplist is not checked for '!any' here since
             * ParseIP should have already FatalError'ed on it. */
        }
        
        free(tok);

        if(*end)
            addr = end + 1;   
        else break;
    }

    return;
}
#endif


IpAddrSet *IpAddrSetParse(char *addr) 
{
    IpAddrSet *ret;
#ifdef SUP_IP6
    int ret_code;
    SnortConfig *sc = snort_conf_for_parsing;
    vartable_t *ip_vartable;

    if ((sc == NULL) || (sc->targeted_policies[getParserPolicy()] == NULL))
    {
        FatalError("%s(%d) Snort conf for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    ip_vartable = sc->targeted_policies[getParserPolicy()]->ip_vartable;
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_CONFIGRULES,"Got address string: %s\n", 
                addr););

    ret = (IpAddrSet*)SnortAlloc(sizeof(IpAddrSet));

#ifdef SUP_IP6 
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
#else

    IpAddrSetBuild(addr, ret, 0);

#endif

    return ret;
}

void IpAddrSetDestroy(IpAddrSet *ipAddrSet)
{
#ifndef SUP_IP6
    IpAddrNode *node, *tmp;
#endif

    if(!ipAddrSet) 
        return;

#ifdef SUP_IP6
    sfvar_free(ipAddrSet);
#else
    node = ipAddrSet->iplist;

    while(node)
    {
        tmp = node;
        node = node->next;
        free(tmp);
    }

    node = ipAddrSet->neg_iplist;

    while(node)
    {
        tmp = node;
        node = node->next;
        free(tmp);
    }
#endif
}

#ifndef SUP_IP6
int IpAddrSetContains(IpAddrSet *ias, struct in_addr test_addr)
{
    IpAddrNode *index;
    uint32_t raw_addr = test_addr.s_addr;
    int match = 0;

    if(!ias)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_ALL,"Null IP address set!\n"););
        return 0;
    }
    if(!ias->iplist) 
        match = 1;

    for(index = ias->iplist; index != NULL; index = index->next)
    {
        if(index->ip_addr == (raw_addr & index->netmask)) 
        {
            match = 1;
            break;
        }
    }   

    if(!match) 
        return 0;

    if(!ias->neg_iplist) 
        return 1;

    for(index = ias->neg_iplist; index != NULL; index = index->next)
    {
        if(index->ip_addr == (raw_addr & index->netmask)) 
            return 0;
    }

    return 1;
}
#endif // SUP_IP6
