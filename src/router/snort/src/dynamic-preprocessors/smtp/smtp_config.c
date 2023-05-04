/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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
 * smtp_config.c
 *
 * Author: Andy Mullican
 * Author: Todd Wease
 *
 * Description:
 *
 * Handle configuration of the SMTP preprocessor
 *
 * Entry point functions:
 *
 *    SMTP_ParseArgs()
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
#include "snort_smtp.h"
#include "smtp_config.h"
#include "snort_bounds.h"
#include "sf_dynamic_preprocessor.h"
#include "sfPolicy.h"


/* Private functions */
static int  ProcessPorts(SMTPConfig *, char *, int, char **);
static int  ProcessCmds(SMTPConfig *, char *, int, char **, int, SMTPCmdTypeEnum);
static int  GetCmdId(SMTPConfig *, char *, SMTPCmdTypeEnum);
static int  AddCmd(SMTPConfig *, char *, SMTPCmdTypeEnum);
static int  ProcessAltMaxCmdLen(SMTPConfig *, char *, int, char **);
static int  ProcessMaxMimeDepth(SMTPConfig *, char *, int, char **);
static int  ProcessLogDepth(SMTPConfig *, char *, int, char **);
static int  ProcessXlink2State(SMTPConfig *, char *, int, char **);

/*
 * Function: SMTP_ParseArgs(char *)
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
void SMTP_ParseArgs(SMTPConfig *config, char *args)
{
    int ret = 0;
    char *arg;
    char *value;
    char errStr[ERRSTRLEN];
    char *saveptr;
    int errStrLen = ERRSTRLEN;
    int deprecated_options = 0;

    if ((config == NULL) || (args == NULL))
        return;

    enablePort( config->ports, SMTP_DEFAULT_SERVER_PORT );
    enablePort( config->ports, XLINK2STATE_DEFAULT_PORT );
    enablePort( config->ports, SMTP_DEFAULT_SUBMISSION_PORT );
    config->inspection_type = SMTP_STATELESS;
    config->max_command_line_len = DEFAULT_MAX_COMMAND_LINE_LEN;
    config->max_header_line_len = DEFAULT_MAX_HEADER_LINE_LEN;
    config->max_response_line_len = DEFAULT_MAX_RESPONSE_LINE_LEN;
    config->max_mime_depth = DEFAULT_MAX_MIME_DEPTH;
    config->memcap = DEFAULT_SMTP_MEMCAP;
    config->alert_xlink2state = 1;
    config->print_cmds = 1;
    config->enable_mime_decoding = 0;
    config->max_auth_command_line_len = DEFAULT_AUTH_MAX_COMMAND_LINE_LEN;
    _dpd.fileAPI->set_mime_decode_config_defauts(&(config->decode_conf));
    _dpd.fileAPI->set_mime_log_config_defauts(&(config->log_config));
    config->log_config.email_hdrs_log_depth = DEFAULT_LOG_DEPTH;

    config->cmd_config = (SMTPCmdConfig *)_dpd.snortAlloc(CMD_LAST, sizeof(SMTPCmdConfig), PP_SMTP,
                                               PP_MEM_CATEGORY_CONFIG);
    if (config->cmd_config == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    *errStr = '\0';

    arg = strtok_r(args, CONF_SEPARATORS, &saveptr);

    while ( arg != NULL )
    {
        unsigned long val = 0;

        if ( !strcasecmp(CONF_PORTS, arg) )
        {
            ret = ProcessPorts(config, errStr, errStrLen, &saveptr);
        }
        else if ( !strcasecmp(CONF_INSPECTION_TYPE, arg) )
        {
            value = strtok_r(NULL, CONF_SEPARATORS, &saveptr);
            if ( value == NULL )
            {
                return;
            }
            if ( !strcasecmp(CONF_STATEFUL, value) )
            {
                config->inspection_type = SMTP_STATEFUL;
            }
            else
            {
                config->inspection_type = SMTP_STATELESS;
            }
        }
        else if ( !strcasecmp(CONF_NORMALIZE, arg) )
        {
            value = strtok_r(NULL, CONF_SEPARATORS, &saveptr);
            if ( value == NULL )
            {
                return;
            }
            if ( !strcasecmp(CONF_NONE, value) )
            {
                config->normalize = NORMALIZE_NONE;
            }
            else if ( !strcasecmp(CONF_ALL, value) )
            {
                config->normalize = NORMALIZE_ALL;
            }
            else
            {
                config->normalize = NORMALIZE_CMDS;
            }
        }
        else if ( !strcasecmp(CONF_IGNORE_DATA, arg) )
        {
            config->decode_conf.ignore_data = 1;
        }
        else if ( !strcasecmp(CONF_IGNORE_TLS_DATA, arg) )
        {
            config->ignore_tls_data = 1;
        }
        else if ( !strcasecmp(CONF_MAX_COMMAND_LINE_LEN, arg) )
        {
            char *endptr;

            value = strtok_r(NULL, CONF_SEPARATORS, &saveptr);
            if ( value == NULL )
                return;

            config->max_command_line_len = strtol(value, &endptr, 10);
        }
        else if ( !strcasecmp(CONF_MAX_AUTH_COMMAND_LINE_LEN, arg) )
        {
            char *endptr;

            value = strtok_r(NULL, CONF_SEPARATORS, &saveptr);
            if ( value == NULL )
                return;

            config->max_auth_command_line_len = strtol(value, &endptr, 10);
        }

        else if ( !strcasecmp(CONF_MAX_HEADER_LINE_LEN, arg) )
        {
            char *endptr;

            value = strtok_r(NULL, CONF_SEPARATORS, &saveptr);
            if ( value == NULL )
                return;

            config->max_header_line_len = strtol(value, &endptr, 10);
        }
        else if ( !strcasecmp(CONF_MAX_RESPONSE_LINE_LEN, arg) )
        {
            char *endptr;

            value = strtok_r(NULL, CONF_SEPARATORS, &saveptr);
            if ( value == NULL )
                return;

            config->max_response_line_len = strtol(value, &endptr, 10);
        }
        else if ( !strcasecmp(CONF_NO_ALERTS, arg) )
        {
            config->no_alerts = 1;
        }
        else if ( !strcasecmp(CONF_ALERT_UNKNOWN_CMDS, arg) )
        {
            config->alert_unknown_cmds = 1;
        }
        else if ( !strcasecmp(CONF_INVALID_CMDS, arg) )
        {
            /* Parse disallowed commands */
            ret = ProcessCmds(config, errStr, errStrLen, &saveptr, ACTION_ALERT, SMTP_CMD_TYPE_NORMAL);
        }
        else if ( !strcasecmp(CONF_VALID_CMDS, arg) )
        {
            /* Parse allowed commands */
            ret = ProcessCmds(config, errStr, errStrLen, &saveptr, ACTION_NO_ALERT, SMTP_CMD_TYPE_NORMAL);
        }
        else if ( !strcasecmp(CONF_AUTH_CMDS, arg) )
        {
            ret = ProcessCmds(config, errStr, errStrLen, &saveptr, ACTION_NO_ALERT, SMTP_CMD_TYPE_AUTH);
        }
        else if ( !strcasecmp(CONF_DATA_CMDS, arg) )
        {
            ret = ProcessCmds(config, errStr, errStrLen, &saveptr, ACTION_NO_ALERT, SMTP_CMD_TYPE_DATA);
        }
        else if ( !strcasecmp(CONF_BDATA_CMDS, arg) )
        {
            ret = ProcessCmds(config, errStr, errStrLen, &saveptr, ACTION_NO_ALERT, SMTP_CMD_TYPE_BDATA);
        }
        else if ( !strcasecmp(CONF_NORMALIZE_CMDS, arg) )
        {
            /* Parse normalized commands */
            ret = ProcessCmds(config, errStr, errStrLen, &saveptr, ACTION_NORMALIZE, SMTP_CMD_TYPE_NORMAL);
        }
        else if ( !strcasecmp(CONF_ALT_MAX_COMMAND_LINE_LEN, arg) )
        {
            /* Parse max line len for commands */
            ret = ProcessAltMaxCmdLen(config, errStr, errStrLen, &saveptr);
        }
        else if ( !strcasecmp(CONF_SMTP_MEMCAP, arg) )
        {
            ret = _dpd.checkValueInRange(strtok_r(NULL, CONF_SEPARATORS, &saveptr), CONF_SMTP_MEMCAP,
                    MIN_SMTP_MEMCAP, MAX_SMTP_MEMCAP, &val);
            config->memcap = (uint32_t)val;
        }
        else if ( !strcasecmp(CONF_MAX_MIME_MEM, arg) )
        {
            ret = _dpd.checkValueInRange(strtok_r(NULL, CONF_SEPARATORS, &saveptr), CONF_MAX_MIME_MEM,
                    MIN_MIME_MEM, MAX_MIME_MEM, &val);
            config->decode_conf.max_mime_mem = (int)val;
        }
        else if ( !strcasecmp(CONF_MAX_MIME_DEPTH, arg) )
        {
            deprecated_options = 1;
            _dpd.logMsg("WARNING: %s(%d) => The SMTP config option 'max_mime_depth' is deprecated.\n",
                            *(_dpd.config_file), *(_dpd.config_line));
            ret = ProcessMaxMimeDepth(config, errStr, errStrLen, &saveptr);
        }
        else if ( !strcasecmp(CONF_ENABLE_MIME_DECODING, arg) )
        {
            deprecated_options = 1;
            _dpd.logMsg("WARNING: %s(%d) => The SMTP config option 'enable_mime_decoding' is deprecated.\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
            config->enable_mime_decoding = 1;
        }
        else if ( !strcasecmp(CONF_DISABLED, arg) )
        {
            config->disabled = 1;
        }
        else if ( !strcasecmp(CONF_XLINK2STATE, arg) )
        {
            ret = ProcessXlink2State(config, errStr, errStrLen, &saveptr);
        }
        else if ( !strcasecmp(CONF_LOG_FILENAME, arg) )
        {
            config->log_config.log_filename = 1;
        }
        else if ( !strcasecmp(CONF_LOG_MAIL_FROM, arg) )
        {
            config->log_config.log_mailfrom = 1;
        }
        else if ( !strcasecmp(CONF_LOG_RCPT_TO, arg) )
        {
            config->log_config.log_rcptto = 1;
        }
        else if ( !strcasecmp(CONF_LOG_EMAIL_HDRS, arg) )
        {
            config->log_config.log_email_hdrs = 1;
        }
        else if ( !strcasecmp(CONF_EMAIL_HDRS_LOG_DEPTH, arg) )
        {
            ret = ProcessLogDepth(config, errStr, errStrLen, &saveptr);
        }

        else if ( !strcasecmp(CONF_PRINT_CMDS, arg) )
        {
            config->print_cmds = 1;
        }

        else if(!_dpd.fileAPI->parse_mime_decode_args(&(config->decode_conf), arg, "SMTP", &saveptr))
        {
            ret = 0;
        }
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Unknown SMTP configuration option %s\n",
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

    // NOTE: the default b64_depth is not defined in this file 
    //       but is equal to DEFAULT_MAX_MIME_DEPTH
    if(config->decode_conf.b64_depth == DEFAULT_MAX_MIME_DEPTH)
    {
        if(config->enable_mime_decoding)
            config->decode_conf.b64_depth = config->max_mime_depth;
    }
    else if(deprecated_options)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Cannot specify 'enable_mime_decoding' or 'max_mime_depth' with "
                                       "'b64_decode_depth'\n",
                                     *(_dpd.config_file), *(_dpd.config_line), arg);
    }

    if(!config->log_config.email_hdrs_log_depth)
    {
        if(config->log_config.log_email_hdrs)
        {
            _dpd.logMsg("WARNING: %s(%d) => 'log_email_hdrs' enabled with 'email_hdrs_log_depth' = 0."
                    "Email headers won't be logged. Please set 'email_hdrs_log_depth' > 0 to enable logging.\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }
        config->log_config.log_email_hdrs = 0;
    }

}

void SMTP_CheckConfig(SMTPConfig *pPolicyConfig, tSfPolicyUserContextId context)
{
    SMTPConfig *defaultConfig =
                (SMTPConfig *)sfPolicyUserDataGetDefault(context);

    if (pPolicyConfig == defaultConfig)
    {
        if (!_dpd.fileAPI->check_decoding_conf(&(pPolicyConfig->decode_conf),
                &(defaultConfig->decode_conf), "SMTP"))
            return;

        if (!pPolicyConfig->memcap)
            pPolicyConfig->memcap = DEFAULT_SMTP_MEMCAP;

        if(pPolicyConfig->disabled && !pPolicyConfig->log_config.email_hdrs_log_depth)
            pPolicyConfig->log_config.email_hdrs_log_depth = DEFAULT_LOG_DEPTH;

    }
    else if (defaultConfig == NULL)
    {
        _dpd.fileAPI->check_decoding_conf(&(pPolicyConfig->decode_conf),
                        NULL, "SMTP");

        if (pPolicyConfig->memcap)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: memcap must be "
                    "configured in the default config.\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

        if(pPolicyConfig->log_config.log_email_hdrs && pPolicyConfig->log_config.email_hdrs_log_depth)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => SMTP: email_hdrs_log_depth must be "
                    "configured in the default config.\n",
                    *(_dpd.config_file), *(_dpd.config_line));
        }

    }
    else
    {
        pPolicyConfig->memcap = defaultConfig->memcap;
        pPolicyConfig->log_config.email_hdrs_log_depth = defaultConfig->log_config.email_hdrs_log_depth;
        if(pPolicyConfig->disabled)
        {
           pPolicyConfig->decode_conf = defaultConfig->decode_conf;
           return;
        }
        _dpd.fileAPI->check_decoding_conf(&(pPolicyConfig->decode_conf),
                        &(defaultConfig->decode_conf), "SMTP");

    }
}

