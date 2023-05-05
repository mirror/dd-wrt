/*
**  $Id$
**
**  fpcreate.c
**
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**  Copyright (C) 2002-2013 Sourcefire, Inc.
**  Dan Roelker <droelker@sourcefire.com>
**  Marc Norton <mnorton@sourcefire.com>
**
**  NOTES
**  5.7.02 - Initial Checkin. Norton/Roelker
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License Version 2 as
**  published by the Free Software Foundation.  You may not use, modify or
**  distribute this program under any other version of the GNU General
**  Public License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
** 6/13/05 - marc norton
**   Added plugin support for fast pattern match data
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "snort.h"
#include "rules.h"
#include "treenodes.h"
#include "treenodes.h"
#include "parser.h"
#include "fpcreate.h"
#include "fpdetect.h"
#include "sp_pattern_match.h"
#include "sp_icmp_code_check.h"
#include "sp_icmp_type_check.h"
#include "sp_file_data.h"
#include "sp_ip_proto.h"
#include "plugin_enum.h"
#include "util.h"
#include "rules.h"
#include "treenodes.h"
#include "treenodes.h"
#include "parser.h"
#include "target-based/sftarget_reader.h"
#include "mpse.h"
#include "bitop_funcs.h"

#ifdef INTEL_SOFT_CPM
#include "sfutil/intel-soft-cpm.h"
#endif

#include "snort.h"
#include "sp_clientserver.h"
#include "sfutil/sfportobject.h"
#include "sfutil/sfrim.h"
#include "detection_options.h"
#include "sfPolicy.h"
#include "dynamic-plugins/sp_dynamic.h"
#include "dynamic-plugins/sp_preprocopt.h"
#include "dynamic-plugins/sf_dynamic_define.h"

/*
 *  Content flag values
 */
enum
{
    PGCT_NOCONTENT=0,
    PGCT_CONTENT=1,
    PGCT_URICONTENT=2
};

static void fpAddIpProtoOnlyRule(SF_LIST **, OptTreeNode *);
static void fpRegIpProto(uint8_t *, OptTreeNode *);
static int fpCreatePortGroups(SnortConfig *, rule_port_tables_t *);
static void fpDeletePortGroup(void *);
static void fpDeletePMX(void *data);
static int fpGetFinalPattern(FastPatternConfig *fp, PatternMatchData *pmd,
        char **ret_pattern, int *ret_bytes);
static FPContentInfo * GetLongestDynamicContent(FPContentInfo *content_list);
static PatternMatchData * GetDynamicFastPatternPmd(DynamicData *dd, int dd_type);
static inline int IsDynamicContentFpEligible(FPContentInfo *content);
static inline PatternMatchData * DynamicContentToPmd(FPContentInfo *content_info);
static inline void FreeDynamicContentList(FPContentInfo *fplist);
static PatternMatchData * GetLongestPmdContent(OptTreeNode *otn, int type);
static int fpFinishPortGroupRule(SnortConfig *sc, PORT_GROUP *pg, PmType pm_type,
        OptTreeNode *otn, PatternMatchData *pmd, FastPatternConfig *fp);
static int fpFinishPortGroup(SnortConfig *sc, PORT_GROUP *pg, FastPatternConfig *fp);
static int fpAllocPms(SnortConfig *sc, PORT_GROUP *pg, FastPatternConfig *fp);
static int fpAddPortGroupRule(SnortConfig *sc, PORT_GROUP *pg, OptTreeNode *otn, FastPatternConfig *fp);
static int fpAddPortGroupPrmx(PORT_GROUP *pg, OptTreeNode *otn, int cflag);
static inline int IsPmdFpEligible(PatternMatchData *content);
static void PrintFastPatternInfo(OptTreeNode *otn, PatternMatchData *pmd,
        const char *pattern, int pattern_length, PmType pm_type);
static int GetPreprocOptPmdList(OptTreeNode *, PatternMatchData **);
static int UsePreprocOptFastPatterns(PatternMatchData *, PatternMatchData *);

static const char *pm_type_strings[PM_TYPE__MAX] =
{
    "Normal Content",
    "HTTP Uri content",
    "HTTP Header content",
    "HTTP Client body content",
    "HTTP Method content",
};

/*
#define LOCAL_DEBUG
*/

extern rule_index_map_t * ruleIndexMap;
extern int rule_count;

#ifdef TARGET_BASED
#include "target-based/sftarget_protocol_reference.h"

static sopg_table_t * ServicePortGroupTableNew(void)
{
    return (sopg_table_t *)SnortAlloc(sizeof(sopg_table_t));
}

/*
 * Test if this otn is for traffic to the server
 */
static int fpOtnFlowToServer( OptTreeNode * otn )
{
    if( OtnFlowFromClient(otn) )
        return  1;

    if (otn->ds_list[PLUGIN_DYNAMIC])
    {
        DynamicData *dd = (DynamicData *)otn->ds_list[PLUGIN_DYNAMIC];
        DynamicOptionType optType = OPTION_TYPE_FLOWFLAGS;
        int flags = FLOW_TO_SERVER;

        if (dd->hasOptionFunction(dd->contextData, optType, flags))
            return 1;
    }

    return 0;
}
/*
 * Test if this otn is for traffic to the client
 */
static
int fpOtnFlowToClient( OptTreeNode * otn )
{
    if( OtnFlowFromServer(otn) )
        return 1;

    if (otn->ds_list[PLUGIN_DYNAMIC])
    {
        DynamicData *dd = (DynamicData *)otn->ds_list[PLUGIN_DYNAMIC];
        DynamicOptionType optType = OPTION_TYPE_FLOWFLAGS;
        int flags = FLOW_TO_CLIENT;

        if (dd->hasOptionFunction(dd->contextData, optType, flags))
            return 1;
    }

    return 0;
}

#if 0
Not currently used
/*
* Extract the Icmp Type field to determine the PortGroup.
*
* returns :
*   -1 : any, or not an EQ tests
*   >0 : any other ip type
*
*/
static
int GetOtnIcmpType (OptTreeNode * otn )
{
   int                 type;
   IcmpTypeCheckData * IcmpType;

   IcmpType = (IcmpTypeCheckData *)otn->ds_list[PLUGIN_ICMP_TYPE];

   if( IcmpType && (IcmpType->operator == ICMP_TYPE_TEST_EQ) )
   {
       type = IcmpType->icmp_type;
   }
   else
   {
       return -1;
   }

   return -1;
}
#endif

static SFGHASH * alloc_srvmap(void)
{
   SFGHASH *p = sfghash_new(1000,
                            0,
                            0,
                            /*nodes are lists,free them in sfghash_delete*/
                            (void(*)(void*))sflist_free);
   if (p == NULL)
       FatalError("could not allocate a service rule map - no memory?\n");

   return p;
}

static srmm_table_t * ServiceMapNew(void)
{
    srmm_table_t *table = (srmm_table_t *)SnortAlloc(sizeof(srmm_table_t));

    table->tcp_to_srv = alloc_srvmap();
    table->tcp_to_cli = alloc_srvmap();

    table->udp_to_srv = alloc_srvmap();
    table->udp_to_cli = alloc_srvmap();

    table->icmp_to_srv = alloc_srvmap();
    table->icmp_to_cli = alloc_srvmap();

    table->ip_to_srv = alloc_srvmap();
    table->ip_to_cli = alloc_srvmap();

    return table;
}

static void ServiceTableFree(SFGHASH *table)
{
    if (table != NULL)
        sfghash_delete(table);
}

static void ServiceMapFree(srmm_table_t *srvc_map)
{
    if (srvc_map == NULL)
        return;

    ServiceTableFree(srvc_map->tcp_to_srv);
    ServiceTableFree(srvc_map->tcp_to_cli);
    ServiceTableFree(srvc_map->udp_to_srv);
    ServiceTableFree(srvc_map->udp_to_cli);
    ServiceTableFree(srvc_map->icmp_to_srv);
    ServiceTableFree(srvc_map->icmp_to_cli);
    ServiceTableFree(srvc_map->ip_to_srv);
    ServiceTableFree(srvc_map->ip_to_cli);

    free(srvc_map);
}

static SFGHASH * alloc_spgmm(void)
{
   SFGHASH * p;

   /* TODO: keys are ascii service names - for now ! */
   p = sfghash_new(1000, /* # rows in table */
                   0, /* size: of key 0 = ascii, >0 = fixed size */
                   0, /* bool:user keys,  if true just store this pointer, don't copy the key */
                   fpDeletePortGroup);
                   /* ??? Why shouldn't we delete the port groups ??? */
                   //(void(*)(void*))0 /* free nodes are port_groups do not delete here */ );

   if (p == NULL)
       FatalError("could not allocate a service port_group map : no memory?\n");

   return p;
}

static srmm_table_t * ServicePortGroupMapNew(void)
{
    srmm_table_t *table = (srmm_table_t *)SnortAlloc(sizeof(srmm_table_t));

    table->tcp_to_srv = alloc_spgmm();
    table->tcp_to_cli = alloc_spgmm();

    table->udp_to_srv = alloc_spgmm();
    table->udp_to_cli = alloc_spgmm();

    table->icmp_to_srv = alloc_spgmm();
    table->icmp_to_cli = alloc_spgmm();

    table->ip_to_srv = alloc_spgmm();
    table->ip_to_cli = alloc_spgmm();

    return table;
}

static void ServicePortGroupTableFree(SFGHASH *table)
{
#if 0
    SFGHASH_NODE *node;
    PORT_GROUP *pg;

    /* Not sure why we wouldn't want to free the data */
    for (node = sfghash_findfirst(table);
         node != NULL;
         node = sfghash_findnext(table))
    {
        pg = (PORT_GROUP *)node->data;
        if (pg == NULL)
            continue;

        /* XXX XXX (if we need to recycle these) free the PORT_GROUP */
        node->data = NULL;
    }
#endif

    if (table == NULL)
        return;

    sfghash_delete(table);
}

static void ServicePortGroupMapFree(srmm_table_t *srvc_pg_map)
{
    if (srvc_pg_map == NULL)
        return;

    ServicePortGroupTableFree(srvc_pg_map->tcp_to_srv);
    ServicePortGroupTableFree(srvc_pg_map->tcp_to_cli);
    ServicePortGroupTableFree(srvc_pg_map->udp_to_srv);
    ServicePortGroupTableFree(srvc_pg_map->udp_to_cli);
    ServicePortGroupTableFree(srvc_pg_map->icmp_to_srv);
    ServicePortGroupTableFree(srvc_pg_map->icmp_to_cli);
    ServicePortGroupTableFree(srvc_pg_map->ip_to_srv);
    ServicePortGroupTableFree(srvc_pg_map->ip_to_cli);

    free(srvc_pg_map);
}

/*
 * Add the otn to the list stored by the key = servicename.
 *
 * table - table of service/otn-list pairs
 * servicename - ascii service name from rule metadata option
 * otn - rule - may be content,-no-content, or uri-content
 *
 */
static
void ServiceMapAddOtnRaw( SFGHASH * table, char * servicename, OptTreeNode * otn )
{
    SF_LIST * list;
    OptTreeNode * otn_tmp = NULL;

    list = (SF_LIST*) sfghash_find( table, servicename );

    if( !list )
    {
        /* create the list */
        list = sflist_new();
        if( !list )
            FatalError("service_rule_map: could not create a  service rule-list\n");

        /* add the service list to the table */
        if( sfghash_add( table, servicename, list ) != SFGHASH_OK )
        {
            FatalError("service_rule_map: could not add a rule to the rule-service-map\n");
        }
    }

    if( list->tail && (otn_tmp = list->tail->ndata) && (otn_tmp == otn))
       return;

    /* add the rule since this is not duplicate */
    if( sflist_add_tail( list, otn ) )
        FatalError("service_rule_map: could not add a rule to the service rule-list\n");
}
/*
 *  maintain a table of service maps, one for each protocol and direction,
 *  each service map maintains a list of otn's for each service it maps to a
 *  service name.
 */
static int ServiceMapAddOtn(srmm_table_t *srmm, int proto, char *servicename, OptTreeNode *otn)
{
    SFGHASH * to_srv; /* to srv service rule map */
    SFGHASH * to_cli; /* to cli service rule map */

    if( !servicename )
        return 0;

    if(!otn )
        return 0;

    if( proto == IPPROTO_TCP)
    {
        to_srv = srmm->tcp_to_srv;
        to_cli = srmm->tcp_to_cli;
    }
    else if( proto == IPPROTO_UDP)
    {
        to_srv = srmm->udp_to_srv;
        to_cli = srmm->udp_to_cli;
    }
    else if( proto == IPPROTO_ICMP )
    {
        to_srv = srmm->icmp_to_srv;
        to_cli = srmm->icmp_to_cli;
    }
    else if( proto ==  ETHERNET_TYPE_IP )
    {
        to_srv = srmm->ip_to_srv;
        to_cli = srmm->ip_to_cli;
    }
    else
    {
        return 0;
    }

    if( fpOtnFlowToServer(otn) )
    {
        ServiceMapAddOtnRaw( to_srv, servicename, otn );
    }
    else if( fpOtnFlowToClient(otn) )
    {
        ServiceMapAddOtnRaw( to_cli, servicename, otn );
    }
    else /* else add to both sides */
    {
        ServiceMapAddOtnRaw( to_srv, servicename, otn );
        ServiceMapAddOtnRaw( to_cli, servicename, otn );
    }

    return 0;
}
// TARGET_BASED
#endif

/*
**  The following functions are wrappers to the pcrm routines,
**  that utilize the variables that we have intialized by
**  calling fpCreateFastPacketDetection().  These functions
**  are also used in the file fpdetect.c, where we do lookups
**  on the initialized variables.
*/
#ifdef TARGET_BASED
int prmFindRuleGroupIp(PORT_RULE_MAP *prm, int ip_proto, PORT_GROUP **ip_group, PORT_GROUP ** gen)
{
    PORT_GROUP *unused_a;
    PORT_GROUP *unused_b;
    PORT_GROUP *unused_c;
    return prmFindRuleGroup(prm, ip_proto, -1, &unused_a, ip_group, &unused_b, &unused_c, gen);
}

