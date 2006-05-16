/*
 * snmp_client.c - a toolkit of common functions for an SNMP client.
 *
 */
/**********************************************************************
	Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <errno.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
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
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include <net-snmp/types.h>

#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/snmp_client.h>
#include <net-snmp/library/snmp_secmod.h>
#include <net-snmp/library/mib.h>


#ifndef BSD4_3
#define BSD4_2
#endif

#ifndef FD_SET

typedef long    fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)        /* bits per mask */

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	memset((p), 0, sizeof(*(p)))
#endif

/*
 * Prototype definitions 
 */
static int      snmp_synch_input(int op, netsnmp_session * session,
                                 int reqid, netsnmp_pdu *pdu, void *magic);

netsnmp_pdu    *
snmp_pdu_create(int command)
{
    netsnmp_pdu    *pdu;

    pdu = (netsnmp_pdu *) calloc(1, sizeof(netsnmp_pdu));
    if (pdu) {
        pdu->version = SNMP_DEFAULT_VERSION;
        pdu->command = command;
        pdu->errstat = SNMP_DEFAULT_ERRSTAT;
        pdu->errindex = SNMP_DEFAULT_ERRINDEX;
        pdu->securityModel = SNMP_DEFAULT_SECMODEL;
        pdu->transport_data = NULL;
        pdu->transport_data_length = 0;
        pdu->securityNameLen = 0;
        pdu->contextNameLen = 0;
        pdu->time = 0;
        pdu->reqid = snmp_get_next_reqid();
        pdu->msgid = snmp_get_next_msgid();
    }
    return pdu;

}


/*
 * Add a null variable with the requested name to the end of the list of
 * variables for this pdu.
 */
netsnmp_variable_list *
snmp_add_null_var(netsnmp_pdu *pdu, const oid * name, size_t name_length)
{
    return snmp_pdu_add_variable(pdu, name, name_length, ASN_NULL, NULL, 0);
}


static int
snmp_synch_input(int op,
                 netsnmp_session * session,
                 int reqid, netsnmp_pdu *pdu, void *magic)
{
    struct synch_state *state = (struct synch_state *) magic;
    int             rpt_type;

    if (reqid != state->reqid && pdu && pdu->command != SNMP_MSG_REPORT) {
        return 0;
    }

    state->waiting = 0;

    if (op == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
        if (pdu->command == SNMP_MSG_REPORT) {
            rpt_type = snmpv3_get_report_type(pdu);
            if (SNMPV3_IGNORE_UNAUTH_REPORTS ||
                rpt_type == SNMPERR_NOT_IN_TIME_WINDOW) {
                state->waiting = 1;
            }
            state->pdu = NULL;
            state->status = STAT_ERROR;
            session->s_snmp_errno = rpt_type;
            SET_SNMP_ERROR(rpt_type);
        } else if (pdu->command == SNMP_MSG_RESPONSE) {
            /*
             * clone the pdu to return to snmp_synch_response 
             */
            state->pdu = snmp_clone_pdu(pdu);
            state->status = STAT_SUCCESS;
            session->s_snmp_errno = SNMPERR_SUCCESS;
        }
    } else if (op == NETSNMP_CALLBACK_OP_TIMED_OUT) {
        state->pdu = NULL;
        state->status = STAT_TIMEOUT;
        session->s_snmp_errno = SNMPERR_TIMEOUT;
        SET_SNMP_ERROR(SNMPERR_TIMEOUT);
    } else if (op == NETSNMP_CALLBACK_OP_DISCONNECT) {
        state->pdu = NULL;
        state->status = STAT_ERROR;
        session->s_snmp_errno = SNMPERR_ABORT;
        SET_SNMP_ERROR(SNMPERR_ABORT);
    }

    return 1;
}


/*
 * Clone an SNMP variable data structure.
 * Sets pointers to structure private storage, or
 * allocates larger object identifiers and values as needed.
 *
 * Caller must make list association for cloned variable.
 *
 * Returns 0 if successful.
 */
