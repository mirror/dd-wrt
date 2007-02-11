#include <net-snmp/net-snmp-config.h>

#include <stdlib.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <net-snmp/agent/scalar_group.h>
#include <net-snmp/agent/scalar.h>
#include <net-snmp/agent/serialize.h>
#include <net-snmp/agent/read_only.h>

/** @defgroup scalar_group_group scalar_group
 *  Process groups of scalars.
 *  @ingroup leaf
 *  @{
 */
netsnmp_mib_handler *
netsnmp_get_scalar_group_handler(oid first, oid last)
{
    netsnmp_mib_handler  *ret    = NULL;
    netsnmp_scalar_group *sgroup = NULL;

    ret = netsnmp_create_handler("scalar_group",
                                  netsnmp_scalar_group_helper_handler);
    if (ret) {
        sgroup = SNMP_MALLOC_TYPEDEF(netsnmp_scalar_group);
        if (NULL == sgroup) {
            netsnmp_handler_free(ret);
            ret = NULL;
        }
        else {
	    sgroup->lbound = first;
	    sgroup->ubound = last;
            ret->myvoid = (void *)sgroup;
	}
    }
    return ret;
}

int
netsnmp_register_scalar_group(netsnmp_handler_registration *reginfo,
                              oid first, oid last)
{
    netsnmp_inject_handler(reginfo, netsnmp_get_instance_handler());
    netsnmp_inject_handler(reginfo, netsnmp_get_scalar_handler());
    netsnmp_inject_handler(reginfo, netsnmp_get_scalar_group_handler(first, last));
    return netsnmp_register_serialize(reginfo);
}


int
netsnmp_scalar_group_helper_handler(netsnmp_mib_handler *handler,
                                netsnmp_handler_registration *reginfo,
                                netsnmp_agent_request_info *reqinfo,
                                netsnmp_request_info *requests)
{
    netsnmp_variable_list *var = requests->requestvb;

    netsnmp_scalar_group *sgroup = (netsnmp_scalar_group *)handler->myvoid;
    int             ret, cmp;
    int             namelen;
    oid             subid, root_tmp[MAX_OID_LEN], *root_save;

    DEBUGMSGTL(("helper:scalar_group", "Got request:\n"));
    namelen = SNMP_MIN(requests->requestvb->name_length,
                       reginfo->rootoid_len);
    cmp = snmp_oid_compare(requests->requestvb->name, namelen,
                           reginfo->rootoid, reginfo->rootoid_len);

    DEBUGMSGTL(( "helper:scalar_group", "  cmp=%d, oid:", cmp));
    DEBUGMSGOID(("helper:scalar_group", var->name, var->name_length));
    DEBUGMSG((   "helper:scalar_group", "\n"));

    /*
     * copy root oid to root_tmp, set instance to 0. (subid set later on)
     * save rootoid, since we'll replace it before calling next handler,
     * and need to restore it afterwards.
     */
    memcpy(root_tmp, reginfo->rootoid, reginfo->rootoid_len * sizeof(oid));
    root_tmp[reginfo->rootoid_len + 1] = 0;
    root_save = reginfo->rootoid;

    ret = SNMP_ERR_NOCREATION;
    switch (reqinfo->mode) {
    /*
     * The handling of "exact" requests is basically the same.
     * The only difference between GET and SET requests is the
     *     error/exception to return on failure.
     */
    case MODE_GET:
        ret = SNMP_NOSUCHOBJECT;
        /* Fallthrough */

    case MODE_SET_RESERVE1:
    case MODE_SET_RESERVE2:
    case MODE_SET_ACTION:
    case MODE_SET_COMMIT:
    case MODE_SET_UNDO:
    case MODE_SET_FREE:
        if (cmp != 0 ||
            requests->requestvb->name_length <= reginfo->rootoid_len) {
	    /*
	     * Common prefix doesn't match, or only *just* matches 
	     *  the registered root (so can't possibly match a scalar)
	     */
            netsnmp_set_request_error(reqinfo, requests, ret);
            return SNMP_ERR_NOERROR;
        } else {
	    /*
	     * Otherwise,
	     *     extract the object subidentifier from the request, 
	     *     check this is (probably) valid, and then fudge the
	     *     registered 'rootoid' to match, before passing the
	     *     request off to the next handler ('scalar').
	     *
	     * Note that we don't bother checking instance subidentifiers
	     *     here.  That's left to the scalar helper.
	     */
            subid = requests->requestvb->name[reginfo->rootoid_len];
	    if (subid < sgroup->lbound ||
	        subid > sgroup->ubound) {
                netsnmp_set_request_error(reqinfo, requests, ret);
                return SNMP_ERR_NOERROR;
	    }
            root_tmp[reginfo->rootoid_len++] = subid;
            reginfo->rootoid = root_tmp;
            ret = netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                            requests);
            reginfo->rootoid = root_save;
            reginfo->rootoid_len--;
            return ret;
        }
        break;

    case MODE_GETNEXT:
	/*
	 * If we're being asked for something before (or exactly matches)
	 *     the registered root OID, then start with the first object.
	 * If we're being asked for something that exactly matches an object
	 *    OID, then that's what we pass down.
	 * Otherwise, we pass down the OID of the *next* object....
	 */
        if (cmp < 0 ||
            requests->requestvb->name_length <= reginfo->rootoid_len) {
            subid  = sgroup->lbound;
        } else if (requests->requestvb->name_length == reginfo->rootoid_len+1)
            subid = requests->requestvb->name[reginfo->rootoid_len];
        else
            subid = requests->requestvb->name[reginfo->rootoid_len]+1;

	/*
	 * ... always assuming this is (potentially) valid, of course.
	 */
	if (subid > sgroup->ubound)
            return SNMP_ERR_NOERROR;
        
        root_tmp[reginfo->rootoid_len++] = subid;
        reginfo->rootoid = root_tmp;
        ret = netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                            requests);
        /*
         * If we didn't get an answer (due to holes in the group)
	 *   set things up to retry again.
         */
        if (!requests->delegated &&
            (requests->requestvb->type == ASN_NULL ||
             requests->requestvb->type == SNMP_NOSUCHOBJECT ||
             requests->requestvb->type == SNMP_NOSUCHINSTANCE)) {
            snmp_set_var_objid(requests->requestvb,
                               reginfo->rootoid, reginfo->rootoid_len);
            requests->requestvb->name[reginfo->rootoid_len-1] = ++subid;
            requests->requestvb->type = ASN_PRIV_RETRY;
        }
        reginfo->rootoid = root_save;
        reginfo->rootoid_len--;
        return ret;
    }
    /*
     * got here only if illegal mode found 
     */
    return SNMP_ERR_GENERR;
}

/** @} 
 */