int prmFindRuleGroupIcmp(PORT_RULE_MAP *prm, int type, PORT_GROUP **type_group, PORT_GROUP ** gen)
{
    PORT_GROUP *unused_a;
    PORT_GROUP *unused_b;
    PORT_GROUP *unused_c;
    return prmFindRuleGroup(prm, type, -1, &unused_a, type_group, &unused_b, &unused_c, gen);
}

int prmFindRuleGroupTcp(PORT_RULE_MAP *prm, int dport, int sport, PORT_GROUP ** src, PORT_GROUP **dst,
        PORT_GROUP **nssrc, PORT_GROUP **nsdst, PORT_GROUP ** gen)
{
    return prmFindRuleGroup( prm, dport, sport, src, dst, nssrc, nsdst, gen);
}

int prmFindRuleGroupUdp(PORT_RULE_MAP *prm, int dport, int sport, PORT_GROUP ** src, PORT_GROUP ** dst,
        PORT_GROUP **nssrc, PORT_GROUP **nsdst, PORT_GROUP ** gen)
{
    return prmFindRuleGroup( prm, dport, sport, src, dst, nssrc, nsdst, gen);
}

#else

int prmFindRuleGroupIp(PORT_RULE_MAP *prm, int ip_proto, PORT_GROUP **ip_group, PORT_GROUP ** gen)
{
    PORT_GROUP *unused_a;
    return prmFindRuleGroup(prm, ip_proto, -1, &unused_a, ip_group, gen);
}

int prmFindRuleGroupIcmp(PORT_RULE_MAP *prm, int type, PORT_GROUP **type_group, PORT_GROUP ** gen)
{
    PORT_GROUP *unused_a;
    return prmFindRuleGroup(prm, type, -1, &unused_a, type_group, gen);
}

int prmFindRuleGroupTcp(PORT_RULE_MAP *prm, int dport, int sport, PORT_GROUP ** src, PORT_GROUP **dst, PORT_GROUP ** gen)
{
    return prmFindRuleGroup( prm, dport, sport, src, dst, gen);
}

int prmFindRuleGroupUdp(PORT_RULE_MAP *prm, int dport, int sport, PORT_GROUP ** src, PORT_GROUP ** dst, PORT_GROUP ** gen)
{
    return prmFindRuleGroup( prm, dport, sport, src, dst, gen);
}
#endif // TARGET_BASED

void free_detection_option_root(void **existing_tree)
{
    detection_option_tree_root_t *root;

    if (!existing_tree || !*existing_tree)
        return;

    root = *existing_tree;
    free(root->children);
    free(root);
    *existing_tree = NULL;
}

void free_detection_option_tree(detection_option_tree_node_t *node)
{
    int i;
    for (i=0;i<node->num_children;i++)
    {
        free_detection_option_tree(node->children[i]);
    }
    free(node->children);
    free(node);
}

/* These aren't currently used */
//static int num_trees = 0;
//static int num_nc_trees = 0;
//static int num_dup_trees = 0;
int finalize_detection_option_tree(SnortConfig *sc, detection_option_tree_root_t *root)
{
    detection_option_tree_node_t *node = NULL;
    void *dup_node = NULL;
    int i;

    if (!root)
        return -1;

    for (i=0;i<root->num_children;i++)
    {
        node = root->children[i];
        if (add_detection_option_tree(sc, node, &dup_node) == DETECTION_OPTION_EQUAL)
        {
            free_detection_option_tree(node);
            root->children[i] = (detection_option_tree_node_t *)dup_node;
            //num_dup_trees++;
        }
        else
        {
            //num_trees++;
        }
#ifdef DEBUG_OPTION_TREE
        print_option_tree(root->children[i], 0);
#endif
    }

    return 0;
}

int otn_create_tree(OptTreeNode *otn, void **existing_tree)
{
    detection_option_tree_node_t *node = NULL, *child;
    detection_option_tree_root_t *root = NULL;
    OptFpList *opt_fp = NULL;
    int i;

    if (!existing_tree)
        return -1;

    if (!*existing_tree)
    {
        *existing_tree = SnortAlloc(sizeof(detection_option_tree_root_t));
    }
    root = *existing_tree;
#ifdef PPM_MGR
    root->tree_state = RULE_STATE_ENABLED;
#endif

    opt_fp = otn->opt_func;

    if (!root->children)
    {
        root->num_children++;
        root->children = SnortAlloc(sizeof(detection_option_tree_node_t *) * root->num_children);
    }

    i = 0;
    child = root->children[i];

    /* Build out sub-nodes for each option in the OTN fp list */
    while (opt_fp)
    {
        /* If child node does not match existing option_data,
         * Create a child branch from a given sub-node. */
        void *option_data = opt_fp->context;
        char found_child_match = 0;

        if (opt_fp->type == RULE_OPTION_TYPE_LEAF_NODE)
        {
            opt_fp = opt_fp->next;
            continue;
        }

        /* Don't add contents that are only for use in the
         * fast pattern matcher */
        if ((opt_fp->type == RULE_OPTION_TYPE_CONTENT)
                || (opt_fp->type == RULE_OPTION_TYPE_CONTENT_URI))
        {
            PatternMatchData *pmd = (PatternMatchData *)option_data;
            if (pmd->fp_only)
            {
                opt_fp = opt_fp->next;
                continue;
            }
        }

        if (!child)
        {
            /* No children at this node */
            child = SnortAlloc(sizeof(detection_option_tree_node_t));
            child->option_data = option_data;
            child->option_type = opt_fp->type;
            child->evaluate = opt_fp->OptTestFunc;
            if (!node)
            {
                root->children[i] = child;
            }
            else
            {
                node->children[i] = child;
            }
            child->num_children++;
            child->children = SnortAlloc(sizeof(detection_option_tree_node_t *) * child->num_children);
            child->last_check.is_relative = opt_fp->isRelative;
            if (node && child->last_check.is_relative)
            {
                node->relative_children++;
            }
        }
        else
        {
            if (child->option_data != option_data)
            {
                if (!node)
                {
                    for (i=1;i<root->num_children;i++)
                    {
                        child = root->children[i];
                        if (child->option_data == option_data)
                        {
                            found_child_match = 1;
                            break;
                        }
                    }
                }
                else
                {
                    for (i=1;i<node->num_children;i++)
                    {
                        child = node->children[i];
                        if (child->option_data == option_data)
                        {
                            found_child_match = 1;
                            break;
                        }
                    }
                }
            }
            else
            {
                found_child_match = 1;
            }

            if (found_child_match == 0)
            {
                /* No matching child node, create a new and add to array */
                detection_option_tree_node_t **tmp_children;
                child = SnortAlloc(sizeof(detection_option_tree_node_t));
                child->option_data = option_data;
                child->option_type = opt_fp->type;
                child->evaluate = opt_fp->OptTestFunc;
                child->num_children++;
                child->children = SnortAlloc(sizeof(detection_option_tree_node_t *) * child->num_children);
                child->last_check.is_relative = opt_fp->isRelative;

                if (!node)
                {
                    root->num_children++;
                    tmp_children = SnortAlloc(sizeof(detection_option_tree_node_t *) * root->num_children);
                    memcpy(tmp_children, root->children,
                            sizeof(detection_option_tree_node_t *) * (root->num_children-1));

                    free(root->children);
                    root->children = tmp_children;
                    root->children[root->num_children-1] = child;
                }
                else
                {
                    node->num_children++;
                    tmp_children = SnortAlloc(sizeof(detection_option_tree_node_t *) * node->num_children);
                    memcpy(tmp_children, node->children,
                            sizeof(detection_option_tree_node_t *) * (node->num_children-1));

                    free(node->children);
                    node->children = tmp_children;
                    node->children[node->num_children-1] = child;
                    if (child->last_check.is_relative)
                        node->relative_children++;
                }
            }
        }
        node = child;
        i=0;
        child = node->children[i];
        opt_fp = opt_fp->next;
    }

    /* Append a leaf node that has option data of the SigInfo/otn pointer */
    child = SnortAlloc(sizeof(detection_option_tree_node_t));
    child->option_data = otn;
    child->option_type = RULE_OPTION_TYPE_LEAF_NODE;
    if (!node)
    {
        if (root->children[0])
        {
            detection_option_tree_node_t **tmp_children;
            root->num_children++;
            tmp_children = SnortAlloc(sizeof(detection_option_tree_node_t *) * root->num_children);
            memcpy(tmp_children, root->children,
                    sizeof(detection_option_tree_node_t *) * (root->num_children-1));
            free(root->children);
            root->children = tmp_children;
        }
        root->children[root->num_children-1] = child;
    }
    else
    {
        if (node->children[0])
        {
            detection_option_tree_node_t **tmp_children;
            node->num_children++;
            tmp_children = SnortAlloc(sizeof(detection_option_tree_node_t *) * node->num_children);
            memcpy(tmp_children, node->children,
                    sizeof(detection_option_tree_node_t *) * (node->num_children-1));
            free(node->children);
            node->children = tmp_children;
        }
        node->children[node->num_children-1] = child;
    }

    return 0;
}

static int add_patrn_to_neg_list(void *id, void **list)
{
    PMX *pmx = (PMX *)id;
    NCListNode **ncl = (NCListNode **)list;
    NCListNode *new;

    if ((id == NULL) || (list == NULL))
        return -1;

    new = (NCListNode *)SnortAlloc(sizeof(NCListNode));
    new->pmx = pmx;
    new->next = *ncl;
    *ncl = new;

    return 0;
}

static void neg_list_free(void **list)
{
    NCListNode *ncln;

    if (list == NULL)
        return;

    ncln = (NCListNode *)*list;
    while (ncln != NULL)
    {
        NCListNode *tmp = ncln->next;
        free(ncln);
        ncln = tmp;
    }

    *list = NULL;
}

int pmx_create_tree(struct _SnortConfig *sc, void *id, void **existing_tree)
{
    PMX              *pmx    = NULL;
    RULE_NODE        *rnNode = NULL;
    OptTreeNode      *otn    = NULL;

    if (!existing_tree)
        return -1;

    if (!*existing_tree)
    {
        *existing_tree = SnortAlloc(sizeof(detection_option_tree_root_t));
    }

    if (!id)
    {
        /* NULL input id (PMX *), last call for this pattern state */
        return finalize_detection_option_tree(sc, (detection_option_tree_root_t *)*existing_tree);
    }

    pmx    = (PMX*)id;
    rnNode = (RULE_NODE*)(pmx->RuleNode);
    otn    = (OptTreeNode *)rnNode->rnRuleData;
    return otn_create_tree(otn, existing_tree);
}

/*
**  The following functions deal with the intialization of the
**  detection engine.  These are set through parser.c with the
**  option 'config detection:'.  This functionality may be
**  broken out later into it's own file to separate from this
**  file's functionality.
*/

/*
**  Initialize detection options.
*/

FastPatternConfig * FastPatternConfigNew(void)
{
    FastPatternConfig *fp =
        (FastPatternConfig *)SnortAlloc(sizeof(FastPatternConfig));

    fpSetDefaults(fp);
    return fp;
}

void fpSetDefaults(FastPatternConfig *fp)
{
    if (fp == NULL)
        return;

    memset(fp, 0, sizeof(FastPatternConfig));

    fp->inspect_stream_insert = 1;
    fp->search_method = MPSE_AC_BNFA;
    fp->max_queue_events = 5;
    fp->bleedover_port_limit = 1024;
}

void FastPatternConfigFree(FastPatternConfig *fp)
{
    if (fp == NULL)
        return;

    free(fp);
}


int fpDetectGetSingleRuleGroup(FastPatternConfig *fp)
{
    return fp->portlists_flags & PL_SINGLE_RULE_GROUP;
}
int fpDetectGetBleedOverPortLimit(FastPatternConfig *fp)
{
    return fp->bleedover_port_limit;
}
int fpDetectGetBleedOverWarnings(FastPatternConfig *fp)
{
    return fp->portlists_flags & PL_BLEEDOVER_WARNINGS_ENABLED;
}
int fpDetectGetDebugPrintNcRules(FastPatternConfig *fp)
{
    return fp->portlists_flags & PL_DEBUG_PRINT_NC_DETECT_RULES;
}
int fpDetectGetDebugPrintRuleGroupBuildDetails(FastPatternConfig *fp)
{
    return fp->portlists_flags & PL_DEBUG_PRINT_RULEGROWP_BUILD;
}
int fpDetectGetDebugPrintRuleGroupsCompiled(FastPatternConfig *fp)
{
    return fp->portlists_flags & PL_DEBUG_PRINT_RULEGROUPS_COMPILED;
}
int fpDetectGetDebugPrintRuleGroupsUnCompiled(FastPatternConfig *fp)
{
    return fp->portlists_flags & PL_DEBUG_PRINT_RULEGROUPS_UNCOMPILED;;
}
int fpDetectGetDebugPrintFastPatterns(FastPatternConfig *fp)
{
    return fp->debug_print_fast_pattern;
}
int fpDetectSplitAnyAny(FastPatternConfig *fp)
{
    return fp->split_any_any;
}
void fpDetectSetSingleRuleGroup(FastPatternConfig *fp)
{
    fp->portlists_flags |= PL_SINGLE_RULE_GROUP;
}
void fpDetectSetBleedOverPortLimit(FastPatternConfig *fp, unsigned int n)
{
    fp->bleedover_port_limit = n;
}
void fpDetectSetBleedOverWarnings(FastPatternConfig *fp)
{
    fp->portlists_flags |= PL_BLEEDOVER_WARNINGS_ENABLED;
}
void fpDetectSetDebugPrintNcRules(FastPatternConfig *fp)
{
    fp->portlists_flags |= PL_DEBUG_PRINT_NC_DETECT_RULES;
}
void fpDetectSetDebugPrintRuleGroupBuildDetails(FastPatternConfig *fp)
{
    fp->portlists_flags |= PL_DEBUG_PRINT_RULEGROWP_BUILD;
}
void fpDetectSetDebugPrintRuleGroupsCompiled(FastPatternConfig *fp)
{
    fp->portlists_flags |= PL_DEBUG_PRINT_RULEGROUPS_COMPILED;
}
void fpDetectSetDebugPrintRuleGroupsUnCompiled(FastPatternConfig *fp)
{
    fp->portlists_flags |= PL_DEBUG_PRINT_RULEGROUPS_UNCOMPILED;
}
void fpDetectSetDebugPrintFastPatterns(FastPatternConfig *fp, int flag)
{
    fp->debug_print_fast_pattern = flag;
}
void fpSetDetectSearchOpt(FastPatternConfig *fp, int flag)
{
    fp->search_opt = flag;
    if (flag)
        LogMessage("    Search-Method-Optimizations = enabled\n");
}

