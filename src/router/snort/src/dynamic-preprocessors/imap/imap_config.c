/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 ****************************************************************************/

/***************************************************************************
 * imap_config.c
 *
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 *
 * Description:
 *
 * Handle configuration of the IMAP preprocessor
 *
 * Entry point functions:
 *
 *    IMAP_ParseArgs()
 *
 ***************************************************************************/

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_imap.h"
#include "imap_config.h"
#include "snort_bounds.h"
#include "sf_dynamic_preprocessor.h"
#include "sfPolicy.h"


/*  Global variable to hold configuration */
extern IMAPConfig **imap_config;

extern const IMAPToken imap_known_cmds[];

/* Private functions */
static int  ProcessPorts(IMAPConfig *, char *, int, char **);

/*
 * Function: IMAP_ParseArgs(char *)
 *
 * Purpose: Process the preprocessor arguments from the rules file and
 *          initialize the preprocessor's data struct.  This function doesn't
 *          have to exist if it makes sense to parse the args in the init
 *          function.
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 */
void IMAP_ParseArgs(IMAPConfig *config, char *args)
{
    int ret = 0;
    char *arg;
    char *saveptr;
    char errStr[ERRSTRLEN];
    int errStrLen = ERRSTRLEN;

    if ((config == NULL) || (args == NULL))
        return;

    enablePort( config->ports, IMAP_DEFAULT_SERVER_PORT );
    config->memcap = DEFAULT_IMAP_MEMCAP;
    _dpd.fileAPI->set_mime_decode_config_defauts(&(config->decode_conf));
    _dpd.fileAPI->set_mime_log_config_defauts(&(config->log_config));

    *errStr = '\0';

    arg = strtok_r(args, CONF_SEPARATORS, &saveptr);

    while ( arg != NULL )
    {
        unsigned long value = 0;

        if ( !strcasecmp(CONF_PORTS, arg) )
        {
            ret = ProcessPorts(config, errStr, errStrLen, &saveptr);
        }
        else if ( !strcasecmp(CONF_IMAP_MEMCAP, arg) )
        {
            ret = _dpd.checkValueInRange(strtok_r(NULL, CONF_SEPARATORS, &saveptr), CONF_IMAP_MEMCAP,
                    MIN_IMAP_MEMCAP, MAX_IMAP_MEMCAP, &value);
            config->memcap = (uint32_t)value;
        }
        else if ( !strcasecmp(CONF_MAX_MIME_MEM, arg) )
        {
            ret = _dpd.checkValueInRange(strtok_r(NULL, CONF_SEPARATORS, &saveptr), CONF_MAX_MIME_MEM,
                    MIN_MIME_MEM, MAX_MIME_MEM, &value);
            config->decode_conf.max_mime_mem = (int)value;
        }
        else if(!_dpd.fileAPI->parse_mime_decode_args(&(config->decode_conf), arg, "IMAP", &saveptr))
        {
            ret = 0;
        }
        else if ( !strcasecmp(CONF_DISABLED, arg) )
        {
            config->disabled = 1;
        }
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Unknown IMAP configuration option %s\n",
                                            *(_dpd.config_file), *(_dpd.config_line), arg);
        }

        if (ret == -1)
        {
            /*
            **  Fatal Error, log error and exit.
            */
            if (*errStr)
            {
                DynamicPreprocessorFatalMessage("%s(%d) => %s\n",
                                                *(_dpd.config_file), *(_dpd.config_line), errStr);
            }
            else
            {
                DynamicPreprocessorFatalMessage("%s(%d) => Undefined Error.\n",
                                                *(_dpd.config_file), *(_dpd.config_line));
            }
        }

        /*  Get next token */
        arg = strtok_r(NULL, CONF_SEPARATORS, &saveptr);
    }

}

void IMAP_CheckConfig(IMAPConfig *pPolicyConfig, tSfPolicyUserContextId context)
{
    IMAPConfig *defaultConfig =
            (IMAPConfig *)sfPolicyUserDataGetDefault(context);

    if (pPolicyConfig == defaultConfig)
    {
       if (! _dpd.fileAPI->check_decoding_conf(&(pPolicyConfig->decode_conf),
                &(defaultConfig->decode_conf), "IMAP"))
           return;

        if (!pPolicyConfig->memcap)
            pPolicyConfig->memcap = DEFAULT_IMAP_MEMCAP;

    }
    else if (defaultConfig == NULL)
    {
        _dpd.fileAPI->check_decoding_conf(&(pPolicyConfig->decode_conf),
                NULL, "IMAP");

    }
    else
    {
        pPolicyConfig->memcap = defaultConfig->memcap;
        if(pPolicyConfig->disabled)
        {
            pPolicyConfig->decode_conf = defaultConfig->decode_conf;
            return;
        }
        _dpd.fileAPI->check_decoding_conf(&(pPolicyConfig->decode_conf),
                &(defaultConfig->decode_conf), "IMAP");

    }
}

