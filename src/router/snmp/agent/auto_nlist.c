#include <net-snmp/net-snmp-config.h>

#ifdef NETSNMP_CAN_USE_NLIST
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#ifdef HAVE_NLIST_H
#include <nlist.h>
#endif
#if HAVE_KVM_H
#include <kvm.h>
#endif

#include <net-snmp/agent/auto_nlist.h>
#include "autonlist.h"
#include "kernel.h"

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/ds_agent.h>

struct autonlist *nlists = 0;
static void     init_nlist(struct nlist *);

long
auto_nlist_value(const char *string)
{
    struct autonlist **ptr, *it = 0;
    int             cmp;

    if (string == 0)
        return 0;

    ptr = &nlists;
    while (*ptr != 0 && it == 0) {
        cmp = strcmp((*ptr)->symbol, string);
        if (cmp == 0)
            it = *ptr;
        else if (cmp < 0) {
            ptr = &((*ptr)->left);
        } else {
            ptr = &((*ptr)->right);
        }
    }
    if (*ptr == 0) {
        *ptr = (struct autonlist *) malloc(sizeof(struct autonlist));
        it = *ptr;
        it->left = 0;
        it->right = 0;
        it->symbol = (char *) malloc(strlen(string) + 1);
        strcpy(it->symbol, string);
        /*
         * allocate an extra byte for inclusion of a preceding '_' later 
         */
        it->nl[0].n_name = (char *) malloc(strlen(string) + 2);
#if defined(aix4) || defined(aix5)
        strcpy(it->nl[0].n_name, string);
#else
        sprintf(it->nl[0].n_name, "_%s", string);
#endif
        it->nl[1].n_name = 0;
        init_nlist(it->nl);
#if !(defined(aix4) || defined(aix5))
        if (it->nl[0].n_type == 0) {
            strcpy(it->nl[0].n_name, string);
            init_nlist(it->nl);
        }
#endif
        if (it->nl[0].n_type == 0) {
            if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
					NETSNMP_DS_AGENT_NO_ROOT_ACCESS)) {
                snmp_log(LOG_ERR, "nlist err: neither %s nor _%s found.\n",
                         string, string);
	    }
            return (-1);
        } else {
            DEBUGMSGTL(("auto_nlist:auto_nlist_value", "found symbol %s at %x.\n",
                        it->symbol, it->nl[0].n_value));
            return (it->nl[0].n_value);
        }
    } else
        return (it->nl[0].n_value);
}

int
auto_nlist(const char *string, char *var, int size)
{
    long            result;
    int             ret;
    result = auto_nlist_value(string);
    if (result != -1) {
        if (var != NULL) {
            ret = klookup(result, var, size);
            if (!ret)
                snmp_log(LOG_ERR,
                         "auto_nlist failed on %s at location %lx\n",
                         string, result);
            return ret;
        } else
            return 1;
    }
    return 0;
}

static void
init_nlist(struct nlist nl[])
{
#ifdef NETSNMP_CAN_USE_NLIST
    int             ret;
#if HAVE_KVM_OPENFILES
    kvm_t          *kernel;
    char            kvm_errbuf[4096];

    if ((kernel = kvm_openfiles(KERNEL_LOC, NULL, NULL, O_RDONLY, kvm_errbuf))
	== NULL) {
        if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
				   NETSNMP_DS_AGENT_NO_ROOT_ACCESS)) {
            return;
	} else {
            snmp_log_perror("kvm_openfiles");
            snmp_log(LOG_ERR, "kvm_openfiles: %s\n", kvm_errbuf);
            exit(1);
        }
    }
    if ((ret = kvm_nlist(kernel, nl)) == -1) {
        if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
				   NETSNMP_DS_AGENT_NO_ROOT_ACCESS)) {
            return;
	} else {
            snmp_log_perror("kvm_nlist");
            exit(1);
        }
    }
    kvm_close(kernel);
#else                           /* ! HAVE_KVM_OPENFILES */
#if (defined(aix4) || defined(aix5)) && defined(HAVE_KNLIST)
    if (knlist(nl, 1, sizeof(struct nlist)) == -1) {
        DEBUGMSGTL(("auto_nlist:init_nlist", "knlist failed on symbol:  %s\n",
                    nl[0].n_name));
        if (errno == EFAULT) {
            nl[0].n_type = 0;
            nl[0].n_value = 0;
        } else {
            snmp_log_perror("knlist");
            if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
				       NETSNMP_DS_AGENT_NO_ROOT_ACCESS)) {
                return;
	    } else {
                exit(1);
	    }
        }
    }
#else
    if ((ret = nlist(KERNEL_LOC, nl)) == -1) {
        if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
				   NETSNMP_DS_AGENT_NO_ROOT_ACCESS)) {
            return;
	} else {
            snmp_log_perror("nlist");
            exit(1);
        }
    }
#endif                          /*aix4 */
#endif                          /* ! HAVE_KVM_OPENFILES */
    for (ret = 0; nl[ret].n_name != NULL; ret++) {
#if defined(aix4) || defined(aix5)
        if (nl[ret].n_type == 0 && nl[ret].n_value != 0)
            nl[ret].n_type = 1;
#endif
        if (nl[ret].n_type == 0) {
            if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
					NETSNMP_DS_AGENT_NO_ROOT_ACCESS)) {
                DEBUGMSGTL(("auto_nlist:init_nlist", "nlist err:  %s not found\n",
                            nl[ret].n_name));
	    }
        } else {
            DEBUGMSGTL(("auto_nlist:init_nlist", "nlist: %s 0x%X\n", nl[ret].n_name,
                        (unsigned int) nl[ret].n_value));
        }
    }
#endif                          /* NETSNMP_CAN_USE_NLIST */
}

int
KNLookup(struct nlist nl[], int nl_which, char *buf, int s)
{
    struct nlist   *nlp = &nl[nl_which];

    if (nlp->n_value == 0) {
        snmp_log(LOG_ERR, "Accessing non-nlisted variable: %s\n",
                 nlp->n_name);
        nlp->n_value = -1;      /* only one error message ... */
        return 0;
    }
    if (nlp->n_value == -1)
        return 0;

    return klookup(nlp->n_value, buf, s);
}

#ifdef TESTING
void
auto_nlist_print_tree(int indent, struct autonlist *ptr)
{
    char            buf[1024];
    if (indent == -2) {
        snmp_log(LOG_ERR, "nlist tree:\n");
        auto_nlist_print_tree(12, nlists);
    } else {
        if (ptr == 0)
            return;
        sprintf(buf, "%%%ds\n", indent);
        /*
         * DEBUGMSGTL(("auto_nlist", "buf: %s\n",buf)); 
         */
        DEBUGMSGTL(("auto_nlist", buf, ptr->symbol));
        auto_nlist_print_tree(indent + 2, ptr->left);
        auto_nlist_print_tree(indent + 2, ptr->right);
    }
}
#endif
#else                           /* !NETSNMP_CAN_USE_NLIST */
#include <net-snmp/agent/auto_nlist.h>
int
auto_nlist_noop(void)
{
    return 0;
}
#endif                          /* NETSNMP_CAN_USE_NLIST */