int
snmp_clone_var(netsnmp_variable_list * var, netsnmp_variable_list * newvar)
{
    if (!newvar || !var)
        return 1;

    memmove(newvar, var, sizeof(netsnmp_variable_list));
    newvar->next_variable = 0;
    newvar->name = 0;
    newvar->val.string = 0;
    newvar->data = 0;
    newvar->dataFreeHook = 0;
    newvar->index = 0;

    /*
     * Clone the object identifier and the value.
     * Allocate memory iff original will not fit into local storage.
     */
    if (snmp_set_var_objid(newvar, var->name, var->name_length))
        return 1;

    /*
     * need a pointer and a length to copy a string value. 
     */
    if (var->val.string && var->val_len) {
        if (var->val.string != &var->buf[0]) {
            if (var->val_len <= sizeof(var->buf))
                newvar->val.string = newvar->buf;
            else {
                newvar->val.string = (u_char *) malloc(var->val_len);
                if (!newvar->val.string)
                    return 1;
            }
            memmove(newvar->val.string, var->val.string, var->val_len);
        } else {                /* fix the pointer to new local store */
            newvar->val.string = newvar->buf;
        }
    } else {
        newvar->val.string = 0;
        newvar->val_len = 0;
    }

    return 0;
}


/*
 * Possibly make a copy of source memory buffer.
 * Will reset destination pointer if source pointer is NULL.
 * Returns 0 if successful, 1 if memory allocation fails.
 */
int
snmp_clone_mem(void **dstPtr, void *srcPtr, unsigned len)
{
    *dstPtr = 0;
    if (srcPtr) {
        *dstPtr = malloc(len + 1);
        if (!*dstPtr) {
            return 1;
        }
        memmove(*dstPtr, srcPtr, len);
        /*
         * this is for those routines that expect 0-terminated strings!!!
         * someone should rather have called strdup
         */
        ((char *) *dstPtr)[len] = 0;
    }
    return 0;
}


/*
 * Walks through a list of varbinds and frees and allocated memory,
 * restoring pointers to local buffers
 */
void
snmp_reset_var_buffers(netsnmp_variable_list * var)
{
    while (var) {
        if (var->name != var->name_loc) {
            free(var->name);
            var->name = var->name_loc;
            var->name_length = 0;
        }
        if (var->val.string != var->buf) {
            free(var->val.string);
            var->val.string = var->buf;
            var->val_len = 0;
        }
        var = var->next_variable;
    }
}

/*
 * Creates and allocates a clone of the input PDU,
 * but does NOT copy the variables.
 * This function should be used with another function,
 * such as _copy_pdu_vars.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure.
 */
static
netsnmp_pdu    *
_clone_pdu_header(netsnmp_pdu *pdu)
{
    netsnmp_pdu    *newpdu;
    struct snmp_secmod_def *sptr;

    newpdu = (netsnmp_pdu *) malloc(sizeof(netsnmp_pdu));
    if (!newpdu)
        return 0;
    memmove(newpdu, pdu, sizeof(netsnmp_pdu));

    /*
     * reset copied pointers if copy fails 
     */
    newpdu->variables = 0;
    newpdu->enterprise = 0;
    newpdu->community = 0;
    newpdu->securityEngineID = 0;
    newpdu->securityName = 0;
    newpdu->contextEngineID = 0;
    newpdu->contextName = 0;
    newpdu->transport_data = 0;

    /*
     * copy buffers individually. If any copy fails, all are freed. 
     */
    if (snmp_clone_mem((void **) &newpdu->enterprise, pdu->enterprise,
                       sizeof(oid) * pdu->enterprise_length) ||
        snmp_clone_mem((void **) &newpdu->community, pdu->community,
                       pdu->community_len) ||
        snmp_clone_mem((void **) &newpdu->contextEngineID,
                       pdu->contextEngineID, pdu->contextEngineIDLen)
        || snmp_clone_mem((void **) &newpdu->securityEngineID,
                          pdu->securityEngineID, pdu->securityEngineIDLen)
        || snmp_clone_mem((void **) &newpdu->contextName, pdu->contextName,
                          pdu->contextNameLen)
        || snmp_clone_mem((void **) &newpdu->securityName,
                          pdu->securityName, pdu->securityNameLen)
        || snmp_clone_mem((void **) &newpdu->transport_data,
                          pdu->transport_data,
                          pdu->transport_data_length)) {
        snmp_free_pdu(newpdu);
        return 0;
    }
    if ((sptr = find_sec_mod(newpdu->securityModel)) != NULL &&
        sptr->pdu_clone != NULL) {
        /*
         * call security model if it needs to know about this 
         */
        (*sptr->pdu_clone) (pdu, newpdu);
    }

    return newpdu;
}

