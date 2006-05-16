/*
 * agent_registry.c
 *
 * Maintain a registry of MIB subtrees, together
 *   with related information regarding mibmodule, sessions, etc
 */

#define IN_SNMP_VARS_C

#include <net-snmp/net-snmp-config.h>
#include <signal.h>
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/agent_callbacks.h>

#include "snmpd.h"
#include "mibgroup/struct.h"
#include <net-snmp/agent/old_api.h>
#include <net-snmp/agent/null.h>
#include <net-snmp/agent/table.h>
#include <net-snmp/agent/table_iterator.h>
#include <net-snmp/agent/agent_registry.h>
#include "mib_module_includes.h"

#ifdef USING_AGENTX_SUBAGENT_MODULE
#include "agentx/subagent.h"
#include "agentx/client.h"
#endif

static void register_mib_detach_node(netsnmp_subtree *s);
static NETSNMP_INLINE void invalidate_lookup_cache(const char *context);
void netsnmp_set_lookup_cache_size(int newsize);
int netsnmp_get_lookup_cache_size(void);

subtree_context_cache *context_subtrees = NULL;

void
netsnmp_subtree_free(netsnmp_subtree *a)
{
  if (a != NULL) {
    if (a->variables != NULL && netsnmp_oid_equals(a->name_a, a->namelen, 
					     a->start_a, a->start_len) == 0) {
      free(a->variables);
    }
    SNMP_FREE(a->name_a);
    SNMP_FREE(a->start_a);
    SNMP_FREE(a->end_a);
    SNMP_FREE(a->label_a);
    netsnmp_handler_registration_free(a->reginfo);
    free(a);
  }
}

netsnmp_subtree *
netsnmp_subtree_deepcopy(netsnmp_subtree *a)
{
  netsnmp_subtree *b = (netsnmp_subtree *)calloc(1, sizeof(netsnmp_subtree));

  if (b != NULL) {
    memcpy(b, a, sizeof(netsnmp_subtree));
    b->name_a  = snmp_duplicate_objid(a->name_a,  a->namelen);
    b->start_a = snmp_duplicate_objid(a->start_a, a->start_len);
    b->end_a   = snmp_duplicate_objid(a->end_a,   a->end_len);
    b->label_a = strdup(a->label_a);
    
    if (b->name_a == NULL || b->start_a == NULL || 
	b->end_a  == NULL || b->label_a == NULL) {
      netsnmp_subtree_free(b);
      return NULL;
    }

    if (a->variables != NULL) {
      b->variables = (struct variable *)malloc(a->variables_len * 
					       a->variables_width);
      if (b->variables != NULL) {
	memcpy(b->variables, a->variables,a->variables_len*a->variables_width);
      } else {
	netsnmp_subtree_free(b);
	return NULL;
      }
    }

    if (a->reginfo != NULL) {
      b->reginfo = netsnmp_handler_registration_dup(a->reginfo);
      if (b->reginfo == NULL) {
	netsnmp_subtree_free(b);
	return NULL;
      }
    }
  }
  return b;
}

subtree_context_cache *
get_top_context_cache(void)
{
    return context_subtrees;
}

netsnmp_subtree *
netsnmp_subtree_find_first(const char *context_name)
{
    subtree_context_cache *ptr;

    if (!context_name) {
        context_name = "";
    }

    DEBUGMSGTL(("subtree", "looking for subtree for context: \"%s\"\n", 
		context_name));
    for (ptr = context_subtrees; ptr != NULL; ptr = ptr->next) {
        if (ptr->context_name != NULL && 
	    strcmp(ptr->context_name, context_name) == 0) {
            DEBUGMSGTL(("subtree", "found one for: \"%s\"\n", context_name));
            return ptr->first_subtree;
        }
    }
    DEBUGMSGTL(("subtree", "didn't find a subtree for context: \"%s\"\n", 
		context_name));
    return NULL;
}

netsnmp_subtree *
add_subtree(netsnmp_subtree *new_tree, const char *context_name)
{
    subtree_context_cache *ptr = SNMP_MALLOC_TYPEDEF(subtree_context_cache);
    if (!context_name) {
        context_name = "";
    }

    if (!ptr) {
        return NULL;
    }

    DEBUGMSGTL(("subtree", "adding subtree for context: \"%s\"\n",	
		context_name));
    ptr->next = context_subtrees;
    ptr->first_subtree = new_tree;
    ptr->context_name = strdup(context_name);
    context_subtrees = ptr;
    return ptr->first_subtree;
}

netsnmp_subtree *
netsnmp_subtree_replace_first(netsnmp_subtree *new_tree, 
			      const char *context_name)
{
    subtree_context_cache *ptr;
    if (!context_name) {
        context_name = "";
    }
    for (ptr = context_subtrees; ptr != NULL; ptr = ptr->next) {
        if (ptr->context_name != NULL &&
	    strcmp(ptr->context_name, context_name) == 0) {
            ptr->first_subtree = new_tree;
            return ptr->first_subtree;
        }
    }
    return add_subtree(new_tree, context_name);
}



int
netsnmp_subtree_compare(const netsnmp_subtree *ap, const netsnmp_subtree *bp)
{
    return snmp_oid_compare(ap->name_a, ap->namelen, bp->name_a, bp->namelen);
}

void
netsnmp_subtree_join(netsnmp_subtree *root)
{
    netsnmp_subtree *s, *tmp, *c, *d;

    while (root != NULL) {
        s = root->next;
        while (s != NULL && root->reginfo == s->reginfo) {
            tmp = s->next;
            DEBUGMSGTL(("subtree", "root start "));
            DEBUGMSGOID(("subtree", root->start_a, root->start_len));
            DEBUGMSG(("subtree", " (original end "));
            DEBUGMSGOID(("subtree", root->end_a, root->end_len));
            DEBUGMSG(("subtree", ")\n"));
            DEBUGMSGTL(("subtree", "  JOINING to "));
            DEBUGMSGOID(("subtree", s->start_a, s->start_len));

	    SNMP_FREE(root->end_a);
	    root->end_a   = s->end_a;
            root->end_len = s->end_len;
	    s->end_a      = NULL;

            for (c = root; c != NULL; c = c->children) {
                c->next = s->next;
            }
            for (c = s; c != NULL; c = c->children) {
                c->prev = root;
            }
            DEBUGMSG(("subtree", " so new end "));
            DEBUGMSGOID(("subtree", root->end_a, root->end_len));
            DEBUGMSG(("subtree", "\n"));
            /*
             * Probably need to free children too?  
             */
            for (c = s->children; c != NULL; c = d) {
                d = c->children;
                netsnmp_subtree_free(c);
            }
            netsnmp_subtree_free(s);
            s = tmp;
        }
        root = root->next;
    }
}


        /*
         *  Split the subtree into two at the specified point,
         *    returning the new (second) subtree
         */