void SMTP_PrintConfig(SMTPConfig *config)
{
    int i;
    const SMTPToken *cmd;
    char buf[8192];

    if (config == NULL)
        return;

    memset(&buf[0], 0, sizeof(buf));

    _dpd.logMsg("SMTP Config:\n");

    if(config->disabled)
    {
        _dpd.logMsg("    SMTP: INACTIVE\n");
    }

    snprintf(buf, sizeof(buf) - 1, "    Ports: ");

    for(i = 0; i < 65536; i++)
    {
        if( isPortEnabled( config->ports, i ) )
        {
            _dpd.printfappend(buf, sizeof(buf) - 1, "%d ", i);
        }
    }

    _dpd.logMsg("%s\n", buf);

    _dpd.logMsg("    Inspection Type: %s\n",
                config->inspection_type ? "Stateful" : "Stateless");

    snprintf(buf, sizeof(buf) - 1, "    Normalize: ");

    switch (config->normalize)
    {
        case NORMALIZE_ALL:
            _dpd.printfappend(buf, sizeof(buf) - 1, "all");
            break;
        case NORMALIZE_NONE:
            _dpd.printfappend(buf, sizeof(buf) - 1, "none");
            break;
        case NORMALIZE_CMDS:
            if (config->print_cmds)
            {
                for (cmd = config->cmds; cmd->name != NULL; cmd++)
                {
                    if (config->cmd_config[cmd->search_id].normalize)
                    {
                        _dpd.printfappend(buf, sizeof(buf) - 1, "%s ", cmd->name);
                    }
                }
            }
            else
            {
                _dpd.printfappend(buf, sizeof(buf) - 1, "cmds");
            }

            break;
    }

    _dpd.logMsg("%s\n", buf);

    _dpd.logMsg("    Ignore Data: %s\n",
                config->decode_conf.ignore_data ? "Yes" : "No");
    _dpd.logMsg("    Ignore TLS Data: %s\n",
                config->ignore_tls_data ? "Yes" : "No");
    _dpd.logMsg("    Ignore SMTP Alerts: %s\n",
                config->no_alerts ? "Yes" : "No");

    if (!config->no_alerts)
    {
        snprintf(buf, sizeof(buf) - 1, "    Max Command Line Length: ");

        if (config->max_command_line_len == 0)
            _dpd.printfappend(buf, sizeof(buf) - 1, "Unlimited");
        else
            _dpd.printfappend(buf, sizeof(buf) - 1, "%d", config->max_command_line_len);
        _dpd.logMsg("%s\n", buf);

        snprintf(buf, sizeof(buf) - 1, "    Max auth Command Line Length: ");

        _dpd.printfappend(buf, sizeof(buf) - 1, "%d", config->max_auth_command_line_len);

        _dpd.logMsg("%s\n", buf);


        if (config->print_cmds)
        {
            int max_line_len_count = 0;
            int max_line_len = 0;

            snprintf(buf, sizeof(buf) - 1, "    Max Specific Command Line Length: ");

            for (cmd = config->cmds; cmd->name != NULL; cmd++)
            {
                max_line_len = config->cmd_config[cmd->search_id].max_line_len;

                if (max_line_len != 0)
                {
                    if (max_line_len_count % 5 == 0)
                    {
                        _dpd.logMsg("%s\n", buf);
                        snprintf(buf, sizeof(buf) - 1, "       %s:%d ", cmd->name, max_line_len);
                    }
                    else
                    {
                        _dpd.printfappend(buf, sizeof(buf) - 1, "%s:%d ", cmd->name, max_line_len);
                    }

                    max_line_len_count++;
                }
            }

            if (max_line_len_count == 0)
                _dpd.logMsg("%sNone\n", buf);
            else
                _dpd.logMsg("%s\n", buf);
        }

        snprintf(buf, sizeof(buf) - 1, "    Max Header Line Length: ");

        if (config->max_header_line_len == 0)
            _dpd.logMsg("%sUnlimited\n", buf);
        else
            _dpd.logMsg("%s%d\n", buf, config->max_header_line_len);

        snprintf(buf, sizeof(buf) - 1, "    Max Response Line Length: ");

        if (config->max_response_line_len == 0)
            _dpd.logMsg("%sUnlimited\n", buf);
        else
            _dpd.logMsg("%s%d\n", buf, config->max_response_line_len);
    }

    _dpd.logMsg("    X-Link2State Alert: %s\n",
                config->alert_xlink2state ? "Yes" : "No");
    if (config->alert_xlink2state)
    {
        _dpd.logMsg("    Drop on X-Link2State Alert: %s\n",
                    config->drop_xlink2state ? "Yes" : "No");
    }

    if (config->print_cmds && !config->no_alerts)
    {
        int alert_count = 0;

        snprintf(buf, sizeof(buf) - 1, "    Alert on commands: ");

        for (cmd = config->cmds; cmd->name != NULL; cmd++)
        {
            if (config->cmd_config[cmd->search_id].alert)
            {
                _dpd.printfappend(buf, sizeof(buf) - 1, "%s ", cmd->name);
                alert_count++;
            }
        }

        if (alert_count == 0)
        {
            _dpd.logMsg("%sNone\n", buf);
        }
        else
        {
            _dpd.logMsg("%s\n", buf);
        }
    }
    _dpd.logMsg("    Alert on unknown commands: %s\n",
            config->alert_unknown_cmds ? "Yes" : "No");

    _dpd.logMsg("    SMTP Memcap: %u\n",
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
        _dpd.logMsg("    Non-Encoded MIME attachment Extraction/text: %s\n", "Disabled");

    _dpd.logMsg("    Log Attachment filename: %s\n",
                config->log_config.log_filename ? "Enabled" : "Not Enabled");

    _dpd.logMsg("    Log MAIL FROM Address: %s\n",
                config->log_config.log_mailfrom ? "Enabled" : "Not Enabled");

    _dpd.logMsg("    Log RCPT TO Addresses: %s\n",
                config->log_config.log_rcptto ? "Enabled" : "Not Enabled");

    _dpd.logMsg("    Log Email Headers: %s\n",
                config->log_config.log_email_hdrs ? "Enabled" : "Not Enabled");

    if(config->log_config.log_email_hdrs)
    {
        _dpd.logMsg("    Email Hdrs Log Depth: %u\n",
                config->log_config.email_hdrs_log_depth);
    }
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
static int ProcessPorts(SMTPConfig *config, char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    char *pcEnd;
    int  iPort;
    int  iEndPorts = 0;
    int num_ports = 0;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
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
    disablePort( config->ports, SMTP_DEFAULT_SERVER_PORT );
    disablePort( config->ports, XLINK2STATE_DEFAULT_PORT );
    disablePort( config->ports, SMTP_DEFAULT_SUBMISSION_PORT );
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
             "SMTP: Empty port list not allowed.");
        return -1;
    }

    return 0;
}

