/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Author(s):   Andrew R. Baker <andrewb@sourcefire.com>
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

#include <string.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "signature.h"
#include "util.h"
#include "rules.h"
#include "treenodes.h"
#include "mstring.h"
#include "sfutil/sfghash.h"
#include "snort.h"
#include "parser.h"

#ifdef TARGET_BASED
#include "target-based/sftarget_protocol_reference.h"
#endif
#include "parser.h"
#include "sfPolicy.h"

/* for eval and free functions */
#include "detection-plugins/sp_pattern_match.h"

static OptTreeNode *soidOTN = NULL;

/********************* Reference Implementation *******************************/

ReferenceNode * AddReference(SnortConfig *sc, ReferenceNode **head, char *system, char *id)
{
    ReferenceNode *node;

    if ((system == NULL) || (id == NULL) ||
        (sc == NULL) || (head == NULL))
    {
        return NULL;
    }

    /* create the new node */
    node = (ReferenceNode *)SnortAlloc(sizeof(ReferenceNode));

    /* lookup the reference system */
    node->system = ReferenceSystemLookup(sc->references, system);
    if (node->system == NULL)
        node->system = ReferenceSystemAdd(&sc->references, system, NULL);

    node->id = SnortStrdup(id);

    /* Add the node to the front of the list */
    node->next = *head;
    *head = node;

    return node;
}

/* print a reference node */
void FPrintReference(FILE *fp, ReferenceNode *ref_node)
{
    if ((fp == NULL) || (ref_node == NULL))
        return;

    if (ref_node->system != NULL)
    {
        if(ref_node->system->url)
        {
            fprintf(fp, "[Xref => %s%s]", ref_node->system->url,
                    ref_node->id);
        }
        else
        {
            fprintf(fp, "[Xref => %s %s]", ref_node->system->name,
                    ref_node->id);
        }
    }
    else
    {
        fprintf(fp, "[Xref => %s]", ref_node->id);
    }
}

/********************* End of Reference Implementation ************************/

/********************** Reference System Implementation ***********************/

ReferenceSystemNode * ReferenceSystemAdd(ReferenceSystemNode **head, char *name, char *url)
{
    ReferenceSystemNode *node;

    if (name == NULL)
    {
        ErrorMessage("NULL reference system name\n");
        return NULL;
    }

    if (head == NULL)
        return NULL;

    /* create the new node */
    node = (ReferenceSystemNode *)SnortAlloc(sizeof(ReferenceSystemNode));

    node->name = SnortStrdup(name);
    if (url != NULL)
        node->url = SnortStrdup(url);

    /* Add to the front of the list */
    node->next = *head;
    *head = node;

    return node;
}

ReferenceSystemNode * ReferenceSystemLookup(ReferenceSystemNode *head, char *name)
{
    if (name == NULL)
        return NULL;

    while (head != NULL)
    {
        if (strcasecmp(name, head->name) == 0)
            break;

        head = head->next;
    }

    return head;
}


/****************** End of Reference System Implementation ********************/

/************************ Class/Priority Implementation ***********************/

/* NOTE:  This lookup can only be done during parse time */
ClassType * ClassTypeLookupByType(SnortConfig *sc, char *type)
{
    ClassType *node;

    if (sc == NULL)
        FatalError("%s(%d) Snort config is NULL.\n", __FILE__, __LINE__);

    if (type == NULL)
        return NULL;

    node = sc->classifications;

    while (node != NULL)
    {
        if (strcasecmp(type, node->type) == 0)
            break;

        node = node->next;
    }

    return node;
}

/* NOTE:  This lookup can only be done during parse time */
ClassType * ClassTypeLookupById(SnortConfig *sc, int id)
{
    ClassType *node;

    if (sc == NULL)
        FatalError("%s(%d) Snort config is NULL.\n", __FILE__, __LINE__);

    node = sc->classifications;

    while (node != NULL)
    {
        if (id == node->id)
            break;

        node = node->next;
    }

    return node;
}