netsnmp_subtree *
netsnmp_subtree_split(netsnmp_subtree *current, oid name[], int name_len)
{
    struct variable *vp = NULL;
    netsnmp_subtree *new_sub, *ptr;
    int i = 0, rc = 0, rc2 = 0;
    size_t common_len = 0;
    char *cp;
    oid *tmp_a, *tmp_b;

    if (snmp_oid_compare(name, name_len, current->end_a, current->end_len)>0) {
	/* Split comes after the end of this subtree */
        return NULL;
    }

    new_sub = netsnmp_subtree_deepcopy(current);
    if (new_sub == NULL) {
        return NULL;
    }

    /*  Set up the point of division.  */
    tmp_a = snmp_duplicate_objid(name, name_len);
    if (tmp_a == NULL) {
	netsnmp_subtree_free(new_sub);
	return NULL;
    }
    tmp_b = snmp_duplicate_objid(name, name_len);
    if (tmp_b == NULL) {
	netsnmp_subtree_free(new_sub);
	free(tmp_a);
	return NULL;
    }

    if (current->end_a != NULL) {
	free(current->end_a);
    }
    current->end_a = tmp_a;
    current->end_len = name_len;
    if (new_sub->start_a != NULL) {
	free(new_sub->start_a);
    }
    new_sub->start_a = tmp_b;
    new_sub->start_len = name_len;

    /*  Split the variables between the two new subtrees.  */
    i = current->variables_len;
    current->variables_len = 0;

    for (vp = current->variables; i > 0; i--) {
	/*  Note that the variable "name" field omits the prefix common to the
	    whole registration, hence the strange comparison here.  */

	rc = snmp_oid_compare(vp->name, vp->namelen,
			      name     + current->namelen, 
			      name_len - current->namelen);

        if (name_len - current->namelen > vp->namelen) {
            common_len = vp->namelen;
        } else {
            common_len = name_len - current->namelen;
        }

        rc2 = snmp_oid_compare(vp->name, common_len,
                               name + current->namelen, common_len);

        if (rc >= 0) {
            break;  /* All following variables belong to the second subtree */
	}

        current->variables_len++;
        if (rc2 < 0) {
            new_sub->variables_len--;
            cp = (char *) new_sub->variables;
            new_sub->variables = (struct variable *)(cp + 
						     new_sub->variables_width);
        }
        vp = (struct variable *) ((char *) vp + current->variables_width);
    }

    /* Delegated trees should retain their variables regardless */
    if (current->variables_len > 0 &&
        IS_DELEGATED((u_char) current->variables[0].type)) {
        new_sub->variables_len = 1;
        new_sub->variables = current->variables;
    }

    /* Propogate this split down through any children */
    if (current->children) {
        new_sub->children = netsnmp_subtree_split(current->children, 
						  name, name_len);
    }

    /* Retain the correct linking of the list */
    for (ptr = current; ptr != NULL; ptr = ptr->children) {
      ptr->next = new_sub;
    }
    for (ptr = new_sub; ptr != NULL; ptr = ptr->children) {
      ptr->prev = current;
    }
    for (ptr = new_sub->next; ptr != NULL; ptr=ptr->children) {
      ptr->prev = new_sub;
    }

    return new_sub;
}

int
netsnmp_subtree_load(netsnmp_subtree *new_sub, const char *context_name)
{
    netsnmp_subtree *tree1, *tree2, *new2;
    netsnmp_subtree *prev, *next;
    int             res, rc = 0;

    if (new_sub == NULL) {
        return MIB_REGISTERED_OK;       /* Degenerate case */
    }

    /*  Find the subtree that contains the start of the new subtree (if
	any)...*/

    tree1 = netsnmp_subtree_find(new_sub->start_a, new_sub->start_len, 
				 NULL, context_name);

    /*  ... and the subtree that follows the new one (NULL implies this is the
	final region covered).  */

    if (tree1 == NULL) {
	tree2 = netsnmp_subtree_find_next(new_sub->start_a, new_sub->start_len,
					  NULL, context_name);
    } else {
	tree2 = tree1->next;
    }

    /*  Handle new subtrees that start in virgin territory.  */

    if (tree1 == NULL) {
	new2 = NULL;
	/*  Is there any overlap with later subtrees?  */
	if (tree2 && snmp_oid_compare(new_sub->end_a, new_sub->end_len,
				      tree2->start_a, tree2->start_len) > 0) {
	    new2 = netsnmp_subtree_split(new_sub, 
					 tree2->start_a, tree2->start_len);
	}

	/*  Link the new subtree (less any overlapping region) with the list of
	    existing registrations.  */

	if (tree2) {
	    new_sub->prev = tree2->prev;
	    tree2->prev   = new_sub;
	} else {
	    new_sub->prev = netsnmp_subtree_find_prev(new_sub->start_a,
				      new_sub->start_len, NULL, context_name);

	    if (new_sub->prev) {
		new_sub->prev->next = new_sub;
	    } else {
		netsnmp_subtree_replace_first(new_sub, context_name);
	    }

	    new_sub->next = tree2;

	    /* If there was any overlap, recurse to merge in the overlapping
	       region (including anything that may follow the overlap).  */
	    if (new2) {
		return netsnmp_subtree_load(new2, context_name);
	    }
	}
    } else {
	/*  If the new subtree starts *within* an existing registration
	    (rather than at the same point as it), then split the existing
	    subtree at this point.  */

	if (netsnmp_oid_equals(new_sub->start_a, new_sub->start_len, 
			     tree1->start_a,   tree1->start_len) != 0) {
	    tree1 = netsnmp_subtree_split(tree1, new_sub->start_a, 
					  new_sub->start_len);
	}

        if (tree1 == NULL) {
            return MIB_REGISTRATION_FAILED;
	}

	/*  Now consider the end of this existing subtree:
	    
	    If it matches the new subtree precisely,
	            simply merge the new one into the list of children

	    If it includes the whole of the new subtree,
		    split it at the appropriate point, and merge again
     
	    If the new subtree extends beyond this existing region,
	            split it, and recurse to merge the two parts.  */

	rc = snmp_oid_compare(new_sub->end_a, new_sub->end_len, 
			      tree1->end_a, tree1->end_len);

        switch (rc) {

	case -1:
	    /*  Existing subtree contains new one.  */
	    netsnmp_subtree_split(tree1, new_sub->end_a, new_sub->end_len);
	    /* Fall Through */

	case  0:
	    /*  The two trees match precisely.  */

	    /*  Note: This is the only point where the original registration
	        OID ("name") is used.  */

	    prev = NULL;
	    next = tree1;
	
	    while (next && next->namelen > new_sub->namelen) {
		prev = next;
		next = next->children;
	    }

	    while (next && next->namelen == new_sub->namelen &&
		   next->priority < new_sub->priority ) {
		prev = next;
		next = next->children;
	    }
	
	    if (next && (next->namelen  == new_sub->namelen) &&
		(next->priority == new_sub->priority)) {
		return MIB_DUPLICATE_REGISTRATION;
	    }

	    if (prev) {
		prev->children    = new_sub;
		new_sub->children = next;
		new_sub->prev = prev->prev;
		new_sub->next = prev->next;
	    } else {
		new_sub->children = next;
		new_sub->prev = next->prev;
		new_sub->next = next->next;
	
		for (next = new_sub->next; next != NULL;next = next->children){
		    next->prev = new_sub;
		}

		for (prev = new_sub->prev; prev != NULL;prev = prev->children){
		    prev->next = new_sub;
		}
	    }
	    break;

	case  1:
	    /*  New subtree contains the existing one.  */
	    new2 = netsnmp_subtree_split(new_sub, tree1->end_a,tree1->end_len);
	    res = netsnmp_subtree_load(new_sub, context_name);
	    if (res != MIB_REGISTERED_OK) {
		netsnmp_subtree_free(new2);
		return res;
	    }
	    return netsnmp_subtree_load(new2, context_name);
	}
    }
    return 0;
}