/*
   Search method is set using:
   config detect: search-method ac-bnfa | ac | ac-full | ac-sparsebands | ac-sparse | ac-banded | ac-std | verbose
*/
int fpSetDetectSearchMethod(FastPatternConfig *fp, char *method)
{
    LogMessage("Detection:\n");

    if( !strcasecmp(method,"ac-std") )
    {
       fp->search_method = MPSE_AC;
       LogMessage("   Search-Method = AC-Std\n");
    }
    else if( !strcasecmp(method,"ac-bnfa-q") ||
             !strcasecmp(method,"ac-bnfa") )
    {
       fp->search_method = MPSE_AC_BNFA_Q;
       LogMessage("   Search-Method = AC-BNFA-Q\n");
    }
    else if( !strcasecmp(method,"ac-bnfa-nq") )
    {
       fp->search_method = MPSE_AC_BNFA;
       LogMessage("   Search-Method = AC-BNFA\n");
    }
    else if( !strcasecmp(method,"ac-q") ||
             !strcasecmp(method,"ac") )
    {
       fp->search_method = MPSE_ACF_Q;
       LogMessage("   Search-Method = AC-Full-Q\n");
    }
    else if( !strcasecmp(method,"ac-split") )
    {
        fp->search_method = MPSE_ACF_Q;
        fp->split_any_any = 1;
        LogMessage("   Search-Method = AC-Full-Q\n");
        LogMessage("    Split Any/Any group = enabled\n");
    }
    else if( !strcasecmp(method,"ac-nq") )
    {
       fp->search_method = MPSE_ACF;
       LogMessage("   Search-Method = AC-Full\n");
    }
    else if( !strcasecmp(method,"acs") )
    {
       fp->search_method = MPSE_ACS;
       LogMessage("   Search-Method = AC-Sparse\n");
    }
    else if( !strcasecmp(method,"ac-banded") )
    {
       fp->search_method = MPSE_ACB;
       LogMessage("   Search-Method = AC-Banded\n");
    }
    else if( !strcasecmp(method,"ac-sparsebands") )
    {
       fp->search_method = MPSE_ACSB;
       LogMessage("   Search-Method = AC-Sparse-Bands\n");
    }
    /* These are for backwards compatability - and will be removed in future releases*/
    else if( !strcasecmp(method,"mwm") )
    {
       fp->search_method = MPSE_LOWMEM;
       LogMessage("   Search-Method = Low-Mem (MWM depracated)\n");
    }
    else if( !strcasecmp(method,"lowmem-q") ||
             !strcasecmp(method,"lowmem") )
    {
       fp->search_method = MPSE_LOWMEM_Q;
       LogMessage("   Search-Method = Low-Mem-Q\n");
    }
    else if( !strcasecmp(method,"lowmem-nq") )
    {
       fp->search_method = MPSE_LOWMEM;
       LogMessage("   Search-Method = Low-Mem\n");
    }
    else if( !strcasecmp(method,"intel-cpm") )
    {
#ifdef INTEL_SOFT_CPM
       fp->search_method = MPSE_INTEL_CPM;
       LogMessage("   Search-Method = Intel CPM\n");
#else
       fp->search_method = MPSE_AC_BNFA_Q;
       LogMessage("   Search-Method = AC-BNFA-Q\n");
#endif
    }
    else
    {
       return -1;
    }

    return 0;
}

void fpDetectSetSplitAnyAny(FastPatternConfig *fp, int enable)
{
    if (enable)
    {
        fp->split_any_any = 1;
        LogMessage("    Split Any/Any group = enabled\n");
    }
    else
    {
        fp->split_any_any = 0;
    }
}

/*
**  Set the debug mode for the detection engine.
*/
void fpSetDebugMode(FastPatternConfig *fp)
{
    fp->debug = 1;
}

/*
**  Revert the detection engine back to not inspecting packets
**  that are going to be rebuilt.
*/
void fpSetStreamInsert(FastPatternConfig *fp)
{
    fp->inspect_stream_insert = 0;
}

/*
**  Sets the maximum number of events to queue up in fpdetect before
**  selecting an event.
*/
void fpSetMaxQueueEvents(FastPatternConfig *fp, unsigned int num_events)
{
    fp->max_queue_events = num_events;
}

/*
**  Sets the maximum length of patterns to be inserted into the
**  pattern matcher used.
*/
void fpSetMaxPatternLen(FastPatternConfig *fp, unsigned int max_len)
{
    if (fp->max_pattern_len != 0)
    {
        LogMessage("WARNING: Maximum pattern length redefined.\n");
    }
    fp->max_pattern_len = max_len;
    LogMessage("    Maximum pattern length = %u\n", max_len);
}

/* FLP_Trim
  *
  * Trim zero byte prefixes, this increases uniqueness
  *
  * returns
  *   length - of trimmed pattern
  *   buff - ptr to new beggining of trimmed buffer
  */
static int FLP_Trim( char * p, int plen, char ** buff )
 {
    int i;
    int size = 0;

    if( !p )
        return 0;

    for(i=0;i<plen;i++)
    {
        if( p[i] != 0 ) break;
    }

    if( i < plen )
        size = plen - i;
    else
        size = 0;

    if( buff && (size==0) )
    {
        *buff = 0;
    }
    else if( buff )
    {
        *buff = &p[i];
    }
    return size;
 }


static inline PatternMatchData * DynamicContentToPmd(FPContentInfo *content_info)
{
    PatternMatchData *pmd;

    if (content_info == NULL)
        return NULL;

    pmd = (PatternMatchData *)SnortAlloc(sizeof(PatternMatchData));
    pmd->pattern_buf = (char *)SnortAlloc(content_info->length);
    memcpy(pmd->pattern_buf, content_info->content, content_info->length);
    pmd->pattern_size = content_info->length;
    pmd->nocase = content_info->noCaseFlag;
    pmd->exception_flag = content_info->exception_flag;
    pmd->fp = content_info->fp;
    pmd->fp_offset = content_info->fp_offset;
    pmd->fp_length = content_info->fp_length;
    pmd->fp_only = content_info->fp_only;
    pmd->http_buffer = content_info->uri_buffer & 0xF;  // FIXTHIS make include

    return pmd;
}

static PatternMatchData * DynamicFpContentsToPmdList(FPContentInfo *fp_list)
{
    FPContentInfo *tmp;
    PatternMatchData *pmd_list = NULL;

    if (fp_list == NULL)
        return NULL;

    for (tmp = fp_list; tmp != NULL; tmp = tmp->next)
    {
        PatternMatchData *pmd = DynamicContentToPmd(tmp);
        if (pmd != NULL)
        {
            /* Add these flags to indicate these patterns are only
             * for the fast pattern matcher */
            pmd->fp = 1;
            pmd->fp_only = 1;

            /* Add to list of pmds */
            (void)AppendPmdToList(&pmd_list, pmd);
        }
    }

    return pmd_list;
}

static inline void FreeDynamicContentList(FPContentInfo *fplist)
{
    while (fplist != NULL)
    {
        FPContentInfo *tmp = fplist->next;
        if (fplist->content != NULL)
            free(fplist->content);
        free(fplist);
        fplist = tmp;
    }
}

static PatternMatchData * GetDynamicFastPatternPmd(DynamicData *dd, int dd_type)
{
    FPContentInfo *dc_list = NULL;

    if (dd == NULL)
        return NULL;

    if (dd->getDynamicContents(dd->contextData, dd_type, &dc_list) == 0)
    {
        PatternMatchData *pmd = DynamicContentToPmd(GetLongestDynamicContent(dc_list));
        FreeDynamicContentList(dc_list);
        return pmd;
    }

    return NULL;
}

static inline int IsDynamicContentFpEligible(FPContentInfo *content)
{
    if (content == NULL)
        return 0;

    if ((content->content != NULL) && (content->length != 0))
    {
        /* Negative contents cannot be considered for dynamic rules since
         * detection option tree evaluation only looks at content types
         * for short circuiting rules found in the pattern matcher and
         * essentially the dynamic rule will always have to be evaluated
         * anyway in the no content tree */
        if (content->exception_flag)
            return 0;

        return 1;
    }

    return 0;
}

static FPContentInfo * GetLongestDynamicContent(FPContentInfo *content_list)
{
    FPContentInfo *content = NULL;
    FPContentInfo *content_zero = NULL;
    FPContentInfo *tmp;
    int max_size = 0;
    int max_zero_size = 0;

    if (content_list == NULL)
        return NULL;

    for (tmp = content_list; tmp != NULL; tmp = tmp->next)
    {
        if (tmp->fp)
            return tmp;

        /* XXX COOKIE contents and some others should not be in the content list
         * See GetDynamicContents() in sf_snort_detection_engine.c */

        if (IsDynamicContentFpEligible(tmp))
        {
            int size = FLP_Trim(tmp->content, tmp->length, NULL);

            /* In case we get all zeros patterns */
            if ((size == 0) && (tmp->length > max_zero_size))
            {
                max_zero_size = tmp->length;
                content_zero = tmp;
            }
            else if (size > max_size)
            {
                max_size = size;
                content = tmp;
            }
        }
    }

    if (content != NULL)
        return content;
    else if (content_zero != NULL)
        return content_zero;

    return NULL;

}

static inline int IsPmdFpEligible(PatternMatchData *content)
{
    if (content == NULL)
        return 0;

    if ((content->pattern_buf != NULL) && (content->pattern_size != 0) && (!content->protected_pattern))
    {
        /* We don't add cookie and some other contents to fast pattern matcher */
        if(content->http_buffer && !IsHttpBufFpEligible(content->http_buffer))
            return 0;

        if (content->exception_flag)
        {
            /* Negative contents can only be considered if they are not relative
             * and don't have any offset or depth.  This is because the pattern
             * matcher does not take these into consideration and may find the
             * content in a non-relevant section of the payload and thus disable
             * the rule when it shouldn't be.
             * Also case sensitive patterns cannot be considered since patterns
             * are inserted into the pattern matcher without case which may
             * lead to false negatives */
            if (content->use_doe || !content->nocase
                    || (content->offset != 0) || (content->depth != 0))
            {
                return 0;
            }
        }

        return 1;
    }

    return 0;
}

static PatternMatchData * GetLongestPmdContent(OptTreeNode *otn, int type)
{
    PatternMatchData *pmd = NULL;
    PatternMatchData *pmd_not = NULL;
    PatternMatchData *pmd_zero = NULL;
    PatternMatchData *pmd_zero_not = NULL;
    OptFpList *ofl;
    int max_size = 0;
    int max_zero_size = 0;
    uint8_t base64_buf_flag = 0;
    uint8_t mime_buf_flag = 0;

    if (otn == NULL)
        return NULL;

    for (ofl = otn->opt_func; ofl != NULL; ofl = ofl->next)
    {
        PatternMatchData *tmp = (PatternMatchData *)ofl->context;
        FileData *filedata;

        switch (ofl->type)
        {
            case RULE_OPTION_TYPE_CONTENT:
                if (type != CONTENT_NORMAL)
                    continue;
                else if(base64_buf_flag || mime_buf_flag)
                    continue;
                break;
            case RULE_OPTION_TYPE_CONTENT_URI:
                base64_buf_flag = 0;
                mime_buf_flag = 0;
                if (type != CONTENT_HTTP)
                    continue;
                break;
            case RULE_OPTION_TYPE_BASE64_DATA:
                base64_buf_flag =1;
                continue;
            case RULE_OPTION_TYPE_PKT_DATA:
                base64_buf_flag = 0;
                mime_buf_flag = 0;
                continue;
            case RULE_OPTION_TYPE_FILE_DATA:
                filedata = (FileData *)ofl->context;
                if(filedata->mime_decode_flag)
                    mime_buf_flag = 1;
                continue;

            default:
                continue;
        }

        if (tmp->fp)
            return tmp;

        if (IsPmdFpEligible(tmp))
        {
            int size = FLP_Trim(tmp->pattern_buf, tmp->pattern_size, NULL);

            /* In case we get all zeros patterns */
            if ((size == 0) && ((int)tmp->pattern_size > max_zero_size))
            {
                if (tmp->exception_flag)
                {
                    pmd_zero_not = tmp;
                }
                else
                {
                    max_zero_size = tmp->pattern_size;
                    pmd_zero = tmp;
                }
            }
            else if (size > max_size)
            {
                if (tmp->exception_flag)
                {
                    pmd_not = tmp;
                }
                else
                {
                    max_size = size;
                    pmd = tmp;
                }
            }
        }
    }

    if (pmd != NULL)
        return pmd;
    else if (pmd_zero != NULL)
        return pmd_zero;
    else if (pmd_not != NULL)
        return pmd_not;
    else if (pmd_zero_not != NULL)
        return pmd_zero_not;

    return NULL;
}

