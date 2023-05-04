/****************************************************************************
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
 ****************************************************************************
 * Provides convenience functions for parsing and querying configuration.
 *
 * 2/17/2011 - Initial implementation ... Hui Cao <hcao@sourcefire.com>
 *
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "sip_config.h"
#include "spp_sip.h"
#include "sip_debug.h"

#define METHOD_NOT_FOUND -1
/*
 * Default SIP port
 */
#define SIP_PORT	5060
#define SIPS_PORT	5061
/*
 * Default values for configurable parameters.
 */
#define SIP_DEFAULT_MAX_SESSIONS  	        10000
#define SIP_DEFAULT_MAX_DIALOGS_IN_SESSION  4
#define SIP_DEFAULT_MAX_URI_LEN  	        256
#define SIP_DEFAULT_MAX_CALL_ID_LEN	        256
#define SIP_DEFAULT_MAX_REQUEST_NAME_LEN	20
#define SIP_DEFAULT_MAX_FROM_LEN	        256
#define SIP_DEFAULT_MAX_TO_LEN	            256
#define SIP_DEFAULT_MAX_VIA_LEN	            1024
#define SIP_DEFAULT_MAX_CONTACT_LEN	        256
#define SIP_DEFAULT_MAX_CONTENT_LEN	        1024

/*
 * Min/Max values for each configurable parameter.
 */
#define MIN_MAX_NUM_SESSION 1024
/*
 * The maximum value that can be used is 4 GB
 * which 4194304 in KB on product. So will be using
 * 1 more than this as maximum limit.
 */
#define MAX_MAX_NUM_SESSION 4194305
#define MIN_MAX_NUM_DIALOG  1
#define MAX_MAX_NUM_DIALOG  4194305
#define MIN_MAX_URI_LEN 0
#define MAX_MAX_URI_LEN 65535
#define MIN_MAX_CALL_ID_LEN 0
#define MAX_MAX_CALL_ID_LEN 65535
#define MIN_MAX_REQUEST_NAME_LEN 0
#define MAX_MAX_REQUEST_NAME_LEN 65535
#define MIN_MAX_FROM_LEN 0
#define MAX_MAX_FROM_LEN 65535
#define MIN_MAX_TO_LEN 0
#define MAX_MAX_TO_LEN 65535
#define MIN_MAX_VIA_LEN 0
#define MAX_MAX_VIA_LEN 65535
#define MIN_MAX_CONTACT_LEN 0
#define MAX_MAX_CONTACT_LEN 65535
#define MIN_MAX_CONTENT_LEN 0
#define MAX_MAX_CONTENT_LEN 65535
/*
 * Keyword strings for parsing configuration options.
 */
#define SIP_DISABLED_KEYWORD			 "disabled"
#define SIP_PORTS_KEYWORD			     "ports"
#define SIP_MAX_SESSION_KEYWORD		     "max_sessions"
#define SIP_MAX_DIALOG_KEYWORD           "max_dialogs"
#define SIP_METHODS_KEYWORD			     "methods"
#define SIP_MAX_URI_LEN_KEYWORD		     "max_uri_len"
#define SIP_MAX_CALL_ID_LEN_KEYWORD		 "max_call_id_len"
#define SIP_MAX_REQUEST_NAME_LEN_KEYWORD "max_requestName_len"
#define SIP_MAX_FROM_LEN_KEYWORD		 "max_from_len"
#define SIP_MAX_TO_LEN_KEYWORD		     "max_to_len"
#define SIP_MAX_VIA_LEN_KEYWORD		     "max_via_len"
#define SIP_MAX_CONTACT_LEN_KEYWORD		 "max_contact_len"
#define SIP_MAX_CONTENT_LEN_KEYWORD		 "max_content_len"
#define SIP_IGNORE_CHANNEL_KEYWORD	     "ignore_call_channel"

#define SIP_SEPERATORS	     "()<>@,;:\\/[]?={}\" "
#define SIP_CONFIG_SECTION_SEPERATORS       ",;"
#define SIP_CONFIG_VALUE_SEPERATORS       " "