/*
**  NAME
**    ProcessCmds::
*/
/**
**  Process the command list.
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
*/
static int ProcessCmds(SMTPConfig *config, char *ErrorString,
        int ErrStrLen, char **saveptr, int action, SMTPCmdTypeEnum type)
{
    char *pcToken;
    int   iEndCmds = 0;
    int   id;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if (!pcToken)
    {
        snprintf(ErrorString, ErrStrLen, "Invalid command list format.");
        return -1;
    }

    if (strcmp(CONF_START_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start a command list with the '%s' token.",
                CONF_START_LIST);

        return -1;
    }

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if (strcmp(CONF_END_LIST, pcToken) == 0)
        {
            iEndCmds = 1;
            break;
        }

        id = GetCmdId(config, pcToken, type);

        if (action == ACTION_ALERT)
        {
            config->cmd_config[id].alert = 1;
        }
        else if (action == ACTION_NO_ALERT)
        {
            config->cmd_config[id].alert = 0;
        }
        else if (action == ACTION_NORMALIZE)
        {
            config->cmd_config[id].normalize = 1;
        }
    }

    if (!iEndCmds)
    {
        snprintf(ErrorString, ErrStrLen, "Must end '%s' configuration with '%s'.",
                 action == ACTION_ALERT ? CONF_INVALID_CMDS :
                 (action == ACTION_NO_ALERT ? CONF_VALID_CMDS :
                  (action == ACTION_NORMALIZE ? CONF_NORMALIZE_CMDS : "")),
                 CONF_END_LIST);

        return -1;
    }

    return 0;
}