static int GetPreprocOptPmdList(OptTreeNode *otn, PatternMatchData **pmd_list)
{
    OptFpList *ofl;
    int dir = 0;

    if ((otn == NULL) || (pmd_list == NULL))
        return -1;

    if (otn->ds_list[PLUGIN_DYNAMIC] != NULL)
    {
        DynamicData *dd = otn->ds_list[PLUGIN_DYNAMIC];
        FPContentInfo *fp_contents = NULL;

        if (dd->getPreprocFpContents(dd->contextData, &fp_contents) == 0)
        {
            *pmd_list = DynamicFpContentsToPmdList(fp_contents);
            FreeDynamicContentList(fp_contents);

            if (*pmd_list == NULL)
                return -1;

            return 0;
        }

        return -1;
    }

    if (otn->ds_list[PLUGIN_CLIENTSERVER] != NULL)
    {
        ClientServerData *csd = (ClientServerData *)otn->ds_list[PLUGIN_CLIENTSERVER];
        if (csd->from_server)
            dir = PKT_FROM_SERVER;
        else if (csd->from_client)
            dir = PKT_FROM_CLIENT;
    }

    for (ofl = otn->opt_func; ofl != NULL; ofl = ofl->next)
    {
        if (ofl->type == RULE_OPTION_TYPE_PREPROCESSOR)
        {
            FPContentInfo *fp_contents = NULL;

            if (GetPreprocFastPatterns(ofl->context, otn->proto, dir, &fp_contents) == 0)
            {
                PatternMatchData *tmp_list = DynamicFpContentsToPmdList(fp_contents);
                (void)AppendPmdToList(pmd_list, tmp_list);

                FreeDynamicContentList(fp_contents);
            }
        }
    }

    if (*pmd_list == NULL)
        return -1;

    return 0;
}

static int UsePreprocOptFastPatterns(PatternMatchData *pmd, PatternMatchData *preproc_pmds)
{
    int pmd_size;
    PatternMatchData *tmp;

    if ((pmd == NULL) || !IsPmdFpEligible(pmd))
        return 1;

    if (preproc_pmds == NULL)
        return 0;

    pmd_size = FLP_Trim(pmd->pattern_buf, pmd->pattern_size, NULL);

    for (tmp = preproc_pmds; tmp != NULL; tmp = tmp->next)
    {
        int tmp_size = FLP_Trim(tmp->pattern_buf, tmp->pattern_size, NULL);

        /* If both are not contents or both are not not contents,
         * compare pattern length */
        if ((tmp->exception_flag && pmd->exception_flag)
                || (!tmp->exception_flag && !pmd->exception_flag))
        {
            if (tmp_size > pmd_size)
                return 1;
        }

        /* If the preproc pattern is not notted and the content pmd is,
         * use the preproc patterns */
        if (!tmp->exception_flag && pmd->exception_flag)
            return 1;
    }

    return 0;
}

static int fpFinishPortGroupRule(SnortConfig *sc, PORT_GROUP *pg, PmType pm_type,
        OptTreeNode *otn, PatternMatchData *pmd_list, FastPatternConfig *fp)
{
    PMX * pmx;
    RULE_NODE * rn;
    char *pattern;
    int pattern_length;
    PatternMatchData *pmd;
    int pg_type;

    if ((pg == NULL) || (otn == NULL) || (fp == NULL))
        return -1;

    switch (pm_type)
    {
        case PM_TYPE__CONTENT:
            if (pmd_list == NULL)
                return -1;
            pg_type = PGCT_CONTENT;
            break;
        case PM_TYPE__HTTP_URI_CONTENT:
        case PM_TYPE__HTTP_HEADER_CONTENT:
        case PM_TYPE__HTTP_CLIENT_BODY_CONTENT:
            if (pmd_list == NULL)
                return -1;
            pg_type = PGCT_URICONTENT;
            break;
        case PM_TYPE__MAX:
        default:
            if (pmd_list != NULL)
                return -1;
            fpAddPortGroupPrmx(pg, otn, PGCT_NOCONTENT);
            return 0;  /* Not adding any content to pattern matcher */
    }

    for (pmd = pmd_list; pmd != NULL; pmd = pmd->next)
    {
        if (pmd->exception_flag)
            fpAddPortGroupPrmx(pg, otn, PGCT_NOCONTENT);
        else
            fpAddPortGroupPrmx(pg, otn, pg_type);

        if (fpGetFinalPattern(fp, pmd, &pattern, &pattern_length) == -1)
            return -1;

        /* create a rule_node */
        rn = (RULE_NODE *)SnortAlloc(sizeof(RULE_NODE));
        rn->rnRuleData = otn;

        /* create pmx */
        pmx = (PMX *)SnortAlloc(sizeof(PMX));
        pmx->RuleNode = rn;
        pmx->PatternMatchData = pmd;

        if (fpDetectGetDebugPrintFastPatterns(fp))
            PrintFastPatternInfo(otn, pmd, pattern, pattern_length, pm_type);

        mpseAddPatternWithSnortConfig(
                sc,
                pg->pgPms[pm_type],
                pattern,
                pattern_length,
                pmd->nocase,
                pmd->offset,
                pmd->depth,
                (unsigned)pmd->exception_flag,
                pmx,
                rn->iRuleNodeID
                );
    }

    return 0;
}

static int fpFinishPortGroup(SnortConfig *sc, PORT_GROUP *pg, FastPatternConfig *fp)
{
    PmType i;
    int rules = 0;

    if ((pg == NULL) || (fp == NULL))
        return -1;

    for (i = PM_TYPE__CONTENT; i < PM_TYPE__MAX; i++)
    {
        if (pg->pgPms[i] != NULL)
        {
            if (mpseGetPatternCount(pg->pgPms[i]) != 0)
            {
                if (mpsePrepPatternsWithSnortConf(sc, pg->pgPms[i], pmx_create_tree,
                            add_patrn_to_neg_list) != 0)
                {
                    FatalError("%s(%d) Failed to compile port group "
                            "patterns.\n", __FILE__, __LINE__);
                }

                if (fp->debug)
                    mpsePrintInfo(pg->pgPms[i]);
                rules = 1;
            }
            else
            {
                mpseFree(pg->pgPms[i]);
                pg->pgPms[i] = NULL;
            }
        }
    }

    if (pg->pgHeadNC != NULL)
    {
        RULE_NODE *ruleNode;

        for (ruleNode = pg->pgHeadNC; ruleNode; ruleNode = ruleNode->rnNext)
        {
            OptTreeNode *otn = (OptTreeNode *)ruleNode->rnRuleData;
            otn_create_tree(otn, &pg->pgNonContentTree);
        }

        finalize_detection_option_tree(sc, (detection_option_tree_root_t*)pg->pgNonContentTree);
        rules = 1;
    }

    if (!rules)
    {
        /* Nothing in the port group so we can just free it */
        free(pg);
        return -1;
    }

    return 0;
}

static int fpAllocPms(SnortConfig *sc, PORT_GROUP *pg, FastPatternConfig *fp)
{
    PmType i;

    for (i = PM_TYPE__CONTENT; i < PM_TYPE__MAX; i++)
    {
        /* init pattern matchers  */
        pg->pgPms[i] = mpseNewWithSnortConfig(sc, fp->search_method,
                MPSE_INCREMENT_GLOBAL_CNT,
                fpDeletePMX,
                free_detection_option_root,
                neg_list_free);

        if (pg->pgPms[i] == NULL)
        {
            PmType j;

            for (j = PM_TYPE__CONTENT; j < i; j++)
            {
                mpseFree(pg->pgPms[j]);
                pg->pgPms[j] = NULL;
            }

            LogMessage("%s(%d) Failed to create pattern matcher for pattern "
                    "matcher type: %d\n", __FILE__, __LINE__, i);

            return -1;
        }

        if (fp->search_opt)
            mpseSetOpt(pg->pgPms[i], 1);
    }

    return 0;
}

static PmType GetPmType (HTTP_BUFFER hb_type)
{
    switch ( hb_type )
    {
    case HTTP_BUFFER_URI:
        return PM_TYPE__HTTP_URI_CONTENT;

    case HTTP_BUFFER_HEADER:
        return PM_TYPE__HTTP_HEADER_CONTENT;

    case HTTP_BUFFER_CLIENT_BODY:
        return PM_TYPE__HTTP_CLIENT_BODY_CONTENT;

    default:
        break;
    }
    return PM_TYPE__CONTENT;
}

static int fpAddPortGroupRule(SnortConfig *sc, PORT_GROUP *pg, OptTreeNode *otn, FastPatternConfig *fp)
{
    PatternMatchData *pmd = NULL;
    PatternMatchData *pmd_uri = NULL;
    PatternMatchData *preproc_opt_pmds = NULL;

    if ((pg == NULL) || (otn == NULL))
        return -1;

    /* Preprocessor or decoder rule, skip inserting it */
    if (otn->sigInfo.rule_type != SI_RULE_TYPE_DETECT)
        return -1;

    /* Rule not enabled */
    if (otn->rule_state != RULE_STATE_ENABLED)
        return -1;

    /* Check for an so dynamic rule */
    if (otn->ds_list[PLUGIN_DYNAMIC] != NULL)
    {
        DynamicData *dd = otn->ds_list[PLUGIN_DYNAMIC];

        /* The pmds returned here will have been dynamically allocated */
        pmd = GetDynamicFastPatternPmd(dd, CONTENT_NORMAL);
        if ((pmd != NULL) && pmd->fp)
        {
            if (fpFinishPortGroupRule(sc, pg, PM_TYPE__CONTENT, otn, pmd, fp) == 0)
            {
                /* Need to do this so the pmd can be freed later */
                (void)AppendPmdToList(&dd->pmds, pmd);
                if (pmd->pattern_size > otn->longestPatternLen)
                    otn->longestPatternLen = pmd->pattern_size;
                return 0;
            }

            PatternMatchFree((void *)pmd);
            pmd = NULL;
        }

        pmd_uri = GetDynamicFastPatternPmd(dd, CONTENT_HTTP);
        if (pmd_uri != NULL)
        {
            PmType pm_type = GetPmType(pmd_uri->http_buffer);

            if (fpFinishPortGroupRule(sc, pg, pm_type, otn, pmd_uri, fp) == 0)
            {
                /* Using the http content so free this */
                if (pmd != NULL)
                    PatternMatchFree((void *)pmd);

                (void)AppendPmdToList(&dd->pmds, pmd_uri);
                if (pmd_uri->pattern_size > otn->longestPatternLen)
                    otn->longestPatternLen = pmd_uri->pattern_size;
                return 0;
            }

            PatternMatchFree((void *)pmd_uri);
            pmd_uri = NULL;
        }

        /* If we get this far then no URI contents were added */

        if (GetPreprocOptPmdList(otn, &preproc_opt_pmds) == 0)
        {
            if (UsePreprocOptFastPatterns(pmd, preproc_opt_pmds))
            {
                /* Preprocessor rule option fast pattern contents
                 * will be used so free and NULL the content pmd */
                if (pmd != NULL)
                    FreePmdList(pmd);

                pmd = preproc_opt_pmds;
            }
            else
            {
                /* The content rule option fast pattern is a better choice
                 * than the preprocessor rule option fast patterns, so use it */
                FreePmdList(preproc_opt_pmds);
            }
        }

        if (fpFinishPortGroupRule(sc, pg, PM_TYPE__CONTENT, otn, pmd, fp) == 0)
        {
            (void)AppendPmdToList(&dd->pmds, pmd);
            if (pmd->pattern_size > otn->longestPatternLen)
                otn->longestPatternLen = pmd->pattern_size;
            return 0;
        }

        /* Either no content or adding pmd failed */

        /* Either single pmd or preprocessor rule option pmd list */
        if (pmd != NULL)
            FreePmdList(pmd);

        if (fpFinishPortGroupRule(sc, pg, PM_TYPE__MAX, otn, NULL, fp) != 0)
            return -1;

        return 0;
    }

    pmd = GetLongestPmdContent(otn, CONTENT_NORMAL);

    /* Pull it out of the ds_list so we can treat it as a one item list
     * It will get free'd via the detection option tree callback for
     * content rule options - the ds pmd list is useless at this point
     * and should not be used anyway because of detection option tree
     * duplicate handling - see FinalizeContentUniqueness() */
    (void)RemovePmdFromList(pmd);

    if ((pmd != NULL) && pmd->fp)
    {
        if (fpFinishPortGroupRule(sc, pg, PM_TYPE__CONTENT, otn, pmd, fp) == 0)
        {
            if (pmd->pattern_size > otn->longestPatternLen)
                otn->longestPatternLen = pmd->pattern_size;

            return 0;
        }
    }

    /* http buffer contents take precedence over normal contents if
     * no normal contents have the fast_pattern option */
    pmd_uri = GetLongestPmdContent(otn, CONTENT_HTTP);
    (void)RemovePmdFromList(pmd_uri);
    if (pmd_uri != NULL)
    {
        PmType pm_type = GetPmType(pmd_uri->http_buffer);

        if (fpFinishPortGroupRule(sc, pg, pm_type, otn, pmd_uri, fp) == 0)
        {
            if (pmd_uri->pattern_size > otn->longestPatternLen)
                otn->longestPatternLen = pmd_uri->pattern_size;
            return 0;
        }
    }

    /* If we get this far then no URI contents were added */

    if (GetPreprocOptPmdList(otn, &preproc_opt_pmds) == 0)
    {
        if (!UsePreprocOptFastPatterns(pmd, preproc_opt_pmds))
        {
            /* The content rule option fast pattern is a better choice
             * than the preprocessor rule option fast patterns, so use it */
            FreePmdList(preproc_opt_pmds);
        }
        else
        {
            pmd = preproc_opt_pmds;

            /* Need to be able to free this list */
            (void)AppendPmdToList(
                    (PatternMatchData **)&otn->preproc_fp_list,
                    preproc_opt_pmds);
        }
    }

    if (fpFinishPortGroupRule(sc, pg, PM_TYPE__CONTENT, otn, pmd, fp) == 0)
    {
        if (pmd->pattern_size > otn->longestPatternLen)
            otn->longestPatternLen = pmd->pattern_size;
        return 0;
    }

#if 0
    /* XXX Not currently used */
    /* If the "or" content rule option should ever be instated, some sort of
     * decision should be made between this and other normal "and" contents.
     * Adding one content is probably preferable over adding multiple contents,
     * but if the one content is only one, two bytes and the "or" contents are
     * more unique, then adding the "or" contents might be preferable.  Maybe
     * if the "and" content is more unique than the least unique "or" content */
    pmd = otn->ds_list[PLUGIN_PATTERN_MATCH_OR];
    if (pmd != NULL)
        fpAddAllContents(pg->pgPms[PM_TYPE__CONTENT], otn, id, pmd, fp);
#endif

    /* No content added */
    if (pmd == preproc_opt_pmds)
        FreePmdList(pmd);

    if (fpFinishPortGroupRule(sc, pg, PM_TYPE__MAX, otn, NULL, fp) != 0)
        return -1;

    return 0;
}

