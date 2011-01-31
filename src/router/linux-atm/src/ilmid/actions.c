#include <unistd.h>
#include <stdlib.h>
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
#include "sysgroup.h"
#include "ilmid.h"

#define COMPONENT	"ILMI"

extern struct attachment_point *apoint;
extern int retries;
extern long int requestID;

void action_A1( int fd, Msgs *msgs ){
	/* Reset Uptime */
	diag( COMPONENT, DIAG_INFO, "Action A1: reset uptime" );
	resetUpTime();
}

void action_A2( int fd, Msgs *msgs ){
	/* Reset Attachment Point */
	diag( COMPONENT, DIAG_INFO, "Action A2: reset attachment point" );

	/* should probably use reset_apoint() here */
	if( apoint->atmfPortMyIfName.octs ){
		free( apoint->atmfPortMyIfName.octs );
		apoint->atmfPortMyIfName.octs = NULL;
	}
	memset( apoint, 0, sizeof( struct attachment_point ));
}

void action_A3( int fd, Msgs *msgs ){
	/* Set Attachment Point */
	/* This doesn't really exist */
	/* see copy_attachment_point() */
	diag( COMPONENT, DIAG_INFO, "Action A3: set attachment point" );
}

void action_A4( int fd, Msgs *msgs ){
	/* Clear Tables */
	diag( COMPONENT, DIAG_INFO, "Action A4: clear tables" );
	deleteNetPrefix();
}

void action_A5( int fd, Msgs *msgs ){
	/* start timer */
	/* Our timer is provided by select() */
	diag( COMPONENT, DIAG_INFO, "Action A5: start timer" );
}

void action_A6( int fd, Msgs *msgs ){
	/* stop timer */
	diag( COMPONENT, DIAG_INFO, "Action A6: stop timer" );
}

void action_A7( int fd, Msgs *msgs ){
	diag( COMPONENT, DIAG_INFO, "Action A7: clear retries" );
	retries = 0;
}

void action_A8( int fd, Msgs *msgs ){
	diag( COMPONENT, DIAG_INFO, "Action A8: increment retries" );
	retries++;
}

void action_A9( int fd, Msgs *msgs ){
	/* clear all SVCs */
	/* not sure how to do this, yet */
	diag( COMPONENT, DIAG_INFO, "Action A9: release SVCs" );
}

void action_A10( int fd, Msgs *msgs ){
	diag( COMPONENT, DIAG_INFO, "Action A10: start signalling" );
	/* not sure how to do this, yet */
}

void action_A11( int fd, Msgs *msgs ){
	diag( COMPONENT, DIAG_INFO, "Action A11: set coldstart" );
	(msgs->coldstart)->data->a.trap->time_stamp = accessUpTime();
	send_message( fd, msgs->coldstart );
}

/* Attachment point info */
/* ILMI spec - page 90 */
void action_A12( int fd, Msgs *msgs ){
	diag( COMPONENT, DIAG_INFO, "Action A12: get attachment point" );
	msgs->apointmsg->data->a.get_next_request->request_id = ++requestID;
	send_message( fd, msgs->apointmsg );
}

/* Configuration info */
/* ILMI spec - page 90 */
void action_A13( int fd, Msgs *msgs ){
	Message *config;
	diag( COMPONENT, DIAG_INFO, "Action A13: get configuration" );

	/* don't actually get all of the config info -- we probably
	   will not be using much of it anyway */

	config = msgs->config;
	config->data->a.get_next_request->request_id = ++requestID;
	send_message( fd, config );
}

void action_A14( int fd, Msgs *msgs ){
	diag( COMPONENT, DIAG_INFO, "Action A14: send getNextRequest" );
	msgs->addrtable->data->a.get_next_request->request_id = ++requestID;
	send_message( fd, msgs->addrtable );
}

void action_A15( int fd, Msgs *msgs ){
	diag( COMPONENT, DIAG_INFO, "Action A15: send set request" );
	msgs->set->data->a.get_next_request->request_id = ++requestID;
	send_message( fd, msgs->set );
}

void action_get_sysgroup( int fd, Msgs *msgs ){
	diag( COMPONENT, DIAG_INFO, "Action Get Sysgroup: request system mib" );
	msgs->sysmsg->data->a.get_request->request_id = ++requestID;
	send_message( fd, msgs->sysmsg );
}