/* Return id associated with a given command string */
static int GetCmdId(SMTPConfig *config, char *name, SMTPCmdTypeEnum type)
{
    SMTPToken *cmd;

    for (cmd = config->cmds; cmd->name != NULL; cmd++)
    {
        if (strcasecmp(cmd->name, name) == 0)
        {
            if (type && (type != cmd->type))
                cmd->type = type;

            return cmd->search_id;
        }
    }

    return AddCmd(config, name, type);
}


static int AddCmd(SMTPConfig *config, char *name, SMTPCmdTypeEnum type)
{
    SMTPToken *cmds, *tmp_cmds;
    SMTPSearch *cmd_search;
    SMTPCmdConfig *cmd_config;
    int ret;

    if (config == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) SMTP config is NULL.\n",
                                        __FILE__, __LINE__);
    }

    config->num_cmds++;

    /* allocate enough memory for new commmand - alloc one extra for NULL entry */
    cmds = (SMTPToken *)_dpd.snortAlloc(config->num_cmds + 1, sizeof(SMTPToken), PP_SMTP,
                             PP_MEM_CATEGORY_CONFIG);
    if (cmds == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    /* This gets filled in later */
    cmd_search = (SMTPSearch *)_dpd.snortAlloc(config->num_cmds, sizeof(SMTPSearch), PP_SMTP,
                                    PP_MEM_CATEGORY_CONFIG);
    if (cmd_search == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    cmd_config = (SMTPCmdConfig *)_dpd.snortAlloc(config->num_cmds, sizeof(SMTPCmdConfig), PP_SMTP,
                                       PP_MEM_CATEGORY_CONFIG);
    if (cmd_config == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    /* copy existing commands into newly allocated memory
     * don't need to copy anything from cmd_search since this hasn't been initialized yet */
    ret = SafeMemcpy(cmds, config->cmds, (config->num_cmds - 1) * sizeof(SMTPToken),
                     cmds, cmds + (config->num_cmds - 1));

    if (ret != SAFEMEM_SUCCESS)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to memory copy SMTP command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    ret = SafeMemcpy(cmd_config, config->cmd_config, (config->num_cmds - 1) * sizeof(SMTPCmdConfig),
                     cmd_config, cmd_config + (config->num_cmds - 1));

    if (ret != SAFEMEM_SUCCESS)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to memory copy SMTP command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    /* add new command to cmds
     * cmd_config doesn't need anything added - this will probably be done by a calling function
     * cmd_search will be initialized when the searches are initialized */
    tmp_cmds = &cmds[config->num_cmds - 1];
    tmp_cmds->name = strdup(name);
    tmp_cmds->name_len = strlen(name);
    tmp_cmds->search_id = config->num_cmds - 1;
    if (type)
        tmp_cmds->type = type;

    if (tmp_cmds->name == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Failed to allocate memory for SMTP "
                                        "command structure\n",
                                        *(_dpd.config_file), *(_dpd.config_line));
    }

    /* free global memory structures */
    if (config->cmds != NULL)
	_dpd.snortFree(config->cmds, sizeof(*(config->cmds)), PP_SMTP,
             PP_MEM_CATEGORY_CONFIG);

    if (config->cmd_search != NULL)
	_dpd.snortFree(config->cmd_search, sizeof(*(config->cmd_search)), PP_SMTP,
             PP_MEM_CATEGORY_CONFIG);

    if (config->cmd_config != NULL)
	_dpd.snortFree(config->cmd_config, sizeof(*(config->cmd_config)), PP_SMTP,
             PP_MEM_CATEGORY_CONFIG);

    /* set globals to new memory */
    config->cmds = cmds;
    config->cmd_search = cmd_search;
    config->cmd_config = cmd_config;

    return (config->num_cmds - 1);
}

static int ProcessMaxMimeDepth(SMTPConfig *config, char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *endptr;
    char *value;
    int max_mime_depth = 0;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    value = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if ( value == NULL )
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for max_mime_depth.");
        return -1;
    }
    max_mime_depth = strtol(value, &endptr, 10);

    if(*endptr)
    {
        snprintf(ErrorString, ErrStrLen,
            "Invalid format for max_mime_depth.");
        return -1;
    }

    if (max_mime_depth < MIN_MIME_DEPTH || max_mime_depth > MAX_MIME_DEPTH)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid value for max_mime_depth."
                "It should range between %d and %d.",
                MIN_MIME_DEPTH, MAX_MIME_DEPTH);
        return -1;
    }
    if(max_mime_depth & 3)
    {
        max_mime_depth += 4 - (max_mime_depth & 3);
        _dpd.logMsg("WARNING: %s(%d) => SMTP: 'max_mime_depth' is not a multiple of 4. "
             "Rounding up to the next multiple of 4. The new 'max_mime_depth' is %d.\n",
              *(_dpd.config_file), *(_dpd.config_line), max_mime_depth);

    }

    config->max_mime_depth = max_mime_depth;
    return 0;
}