/*
 * Original PortRuleMaps for each protocol requires creating the following structures.
 *          -pcrm.h
 *          PORT_RULE_MAP -> srcPortGroup,dstPortGroup,genericPortGroup
 *          PORT_GROUP    -> pgPatData, pgPatDataUri (acsm objects), (also rule_node lists 1/rule, not neeed)
 *                           each rule content added to an acsm object has a PMX data ptr associated with it.
 *          RULE_NODE     -> iRuleNodeID (used for bitmap object index)
 *
 *          -fpcreate.h
 *          PMX   -> RULE_NODE(->otn), PatternMatchData
 *
 *  PortList model supports the same structures except:
 *
 *          -pcrm.h
 *          PORT_GROUP    -> no rule_node lists needed, PortObjects maintain a list of rules used
 *
 *  Generation of PortRuleMaps and data is done differently.
 *
 *    1) Build tcp/udp/icmp/ip src and dst PORT_GROUP objects based on the PortList Objects rules.
 *
 *    2) For each protocols PortList objects walk it's ports and assign the PORT_RULE_MAP src and dst
 *         PORT_GROUP[port] array pointers to that PortList objects PORT_GROUP.
 *
 *    Implementation:
 *
 *    Each PortList Object will be translated into a PORT_GROUP, than pointed to by the
 *    PORT_GROUP array in the PORT_RULE_MAP for the procotocol
 *
 *    protocol = tcp, udp, ip, icmp - one port_rule_map for each of these protocols
 *    { create a port_rule_map
 *      dst port processing
 *          for each port-list object create a port_group object
 *          {   create a pattern match object, store its pointer in port_group
 *              for each rule index in port-list object
 *              {
 *                  get the gid+sid for the index
 *                  lookup up the otn
 *                  create pmx
 *                  create RULE_NODE, set iRuleNodeID within this port-list object
 *                  get longest content for the rule
 *                  set up pmx,RULE_NODE
 *                  add the content and pmx to the pattern match object
 *              }
 *              compile the pattern match object
 *
 *              repeat for uri content
 *          }
 *      src port processing
 *          repeat as for dst port processing
 *    }
 *    ** bidirectional rules - these are added to both src and dst PortList objects, so they are
 *    automatically handled during conversion to port_group objects.
 */
/*
**  Build a Pattern group for the Uri-Content rules in this group
**
**  The patterns added for each rule must be suffcient so if we find any of them
**  we proceed to fully analyze the OTN and RTN against the packet.
**
*/
/*
 *  Init a port-list based rule map
 */
static int fpCreateInitRuleMap ( 
        PORT_RULE_MAP * prm,
        PortTable * src,
        PortTable * dst,
        PortObject * anyany,
        PortObject * nc,
        PortTable * ns_src, // TARGET_BASED
        PortTable * ns_dst )// TARGET_BASED 
{
    SFGHASH_NODE   * node;
    PortObjectItem * poi;
    PortObject2    * po;
    int              i;

    /* setup the any-any-port content port group */
    prm->prmGeneric =(PORT_GROUP*) anyany->data;

    /* all rules that are any any some may not be content ? */
    prm->prmNumGenericRules = anyany->rule_list->count;

    prm->prmNumSrcRules= 0;
    prm->prmNumDstRules= 0;

    prm->prmNumSrcGroups= 0;
    prm->prmNumDstGroups= 0;
#ifdef TARGET_BASED
    prm->prmNumNoServiceSrcRules= 0;
    prm->prmNumNoServiceDstRules= 0;
    prm->prmNumNoServiceSrcGroups= 0;
    prm->prmNumNoServiceDstGroups= 0;
#endif

    /* Process src PORT groups */
    if(src )
    {
        for( node=sfghash_findfirst(src->pt_mpxo_hash);
                node;
                node=sfghash_findnext(src->pt_mpxo_hash) )
        {
            po = (PortObject2*)node->data;

            if( !po ) continue;
            if( !po->data ) continue;

            /* Add up the total src rules */
            prm->prmNumSrcRules  += po->rule_hash->count;

            /* Increment the port group count */
            prm->prmNumSrcGroups++;

            /* Add this port group to the src table at each port that uses it */
            for( poi = (PortObjectItem*)sflist_first(po->item_list);
                    poi;
                    poi = (PortObjectItem*)sflist_next(po->item_list) )
            {
                switch(poi->type)
                {
                    case PORT_OBJECT_ANY:
                        break;
                    case PORT_OBJECT_PORT:
#if 0
                        /* This test is always true since poi->lport is a 16 bit
                         * int and MAX_PORTS is 64K.  If this relationship should
                         * change, the test should be compiled back in.
                         */
                        if(  poi->lport < MAX_PORTS )
#endif
                            prm->prmSrcPort[ poi->lport ] = (PORT_GROUP*)po->data;
                        break;
                    case PORT_OBJECT_RANGE:
                        for(i= poi->lport;i<= poi->hport;i++ )
                        {
                            prm->prmSrcPort[ i ] = (PORT_GROUP*)po->data;
                        }
                        break;
                }
            }
        }
    }

    /* process destination port groups */
    if( dst )
    {
        for( node=sfghash_findfirst(dst->pt_mpxo_hash);
                node;
                node=sfghash_findnext(dst->pt_mpxo_hash) )
        {
            po = (PortObject2*)node->data;

            if( !po ) continue;
            if( !po->data ) continue;

            /* Add up the total src rules */
            prm->prmNumDstRules  += po->rule_hash->count;

            /* Increment the port group count */
            prm->prmNumDstGroups++;

            /* Add this port group to the src table at each port that uses it */
            for( poi = (PortObjectItem*)sflist_first(po->item_list);
                    poi;
                    poi = (PortObjectItem*)sflist_next(po->item_list) )
            {
                switch(poi->type)
                {
                    case PORT_OBJECT_ANY:
                        break;
                    case PORT_OBJECT_PORT:
#if 0
                        /* This test is always true since poi->lport is a 16 bit
                         * int and MAX_PORTS is 64K.  If this relationship should
                         * change, the test should be compiled back in.
                         */
                        if(  poi->lport < MAX_PORTS )
#endif
                            prm->prmDstPort[ poi->lport ] = (PORT_GROUP*)po->data;
                        break;
                    case PORT_OBJECT_RANGE:
                        for(i= poi->lport;i<= poi->hport;i++ )
                        {
                            prm->prmDstPort[ i ] = (PORT_GROUP*)po->data;
                        }
                        break;
                }
            }
        }
    }

    if( ns_src )
    {
        for( node=sfghash_findfirst(ns_src->pt_mpxo_hash);
                node;
                node=sfghash_findnext(ns_src->pt_mpxo_hash) )
        {
            po = (PortObject2*)node->data;

            if( !po ) continue;
            if( !po->data ) continue;

            /* Add up the total ns_src rules */
#ifdef TARGET_BASED
            prm->prmNumNoServiceSrcRules += po->rule_hash->count;

            /* Increment the port group count */
            prm->prmNumNoServiceSrcGroups ++;
#endif
            /* Add this port group to the ns_src table at each port that uses it */
            for( poi = (PortObjectItem*)sflist_first(po->item_list); poi;
                    poi = (PortObjectItem*)sflist_next(po->item_list) )
            {
                switch(poi->type)
                {
                    case PORT_OBJECT_ANY:
                        break;
                    case PORT_OBJECT_PORT:
#if 0
                        /* This test is always true since poi->lport is a 16 bit
                         * int and MAX_PORTS is 64K.  If this relationship should
                         * change, the test should be compiled back in.
                         */
                        if(  poi->lport < MAX_PORTS )
#endif
#ifdef TARGET_BASED
                            prm->prmNoServiceSrcPort[ poi->lport ] = (PORT_GROUP*)po->data;
#endif
                        break;
                    case PORT_OBJECT_RANGE:
#ifdef TARGET_BASED
                        for(i= poi->lport;i<= poi->hport;i++ )
                        {
                            prm->prmNoServiceSrcPort[ i ] = (PORT_GROUP*)po->data;
                        }
#endif
                        break;
                }
            }
        }
    }

    if( ns_dst )
    {
        for( node=sfghash_findfirst(ns_dst->pt_mpxo_hash); node;
                node=sfghash_findnext(ns_dst->pt_mpxo_hash) )
        {
            po = (PortObject2*)node->data;

            if( !po ) continue;
            if( !po->data ) continue;

#ifdef TARGET_BASED
            /* Add up the total ns_dst rules */
            prm->prmNumNoServiceDstRules += po->rule_hash->count;

            /* Increment the port group count */
            prm->prmNumNoServiceDstGroups ++;
#endif

            /* Add this port group to the ns_dst table at each port that uses it */
            for( poi = (PortObjectItem*)sflist_first(po->item_list);
                    poi;
                    poi = (PortObjectItem*)sflist_next(po->item_list) )
            {
                switch(poi->type)
                {
                    case PORT_OBJECT_ANY:
                        break;
                    case PORT_OBJECT_PORT:
#if 0
                        /* This test is always true since poi->lport is a 16 bit
                         * int and MAX_PORTS is 64K.  If this relationship should
                         * change, the test should be compiled back in.
                         */
                        if(  poi->lport < MAX_PORTS )
#endif
#ifdef TARGET_BASED
                            prm->prmNoServiceDstPort[ poi->lport ] = (PORT_GROUP*)po->data;
#endif
                        break;
                    case PORT_OBJECT_RANGE:
#ifdef TARGET_BASED
                        for(i= poi->lport;i<= poi->hport;i++ )
                        {
                            prm->prmNoServiceDstPort[ i ] = (PORT_GROUP*)po->data;
                        }
#endif
                        break;
                }
            }
        }
    }

    return 0;
}
/*
 * Create and initialize the rule maps
 */
static int fpCreateRuleMaps(SnortConfig *sc, rule_port_tables_t *p)
{
    sc->prmTcpRTNX = prmNewMap();
    if (sc->prmTcpRTNX == NULL)
        return 1;

#ifdef TARGET_BASED
    if (fpCreateInitRuleMap(sc->prmTcpRTNX, p->tcp_src, p->tcp_dst, p->tcp_anyany, p->tcp_nocontent, p->ns_tcp_src, p->ns_tcp_dst ))
        return -1;
#endif

    sc->prmUdpRTNX = prmNewMap();
    if (sc->prmUdpRTNX == NULL)
        return -1;

#ifdef TARGET_BASED
    if (fpCreateInitRuleMap(sc->prmUdpRTNX, p->udp_src, p->udp_dst, p->udp_anyany, p->udp_nocontent, p->ns_udp_src, p->ns_udp_dst))
        return -1;
#endif

    sc->prmIpRTNX = prmNewMap();
    if (sc->prmIpRTNX == NULL)
        return 1;

#ifdef TARGET_BASED
    if (fpCreateInitRuleMap(sc->prmIpRTNX, p->ip_src, p->ip_dst, p->ip_anyany, p->ip_nocontent, p->ns_ip_src, p->ns_ip_dst))
        return -1;
#endif

    sc->prmIcmpRTNX = prmNewMap();
    if (sc->prmIcmpRTNX == NULL)
        return 1;

#ifdef TARGET_BASED
    if (fpCreateInitRuleMap(sc->prmIcmpRTNX, p->icmp_src, p->icmp_dst, p->icmp_anyany, p->icmp_nocontent, p->ns_icmp_src, p->ns_icmp_dst))
        return -1;
#endif

    return 0;
}

static void fpFreeRuleMaps(SnortConfig *sc)
{
    if (sc == NULL)
        return;

    if (sc->prmTcpRTNX != NULL)
    {
        free(sc->prmTcpRTNX);
        sc->prmTcpRTNX = NULL;
    }

    if (sc->prmUdpRTNX != NULL)
    {
        free(sc->prmUdpRTNX);
        sc->prmUdpRTNX = NULL;
    }

    if (sc->prmIpRTNX != NULL)
    {
        free(sc->prmIpRTNX);
        sc->prmIpRTNX = NULL;
    }

    if (sc->prmIcmpRTNX != NULL)
    {
        free(sc->prmIcmpRTNX);
        sc->prmIcmpRTNX = NULL;
    }
}