int
netsnmp_register_mib(const char *moduleName,
                     struct variable *var,
                     size_t varsize,
                     size_t numvars,
                     oid * mibloc,
                     size_t mibloclen,
                     int priority,
                     int range_subid,
                     oid range_ubound,
                     netsnmp_session * ss,
                     const char *context,
                     int timeout,
                     int flags,
                     netsnmp_handler_registration *reginfo,
                     int perform_callback)
{
    netsnmp_subtree *subtree, *sub2;
    int             res, i;
    struct register_parameters reg_parms;
    int old_lookup_cache_val = netsnmp_get_lookup_cache_size();

    subtree = (netsnmp_subtree *)calloc(1, sizeof(netsnmp_subtree));
    if (subtree == NULL) {
        return MIB_REGISTRATION_FAILED;
    }

    DEBUGMSGTL(("register_mib", "registering \"%s\" at ", moduleName));
    DEBUGMSGOIDRANGE(("register_mib", mibloc, mibloclen, range_subid,
                      range_ubound));
    DEBUGMSG(("register_mib", "\n"));

    /*  Create the new subtree node being registered.  */

    subtree->name_a  = snmp_duplicate_objid(mibloc, mibloclen);
    subtree->start_a = snmp_duplicate_objid(mibloc, mibloclen);
    subtree->end_a   = snmp_duplicate_objid(mibloc, mibloclen);
    subtree->label_a = strdup(moduleName);
    if (subtree->name_a == NULL || subtree->start_a == NULL || 
	subtree->end_a  == NULL || subtree->label_a == NULL) {
	netsnmp_subtree_free(subtree);
	return MIB_REGISTRATION_FAILED;
    }
    subtree->namelen   = (u_char)mibloclen;
    subtree->start_len = (u_char)mibloclen;
    subtree->end_len   = (u_char)mibloclen;
    subtree->end_a[mibloclen - 1]++;

    if (var != NULL) {
	subtree->variables = (struct variable *)malloc(varsize*numvars);
	if (subtree->variables == NULL) {
	    netsnmp_subtree_free(subtree);
	    return MIB_REGISTRATION_FAILED;
	}
	memcpy(subtree->variables, var, numvars*varsize);
	subtree->variables_len = numvars;
	subtree->variables_width = varsize;
    }
    subtree->priority = priority;
    subtree->timeout = timeout;
    subtree->range_subid = range_subid;
    subtree->range_ubound = range_ubound;
    subtree->session = ss;
    subtree->reginfo = reginfo;
    subtree->flags = (u_char)flags;    /*  used to identify instance oids  */
    subtree->flags |= SUBTREE_ATTACHED;
    subtree->global_cacheid = reginfo->global_cacheid;

    netsnmp_set_lookup_cache_size(0);
    res = netsnmp_subtree_load(subtree, context);

    /*  If registering a range, use the first subtree as a template for the
	rest of the range.  */

    if (res == MIB_REGISTERED_OK && range_subid != 0) {
	for (i = mibloc[range_subid - 1] + 1; i <= (int)range_ubound; i++) {
	    sub2 = netsnmp_subtree_deepcopy(subtree);

	    if (sub2 == NULL) {
                unregister_mib_context(mibloc, mibloclen, priority,
                                       range_subid, range_ubound, context);
                netsnmp_set_lookup_cache_size(old_lookup_cache_val);
                invalidate_lookup_cache(context);
                return MIB_REGISTRATION_FAILED;
            }

            sub2->name_a[range_subid - 1]  = i;
            sub2->start_a[range_subid - 1] = i;
            sub2->end_a[range_subid - 1]   = i;     /* XXX - ???? */
            res = netsnmp_subtree_load(sub2, context);
            sub2->flags |= SUBTREE_ATTACHED;
            if (res != MIB_REGISTERED_OK) {
                unregister_mib_context(mibloc, mibloclen, priority,
                                       range_subid, range_ubound, context);
		netsnmp_subtree_free(sub2);
                netsnmp_set_lookup_cache_size(old_lookup_cache_val);
                invalidate_lookup_cache(context);
                return res;
            }
        }
    } else if (res == MIB_DUPLICATE_REGISTRATION ||
               res == MIB_REGISTRATION_FAILED) {
        netsnmp_subtree_free(subtree);
    }

    /*
     * mark the MIB as detached, if there's no master agent present as of now 
     */
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_ROLE) != MASTER_AGENT) {
        extern struct snmp_session *main_session;
        if (main_session == NULL) {
            register_mib_detach_node(subtree);
	}
    }

    if (perform_callback) {
        reg_parms.name = mibloc;
        reg_parms.namelen = mibloclen;
        reg_parms.priority = priority;
        reg_parms.range_subid = range_subid;
        reg_parms.range_ubound = range_ubound;
        reg_parms.timeout = timeout;
        reg_parms.flags = (u_char) flags;

        /*
         * Should this really be called if the registration hasn't actually 
         * succeeded?  
         */

        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                            SNMPD_CALLBACK_REGISTER_OID, &reg_parms);
    }

    netsnmp_set_lookup_cache_size(old_lookup_cache_val);
    invalidate_lookup_cache(context);
    return res;
}

/*
 * Reattach a particular node.  
 */