static
netsnmp_variable_list *
_copy_varlist(netsnmp_variable_list * var,      /* source varList */
              int errindex,     /* index of variable to drop (if any) */
              int copy_count)
{                               /* !=0 number variables to copy */
    netsnmp_variable_list *newhead, *newvar, *oldvar;
    int             ii = 0;

    newhead = NULL;
    oldvar = NULL;

    while (var && (copy_count-- > 0)) {
        /*
         * Drop the specified variable (if applicable) 
         */
        if (++ii == errindex) {
            var = var->next_variable;
            continue;
        }

        /*
         * clone the next variable. Cleanup if alloc fails 
         */
        newvar = (netsnmp_variable_list *)
            malloc(sizeof(netsnmp_variable_list));
        if (snmp_clone_var(var, newvar)) {
            if (newvar)
                free((char *) newvar);
            snmp_free_varbind(newhead);
            return 0;
        }

        /*
         * add cloned variable to new list  
         */
        if (0 == newhead)
            newhead = newvar;
        if (oldvar)
            oldvar->next_variable = newvar;
        oldvar = newvar;

        var = var->next_variable;
    }
    return newhead;
}


/*
 * Copy some or all variables from source PDU to target PDU.
 * This function consolidates many of the needs of PDU variables:
 * Clone PDU : copy all the variables.
 * Split PDU : skip over some variables to copy other variables.
 * Fix PDU   : remove variable associated with error index.
 *
 * Designed to work with _clone_pdu_header.
 *
 * If drop_err is set, drop any variable associated with errindex.
 * If skip_count is set, skip the number of variable in pdu's list.
 * While copy_count is greater than zero, copy pdu variables to newpdu.
 *
 * If an error occurs, newpdu is freed and pointer is set to 0.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure.
 */
static
netsnmp_pdu    *
_copy_pdu_vars(netsnmp_pdu *pdu,        /* source PDU */
               netsnmp_pdu *newpdu,     /* target PDU */
               int drop_err,    /* !=0 drop errored variable */
               int skip_count,  /* !=0 number of variables to skip */
               int copy_count)
{                               /* !=0 number of variables to copy */
    netsnmp_variable_list *var, *oldvar;
    int             ii, copied, drop_idx;

    if (!newpdu)
        return 0;               /* where is PDU to copy to ? */

    if (drop_err)
        drop_idx = pdu->errindex - skip_count;
    else
        drop_idx = 0;

    var = pdu->variables;
    while (var && (skip_count-- > 0))   /* skip over pdu variables */
        var = var->next_variable;

    oldvar = 0;
    ii = 0;
    copied = 0;
    if (pdu->flags & UCD_MSG_FLAG_FORCE_PDU_COPY)
        copied = 1;             /* We're interested in 'empty' responses too */

    newpdu->variables = _copy_varlist(var, drop_idx, copy_count);
    if (newpdu->variables)
        copied = 1;

#if ALSO_TEMPORARILY_DISABLED
    /*
     * Error if bad errindex or if target PDU has no variables copied 
     */
    if ((drop_err && (ii < pdu->errindex))
#if TEMPORARILY_DISABLED
        /*
         * SNMPv3 engineID probes are allowed to be empty.
         * See the comment in snmp_api.c for further details 
         */
        || copied == 0
#endif
        ) {
        snmp_free_pdu(newpdu);
        return 0;
    }
#endif
    return newpdu;
}


/*
 * Creates (allocates and copies) a clone of the input PDU.
 * If drop_err is set, don't copy any variable associated with errindex.
 * This function is called by snmp_clone_pdu and snmp_fix_pdu.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure.
 */