/*
 *  method names defined by standard, 14 methods defined up to Mar. 2011
 *  The first 6 methods are standard defined by RFC3261
 */

SIPMethod StandardMethods[] =
{
        {"invite", SIP_METHOD_INVITE},
        {"cancel",SIP_METHOD_CANCEL},
        {"ack", SIP_METHOD_ACK},
        {"bye", SIP_METHOD_BYE},
        {"register", SIP_METHOD_REGISTER},
        {"options",SIP_METHOD_OPTIONS},
        {"refer", SIP_METHOD_REFER},
        {"subscribe", SIP_METHOD_SUBSCRIBE},
        {"update", SIP_METHOD_UPDATE},
        {"join", SIP_METHOD_JOIN},
        {"info", SIP_METHOD_INFO},
        {"message", SIP_METHOD_MESSAGE},
        {"notify", SIP_METHOD_NOTIFY},
        {"prack", SIP_METHOD_PRACK},
        {NULL, SIP_METHOD_NULL}
};

static SIPMethodsFlag currentUseDefineMethod = SIP_METHOD_USER_DEFINE;
/*
 * Function prototype(s)
 */

static void DisplaySIPConfig(SIPConfig *);
static void SIP_SetDefaultMethods(SIPConfig *);
static void SIP_ParsePortList(char **, uint8_t *);
static void SIP_ParseMethods(char **, uint32_t *,SIPMethodlist*);
static SIPMethodNode* SIP_AddMethodToList(char *, SIPMethodsFlag, SIPMethodlist*);
static int SIP_findMethod(char *, SIPMethod *);
static int ParseNumInRange(char *token, char *keyword, int min, int max);

/*
 * Find method from the array methods
 *
 * PARAMETERS:
 * char *token: the method token name to be checked
 * SIPMethod* methods: methods array.
 *
 * RETURNS:
 *  the index of the method in the array, -1 if not found
 */
static int SIP_findMethod(char *token, SIPMethod* methods)
{
    int i = 0;
    while(NULL != methods[i].name)
    {
        if ((strlen(token) == strlen(methods[i].name))&&
                (strncasecmp(methods[i].name, token, strlen(token)) == 0))
            return i;
        i++;
    }
    return METHOD_NOT_FOUND;
}
/* Display the configuration for the SIP preprocessor.
 *
 * PARAMETERS:
 *
 * SIPConfig *config: SIP preprocessor configuration.
 *
 * RETURNS: Nothing.
 */