static int fpGetFinalPattern(FastPatternConfig *fp, PatternMatchData *pmd,
        char **ret_pattern, int *ret_bytes)
{
    char *pattern;
    int bytes;

    assert(fp);
    assert(pmd);
    assert(ret_pattern);
    assert(ret_bytes);

    if ((fp == NULL) || (pmd == NULL) || (ret_pattern == NULL) || (ret_bytes == NULL))
    {
        return -1;
    }

    pattern = pmd->pattern_buf;
    bytes = pmd->pattern_size;

    /* Don't mess with fast pattern only contents - they should be inserted
     * into the pattern matcher as is since the content won't be evaluated
     * as a rule option.
     * Don't mess with negated contents since truncating them could
     * inadvertantly disable evaluation of a rule - the shorter pattern
     * may be found, while the unaltered pattern may not be found,
     * disabling inspection of a rule we should inspect */
    if (pmd->fp_only || pmd->exception_flag)
    {
        *ret_pattern = pattern;
        *ret_bytes = bytes;

        return 0;
    }

    if (pmd->fp && (pmd->fp_length != 0))
    {
        /* (offset + length) potentially being larger than the pattern itself
         * is taken care of during parsing */
        pattern = pmd->pattern_buf + pmd->fp_offset;
        bytes = pmd->fp_length;
    }
    else
    {
        /* Trim leading null bytes for non-deterministic pattern matchers.
         * Assuming many packets may have strings of 0x00 bytes in them,
         * this should help performance with non-deterministic pattern matchers
         * that have a full next state vector at state 0.  If no patterns are
         * inserted into the state machine that start with 0x00, failstates that
         * land us at state 0 will allow us to roll through the 0x00 bytes,
         * since the next state is deterministic in state 0 and we won't move
         * beyond state 0 as long as the next input char is 0x00 */
        if ((fp->search_method == MPSE_AC_BNFA_Q)
                || (fp->search_method == MPSE_AC_BNFA))
        {
            bytes =
                FLP_Trim(pmd->pattern_buf, pmd->pattern_size, &pattern);

            if (bytes < (int)pmd->pattern_size)
            {
                /* The patten is all '\0' - use the whole pattern
                 * XXX This potentially hurts the performance boost
                 * gained by stripping leading zeros */
                if (bytes == 0)
                {
                    bytes = pmd->pattern_size;
                    pattern = pmd->pattern_buf;
                }
                else
                {
                    fp->num_patterns_trimmed++;
                }
            }
        }
    }

    if ((fp->max_pattern_len != 0)
            && (bytes > fp->max_pattern_len))
    {
        bytes = fp->max_pattern_len;
        fp->num_patterns_truncated++;
    }

    *ret_pattern = pattern;
    *ret_bytes = bytes;

    return 0;
}

void fpDynamicDataFree(void *data)
{
    DynamicData *dd = (DynamicData *)data;
    PatternMatchData *pmd;

    if (dd == NULL)
        return;

    pmd = (PatternMatchData *)dd->pmds;
    while (pmd != NULL)
    {
        PatternMatchData *tmp = pmd->next;
        PatternMatchFree((void *)pmd);
        pmd = tmp;
    }

    free(dd);
}

/*
 *  Add a rule to the proper port group RULE_NODE list
 *
 *  cflag : content flag  ( 0=no content, 1=content, 2=uri-content)
 */
static int fpAddPortGroupPrmx(PORT_GROUP *pg, OptTreeNode *otn, int cflag)
{
    /* Add the no content rule_node to the port group (NClist) */
    switch (cflag)
    {
        case PGCT_NOCONTENT:
            prmxAddPortRuleNC( pg, otn );
            break;
        case PGCT_CONTENT:
            prmxAddPortRule( pg, otn );
            break;
        case PGCT_URICONTENT:
            prmxAddPortRuleUri( pg, otn );
            break;
        default:
            return -1;
    }

    return 0;
}

static void fpPortGroupPrintRuleCount(PORT_GROUP *pg)
{
    PmType type;

    if (pg == NULL)
        return;

    LogMessage("PortGroup rule summary:\n");

    for (type = PM_TYPE__CONTENT; type < PM_TYPE__MAX; type++)
    {
        int count = mpseGetPatternCount(pg->pgPms[type]);

        switch (type)
        {
            case PM_TYPE__CONTENT:
                LogMessage("\tContent: %d\n", count);
                break;
            case PM_TYPE__HTTP_URI_CONTENT:
                LogMessage("\tHttp Uri Content: %d\n", count);
                break;
            case PM_TYPE__HTTP_HEADER_CONTENT:
                LogMessage("\tHttp Header Content: %d\n", count);
                break;
            case PM_TYPE__HTTP_CLIENT_BODY_CONTENT:
                LogMessage("\tHttp Client Body Content: %d\n", count);
                break;
            default:
                break;
        }
    }

    LogMessage("\tNo content: %u\n", pg->pgNoContentCount);
}

static void fpDeletePMX(void *data)
{
    PMX *pmx = (PMX *)data;

    if (data == NULL)
        return;

    if (pmx->RuleNode != NULL)
        free(pmx->RuleNode);

    free(pmx);
}

static void fpDeletePortGroup(void *data)
{
    PORT_GROUP *pg = (PORT_GROUP *)data;
    RULE_NODE *rn, *tmpRn;
    PmType i;

    rn = pg->pgHead;
    while (rn)
    {
        tmpRn = rn->rnNext;
        free(rn);
        rn = tmpRn;
    }
    pg->pgHead = NULL;

    rn = pg->pgUriHead;
    while (rn)
    {
        tmpRn = rn->rnNext;
        free(rn);
        rn = tmpRn;
    }
    pg->pgUriHead = NULL;

    rn = pg->pgHeadNC;
    while (rn)
    {
        tmpRn = rn->rnNext;
        free(rn);
        rn = tmpRn;
    }
    pg->pgHeadNC = NULL;

    for (i = PM_TYPE__CONTENT; i < PM_TYPE__MAX; i++)
    {
        if (pg->pgPms[i] != NULL)
        {
            mpseFree(pg->pgPms[i]);
            pg->pgPms[i] = NULL;
        }
    }

    free_detection_option_root(&pg->pgNonContentTree);

    free(pg);
}

/*
 *  Create the PortGroup for these PortObject2 entitiies
 *
 *  This builds the 1st pass multi-pattern state machines for
 *  content and uricontent based on the rules in the PortObjects
 *  hash table.
 */
static int fpCreatePortObject2PortGroup(SnortConfig *sc, PortObject2 *po, PortObject2 *poaa)
{
    SFGHASH_NODE *node;
    unsigned sid, gid;
    OptTreeNode * otn;
    PORT_GROUP * pg;
    PortObject2 *pox;
    FastPatternConfig *fp = sc->fast_pattern_config;

    /* verify we have a port object */
    if (po == NULL)
        return 0;

    po->data = 0;

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        PortObject2PrintPorts( po );

    /* Check if we have any rules */
    if (po->rule_hash == NULL)
        return 0;

    /* create a port_group */
    pg = (PORT_GROUP *)SnortAlloc(sizeof(PORT_GROUP));

    if (fpAllocPms(sc, pg, fp) != 0)
    {
        free(pg);
        return -1;
    }

    /*
     * Walk the rules in the PortObject and add to
     * the PORT_GROUP pattern state machine
     *  and to the port group RULE_NODE lists.
     * (The lists are still used in some cases
     *  during detection to walk the rules in a group
     *  so we have to load these as well...fpEvalHeader()... for now.)
     *
     * po   src/dst ports : content/uri and nocontent
     * poaa any-any ports : content/uri and nocontent
     *
     * each PG has src or dst contents, generic-contents, and no-contents
     * (src/dst or any-any ports)
     *
     */
    pox = po;

    while (pox != NULL)
    {
        for (node = sfghash_findfirst(pox->rule_hash);
                node;
                node = sfghash_findnext(pox->rule_hash))
        {
            int *prindex = (int *)node->data;

            /* be safe - no rule index, ignore it */
            if (prindex == NULL)
                continue;

            /* look up gid:sid */
            gid = RuleIndexMapGid(ruleIndexMap, *prindex);
            sid = RuleIndexMapSid(ruleIndexMap, *prindex);

            /* look up otn */
            otn = OtnLookup(sc->otn_map, gid, sid);
            if (otn == NULL)
            {
                LogMessage("fpCreatePortObject2PortGroup...failed otn lookup, "
                        "gid=%u sid=%u\n", gid, sid);
                continue;
            }

            if (otn->proto == ETHERNET_TYPE_IP)
            {
                /* If only one detection option and it's ip_proto it will be evaluated
                 * at decode time instead of detection time */
                if ((otn->ds_list[PLUGIN_IP_PROTO_CHECK] != NULL) &&
                        (otn->num_detection_opts == 1))
                {
                    fpAddIpProtoOnlyRule(sc->ip_proto_only_lists, otn);
                    continue;
                }

                fpRegIpProto(sc->ip_proto_array, otn);
            }

            if (fpAddPortGroupRule(sc, pg, otn, fp) != 0)
                continue;
        }

        if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
            fpPortGroupPrintRuleCount(pg);

        if (pox == poaa)
            break;

        pox = poaa;
    }

    /* This might happen if there was ip proto only rules
     * Don't return failure */
    if (fpFinishPortGroup(sc, pg, fp) != 0)
        return 0;

    po->data = pg;
    po->data_free = fpDeletePortGroup;

    return 0;
}

/*
 *  Create the port groups for this port table
 */
static int fpCreatePortTablePortGroups(SnortConfig *sc, PortTable *p, PortObject2 *poaa)
{
   SFGHASH_NODE * node;
   int cnt=1;
   FastPatternConfig *fp = sc->fast_pattern_config;

   if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
       LogMessage("%d Port Groups in Port Table\n",p->pt_mpo_hash->count);

   for (node=sfghash_findfirst(p->pt_mpo_hash);  //p->pt_mpxo_hash
        node;
        node=sfghash_findnext(p->pt_mpo_hash) ) //p->pt->mpxo_hash
   {
        PortObject2 * po;

        po = (PortObject2*)node->data;
        if (po == NULL)
            continue;

        if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
            LogMessage("Creating Port Group Object %d of %d\n",cnt++,p->pt_mpo_hash->count);

        /* if the object is not referenced, don't add it to the PORT_GROUPs
         * as it may overwrite other objects that are more inclusive. */
        if (!po->port_cnt)
            continue;

        if (fpCreatePortObject2PortGroup(sc, po, poaa))
        {
            LogMessage("fpCreatePortObject2PortGroup() failed\n");
            return -1;
        }

        if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
            mpsePrintSummary(fp->search_method);
   }

   return 0;
}

/*
 *  Create port group objects for all port tables
 *
 *  note: any-any ports are standard PortObjects not PortObject2's so we have to
 *  uprade them for the create port group function
 */