static int ProcessLogDepth(SMTPConfig *config, char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *endptr;
    char *value;
    uint32_t log_depth = 0;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    value = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if ( value == NULL )
    {
        snprintf(ErrorString, ErrStrLen,
                "Missing value for email_hdrs_log_depth.");
        return -1;
    }
    log_depth = strtoul(value, &endptr, 10);

    if((value[0] == '-') || (*endptr != '\0'))
    {
        snprintf(ErrorString, ErrStrLen,
            "Invalid format '%s' for email_hdrs_log_depth.",
            value);
        return -1;
    }

    if(log_depth && log_depth < MIN_LOG_DEPTH)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid value for email_hdrs_log_depth."
                "It should range between %d and %d.",
                MIN_LOG_DEPTH, MAX_LOG_DEPTH);
        return -1;
    }
    else if (log_depth > MAX_LOG_DEPTH)
    {
        _dpd.logMsg("WARNING: %s(%d) => Invalid value for email_hdrs_log_depth. "
                "It should range between %d and %d. The email_hdrs_log_depth "
                "will be reduced to the max value.\n", *(_dpd.config_file), *(_dpd.config_line),
                MIN_LOG_DEPTH, MAX_LOG_DEPTH);

        log_depth = MAX_LOG_DEPTH;
    }

    /* Rounding the log depth to a multiple of 8 since
     * multiple sessions use the same mempool
     *
     * Moved from spp_smtp.c
     */
    if (log_depth & 7)
        log_depth += (8 - (log_depth & 7));

    config->log_config.email_hdrs_log_depth = log_depth;
    return 0;
}