static void
register_mib_reattach_node(netsnmp_subtree *s)
{
    if ((s != NULL) && (s->namelen > 1) && !(s->flags & SUBTREE_ATTACHED)) {
        struct register_parameters reg_parms;
        /*
         * only do registrations that are not the top level nodes 
         */
        /*
         * XXX: do this better 
         */
        reg_parms.name = s->name_a;
        reg_parms.namelen = s->namelen;
        reg_parms.priority = s->priority;
        reg_parms.range_subid = s->range_subid;
        reg_parms.range_ubound = s->range_ubound;
        reg_parms.timeout = s->timeout;
        reg_parms.flags = s->flags;
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                            SNMPD_CALLBACK_REGISTER_OID, &reg_parms);
        s->flags |= SUBTREE_ATTACHED;
    }
}

/*
 * Call callbacks to reattach all our nodes.  
 */

void
register_mib_reattach(void)
{
    netsnmp_subtree *s, *t;
    subtree_context_cache *ptr;

    for (ptr = context_subtrees; ptr; ptr = ptr->next) {
        for (s = ptr->first_subtree; s != NULL; s = s->next) {
            register_mib_reattach_node(s);
            for (t = s->children; t != NULL; t = t->children) {
                register_mib_reattach_node(t);
            }
        }
    }
}

/*
 * Mark a node as detached.  
 */

static void
register_mib_detach_node(netsnmp_subtree *s)
{
    if (s != NULL) {
        s->flags = s->flags & ~SUBTREE_ATTACHED;
    }
}

/*
 * Mark all our registered OIDs as detached.  This is only really
 * useful for subagent protocols, when a connection is lost or
 * something.  
 */

void
register_mib_detach(void)
{
    netsnmp_subtree *s, *t;
    subtree_context_cache *ptr;
    for (ptr = context_subtrees; ptr; ptr = ptr->next) {
        for (s = ptr->first_subtree; s != NULL; s = s->next) {
            register_mib_detach_node(s);
            for (t = s->children; t != NULL; t = t->children) {
                register_mib_detach_node(t);
            }
        }
    }
}

int
register_mib_context(const char *moduleName,
                     struct variable *var,
                     size_t varsize,
                     size_t numvars,
                     oid * mibloc,
                     size_t mibloclen,
                     int priority,
                     int range_subid,
                     oid range_ubound,
                     netsnmp_session * ss,
                     const char *context, int timeout, int flags)
{
    return netsnmp_register_old_api(moduleName, var, varsize, numvars,
                                    mibloc, mibloclen, priority,
                                    range_subid, range_ubound, ss, context,
                                    timeout, flags);
}

int
register_mib_range(const char *moduleName,
                   struct variable *var,
                   size_t varsize,
                   size_t numvars,
                   oid * mibloc,
                   size_t mibloclen,
                   int priority,
                   int range_subid, oid range_ubound, netsnmp_session * ss)
{
    return register_mib_context(moduleName, var, varsize, numvars,
                                mibloc, mibloclen, priority,
                                range_subid, range_ubound, ss, "", -1, 0);
}

int
register_mib_priority(const char *moduleName,
                      struct variable *var,
                      size_t varsize,
                      size_t numvars,
                      oid * mibloc, size_t mibloclen, int priority)
{
    return register_mib_range(moduleName, var, varsize, numvars,
                              mibloc, mibloclen, priority, 0, 0, NULL);
}

int
register_mib(const char *moduleName,
             struct variable *var,
             size_t varsize,
             size_t numvars, oid * mibloc, size_t mibloclen)
{
    return register_mib_priority(moduleName, var, varsize, numvars,
                                 mibloc, mibloclen, DEFAULT_MIB_PRIORITY);
}

void
netsnmp_subtree_unload(netsnmp_subtree *sub, netsnmp_subtree *prev, const char *context)
{
    netsnmp_subtree *ptr;

    DEBUGMSGTL(("register_mib", "unload("));
    if (sub != NULL) {
        DEBUGMSGOID(("register_mib", sub->start_a, sub->start_len));
    } else {
        DEBUGMSG(("register_mib", "[NIL]"));
    }
    DEBUGMSG(("register_mib", ", "));
    if (prev != NULL) {
        DEBUGMSGOID(("register_mib", prev->start_a, prev->start_len));
    } else {
        DEBUGMSG(("register_mib", "[NIL]"));
    }
    DEBUGMSG(("register_mib", ")\n"));

    if (prev != NULL) {         /* non-leading entries are easy */
        prev->children = sub->children;
        return;
    }
    /*
     * otherwise, we need to amend our neighbours as well 
     */

    if (sub->children == NULL) {        /* just remove this node completely */
        for (ptr = sub->prev; ptr; ptr = ptr->children)
            ptr->next = sub->next;
        for (ptr = sub->next; ptr; ptr = ptr->children)
            ptr->prev = sub->prev;

	if (sub->prev == NULL) {
	    netsnmp_subtree_replace_first(sub->next, context);
	}

        return;
    } else {
        for (ptr = sub->prev; ptr; ptr = ptr->children)
            ptr->next = sub->children;
        for (ptr = sub->next; ptr; ptr = ptr->children)
            ptr->prev = sub->children;

	if (sub->prev == NULL) {
	    netsnmp_subtree_replace_first(sub->children, context);
	}
        return;
    }
}

int
unregister_mib_context(oid * name, size_t len, int priority,
                       int range_subid, oid range_ubound,
                       const char *context)
{
    netsnmp_subtree *list, *myptr;
    netsnmp_subtree *prev, *child;       /* loop through children */
    struct register_parameters reg_parms;
    int old_lookup_cache_val = netsnmp_get_lookup_cache_size();
    netsnmp_set_lookup_cache_size(0);

    DEBUGMSGTL(("register_mib", "unregistering "));
    DEBUGMSGOIDRANGE(("register_mib", name, len, range_subid, range_ubound));
    DEBUGMSG(("register_mib", "\n"));

    list = netsnmp_subtree_find(name, len, netsnmp_subtree_find_first(context),
				context);
    if (list == NULL) {
        return MIB_NO_SUCH_REGISTRATION;
    }

    for (child = list, prev = NULL; child != NULL;
         prev = child, child = child->children) {
        if (netsnmp_oid_equals(child->name_a, child->namelen, name, len) == 0 &&
            child->priority == priority) {
            break;              /* found it */
	}
    }

    if (child == NULL) {
        return MIB_NO_SUCH_REGISTRATION;
    }

    netsnmp_subtree_unload(child, prev, context);
    myptr = child;              /* remember this for later */

    /*
     *  Now handle any occurances in the following subtrees,
     *      as a result of splitting this range.  Due to the
     *      nature of the way such splits work, the first
     *      subtree 'slice' that doesn't refer to the given
     *      name marks the end of the original region.
     *
     *  This should also serve to register ranges.
     */

    for (list = myptr->next; list != NULL; list = list->next) {
        for (child = list, prev = NULL; child != NULL;
             prev = child, child = child->children) {
            if ((netsnmp_oid_equals(child->name_a, child->namelen,
				  name, len) == 0) &&
		(child->priority == priority)) {
                netsnmp_subtree_unload(child, prev, context);
                netsnmp_subtree_free(child);
                break;
            }
        }
        if (child == NULL)      /* Didn't find the given name */
            break;
    }
    netsnmp_subtree_free(myptr);

    reg_parms.name = name;
    reg_parms.namelen = len;
    reg_parms.priority = priority;
    reg_parms.range_subid = range_subid;
    reg_parms.range_ubound = range_ubound;
    reg_parms.flags = 0x00;     /*  this is okay I think  */
    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_UNREGISTER_OID, &reg_parms);

    netsnmp_set_lookup_cache_size(old_lookup_cache_val);
    invalidate_lookup_cache(context);
    return MIB_UNREGISTERED_OK;
}