void IMAP_PrintConfig(IMAPConfig *config)
{
    int i;
    int j = 0;
    char buf[8192];

    if (config == NULL)
        return;

    memset(&buf[0], 0, sizeof(buf));

    _dpd.logMsg("IMAP Config:\n");

    if(config->disabled)
        _dpd.logMsg("    IMAP: INACTIVE\n");

    snprintf(buf, sizeof(buf) - 1, "    Ports: ");

    for(i = 0; i < 65536; i++)
    {
        if( isPortEnabled( config->ports, i ) )
        {
            j++;
            _dpd.printfappend(buf, sizeof(buf) - 1, "%d ", i);
            if(!(j%10))
                _dpd.printfappend(buf, sizeof(buf) - 1, "\n    ");
        }
    }

    _dpd.logMsg("%s\n", buf);


    _dpd.logMsg("    IMAP Memcap: %u\n",
            config->memcap);

    _dpd.logMsg("    MIME Max Mem: %d\n",
            config->decode_conf.max_mime_mem);

    if(config->decode_conf.b64_depth > -1)
    {
        _dpd.logMsg("    Base64 Decoding: %s\n", "Enabled");
        switch(config->decode_conf.b64_depth)
        {
            case 0:
                _dpd.logMsg("    Base64 Decoding Depth: %s\n", "Unlimited");
                break;
            default:
                _dpd.logMsg("    Base64 Decoding Depth: %d\n", config->decode_conf.b64_depth);
                break;
        }
    }
    else
    _dpd.logMsg("    Base64 Decoding: %s\n", "Disabled");

    if(config->decode_conf.qp_depth > -1)
    {
        _dpd.logMsg("    Quoted-Printable Decoding: %s\n","Enabled");
        switch(config->decode_conf.qp_depth)
        {
            case 0:
                _dpd.logMsg("    Quoted-Printable Decoding Depth: %s\n", "Unlimited");
                break;
            default:
                _dpd.logMsg("    Quoted-Printable Decoding Depth: %d\n", config->decode_conf.qp_depth);
                break;
        }
    }
    else
        _dpd.logMsg("    Quoted-Printable Decoding: %s\n", "Disabled");

    if(config->decode_conf.uu_depth > -1)
    {
        _dpd.logMsg("    Unix-to-Unix Decoding: %s\n","Enabled");
        switch(config->decode_conf.uu_depth)
        {
            case 0:
                _dpd.logMsg("    Unix-to-Unix Decoding Depth: %s\n", "Unlimited");
                break;
            default:
                _dpd.logMsg("    Unix-to-Unix Decoding Depth: %d\n", config->decode_conf.uu_depth);
                break;
        }
    }
    else
        _dpd.logMsg("    Unix-to-Unix Decoding: %s\n", "Disabled");

    if(config->decode_conf.bitenc_depth > -1)
    {
        _dpd.logMsg("    Non-Encoded MIME attachment Extraction: %s\n","Enabled");
        switch(config->decode_conf.bitenc_depth)
        {
            case 0:
                _dpd.logMsg("    Non-Encoded MIME attachment Extraction Depth: %s\n", "Unlimited");
                break;
            default:
                _dpd.logMsg("    Non-Encoded MIME attachment Extraction Depth: %d\n", config->decode_conf.bitenc_depth);
                break;
        }
    }
    else
        _dpd.logMsg("    Non-Encoded MIME attachment Extraction: %s\n", "Disabled");

}

/*
**  NAME
**    ProcessPorts::
*/
/**
**  Process the port list.
**
**  This configuration is a list of valid ports and is ended by a
**  delimiter.
**
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**  @param saveptr     the strtok_r saved state
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
**  @retval  1 generic non-fatal error
*/
static int ProcessPorts(IMAPConfig *config, char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    char *pcEnd;
    int  iPort;
    int  iEndPorts = 0;
    int num_ports = 0;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "IMAP config is NULL.\n");
        return -1;
    }

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(!pcToken)
    {
        snprintf(ErrorString, ErrStrLen, "Invalid port list format.");
        return -1;
    }

    if(strcmp(CONF_START_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start a port list with the '%s' token.", CONF_START_LIST);

        return -1;
    }

    /* Since ports are specified, clear default ports */
    disablePort( config->ports, IMAP_DEFAULT_SERVER_PORT );

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if(!strcmp(CONF_END_LIST, pcToken))
        {
            iEndPorts = 1;
            break;
        }

        iPort = strtol(pcToken, &pcEnd, 10);

        /*
        **  Validity check for port
        */
        if(*pcEnd)
        {
            snprintf(ErrorString, ErrStrLen,
                     "Invalid port number.");

            return -1;
        }

        if(iPort < 0 || iPort > MAXPORTS-1)
        {
            snprintf(ErrorString, ErrStrLen,
                     "Invalid port number.  Must be between 0 and 65535.");

            return -1;
        }

        enablePort( config->ports, iPort );
        num_ports++;
    }

    if(!iEndPorts)
    {
        snprintf(ErrorString, ErrStrLen,
                 "Must end '%s' configuration with '%s'.",
                 CONF_PORTS, CONF_END_LIST);

        return -1;
    }
    else if(!num_ports)
    {
        snprintf(ErrorString, ErrStrLen,
             "IMAP: Empty port list not allowed.");
        return -1;
    }

    return 0;
}
