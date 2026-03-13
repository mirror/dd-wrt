#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>

/*
 * Portions of this file are copyrighted by:
 * Copyright (c) 2016 VMware, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <sys/types.h>

#include <net-snmp/types.h>
#include <net-snmp/config_api.h>

#include <net-snmp/library/snmp_enum.h>
#include <net-snmp/library/tools.h>
#include <net-snmp/library/system.h>      /* strcasecmp() */
#include <net-snmp/library/snmp_assert.h>

netsnmp_feature_child_of(snmp_enum_all, libnetsnmp);

netsnmp_feature_child_of(se_find_free_value_in_slist, snmp_enum_all);
netsnmp_feature_child_of(snmp_enum_store_list, snmp_enum_all);
netsnmp_feature_child_of(snmp_enum_store_slist, snmp_enum_all);
netsnmp_feature_child_of(snmp_enum_clear, snmp_enum_all);

struct snmp_enum_list_str {
    char           *name;
    struct snmp_enum_list *list;
    struct snmp_enum_list_str *next;
};

static struct snmp_enum_list **snmp_enum_lists;
static unsigned int current_maj_num;
static unsigned int current_min_num;
static struct snmp_enum_list_str *sliststorage;

static void
free_enum_list(struct snmp_enum_list *list);

int
init_snmp_enum(const char *type)
{
    if (NULL != snmp_enum_lists)
        return SE_OK;

    snmp_enum_lists = calloc(SE_MAX_IDS * SE_MAX_SUBIDS,
                             sizeof(*snmp_enum_lists));
    if (!snmp_enum_lists)
        return SE_NOMEM;
    current_maj_num = SE_MAX_IDS;
    current_min_num = SE_MAX_SUBIDS;

    register_const_config_handler(type, "enum", se_read_conf, NULL, NULL);
    return SE_OK;
}

void
se_read_conf(const char *word, const char *cptr)
{
    int major, minor;
    int value;
    const char *cp, *cp2;
    char e_name[BUFSIZ];
    char e_enum[  BUFSIZ];

    if (!cptr || *cptr=='\0')
        return;

    /*
     * Extract the first token
     *   (which should be the name of the list)
     */
    cp = copy_nword_const(cptr, e_name, sizeof(e_name));
    cp = skip_white_const(cp);
    if (!cp || *cp=='\0')
        return;


    /*
     * Add each remaining enumeration to the list,
     *   using the appropriate style interface
     */
    if (sscanf(e_name, "%d:%d", &major, &minor) == 2) {
        /*
         *  Numeric major/minor style
         */
        while (1) {
            cp = copy_nword_const(cp, e_enum, sizeof(e_enum));
            if (sscanf(e_enum, "%d:", &value) != 1) {
                break;
            }
            cp2 = e_enum;
            while (*cp2 != 0 && *cp2++ != ':')
                ;
            se_add_pair(major, minor, strdup(cp2), value);
            if (!cp)
                break;
        }
    } else {
        /*
         *  Named enumeration
         */
        while (1) {
            cp = copy_nword_const(cp, e_enum, sizeof(e_enum));
            if (sscanf(e_enum, "%d:", &value) != 1) {
                break;
            }
            cp2 = e_enum;
            while (*cp2 != 0 && *cp2++ != ':')
                ;
            se_add_pair_to_slist(e_name, strdup(cp2), value);
            if (!cp)
                break;
        }
    }
}

void
se_store_enum_list(struct snmp_enum_list *new_list,
                   const char *token, const char *type)
{
    struct snmp_enum_list *listp = new_list;
    char line[2048];
    char buf[512];
    int  len;

    snprintf(line, sizeof(line), "enum %s", token);
    while (listp) {
        snprintf(buf, sizeof(buf), " %d:%s", listp->value, listp->label);
        /*
         * Calculate the space left in the buffer.
         * If this is not sufficient to include the next enum,
         *   then save the line so far, and start again.
         */
	len = sizeof(line) - strlen(line);
	if ((int)strlen(buf) > len) {
	    read_config_store(type, line);
            snprintf(line, sizeof(line), "enum %s", token);
	    len = sizeof(line) - strlen(line);
	}

	strncat(line, buf, len);
        listp = listp->next;
    }

    read_config_store(type, line);
}

