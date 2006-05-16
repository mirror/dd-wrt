#include <net-snmp/net-snmp-config.h>

#include <string.h>

#include <stdlib.h>
#include <sys/types.h>

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/net-snmp-includes.h>

/*
 * xxx-rks: when you have some spare time:
 *
 * a) add get-next access
 * 
 * b) basically, everything currently creates one node per sub-oid,
 *    which is less than optimal. add code to create nodes with the
 *    longest possible OID per node, and split nodes when necessary
 *    during adds.
 *
 * c) If you are feeling really ambitious, also merge split nodes if
 *    possible on a delete.
 *
 */

/***************************************************************************
 *
 *
 ***************************************************************************/

/*
 * Create an netsnmp_oid_stash node
 *
 * @param mysize  the size of the child pointer array
 *
 * @return NULL on error, otherwise the newly allocated node
 */
netsnmp_oid_stash_node *
netsnmp_oid_stash_create_sized_node(size_t mysize)
{
    netsnmp_oid_stash_node *ret;
    ret = SNMP_MALLOC_TYPEDEF(netsnmp_oid_stash_node);
    if (!ret)
        return NULL;
    ret->children = calloc(mysize, sizeof(netsnmp_oid_stash_node *));
    if (!ret->children) {
        free(ret);
        return NULL;
    }
    ret->children_size = mysize;
    return ret;
}

NETSNMP_INLINE netsnmp_oid_stash_node *
netsnmp_oid_stash_create_node(void)
{
    return netsnmp_oid_stash_create_sized_node(OID_STASH_CHILDREN_SIZE);
}

/** adds data to the stash at a given oid.
 * returns SNMPERR_SUCCESS on success.
 * returns SNMPERR_GENERR if data is already there.
 * returns SNMPERR_MALLOC on malloc failures or if arguments passed in
 *   with NULL values.
 */
int
netsnmp_oid_stash_add_data(netsnmp_oid_stash_node **root,
                           oid * lookup, size_t lookup_len, void *mydata)
{
    netsnmp_oid_stash_node *curnode, *tmpp, *loopp;
    unsigned int    i;

    if (!root || !lookup || lookup_len == 0)
        return SNMPERR_GENERR;

    if (!*root) {
        *root = netsnmp_oid_stash_create_node();
        if (!*root)
            return SNMPERR_MALLOC;
    }
    tmpp = NULL;
    for (curnode = *root, i = 0; i < lookup_len; i++) {
        tmpp = curnode->children[lookup[i] % curnode->children_size];
        if (!tmpp) {
            /*
             * no child in array at all 
             */
            tmpp = curnode->children[lookup[i] % curnode->children_size] =
                netsnmp_oid_stash_create_node();
            tmpp->value = lookup[i];
        } else {
            for (loopp = tmpp; loopp; loopp = loopp->next_sibling) {
                if (loopp->value == lookup[i])
                    break;
            }
            if (loopp) {
                tmpp = loopp;
            } else {
                /*
                 * none exists.  Create it 
                 */
                loopp = netsnmp_oid_stash_create_node();
                loopp->value = lookup[i];
                loopp->next_sibling = tmpp;
                tmpp->prev_sibling = loopp;
                curnode->children[lookup[i] % curnode->children_size] =
                    loopp;
                tmpp = loopp;
            }
            /*
             * tmpp now points to the proper node 
             */
        }
        curnode = tmpp;
    }
    /*
     * tmpp now points to the exact match 
     */
    if (curnode->thedata)
        return SNMPERR_GENERR;
    if (NULL == tmpp)
        return SNMPERR_GENERR;
    tmpp->thedata = mydata;
    return SNMPERR_SUCCESS;
}

/** returns a node associated with a given OID.
 */
netsnmp_oid_stash_node *
netsnmp_oid_stash_get_node(netsnmp_oid_stash_node *root,
                           oid * lookup, size_t lookup_len)
{
    netsnmp_oid_stash_node *curnode, *tmpp, *loopp;
    unsigned int    i;

    if (!root)
        return NULL;
    tmpp = NULL;
    for (curnode = root, i = 0; i < lookup_len; i++) {
        tmpp = curnode->children[lookup[i] % curnode->children_size];
        if (!tmpp) {
            return NULL;
        } else {
            for (loopp = tmpp; loopp; loopp = loopp->next_sibling) {
                if (loopp->value == lookup[i])
                    break;
            }
            if (loopp) {
                tmpp = loopp;
            } else {
                return NULL;
            }
        }
        curnode = tmpp;
    }
    return tmpp;
}

