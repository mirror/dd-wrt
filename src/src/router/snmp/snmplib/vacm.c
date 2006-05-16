/*
 * vacm.c
 *
 * SNMPv3 View-based Access Control Model
 */

#include <net-snmp/net-snmp-config.h>

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
#include <stdio.h>
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

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>

#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/vacm.h>

static struct vacm_viewEntry *viewList = NULL, *viewScanPtr = NULL;
static struct vacm_accessEntry *accessList = NULL, *accessScanPtr = NULL;
static struct vacm_groupEntry *groupList = NULL, *groupScanPtr = NULL;

void
vacm_save(const char *token, const char *type)
{
    struct vacm_viewEntry *vptr;
    struct vacm_accessEntry *aptr;
    struct vacm_groupEntry *gptr;

    for (vptr = viewList; vptr != NULL; vptr = vptr->next) {
        if (vptr->viewStorageType == ST_NONVOLATILE)
            vacm_save_view(vptr, token, type);
    }

    for (aptr = accessList; aptr != NULL; aptr = aptr->next) {
        if (aptr->storageType == ST_NONVOLATILE)
            vacm_save_access(aptr, token, type);
    }

    for (gptr = groupList; gptr != NULL; gptr = gptr->next) {
        if (gptr->storageType == ST_NONVOLATILE)
            vacm_save_group(gptr, token, type);
    }
}

/*
 * vacm_save_view(): saves a view entry to the persistent cache 
 */
void
vacm_save_view(struct vacm_viewEntry *view, const char *token,
               const char *type)
{
    char            line[4096];
    char           *cptr;

    memset(line, 0, sizeof(line));
    snprintf(line, sizeof(line), "%s%s %d %d %d ", token, "View",
            view->viewStatus, view->viewStorageType, view->viewType);
    line[ sizeof(line)-1 ] = 0;
    cptr = &line[strlen(line)]; /* the NULL */

    cptr =
        read_config_save_octet_string(cptr, (u_char *) view->viewName + 1,
                                      view->viewName[0] + 1);
    *cptr++ = ' ';
    cptr =
        read_config_save_objid(cptr, view->viewSubtree,
                               view->viewSubtreeLen);
    *cptr++ = ' ';
    cptr = read_config_save_octet_string(cptr, (u_char *) view->viewMask,
                                         view->viewMaskLen);

    read_config_store(type, line);
}

void
vacm_parse_config_view(const char *token, char *line)
{
    struct vacm_viewEntry view;
    struct vacm_viewEntry *vptr;
    char           *viewName = (char *) &view.viewName;
    oid            *viewSubtree = (oid *) & view.viewSubtree;
    u_char         *viewMask;
    size_t          len;

    view.viewStatus = atoi(line);
    line = skip_token(line);
    view.viewStorageType = atoi(line);
    line = skip_token(line);
    view.viewType = atoi(line);
    line = skip_token(line);
    line =
        read_config_read_octet_string(line, (u_char **) & viewName, &len);
    view.viewSubtreeLen = MAX_OID_LEN;
    line =
        read_config_read_objid(line, (oid **) & viewSubtree,
                               &view.viewSubtreeLen);

    vptr =
        vacm_createViewEntry(view.viewName, view.viewSubtree,
                             view.viewSubtreeLen);
    if (!vptr)
        return;

    vptr->viewStatus = view.viewStatus;
    vptr->viewStorageType = view.viewStorageType;
    vptr->viewType = view.viewType;
    viewMask = (u_char *) vptr->viewMask;
    line =
        read_config_read_octet_string(line, (u_char **) & viewMask,
                                      &vptr->viewMaskLen);
}

/*
 * vacm_save_access(): saves an access entry to the persistent cache 
 */