/*
**  NAME
**    ProcessAltMaxCmdLen::
*/
/**
**
**   alt_max_command_line_len <int> { <cmd> [<cmd>] }
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
*/
static int ProcessAltMaxCmdLen(SMTPConfig *config, char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    char *pcLen;
    char *pcLenEnd;
    int   iEndCmds = 0;
    int   id;
    int   cmd_len;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    /* Find number */
    pcLen = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if (!pcLen)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for alt_max_command_line_len.");

        return -1;
    }

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if (!pcToken)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for alt_max_command_line_len.");

        return -1;
    }

    cmd_len = strtoul(pcLen, &pcLenEnd, 10);
    if (pcLenEnd == pcLen)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid format for alt_max_command_line_len (non-numeric).");

        return -1;
    }

    if (strcmp(CONF_START_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start alt_max_command_line_len list with the '%s' token.",
                CONF_START_LIST);

        return -1;
    }

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if (strcmp(CONF_END_LIST, pcToken) == 0)
        {
            iEndCmds = 1;
            break;
        }

        id = GetCmdId(config, pcToken, SMTP_CMD_TYPE_NORMAL);

        config->cmd_config[id].max_line_len = cmd_len;
    }

    if (!iEndCmds)
    {
        snprintf(ErrorString, ErrStrLen,
                "Must end alt_max_command_line_len configuration with '%s'.", CONF_END_LIST);

        return -1;
    }

    return 0;
}