/** returns a data pointer associated with a given OID.
 */
void           *
netsnmp_oid_stash_get_data(netsnmp_oid_stash_node *root,
                           oid * lookup, size_t lookup_len)
{
    netsnmp_oid_stash_node *ret;
    ret = netsnmp_oid_stash_get_node(root, lookup, lookup_len);
    if (ret)
        return ret->thedata;
    return NULL;
}

int
netsnmp_oid_stash_store_all(int majorID, int minorID,
                            void *serverarg, void *clientarg) {
    oid oidbase[MAX_OID_LEN];
    netsnmp_oid_stash_save_info *sinfo;
    
    if (!clientarg)
        return SNMP_ERR_NOERROR;
    
    sinfo = clientarg;
    netsnmp_oid_stash_store(*(sinfo->root), sinfo->token, sinfo->dumpfn,
                            oidbase,0);
    return SNMP_ERR_NOERROR;
}

void
netsnmp_oid_stash_store(netsnmp_oid_stash_node *root,
                        const char *tokenname, NetSNMPStashDump *dumpfn,
                        oid *curoid, size_t curoid_len) {

    char buf[SNMP_MAXBUF];
    netsnmp_oid_stash_node *tmpp;
    char *cp;
    char *appname = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
                                          NETSNMP_DS_LIB_APPTYPE);
    int i;
    
    if (!tokenname || !root || !curoid || !dumpfn)
        return;

    for (i = 0; i < root->children_size; i++) {
        if (root->children[i]) {
            for (tmpp = root->children[i]; tmpp; tmpp = tmpp->next_sibling) {
                curoid[curoid_len] = tmpp->value;
                if (tmpp->thedata) {
                    snprintf(buf, sizeof(buf), "%s ", tokenname);
                    cp = read_config_save_objid(buf+strlen(buf), curoid,
                                                curoid_len+1);
                    *cp++ = ' ';
                    *cp = '\0';
                    if ((*dumpfn)(cp, sizeof(buf) - strlen(buf),
                                  tmpp->thedata, tmpp))
                        read_config_store(appname, buf);
                }
                netsnmp_oid_stash_store(tmpp, tokenname, dumpfn,
                                        curoid, curoid_len+1);
            }
        }
    }
}

void
oid_stash_dump(netsnmp_oid_stash_node *root, char *prefix)
{
    char            myprefix[MAX_OID_LEN * 4];
    netsnmp_oid_stash_node *tmpp;
    int             prefix_len = strlen(prefix) + 1;    /* actually it's +2 */
    unsigned int    i;

    memset(myprefix, ' ', MAX_OID_LEN * 4);
    myprefix[prefix_len] = '\0';

    for (i = 0; i < root->children_size; i++) {
        if (root->children[i]) {
            for (tmpp = root->children[i]; tmpp; tmpp = tmpp->next_sibling) {
                printf("%s%ld@%d:\n", prefix, tmpp->value, i);
                oid_stash_dump(tmpp, myprefix);
            }
        }
    }
}

void netsnmp_oid_stash_free(netsnmp_oid_stash_node **root,
                            NetSNMPStashFreeNode *freefn) {
     netsnmp_oid_stash_node *curnode, *tmpp;
     unsigned int    i;

     if (!root || !*root)
         return;
 
     /* loop through all our children and free each node */
     for (i = 0; i < (*root)->children_size; i++) {
         if ((*root)->children[i]) {
             for(tmpp = (*root)->children[i]; tmpp; tmpp = curnode) {
                 if (tmpp->thedata) {
                     if (freefn)
                         (*freefn)(tmpp->thedata);
                     else
                         free(tmpp->thedata);
		 }
		 curnode = tmpp->next_sibling;
		 netsnmp_oid_stash_free(&tmpp, freefn);
	     }
	 }
     }
     free((*root)->children);
     free (*root);
     *root = NULL;
}


void netsnmp_oid_stash_no_free(void *bogus)
{
     /* noop */
}
