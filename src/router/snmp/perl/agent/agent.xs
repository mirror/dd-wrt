/* -*- C -*- */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include <netdb.h>
#include <sys/socket.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#ifndef sv_undef
#define sv_undef PL_sv_undef
#endif

typedef struct handler_cb_data_s {
   SV *perl_cb;
} handler_cb_data;

#define NETSNMP_NAMEBUF_LEN 128
typedef struct netsnmp_oid_s {
    unsigned int        *name;
    unsigned int         len;
    unsigned int         namebuf[ NETSNMP_NAMEBUF_LEN ];
} netsnmp_oid;

static int have_done_agent = 0;
static int have_done_lib = 0;

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant_MODE_G(char *name, int len, int arg)
{
    if (6 + 2 > len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[6 + 2]) {
    case '\0':
	if (strEQ(name + 6, "ET")) {	/* MODE_G removed */
#ifdef MODE_GET
	    return MODE_GET;
#else
	    goto not_there;
#endif
	}
    case 'B':
	if (strEQ(name + 6, "ETBULK")) {	/* MODE_G removed */
#ifdef MODE_GETBULK
	    return MODE_GETBULK;
#else
	    goto not_there;
#endif
	}
    case 'N':
	if (strEQ(name + 6, "ETNEXT")) {	/* MODE_G removed */
#ifdef MODE_GETNEXT
	    return MODE_GETNEXT;
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
constant_MODE_SET_R(char *name, int len, int arg)
{
    if (10 + 6 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[10 + 6]) {
    case '1':
	if (strEQ(name + 10, "ESERVE1")) {	/* MODE_SET_R removed */
#ifdef MODE_SET_RESERVE1
	    return MODE_SET_RESERVE1;
#else
	    goto not_there;
#endif
	}
    case '2':
	if (strEQ(name + 10, "ESERVE2")) {	/* MODE_SET_R removed */
#ifdef MODE_SET_RESERVE2
	    return MODE_SET_RESERVE2;
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
constant_MODE_S(char *name, int len, int arg)
{
    if (6 + 3 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[6 + 3]) {
    case 'A':
	if (strEQ(name + 6, "ET_ACTION")) {	/* MODE_S removed */
#ifdef MODE_SET_ACTION
	    return MODE_SET_ACTION;
#else
	    goto not_there;
#endif
	}
    case 'B':
	if (strEQ(name + 6, "ET_BEGIN")) {	/* MODE_S removed */
#ifdef MODE_SET_BEGIN
	    return MODE_SET_BEGIN;
#else
	    goto not_there;
#endif
	}
    case 'C':
	if (strEQ(name + 6, "ET_COMMIT")) {	/* MODE_S removed */
#ifdef MODE_SET_COMMIT
	    return MODE_SET_COMMIT;
#else
	    goto not_there;
#endif
	}
    case 'F':
	if (strEQ(name + 6, "ET_FREE")) {	/* MODE_S removed */
#ifdef MODE_SET_FREE
	    return MODE_SET_FREE;
#else
	    goto not_there;
#endif
	}
    case 'R':
	if (!strnEQ(name + 6,"ET_", 3))
	    break;
	return constant_MODE_SET_R(name, len, arg);
    case 'U':
	if (strEQ(name + 6, "ET_UNDO")) {	/* MODE_S removed */
#ifdef MODE_SET_UNDO
	    return MODE_SET_UNDO;
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
    if (0 + 5 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[0 + 5]) {
    case 'G':
	if (!strnEQ(name + 0,"MODE_", 5))
	    break;
	return constant_MODE_G(name, len, arg);
    case 'S':
	if (!strnEQ(name + 0,"MODE_", 5))
	    break;
	return constant_MODE_S(name, len, arg);
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

int
handler_wrapper(netsnmp_mib_handler          *handler,
                netsnmp_handler_registration *reginfo,
                netsnmp_agent_request_info   *reqinfo,
                netsnmp_request_info         *requests) 
{
    u_long intret = 5;
    handler_cb_data *cb_data = (handler_cb_data *) handler->myvoid;
    SV *cb;

    if (cb_data && (cb = cb_data->perl_cb)) {
        SV *arg;
        SV *rarg;
        dSP;
        ENTER;
        SAVETMPS;
        PUSHMARK(sp);
        rarg = newSViv(0);
        arg = newSVrv(rarg, "NetSNMP::agent::netsnmp_mib_handler");
        sv_setiv(arg, (int) handler);
        XPUSHs(rarg);
        rarg = newSViv(0);
        arg = newSVrv(rarg, "NetSNMP::agent::reginfo");
        sv_setiv(arg, (int) reginfo);
        XPUSHs(rarg);
        rarg = newSViv(0);
        arg = newSVrv(rarg, "NetSNMP::agent::netsnmp_agent_request_info");
        sv_setiv(arg, (int) reqinfo);
        XPUSHs(rarg);
        rarg = newSViv(0);
        arg = newSVrv(rarg, "NetSNMP::agent::netsnmp_request_infoPtr");
        sv_setiv(arg, (int) requests);
        XPUSHs(rarg);
        PUTBACK;
        if (SvTYPE(cb) == SVt_PVCV) {
            perl_call_sv(cb, G_DISCARD); /* I have no idea what discard does */
                                         /* XXX: it discards the results,
                                            which isn't right */
        } else if (SvROK(cb) && SvTYPE(SvRV(cb)) == SVt_PVCV) {
            /* reference to code */
            perl_call_sv(SvRV(cb), G_DISCARD);
        }
        SPAGAIN;
        PUTBACK;
        FREETMPS;
        LEAVE;
    }
    return SNMP_ERR_NOERROR;
}

MODULE = NetSNMP::agent		PACKAGE = NetSNMP::agent		

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
__agent_check_and_process(block = 1)
	int block;
    CODE:
	RETVAL = agent_check_and_process(block);
    OUTPUT:
	RETVAL

void
init_mib()

int
init_agent(name)
        const char *name;

void
init_snmp(name)
        const char *name;

int
init_master_agent()

void    
snmp_enable_stderrlog()    

MODULE = NetSNMP::agent  PACKAGE = NetSNMP::agent PREFIX = na_

void
na_shutdown(me)
    SV *me;
    CODE:
    {
        snmp_shutdown("perl");
    }

MODULE = NetSNMP::agent  PACKAGE = NetSNMP::agent::netsnmp_handler_registration  PREFIX = nsahr_

netsnmp_handler_registration *
nsahr_new(name, regoid, perlcallback)
        char *name;
	char *regoid;
        SV   *perlcallback;
    PREINIT:
	oid myoid[MAX_OID_LEN];
	size_t myoid_len = MAX_OID_LEN;
        handler_cb_data *cb_data;
        int gotit=1;
    CODE:
	if (!snmp_parse_oid(regoid, myoid, &myoid_len)) {
            if (!read_objid(regoid, myoid, &myoid_len)) {
                fprintf(stderr, "couldn't parse %s (reg name: %s)\n",
                        regoid, name);
                RETVAL = NULL;
                gotit = 0;
            }
        }
        if (gotit) {
            cb_data = (handler_cb_data *) malloc(sizeof(handler_cb_data));
            RETVAL = netsnmp_create_handler_registration(name, handler_wrapper,
                                                 myoid, myoid_len,
                                                 HANDLER_CAN_RWRITE);
            cb_data->perl_cb = newSVsv(perlcallback);
            RETVAL->handler->myvoid = cb_data;
        }
    OUTPUT:
        RETVAL

MODULE = NetSNMP::agent  PACKAGE = netsnmp_handler_registrationPtr  PREFIX = nsahr_

void
nsahr_DESTROY(reginfo)
	netsnmp_handler_registration *reginfo
    CODE:
	netsnmp_handler_registration_free(reginfo);

int
nsahr_register(me)
        SV *me;
        PREINIT:
        netsnmp_handler_registration *reginfo;
        CODE:
            {
                reginfo = (netsnmp_handler_registration *) SvIV(SvRV(me));
                RETVAL = netsnmp_register_handler(reginfo);
                if (!RETVAL) {
                    /* the agent now has a "reference" to this reg pointer */
                    SvREFCNT_inc(me);
                }
            }
    OUTPUT:
	RETVAL

void
nsahr_getRootOID(me)
    SV *me;
    PREINIT:
        int i;
        netsnmp_oid *o;
        netsnmp_handler_registration *reginfo;
        SV *optr, *arg, *rarg;
    PPCODE:
    {
        dSP;
        PUSHMARK(SP);
        reginfo = (netsnmp_handler_registration *) SvIV(SvRV(me));

        o = SNMP_MALLOC_TYPEDEF(netsnmp_oid);
        o->name = o->namebuf;
        o->len = reginfo->rootoid_len;
        memcpy(o->name, reginfo->rootoid,
               reginfo->rootoid_len * sizeof(oid));

        rarg = newSViv((int) 0);
        arg = newSVrv(rarg, "netsnmp_oidPtr");
        sv_setiv(arg, (int) o);

        XPUSHs(rarg);

        PUTBACK;
        i = perl_call_pv("NetSNMP::OID::newwithptr", G_SCALAR);
        SPAGAIN;
        if (i != 1) {
            fprintf(stderr, "unhandled OID error.\n");
            /* ack XXX */
        }
        ST(0) = POPs;
        XSRETURN(1);
    }

MODULE = NetSNMP::agent  PACKAGE = NetSNMP::agent::netsnmp_request_infoPtr PREFIX = nari_

void
getOID(me)
    SV *me;
    PREINIT:
        int i;
        netsnmp_oid *o;
        netsnmp_request_info *request;
        SV *optr, *arg, *rarg;
    PPCODE:
    {
        dSP;
        PUSHMARK(SP);
        request = (netsnmp_request_info *) SvIV(SvRV(me));

        o = SNMP_MALLOC_TYPEDEF(netsnmp_oid);
        o->name = o->namebuf;
        o->len = request->requestvb->name_length;
        memcpy(o->name, request->requestvb->name,
               request->requestvb->name_length * sizeof(oid));

        rarg = newSViv((int) 0);
        arg = newSVrv(rarg, "netsnmp_oidPtr");
        sv_setiv(arg, (int) o);

        XPUSHs(rarg);

        PUTBACK;
        i = perl_call_pv("NetSNMP::OID::newwithptr", G_SCALAR);
        SPAGAIN;
        if (i != 1) {
            fprintf(stderr, "unhandled OID error.\n");
            /* ack XXX */
        }
        ST(0) = POPs;
        XSRETURN(1);
    }
        
netsnmp_oid *
nari_getOIDptr(me)
        SV *me;
        PREINIT:
        netsnmp_request_info *request;
        CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        RETVAL = SNMP_MALLOC_TYPEDEF(netsnmp_oid);
        RETVAL->name = RETVAL->namebuf;
        RETVAL->len = request->requestvb->name_length;
        memcpy(RETVAL->name, request->requestvb->name,
               request->requestvb->name_length * sizeof(oid));
    OUTPUT:
        RETVAL

char *
nari_getValue(me)
        SV *me;
    PREINIT:
        u_char *oidbuf = NULL;
        size_t ob_len = 0, oo_len = 0;
        netsnmp_request_info *request;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
	sprint_realloc_by_type(&oidbuf, &ob_len, &oo_len, 1,
                               request->requestvb, 0, 0, 0);
        RETVAL = oidbuf; /* mem leak */
    OUTPUT:
        RETVAL

int
nari_getDelegated(me)
        SV *me;
    PREINIT:
        netsnmp_request_info *request;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        RETVAL = request->delegated;
    OUTPUT:
        RETVAL

void
nari_setDelegated(me, newdelegated)
        SV *me;
        int newdelegated;
    PREINIT:
        netsnmp_request_info *request;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        request->delegated = newdelegated;

int
nari_getProcessed(me)
        SV *me;
    PREINIT:
        netsnmp_request_info *request;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        RETVAL = request->processed;
    OUTPUT:
        RETVAL

void
nari_setProcessed(me, newprocessed)
        SV *me;
        int newprocessed;
    PREINIT:
        netsnmp_request_info *request;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        request->processed = newprocessed;

int
nari_getStatus(me)
        SV *me;
    PREINIT:
        netsnmp_request_info *request;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        RETVAL = request->status;
    OUTPUT:
        RETVAL

void
nari_setStatus(me, newstatus)
        SV *me;
        int newstatus;
    PREINIT:
        netsnmp_request_info *request;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        request->status = newstatus;

int
nari_getRepeat(me)
        SV *me;
    PREINIT:
        netsnmp_request_info *request;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        RETVAL = request->repeat;
    OUTPUT:
        RETVAL

void
nari_setRepeat(me, newrepeat)
        SV *me;
        int newrepeat;
    PREINIT:
        netsnmp_request_info *request;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        request->repeat = newrepeat;

int
nari_setValue(me, type, value)
        SV *me;
        int type;
        SV *value;
    PREINIT:
        u_char *oidbuf = NULL;
        size_t ob_len = 0, oo_len = 0;
        netsnmp_request_info *request;
        u_long utmp;
        long ltmp;
	oid myoid[MAX_OID_LEN];
	size_t myoid_len;
        STRLEN stringlen;
        char * stringptr;
    CODE:
        request = (netsnmp_request_info *) SvIV(SvRV(me));
        switch(type) {
          case ASN_INTEGER:
	      /* We want an integer here */
	      if ((SvTYPE(value) == SVt_IV) || (SvTYPE(value) == SVt_PVMG)) {
		  /* Good - got a real one (or a blessed object that we hope will turn out OK) */
		  ltmp = SvIV(value);
		  snmp_set_var_typed_value(request->requestvb, type,
					   (u_char *) &ltmp, sizeof(ltmp));
		  RETVAL = 1;
		  break;
	      }
	      else if (SvPOKp(value)) {
	          /* Might be OK - got a string, so try to convert it, allowing base 10, octal, and hex forms */
	          stringptr = SvPV(value, stringlen);
		  ltmp = strtol( stringptr, NULL, 0 );
		  if (errno == EINVAL) {
		  	fprintf(stderr, "Could not convert string to number in setValue: '%s'", stringptr);
			RETVAL = 0;
			break;
		  }

		  snmp_set_var_typed_value(request->requestvb, type,
					   (u_char *) &ltmp, sizeof(ltmp));
		  RETVAL = 1;
		  break;
	      }
	      else {
		fprintf(stderr, "Non-integer value passed to setValue with ASN_INTEGER: type was %d\n",
			SvTYPE(value));
		RETVAL = 0;
		break;
	      }


          case ASN_UNSIGNED:
          case ASN_COUNTER:
          case ASN_COUNTER64:
          case ASN_TIMETICKS:
	      /* We want an integer here */
	      if ((SvTYPE(value) == SVt_IV) || (SvTYPE(value) == SVt_PVMG)) {
		  /* Good - got a real one (or a blessed scalar which we have to hope will turn out OK) */
		  utmp = SvIV(value);
                  snmp_set_var_typed_value(request->requestvb, type,
                                       (u_char *) &utmp, sizeof(utmp));
		  RETVAL = 1;
		  break;
	      }
	      else if (SvPOKp(value)) {
	          /* Might be OK - got a string, so try to convert it, allowing base 10, octal, and hex forms */
	          stringptr = SvPV(value, stringlen);
		  utmp = strtoul( stringptr, NULL, 0 );
		  if (errno == EINVAL) {
		  	fprintf(stderr, "Could not convert string to number in setValue: '%s'", stringptr);
			RETVAL = 0;
			break;
		  }

                  snmp_set_var_typed_value(request->requestvb, type,
                                       (u_char *) &utmp, sizeof(utmp));
		  RETVAL = 1;
		  break;
	      }
	      else {
		fprintf(stderr, "Non-unsigned-integer value passed to setValue with ASN_UNSIGNED/ASN_COUNTER/ASN_TIMETICKS: type was %d\n",
			SvTYPE(value));
		RETVAL = 0;
		break;
	      }

          case ASN_OCTET_STR:
          case ASN_BIT_STR:
	      /* Check that we have been passed something with a string value (or a blessed scalar) */
	      if (!SvPOKp(value) && (SvTYPE(value) != SVt_PVMG)) {
		fprintf(stderr, "Non-string value passed to setValue with ASN_OCTET_STR/ASN_BIT_STR: type was %d\n",
			SvTYPE(value));
		RETVAL = 0;
		break;
	      }

	      /* Find length of string (strlen will *not* do, as these are binary strings) */
	      stringptr = SvPV(value, stringlen);

              snmp_set_var_typed_value(request->requestvb, type,
                                       (u_char *) stringptr,
                                       stringlen);
              RETVAL = 1;
              break;

          case ASN_IPADDRESS:
	      /* IP addresses are passed as *binary* strings.
	       * In the case of IPv4 addresses, these are 4 bytes long.
	       * NOTE: the use of binary strings rather than dotted-quad or FQDNs was
	       * introduced here by Andrew Findlay's patch of March 17th 2003,
	       * and is effectively a change to the previous implied API which assumed
	       * the value was a (valid) hostname.
	       * Responsibility for decoding and resolving has been passed up to the Perl script.
	       */

	      /* Check that we have been passed something with a string value (or a blessed scalar) */
	      if (!SvPOKp(value) && (SvTYPE(value) != SVt_PVMG)) {
		fprintf(stderr, "Non-string value passed to setValue with ASN_IPADDRESS: type was %d\n",
			SvTYPE(value));
		RETVAL = 0;
		break;
	      }

	      /* Find length of string (strlen will *not* do, as these are binary strings) */
	      stringptr = SvPV(value, stringlen);

	      # fprintf(stderr, "IP address returned with length %d: %u.%u.%u.%u\n", stringlen, stringptr[0],
	      #     stringptr[1], stringptr[2], stringptr[3] );

	      # Sanity check on address length
	      if ((stringlen != 4) && (stringlen != 16)) {
	      		fprintf(stderr, "IP address of %d bytes passed to setValue with ASN_IPADDRESS\n", stringlen);
			RETVAL = 0;
			break;
	      }

              snmp_set_var_typed_value(request->requestvb, type,
                                   stringptr, stringlen);
              RETVAL = 1;
              break;

          case ASN_OBJECT_ID:
	      /* Check that we have been passed something with a string value (or a blessed scalar) */
	      if (!SvPOKp(value) && (SvTYPE(value) != SVt_PVMG)) {
		fprintf(stderr, "Non-string value passed to setValue with ASN_OBJECT_ID: type was %d\n",
			SvTYPE(value));
		RETVAL = 0;
		break;
	      }

	      /* Extract the string */
	      stringptr = SvPV(value, stringlen);

	      /* fprintf(stderr, "setValue returning OID '%s'\n", stringptr); */

	      myoid_len = MAX_OID_LEN;
              if (!snmp_parse_oid(stringptr, myoid, &myoid_len)) {
                  fprintf(stderr, "couldn't parse %s in setValue\n", stringptr);
		  RETVAL = 0;
		  break;
              } else {
		  /* fprintf(stderr, "setValue returning OID length %d\n", myoid_len); */

                  request = (netsnmp_request_info *) SvIV(SvRV(me));
                  snmp_set_var_typed_value(request->requestvb, type,
                                           (u_char *) myoid, (myoid_len * sizeof(myoid[0])) );
              }

              RETVAL = 1;
              break;
              
            default:
                fprintf(stderr, "unknown var value type: %d\n",
                        type);
                RETVAL = 0;
                break;
        }

    OUTPUT:
        RETVAL
        
void
nari_setOID(me, value)
        SV *me;
        char *value;
    PREINIT:
	oid myoid[MAX_OID_LEN];
	size_t myoid_len = MAX_OID_LEN;
        netsnmp_request_info *request;
    CODE:
	myoid_len = MAX_OID_LEN;
	if (!snmp_parse_oid(value, myoid, &myoid_len)) {
            fprintf(stderr, "couldn't parse %s in setOID\n", value);
        } else {
            request = (netsnmp_request_info *) SvIV(SvRV(me));
            snmp_set_var_objid(request->requestvb, myoid, myoid_len);
        }


SV *
nari_next(me)
        SV *me;
    PREINIT:
        netsnmp_request_info *request;
        SV *arg, *rarg;
    CODE:
        {
            request = (netsnmp_request_info *) SvIV(SvRV(me));
            if (request && request->next) {
                request = request->next;
                rarg = newSViv(0);
                arg = newSVrv(rarg, "NetSNMP::agent::netsnmp_request_infoPtr");
                sv_setiv(arg, (int) request);
                ST(0) = rarg;
            } else {
                ST(0) = &sv_undef;
            }
        }

MODULE = NetSNMP::agent  PACKAGE = NetSNMP::agent::netsnmp_agent_request_info PREFIX = narqi_

int
narqi_getMode(me)
        SV *me;
    PREINIT:
        netsnmp_agent_request_info *reqinfo;
    CODE:
        reqinfo = (netsnmp_agent_request_info *) SvIV(SvRV(me));
        RETVAL = reqinfo->mode;
    OUTPUT:
        RETVAL

void
narqi_setMode(me, newvalue)
        SV *me;
        int newvalue;
    PREINIT:
        netsnmp_agent_request_info *reqinfo;
    CODE:
        reqinfo = (netsnmp_agent_request_info *) SvIV(SvRV(me));
        reqinfo->mode = newvalue;
        

MODULE = NetSNMP::agent		PACKAGE = NetSNMP::agent		
