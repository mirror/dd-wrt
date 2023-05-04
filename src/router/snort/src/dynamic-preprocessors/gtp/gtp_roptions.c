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
 * This processes the rule options for this preprocessor
 *
 * Author: Hui Cao
 * Date: 07-25-2011
 ****************************************************************************/

#include <errno.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "gtp_roptions.h"
#include "spp_gtp.h"
#include "sf_types.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "sf_dynamic_engine.h"
#include "sf_snort_plugin_api.h"
#include "sfhashfcn.h"
#include "profiler.h"
#include "gtp_debug.h"
#include "gtp_config.h"
#include "treenodes.h"

#define GTP_ROPT__TYPE         "gtp_type"
#define GTP_ROPT__IE           "gtp_info"
#define GTP_ROPT__VERSION      "gtp_version"

#define GTP_VERSION_0_FLAG        (0x01)
#define GTP_VERSION_1_FLAG        (0x02)
#define GTP_VERSION_2_FLAG        (0x04)

#define GTP_VERSION_ALL_FLAG      (GTP_VERSION_0_FLAG|GTP_VERSION_1_FLAG|GTP_VERSION_2_FLAG)

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static int GTP_TypeInit(struct _SnortConfig *, char *, char *, void **);
static int GTP_TypeEval(void *, const uint8_t **, void *);
static int GTP_IEInit(struct _SnortConfig *, char *, char *, void **);
static int GTP_IEEval(void *, const uint8_t **, void *);
static int GTP_VersionInit(struct _SnortConfig *, char *, char *, void **);
static int GTP_VersionEval(void *, const uint8_t **, void *);

static inline int GTP_RoptDoEval(SFSnortPacket *p)
{
    if ((p->payload_size == 0) ||
            (p->stream_session == NULL) ||
            (!IsUDP(p)))
    {

        DEBUG_WRAP(DebugMessage(DEBUG_GTP, "No payload or no "
                "session pointer or not TCP or UDP - not evaluating.\n"));
        return 0;
    }

    return 1;
}

/*gtp type can be numbers*/
static bool GTP_AddTypeByNumer(GTP_TypeRuleOptData *sdata, char *tok)
{
    char *endStr = NULL;
    unsigned long gtpType;

    gtpType = _dpd.SnortStrtoul(tok, &endStr, 10);

    if ( *endStr)
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                "Please specify an integer between %d and %d, OR a correct name.\n",
                *(_dpd.config_file), *(_dpd.config_line),
                GTP_ROPT__TYPE, MIN_GTP_TYPE_CODE, MAX_GTP_TYPE_CODE);
    }

    if ((gtpType > MAX_GTP_TYPE_CODE) || (errno == ERANGE))
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Value specified for %s is out of "
                "bounds.  Please specify an integer between %d and %d, OR a correct name.\n",
                *(_dpd.config_file), *(_dpd.config_line),
                GTP_ROPT__TYPE, MIN_GTP_TYPE_CODE, MAX_GTP_TYPE_CODE);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "Rule GTP type: %d.\n",gtpType));

    sdata->types[gtpType] = GTP_VERSION_ALL_FLAG;

    return true;
}

/*gtp type can be names*/
static bool GTP_AddTypeByKeword(GTP_TypeRuleOptData *sdata, char *name)
{
    GTP_MsgType *msgType;
    int i;
    bool found = false;

    for( i = 0; i < MAX_GTP_VERSION_CODE + 1; i++)
    {
        if (NULL != (msgType = GetMsgTypeByName((uint8_t)i, name)))
        {
            sdata->types[msgType->type] |= 1 << i;
            found = true;
        }
    }
    return found;
}

/* Parsing for the rule option */
static int GTP_TypeInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
    char *nextPara = NULL;
    char *tok;
    GTP_TypeRuleOptData *sdata;

    if (strcasecmp(name, GTP_ROPT__TYPE) != 0)
        return 0;

    /* Must have arguments */
    if (_dpd.SnortIsStrEmpty(params))
    {
        DynamicPreprocessorFatalMessage("%s(%d) => missing argument to gtp_type keyword\n",
                *(_dpd.config_file), *(_dpd.config_line));
    }

    tok = strtok_r(params, ",", &nextPara);

    if(!tok)
        DynamicPreprocessorFatalMessage("%s(%d) => missing argument to gtp_type keyword\n",
                *(_dpd.config_file), *(_dpd.config_line));

    sdata = (GTP_TypeRuleOptData *)calloc(1, sizeof(*sdata));

    if (sdata == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for the "
                "gtp preprocessor rule option.\n");
    }

    while (NULL != tok)
    {

        bool found;

        if ( isdigit(*tok))
        {
           found = GTP_AddTypeByNumer(sdata, tok);

        }
        else /*check keyword*/
        {
           found = GTP_AddTypeByKeword(sdata, tok);

        }

        if (! found )
        {
            DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                    "Please specify an integer between %d and %d, OR a correct name.\n",
                    *(_dpd.config_file), *(_dpd.config_line),
                    GTP_ROPT__TYPE, MIN_GTP_TYPE_CODE, MAX_GTP_TYPE_CODE);

        }
        tok = strtok_r(NULL, ", ", &nextPara);
    }

    *data = (void *)sdata;
    return 1;

}