OptTreeNode * SoRuleOtnLookup(SFGHASH *so_rule_otn_map, uint32_t gid, uint32_t sid)
{
    OptTreeNode *otn = NULL;
    OtnKey key;

    if (so_rule_otn_map == NULL)
        return NULL;

    key.gid = gid;
    key.sid = sid;

    soidOTN = otn = (OptTreeNode *)sfghash_find(so_rule_otn_map, &key);

    return otn;
}

OptTreeNode * SoRuleOtnLookupNext(uint32_t gid, uint32_t sid)
{
    OptTreeNode * otn = NULL;

    if (soidOTN)
    {
        otn = soidOTN->nextSoid;
        soidOTN = soidOTN->nextSoid;
    }

    return otn;
}

SFGHASH * SoRuleOtnLookupNew(void)
{
    return sfghash_new(10000, sizeof(OtnKey), 0, NULL);
}

void SoRuleOtnLookupAdd(SFGHASH *so_rule_otn_map, OptTreeNode *otn)
{
    if ((so_rule_otn_map == NULL) || (otn == NULL))
        return;

    if (otn->sigInfo.otnKey.gid == 0)
    {
         otn->sigInfo.otnKey.gid = otn->sigInfo.generator;
         otn->sigInfo.otnKey.sid = otn->sigInfo.id;
    }

    if (sfghash_add(so_rule_otn_map, &(otn->sigInfo.otnKey), otn) == SFGHASH_INTABLE)
    {
         OptTreeNode *otn_original = so_rule_otn_map->cnode->data;

         if (!otn_original)
         {
             /* */
             FatalError("Missing Duplicate\n");
         }
         while (otn_original->nextSoid)
         {
             otn_original = otn_original->nextSoid;
         }

         otn_original->nextSoid = otn;
    }
}

void SoRuleOtnLookupFree(SFGHASH *so_rule_otn_map)
{
    if (so_rule_otn_map == NULL)
        return;

    sfghash_delete(so_rule_otn_map);
}

void OtnRemove(SFGHASH *otn_map, SFGHASH *so_rule_otn_map, OptTreeNode *otn)
{
    OtnKey key;

    if (otn == NULL)
        return;

    key.gid = otn->sigInfo.generator;
    key.sid = otn->sigInfo.id;

    if (so_rule_otn_map != NULL)
        sfghash_remove(so_rule_otn_map, &(otn->sigInfo.otnKey));

    if (otn_map != NULL)
        sfghash_remove(otn_map, &key);
}

void OtnDeleteData(void *data)
{
    OptTreeNode *otn = (OptTreeNode *)data;
    OptFpList *opt_func;

    if (otn == NULL)
        return;

    opt_func = otn->opt_func;
    while (opt_func != NULL)
    {
        /* For each of the pattern matcher options in this rule,
         * delete the data associated with it.  This is the only
         * rule option type (as of now) that this is required for since
         * patterns are not added to the hash table (via
         * add_detection_option()) until FinalizeContentUniqueness() is
         * called -- after the duplicate OTN checks.
         *
         * All other rule option types are added to the hash table
         * at parse time, thus the data associated with that rule
         * option is cleaned from the hash table when the table itself
         * is cleaned up.
         */
        OptFpList *tmp = opt_func;

        opt_func = opt_func->next;

        if ((tmp->OptTestFunc == CheckANDPatternMatch) ||
            (tmp->OptTestFunc == CheckUriPatternMatch))
        {
            PatternMatchFree(tmp->context);
        }
    }
}