int
netsnmp_unregister_mib_table_row(oid * name, size_t len, int priority,
                                 int var_subid, oid range_ubound,
                                 const char *context)
{
    netsnmp_subtree *list, *myptr;
    netsnmp_subtree *prev, *child;       /* loop through children */
    struct register_parameters reg_parms;
    oid             range_lbound = name[var_subid - 1];

    DEBUGMSGTL(("register_mib", "unregistering "));
    DEBUGMSGOIDRANGE(("register_mib", name, len, var_subid, range_ubound));
    DEBUGMSG(("register_mib", "\n"));

    for (; name[var_subid - 1] <= range_ubound; name[var_subid - 1]++) {
        list = netsnmp_subtree_find(name, len, 
				netsnmp_subtree_find_first(context), context);

        if (list == NULL) {
            continue;
        }

        for (child = list, prev = NULL; child != NULL;
             prev = child, child = child->children) {

            if (netsnmp_oid_equals(child->name_a, child->namelen, 
				 name, len) == 0 && 
		(child->priority == priority)) {
                break;          /* found it */
            }
        }

        if (child == NULL) {
            continue;
        }

        netsnmp_subtree_unload(child, prev, context);
        myptr = child;          /* remember this for later */

        for (list = myptr->next; list != NULL; list = list->next) {
            for (child = list, prev = NULL; child != NULL;
                 prev = child, child = child->children) {

                if (netsnmp_oid_equals(child->name_a, child->namelen, 
				      name, len) == 0 &&
                    (child->priority == priority)) {
                    netsnmp_subtree_unload(child, prev, context);
                    netsnmp_subtree_free(child);
                    break;
                }
            }
            if (child == NULL) {        /* Didn't find the given name */
                break;
            }
        }
        netsnmp_subtree_free(myptr);
    }

    name[var_subid - 1] = range_lbound;
    reg_parms.name = name;
    reg_parms.namelen = len;
    reg_parms.priority = priority;
    reg_parms.range_subid = var_subid;
    reg_parms.range_ubound = range_ubound;
    reg_parms.flags = 0x00;     /*  this is okay I think  */
    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_UNREGISTER_OID, &reg_parms);

    return 0;
}

int
unregister_mib_range(oid * name, size_t len, int priority,
                     int range_subid, oid range_ubound)
{
    return unregister_mib_context(name, len, priority, range_subid,
                                  range_ubound, "");
}

int
unregister_mib_priority(oid * name, size_t len, int priority)
{
    return unregister_mib_range(name, len, priority, 0, 0);
}

int
unregister_mib(oid * name, size_t len)
{
    return unregister_mib_priority(name, len, DEFAULT_MIB_PRIORITY);
}

void
unregister_mibs_by_session(netsnmp_session * ss)
{
    netsnmp_subtree *list, *list2;
    netsnmp_subtree *child, *prev, *next_child;
    struct register_parameters rp;
    subtree_context_cache *contextptr;

    DEBUGMSGTL(("register_mib", "unregister_mibs_by_session(%p) ctxt \"%s\"\n",
		ss, ss->contextName ? ss->contextName : "[NIL]"));

    for (contextptr = get_top_context_cache(); contextptr != NULL;
         contextptr = contextptr->next) {
        for (list = contextptr->first_subtree; list != NULL; list = list2) {
            list2 = list->next;

            for (child = list, prev = NULL; child != NULL; child = next_child){
                next_child = child->children;

                if (((ss->flags & SNMP_FLAGS_SUBSESSION) &&
		     child->session == ss) ||
                    (!(ss->flags & SNMP_FLAGS_SUBSESSION) && child->session &&
                     child->session->subsession == ss)) {

                    rp.name = child->name_a;
		    child->name_a = NULL;
                    rp.namelen = child->namelen;
                    rp.priority = child->priority;
                    rp.range_subid = child->range_subid;
                    rp.range_ubound = child->range_ubound;
                    rp.timeout = child->timeout;
                    rp.flags = child->flags;

                    if (child->reginfo != NULL) {
                        /*
                         * Don't let's free the session pointer just yet!  
                         */
                        child->reginfo->handler->myvoid = NULL;
                        netsnmp_handler_registration_free(child->reginfo);
			child->reginfo = NULL;
                    }

                    netsnmp_subtree_unload(child, prev, contextptr->context_name);
                    netsnmp_subtree_free(child);

                    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                                        SNMPD_CALLBACK_UNREGISTER_OID, &rp);
		    SNMP_FREE(rp.name);
                } else {
                    prev = child;
                }
            }
        }
        netsnmp_subtree_join(contextptr->first_subtree);
    }
}

/*
 * in_a_view: determines if a given snmp_pdu is allowed to see a
 * given name/namelen OID pointer
 * name         IN - name of var, OUT - name matched
 * nameLen      IN -number of sub-ids in name, OUT - subid-is in matched name
 * pi           IN - relevant auth info re PDU 
 * cvp          IN - relevant auth info re mib module
 */

int
in_a_view(oid *name, size_t *namelen, netsnmp_pdu *pdu, int type)
{
    struct view_parameters view_parms;

    if (pdu->flags & UCD_MSG_FLAG_ALWAYS_IN_VIEW) {
	/* Enable bypassing of view-based access control */
        return VACM_SUCCESS;
    }

    /*
     * check for v1 and counter64s, since snmpv1 doesn't support it 
     */
    if (pdu->version == SNMP_VERSION_1 && type == ASN_COUNTER64) {
        return VACM_NOTINVIEW;
    }

    view_parms.pdu = pdu;
    view_parms.name = name;
    if (namelen != NULL) {
        view_parms.namelen = *namelen;
    } else {
        view_parms.namelen = 0;
    }
    view_parms.errorcode = 0;
    view_parms.check_subtree = 0;

    switch (pdu->version) {
    case SNMP_VERSION_1:
    case SNMP_VERSION_2c:
    case SNMP_VERSION_3:
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                            SNMPD_CALLBACK_ACM_CHECK, &view_parms);
        return view_parms.errorcode;
    }
    return VACM_NOSECNAME;
}

