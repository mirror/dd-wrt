#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <net-snmp/agent/ds_agent.h>

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant_NETSNMP_DS_AGENT_P(char *name, int len, int arg)
{
    switch (name[18 + 0]) {
    case 'E':
	if (strEQ(name + 18, "ERL_INIT_FILE")) {	/* NETSNMP_DS_AGENT_P removed */
#ifdef NETSNMP_DS_AGENT_PERL_INIT_FILE
	    return NETSNMP_DS_AGENT_PERL_INIT_FILE;
#else
	    goto not_there;
#endif
	}
    case 'O':
	if (strEQ(name + 18, "ORTS")) {	/* NETSNMP_DS_AGENT_P removed */
#ifdef NETSNMP_DS_AGENT_PORTS
	    return NETSNMP_DS_AGENT_PORTS;
#else
	    goto not_there;
#endif
	}
    case 'R':
	if (strEQ(name + 18, "ROGNAME")) {	/* NETSNMP_DS_AGENT_P removed */
#ifdef NETSNMP_DS_AGENT_PROGNAME
	    return NETSNMP_DS_AGENT_PROGNAME;
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
constant_NETSNMP_DS_AGENT_A(char *name, int len, int arg)
{
    if (18 + 6 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[18 + 6]) {
    case 'M':
	if (strEQ(name + 18, "GENTX_MASTER")) {	/* NETSNMP_DS_AGENT_A removed */
#ifdef NETSNMP_DS_AGENT_AGENTX_MASTER
	    return NETSNMP_DS_AGENT_AGENTX_MASTER;
#else
	    goto not_there;
#endif
	}
    case 'P':
	if (strEQ(name + 18, "GENTX_PING_INTERVAL")) {	/* NETSNMP_DS_AGENT_A removed */
#ifdef NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL
	    return NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL;
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
    if (0 + 17 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[0 + 17]) {
    case 'A':
	if (!strnEQ(name + 0,"NETSNMP_DS_AGENT_", 17))
	    break;
	return constant_NETSNMP_DS_AGENT_A(name, len, arg);
    case 'D':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_DISABLE_PERL")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_DISABLE_PERL
	    return NETSNMP_DS_AGENT_DISABLE_PERL;
#else
	    goto not_there;
#endif
	}
    case 'F':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_FLAGS")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_FLAGS
	    return NETSNMP_DS_AGENT_FLAGS;
#else
	    goto not_there;
#endif
	}
    case 'G':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_GROUPID")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_GROUPID
	    return NETSNMP_DS_AGENT_GROUPID;
#else
	    goto not_there;
#endif
	}
    case 'H':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_H")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_H
	    return NETSNMP_DS_AGENT_H;
#else
	    goto not_there;
#endif
	}
    case 'I':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_INTERNAL_SECNAME")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_INTERNAL_SECNAME
	    return NETSNMP_DS_AGENT_INTERNAL_SECNAME;
#else
	    goto not_there;
#endif
	}
    case 'N':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_NO_ROOT_ACCESS")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_NO_ROOT_ACCESS
	    return NETSNMP_DS_AGENT_NO_ROOT_ACCESS;
#else
	    goto not_there;
#endif
	}
    case 'P':
	if (!strnEQ(name + 0,"NETSNMP_DS_AGENT_", 17))
	    break;
	return constant_NETSNMP_DS_AGENT_P(name, len, arg);
    case 'Q':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_QUIT_IMMEDIATELY")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_QUIT_IMMEDIATELY
	    return NETSNMP_DS_AGENT_QUIT_IMMEDIATELY;
#else
	    goto not_there;
#endif
	}
    case 'R':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_ROLE")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_ROLE
	    return NETSNMP_DS_AGENT_ROLE;
#else
	    goto not_there;
#endif
	}
    case 'U':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_USERID")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_USERID
	    return NETSNMP_DS_AGENT_USERID;
#else
	    goto not_there;
#endif
	}
    case 'V':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_VERBOSE")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_VERBOSE
	    return NETSNMP_DS_AGENT_VERBOSE;
#else
	    goto not_there;
#endif
	}
    case 'X':
	if (strEQ(name + 0, "NETSNMP_DS_AGENT_X_SOCKET")) {	/*  removed */
#ifdef NETSNMP_DS_AGENT_X_SOCKET
	    return NETSNMP_DS_AGENT_X_SOCKET;
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


MODULE = NetSNMP::agent::default_store		PACKAGE = NetSNMP::agent::default_store		


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