void
vacm_save_access(struct vacm_accessEntry *access_entry, const char *token,
                 const char *type)
{
    char            line[4096];
    char           *cptr;

    memset(line, 0, sizeof(line));
    snprintf(line, sizeof(line), "%s%s %d %d %d %d %d ",
            token, "Access", access_entry->status,
            access_entry->storageType, access_entry->securityModel,
            access_entry->securityLevel, access_entry->contextMatch);
    line[ sizeof(line)-1 ] = 0;
    cptr = &line[strlen(line)]; /* the NULL */
    cptr =
        read_config_save_octet_string(cptr,
                                      (u_char *) access_entry->groupName + 1,
                                      access_entry->groupName[0] + 1);
    *cptr++ = ' ';
    cptr =
        read_config_save_octet_string(cptr,
                                      (u_char *) access_entry->contextPrefix + 1,
                                      access_entry->contextPrefix[0] + 1);

    *cptr++ = ' ';
    cptr = read_config_save_octet_string(cptr, (u_char *) access_entry->readView,
                                         strlen(access_entry->readView) + 1);
    *cptr++ = ' ';
    cptr =
        read_config_save_octet_string(cptr, (u_char *) access_entry->writeView,
                                      strlen(access_entry->writeView) + 1);
    *cptr++ = ' ';
    cptr =
        read_config_save_octet_string(cptr, (u_char *) access_entry->notifyView,
                                      strlen(access_entry->notifyView) + 1);

    read_config_store(type, line);
}

void
vacm_parse_config_access(const char *token, char *line)
{
    struct vacm_accessEntry access;
    struct vacm_accessEntry *aptr;
    char           *contextPrefix = (char *) &access.contextPrefix;
    char           *groupName = (char *) &access.groupName;
    char           *readView, *writeView, *notifyView;
    size_t          len;

    access.status = atoi(line);
    line = skip_token(line);
    access.storageType = atoi(line);
    line = skip_token(line);
    access.securityModel = atoi(line);
    line = skip_token(line);
    access.securityLevel = atoi(line);
    line = skip_token(line);
    access.contextMatch = atoi(line);
    line = skip_token(line);
    line =
        read_config_read_octet_string(line, (u_char **) & groupName, &len);
    line =
        read_config_read_octet_string(line, (u_char **) & contextPrefix,
                                      &len);

    aptr = vacm_createAccessEntry(access.groupName, access.contextPrefix,
                                  access.securityModel,
                                  access.securityLevel);
    if (!aptr)
        return;

    aptr->status = access.status;
    aptr->storageType = access.storageType;
    aptr->securityModel = access.securityModel;
    aptr->securityLevel = access.securityLevel;
    aptr->contextMatch = access.contextMatch;
    readView = (char *) aptr->readView;
    line =
        read_config_read_octet_string(line, (u_char **) & readView, &len);
    writeView = (char *) aptr->writeView;
    line =
        read_config_read_octet_string(line, (u_char **) & writeView, &len);
    notifyView = (char *) aptr->notifyView;
    line =
        read_config_read_octet_string(line, (u_char **) & notifyView,
                                      &len);
}

/*
 * vacm_save_group(): saves a group entry to the persistent cache 
 */
void
vacm_save_group(struct vacm_groupEntry *group_entry, const char *token,
                const char *type)
{
    char            line[4096];
    char           *cptr;

    memset(line, 0, sizeof(line));
    snprintf(line, sizeof(line), "%s%s %d %d %d ",
            token, "Group", group_entry->status,
            group_entry->storageType, group_entry->securityModel);
    line[ sizeof(line)-1 ] = 0;
    cptr = &line[strlen(line)]; /* the NULL */

    cptr =
        read_config_save_octet_string(cptr,
                                      (u_char *) group_entry->securityName + 1,
                                      group_entry->securityName[0] + 1);
    *cptr++ = ' ';
    cptr = read_config_save_octet_string(cptr, (u_char *) group_entry->groupName,
                                         strlen(group_entry->groupName) + 1);

    read_config_store(type, line);
}