static
netsnmp_pdu    *
_clone_pdu(netsnmp_pdu *pdu, int drop_err)
{
    netsnmp_pdu    *newpdu;
    newpdu = _clone_pdu_header(pdu);
    newpdu = _copy_pdu_vars(pdu, newpdu, drop_err, 0, 10000);   /* skip none, copy all */

    return newpdu;
}


/*
 * This function will clone a full varbind list
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure
 */
netsnmp_variable_list *
snmp_clone_varbind(netsnmp_variable_list * varlist)
{
    return _copy_varlist(varlist, 0, 10000);    /* skip none, copy all */
}

/*
 * This function will clone a PDU including all of its variables.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure
 */
netsnmp_pdu    *
snmp_clone_pdu(netsnmp_pdu *pdu)
{
    return _clone_pdu(pdu, 0);  /* copies all variables */
}


/*
 * This function will clone a PDU including some of its variables.
 *
 * If skip_count is not zero, it defines the number of variables to skip.
 * If copy_count is not zero, it defines the number of variables to copy.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure.
 */
netsnmp_pdu    *
snmp_split_pdu(netsnmp_pdu *pdu, int skip_count, int copy_count)
{
    netsnmp_pdu    *newpdu;
    newpdu = _clone_pdu_header(pdu);
    newpdu = _copy_pdu_vars(pdu, newpdu, 0,     /* don't drop any variables */
                            skip_count, copy_count);

    return newpdu;
}


/*
 * If there was an error in the input pdu, creates a clone of the pdu
 * that includes all the variables except the one marked by the errindex.
 * The command is set to the input command and the reqid, errstat, and
 * errindex are set to default values.
 * If the error status didn't indicate an error, the error index didn't
 * indicate a variable, the pdu wasn't a get response message, or there
 * would be no remaining variables, this function will return 0.
 * If everything was successful, a pointer to the fixed cloned pdu will
 * be returned.
 */
netsnmp_pdu    *
snmp_fix_pdu(netsnmp_pdu *pdu, int command)
{
    netsnmp_pdu    *newpdu;

    if ((pdu->command != SNMP_MSG_RESPONSE)
        || (pdu->errstat == SNMP_ERR_NOERROR)
        || (0 == pdu->variables)
        || (pdu->errindex <= 0)) {
        return 0;               /* pre-condition tests fail */
    }

    newpdu = _clone_pdu(pdu, 1);        /* copies all except errored variable */
    if (!newpdu)
        return 0;
    if (!newpdu->variables) {
        snmp_free_pdu(newpdu);
        return 0;               /* no variables. "should not happen" */
    }
    newpdu->command = command;
    newpdu->reqid = snmp_get_next_reqid();
    newpdu->msgid = snmp_get_next_msgid();
    newpdu->errstat = SNMP_DEFAULT_ERRSTAT;
    newpdu->errindex = SNMP_DEFAULT_ERRINDEX;

    return newpdu;
}


/*
 * Returns the number of variables bound to a PDU structure
 */
unsigned long
snmp_varbind_len(netsnmp_pdu *pdu)
{
    register netsnmp_variable_list *vars;
    unsigned long   retVal = 0;
    if (pdu)
        for (vars = pdu->variables; vars; vars = vars->next_variable) {
            retVal++;
        }

    return retVal;
}

/*
 * Add object identifier name to SNMP variable.
 * If the name is large, additional memory is allocated.
 * Returns 0 if successful.
 */

int
snmp_set_var_objid(netsnmp_variable_list * vp,
                   const oid * objid, size_t name_length)
{
    size_t          len = sizeof(oid) * name_length;

    if (vp->name != vp->name_loc && vp->name != NULL &&
        vp->name_length > (sizeof(vp->name_loc) / sizeof(oid))) {
        /*
         * Probably previously-allocated "big storage".  Better free it
         * else memory leaks possible.  
         */
        free(vp->name);
    }

    /*
     * use built-in storage for smaller values 
     */
    if (len <= sizeof(vp->name_loc)) {
        vp->name = vp->name_loc;
    } else {
        vp->name = (oid *) malloc(len);
        if (!vp->name)
            return 1;
    }
    if (objid)
        memmove(vp->name, objid, len);
    vp->name_length = name_length;
    return 0;
}

