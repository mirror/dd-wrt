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
 *
 ****************************************************************************/

#include <errno.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "sip_roptions.h"
#include "spp_sip.h"
#include "sf_types.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "sf_dynamic_engine.h"
#include "sf_snort_plugin_api.h"
#include "sfhashfcn.h"
#include "profiler.h"
#include "sip_utils.h"
#include "sip_debug.h"
#include "sip_config.h"
#include "treenodes.h"

#define SIP_ROPT__METHOD       "sip_method"
#define SIP_ROPT__STATUS_CODE  "sip_stat_code"
#define SIP_ROPT__HEADER       "sip_header"
#define SIP_ROPT__BODY         "sip_body"


/********************************************************************
 * Private function prototypes
 ********************************************************************/
static int SIP_MethodInit(struct _SnortConfig *sc, char *, char *, void **);
static int SIP_MethodEval(void *, const uint8_t **, void *);
static int SIP_HeaderInit(struct _SnortConfig *sc, char *, char *, void **);
static int SIP_HeaderEval(void *, const uint8_t **, void *);
static int SIP_StatCodeInit(struct _SnortConfig *sc, char *, char *, void **);
static int SIP_StatCodeEval(void *, const uint8_t **, void *);
static int SIP_BodyInit(struct _SnortConfig *sc, char *, char *, void **);
static int SIP_BodyEval(void *, const uint8_t **, void *);
static int SIP_MethodAddFastPatterns(void *, int, int, FPContentInfo **);


static inline int SIP_RoptDoEval(SFSnortPacket *p)
{
	if ((p->payload_size == 0) ||
			(p->stream_session == NULL) ||
			(!IsTCP(p) && !IsUDP(p)))
	{

		DEBUG_WRAP(DebugMessage(DEBUG_SIP, "No payload or no "
				"session pointer or not TCP or UDP - not evaluating.\n"));
		return 0;
	}

	return 1;
}

static inline int IsRequest(SIP_Roptions *ropts)
{
	if (ropts->status_code)
		return FALSE;
	else
		return TRUE;
}

/* Parsing for the rule option */
static int SIP_MethodInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{

	int flags = 0, mask = 0;
	char *end = NULL;
	char *tok;
	int negated = 0;
	int numTokens = 0;
	SipMethodRuleOptData *sdata;
	SIPMethodNode *method;
	SIPConfig * sip_parsing_config;

	if (strcasecmp(name, SIP_ROPT__METHOD) != 0)
		return 0;


	/*Evaluate whether all the methods are in the PP configurations */
	sip_parsing_config = getParsingSIPConfig(sc);

	if (NULL == sip_parsing_config)
	    DynamicPreprocessorFatalMessage("%s(%d) => Configuration error!\n",
	            *(_dpd.config_file), *(_dpd.config_line));

	/* Must have arguments */
	if (SIP_IsEmptyStr(params))
	{
	    DynamicPreprocessorFatalMessage("%s(%d) => missing argument to sip_method keyword\n",
	                    *(_dpd.config_file), *(_dpd.config_line));
	}

	tok = strtok_r(params, ",", &end);

	if(!tok)
		DynamicPreprocessorFatalMessage("%s(%d) => missing argument to sip_method keyword\n",
				*(_dpd.config_file), *(_dpd.config_line));

	while (NULL != tok)
	{

	    numTokens++;

	    if (tok[0] == '!')
	    {
	        negated = 1;
	        tok++;
	    }

	    /*Only one method is allowed with !*/
	    if (negated && (numTokens > 1))
	    {
	        DynamicPreprocessorFatalMessage("%s(%d) => %s, only one method is allowed with ! for %s.\n",
	                *(_dpd.config_file), *(_dpd.config_line), tok, name);
	    }
		method = SIP_FindMethod (sip_parsing_config->methods, tok, strlen (tok));

		/*if method is not found, add it as a user defined method*/
		if (NULL == method)
		{
		    method = SIP_AddUserDefinedMethod(tok, &sip_parsing_config->methodsConfig, &sip_parsing_config->methods );
		    if (NULL == method)
		        DynamicPreprocessorFatalMessage("%s(%d) => %s can't add new method to %s.\n",
		                *(_dpd.config_file), *(_dpd.config_line), tok, name);
		    _dpd.logMsg("%s(%d) => Add user defined method: %s to SIP preprocessor through rule.\n",
		            *(_dpd.config_file), *(_dpd.config_line), method->methodName);
		}

		flags |= 1 << (method->methodFlag - 1);
		if (negated)
			mask |= 1 << (method->methodFlag - 1);

		tok = strtok_r(NULL, ", ", &end);

	}

	sdata = (SipMethodRuleOptData *)calloc(1, sizeof(*sdata));
	if (sdata == NULL)
	{
		DynamicPreprocessorFatalMessage("Could not allocate memory for the "
				"sip preprocessor rule option.\n");
	}

	sdata->flags = flags;
	sdata->mask = mask;
	*data = (void *)sdata;
	return 1;

}
/* Rule option evaluation */
static int SIP_MethodEval(void *pkt, const uint8_t **cursor, void *data)
{
	SFSnortPacket *p = (SFSnortPacket *)pkt;
	SIPData *sd;
	SIP_Roptions *ropts;
	SipMethodRuleOptData *sdata = (SipMethodRuleOptData *)data;
    uint32_t methodFlag;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP,
			"Evaluating \"%s\" rule option.\n", SIP_ROPT__METHOD));

	if (!SIP_RoptDoEval(p))
		return RULE_NOMATCH;

	sd = (SIPData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_SIP);
	if (sd == NULL)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP,
				"No session data - not evaluating.\n"));
		return RULE_NOMATCH;
	}

	ropts = &sd->ropts;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP,
			"Rule Flags: %x Data Flags: %x, Mask: %x \n", sdata->flags, ropts->methodFlag, sdata->mask ));
    // Not response
	methodFlag = 1 << (ropts->methodFlag - 1);
	if (IsRequest(ropts) && ((sdata->flags & methodFlag) ^ sdata->mask))
	{
		return RULE_MATCH;
	}
	return RULE_NOMATCH;

}