static void DisplaySIPConfig(SIPConfig *config)
{
    int index;
    int newline;
    SIPMethodNode *method;
    if (config == NULL)
        return;

    _dpd.logMsg("SIP config: \n");
    _dpd.logMsg("    Max number of sessions: %d %s \n",
            config->maxNumSessions,
            config->maxNumSessions
            == SIP_DEFAULT_MAX_SESSIONS ?
                    "(Default)" : "" );
    _dpd.logMsg("    Max number of dialogs in a session: %d %s \n",
            config->maxNumDialogsInSession,
            config->maxNumDialogsInSession
            == SIP_DEFAULT_MAX_DIALOGS_IN_SESSION ?
                    "(Default)" : "" );
    _dpd.logMsg("    Status: %s\n",
            config->disabled ?
                    "DISABLED":"ENABLED");

    if (config->disabled)
        return;

    _dpd.logMsg("    Ignore media channel: %s\n",
            config->ignoreChannel ?
                    "ENABLED":"DISABLED");
    _dpd.logMsg("    Max URI length: %d %s \n",
            config->maxUriLen,
            config->maxUriLen
            == SIP_DEFAULT_MAX_URI_LEN ?
                    "(Default)" : "" );
    _dpd.logMsg("    Max Call ID length: %d %s \n",
            config->maxCallIdLen,
            config->maxCallIdLen
            == SIP_DEFAULT_MAX_CALL_ID_LEN ?
                    "(Default)" : "" );
    _dpd.logMsg("    Max Request name length: %d %s \n",
            config->maxRequestNameLen,
            config->maxRequestNameLen
            == SIP_DEFAULT_MAX_REQUEST_NAME_LEN ?
                    "(Default)" : "" );
    _dpd.logMsg("    Max From length: %d %s \n",
            config->maxFromLen,
            config->maxFromLen
            == SIP_DEFAULT_MAX_FROM_LEN ?
                    "(Default)" : "" );
    _dpd.logMsg("    Max To length: %d %s \n",
            config->maxToLen,
            config->maxToLen
            == SIP_DEFAULT_MAX_TO_LEN ?
                    "(Default)" : "" );
    _dpd.logMsg("    Max Via length: %d %s \n",
            config->maxViaLen,
            config->maxViaLen
            == SIP_DEFAULT_MAX_VIA_LEN ?
                    "(Default)" : "" );
    _dpd.logMsg("    Max Contact length: %d %s \n",
            config->maxContactLen,
            config->maxContactLen
            == SIP_DEFAULT_MAX_CONTACT_LEN ?
                    "(Default)" : "" );
    _dpd.logMsg("    Max Content length: %d %s \n",
            config->maxContentLen,
            config->maxContentLen
            == SIP_DEFAULT_MAX_CONTENT_LEN ?
                    "(Default)" : "" );


    /* Traverse list, printing ports, 5 per line */
    newline = 1;
    _dpd.logMsg("    Ports:\n");
    for(index = 0; index < MAXPORTS; index++)
    {
        if( config->ports[ PORT_INDEX(index) ] & CONV_PORT(index) )
        {
            _dpd.logMsg("\t%d", index);
            if ( !((newline++)% 5) )
            {
                _dpd.logMsg("\n");
            }
        }
    }
    _dpd.logMsg("\n");
    _dpd.logMsg("    Methods:\n");
    _dpd.logMsg("\t%s ",
            config->methodsConfig
            == SIP_METHOD_DEFAULT ?
                    "(Default)" : "");
    method = config->methods;
    while(NULL != method)
    {
        _dpd.logMsg(" %s", method->methodName);
        method = method->nextm;
    }

    _dpd.logMsg("\n");
}

/*
 *  The first 6 methods are standard defined by RFC3261
 *  We use those first 6 methods as default
 *
 */
static void SIP_SetDefaultMethods(SIPConfig *config)
{
    int i;
    config->methodsConfig = SIP_METHOD_DEFAULT;
    for (i = 0; i < 6 ; i++)
    {
        if (SIP_AddMethodToList(StandardMethods[i].name,
                    StandardMethods[i].methodFlag, &config->methods) == NULL)
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Failed to add SIP "
                    "default method: %s.\n", *(_dpd.config_file),
                    *(_dpd.config_line), StandardMethods[i].name);
        }
    }
}

/********************************************************************
 * Function: SIP_ParsePortList()
 *
 * Parses a port list and adds bits associated with the ports
 * parsed to a bit array.
 *
 * Arguments:
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the IP list.
 *  uint8_t *
 *      Pointer to the port array mask to set bits for the ports
 *      parsed.
 *
 * Returns:
 *  SIP_Ret
 *      SIP_SUCCESS if we were able to successfully parse the
 *          port list.
 *      SIP_FAILURE if an error occured in parsing the port list.
 *
 ********************************************************************/