/*
 * check_acces: determines if a given snmp_pdu is ever going to be
 * allowed to do anynthing or if it's not going to ever be
 * authenticated.
 */
int
check_access(netsnmp_pdu *pdu)
{                               /* IN - pdu being checked */
    struct view_parameters view_parms;
    view_parms.pdu = pdu;
    view_parms.name = 0;
    view_parms.namelen = 0;
    view_parms.errorcode = 0;
    view_parms.check_subtree = 0;

    if (pdu->flags & UCD_MSG_FLAG_ALWAYS_IN_VIEW) {
	/* Enable bypassing of view-based access control */
        return 0;
    }

    switch (pdu->version) {
    case SNMP_VERSION_1:
    case SNMP_VERSION_2c:
    case SNMP_VERSION_3:
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                            SNMPD_CALLBACK_ACM_CHECK_INITIAL, &view_parms);
        return view_parms.errorcode;
    }
    return 1;
}

/** checks to see if everything within a
 *  given subtree is either: in view, not in view, or possibly both.
 *  If the entire subtree is not-in-view we can use this information to
 *  skip calling the sub-handlers entirely.
 *  @returns 0 if entire subtree is accessible, 5 if not and 7 if
 *  portions are both.  1 on error (illegal pdu version).
 */
int
netsnmp_acm_check_subtree(netsnmp_pdu *pdu, oid *name, size_t namelen)
{                               /* IN - pdu being checked */
    struct view_parameters view_parms;
    view_parms.pdu = pdu;
    view_parms.name = name;
    view_parms.namelen = namelen;
    view_parms.errorcode = 0;
    view_parms.check_subtree = 1;

    if (pdu->flags & UCD_MSG_FLAG_ALWAYS_IN_VIEW) {
	/* Enable bypassing of view-based access control */
        return 0;
    }

    switch (pdu->version) {
    case SNMP_VERSION_1:
    case SNMP_VERSION_2c:
    case SNMP_VERSION_3:
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                            SNMPD_CALLBACK_ACM_CHECK_SUBTREE, &view_parms);
        return view_parms.errorcode;
    }
    return 1;
}

#define SUBTREE_DEFAULT_CACHE_SIZE 8
#define SUBTREE_MAX_CACHE_SIZE     32
int lookup_cache_size = 0; /*enabled later after registrations are loaded */

typedef struct lookup_cache_s {
   netsnmp_subtree *next;
   netsnmp_subtree *previous;
} lookup_cache;

typedef struct lookup_cache_context_s {
   char *context;
   struct lookup_cache_context_s *next;
   int thecachecount;
   int currentpos;
   lookup_cache cache[SUBTREE_MAX_CACHE_SIZE];
} lookup_cache_context;

static lookup_cache_context *thecontextcache = NULL;

/** set the lookup cache size for optimized agent registration performance.
 * @param newsize set to the maximum size of a cache for a given
 * context.  Set to 0 to completely disable caching, or to -1 to set
 * to the default cache size (8), or to a number of your chosing.  The
 * rough guide is that it should be equal to the maximum number of
 * simultanious managers you expect to talk to the agent (M) times 80%
 * (or so, he says randomly) the average number (N) of varbinds you
 * expect to receive in a given request for a manager.  ie, M times N.
 * Bigger does NOT necessarily mean better.  Certainly 16 should be an
 * upper limit.  32 is the hard coded limit.
 */
void
netsnmp_set_lookup_cache_size(int newsize) {
    if (newsize < 0)
        lookup_cache_size = SUBTREE_DEFAULT_CACHE_SIZE;
    else if (newsize < SUBTREE_MAX_CACHE_SIZE)
        lookup_cache_size = newsize;
    else
        lookup_cache_size = SUBTREE_MAX_CACHE_SIZE;
}

/** retrieves the current value of the lookup cache size
 *  @return the current lookup cache size
 */
int
netsnmp_get_lookup_cache_size(void) {
    return lookup_cache_size;
}

static NETSNMP_INLINE lookup_cache_context *
get_context_lookup_cache(const char *context) {
    lookup_cache_context *ptr;
    if (!context)
        context = "";

    for(ptr = thecontextcache; ptr; ptr = ptr->next) {
        if (strcmp(ptr->context, context) == 0)
            break;
    }
    if (!ptr) {
        if (netsnmp_subtree_find_first(context)) {
            ptr = SNMP_MALLOC_TYPEDEF(lookup_cache_context);
            ptr->next = thecontextcache;
            ptr->context = strdup(context);
            thecontextcache = ptr;
        } else {
            return NULL;
        }
    }
    return ptr;
}

static NETSNMP_INLINE void
lookup_cache_add(const char *context,
                 netsnmp_subtree *next, netsnmp_subtree *previous) {
    lookup_cache_context *cptr;

    if ((cptr = get_context_lookup_cache(context)) == NULL)
        return;
    
    if (cptr->thecachecount < lookup_cache_size)
        cptr->thecachecount++;

    cptr->cache[cptr->currentpos].next = next;
    cptr->cache[cptr->currentpos].previous = previous;

    if (++cptr->currentpos >= lookup_cache_size)
        cptr->currentpos = 0;
}

static NETSNMP_INLINE void
lookup_cache_replace(lookup_cache *ptr,
                     netsnmp_subtree *next, netsnmp_subtree *previous) {

    ptr->next = next;
    ptr->previous = previous;
}

static NETSNMP_INLINE lookup_cache *
lookup_cache_find(const char *context, oid *name, size_t name_len,
                  int *retcmp) {
    lookup_cache_context *cptr;
    lookup_cache *ret = NULL;
    int cmp;
    int i;

    if ((cptr = get_context_lookup_cache(context)) == NULL)
        return NULL;

    for(i = 0; i < cptr->thecachecount && i < lookup_cache_size; i++) {
        if (cptr->cache[i].previous->start_a)
            cmp = snmp_oid_compare(name, name_len,
                                   cptr->cache[i].previous->start_a,
                                   cptr->cache[i].previous->start_len);
        else
            cmp = 1;
        if (cmp >= 0) {
            *retcmp = cmp;
            ret = &(cptr->cache[i]);
        }
    }
    return ret;
}