int
snmp_set_var_typed_value(netsnmp_variable_list * newvar, u_char type,
                         const u_char * val_str, size_t val_len)
{
    newvar->type = type;
    return snmp_set_var_value(newvar, val_str, val_len);
}

int
count_varbinds(netsnmp_variable_list * var_ptr)
{
    int             count = 0;

    for (; var_ptr != NULL; var_ptr = var_ptr->next_variable)
        count++;

    return count;
}

int
count_varbinds_of_type(netsnmp_variable_list * var_ptr, u_char type)
{
    int             count = 0;

    for (; var_ptr != NULL; var_ptr = var_ptr->next_variable)
        if (var_ptr->type == type)
            count++;

    return count;
}

netsnmp_variable_list *
find_varbind_of_type(netsnmp_variable_list * var_ptr, u_char type)
{
    for (; var_ptr != NULL && var_ptr->type != type;
         var_ptr = var_ptr->next_variable);

    return var_ptr;
}

/*
 * Add some value to SNMP variable.
 * If the value is large, additional memory is allocated.
 * Returns 0 if successful.
 */

int
snmp_set_var_value(netsnmp_variable_list * newvar,
                   const u_char * val_str, size_t val_len)
{
    if (newvar->val.string && newvar->val.string != newvar->buf) {
        free(newvar->val.string);
    }

    newvar->val.string = 0;
    newvar->val_len = 0;

    /*
     * need a pointer and a length to copy a string value. 
     */
    if (val_str && val_len) {
        if (val_len <= sizeof(newvar->buf))
            newvar->val.string = newvar->buf;
        else {
            newvar->val.string = (u_char *) malloc(val_len);
            if (!newvar->val.string)
                return 1;
        }
        memmove(newvar->val.string, val_str, val_len);
        newvar->val_len = val_len;
    } else if (val_str) {
        /*
         * NULL STRING != NULL ptr 
         */
        newvar->val.string = newvar->buf;
        newvar->val.string[0] = '\0';
        newvar->val_len = 0;
    }
    return 0;
}

void
snmp_replace_var_types(netsnmp_variable_list * vbl, u_char old_type,
                       u_char new_type)
{
    while (vbl) {
        if (vbl->type == old_type) {
            snmp_set_var_typed_value(vbl, new_type, NULL, 0);
        }
        vbl = vbl->next_variable;
    }
}

void
snmp_reset_var_types(netsnmp_variable_list * vbl, u_char new_type)
{
    while (vbl) {
        snmp_set_var_typed_value(vbl, new_type, NULL, 0);
        vbl = vbl->next_variable;
    }
}

int
snmp_synch_response_cb(netsnmp_session * ss,
                       netsnmp_pdu *pdu,
                       netsnmp_pdu **response, snmp_callback pcb)
{
    struct synch_state lstate, *state;
    snmp_callback   cbsav;
    void           *cbmagsav;
    int             numfds, count;
    fd_set          fdset;
    struct timeval  timeout, *tvp;
    int             block;

    memset((void *) &lstate, 0, sizeof(lstate));
    state = &lstate;
    cbsav = ss->callback;
    cbmagsav = ss->callback_magic;
    ss->callback = pcb;
    ss->callback_magic = (void *) state;

    if ((state->reqid = snmp_send(ss, pdu)) == 0) {
        snmp_free_pdu(pdu);
        state->status = STAT_ERROR;
    } else
        state->waiting = 1;

    while (state->waiting) {
        numfds = 0;
        FD_ZERO(&fdset);
        block = SNMPBLOCK;
        tvp = &timeout;
        timerclear(tvp);
        snmp_select_info(&numfds, &fdset, tvp, &block);
        if (block == 1)
            tvp = NULL;         /* block without timeout */
        count = select(numfds, &fdset, 0, 0, tvp);
        if (count > 0) {
            snmp_read(&fdset);
        } else
            switch (count) {
            case 0:
                snmp_timeout();
                break;
            case -1:
                if (errno == EINTR) {
                    continue;
                } else {
                    snmp_errno = SNMPERR_GENERR;
                    /*
                     * CAUTION! if another thread closed the socket(s)
                     * waited on here, the session structure was freed.
                     * It would be nice, but we can't rely on the pointer.
                     * ss->s_snmp_errno = SNMPERR_GENERR;
                     * ss->s_errno = errno;
                     */
                    snmp_set_detail(strerror(errno));
                }
                /*
                 * FALLTHRU 
                 */
            default:
                state->status = STAT_ERROR;
                state->waiting = 0;
            }
    }
    *response = state->pdu;
    ss->callback = cbsav;
    ss->callback_magic = cbmagsav;
    return state->status;
}

