/*
 * ilmi.c - ILMI demon
 *
 * Written by Scott W. Shumate
 * 
 * Copyright (c) 1995-97 All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * I ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIM ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 */

/* Change Log:
 * 1-30-97 Added VERIFY state to allow resending of GetNext
 *         PDUs after a ColdStart.  This addresses a bug that
 *         some switches have with receiving the GetNext too
 *         soon after the ColdStart.  Also added state IDLE
 *         for a more configurable poll period.
 */

/* 
 * Reference material used: af-ilmi-0065.000
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include "asn_incl.h"
#include "rfc1155_smi.h"
#include "rfc1157_snmp.h"
#include "message.h"
#include "util.h"
#include "io.h"
#include "atmd.h"
#include "mib.h"
#include "atmf_uni.h"
#include "sysgroup.h"
#include "states.h"

#define RESPONSE_TIMEOUT 2
#define POLL_PERIOD 15
#define COMPONENT "ILMI"
#ifndef MAX
# define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif


/* Globals */
/*
static const char *state_names[STATES] = {
	"Stopped",
	"Link Failing",
	"Establishing",
	"Configuring",
	"Retrieve Network Prefixes",
	"Registering Network Prefixes",
	"Retrieving Addresses",
	"Registering Addresses",
	"Verifying"
};
*/

typedef int (*Func)(int,int,Msgs *);
static Func state_functions[STATES] = {
	state_stopped,
	state_failing,
	state_establishing,
	state_config,
	state_retrievePrefixes,
	state_registerPrefixes,
	state_retrieveAddresses,
	state_registerAddresses,
	state_verify
};

AsnOid *esi_oid = NULL;
int retries = 0;
int ilmi_errno = 0;
int alarm_flag = 0;
long int requestID = 0;
State ilmi_state = down;

static int diagfd = -1;

AttPoint _apoint;
AttPoint *apoint = &_apoint;	/* attachment point */
Config _config;
Config *config = &_config;	/* Remote IME configuration */
SysGroup _remsys;
SysGroup *remsys = &_remsys;
AttPoint _newapoint;
AttPoint *newapoint = &_newapoint;

void usage( char * );
void ilmi_loop( int, int );

void usage( char *name ){
  fprintf(stderr, "usage: %s [-b] [-d] [-v] [-l logfile] [-x] [-q qos]\n"
    "%13s[ -i local_ip ] [ -u uni ] <itf>[.<vpi>.<vci>]\n",name,"");
  fprintf(stderr, "%6s %s -V\n","",name);
  exit(1);
}

/* returns non-zero if switch set a prefix */
int handle_set_request( int fd, int itf, Msgs *msgs, Message *m ){
	VarBindList *vbl;
  VarBind *varbind;
	AsnOid *netprefix_oid, *set_oid;

	/* use varbind to compare OIDs of incoming message and that of
	   an address status message */
	varbind = (VarBind *)FIRST_LIST_ELMT(
		m->data->a.get_request->variable_bindings );

	if( AsnOidCompare( &atmfNetPrefixStatus, &varbind->name ) == AsnOidRoot ){
		vbl = msgs->set->data->a.set_request->variable_bindings;
		set_oid = &((VarBind *)FIRST_LIST_ELMT( vbl ))->name;
		netprefix_oid = accessNetPrefix();
		if( netprefix_oid != NULL ){
			diag( COMPONENT, DIAG_INFO,
				"switch registered a network prefix");
			set_oid->octetLen = ADDRESS_LEN + 1;
			AsnOidAppend( set_oid, netprefix_oid );
			AsnOidAppend( set_oid, esi_oid );
			update_nsap( itf, netprefix_oid, esi_oid );
			return 1;
		} else{
			diag( COMPONENT, DIAG_INFO,
				"switch did not register prefix?" );
		}
	}
	return 0;
	}