static NETSNMP_INLINE void
invalidate_lookup_cache(const char *context) {
    lookup_cache_context *cptr;
    if ((cptr = get_context_lookup_cache(context)) != NULL) {
        cptr->thecachecount = 0;
        cptr->currentpos = 0;
    }
}

netsnmp_subtree *
netsnmp_subtree_find_prev(oid *name, size_t len, netsnmp_subtree *subtree,
			  const char *context_name)
{
    lookup_cache *lookup_cache = NULL;
    netsnmp_subtree *myptr = NULL, *previous = NULL;
    int cmp = 1;

    if (subtree) {
        myptr = subtree;
    } else {
	/* look through everything */
        if (lookup_cache_size) {
            lookup_cache = lookup_cache_find(context_name, name, len, &cmp);
            if (lookup_cache) {
                myptr = lookup_cache->next;
                previous = lookup_cache->previous;
            }
            if (!myptr)
                myptr = netsnmp_subtree_find_first(context_name);
        } else {
            myptr = netsnmp_subtree_find_first(context_name);
        }
    }

    for (; myptr != NULL; previous = myptr, myptr = myptr->next) {
        if (snmp_oid_compare(name, len, myptr->start_a, myptr->start_len) < 0) {
            if (lookup_cache_size && previous && cmp) {
                if (lookup_cache) {
                    lookup_cache_replace(lookup_cache, myptr, previous);
                } else {
                    lookup_cache_add(context_name, myptr, previous);
                }
            }
            return previous;
        }
    }
    return previous;
}

netsnmp_subtree *
netsnmp_subtree_find_next(oid *name, size_t len,
			  netsnmp_subtree *subtree, const char *context_name)
{
    netsnmp_subtree *myptr = NULL;

    myptr = netsnmp_subtree_find_prev(name, len, subtree, context_name);

    if (myptr != NULL) {
        myptr = myptr->next;
        while (myptr != NULL && (myptr->variables == NULL || 
				 myptr->variables_len == 0)) {
            myptr = myptr->next;
        }
        return myptr;
    } else if (subtree != NULL && snmp_oid_compare(name, len, 
				   subtree->start_a, subtree->start_len) < 0) {
        return subtree;
    } else {
        return NULL;
    }
}

netsnmp_subtree *
netsnmp_subtree_find(oid *name, size_t len, netsnmp_subtree *subtree, 
		     const char *context_name)
{
    netsnmp_subtree *myptr;

    myptr = netsnmp_subtree_find_prev(name, len, subtree, context_name);
    if (myptr && myptr->end_a &&
        snmp_oid_compare(name, len, myptr->end_a, myptr->end_len)<0) {
        return myptr;
    }

    return NULL;
}

netsnmp_session *
get_session_for_oid(oid *name, size_t len, const char *context_name)
{
    netsnmp_subtree *myptr;

    myptr = netsnmp_subtree_find_prev(name, len, 
				      netsnmp_subtree_find_first(context_name),
				      context_name);

    while (myptr && myptr->variables == NULL) {
        myptr = myptr->next;
    }

    if (myptr == NULL) {
        return NULL;
    } else {
        return myptr->session;
    }
}

void
setup_tree(void)
{
    oid ccitt[1]           = { 0 };
    oid iso[1]             = { 1 };
    oid joint_ccitt_iso[1] = { 2 };

#ifdef USING_AGENTX_SUBAGENT_MODULE
    int role =  netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
				       NETSNMP_DS_AGENT_ROLE);

    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 
			   MASTER_AGENT);
#endif

    netsnmp_register_null(ccitt, 1);
    netsnmp_register_null(iso, 1);
    netsnmp_register_null(joint_ccitt_iso, 1);

#ifdef USING_AGENTX_SUBAGENT_MODULE
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 
			   role);
#endif
}


extern void     dump_idx_registry(void);
void
dump_registry(void)
{
    struct variable *vp = NULL;
    netsnmp_subtree *myptr, *myptr2;
    u_char *s = NULL, *e = NULL, *v = NULL;
    size_t sl = 256, el = 256, vl = 256, sl_o = 0, el_o = 0, vl_o = 0;
    int i = 0;

    if ((s = (u_char *) calloc(sl, 1)) != NULL &&
        (e = (u_char *) calloc(sl, 1)) != NULL &&
        (v = (u_char *) calloc(sl, 1)) != NULL) {

        subtree_context_cache *ptr;
        for (ptr = context_subtrees; ptr; ptr = ptr->next) {
            printf("Subtrees for Context: %s\n", ptr->context_name);
            for (myptr = ptr->first_subtree; myptr != NULL;
                 myptr = myptr->next) {
                sl_o = el_o = vl_o = 0;

                if (!sprint_realloc_objid(&s, &sl, &sl_o, 1,
                                          myptr->start_a,
                                          myptr->start_len)) {
                    break;
                }
                if (!sprint_realloc_objid(&e, &el, &el_o, 1,
                                          myptr->end_a,
					  myptr->end_len)) {
                    break;
                }

                if (myptr->variables) {
                    printf("%02x ( %s - %s ) [", myptr->flags, s, e);
                    for (i = 0, vp = myptr->variables;
                         i < myptr->variables_len; i++) {
                        vl_o = 0;
                        if (!sprint_realloc_objid
                            (&v, &vl, &vl_o, 1, vp->name, vp->namelen)) {
                            break;
                        }
                        printf("%s, ", v);
                        vp = (struct variable *) ((char *) vp +
                                                  myptr->variables_width);
                    }
                    printf("]\n");
                } else {
                    printf("%02x   %s - %s  \n", myptr->flags, s, e);
                }
                for (myptr2 = myptr; myptr2 != NULL;
                     myptr2 = myptr2->children) {
                    if (myptr2->label_a && myptr2->label_a[0]) {
                        if (strcmp(myptr2->label_a, "old_api") == 0) {
                            struct variable *vp =
                                myptr2->reginfo->handler->myvoid;

                            sprint_realloc_objid(&s, &sl, &sl_o, 1,
                                                 vp->name, vp->namelen);
                            printf("\t%s[%s] %p var %s\n", myptr2->label_a,
                                   myptr2->reginfo->handlerName ? myptr2->
                                   reginfo->handlerName : "no-name",
                                   myptr2->reginfo, s);
                        } else {
                            printf("\t%s %s %p\n", myptr2->label_a,
                                   myptr2->reginfo->handlerName ? myptr2->
                                   reginfo->
                                   handlerName : "no-handler-name",
                                   myptr2->reginfo);
                        }
                    }
                }
            }
        }
    }

    if (s != NULL) {
        free(s);
    }
    if (e != NULL) {
        free(e);
    }
    if (v != NULL) {
        free(v);
    }

    dump_idx_registry();
}