static void SIP_ParsePortList(char **ptr, uint8_t *port_array)
{
    int port;
    char* cur_tokenp = *ptr;
    /* If the user specified ports, remove SIP_PORT for now since
     * it now needs to be set explicitly. */
    port_array[ PORT_INDEX( SIP_PORT ) ] = 0;
    port_array[ PORT_INDEX( SIPS_PORT ) ] = 0;

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Port configurations: %s\n",*ptr ););

    /* Eat the open brace. */
    cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Port token: %s\n",cur_tokenp ););

    /* Check the space after '{'*/
    if (( !cur_tokenp ) || ( 0 != strncmp (cur_tokenp,  "{", 2 )))
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s, make sure space before and after '{'.\n",
                *(_dpd.config_file), *(_dpd.config_line), SIP_PORTS_KEYWORD);
    }

    cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
    while (( cur_tokenp ) && (  0 != strncmp (cur_tokenp,  "}", 2 )))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Port token: %s\n",cur_tokenp ););

        port = ParseNumInRange(cur_tokenp, SIP_PORTS_KEYWORD, 1, MAXPORTS-1);
        port_array[ PORT_INDEX( port ) ] |= CONV_PORT(port);

        cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
    }
    if ( NULL == cur_tokenp )
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s, missing '}'.\n",
                *(_dpd.config_file), *(_dpd.config_line), SIP_PORTS_KEYWORD);
    }
    *ptr = cur_tokenp;
}
/* Parses a single numerical value.
 * A fatal error is made if the parsed value is out of bounds.
 *
 * PARAMETERS:
 *
 * token:       String containing argument
 * keyword:     String containing option's name. Used when printing an error.
 * min:         Minimum value of argument
 * max:         Maximum value of argument
 *
 * RETURNS:     bounds-checked integer value of argument.
 */
static int ParseNumInRange(char *token, char *keyword, int min, int max)
{
    long int value;
    char *str;

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Num token: %s\n",token ););

    if (( !token ) || !isdigit((int)token[0]) )
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                "Please specify an integer  between %d and %d.\n",
                *(_dpd.config_file), *(_dpd.config_line), keyword, min, max);
    }

    value = _dpd.SnortStrtol( token, &str, 10);

    if (0 != strlen(str))
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                "Please specify an integer between %d and %d.\n",
                *(_dpd.config_file), *(_dpd.config_line), keyword, min, max);
    }

    if (value < min || value > max)
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Value specified for %s is out of "
                "bounds.  Please specify an integer between %d and %d.\n",
                *(_dpd.config_file), *(_dpd.config_line), keyword, min, max);
    }

    return value;
}

/********************************************************************
 * Function: SIP_ParseMethods()
 *
 * Parses the methods to detect
 *
 *
 * Arguments:
 *  char **
 *      Pointer to the pointer to the current position in the
 *      configuration line.  This is updated to the current position
 *      after parsing the methods list.
 *  SIPMethods*
 *      Flag for the methods.
 *      NULL flag if not a valid method type
 * Returns:
 *
 ********************************************************************/
static void SIP_ParseMethods(char **ptr, uint32_t *methodsConfig, SIPMethodlist* pmethods)
{
    char* cur_tokenp = *ptr;

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Method configurations: %s\n",*ptr ););
    /* If the user specified methods, remove default methods for now since
     * it now needs to be set explicitly. */
    *methodsConfig =  SIP_METHOD_NULL;
    /* Eat the open brace. */
    cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Method token: %s\n",cur_tokenp ););

    /* Check the space after '{'*/
    if (( !cur_tokenp ) || ( 0 != strncmp (cur_tokenp,  "{", 2 )))
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s, make sure space before and after '{'.\n",
                *(_dpd.config_file), *(_dpd.config_line), SIP_METHODS_KEYWORD);
    }

    cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);

    while (( cur_tokenp ) && (0 != strncmp (cur_tokenp,  "}", 2 )))
    {
        int i_method;
        DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Method token: %s\n",cur_tokenp ););
        // Check whether this is a standard method
        i_method = SIP_findMethod(cur_tokenp, StandardMethods);
        if (METHOD_NOT_FOUND != i_method )
        {
            *methodsConfig |= 1 << (StandardMethods[i_method].methodFlag - 1);
            if (SIP_AddMethodToList(cur_tokenp,
                        StandardMethods[i_method].methodFlag, pmethods) == NULL)
            {
                DynamicPreprocessorFatalMessage(
                        "%s(%d) => Failed to add SIP method: %s.\n",
                        *(_dpd.config_file), *(_dpd.config_line), cur_tokenp);
            }
        }
        else
        {
            if (SIP_AddUserDefinedMethod(cur_tokenp,
                        methodsConfig, pmethods) == NULL)
            {
                DynamicPreprocessorFatalMessage(
                        "%s(%d) => Failed to add user defined SIP method: %s.\n",
                        *(_dpd.config_file), *(_dpd.config_line), cur_tokenp);
            }
        }
        cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);

    }
    if ( NULL == cur_tokenp )
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s, missing '}'.\n",
                *(_dpd.config_file), *(_dpd.config_line), SIP_METHODS_KEYWORD);
    }
    *ptr = cur_tokenp;

}