/* returns E value (ECOLDSTART, . . .) */
int handle_request( int fd, int itf, Msgs *msgs, Message *in_message ){
	int retval = 0;
	switch( in_message->data->choiceId ){

		  case PDUS_GET_REQUEST:
		    diag(COMPONENT, DIAG_INFO, "received get request");
		    MIBget(in_message->data->a.get_next_request->variable_bindings,
			   &in_message->data->a.get_next_request->error_status,
			   &in_message->data->a.get_next_request->error_index);
		    in_message->data->choiceId = PDUS_GET_RESPONSE;
		    send_message(fd, in_message);
		    break;
		    
		  case PDUS_GET_NEXT_REQUEST:
		    diag(COMPONENT, DIAG_INFO, "received get-next request");
		    MIBgetnext(in_message->data->a.get_next_request->variable_bindings,
			       &in_message->data->a.get_next_request->error_status,
			       &in_message->data->a.get_next_request->error_index);
		    in_message->data->choiceId = PDUS_GET_RESPONSE;
		    send_message(fd, in_message);
		    break;
		    
		  case PDUS_SET_REQUEST:
		    diag(COMPONENT, DIAG_INFO, "received set request");
		    MIBset(in_message->data->a.set_request->variable_bindings,
			   &in_message->data->a.set_request->error_status,
			   &in_message->data->a.set_request->error_index);
		    in_message->data->choiceId = PDUS_GET_RESPONSE;
		if( handle_set_request( fd, itf, msgs, in_message ))
			retval = ESETPREFIX;
		    send_message(fd, in_message);
		    break;
		    
		  case PDUS_TRAP:
		if( in_message->data->a.trap->generic_trap == COLDSTART ){
			diag( COMPONENT, DIAG_INFO, "received coldstart trap" );
			retval = ECOLDSTART;
		} else diag( COMPONENT, DIAG_INFO, "received trap" );
		    break;

		case PDUS_GET_RESPONSE:
		    break;
		  }
	return retval;
		  }

/* if a non-null value is returned from this function, the caller must
   free that memory */

Message *wait_for_response( int fd, int itf, Msgs *msgs, int sec ){
	Message *in_message;
	static Message m;
	struct timeval timeout;
	int numfds = 0;
	fd_set fdvar;

	in_message = (Message *)(&m);
	timeout.tv_sec = sec;
	timeout.tv_usec = 0;

	ilmi_errno = 0;
	wait_top:
	waitpid(0, NULL, WNOHANG);
	ResetNibbleMem();
	FD_ZERO(&fdvar);
	FD_SET(fd, &fdvar);
	FD_SET(diagfd, &fdvar);
	numfds = select(MAX(fd,diagfd)+1, &fdvar, NULL, NULL, &timeout);

	if ((numfds > 0) && FD_ISSET(diagfd, &fdvar)) {
		if (fork() == 0) {
			close(fd);
			handle_ilmidiag(diagfd, apoint, config);
			exit(0);
		}
	}

	if(( numfds > 0 ) && FD_ISSET(fd, &fdvar) &&
	   !read_message( fd, in_message )){
		switch( in_message->data->choiceId ){
			case PDUS_GET_REQUEST:
			case PDUS_GET_NEXT_REQUEST:
			case PDUS_SET_REQUEST:
			case PDUS_TRAP:
				ilmi_errno = 
				handle_request( fd, itf, msgs, in_message );
				if( ilmi_errno > 0 ) return NULL;
				else goto wait_top;
					/* no more evil than "break" */
			case PDUS_GET_RESPONSE:
				diag( COMPONENT, DIAG_INFO,
						"received get-response" );
				return in_message;
			default:
				diag( COMPONENT, DIAG_ERROR,
				"received message with invalid choice" );
				goto wait_top;
	      }
	} else{
		if( numfds == 0 ) ilmi_errno = ETIMEOUT;
		if(( numfds < 0 ) && ( errno == EINTR )) ilmi_errno = EALARM;
		return NULL;	/* no data read */
	}
}