int	external_readfd[NUM_EXTERNAL_FDS],   external_readfdlen   = 0;
int	external_writefd[NUM_EXTERNAL_FDS],  external_writefdlen  = 0;
int	external_exceptfd[NUM_EXTERNAL_FDS], external_exceptfdlen = 0;
void  (*external_readfdfunc[NUM_EXTERNAL_FDS]) (int, void *);
void  (*external_writefdfunc[NUM_EXTERNAL_FDS]) (int, void *);
void  (*external_exceptfdfunc[NUM_EXTERNAL_FDS]) (int, void *);
void   *external_readfd_data[NUM_EXTERNAL_FDS];
void   *external_writefd_data[NUM_EXTERNAL_FDS];
void   *external_exceptfd_data[NUM_EXTERNAL_FDS];

int
register_readfd(int fd, void (*func) (int, void *), void *data)
{
    if (external_readfdlen < NUM_EXTERNAL_FDS) {
        external_readfd[external_readfdlen] = fd;
        external_readfdfunc[external_readfdlen] = func;
        external_readfd_data[external_readfdlen] = data;
        external_readfdlen++;
        DEBUGMSGTL(("register_readfd", "registered fd %d\n", fd));
        return FD_REGISTERED_OK;
    } else {
        snmp_log(LOG_CRIT, "register_readfd: too many file descriptors\n");
        return FD_REGISTRATION_FAILED;
    }
}

int
register_writefd(int fd, void (*func) (int, void *), void *data)
{
    if (external_writefdlen < NUM_EXTERNAL_FDS) {
        external_writefd[external_writefdlen] = fd;
        external_writefdfunc[external_writefdlen] = func;
        external_writefd_data[external_writefdlen] = data;
        external_writefdlen++;
        DEBUGMSGTL(("register_writefd", "registered fd %d\n", fd));
        return FD_REGISTERED_OK;
    } else {
        snmp_log(LOG_CRIT,
                 "register_writefd: too many file descriptors\n");
        return FD_REGISTRATION_FAILED;
    }
}

int
register_exceptfd(int fd, void (*func) (int, void *), void *data)
{
    if (external_exceptfdlen < NUM_EXTERNAL_FDS) {
        external_exceptfd[external_exceptfdlen] = fd;
        external_exceptfdfunc[external_exceptfdlen] = func;
        external_exceptfd_data[external_exceptfdlen] = data;
        external_exceptfdlen++;
        DEBUGMSGTL(("register_exceptfd", "registered fd %d\n", fd));
        return FD_REGISTERED_OK;
    } else {
        snmp_log(LOG_CRIT,
                 "register_exceptfd: too many file descriptors\n");
        return FD_REGISTRATION_FAILED;
    }
}

int
unregister_readfd(int fd)
{
    int             i, j;

    for (i = 0; i < external_readfdlen; i++) {
        if (external_readfd[i] == fd) {
            external_readfdlen--;
            for (j = i; j < external_readfdlen; j++) {
                external_readfd[j] = external_readfd[j + 1];
                external_readfdfunc[j] = external_readfdfunc[j + 1];
                external_readfd_data[j] = external_readfd_data[j + 1];
            }
            DEBUGMSGTL(("unregister_readfd", "unregistered fd %d\n", fd));
            return FD_UNREGISTERED_OK;
        }
    }
    return FD_NO_SUCH_REGISTRATION;
}

int
unregister_writefd(int fd)
{
    int             i, j;

    for (i = 0; i < external_writefdlen; i++) {
        if (external_writefd[i] == fd) {
            external_writefdlen--;
            for (j = i; j < external_writefdlen; j++) {
                external_writefd[j] = external_writefd[j + 1];
                external_writefdfunc[j] = external_writefdfunc[j + 1];
                external_writefd_data[j] = external_writefd_data[j + 1];
            }
            DEBUGMSGTL(("unregister_writefd", "unregistered fd %d\n", fd));
            return FD_UNREGISTERED_OK;
        }
    }
    return FD_NO_SUCH_REGISTRATION;
}

int
unregister_exceptfd(int fd)
{
    int             i, j;

    for (i = 0; i < external_exceptfdlen; i++) {
        if (external_exceptfd[i] == fd) {
            external_exceptfdlen--;
            for (j = i; j < external_exceptfdlen; j++) {
                external_exceptfd[j] = external_exceptfd[j + 1];
                external_exceptfdfunc[j] = external_exceptfdfunc[j + 1];
                external_exceptfd_data[j] = external_exceptfd_data[j + 1];
            }
            DEBUGMSGTL(("unregister_exceptfd", "unregistered fd %d\n",
                        fd));
            return FD_UNREGISTERED_OK;
        }
    }
    return FD_NO_SUCH_REGISTRATION;
}

int             external_signal_scheduled[NUM_EXTERNAL_SIGS];
void            (*external_signal_handler[NUM_EXTERNAL_SIGS]) (int);

#ifndef WIN32

/*
 * TODO: add agent_SIGXXX_handler functions and `case SIGXXX: ...' lines
 *       below for every single that might be handled by register_signal().
 */

RETSIGTYPE
agent_SIGCHLD_handler(int sig)
{
    external_signal_scheduled[SIGCHLD]++;
#ifndef HAVE_SIGACTION
    /*
     * signal() sucks. It *might* have SysV semantics, which means that
     * * a signal handler is reset once it gets called. Ensure that it
     * * remains active.
     */
    signal(SIGCHLD, agent_SIGCHLD_handler);
#endif
}

int
register_signal(int sig, void (*func) (int))
{

    switch (sig) {
#if defined(SIGCHLD)
    case SIGCHLD:
#ifdef HAVE_SIGACTION
        {
            static struct sigaction act;
            act.sa_handler = agent_SIGCHLD_handler;
            sigemptyset(&act.sa_mask);
            act.sa_flags = 0;
            sigaction(SIGCHLD, &act, NULL);
        }
#else
        signal(SIGCHLD, agent_SIGCHLD_handler);
#endif
        break;
#endif
    default:
        snmp_log(LOG_CRIT,
                 "register_signal: signal %d cannot be handled\n", sig);
        return SIG_REGISTRATION_FAILED;
    }

    external_signal_handler[sig] = func;
    external_signal_scheduled[sig] = 0;

    DEBUGMSGTL(("register_signal", "registered signal %d\n", sig));
    return SIG_REGISTERED_OK;
}

int
unregister_signal(int sig)
{
    signal(sig, SIG_DFL);
    DEBUGMSGTL(("unregister_signal", "unregistered signal %d\n", sig));
    return SIG_UNREGISTERED_OK;
}

#endif                          /* !WIN32 */