/* Rule option evaluation */
static int GTP_TypeEval(void *pkt, const uint8_t **cursor, void *data)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    GTPData *sd;
    GTP_Roptions *ropts;
    GTP_TypeRuleOptData *sdata = (GTP_TypeRuleOptData *)data;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "Evaluating \"%s\" rule option.\n", GTP_ROPT__TYPE));

    if (!GTP_RoptDoEval(p))
        return RULE_NOMATCH;

    sd = (GTPData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_GTP);

    if (sd == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP,
                "No session data - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    ropts = &sd->ropts;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "GTP type in packet: %d \n", ropts->gtp_type));

    /*Match the GTP type*/
    if ((1 << ropts->gtp_version) & sdata->types[ropts->gtp_type])
        return RULE_MATCH;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "Rule No Match\n"));
    return RULE_NOMATCH;
}

/*gtp information element can be number*/
static bool GTP_AddInfoElementByNumer(GTP_InfoRuleOptData *sdata, char *tok)
{
    char *end = NULL;
    unsigned long gtpIE;
    int i;

    gtpIE =  _dpd.SnortStrtoul(tok, &end, 10);

    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "Rule GTP information element: %d.\n",gtpIE));

    if ( *end)
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                "Please specify an integer between %d and %d, OR a correct name.\n",
                *(_dpd.config_file), *(_dpd.config_line),
                GTP_ROPT__IE, MIN_GTP_IE_CODE, MAX_GTP_IE_CODE);
    }

    if ((gtpIE > MAX_GTP_IE_CODE) || (errno == ERANGE))
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Value specified for %s is out of "
                "bounds. Please specify an integer between %d and %d,"
                "OR a correct name.\n ",
                *(_dpd.config_file), *(_dpd.config_line),
                GTP_ROPT__IE, MIN_GTP_IE_CODE, MAX_GTP_IE_CODE);
    }


    for( i = 0; i < MAX_GTP_VERSION_CODE + 1; i++)
    {
        sdata->types[i] = (uint8_t)gtpIE;
    }

    return true;

}

/*gtp information element can be name*/
static bool GTP_AddInfoElementByKeyword(GTP_InfoRuleOptData *sdata, char *name)
{

    int i;
    bool found = false;
    GTP_InfoElement* infoElement;

    for( i = 0; i < MAX_GTP_VERSION_CODE + 1; i++)
    {
        if (NULL != (infoElement = GetInfoElementByName((uint8_t)i, name)))
        {
            sdata->types[i] = infoElement->type;
            found = true;
        }
    }
    return found;
}

/* Parsing for the rule option */
static int GTP_IEInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
    char *nextPara = NULL;
    char *tok;
    GTP_InfoRuleOptData *sdata;
    bool found = false;


    if (strcasecmp(name, GTP_ROPT__IE) != 0)
        return 0;

    /* Must have arguments */
    if (_dpd.SnortIsStrEmpty(params))
    {
        DynamicPreprocessorFatalMessage("%s(%d) => missing argument to %s keyword\n",
                *(_dpd.config_file), *(_dpd.config_line), GTP_ROPT__IE);
    }

    tok = strtok_r(params, ",", &nextPara);

    if(!tok)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => missing argument to %s keyword\n",
                *(_dpd.config_file), *(_dpd.config_line), GTP_ROPT__IE);
    }
    sdata = (GTP_InfoRuleOptData *)calloc(1, sizeof(*sdata));

    if (sdata == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for the "
                "gtp preprocessor rule option.\n");
    }

    if ( isdigit(*tok))
    {
       found = GTP_AddInfoElementByNumer(sdata, tok);

    }
    else
    {
        found = GTP_AddInfoElementByKeyword(sdata, tok);

    }

    if (! found )
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                "Please specify an integer between %d and %d, OR a correct name.\n",
                *(_dpd.config_file), *(_dpd.config_line),
                GTP_ROPT__IE, MIN_GTP_IE_CODE, MAX_GTP_IE_CODE);

    }


    if (!_dpd.SnortIsStrEmpty(nextPara))
    {
        /* Must have only 1 argument*/
        DynamicPreprocessorFatalMessage("%s, %s(%d) => rule option: This option has no arguments.\n",
                GTP_ROPT__IE, *(_dpd.config_file), *(_dpd.config_line));

    }

    *data = (void *)sdata;
    return 1;

}