static SIPMethodNode* SIP_AddMethodToList(char *methodName, SIPMethodsFlag methodConf, SIPMethodlist* p_methodList)
{

    SIPMethodNode* method;
    int methodLen;
    SIPMethodNode* lastMethod;

    if (NULL == methodName)
        return NULL;
    methodLen = strlen(methodName);
    method =*p_methodList;
    lastMethod = *p_methodList;
    while(method)
    {
        // Already in the list, return
        if(strcasecmp(method->methodName, methodName) == 0)
            return method;
        lastMethod = method;
        method =  method->nextm;
    }

    method = (SIPMethodNode *) _dpd.snortAlloc(1, sizeof(SIPMethodNode),
                                        PP_SIP, PP_MEM_CATEGORY_CONFIG);
    if (NULL == method)
        return NULL;
    method->methodName = strdup(methodName);
    if (NULL == method->methodName)
    {
        _dpd.snortFree(method, sizeof(SIPMethodNode),
                       PP_SIP, PP_MEM_CATEGORY_CONFIG);
        return NULL;
    }

    method->methodLen =  methodLen;
    method->methodFlag =  methodConf;
    method->nextm = NULL;
    // The first method, point to the first created one
    if (NULL ==  *p_methodList)
    {
        *p_methodList =  method;
    }
    else
    {
        lastMethod->nextm = method;
    }

    return method;
}
/********************************************************************
 * Function: SIP_FreeConfig
 *
 * Frees a sip configuration
 *
 * Arguments:
 *  SIP_Config *
 *      The configuration to free.
 *
 * Returns: None
 *
 ********************************************************************/
void SIP_FreeConfig (SIPConfig *config)
{
    SIPMethodNode *nextNode;
    SIPMethodNode *curNode;
    if (config == NULL)
        return;
    curNode = config->methods;

    while (NULL != curNode)
    {
        if (NULL != curNode->methodName)
            free(curNode->methodName);
        nextNode = curNode->nextm;
        _dpd.snortFree(curNode, sizeof(SIPMethodNode), PP_SIP, 
                       PP_MEM_CATEGORY_CONFIG);
        curNode = nextNode;
    }
    _dpd.snortFree(config, sizeof(SIPConfig), PP_SIP, 
                   PP_MEM_CATEGORY_CONFIG);
}
/* Parses and processes the configuration arguments
 * supplied in the SIP preprocessor rule.
 *
 * PARAMETERS:
 *
 * SIPConfig *config: SIP preprocessor configuration.
 * argp:              Pointer to string containing the config arguments.
 *
 * RETURNS:     Nothing.
 */