Msgs *create_msgs( void ){
	Msgs *m = alloc_t( Msgs );
	if( m == NULL ) return NULL;

	m->coldstart = create_coldstart_message();
	m->addrtable = create_va_getnext_message( 2,
		&atmfAddressStatus,
		&atmfSrvcRegTable );
	m->poll = create_getnext_message( atmfAddressTable );
	m->set = create_set_message();
	m->config = create_va_get_message( 6,
		&atmfAtmLayerUniVersion,
		&atmfAtmLayerMaxVpiBits,
		&atmfAtmLayerMaxVciBits,
		&atmfAtmLayerUniType,
		&atmfAtmLayerDeviceType,
		&atmfAddressRegistrationAdminStatus );
	m->apointmsg = create_va_get_message( 4,
		&atmfPortMyIdentifier,
		&atmfPortMyIfName,	/* some extra adjacency info */
		&atmfMySystemIdentifier,
		&sysUpTime );
	m->sysmsg = create_va_get_message( 7,
		&sysDescr,
		&sysObjectID,
		&sysUpTime,
		&sysContact,
		&sysName,
		&sysLocation,
		&sysServices );

	return m;
}

void copy_attachment_point( AttPoint *dest, AttPoint *src ){
	dest->sysUpTime = src->sysUpTime;
	dest->atmfPortMyIfIdentifier = src->atmfPortMyIfIdentifier;

	dest->atmfMySystemIdentifier.octs =
		malloc( src->atmfMySystemIdentifier.octetLen );
	dest->atmfMySystemIdentifier.octetLen =
		src->atmfMySystemIdentifier.octetLen;
	if( dest->atmfMySystemIdentifier.octs != NULL ){
		memcpy( dest->atmfMySystemIdentifier.octs,
			src->atmfMySystemIdentifier.octs, 6 );
	} else diag( COMPONENT, DIAG_FATAL, "malloc() failed" );

	/* copy the port name */
	dest->atmfPortMyIfName.octs =
		malloc( src->atmfPortMyIfName.octetLen + 1 );
	dest->atmfPortMyIfName.octetLen = src->atmfPortMyIfName.octetLen;
	if( dest->atmfPortMyIfName.octs != NULL ){
		memcpy( dest->atmfPortMyIfName.octs,
			src->atmfPortMyIfName.octs,
			src->atmfPortMyIfName.octetLen );
	} else diag( COMPONENT, DIAG_FATAL, "malloc() failed" );
}

/* section 8.3.2 */
int compare_attachment_point( a, b )
AttPoint *a;	/* current a'point */
AttPoint *b;	/* newly aquired a'point info */
{
	if(( a == NULL ) || ( b == NULL )) return -1;
	if( a->atmfPortMyIfIdentifier != b->atmfPortMyIfIdentifier )
		return -1;
	if( a->atmfMySystemIdentifier.octetLen !=
		b->atmfMySystemIdentifier.octetLen ) return -1;
	if( memcmp( a->atmfMySystemIdentifier.octs,
		b->atmfMySystemIdentifier.octs,
		b->atmfMySystemIdentifier.octetLen ) != 0 ) return -1;
	if( b->sysUpTime < a->sysUpTime ) return -1;
	return 0;
	    }

void handler( int sig ){
	switch( sig ){
	case SIGALRM:
		alarm_flag = 1;
		diag( COMPONENT, DIAG_INFO,
			"received alarm: time to poll for attachment point" );
		break;
	case SIGTERM:
	case SIGSTOP:
	case SIGINT:
	case SIGKILL:
		ilmi_state = down;
		exit( 1 );
	}
    }

