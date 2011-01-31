#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "asn_incl.h"
#include "rfc1155_smi.h"
#include "rfc1157_snmp.h"
#include "message.h"
#include "util.h"
#include "io.h"
#include "atmd.h"
#include "mib.h"
#include "atmf_uni.h"
#include "states.h"
#include "ilmid.h"
#include "actions.h"
#include "wait.h"

extern int retries;
extern int ilmi_errno;
extern long int requestID;
extern AsnOid *esi_oid;
extern AttPoint *apoint;
extern Config *config;
extern int alarm_flag;
extern State ilmi_state;
extern AttPoint *newapoint;

Message *wait_for_response( int fd, int itf, Msgs *msgs, int sec );
void copy_attachment_point( AttPoint *, AttPoint * );
int compare_attachment_point( AttPoint *, AttPoint * );

#define RESPONSE_TIMEOUT 2
#define POLL_PERIOD 15
#define COMPONENT "ILMI"

/* state functions */

int state_stopped( int fd, int itf, Msgs *msgs ){
	diag( COMPONENT, DIAG_INFO, "State S1: stopped" );
	ilmi_state = down;
	action_A1( fd, msgs );		/* reset uptime */
	action_A2( fd, msgs );		/* reset attachment point info */
	action_A4( fd, msgs );		/* clear tables */
	return S2;
}

int state_failing( int fd, int itf, Msgs *msgs ){
	Message *m;

	diag( COMPONENT, DIAG_INFO, "State S2: failing" );
	ilmi_state = down;
	action_A11( fd, msgs );		/* send coldstart */
	action_A12( fd, msgs );		/* send "get" for attachment point */

	/* continue to ask for attachment point as long as it times out */
	while(( m = wait_for_attachment_point( fd, itf, msgs )) == NULL ){
		if( ilmi_errno == ECOLDSTART ) return S1;
		action_A11( fd, msgs );	/* send coldstart */
		action_A12( fd, msgs );	/* send "get" for attachment point */
	}
	action_A11( fd, msgs );		/* one more for good measure */

	return S3;
}

int state_establishing( int fd, int itf, Msgs *msgs ){
	Message *m;

	diag( COMPONENT, DIAG_INFO, "State S3: establishing" );
	ilmi_state = down;

	/* if they are the same ... */
	if( compare_attachment_point( apoint, newapoint ) == 0 ){
		action_A13( fd, msgs );	/* request config */
	} else{
//		reset_apoint( apoint );
		/* copy( dest, src ) */
		copy_attachment_point( apoint, newapoint );	/* action_A3 */
		action_A9( fd, msgs );	/* release SVCs */
		action_A13( fd, msgs );	/* request config */
		action_A7( fd, msgs );	/* retries = 0 */
		action_A5( fd, msgs );	/* start timer */
	}

	while(( m = wait_for_config( fd, itf, msgs )) == NULL ){
		if( ilmi_errno == ETIMEOUT ) action_A13( fd, msgs );
		else if( ilmi_errno == ECOLDSTART ) return S1;
	}


	return S4;
}

int state_config( int fd, int itf, Msgs *msgs ){
	Message *tmp;
	int newstate = S1;

	diag( COMPONENT, DIAG_INFO, "State S4: config" );
	ilmi_state = down;

	if( config->atmfAtmLayerUniVersion == 0 ){
		diag( COMPONENT, DIAG_ERROR, "remote IME did not negotiate "
			"a UNI version -- using configured uni version." );
	} else
	if( config->atmfAtmLayerUniVersion < atmfAtmLayerUniVersionValue ){
		diag( COMPONENT, DIAG_ERROR, "remote IME does not support"
			" the locally configured uni version" );
		atmfAtmLayerUniVersionValue = config->atmfAtmLayerUniVersion;
	}

	if( config->atmfAtmLayerDeviceType == 2 /* NODE */ ){
		action_A6( fd, msgs );	/* stop timer */
		action_A10( fd, msgs );	/* Start signalling */
		action_A14( fd, msgs );	/* request address status */
		action_A7( fd, msgs );	/* retries = 0 */
		action_A5( fd, msgs );	/* start timer */
		newstate = S5;	/* retrieve network prefixes */
	} else if( config->atmfAtmLayerDeviceType == 1 /* USER */ ){
		diag( COMPONENT, DIAG_ERROR, "Remote device is also a host" );
		action_A10( fd, msgs );	/* start signalling */
		action_A14( fd, msgs );	/* get next prefix */
		action_A7( fd, msgs );	/* retries = 0 */
		action_A5( fd, msgs );	/* start timer */
		newstate = S6;
	} else if( config->atmfAtmLayerDeviceType == 0 /* ??? */ ){
		diag( COMPONENT, DIAG_ERROR, "Remote IME does not know "
			"what type of device it is -- assume NODE" );
		action_A6( fd, msgs );	/* stop timer */
		action_A10( fd, msgs );	/* Start signalling */
		action_A14( fd, msgs );	/* request address status */
		action_A7( fd, msgs );	/* retries = 0 */
		action_A5( fd, msgs );	/* start timer */
		newstate = S5;	/* retrieve network prefixes */
	}

	while(( tmp = wait_for_status( fd, itf, msgs )) == NULL ){
		if( ilmi_errno == ETIMEOUT ){
			action_A14( fd, msgs );	/* request address status */
			action_A8( fd, msgs );	/* retries++ */
			action_A5( fd, msgs );	/* start timer */
		} else if( ilmi_errno == ECOLDSTART ){
			return S1;
		}
	}

	action_get_sysgroup( fd, msgs );
	while(( tmp = wait_for_sysgroup( fd, itf, msgs )) == NULL ){
		if( ilmi_errno == ETIMEOUT ){
			action_get_sysgroup( fd, msgs );
			action_A8( fd, msgs );	/* retries++ */
			action_A5( fd, msgs );	/* start timer */
		} else if( ilmi_errno == ECOLDSTART ){
			return S1;
		}
	}

	return newstate;
}

