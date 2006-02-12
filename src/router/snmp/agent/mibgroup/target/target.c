#include <net-snmp/net-snmp-config.h>

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "snmpTargetAddrEntry.h"
#include "snmpTargetParamsEntry.h"
#include "target.h"

#define MAX_TAGS 128

netsnmp_session *
get_target_sessions(char *taglist, TargetFilterFunction * filterfunct,
                    void *filterArg)
{
    netsnmp_session *ret = NULL, thissess;
    struct targetAddrTable_struct *targaddrs;
    char            buf[SPRINT_MAX_LEN];
    char            tags[MAX_TAGS][SPRINT_MAX_LEN], *cp;
    int             numtags = 0, i;
    static struct targetParamTable_struct *param;

    DEBUGMSGTL(("target_sessions", "looking for: %s\n", taglist));
    for (cp = taglist; cp && numtags < MAX_TAGS;) {
        cp = copy_nword(cp, tags[numtags], sizeof(tags[numtags]));
        DEBUGMSGTL(("target_sessions", " for: %d=%s\n", numtags,
                    tags[numtags]));
        numtags++;
    }

    for (targaddrs = get_addrTable(); targaddrs;
         targaddrs = targaddrs->next) {

        /*
         * legal row? 
         */
        if (targaddrs->tDomain == NULL ||
            targaddrs->tAddress == NULL ||
            targaddrs->rowStatus != SNMP_ROW_ACTIVE) {
            DEBUGMSGTL(("target_sessions", "  which is not ready yet\n"));
            continue;
        }

        if (netsnmp_tdomain_support
            (targaddrs->tDomain, targaddrs->tDomainLen, NULL, NULL) == 0) {
            snmp_log(LOG_ERR,
                     "unsupported domain for target address table entry %s\n",
                     targaddrs->name);
        }

        /*
         * check tag list to see if we match 
         */
        if (targaddrs->tagList) {
            /*
             * loop through tag list looking for requested tags 
             */
            for (cp = targaddrs->tagList; cp;) {
                cp = copy_nword(cp, buf, sizeof(buf));
                for (i = 0; i < numtags; i++) {
                    if (strcmp(buf, tags[i]) == 0) {
                        /*
                         * found a valid target table entry 
                         */
                        DEBUGMSGTL(("target_sessions", "found one: %s\n",
                                    tags[i]));

                        if (targaddrs->params) {
                            param = get_paramEntry(targaddrs->params);
                            if (!param
                                || param->rowStatus != SNMP_ROW_ACTIVE) {
                                /*
                                 * parameter entry must exist and be active 
                                 */
                                continue;
                            }
                        } else {
                            /*
                             * parameter entry must be specified 
                             */
                            continue;
                        }

                        /*
                         * last chance for caller to opt-out.  Call
                         * filtering function 
                         */
                        if (filterfunct &&
                            (*(filterfunct)) (targaddrs, param,
                                              filterArg)) {
                            continue;
                        }

                        if (targaddrs->storageType != ST_READONLY &&
                            targaddrs->sess &&
                            param->updateTime >=
                            targaddrs->sessionCreationTime) {
                            /*
                             * parameters have changed, nuke the old session 
                             */
                            snmp_close(targaddrs->sess);
                            targaddrs->sess = NULL;
                        }

                        /*
                         * target session already exists? 
                         */
                        if (targaddrs->sess == NULL) {
                            /*
                             * create an appropriate snmp session and add
                             * it to our return list 
                             */
                            netsnmp_transport *t = NULL;

                            t = netsnmp_tdomain_transport_oid(targaddrs->
                                                              tDomain,
                                                              targaddrs->
                                                              tDomainLen,
                                                              targaddrs->
                                                              tAddress,
                                                              targaddrs->
                                                              tAddressLen,
                                                              0);
                            if (t == NULL) {
                                DEBUGMSGTL(("target_sessions",
                                            "bad dest \""));
                                DEBUGMSGOID(("target_sessions",
                                             targaddrs->tDomain,
                                             targaddrs->tDomainLen));
                                DEBUGMSG(("target_sessions", "\", \""));
                                DEBUGMSGHEX(("target_sessions",
                                             targaddrs->tAddress,
                                             targaddrs->tAddressLen));
                                DEBUGMSG(("target_sessions", "\n"));
                                continue;
                            } else {
                                char           *dst_str =
                                    t->f_fmtaddr(t, NULL, 0);
                                if (dst_str != NULL) {
                                    DEBUGMSGTL(("target_sessions",
                                                "  to: %s\n", dst_str));
                                    free(dst_str);
                                }
                            }
                            memset(&thissess, 0, sizeof(thissess));
                            thissess.timeout = (targaddrs->timeout) * 1000;
                            thissess.retries = targaddrs->retryCount;
                            DEBUGMSGTL(("target_sessions",
                                        "timeout: %d -> %d\n",
                                        targaddrs->timeout,
                                        thissess.timeout));

                            if (param->mpModel == SNMP_VERSION_3 &&
                                param->secModel != 3) {
                                snmp_log(LOG_ERR,
                                         "unsupported model/secmodel combo for target %s\n",
                                         targaddrs->name);
                                /*
                                 * XXX: memleak 
                                 */
                                netsnmp_transport_free(t);
                                continue;
                            }
                            thissess.version = param->mpModel;
                            if (param->mpModel == SNMP_VERSION_3) {
                                thissess.securityName = param->secName;
                                thissess.securityNameLen =
                                    strlen(thissess.securityName);
                                thissess.securityLevel = param->secLevel;
                            } else {
                                thissess.community =
                                    (u_char *) strdup(param->secName);
                                thissess.community_len =
                                    strlen((char *) thissess.community);
                            }

                            targaddrs->sess = snmp_add(&thissess, t,
                                                       NULL, NULL);
                            targaddrs->sessionCreationTime = time(NULL);
                        }
                        if (targaddrs->sess) {
                            if (ret) {
                                targaddrs->sess->next = ret;
                            }
                            ret = targaddrs->sess;
                        } else {
                            snmp_sess_perror("target session", &thissess);
                        }
                    }
                }
            }
        }
    }
    return ret;
}