static int SIP_MethodAddFastPatterns(void *data, int protocol,
		int direction, FPContentInfo **info)
{
    char *sip = "SIP";
    FPContentInfo *method_fp;
    SipMethodRuleOptData *sdata = (SipMethodRuleOptData *)data;
    DEBUG_WRAP(DebugMessage(DEBUG_SIP,
        "Evaluating \"%s\" fast pattern rule option.\n", SIP_ROPT__METHOD));
    if ((sdata == NULL) || (info == NULL))
        return -1;

    if ((protocol != IPPROTO_TCP) && (protocol != IPPROTO_UDP))
        return -1;

    DEBUG_WRAP(DebugMessage(DEBUG_SIP,
        "adding info to \"%s\" fast pattern rule option.\n", SIP_ROPT__METHOD));

    method_fp = (FPContentInfo *)calloc(1,sizeof(FPContentInfo));
    if (NULL == method_fp)
        return -1;

    method_fp->content = (char *)malloc(strlen(sip));
    if (NULL == method_fp->content)
    {
        free(method_fp);
        return -1;
    }

    memcpy(method_fp->content, sip, strlen(sip));
    method_fp->length =  strlen(sip);
    *info = method_fp;
    return 0;
}
/* Parsing for the rule option */
static int SIP_HeaderInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
	if (strcasecmp(name, SIP_ROPT__HEADER) != 0)
		return 0;

	/* Must not have arguments */
	if (!SIP_IsEmptyStr(params))
	{
		DynamicPreprocessorFatalMessage("%s, %s(%d) => rule option: This option has no arguments.\n",
				SIP_ROPT__HEADER, *(_dpd.config_file), *(_dpd.config_line));

	}

	return 1;
}
/* Rule option evaluation */
static int SIP_HeaderEval(void *pkt, const uint8_t **cursor, void *data)
{
	SFSnortPacket *p = (SFSnortPacket *)pkt;
	SIPData *sd;
	SIP_Roptions *ropts;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP,
			"Evaluating \"%s\" rule option.\n", SIP_ROPT__HEADER));

	if (!SIP_RoptDoEval(p))
		return RULE_NOMATCH;

	sd = (SIPData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_SIP);
	if (sd == NULL)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP,
				"No session data - not evaluating.\n"));
		return RULE_NOMATCH;
	}

	ropts = &sd->ropts;

	if (ropts->header_data != NULL)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP,
				"Setting cursor to header data: %p.\n", ropts->header_data));
		*cursor = ropts->header_data;
		//Limit the length
	    _dpd.SetAltDetect((uint8_t *)ropts->header_data, ropts->header_len);

		return RULE_MATCH;
	}
	return RULE_NOMATCH;
}