void
vacm_parse_config_group(const char *token, char *line)
{
    struct vacm_groupEntry group;
    struct vacm_groupEntry *gptr;
    char           *securityName = (char *) &group.securityName;
    char           *groupName;
    size_t          len;

    group.status = atoi(line);
    line = skip_token(line);
    group.storageType = atoi(line);
    line = skip_token(line);
    group.securityModel = atoi(line);
    line = skip_token(line);
    line =
        read_config_read_octet_string(line, (u_char **) & securityName,
                                      &len);

    gptr = vacm_createGroupEntry(group.securityModel, group.securityName);
    if (!gptr)
        return;

    gptr->status = group.status;
    gptr->storageType = group.storageType;
    groupName = (char *) gptr->groupName;
    line =
        read_config_read_octet_string(line, (u_char **) & groupName, &len);
}

struct vacm_viewEntry *
vacm_getViewEntry(const char *viewName,
                  oid * viewSubtree, size_t viewSubtreeLen, int mode)
{
    struct vacm_viewEntry *vp, *vpret = NULL;
    char            view[VACMSTRINGLEN];
    int             found, glen;
    int count=0;

    glen = (int) strlen(viewName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    view[0] = glen;
    strcpy(view + 1, viewName);
    for (vp = viewList; vp; vp = vp->next) {
        if (!memcmp(view, vp->viewName, glen + 1)
            && viewSubtreeLen >= (vp->viewSubtreeLen - 1)) {
            int             mask = 0x80, maskpos = 0;
            int             oidpos;
            found = 1;

            if (mode != VACM_MODE_IGNORE_MASK) {  /* check the mask */
                for (oidpos = 0;
                     found && oidpos < (int) vp->viewSubtreeLen - 1;
                     oidpos++) {
                    if ((vp->viewMask[maskpos] & mask) != 0) {
                        if (viewSubtree[oidpos] !=
                            vp->viewSubtree[oidpos + 1])
                            found = 0;
                    }
                    if (mask == 1) {
                        mask = 0x80;
                        maskpos++;
                    } else
                        mask >>= 1;
                }
            }
            if (found) {
                /*
                 * match successful, keep this node if its longer than
                 * the previous or (equal and lexicographically greater
                 * than the previous). 
                 */
                count++;
                if (mode == VACM_MODE_CHECK_SUBTREE) {
                    vpret = vp;
                } else if (vpret == NULL
                           || vp->viewSubtreeLen > vpret->viewSubtreeLen
                           || (vp->viewSubtreeLen == vpret->viewSubtreeLen
                               && snmp_oid_compare(vp->viewSubtree + 1,
                                                   vp->viewSubtreeLen - 1,
                                                   vpret->viewSubtree + 1,
                                                   vpret->viewSubtreeLen - 1) >
                               0)) {
                    vpret = vp;
                }
            }
        }
    }
    DEBUGMSGTL(("vacm:getView", ", %s\n", (vpret) ? "found" : "none"));
    if (mode == VACM_MODE_CHECK_SUBTREE && count > 1) {
        return NULL;
    }
    return vpret;
}

/*******************************************************************o-o******
 * vacm_checkSubtree
 *
 * Check to see if everything within a subtree is in view, not in view,
 * or possibly both.
 *
 * Parameters:
 *   *viewName           - Name of view to check
 *   *viewSubtree        - OID of subtree
 *    viewSubtreeLen     - length of subtree OID
 *      
 * Returns:
 *   VACM_SUCCESS          The OID is included in the view.
 *   VACM_NOTINVIEW        If no entry in the view list includes the
 *                         provided OID, or the OID is explicitly excluded
 *                         from the view. 
 *   VACM_SUBTREE_UNKNOWN  The entire subtree has both allowed and disallowed
 *                         portions.
 */
int
vacm_checkSubtree(const char *viewName,
                  oid * viewSubtree, size_t viewSubtreeLen)
{
    struct vacm_viewEntry *vp, *vpShorter = NULL, *vpLonger = NULL;
    char            view[VACMSTRINGLEN];
    int             found, glen;
    int count=0;

    glen = (int) strlen(viewName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return VACM_NOTINVIEW;
    view[0] = glen;
    strcpy(view + 1, viewName);
    for (vp = viewList; vp; vp = vp->next) {
        if (!memcmp(view, vp->viewName, glen + 1)) {
            /*
             * If the subtree defined in the view is shorter than or equal
             * to the subtree we are comparing, then it might envelop the
             * subtree we are comparing against.
             */
            if (viewSubtreeLen >= (vp->viewSubtreeLen - 1)) {
                int             mask = 0x80, maskpos = 0;
                int             oidpos;
                found = 1;

                /*
                 * check the mask
                 */
                for (oidpos = 0;
                     found && oidpos < (int) vp->viewSubtreeLen - 1;
                     oidpos++) {
                    if ((vp->viewMask[maskpos] & mask) != 0) {
                        if (viewSubtree[oidpos] !=
                            vp->viewSubtree[oidpos + 1])
                            found = 0;
                    }
                    if (mask == 1) {
                        mask = 0x80;
                        maskpos++;
                    } else
                        mask >>= 1;
                }

                if (found) {
                    /*
                     * match successful, keep this node if it's longer than
                     * the previous or (equal and lexicographically greater
                     * than the previous). 
                     */
    
                    if (vpShorter == NULL
                        || vp->viewSubtreeLen > vpShorter->viewSubtreeLen
                        || (vp->viewSubtreeLen == vpShorter->viewSubtreeLen
                           && snmp_oid_compare(vp->viewSubtree + 1,
                                               vp->viewSubtreeLen - 1,
                                               vpShorter->viewSubtree + 1,
                                               vpShorter->viewSubtreeLen - 1) >
                                   0)) {
                        vpShorter = vp;
                    }
                }
            }
            /*
             * If the subtree defined in the view is longer than the
             * subtree we are comparing, then it might ambiguate our
             * response.
             */
            else {
                int             mask = 0x80, maskpos = 0;
                int             oidpos;
                found = 1;

                /*
                 * check the mask up to the length of the provided subtree
                 */
                for (oidpos = 0;
                     found && oidpos < (int) viewSubtreeLen;
                     oidpos++) {
                    if ((vp->viewMask[maskpos] & mask) != 0) {
                        if (viewSubtree[oidpos] !=
                            vp->viewSubtree[oidpos + 1])
                            found = 0;
                    }
                    if (mask == 1) {
                        mask = 0x80;
                        maskpos++;
                    } else
                        mask >>= 1;
                }

                if (found) {
                    /*
                     * match successful.  If we already found a match
                     * with a different view type, then parts of the subtree 
                     * are included and others are excluded, so return UNKNOWN.
                     */
                    if (vpLonger != NULL
                        && (vpLonger->viewType != vp->viewType)) {
                        DEBUGMSGTL(("vacm:checkSubtree", ", %s\n", "unknown"));
                        return VACM_SUBTREE_UNKNOWN;
                    }
                    else if (vpLonger == NULL) {
                        vpLonger = vp;
                    }
                }
            }
        }
    }

    /*
     * If we found a matching view subtree with a longer OID than the provided
     * OID, check to see if its type is consistent with any matching view
     * subtree we may have found with a shorter OID than the provided OID.
     *
     * The view type of the longer OID is inconsistent with the shorter OID in
     * either of these two cases:
     *  1) No matching shorter OID was found and the view type of the longer
     *     OID is INCLUDE.
     *  2) A matching shorter ID was found and its view type doesn't match
     *     the view type of the longer OID.
     */
    if (vpLonger != NULL) {
        if ((!vpShorter && vpLonger->viewType != SNMP_VIEW_EXCLUDED)
            || (vpShorter && vpLonger->viewType != vpShorter->viewType)) {
            DEBUGMSGTL(("vacm:checkSubtree", ", %s\n", "unknown"));
            return VACM_SUBTREE_UNKNOWN;
        }
    }

    if (vpShorter && vpShorter->viewType != SNMP_VIEW_EXCLUDED) {
        DEBUGMSGTL(("vacm:checkSubtree", ", %s\n", "included"));
        return VACM_SUCCESS;
    }

    DEBUGMSGTL(("vacm:checkSubtree", ", %s\n", "excluded"));
    return VACM_NOTINVIEW;
}

void
vacm_scanViewInit(void)
{
    viewScanPtr = viewList;
}

struct vacm_viewEntry *
vacm_scanViewNext(void)
{
    struct vacm_viewEntry *returnval = viewScanPtr;
    if (viewScanPtr)
        viewScanPtr = viewScanPtr->next;
    return returnval;
}

struct vacm_viewEntry *
vacm_createViewEntry(const char *viewName,
                     oid * viewSubtree, size_t viewSubtreeLen)
{
    struct vacm_viewEntry *vp, *lp, *op = NULL;
    int             cmp, cmp2, glen;

    glen = (int) strlen(viewName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    vp = (struct vacm_viewEntry *) calloc(1,
                                          sizeof(struct vacm_viewEntry));
    if (vp == NULL)
        return NULL;
    vp->reserved =
        (struct vacm_viewEntry *) calloc(1, sizeof(struct vacm_viewEntry));
    if (vp->reserved == NULL) {
        free(vp);
        return NULL;
    }

    vp->viewName[0] = glen;
    strcpy(vp->viewName + 1, viewName);
    vp->viewSubtree[0] = viewSubtreeLen;
    memcpy(vp->viewSubtree + 1, viewSubtree, viewSubtreeLen * sizeof(oid));
    vp->viewSubtreeLen = viewSubtreeLen + 1;

    lp = viewList;
    while (lp) {
        cmp = memcmp(lp->viewName, vp->viewName, glen + 1);
        cmp2 = snmp_oid_compare(lp->viewSubtree, lp->viewSubtreeLen,
                                vp->viewSubtree, vp->viewSubtreeLen);
        if (cmp == 0 && cmp2 > 0)
            break;
        if (cmp > 0)
            break;
        op = lp;
        lp = lp->next;
    }
    vp->next = lp;
    if (op)
        op->next = vp;
    else
        viewList = vp;
    return vp;
}

void
vacm_destroyViewEntry(const char *viewName,
                      oid * viewSubtree, size_t viewSubtreeLen)
{
    struct vacm_viewEntry *vp, *lastvp = NULL;

    if (viewList && !strcmp(viewList->viewName + 1, viewName)
        && viewList->viewSubtreeLen == viewSubtreeLen
        && !memcmp((char *) viewList->viewSubtree, (char *) viewSubtree,
                   viewSubtreeLen * sizeof(oid))) {
        vp = viewList;
        viewList = viewList->next;
    } else {
        for (vp = viewList; vp; vp = vp->next) {
            if (!strcmp(vp->viewName + 1, viewName)
                && vp->viewSubtreeLen == viewSubtreeLen
                && !memcmp((char *) vp->viewSubtree, (char *) viewSubtree,
                           viewSubtreeLen * sizeof(oid)))
                break;
            lastvp = vp;
        }
        if (!vp)
            return;
        lastvp->next = vp->next;
    }
    if (vp->reserved)
        free(vp->reserved);
    free(vp);
    return;
}

void
vacm_destroyAllViewEntries(void)
{
    struct vacm_viewEntry *vp;
    while ((vp = viewList)) {
        viewList = vp->next;
        if (vp->reserved)
            free(vp->reserved);
        free(vp);
    }
}

struct vacm_groupEntry *
vacm_getGroupEntry(int securityModel, const char *securityName)
{
    struct vacm_groupEntry *vp;
    char            secname[VACMSTRINGLEN];
    int             glen;

    glen = (int) strlen(securityName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    secname[0] = glen;
    strcpy(secname + 1, securityName);

    for (vp = groupList; vp; vp = vp->next) {
        if ((securityModel == vp->securityModel
             || vp->securityModel == SNMP_SEC_MODEL_ANY)
            && !memcmp(vp->securityName, secname, glen + 1))
            return vp;
    }
    return NULL;
}

void
vacm_scanGroupInit(void)
{
    groupScanPtr = groupList;
}

struct vacm_groupEntry *
vacm_scanGroupNext(void)
{
    struct vacm_groupEntry *returnval = groupScanPtr;
    if (groupScanPtr)
        groupScanPtr = groupScanPtr->next;
    return returnval;
}

struct vacm_groupEntry *
vacm_createGroupEntry(int securityModel, const char *securityName)
{
    struct vacm_groupEntry *gp, *lg, *og;
    int             cmp, glen;

    glen = (int) strlen(securityName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    gp = (struct vacm_groupEntry *) calloc(1,
                                           sizeof(struct vacm_groupEntry));
    if (gp == NULL)
        return NULL;
    gp->reserved =
        (struct vacm_groupEntry *) calloc(1,
                                          sizeof(struct vacm_groupEntry));
    if (gp->reserved == NULL) {
        free(gp);
        return NULL;
    }

    gp->securityModel = securityModel;
    gp->securityName[0] = glen;
    strcpy(gp->securityName + 1, securityName);

    lg = groupList;
    og = NULL;
    while (lg) {
        if (lg->securityModel > securityModel)
            break;
        if (lg->securityModel == securityModel &&
            (cmp =
             memcmp(lg->securityName, gp->securityName, glen + 1)) > 0)
            break;
        /*
         * if (lg->securityModel == securityModel && cmp == 0) abort(); 
         */
        og = lg;
        lg = lg->next;
    }
    gp->next = lg;
    if (og == NULL)
        groupList = gp;
    else
        og->next = gp;
    return gp;
}

void
vacm_destroyGroupEntry(int securityModel, const char *securityName)
{
    struct vacm_groupEntry *vp, *lastvp = NULL;

    if (groupList && groupList->securityModel == securityModel
        && !strcmp(groupList->securityName + 1, securityName)) {
        vp = groupList;
        groupList = groupList->next;
    } else {
        for (vp = groupList; vp; vp = vp->next) {
            if (vp->securityModel == securityModel
                && !strcmp(vp->securityName + 1, securityName))
                break;
            lastvp = vp;
        }
        if (!vp)
            return;
        lastvp->next = vp->next;
    }
    if (vp->reserved)
        free(vp->reserved);
    free(vp);
    return;
}

void
vacm_destroyAllGroupEntries(void)
{
    struct vacm_groupEntry *gp;
    while ((gp = groupList)) {
        groupList = gp->next;
        if (gp->reserved)
            free(gp->reserved);
        free(gp);
    }
}

struct vacm_accessEntry *
vacm_getAccessEntry(const char *groupName,
                    const char *contextPrefix,
                    int securityModel, int securityLevel)
{
    struct vacm_accessEntry *vp;
    char            group[VACMSTRINGLEN];
    char            context[VACMSTRINGLEN];
    int             glen, clen;

    glen = (int) strlen(groupName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    clen = (int) strlen(contextPrefix);
    if (clen < 0 || clen >= VACM_MAX_STRING)
        return NULL;

    group[0] = glen;
    strcpy(group + 1, groupName);
    context[0] = clen;
    strcpy(context + 1, contextPrefix);
    for (vp = accessList; vp; vp = vp->next) {
        if ((securityModel == vp->securityModel
             || vp->securityModel == SNMP_SEC_MODEL_ANY)
            && securityLevel >= vp->securityLevel
            && !memcmp(vp->groupName, group, glen + 1)
            &&
            ((vp->contextMatch == CONTEXT_MATCH_EXACT
              && clen == vp->contextPrefix[0]
              && (memcmp(vp->contextPrefix, context, clen + 1) == 0))
             || (vp->contextMatch == CONTEXT_MATCH_PREFIX
                 && clen >= vp->contextPrefix[0]
                 && (memcmp(vp->contextPrefix + 1, context + 1,
                            vp->contextPrefix[0]) == 0))))
            return vp;
    }
    return NULL;
}

void
vacm_scanAccessInit(void)
{
    accessScanPtr = accessList;
}

struct vacm_accessEntry *
vacm_scanAccessNext(void)
{
    struct vacm_accessEntry *returnval = accessScanPtr;
    if (accessScanPtr)
        accessScanPtr = accessScanPtr->next;
    return returnval;
}

struct vacm_accessEntry *
vacm_createAccessEntry(const char *groupName,
                       const char *contextPrefix,
                       int securityModel, int securityLevel)
{
    struct vacm_accessEntry *vp, *lp, *op = NULL;
    int             cmp, glen, clen;

    glen = (int) strlen(groupName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    clen = (int) strlen(contextPrefix);
    if (clen < 0 || clen >= VACM_MAX_STRING)
        return NULL;
    vp = (struct vacm_accessEntry *) calloc(1,
                                            sizeof(struct
                                                   vacm_accessEntry));
    if (vp == NULL)
        return NULL;
    vp->reserved =
        (struct vacm_accessEntry *) calloc(1,
                                           sizeof(struct
                                                  vacm_accessEntry));
    if (vp->reserved == NULL) {
        free(vp);
        return NULL;
    }

    vp->securityModel = securityModel;
    vp->securityLevel = securityLevel;
    vp->groupName[0] = glen;
    strcpy(vp->groupName + 1, groupName);
    vp->contextPrefix[0] = clen;
    strcpy(vp->contextPrefix + 1, contextPrefix);

    lp = accessList;
    while (lp) {
        cmp = memcmp(lp->groupName, vp->groupName, glen + 1);
        if (cmp > 0)
            break;
        if (cmp < 0)
            goto next;
        cmp = memcmp(lp->contextPrefix, vp->contextPrefix, clen + 1);
        if (cmp > 0)
            break;
        if (cmp < 0)
            goto next;
        if (lp->securityModel > securityModel)
            break;
        if (lp->securityModel < securityModel)
            goto next;
        if (lp->securityLevel > securityLevel)
            break;
      next:
        op = lp;
        lp = lp->next;
    }
    vp->next = lp;
    if (op == NULL)
        accessList = vp;
    else
        op->next = vp;
    return vp;
}

void
vacm_destroyAccessEntry(const char *groupName,
                        const char *contextPrefix,
                        int securityModel, int securityLevel)
{
    struct vacm_accessEntry *vp, *lastvp = NULL;

    if (accessList && accessList->securityModel == securityModel
        && accessList->securityModel == securityModel
        && !strcmp(accessList->groupName + 1, groupName)
        && !strcmp(accessList->contextPrefix + 1, contextPrefix)) {
        vp = accessList;
        accessList = accessList->next;
    } else {
        for (vp = accessList; vp; vp = vp->next) {
            if (vp->securityModel == securityModel
                && vp->securityLevel == securityLevel
                && !strcmp(vp->groupName + 1, groupName)
                && !strcmp(vp->contextPrefix + 1, contextPrefix))
                break;
            lastvp = vp;
        }
        if (!vp)
            return;
        lastvp->next = vp->next;
    }
    if (vp->reserved)
        free(vp->reserved);
    free(vp);
    return;
}

void
vacm_destroyAllAccessEntries(void)
{
    struct vacm_accessEntry *ap;
    while ((ap = accessList)) {
        accessList = ap->next;
        if (ap->reserved)
            free(ap->reserved);
        free(ap);
    }
}

int
store_vacm(int majorID, int minorID, void *serverarg, void *clientarg)
{
    /*
     * figure out our application name 
     */
    char           *appname = (char *) clientarg;
    if (appname == NULL) {
        appname = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
					NETSNMP_DS_LIB_APPTYPE);
    }

    /*
     * save the VACM MIB 
     */
    vacm_save("vacm", appname);
    return SNMPERR_SUCCESS;
}

/*
 * returns 1 if vacm has *any* configuration entries in it (regardless
 * of weather or not there is enough to make a decision based on it),
 * else return 0 
 */
int
vacm_is_configured(void)
{
    if (viewList == NULL && accessList == NULL && groupList == NULL) {
        return 0;
    }
    return 1;
}