static int fpCreatePortGroups(SnortConfig *sc, rule_port_tables_t *p)
{
    PortObject2 *po2, *add_any_any = NULL;
    FastPatternConfig *fp = sc->fast_pattern_config;

    if (!rule_count)
        return 0 ;

    /* TCP */
    /* convert the tcp-any-any to a PortObject2 creature */
    po2 = PortObject2Dup(p->tcp_anyany);
    if (po2 == NULL)
        FatalError("Could not create a PortObject version 2 for tcp-any-any rules\n!");

    if (!fpDetectSplitAnyAny(fp))
        add_any_any = po2;

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("TCP-SRC\n");

    if (fpCreatePortTablePortGroups(sc, p->tcp_src, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-tcp_src\n");
        return -1;
    }

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("TCP-DST\n");

    if (fpCreatePortTablePortGroups(sc, p->tcp_dst, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-tcp_dst\n");
        return -1;
    }

#ifdef TARGET_BASED
    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("NS-TCP-SRC\n");

    if (fpCreatePortTablePortGroups(sc, p->ns_tcp_src, add_any_any))
    {
        LogMessage("fpCreatePortTablePortGroups failed-ns_tcp_src\n");
        return -1;
    }

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("NS-TCP-DST\n");

    if (fpCreatePortTablePortGroups(sc, p->ns_tcp_dst, add_any_any))
    {
        LogMessage("fpCreatePortTablePortGroups failed-ns_tcp_dst\n");
        return -1;
    }
#endif

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("TCP-ANYANY\n");

    if (fpCreatePortObject2PortGroup(sc, po2, 0))
    {
        LogMessage("fpCreatePorTablePortGroups failed-tcp any-any\n");
        return -1;
    }

    /* save the any-any port group */
    p->tcp_anyany->data = po2->data;
    p->tcp_anyany->data_free = fpDeletePortGroup;
    po2->data = 0;
    /* release the dummy PortObject2 copy of tcp-any-any */
    //LogMessage("fpcreate: calling PortObjectFree2(po2), line = %d\n",__LINE__ );
    PortObject2Free(po2);

    /* UDP */
    po2 = PortObject2Dup(p->udp_anyany);
    if (po2 == NULL )
        FatalError("Could not create a PortObject version 2 for udp-any-any rules\n!");

    if (!fpDetectSplitAnyAny(fp))
        add_any_any = po2;

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("UDP-SRC\n");

    if (fpCreatePortTablePortGroups(sc, p->udp_src, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-udp_src\n");
        return -1;
    }

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("UDP-DST\n");

    if (fpCreatePortTablePortGroups(sc, p->udp_dst, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-udp_dst\n");
        return -1;
    }

#ifdef TARGET_BASED
    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("NS-UDP-SRC\n");

    if (fpCreatePortTablePortGroups(sc, p->ns_udp_src, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-ns_udp_src\n");
        return -1;
    }

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("NS-UDP-DST\n");

    if (fpCreatePortTablePortGroups(sc, p->ns_udp_dst, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-ns_udp_dst\n");
        return -1;
    }
#endif // TARGET_BASED

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("UDP-ANYANY\n");

    if (fpCreatePortObject2PortGroup(sc, po2, 0))
    {
        LogMessage("fpCreatePortObject2PortGroup failed-udp_anyany\n");
        return -1;
    }

    p->udp_anyany->data = po2->data;
    p->udp_anyany->data_free = fpDeletePortGroup;
    po2->data = 0;
    //LogMessage("fpcreate: calling PortObjectFree2(po2), line = %d\n",__LINE__ );
    PortObject2Free(po2);

    /* ICMP */
    po2 = PortObject2Dup(p->icmp_anyany);
    if (po2 == NULL)
        FatalError("Could not create a PortObject version 2 for icmp-any-any rules\n!");

    if (!fpDetectSplitAnyAny(fp))
        add_any_any = po2;

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nICMP-SRC ");

    if (fpCreatePortTablePortGroups(sc, p->icmp_src, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-icmp_src\n");
        return -1;
    }

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nICMP-DST ");

    if (fpCreatePortTablePortGroups(sc, p->icmp_dst, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-icmp_src\n");
        return -1;
    }

#ifdef TARGET_BASED
    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nNS-ICMP-SRC"); 
    if (fpCreatePortTablePortGroups(sc, p->ns_icmp_src, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-ns_icmp_src\n");
        return -1;
    }

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nNS-ICMP-DST");

    if (fpCreatePortTablePortGroups(sc, p->ns_icmp_dst, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-ns_icmp_dst\n");
        return -1;
    }
#endif // TARGET_BASED

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nICMP-ANYANY ");

    if (fpCreatePortObject2PortGroup(sc, po2, 0))
    {
        LogMessage("fpCreatePorTablePortGroups failed-icmp any-any\n");
        return -1;
    }

    p->icmp_anyany->data = po2->data;
    p->icmp_anyany->data_free = fpDeletePortGroup;
    po2->data = 0;
    //LogMessage("fpcreate: calling PortObjectFree2(po2), line = %d\n",__LINE__ );
    PortObject2Free(po2);

    /* IP */
    po2 = PortObject2Dup(p->ip_anyany);
    if (po2 == NULL)
        FatalError("Could not create a PortObject version 2 for ip-any-any rules\n!");

    if (!fpDetectSplitAnyAny(fp))
        add_any_any = po2;

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nIP-SRC ");

    if (fpCreatePortTablePortGroups(sc, p->ip_src, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-ip_src\n");
        return -1;
    }

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nIP-DST ");

    if (fpCreatePortTablePortGroups(sc, p->ip_dst, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-ip_dst\n");
        return -1;
    }

#ifdef TARGET_BASED
    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nNS-IP-SRC ");

    if (fpCreatePortTablePortGroups(sc, p->ns_ip_src, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-ns_ip_src\n");
        return -1;
    }

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nNS-IP-DST ");

    if (fpCreatePortTablePortGroups(sc, p->ns_ip_dst, add_any_any))
    {
        LogMessage("fpCreatePorTablePortGroups failed-ns_ip_dst\n");
        return -1;
    }
#endif // TARGET_BASED

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("\nIP-ANYANY ");

    if (fpCreatePortObject2PortGroup(sc, po2, 0))
    {
        LogMessage("fpCreatePorTablePortGroups failed-ip any-any\n");
        return -1;
    }

    p->ip_anyany->data = po2->data;
    p->ip_anyany->data_free = fpDeletePortGroup;
    po2->data = 0;
    //LogMessage("fpcreate: calling PortObjectFree2(po2), line = %d\n",__LINE__ );
    PortObject2Free(po2);

    return 0;
}



/*
 *  Scan the master otn lists and and pass
 *
 *
 *  enabled - if true requires otn to be enabled
 *  fcn - callback
 *  proto - IP,TCP,IDP,ICMP protocol flag
 *  otn   - OptTreeNode
 */
void fpWalkOtns(int enabled, OtnWalkFcn fcn)
{
    RuleTreeNode *rtn;
    SFGHASH_NODE *hashNode;
    OptTreeNode *otn  = NULL;
    tSfPolicyId policyId = 0;

    if (snort_conf == NULL)
        return;

    for (hashNode = sfghash_findfirst(snort_conf->otn_map);
            hashNode;
            hashNode = sfghash_findnext(snort_conf->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;
        for ( policyId = 0;
              policyId < otn->proto_node_num;
              policyId++ )
        {
            rtn = getRtnFromOtn(otn, policyId);

            /* There can be gaps in the list of rtns. */
            if (rtn == NULL)
                continue;

            if ((rtn->proto == IPPROTO_TCP) || (rtn->proto == IPPROTO_UDP)
                || (rtn->proto == IPPROTO_ICMP) || (rtn->proto == ETHERNET_TYPE_IP))
            {
                //do operation

                if ( enabled && (otn->rule_state != RULE_STATE_ENABLED) )
                {
                    continue;
                }

                fcn( rtn->proto, rtn, otn );
            }
        }
    }
}

#ifdef TARGET_BASED
/*
 *  Scan the master otn lists and load the Service maps
 *  for service based rule grouping.
 */
static int fpCreateServiceMaps(SnortConfig *sc)
{
    RuleTreeNode *rtn;
    SFGHASH_NODE *hashNode;
    OptTreeNode *otn  = NULL;
    tSfPolicyId policyId = 0;
    unsigned int svc_idx;

    for (hashNode = sfghash_findfirst(sc->otn_map);
         hashNode;
         hashNode = sfghash_findnext(sc->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;
        for ( policyId = 0;
              policyId < otn->proto_node_num;
              policyId++ )
        {
            rtn = getRtnFromOtn(otn, policyId);

            if (rtn && ((rtn->proto == IPPROTO_TCP) || (rtn->proto == IPPROTO_UDP)
                    || (rtn->proto == IPPROTO_ICMP) || (rtn->proto == ETHERNET_TYPE_IP)))
            {
                /* Non-content preprocessor or decoder rule.
                 * don't add it */
                if (otn->sigInfo.rule_type != SI_RULE_TYPE_DETECT)
                    continue;

                /* Not enabled, don't do the FP content */
                if (otn->rule_state != RULE_STATE_ENABLED)
                    continue;

                for (svc_idx = 0; svc_idx < otn->sigInfo.num_services; svc_idx++)
                {
                    if (ServiceMapAddOtn(sc->srmmTable, rtn->proto, otn->sigInfo.services[svc_idx].service, otn))
                        return -1;
                }
            }
        }
    }

    return 0;
}


/*
* Build a Port Group for this service based on the list of otns. The final
* port_group pointer is stored using the service name as the key.
*
* p   - hash table mapping services to port_groups
* srvc- service name, key used to store the port_group
*       ...could use a service id instead (bytes, fixed length,etc...)
* list- list of otns for this service
*/
void fpBuildServicePortGroupByServiceOtnList(SnortConfig *sc, SFGHASH *p, const char *srvc, SF_LIST *list, FastPatternConfig *fp)
{
    OptTreeNode * otn;
    int status;
    PORT_GROUP *pg = (PORT_GROUP *)SnortAlloc(sizeof(PORT_GROUP));

    if (fpAllocPms(sc, pg, fp) != 0)
    {
        free(pg);
        return;
    }

    /*
     * add each rule to the port group pattern matchers,
     * or to the no-content rule list
     */
    for (otn = sflist_first(list);
            otn;
            otn = sflist_next(list))
    {
        if (otn->proto == ETHERNET_TYPE_IP)
        {
            /* If only one detection option and it's ip_proto it will be evaluated
             * at decode time instead of detection time
             * These will have already been added when adding port groups */
            if ((otn->ds_list[PLUGIN_IP_PROTO_CHECK] != NULL) &&
                    (otn->num_detection_opts == 1))
            {
                continue;
            }
        }

        if (fpAddPortGroupRule(sc, pg, otn, fp) != 0)
            continue;
    }

    if (fpFinishPortGroup(sc, pg, fp) != 0)
        return;

    /* Add the port_group using it's service name */
    status = sfghash_add(p, srvc, pg);
    switch(status)
    {
        case SFGHASH_OK :
             /* port_group is successfully added using it's service name */
             break;
        case SFGHASH_ERR :
             LogMessage("fpBuildServicePortGroupByServiceOtnList : Hash table is NULL. \n");
             break;
        case SFGHASH_INTABLE :
             break;
        case SFGHASH_NOMEM :
             LogMessage("Failed to allocate memory to port_group/service.\n");
             break;
        default :
             break;

    }
}

/*
 * For each service we create a PORT_GROUP based on the otn's defined to
 * be applicable to that service by the metadata option.
 *
 * Than we lookup the protocol/srvc oridinal in the target-based area
 * and assign the PORT_GROUP for the srvc to it.
 *
 * spg - service port group (lookup should be by service id/tag)
 *     - this table maintains a port_group ptr for each service
 * srm - service rule map table (lookup by ascii service name)
 *     - this table maintains a sf_list ptr (list of rule otns) for each service
 *
 */
void fpBuildServicePortGroups(SnortConfig *sc, SFGHASH *spg, PORT_GROUP **sopg, SFGHASH *srm, FastPatternConfig *fp)
{
    SFGHASH_NODE * n;
    const char * srvc;
    SF_LIST * list;
    PORT_GROUP * pg;

    for(n=sfghash_findfirst(srm);
        n;
        n=sfghash_findnext(srm) )
    {
        list = (SF_LIST *)n->data;
        if(!list)continue;

        srvc = n->key;
        if(!srvc)continue;

        fpBuildServicePortGroupByServiceOtnList(sc, spg, srvc, list, fp);

        /* Add this PORT_GROUP to the protocol-ordinal -> port_group table */
        pg = sfghash_find( spg, srvc );
        if( pg )
        {
            int16_t id;
            id = FindProtocolReference(srvc);
            if(id==SFTARGET_UNKNOWN_PROTOCOL)
            {
                id = AddProtocolReference(srvc);
                if(id <=0 )
                {
                    FatalError("Could not AddProtocolReference!\n");
                }
                if( id >= MAX_PROTOCOL_ORDINAL )
                {
                    LogMessage("fpBuildServicePortGroups: protocol-ordinal=%d exceeds "
                               "limit of %d for service=%s\n",id,MAX_PROTOCOL_ORDINAL,srvc);
                }
            }
            else if( id > 0 )
            {
                if( id < MAX_PROTOCOL_ORDINAL )
                {
                    sopg[ id ] = pg;
                    LogMessage("fpBuildServicePortGroups: adding protocol-ordinal=%d "
                               "as service=%s\n",id,srvc);
                }
                else
                {
                    LogMessage("fpBuildServicePortGroups: protocol-ordinal=%d exceeds "
                               "limit of %d for service=%s\n",id,MAX_PROTOCOL_ORDINAL,srvc);
                }
            }
            else /* id < 0 */
            {
                LogMessage("fpBuildServicePortGroups: adding protocol-ordinal=%d for "
                           "service=%s, can't use that !!!\n",id,srvc);

            }
        }
        else
        {
            LogMessage("*** fpBuildServicePortGroups: failed to create and find a port group for '%s' !!! \n",srvc );
        }
    }
}

/*
 * For each proto+dir+service build a PORT_GROUP
 */
static void fpCreateServiceMapPortGroups(SnortConfig *sc)
{
    FastPatternConfig *fp = sc->fast_pattern_config;

    sc->spgmmTable = ServicePortGroupMapNew();
    sc->sopgTable = ServicePortGroupTableNew();

    fpBuildServicePortGroups(sc, sc->spgmmTable->tcp_to_srv, sc->sopgTable->tcp_to_srv,
                             sc->srmmTable->tcp_to_srv, fp);
    fpBuildServicePortGroups(sc, sc->spgmmTable->tcp_to_cli, sc->sopgTable->tcp_to_cli,
                             sc->srmmTable->tcp_to_cli, fp);

    fpBuildServicePortGroups(sc, sc->spgmmTable->udp_to_srv, sc->sopgTable->udp_to_srv,
                             sc->srmmTable->udp_to_srv, fp);
    fpBuildServicePortGroups(sc, sc->spgmmTable->udp_to_cli, sc->sopgTable->udp_to_cli,
                             sc->srmmTable->udp_to_cli, fp);

    fpBuildServicePortGroups(sc, sc->spgmmTable->icmp_to_srv, sc->sopgTable->icmp_to_srv,
                             sc->srmmTable->icmp_to_srv, fp);
    fpBuildServicePortGroups(sc, sc->spgmmTable->icmp_to_cli, sc->sopgTable->icmp_to_cli,
                             sc->srmmTable->icmp_to_cli, fp);

    fpBuildServicePortGroups(sc, sc->spgmmTable->ip_to_srv, sc->sopgTable->ip_to_srv,
                             sc->srmmTable->ip_to_srv, fp);
    fpBuildServicePortGroups(sc, sc->spgmmTable->ip_to_cli, sc->sopgTable->ip_to_srv,
                             sc->srmmTable->ip_to_cli, fp);
}

PORT_GROUP * fpGetServicePortGroupByOrdinal(sopg_table_t *sopg, int proto, int dir, int16_t proto_ordinal)
{
   //SFGHASH_NODE * n;
   PORT_GROUP *pg = NULL;

   if (proto_ordinal >= MAX_PROTOCOL_ORDINAL)
       return NULL;

   if (sopg == NULL)
       return NULL;

   switch (proto)
   {
       case IPPROTO_TCP:
           if (dir == TO_SERVER)
               pg = sopg->tcp_to_srv[proto_ordinal];
           else
               pg = sopg->tcp_to_cli[proto_ordinal];

           break;

       case IPPROTO_UDP:
           if (dir == TO_SERVER)
               pg = sopg->udp_to_srv[proto_ordinal];
           else
               pg = sopg->udp_to_cli[proto_ordinal];

           break;

       case IPPROTO_ICMP:
           if (dir == TO_SERVER)
               pg = sopg->icmp_to_srv[proto_ordinal];
           else
               pg = sopg->icmp_to_cli[proto_ordinal];

           break;

       case ETHERNET_TYPE_IP:
           if (dir == TO_SERVER)
               pg = sopg->ip_to_srv[proto_ordinal];
           else
               pg = sopg->ip_to_cli[proto_ordinal];

           break;

       default:
           break;
   }

   return pg;
}


/*
 *  Print the rule gid:sid based onm the otn list
 */
void fpPrintRuleList( SF_LIST * list )
{
    OptTreeNode * otn;

    for( otn=(OptTreeNode*)sflist_first(list);
         otn;
         otn=(OptTreeNode*)sflist_next(list) )
    {
         LogMessage("|   %u:%u\n",otn->sigInfo.generator,otn->sigInfo.id);
    }
}
static
void fpPrintServiceRuleMapTable(  SFGHASH * p, char * msg )
{
     SFGHASH_NODE * n;

     if( !p || !p->count )
         return;

     LogMessage("| Protocol [%s] %d services\n",msg,p->count );
     LogMessage("----------------------------------------------------\n");

     for( n = sfghash_findfirst(p);
          n;
          n = sfghash_findnext(p) )
     {
          SF_LIST * list;

          list = (SF_LIST*)n->data;
          if( !list ) continue;

          if( !n->key ) continue;

          LogMessage("| Service [%s] %d rules, rule list follows as gid:sid.\n",
              (char*)n->key, list->count);

          fpPrintRuleList( list );
     }
     LogMessage("----------------------------------------------------\n");
}

static void fpPrintServiceRuleMaps(srmm_table_t *service_map)
{
    LogMessage("+---------------------------------------------------\n");
    LogMessage("| Service Rule Maps\n");
    LogMessage("----------------------------------------------------\n");
    fpPrintServiceRuleMapTable( service_map->tcp_to_srv,  "tcp to server" );
    fpPrintServiceRuleMapTable( service_map->tcp_to_cli,  "tcp to client" );

    fpPrintServiceRuleMapTable( service_map->udp_to_srv,  "udp to server" );
    fpPrintServiceRuleMapTable( service_map->udp_to_cli,  "udp to client" );

    fpPrintServiceRuleMapTable( service_map->icmp_to_srv, "icmp to server" );
    fpPrintServiceRuleMapTable( service_map->icmp_to_cli, "icmp to client" );

    fpPrintServiceRuleMapTable( service_map->ip_to_srv,   "ip to server" );
    fpPrintServiceRuleMapTable( service_map->ip_to_cli,   "ip to client" );
}

/*
 *
 */
void fpPrintServicePortGroupSummary(srmm_table_t *srvc_pg_map)
{

    LogMessage("+--------------------------------\n");
    LogMessage("| Service-PortGroup Table Summary \n");
    LogMessage("---------------------------------\n");

    if(srvc_pg_map->tcp_to_srv->count)
    LogMessage("| tcp to server  : %d services\n",srvc_pg_map->tcp_to_srv->count);
    if(srvc_pg_map->tcp_to_cli->count)
    LogMessage("| tcp to cient   : %d services\n",srvc_pg_map->tcp_to_cli->count);

    if(srvc_pg_map->udp_to_srv->count)
    LogMessage("| udp to server  : %d services\n",srvc_pg_map->udp_to_srv->count);
    if(srvc_pg_map->udp_to_cli->count)
    LogMessage("| udp to cient   : %d services\n",srvc_pg_map->udp_to_cli->count);

    if(srvc_pg_map->icmp_to_srv->count)
    LogMessage("| icmp to server : %d services\n",srvc_pg_map->icmp_to_srv->count);
    if(srvc_pg_map->icmp_to_cli->count)
    LogMessage("| icmp to cient  : %d services\n",srvc_pg_map->icmp_to_cli->count);

    if(srvc_pg_map->ip_to_srv->count)
    LogMessage("| ip to server   : %d services\n",srvc_pg_map->ip_to_srv->count);
    if(srvc_pg_map->ip_to_cli->count)
    LogMessage("| ip to cient    : %d services\n",srvc_pg_map->ip_to_cli->count);
    LogMessage("---------------------------------\n");
}

/*
 *  Build Service based PORT_GROUPs using the rules
 *  metadata option service parameter.
 */
static int fpCreateServicePortGroups(SnortConfig *sc)
{
    FastPatternConfig *fp = sc->fast_pattern_config;

    sc->srmmTable = ServiceMapNew();

    if (fpCreateServiceMaps(sc))
        return -1;

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        fpPrintServiceRuleMaps(sc->srmmTable);

    fpCreateServiceMapPortGroups(sc);

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        fpPrintServicePortGroupSummary(sc->spgmmTable);

    //srvcmap_term();

    return 0;
}
//TARGET_BASED
#endif

/*
*  Port list version
*
*  7/2007 - man
*
*  Build Pattern Groups for 1st pass of content searching using
*  multi-pattern search method.
*/
int fpCreateFastPacketDetection(SnortConfig *sc)
{
    rule_port_tables_t *port_tables;
    FastPatternConfig *fp;
    
#ifndef REG_TEST
    struct rusage ru;
    if (ScTestMode())
    {
        getrusage(RUSAGE_SELF, &ru);
        LogMessage("\nMaxRss at the end of rules:%li\n", ru.ru_maxrss);
    }
#endif

    /* This is somewhat necessary because of how the detection option trees
     * are added via a callback from the pattern matcher */
    if(!rule_count || (sc == NULL))
        return 0;

    port_tables = sc->port_tables;
    fp = sc->fast_pattern_config;

    if ((port_tables == NULL) || (fp == NULL))
        return 0;

#ifdef INTEL_SOFT_CPM
    if (fp->search_method == MPSE_INTEL_CPM)
        IntelPmStartInstance();
#endif

    /* Use PortObjects to create PORT_GROUPs */
    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("Creating Port Groups....\n");

    if (fpCreatePortGroups(sc, port_tables))
        FatalError("Could not create PortGroup objects for PortObjects\n");

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("Port Groups Done....\n");

    /* Create rule_maps */
    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("Creating Rule Maps....\n");

    if (fpCreateRuleMaps(sc, port_tables))
        FatalError("Could not create rule maps\n");

    if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
        LogMessage("Rule Maps Done....\n");

#ifndef TARGET_BASED
    LogMessage("\n");
    LogMessage("[ Port Based Pattern Matching Memory ]\n" );
    mpsePrintSummary(fp->search_method);
    if (fp->max_pattern_len != 0)
    {
        LogMessage("[ Number of patterns truncated to %d bytes: %d ]\n",
                fp->max_pattern_len, fp->num_patterns_truncated);
    }
    if (fp->num_patterns_trimmed != 0)
    {
        LogMessage("[ Number of null byte prefixed patterns trimmed: %d ]\n",
                fp->num_patterns_trimmed);
    }
#else
    if (IsAdaptiveConfiguredForSnortConfig(sc) || fpDetectGetDebugPrintFastPatterns(fp))
    {
        if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
            LogMessage("Creating Service Based Rule Maps....\n");

        /* Build Service based port groups - rules require service metdata
         * i.e. 'metatdata: service [=] service-name, ... ;'
         *
         * Also requires a service attribute for lookup ...
         */
        if (fpCreateServicePortGroups(sc))
            FatalError("Could not create service based port groups\n");

        if (fpDetectGetDebugPrintRuleGroupBuildDetails(fp))
            LogMessage("Service Based Rule Maps Done....\n");

        LogMessage("\n");
        LogMessage("[ Port and Service Based Pattern Matching Memory ]\n" );
    }
    else
    {
        LogMessage("\n");
        LogMessage("[ Port Based Pattern Matching Memory ]\n" );
    }

    mpsePrintSummary(fp->search_method);
    if (fp->max_pattern_len != 0)
    {
        LogMessage("[ Number of patterns truncated to %d bytes: %d ]\n",
                fp->max_pattern_len, fp->num_patterns_truncated);
    }
    if (fp->num_patterns_trimmed != 0)
    {
        LogMessage("[ Number of null byte prefixed patterns trimmed: %d ]\n",
                fp->num_patterns_trimmed);
    }
#endif

#ifdef INTEL_SOFT_CPM
    if (fp->search_method == MPSE_INTEL_CPM)
        IntelPmCompile(sc);
#endif
#ifndef REG_TEST
    if (ScTestMode())
    {
        getrusage(RUSAGE_SELF, &ru);
        LogMessage("\nMaxRss at the end of detection rules:%li\n", ru.ru_maxrss);
    }
#endif
    return 0;
}

void fpDeleteFastPacketDetection(SnortConfig *sc)
{
    if (sc == NULL)
        return;

    /* Cleanup the detection option tree */
    DetectionHashTableFree(sc->detection_option_hash_table);
    DetectionTreeHashTableFree(sc->detection_option_tree_hash_table);

    fpFreeRuleMaps(sc);

#ifdef TARGET_BASED
    ServiceMapFree(sc->srmmTable);
    ServicePortGroupMapFree(sc->spgmmTable);
    if (sc->sopgTable != NULL)
        free(sc->sopgTable);
#endif

}

/*
**  Wrapper for prmShowEventStats
*/
void fpShowEventStats(SnortConfig *sc)
{
    if ((sc == NULL) || (sc->fast_pattern_config == NULL))
        return;

    /* If not debug, then we don't print anything. */
    if (!sc->fast_pattern_config->debug)
        return;

    LogMessage("\n");
    LogMessage("** TCP Event Stats --\n");
    prmShowEventStats(sc->prmTcpRTNX);

    LogMessage("\n");
    LogMessage("** UDP Event Stats --\n");
    prmShowEventStats(sc->prmUdpRTNX);

    LogMessage("\n");
    LogMessage("** ICMP Event Stats --\n");
    prmShowEventStats(sc->prmIcmpRTNX);

    LogMessage("\n");
    LogMessage("** IP Event Stats --\n");
    prmShowEventStats(sc->prmIpRTNX);
}

static void fpAddIpProtoOnlyRule(SF_LIST **ip_proto_only_lists, OptTreeNode *otn)
{
    uint8_t ip_protos[NUM_IP_PROTOS];
    unsigned int i;

    if ((otn->ds_list[PLUGIN_IP_PROTO_CHECK] == NULL) ||
        (otn->num_detection_opts != 1))
    {
        return;
    }

    if (GetIpProtos(otn->ds_list[PLUGIN_IP_PROTO_CHECK], ip_protos, sizeof(ip_protos)) != 0)
        FatalError("%s(%d) Error getting ip protocols\n", __FILE__, __LINE__);

    for (i = 0; i < NUM_IP_PROTOS; i++)
    {
        OptTreeNode *dup;

        if (ip_protos[i])
        {
            if (ip_proto_only_lists[i] == NULL)
            {
                ip_proto_only_lists[i] = sflist_new();
                if (ip_proto_only_lists[i] == NULL)
                {
                    FatalError("%s(%d) Could not allocate memory for "
                               "ip_proto array\n", __FILE__, __LINE__);
                }
            }

            /* Search for dups */
            for (dup = (OptTreeNode *)sflist_first(ip_proto_only_lists[i]);
                 dup != NULL;
                 dup = (OptTreeNode *)sflist_next(ip_proto_only_lists[i]))
            {
                if (dup == otn)
                    return;
            }

            if (sflist_add_head(ip_proto_only_lists[i], otn) != 0)
            {
                FatalError("%s(%d) Failed to add otn to ip_proto array\n",
                           __FILE__, __LINE__);
            }
        }
    }
}

static void fpRegIpProto(uint8_t *ip_proto_array, OptTreeNode *otn)
{
    uint8_t ip_protos[NUM_IP_PROTOS];
    unsigned int i;

    if (GetIpProtos(otn->ds_list[PLUGIN_IP_PROTO_CHECK], ip_protos, sizeof(ip_protos)) != 0)
        FatalError("%s(%d) Error getting ip protocols\n", __FILE__, __LINE__);

    for (i = 0; i < NUM_IP_PROTOS; i++)
        if (ip_protos[i]) ip_proto_array[i] = 1;
}

const char * PatternRawToContent(const char *pattern, int pattern_len)
{
    static char content_buf[1024];
    int max_write_size = sizeof(content_buf) - 64;
    int i, j = 0;
    int hex = 0;

    if ((pattern == NULL) || (pattern_len <= 0))
        return "";

    content_buf[j++] = '"';

    for (i = 0; i < pattern_len; i++)
    {
        uint8_t c = (uint8_t)pattern[i];

        if ((c < 128) && isprint(c) && !isspace(c)
                && (c != '|') && (c != '"') && (c != ';'))
        {
            if (hex)
            {
                content_buf[j-1] = '|';
                hex = 0;
            }

            content_buf[j++] = c;
        }
        else
        {
            uint8_t up4, lo4;

            if (!hex)
            {
                content_buf[j++] = '|';
                hex = 1;
            }

            up4 = c >> 4;
            lo4 = c & 0x0f;

            if (up4 > 0x09) up4 += ('A' - 0x0a);
            else up4 += '0';

            if (lo4 > 0x09) lo4 += ('A' - 0x0a);
            else lo4 += '0';

            content_buf[j++] = up4;
            content_buf[j++] = lo4;
            content_buf[j++] = ' ';
        }

        if (j > max_write_size)
            break;
    }

    if (j > max_write_size)
    {
        content_buf[j] = 0;
        SnortSnprintfAppend(content_buf, sizeof(content_buf),
                " ... \" (pattern too large)");
    }
    else
    {
        if (hex)
            content_buf[j-1] = '|';

        content_buf[j++] = '"';
        content_buf[j] = 0;
    }

    return content_buf;
}

static void PrintFastPatternInfo(OptTreeNode *otn, PatternMatchData *pmd,
        const char *pattern, int pattern_length, PmType pm_type)
{
    if ((otn == NULL) || (pmd == NULL))
        return;

    LogMessage("%u:%u\n", otn->sigInfo.generator, otn->sigInfo.id);
    LogMessage("  Fast pattern matcher: %s\n", pm_type_strings[pm_type]);
    LogMessage("  Fast pattern set: %s\n", pmd->fp ? "yes" : "no");
    LogMessage("  Fast pattern only: %s\n", pmd->fp_only ? "yes" : "no");
    LogMessage("  Negated: %s\n", pmd->exception_flag ? "yes" : "no");

    /* Fast pattern only patterns don't use offset and length */
    if ((pmd->fp_length != 0) && !pmd->fp_only)
    {
        LogMessage("  Pattern <offset,length>: %d,%d\n",
                pmd->fp_offset, pmd->fp_length);
        LogMessage("    %s\n",
                PatternRawToContent(pmd->pattern_buf + pmd->fp_offset,
                    pmd->fp_length));
    }
    else
    {
        LogMessage("  Pattern offset,length: none\n");
    }

    /* Fast pattern only patterns don't get truncated */
    if (!pmd->fp_only
            && (((pmd->fp_length != 0) && (pmd->fp_length > pattern_length))
                || ((pmd->fp_length == 0) && ((int)pmd->pattern_size > pattern_length))))
    {
        LogMessage("  Pattern truncated: %d to %d bytes\n",
                pmd->fp_length ? pmd->fp_length : pmd->pattern_size,
                pattern_length);
    }
    else
    {
        LogMessage("  Pattern truncated: no\n");
    }

    LogMessage("  Original pattern\n");
    LogMessage("    %s\n",
            PatternRawToContent(pmd->pattern_buf,pmd->pattern_size));

    LogMessage("  Final pattern\n");
    LogMessage("    %s\n", PatternRawToContent(pattern, pattern_length));
}
