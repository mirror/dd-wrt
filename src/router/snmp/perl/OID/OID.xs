/* -*- C -*- */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/* pulled from Dave's, yet-to-be-used, net-snmp library rewrite.
   autocompatibility for the future? */

#define NETSNMP_NAMEBUF_LEN 128
typedef struct netsnmp_oid_s {
    oid                 *name;
    unsigned int         len;
    oid                  namebuf[ NETSNMP_NAMEBUF_LEN ];
} netsnmp_oid;

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(char *name, int len, int arg)
{
    errno = EINVAL;
    return 0;
}

netsnmp_oid *
nso_newarrayptr(oid *name, size_t name_len) 
{
    netsnmp_oid *RETVAL;
    RETVAL = SNMP_MALLOC_TYPEDEF(netsnmp_oid);
    RETVAL->name = RETVAL->namebuf;
    RETVAL->len = name_len;
    memcpy(RETVAL->name, name, name_len * sizeof(oid));
    return RETVAL;
}

MODULE = NetSNMP::OID		PACKAGE = NetSNMP::OID		PREFIX=nso_

netsnmp_oid *
nso_newptr(initstring)
    char *initstring
    CODE:
        RETVAL = SNMP_MALLOC_TYPEDEF(netsnmp_oid);
        RETVAL->name = RETVAL->namebuf;
        RETVAL->len = sizeof(RETVAL->namebuf)/sizeof(RETVAL->namebuf[0]);
        if (!snmp_parse_oid(initstring, (oid *) RETVAL->name, &RETVAL->len)) {
            fprintf(stderr, "Can't parse: %s\n", initstring);
            RETVAL->len = 0;
            RETVAL = NULL;
        }
    OUTPUT:
        RETVAL

double
constant(sv,arg)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
	int		arg
    CODE:
	RETVAL = constant(s,len,arg);
    OUTPUT:
	RETVAL

int
_snmp_oid_compare(oid1, oid2)
    netsnmp_oid *oid1;
    netsnmp_oid *oid2;
    CODE:
        RETVAL = snmp_oid_compare((oid *) oid1->name, oid1->len,
                                  (oid *) oid2->name, oid2->len);
    OUTPUT:
        RETVAL

MODULE = NetSNMP::OID  PACKAGE = netsnmp_oidPtr  PREFIX = nsop_

void
nsop_DESTROY(oid1)
	netsnmp_oid *oid1
    CODE:
	free(oid1);
        
char *
nsop_to_string(oid1)
    	netsnmp_oid *oid1
    PREINIT:
        static char mystr[SNMP_MAXBUF];
    CODE:
        {
            if (oid1->len == 0)
                snprintf(mystr, sizeof(mystr), "Illegal OID");
            else
                snprint_objid(mystr, sizeof(mystr),
                              (oid *) oid1->name, oid1->len);
            RETVAL = mystr;
        }
                
    OUTPUT:
        RETVAL

void
nsop_to_array(oid1)
    netsnmp_oid *oid1;
    PREINIT:
        int i;

    PPCODE:
        EXTEND(SP, oid1->len);
        for(i=0; i < oid1->len; i++) {
            PUSHs(sv_2mortal(newSVnv(oid1->name[i])));
        }

void
nsop_append(oid1, string)
    netsnmp_oid *oid1;
    char *string;
    PREINIT:
    oid name[128];
    size_t name_len = 128;
    int i;
    CODE: 
    {
        if (!snmp_parse_oid(string, (oid *) name, &name_len)) {
            /* XXX */
        }
        if (oid1->len + name_len > 128) {
            /* XXX: illegal */
        }
        for(i = 0; i < name_len; i++) {
            oid1->name[i+oid1->len] = name[i];
        }
        oid1->len += name_len;
    }

void
nsop_append_oid(oid1, oid2)
    netsnmp_oid *oid1;
    netsnmp_oid *oid2;
    PREINIT:
    int i;
    CODE: 
    {
        if (oid1->len + oid2->len > 128) {
            /* XXX: illegal */
        }
        for(i = 0; i < oid2->len; i++) {
            oid1->name[i+oid1->len] = oid2->name[i];
        }
        oid1->len += oid2->len;
    }

int
nsop_length(oid1)
    netsnmp_oid *oid1;
    CODE: 
    {
        RETVAL = oid1->len;
    }
    OUTPUT:
    RETVAL
    
netsnmp_oid *
nsop_clone(oid1)
    netsnmp_oid *oid1;
    PREINIT:
    netsnmp_oid *oid2;
    CODE:
    {
        oid2 = nso_newarrayptr(oid1->name, oid1->len);
        RETVAL = oid2;
    }
OUTPUT:
    RETVAL
        