void ParseSIPArgs(SIPConfig *config, u_char* argp)
{
    char* cur_sectionp = NULL;
    char* next_sectionp = NULL;
    char* argcpyp = NULL;

    if (config == NULL)
        return;
    config->maxNumSessions = SIP_DEFAULT_MAX_SESSIONS;
    config->maxNumDialogsInSession = SIP_DEFAULT_MAX_DIALOGS_IN_SESSION;
    config->maxUriLen = SIP_DEFAULT_MAX_URI_LEN;
    config->maxCallIdLen = SIP_DEFAULT_MAX_CALL_ID_LEN;
    config->maxRequestNameLen = SIP_DEFAULT_MAX_REQUEST_NAME_LEN;
    config->maxFromLen = SIP_DEFAULT_MAX_FROM_LEN;
    config->maxToLen = SIP_DEFAULT_MAX_TO_LEN;
    config->maxViaLen = SIP_DEFAULT_MAX_VIA_LEN;
    config->maxContactLen = SIP_DEFAULT_MAX_CONTACT_LEN;
    config->maxContentLen = SIP_DEFAULT_MAX_CONTENT_LEN;

    /* Set up default port to listen on */
    config->ports[ PORT_INDEX( SIP_PORT ) ] |= CONV_PORT(SIP_PORT);
    config->ports[ PORT_INDEX( SIPS_PORT ) ] |= CONV_PORT(SIPS_PORT);

    config->methodsConfig = SIP_METHOD_NULL;
    config->methods = NULL;

    /* Reset user defined method for every policy*/
    currentUseDefineMethod = SIP_METHOD_USER_DEFINE;

    /* Sanity check(s) */
    if ( !argp )
    {
        SIP_SetDefaultMethods(config);
        DisplaySIPConfig(config);
        return;
    }

    argcpyp = strdup( (char*) argp );

    if ( !argcpyp )
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory to parse SIP options.\n");
        return;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "SIP configurations: %s\n",argcpyp ););

    cur_sectionp = strtok_r( argcpyp, SIP_CONFIG_SECTION_SEPERATORS, &next_sectionp);
    DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Arguments token: %s\n",cur_sectionp ););

    while ( cur_sectionp )
    {

        char* cur_config;
        char* cur_tokenp = 	strtok( cur_sectionp, SIP_CONFIG_VALUE_SEPERATORS);

        if (!cur_tokenp)
        {
            cur_sectionp = strtok_r( next_sectionp, SIP_CONFIG_SECTION_SEPERATORS, &next_sectionp);
            continue;
        }

        cur_config = cur_tokenp;

        if ( !strcmp( cur_tokenp, SIP_PORTS_KEYWORD ))
        {
            SIP_ParsePortList(&cur_tokenp, config->ports);

        }
        else if ( !strcmp( cur_tokenp, SIP_METHODS_KEYWORD ))
        {
            SIP_ParseMethods(&cur_tokenp, &config->methodsConfig, &config->methods );

        }
        else if ( !strcmp( cur_tokenp, SIP_DISABLED_KEYWORD ))
        {
            config->disabled = 1;
        }
        else if ( !strcmp( cur_tokenp, SIP_MAX_SESSION_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxNumSessions = (uint32_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_SESSION_KEYWORD,
                    MIN_MAX_NUM_SESSION,
                    MAX_MAX_NUM_SESSION);
        }
        else if ( !strcmp( cur_tokenp, SIP_MAX_DIALOG_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxNumDialogsInSession = (uint32_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_DIALOG_KEYWORD,
                    MIN_MAX_NUM_DIALOG,
                    MAX_MAX_NUM_DIALOG);
        }
        else if ( !strcmp( cur_tokenp, SIP_MAX_URI_LEN_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxUriLen = (uint16_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_URI_LEN_KEYWORD,
                    MIN_MAX_URI_LEN,
                    MAX_MAX_URI_LEN);
        }
        else if ( !strcmp( cur_tokenp, SIP_MAX_CALL_ID_LEN_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxCallIdLen = (uint16_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_CALL_ID_LEN_KEYWORD,
                    MIN_MAX_CALL_ID_LEN,
                    MAX_MAX_CALL_ID_LEN);
        }
        else if ( !strcmp( cur_tokenp, SIP_MAX_REQUEST_NAME_LEN_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxRequestNameLen = (uint16_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_REQUEST_NAME_LEN_KEYWORD,
                    MIN_MAX_REQUEST_NAME_LEN,
                    MAX_MAX_REQUEST_NAME_LEN);
        }
        else if ( !strcmp( cur_tokenp, SIP_MAX_FROM_LEN_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxFromLen = (uint16_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_FROM_LEN_KEYWORD,
                    MIN_MAX_FROM_LEN,
                    MAX_MAX_FROM_LEN);
        }
        else if ( !strcmp( cur_tokenp, SIP_MAX_TO_LEN_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxToLen = (uint16_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_TO_LEN_KEYWORD,
                    MIN_MAX_TO_LEN,
                    MAX_MAX_TO_LEN);
        }

        else if ( !strcmp( cur_tokenp, SIP_MAX_VIA_LEN_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxViaLen = (uint16_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_VIA_LEN_KEYWORD,
                    MIN_MAX_VIA_LEN,
                    MAX_MAX_VIA_LEN);
        }
        else if ( !strcmp( cur_tokenp, SIP_MAX_CONTACT_LEN_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxContactLen = (uint16_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_CONTACT_LEN_KEYWORD,
                    MIN_MAX_CONTACT_LEN,
                    MAX_MAX_CONTACT_LEN);
        }
        else if ( !strcmp( cur_tokenp, SIP_MAX_CONTENT_LEN_KEYWORD ))
        {
            cur_tokenp = strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS);
            config->maxContentLen = (uint16_t)ParseNumInRange(cur_tokenp,
                    SIP_MAX_CONTENT_LEN_KEYWORD,
                    MIN_MAX_CONTENT_LEN,
                    MAX_MAX_CONTENT_LEN);
        }
        else if ( !strcmp( cur_tokenp, SIP_IGNORE_CHANNEL_KEYWORD ))
        {
            config->ignoreChannel = 1;
        }
        else
        {
            DynamicPreprocessorFatalMessage(" %s(%d) => Invalid argument: %s\n",
                    *(_dpd.config_file), *(_dpd.config_line), cur_tokenp);
            return;
        }
        /*Check whether too many parameters*/
        if (NULL != strtok( NULL, SIP_CONFIG_VALUE_SEPERATORS))
        {
            DynamicPreprocessorFatalMessage("%s(%d) => To many arguments: %s\n",
                    *(_dpd.config_file), *(_dpd.config_line), cur_config);

        }
        cur_sectionp = strtok_r( next_sectionp, SIP_CONFIG_SECTION_SEPERATORS, &next_sectionp);
        DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Arguments token: %s\n",cur_sectionp ););
    }
    /*If no methods defined, use the default*/
    if (SIP_METHOD_NULL == config->methodsConfig)
    {
        SIP_SetDefaultMethods(config);
    }
    DisplaySIPConfig(config);
    free(argcpyp);
}
/********************************************************************
 * Function: SIP_AddUserDefinedMethod
 *
 * Add a user defined method
 *
 * Arguments:
 *  char *: the method name
 *  SIPMethodlist *: the list to be added
 *
 * Returns: user defined method
 *
 ********************************************************************/
SIPMethodNode*  SIP_AddUserDefinedMethod(char *methodName, uint32_t *methodsConfig, SIPMethodlist* pmethods)
{

    int i = 0;
    SIPMethodNode* method;

    /*Check whether all the chars are defined by RFC2616*/
    while(methodName[i])
    {
        if (iscntrl(methodName[i])|(NULL != strchr(SIP_SEPERATORS,methodName[i]))| (methodName[i] < 0) )
        {
            DynamicPreprocessorFatalMessage(" %s(%d) => Bad character included in the User defined method: %s."
                    "Make sure space before and after '}'. \n",
                    *(_dpd.config_file), *(_dpd.config_line), methodName );
            return NULL;
        }
        i++;
    }
    if (currentUseDefineMethod > SIP_METHOD_USER_DEFINE_MAX)
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Exceeded max number of user defined methods (%d), can't add %s.\n",
                *(_dpd.config_file), *(_dpd.config_line), SIP_METHOD_USER_DEFINE_MAX - SIP_METHOD_USER_DEFINE + 1,
                methodName );
        return NULL;
    }
    *methodsConfig |= 1 << (currentUseDefineMethod - 1);
    method = SIP_AddMethodToList(methodName, currentUseDefineMethod, pmethods);
    currentUseDefineMethod = (SIPMethodsFlag) (currentUseDefineMethod + 1);
    return method;
}