void OtnFree(void *data)
{
    OptTreeNode *otn = (OptTreeNode *)data;
    OptFpList *opt_func;
    RspFpList *rsp_func;
    ReferenceNode *ref_node;
#ifdef TARGET_BASED
    unsigned int svc_idx;
#endif

    if (otn == NULL)
        return;

    /* If the opt_func list was copied from another OTN, don't free it here */
    if (!otn->sigInfo.dup_opt_func)
    {
        opt_func = otn->opt_func;
        while (opt_func != NULL)
        {
            OptFpList *tmp = opt_func;
            opt_func = opt_func->next;
            free(tmp);
        }
    }

    rsp_func = otn->rsp_func;
    while (rsp_func)
    {
        RspFpList *tmp = rsp_func;
        rsp_func = rsp_func->next;

        // we don't free params here because they should have been
        // passed to add_detection_option() which will ensure the
        // unique ones are freed once.
        free(tmp);
    }

    if (otn->sigInfo.message != NULL)
    {
        if (!otn->generated)
            free((void*)otn->sigInfo.message);
    }
#ifdef TARGET_BASED
    for (svc_idx = 0; svc_idx < otn->sigInfo.num_services; svc_idx++)
    {
        if (otn->sigInfo.services[svc_idx].service)
            free(otn->sigInfo.services[svc_idx].service);
    }
    if (otn->sigInfo.services)
        free(otn->sigInfo.services);
#endif

    ref_node = otn->sigInfo.refs;
    while (ref_node != NULL)
    {
        ReferenceNode *tmp = ref_node;

        ref_node = ref_node->next;
        free(tmp->id);
        free(tmp);
    }

    if (otn->tag != NULL)
        free(otn->tag);

    /* RTN was generated on the fly.  Don't necessarily know which policy
     * at this point so go through all RTNs and delete them */
    if (otn->generated)
    {
        int i;

        for (i = 0; i < otn->proto_node_num; i++)
        {
            RuleTreeNode *rtn = deleteRtnFromOtn(NULL, otn, i);
            if (rtn != NULL)
                free(rtn);
        }
    }

    if (otn->proto_nodes)
        free(otn->proto_nodes);

    if (otn->detection_filter)
        free(otn->detection_filter);

    if (otn->preproc_fp_list != NULL)
        FreePmdList(otn->preproc_fp_list);

    free(otn);
}

SFGHASH * OtnLookupNew(void)
{
    return sfghash_new(10000, sizeof(OtnKey), 0, OtnFree);
}

void OtnLookupAdd(SFGHASH *otn_map, OptTreeNode *otn)
{
    int status;
    OtnKey key;

    if (otn_map == NULL)
        return;

    key.gid = otn->sigInfo.generator;
    key.sid = otn->sigInfo.id;

    status = sfghash_add(otn_map, &key, otn);
    switch (status)
    {
        case SFGHASH_OK:
            /* otn was inserted successfully */
            break;

        case SFGHASH_INTABLE:
            /* Assume it's a rule without an sid */
            if (key.sid == 0)
            {
                ParseError("Duplicate rule with same gid (%u) and no sid.  To "
                           "avoid this, make sure all of your rules define an "
                           "sid.\n", key.gid);
            }
            else
            {
                ParseError("Duplicate rule with same gid (%u) and sid (%u)\n",
                           key.gid, key.sid);
            }

            break;

        case SFGHASH_NOMEM:
            FatalError("Failed to allocate memory for rule.\n");
            break;

        default:
            FatalError("%s(%d): OtnLookupAdd() - unexpected return value "
                       "from sfghash_add().\n", __FILE__, __LINE__);
            break;
    }
}

OptTreeNode * OtnLookup(SFGHASH *otn_map, uint32_t gid, uint32_t sid)
{
    OptTreeNode * otn;
    OtnKey key;

    if (otn_map == NULL)
        return NULL;

    key.gid = gid;
    key.sid = sid;

    otn = (OptTreeNode *)sfghash_find(otn_map, &key);

    return otn;
}

void OtnLookupFree(SFGHASH *otn_map)
{
    if (otn_map == NULL)
        return;

    sfghash_delete(otn_map);
}


/***************** End of Class/Priority Implementation ***********************/