int
snmp_synch_response(netsnmp_session * ss,
                    netsnmp_pdu *pdu, netsnmp_pdu **response)
{
    return snmp_synch_response_cb(ss, pdu, response, snmp_synch_input);
}

int
snmp_sess_synch_response(void *sessp,
                         netsnmp_pdu *pdu, netsnmp_pdu **response)
{
    netsnmp_session *ss;
    struct synch_state lstate, *state;
    snmp_callback   cbsav;
    void           *cbmagsav;
    int             numfds, count;
    fd_set          fdset;
    struct timeval  timeout, *tvp;
    int             block;

    ss = snmp_sess_session(sessp);
    memset((void *) &lstate, 0, sizeof(lstate));
    state = &lstate;
    cbsav = ss->callback;
    cbmagsav = ss->callback_magic;
    ss->callback = snmp_synch_input;
    ss->callback_magic = (void *) state;

    if ((state->reqid = snmp_sess_send(sessp, pdu)) == 0) {
        snmp_free_pdu(pdu);
        state->status = STAT_ERROR;
    } else
        state->waiting = 1;

    while (state->waiting) {
        numfds = 0;
        FD_ZERO(&fdset);
        block = SNMPBLOCK;
        tvp = &timeout;
        timerclear(tvp);
        snmp_sess_select_info(sessp, &numfds, &fdset, tvp, &block);
        if (block == 1)
            tvp = NULL;         /* block without timeout */
        count = select(numfds, &fdset, 0, 0, tvp);
        if (count > 0) {
            snmp_sess_read(sessp, &fdset);
        } else
            switch (count) {
            case 0:
                snmp_sess_timeout(sessp);
                break;
            case -1:
                if (errno == EINTR) {
                    continue;
                } else {
                    snmp_errno = SNMPERR_GENERR;
                    /*
                     * CAUTION! if another thread closed the socket(s)
                     * waited on here, the session structure was freed.
                     * It would be nice, but we can't rely on the pointer.
                     * ss->s_snmp_errno = SNMPERR_GENERR;
                     * ss->s_errno = errno;
                     */
                    snmp_set_detail(strerror(errno));
                }
                /*
                 * FALLTHRU 
                 */
            default:
                state->status = STAT_ERROR;
                state->waiting = 0;
            }
    }
    *response = state->pdu;
    ss->callback = cbsav;
    ss->callback_magic = cbmagsav;
    return state->status;
}


const char     *error_string[19] = {
    "(noError) No Error",
    "(tooBig) Response message would have been too large.",
    "(noSuchName) There is no such variable name in this MIB.",
    "(badValue) The value given has the wrong type or length.",
    "(readOnly) The two parties used do not have access to use the specified SNMP PDU.",
    "(genError) A general failure occured",
    "noAccess",
    "wrongType (The set datatype does not match the data type the agent expects)",
    "wrongLength (The set value has an illegal length from what the agent expects)",
    "wrongEncoding",
    "wrongValue (The set value is illegal or unsupported in some way)",
    "noCreation (that table does not support row creation)",
    "inconsistentValue (The set value is illegal or unsupported in some way)",
    "resourceUnavailable (This is likely a out-of-memory failure within the agent)",
    "commitFailed",
    "undoFailed",
    "authorizationError (access denied to that object)",
    "notWritable (that object does not support modification)",
    "inconsistentName"
};

const char     *
snmp_errstring(int errstat)
{
    if (errstat <= MAX_SNMP_ERR && errstat >= SNMP_ERR_NOERROR) {
        return error_string[errstat];
    } else {
        return "Unknown Error";
    }
}