void ilmi_loop( int fd, int itf ){
	int state = S1;
	Msgs *msgs = create_msgs();
	struct itimerval setpoint;
	esi_oid = get_esi( fd, itf);
//	apoint->atmfMySystemIdentifier.octs = &(apoint->sysIdMem[0]);

	setpoint.it_interval.tv_sec = 30;
	setpoint.it_interval.tv_usec = 0;
	setpoint.it_value.tv_sec = 30;
	setpoint.it_value.tv_usec = 0;
	signal( SIGALRM, handler );
	setitimer( ITIMER_REAL, &setpoint, NULL );

	for( ;; ) state = state_functions[state]( fd, itf, msgs );
}

int main(int argc, char *argv[])
{
  int opt, bg = 0;
  int fd;
  pid_t pid;
  const char *qos;
  struct sockaddr_atmpvc ilmipvc;

  set_application("ilmid");
  set_verbosity(NULL, DIAG_WARN);
  text2atm("0.0.16", (struct sockaddr *)&ilmipvc, sizeof(ilmipvc), T2A_PVC);
  qos = NULL;
  while((opt = getopt(argc, argv, "bdhi:l:q:xvu:V")) != EOF)
    switch (opt)
      {
      case 'd':
	set_verbosity(NULL, DIAG_INFO);
	break;
      case 'v':
        set_verbosity(NULL, DIAG_DEBUG);
        break;
      case 'b':
	bg = 1;
	break;
      case 'i':
	{
	  uint32_t ip;

	  ip = text2ip(optarg,NULL,T2I_ERROR);
	  if (!ip) return 1;
	  set_local_ip(ip);
	}
	break;
      case 'l':
	set_logfile(optarg);
	break;
      case 'q':
	qos = optarg;
	break;
      case 'x':
	no_var_bindings = 1;
	break;
      case 'u':
	{
	  int version = 2;

	  if (!strcmp(optarg,"3.0") || !strcmp(optarg,"30"))
	    version = 2; // version3point0(2)
	  else if (!strcmp(optarg,"3.1") || !strcmp(optarg,"31"))
	    version = 3; // version3point1(3)
	  else if (!strcmp(optarg,"4.0") || !strcmp(optarg,"40"))
	    version = 4; // version4point0(4)
	  else usage(argv[0]);
#ifdef DYNAMIC_UNI
	  atmfAtmLayerUniVersionValue = version;
#else
	  if (atmfAtmLayerUniVersionValue != version) {
	    fprintf(stderr,"UNI version not available\n");
	    exit(1);
	  }
#endif
	}
	break;
      case 'V':
	printf("%s\n",VERSION);
	return 0;
      case 'h':
      default:
	usage(argv[0]);
      }
  if(argc == optind + 1)
    if (text2atm(argv[optind++], (struct sockaddr *)&ilmipvc,
        sizeof(ilmipvc), T2A_PVC) < 0) {
          char *endp = NULL;
          int itf = strtoul(argv[optind-1], &endp, 0);
          if (argv[optind-1] == endp || *endp != '\0')
            diag(COMPONENT, DIAG_FATAL, "Invalid pvc or interface number.");
          ilmipvc.sap_addr.itf = itf;
    }

  if(argc != optind)
    usage(argv[0]);

  diag(COMPONENT, DIAG_INFO, "Linux ATM ILMI, version %s", VERSION);

  if(bg)
    {
      pid = fork();
      if(pid < 0)
	diag(COMPONENT, DIAG_FATAL, "fork: %s", strerror(errno));
      if(pid > 0) exit(0);
    }
  
  InitNibbleMem(1024, 512);
  memset( remsys, 0, sizeof( _remsys ));
  memset( apoint, 0, sizeof( _apoint ));
  memset( newapoint, 0, sizeof( _newapoint ));
	signal( SIGTERM, handler );
	signal( SIGKILL, handler );
	signal( SIGINT, handler );
	signal( SIGSTOP, handler );
  fd = open_ilmi(&ilmipvc,qos);
  open_ilmidiag(&diagfd, &ilmipvc);
  ilmi_loop(fd, ilmipvc.sap_addr.itf);
  close(fd);
  return 0;
}