#ifndef NETSNMP_FEATURE_REMOVE_SNMP_ENUM_STORE_LIST
void
se_store_list(unsigned int major, unsigned int minor, const char *type)
{
    char token[32];

    snprintf(token, sizeof(token), "%d:%d", major, minor);
    se_store_enum_list(se_find_list(major, minor), token, type);
}
#endif /* NETSNMP_FEATURE_REMOVE_SNMP_ENUM_STORE_LIST */

static struct snmp_enum_list **
se_find_list_ptr(unsigned int major, unsigned int minor)
{
    if (major >= current_maj_num || minor >= current_min_num)
        return NULL;
    netsnmp_assert(NULL != snmp_enum_lists);

    return &snmp_enum_lists[major * current_min_num + minor];
}

struct snmp_enum_list *
se_find_list(unsigned int major, unsigned int minor)
{
    struct snmp_enum_list **p = se_find_list_ptr(major, minor);

    return p ? *p : NULL;
}

int
se_find_value_in_list(struct snmp_enum_list *list, const char *label)
{
    if (!list)
        return SE_DNE;          /* XXX: um, no good solution here */
    while (list) {
        if (strcmp(list->label, label) == 0)
            return (list->value);
        list = list->next;
    }

    return SE_DNE;              /* XXX: um, no good solution here */
}

int
se_find_casevalue_in_list(struct snmp_enum_list *list, const char *label)
{
    if (!list)
        return SE_DNE;          /* XXX: um, no good solution here */
    while (list) {
        if (strcasecmp(list->label, label) == 0)
            return (list->value);
        list = list->next;
    }

    return SE_DNE;              /* XXX: um, no good solution here */
}

int
se_find_free_value_in_list(struct snmp_enum_list *list)
{
    int max_value = 0;
    if (!list)
        return SE_DNE;

    for (;list; list=list->next) {
        if (max_value < list->value)
            max_value = list->value;
    }
    return max_value+1;
}

int
se_find_value(unsigned int major, unsigned int minor, const char *label)
{
    return se_find_value_in_list(se_find_list(major, minor), label);
}

int
se_find_free_value(unsigned int major, unsigned int minor)
{
    return se_find_free_value_in_list(se_find_list(major, minor));
}

char           *
se_find_label_in_list(struct snmp_enum_list *list, int value)
{
    if (!list)
        return NULL;
    while (list) {
        if (list->value == value)
            return (list->label);
        list = list->next;
    }
    return NULL;
}

char           *
se_find_label(unsigned int major, unsigned int minor, int value)
{
    return se_find_label_in_list(se_find_list(major, minor), value);
}

/*
 * Ownership of 'label' is transferred from the caller to this function.
 * 'label' is freed if list insertion fails.
 */
int
se_add_pair_to_list(struct snmp_enum_list **list, char *label, int value)
{
    struct snmp_enum_list *lastnode = NULL, *new_node, *tmp;

    if (!list) {
        free(label);
        return SE_DNE;
    }

    tmp = *list;
    while (tmp) {
        if (tmp->value == value) {
            free(label);
            return (SE_ALREADY_THERE);
        }
        lastnode = tmp;
        tmp = tmp->next;
    }

    new_node = SNMP_MALLOC_STRUCT(snmp_enum_list);
    if (!new_node) {
        free(label);
        return (SE_NOMEM);
    }

    if (lastnode)
        lastnode->next = new_node;
    else
        *list = new_node;
    new_node->label = label;
    new_node->value = value;
    new_node->next = NULL;
    return (SE_OK);
}

int
se_add_pair(unsigned int major, unsigned int minor, char *label, int value)
{
    return se_add_pair_to_list(se_find_list_ptr(major, minor), label, value);
}

/*
 * remember a list of enums based on a lookup name.
 */
static struct snmp_enum_list **
se_find_slist_ptr(const char *listname)
{
    struct snmp_enum_list_str *sptr;
    if (!listname)
        return NULL;

    for (sptr = sliststorage; sptr != NULL; sptr = sptr->next)
        if (sptr->name && strcmp(sptr->name, listname) == 0)
            return &sptr->list;

    return NULL;
}

struct snmp_enum_list *
se_find_slist(const char *listname)
{
    struct snmp_enum_list **ptr = se_find_slist_ptr(listname);
    return ptr ? *ptr : NULL;
}

