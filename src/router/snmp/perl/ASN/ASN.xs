/* -*- C -*- */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/library/asn1.h>
#include <net-snmp/library/snmp_impl.h>

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant_ASN_O(char *name, int len, int arg)
{
    switch (name[5 + 0]) {
    case 'B':
	if (strEQ(name + 5, "BJECT_ID")) {	/* ASN_O removed */
#ifdef ASN_OBJECT_ID
	    return ASN_OBJECT_ID;
#else
	    goto not_there;
#endif
	}
    case 'C':
	if (strEQ(name + 5, "CTET_STR")) {	/* ASN_O removed */
#ifdef ASN_OCTET_STR
	    return ASN_OCTET_STR;
#else
	    goto not_there;
#endif
	}
    case 'P':
	if (strEQ(name + 5, "PAQUE")) {	/* ASN_O removed */
#ifdef ASN_OPAQUE
	    return ASN_OPAQUE;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_ASN_B(char *name, int len, int arg)
{
    switch (name[5 + 0]) {
    case 'I':
	if (strEQ(name + 5, "IT_STR")) {	/* ASN_B removed */
#ifdef ASN_BIT_STR
	    return ASN_BIT_STR;
#else
	    goto not_there;
#endif
	}
    case 'O':
	if (strEQ(name + 5, "OOLEAN")) {	/* ASN_B removed */
#ifdef ASN_BOOLEAN
	    return ASN_BOOLEAN;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_ASN_S(char *name, int len, int arg)
{
    if (5 + 1 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[5 + 1]) {
    case 'Q':
	if (strEQ(name + 5, "EQUENCE")) {	/* ASN_S removed */
#ifdef ASN_SEQUENCE
	    return ASN_SEQUENCE;
#else
	    goto not_there;
#endif
	}
    case 'T':
	if (strEQ(name + 5, "ET")) {	/* ASN_S removed */
#ifdef ASN_SET
	    return ASN_SET;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_ASN_C(char *name, int len, int arg)
{
    if (5 + 6 > len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[5 + 6]) {
    case '\0':
	if (strEQ(name + 5, "OUNTER")) {	/* ASN_C removed */
#ifdef ASN_COUNTER
	    return ASN_COUNTER;
#else
	    goto not_there;
#endif
	}
    case '6':
	if (strEQ(name + 5, "OUNTER64")) {	/* ASN_C removed */
#ifdef ASN_COUNTER64
	    return ASN_COUNTER64;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_ASN_U(char *name, int len, int arg)
{
    if (5 + 7 > len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[5 + 7]) {
    case '\0':
	if (strEQ(name + 5, "NSIGNED")) {	/* ASN_U removed */
#ifdef ASN_UNSIGNED
	    return ASN_UNSIGNED;
#else
	    goto not_there;
#endif
	}
    case '6':
	if (strEQ(name + 5, "NSIGNED64")) {	/* ASN_U removed */
#ifdef ASN_UNSIGNED64
	    return ASN_UNSIGNED64;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_ASN_IN(char *name, int len, int arg)
{
    if (6 + 5 > len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[6 + 5]) {
    case '\0':
	if (strEQ(name + 6, "TEGER")) {	/* ASN_IN removed */
#ifdef ASN_INTEGER
	    return ASN_INTEGER;
#else
	    goto not_there;
#endif
	}
    case '6':
	if (strEQ(name + 6, "TEGER64")) {	/* ASN_IN removed */
#ifdef ASN_INTEGER64
	    return ASN_INTEGER64;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_ASN_I(char *name, int len, int arg)
{
    switch (name[5 + 0]) {
    case 'N':
	return constant_ASN_IN(name, len, arg);
    case 'P':
	if (strEQ(name + 5, "PADDRESS")) {	/* ASN_I removed */
#ifdef ASN_IPADDRESS
	    return ASN_IPADDRESS;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant(char *name, int len, int arg)
{
    errno = 0;
    if (0 + 4 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[0 + 4]) {
    case 'A':
	if (strEQ(name + 0, "ASN_APPLICATION")) {	/*  removed */
#ifdef ASN_APPLICATION
	    return ASN_APPLICATION;
#else
	    goto not_there;
#endif
	}
    case 'B':
	if (!strnEQ(name + 0,"ASN_", 4))
	    break;
	return constant_ASN_B(name, len, arg);
    case 'C':
	if (!strnEQ(name + 0,"ASN_", 4))
	    break;
	return constant_ASN_C(name, len, arg);
    case 'D':
	if (strEQ(name + 0, "ASN_DOUBLE")) {	/*  removed */
#ifdef ASN_DOUBLE
	    return ASN_DOUBLE;
#else
	    goto not_there;
#endif
	}
    case 'F':
	if (strEQ(name + 0, "ASN_FLOAT")) {	/*  removed */
#ifdef ASN_FLOAT
	    return ASN_FLOAT;
#else
	    goto not_there;
#endif
	}
    case 'G':
	if (strEQ(name + 0, "ASN_GAUGE")) {	/*  removed */
#ifdef ASN_GAUGE
	    return ASN_GAUGE;
#else
	    goto not_there;
#endif
	}
    case 'I':
	if (!strnEQ(name + 0,"ASN_", 4))
	    break;
	return constant_ASN_I(name, len, arg);
    case 'N':
	if (strEQ(name + 0, "ASN_NULL")) {	/*  removed */
#ifdef ASN_NULL
	    return ASN_NULL;
#else
	    goto not_there;
#endif
	}
    case 'O':
	if (!strnEQ(name + 0,"ASN_", 4))
	    break;
	return constant_ASN_O(name, len, arg);
    case 'S':
	if (!strnEQ(name + 0,"ASN_", 4))
	    break;
	return constant_ASN_S(name, len, arg);
    case 'T':
	if (strEQ(name + 0, "ASN_TIMETICKS")) {	/*  removed */
#ifdef ASN_TIMETICKS
	    return ASN_TIMETICKS;
#else
	    goto not_there;
#endif
	}
    case 'U':
	if (!strnEQ(name + 0,"ASN_", 4))
	    break;
	return constant_ASN_U(name, len, arg);
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}


MODULE = NetSNMP::ASN		PACKAGE = NetSNMP::ASN		


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