/*
**  NAME
**    ProcessXlink2State::
*/
/**
**
**   xlink2state { <enable/disable> <drop> }
**
**  @param ErrorString error string buffer
**  @param ErrStrLen   the length of the error string buffer
**
**  @return an error code integer
**          (0 = success, >0 = non-fatal error, <0 = fatal error)
**
**  @retval  0 successs
**  @retval -1 generic fatal error
*/
static int ProcessXlink2State(SMTPConfig *config, char *ErrorString, int ErrStrLen, char **saveptr)
{
    char *pcToken;
    int  iEnd = 0;

    if (config == NULL)
    {
        snprintf(ErrorString, ErrStrLen, "SMTP config is NULL.\n");
        return -1;
    }

    pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr);
    if(!pcToken)
    {
        snprintf(ErrorString, ErrStrLen,
                "Invalid xlink2state argument format.");

        return -1;
    }

    if(strcmp(CONF_START_LIST, pcToken))
    {
        snprintf(ErrorString, ErrStrLen,
                "Must start xlink2state arguments with the '%s' token.",
                CONF_START_LIST);

        return -1;
    }

    while ((pcToken = strtok_r(NULL, CONF_SEPARATORS, saveptr)) != NULL)
    {
        if(!strcmp(CONF_END_LIST, pcToken))
        {
            iEnd = 1;
            break;
        }

        if ( !strcasecmp(CONF_DISABLE, pcToken) )
        {
            config->alert_xlink2state = 0;
            disablePort( config->ports, XLINK2STATE_DEFAULT_PORT );
        }
        else if ( !strcasecmp(CONF_ENABLE, pcToken) )
        {
            config->alert_xlink2state = 1;
            disablePort( config->ports, XLINK2STATE_DEFAULT_PORT );
        }
        else if ( !strcasecmp(CONF_INLINE_DROP, pcToken) )
        {
            if (!config->alert_xlink2state)
            {
                snprintf(ErrorString, ErrStrLen,
                         "Alerting on X-LINK2STATE must be enabled to drop.");

                return -1;
            }
            config->drop_xlink2state = 1;
        }
    }

    if(!iEnd)
    {
        snprintf(ErrorString, ErrStrLen,
                "Must end '%s' configuration with '%s'.",
                CONF_XLINK2STATE, CONF_END_LIST);

        return -1;
    }

    return 0;
}