/* Parsing for the rule option */
static int SIP_StatCodeInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
	char *end = NULL;
	char *tok;
	int i_tok = 0;
	SipStatCodeRuleOptData *sdata;

	if (strcasecmp(name, SIP_ROPT__STATUS_CODE) != 0)
		return 0;

	/* Must have arguments */
	if (SIP_IsEmptyStr(params))
	{
	    DynamicPreprocessorFatalMessage("%s(%d) => missing argument to sip_stat_code keyword\n",
	            *(_dpd.config_file), *(_dpd.config_line));
	}
	tok = strtok_r(params, ",", &end);

	if(!tok)
		DynamicPreprocessorFatalMessage("%s(%d) => missing argument to sip_stat_code keyword\n",
				*(_dpd.config_file), *(_dpd.config_line));

	sdata = (SipStatCodeRuleOptData *)calloc(1, sizeof(*sdata));

	if (sdata == NULL)
	{
		DynamicPreprocessorFatalMessage("Could not allocate memory for the "
				"sip preprocessor rule option.\n");
	}

	while ((NULL != tok) && (i_tok < SIP_NUM_STAT_CODE_MAX))
	{

		unsigned long statCode =  _dpd.SnortStrtoul(tok, NULL, 10);
		DEBUG_WRAP(DebugMessage(DEBUG_SIP,
					"Rule Status code: %d.\n",sdata->stat_codes[i_tok]));
		if ((statCode > MAX_STAT_CODE) ||
				((statCode > NUM_OF_RESPONSE_TYPES - 1) && (statCode < MIN_STAT_CODE)))
		{
			DynamicPreprocessorFatalMessage("%s(%d) => Status code %u specified is not a 3 digit number or 1 - %d\n ",
					*(_dpd.config_file), *(_dpd.config_line), statCode, NUM_OF_RESPONSE_TYPES-1);
		}
		sdata->stat_codes[i_tok] = (uint16_t)statCode;

		tok = strtok_r(NULL, ", ", &end);
		i_tok++;
	}

	if (NULL != tok)
		DynamicPreprocessorFatalMessage("%s(%d) => More than %d argument to sip_stat_code keyword\n",
				*(_dpd.config_file), *(_dpd.config_line), SIP_NUM_STAT_CODE_MAX);


	*data = (void *)sdata;
	return 1;

}
/* Rule option evaluation */
static int SIP_StatCodeEval(void *pkt, const uint8_t **cursor, void *data)
{
	SFSnortPacket *p = (SFSnortPacket *)pkt;
	SIPData *sd;
	SIP_Roptions *ropts;
	SipStatCodeRuleOptData *sdata = (SipStatCodeRuleOptData *)data;
	uint16_t short_code;
    int i_code;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP,
			"Evaluating \"%s\" rule option.\n", SIP_ROPT__STATUS_CODE));

	if (!SIP_RoptDoEval(p))
		return RULE_NOMATCH;

	sd = (SIPData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_SIP);
	if (sd == NULL)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP,
				"No session data - not evaluating.\n"));
		return RULE_NOMATCH;
	}

	ropts = &sd->ropts;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP,
			"Status code in packet: %d \n", ropts->status_code));

   if (0 == ropts->status_code)
    	return RULE_NOMATCH;

    /*Match the status code*/
	short_code = ropts->status_code / 100;
	for(i_code = 0; i_code < SIP_NUM_STAT_CODE_MAX; i_code++)
	{
	   if ((sdata->stat_codes[i_code] == short_code)||
			   (sdata->stat_codes[i_code] == ropts->status_code))
		return RULE_MATCH;
	}
	DEBUG_WRAP(DebugMessage(DEBUG_SIP,
				"Rule No Match\n"));
	return RULE_NOMATCH;
}

/* Parsing for the rule option */
static int SIP_BodyInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{

	if (strcasecmp(name, SIP_ROPT__BODY) != 0)
		return 0;

	/* Must not have arguments */
	if (!SIP_IsEmptyStr(params))
	{
		DynamicPreprocessorFatalMessage("%s, %s(%d) => rule option: This option has no arguments.\n",
				SIP_ROPT__BODY, *(_dpd.config_file), *(_dpd.config_line));

	}

	return 1;
}
/* Rule option evaluation */
static int SIP_BodyEval(void *pkt, const uint8_t **cursor, void *data)
{
	SFSnortPacket *p = (SFSnortPacket *)pkt;
	SIPData *sd;
	SIP_Roptions *ropts;

	DEBUG_WRAP(DebugMessage(DEBUG_SIP,
			"Evaluating \"%s\" rule option.\n", SIP_ROPT__BODY));

	if (!SIP_RoptDoEval(p))
		return RULE_NOMATCH;

	sd = (SIPData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_SIP);
	if (sd == NULL)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP,
				"No session data - not evaluating.\n"));
		return RULE_NOMATCH;
	}

	ropts = &sd->ropts;

	if (ropts->body_data != NULL)
	{
		DEBUG_WRAP(DebugMessage(DEBUG_SIP,
				"Setting cursor to body data: %p.\n", ropts->body_data));
		*cursor = ropts->body_data;
		//Limit the length
	    _dpd.SetAltDetect((uint8_t *)ropts->body_data, ropts->body_len);

		return RULE_MATCH;
	}

	return RULE_NOMATCH;
}
/********************************************************************
 * Function: SIP_RegRuleOptions
 *
 * Purpose: Register rule options
 *
 * Arguments: void
 *
 * Returns: void
 *
 ********************************************************************/
void SIP_RegRuleOptions(struct _SnortConfig *sc)
{
	_dpd.preprocOptRegister(sc, SIP_ROPT__METHOD, SIP_MethodInit, SIP_MethodEval,
			free, NULL, NULL, NULL, SIP_MethodAddFastPatterns);
	_dpd.preprocOptRegister(sc, SIP_ROPT__HEADER, SIP_HeaderInit, SIP_HeaderEval,
			NULL, NULL, NULL, NULL, NULL);
	_dpd.preprocOptRegister(sc, SIP_ROPT__STATUS_CODE, SIP_StatCodeInit, SIP_StatCodeEval,
			free, NULL, NULL, NULL, NULL);
	_dpd.preprocOptRegister(sc, SIP_ROPT__BODY, SIP_BodyInit, SIP_BodyEval,
			NULL, NULL, NULL, NULL, NULL);
}