char           *
se_find_label_in_slist(const char *listname, int value)
{
    return (se_find_label_in_list(se_find_slist(listname), value));
}

int
se_find_value_in_slist(const char *listname, const char *label)
{
    return (se_find_value_in_list(se_find_slist(listname), label));
}

int
se_find_casevalue_in_slist(const char *listname, const char *label)
{
    return (se_find_casevalue_in_list(se_find_slist(listname), label));
}

#ifndef NETSNMP_FEATURE_REMOVE_SE_FIND_FREE_VALUE_IN_SLIST
int
se_find_free_value_in_slist(const char *listname)
{
    return (se_find_free_value_in_list(se_find_slist(listname)));
}
#endif /* NETSNMP_FEATURE_REMOVE_SE_FIND_FREE_VALUE_IN_SLIST */

/*
 * Ownership of 'label' is transferred from the caller to this function.
 * 'label' is freed if list insertion fails.
 */
int
se_add_pair_to_slist(const char *listname, char *label, int value)
{
    struct snmp_enum_list **list_p = se_find_slist_ptr(listname);

    if (!list_p) {
        struct snmp_enum_list_str *sptr =
            SNMP_MALLOC_STRUCT(snmp_enum_list_str);
        if (!sptr) {
            free(label);
            return SE_NOMEM;
        }
        sptr->next = sliststorage;
        sptr->name = strdup(listname);
        if (!sptr->name) {
            free(sptr);
            free(label);
            return SE_NOMEM;
        }
        list_p = &sptr->list;
        sliststorage = sptr;
    }

    return se_add_pair_to_list(list_p, label, value);
}

static void
free_enum_list(struct snmp_enum_list *list)
{
    struct snmp_enum_list *next;

    while (list) {
        next = list->next;
        SNMP_FREE(list->label);
        SNMP_FREE(list);
        list = next;
    }
}

void
clear_snmp_enum(void)
{
    struct snmp_enum_list_str *sptr = sliststorage, *next = NULL;
    unsigned int major, minor;

    while (sptr != NULL) {
	next = sptr->next;
	free_enum_list(sptr->list);
	SNMP_FREE(sptr->name);
	SNMP_FREE(sptr);
	sptr = next;
    }
    sliststorage = NULL;

    for (major = 0; major < current_maj_num; major++) {
        for (minor = 0; minor < current_min_num; minor++) {
            struct snmp_enum_list **list_ptr = se_find_list_ptr(major, minor);

            if (!list_ptr || !*list_ptr)
                continue;
            free_enum_list(*list_ptr);
        }
    }
    current_maj_num = 0;
    current_min_num = 0;
    SNMP_FREE(snmp_enum_lists);
}

void
se_clear_list(struct snmp_enum_list **list)
{
    struct snmp_enum_list *this_entry, *next_entry;

    if (!list)
        return;

    this_entry = *list;
    while (this_entry) {
        next_entry = this_entry->next;
        SNMP_FREE(this_entry->label);
        SNMP_FREE(this_entry);
        this_entry = next_entry;
    }
    *list = NULL;
    return;
}

#ifndef NETSNMP_FEATURE_REMOVE_SNMP_ENUM_STORE_SLIST
void
se_store_slist(const char *listname, const char *type)
{
    struct snmp_enum_list *list = se_find_slist(listname);
    se_store_enum_list(list, listname, type);
}

int
se_store_slist_callback(int majorID, int minorID,
                        void *serverargs, void *clientargs)
{
    char *appname = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
                                          NETSNMP_DS_LIB_APPTYPE);
    se_store_slist((char *)clientargs, appname);
    return SNMPERR_SUCCESS;
}
#endif /* NETSNMP_FEATURE_REMOVE_SNMP_ENUM_STORE_SLIST */

#ifndef NETSNMP_FEATURE_REMOVE_SNMP_ENUM_CLEAR
void
se_clear_slist(const char *listname)
{
    se_clear_list(se_find_slist_ptr(listname));
}

void
se_clear_all_lists(void)
{
    struct snmp_enum_list_str *sptr = NULL;

    for (sptr = sliststorage; sptr != NULL; sptr = sptr->next)
        se_clear_list(&(sptr->list));
}
#endif /* NETSNMP_FEATURE_REMOVE_SNMP_ENUM_CLEAR */