/* Rule option evaluation */
static int GTP_IEEval(void *pkt, const uint8_t **cursor, void *data)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    GTPData *sd;
    GTP_Roptions *ropts;
    GTP_InfoRuleOptData *ie;
    uint8_t ieType;
    GTP_IEData *ieData;

    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "Evaluating \"%s\" rule option.\n", GTP_ROPT__IE));

    if (!GTP_RoptDoEval(p))
        return RULE_NOMATCH;

    sd = (GTPData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_GTP);

    if (sd == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP,
                "No session data - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    ropts = &sd->ropts;

    if (NULL == ropts->gtp_infoElements)
        return RULE_NOMATCH;

    /*Match the status code*/
    ie = (GTP_InfoRuleOptData *)data;
    ieType = ie->types[ropts->gtp_version];
    if (!ieType)
    {
        return RULE_NOMATCH;
    }

    ieData = &ropts->gtp_infoElements[ieType];

    /*if the data is up to date*/
    if (ieData->msg_id == ropts->msg_id)
    {
        *cursor = ieData->shift + (uint8_t *)ropts->gtp_header;
        DEBUG_WRAP(DebugMessage(DEBUG_GTP,
                "Setting cursor to IE data: %p.\n", *cursor));
        /*Limit the length*/
        _dpd.SetAltDetect((uint8_t *)*cursor, ieData->length);
        return RULE_MATCH;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "Rule No Match\n"));
    return RULE_NOMATCH;
}

/* Parsing for the rule option */
static int GTP_VersionInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
    char *end = NULL;
    char *nextPara = NULL;
    char *tok;
    uint8_t *sdata;
    unsigned long gtpVersion;

    if (strcasecmp(name, GTP_ROPT__VERSION) != 0)
        return 0;

    /* Must have arguments */
    if (_dpd.SnortIsStrEmpty(params))
    {
        DynamicPreprocessorFatalMessage("%s(%d) => missing argument to %s keyword\n",
                *(_dpd.config_file), *(_dpd.config_line), GTP_ROPT__VERSION);
    }

    tok = strtok_r(params, ",", &nextPara);

    if(!tok)
    {
        DynamicPreprocessorFatalMessage("%s(%d) => missing argument to %s keyword\n",
                *(_dpd.config_file), *(_dpd.config_line), GTP_ROPT__VERSION);
    }

    sdata = (uint8_t *)calloc(1, sizeof(*sdata));

    if (sdata == NULL)
    {
        DynamicPreprocessorFatalMessage("Could not allocate memory for the "
                "gtp preprocessor rule option.\n");
    }


    gtpVersion =  _dpd.SnortStrtoul(tok,  &end, 10);
    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "Rule GTP version: %d.\n",gtpVersion));
    if ( *end)
    {
        DynamicPreprocessorFatalMessage(" %s(%d) => Bad value specified for %s. "
                "Please specify an integer between %d and %d.\n",
                *(_dpd.config_file), *(_dpd.config_line),
                GTP_ROPT__VERSION, MIN_GTP_VERSION_CODE, MAX_GTP_VERSION_CODE);
    }
    if ((gtpVersion > MAX_GTP_VERSION_CODE) || (errno == ERANGE))
    {
        DynamicPreprocessorFatalMessage("%s(%d) => Value specified for %s is out of "
                "bounds. Please specify an integer between %d and %d\n ",
                *(_dpd.config_file), *(_dpd.config_line),
                GTP_ROPT__VERSION, MIN_GTP_VERSION_CODE, MAX_GTP_VERSION_CODE);
    }
    *sdata = (uint8_t) gtpVersion;

    if (!_dpd.SnortIsStrEmpty(nextPara))
    {
        /* Must have only 1 argument*/
        DynamicPreprocessorFatalMessage("%s, %s(%d) => rule option: This option has only one argument.\n",
                GTP_ROPT__IE, *(_dpd.config_file), *(_dpd.config_line));

    }

    *data = (void *)sdata;
    return 1;

}

/* Rule option evaluation */
static int GTP_VersionEval(void *pkt, const uint8_t **cursor, void *data)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    GTPData *sd;
    GTP_Roptions *ropts;

    uint8_t version = *((uint8_t *)data);

    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "Evaluating \"%s\" rule option.\n", GTP_ROPT__VERSION));

    if (!GTP_RoptDoEval(p))
        return RULE_NOMATCH;

    sd = (GTPData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_GTP);

    if (sd == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_GTP,
                "No session data - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    ropts = &sd->ropts;

    /*Match the status code*/

    if (version == ropts->gtp_version)
    {
        return RULE_MATCH;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_GTP,
            "Rule No Match\n"));
    return RULE_NOMATCH;
}

/********************************************************************
 * Function: GTP_RegRuleOptions
 *
 * Purpose: Register rule options
 *
 * Arguments: void
 *
 * Returns: void
 *
 ********************************************************************/
void GTP_RegRuleOptions(struct _SnortConfig *sc)
{
    _dpd.preprocOptRegister(sc, GTP_ROPT__TYPE, GTP_TypeInit, GTP_TypeEval,
            free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(sc, GTP_ROPT__IE, GTP_IEInit, GTP_IEEval,
            free, NULL, NULL, NULL, NULL);
    _dpd.preprocOptRegister(sc, GTP_ROPT__VERSION, GTP_VersionInit, GTP_VersionEval,
            free, NULL, NULL, NULL, NULL);
}