int state_retrievePrefixes( int fd, int itf, Msgs *msgs ){
	int newstate = S1;

	diag( COMPONENT, DIAG_INFO, "State S5: retrieve prefixes" );
	ilmi_state = down;

	/* assume address registration is supported on peer */
	wait_for_prefix( fd, itf, msgs );
	newstate = S6;	/* register network prefixes */
	return newstate;
}

int state_registerPrefixes( int fd, int itf, Msgs *msgs ){
	Message *m;

	diag( COMPONENT, DIAG_INFO, "State S6: register prefixes" );
	ilmi_state = down;

	/* If RegistrationAdminStatus == 0, switch is confused.  Assume
	 * it supports registration. */

	if(( config->atmfAddressRegistrationAdminStatus == 1 ) ||
	   ( config->atmfAddressRegistrationAdminStatus == 0 )){
		action_A6( fd, msgs );	/* stop timer */
		action_A15( fd, msgs );	/* set request for table entry */
		action_A7( fd, msgs );	/* retries = 0 */
		action_A5( fd, msgs );	/* start timer */

		while(( m = wait_for_setresponse( fd, itf, msgs )) == NULL ){
			if( ilmi_errno == ETIMEOUT ){
				/* set request for table entry */
				action_A15( fd, msgs );
				action_A8( fd, msgs );	/* retries++ */
				action_A5( fd, msgs );	/* start timer */
			} else if( ilmi_errno == ECOLDSTART ){
				return S1;
			}
		}
	} else{
		diag( COMPONENT, DIAG_ERROR,
			"address registration not supported" );
		action_A6( fd, msgs );	/* stop timer */
		action_A12( fd, msgs );	/* request a.point */
		action_A7( fd, msgs );	/* retries = 0 */
		action_A5( fd, msgs );	/* start timer */
	}

	return S9;	/* verify */
}

int state_retrieveAddresses( int fd, int itf, Msgs *msgs ){
	/* This would only be called on a switch -- not yet implemented */
	diag( COMPONENT, DIAG_INFO, "State S7: retrieve addresses" );
	ilmi_state = down;
	return S9;
}

int state_registerAddresses( int fd, int itf, Msgs *msgs ){
	/* not yet implemented */
	diag( COMPONENT, DIAG_INFO, "State S8: register addresses" );
	ilmi_state = down;
	return S9;
}

/* Should spend most of our time here */
/* ask for updates.  if new prefix is set, update and notify user */
int state_verify( int fd, int itf, Msgs *msgs ){
	Message *m;

	diag( COMPONENT, DIAG_INFO, "State S9: verify" );
	ilmi_state = up;
	action_A6( fd, msgs );	/* stop timer */
	action_A7( fd, msgs );	/* retries = 0 */
	action_A5( fd, msgs );	/* start timer */

	m = wait_for_response( fd, itf, msgs, POLL_PERIOD );
	if(( m == NULL ) && ( ilmi_errno == ECOLDSTART )){
		return S1;
	} else if( ilmi_errno == EALARM ){
		/* interface information must be no older than 30 seconds */
		alarm_flag = 0;
		action_A12( fd, msgs );	/* request attachment point */
		while((( m = wait_for_attachment_point( fd, itf, msgs ))
						== NULL ) && ( retries < 4 )){
			action_A12( fd, msgs );
			action_A8( fd, msgs );
		}
		if( retries >= 4 ) return S2;
		/* check for attachment point change */
	}
	return S9;
}

