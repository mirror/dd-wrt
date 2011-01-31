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

extern int ilmi_errno;
extern AttPoint *apoint;
extern Config *config;
extern SysGroup *remsys;

Message *wait_for_response( int fd, int itf, Msgs *msgs, int sec );
void copy_attachment_point( AttPoint *, AttPoint * );
int compare_attachment_point( AttPoint *, AttPoint * );

extern AttPoint *newapoint;

#define RESPONSE_TIMEOUT 2
#define POLL_PERIOD 15
#define COMPONENT "ILMI"


/* support functions */

#define RESP_RID(x)	((x)->data->a.get_response->request_id)
#define GETNEXT_RID(x)	((x)->data->a.get_next_request->request_id)

void reset_apoint( AttPoint *a ){
	if( a->atmfPortMyIfName.octs != NULL ){
		free( a->atmfPortMyIfName.octs );
		a->atmfPortMyIfName.octs = NULL;
	}
	if( a->atmfMySystemIdentifier.octs != NULL ){
		free( a->atmfMySystemIdentifier.octs );
		a->atmfMySystemIdentifier.octs = NULL;
	}
	memset( a, 0, sizeof( AttPoint ));
}

static void reset_remsys( void ){
	if( remsys->sysDescr.octs != NULL ) free( remsys->sysDescr.octs );
	if( remsys->sysObjectID.octs != NULL ) free( remsys->sysObjectID.octs );        if( remsys->sysContact.octs != NULL ) free( remsys->sysContact.octs );
	if( remsys->sysName.octs != NULL ) free( remsys->sysName.octs );
	if( remsys->sysLocation.octs != NULL ) free( remsys->sysLocation.octs );        memset( remsys, 0, sizeof( SysGroup ));
}

void wait_for_prefix( int fd, int itf, Msgs *msgs ){
	while(( wait_for_response(
		fd, itf, msgs, RESPONSE_TIMEOUT ) != NULL ) &&
		( ilmi_errno != ESETPREFIX ));
}

static void AsnOctsCopy( dest, src )
AsnOcts *dest;
AsnOcts *src;
{
	if( src->octetLen > 0 ){
		dest->octs = (char *)malloc( src->octetLen + 1 );
		memset( dest->octs, 0, src->octetLen + 1 );
		dest->octetLen = src->octetLen;
		memcpy( dest->octs, src->octs, src->octetLen );
	} else {
		dest->octs = (char *)malloc( 8 );
		dest->octetLen = 8;
		memcpy( dest->octs, "(empty)\0", 8 );
	}
}

Message *wait_for_sysgroup( int fd, int itf, Msgs *msgs ){
	Message *m = NULL;
	VarBind *varbind;
	VarBindList *vbl;

	m = wait_for_response( fd, itf, msgs, RESPONSE_TIMEOUT );
	if( m == NULL ) return NULL;
	while( GETNEXT_RID( msgs->sysmsg ) != RESP_RID( m ))
		m = wait_for_response( fd, itf, msgs, RESPONSE_TIMEOUT );

	reset_remsys();
	vbl = m->data->a.get_response->variable_bindings;
	FOR_EACH_LIST_ELMT( varbind, vbl ){
		if( AsnOidCompare( &sysDescr, &varbind->name ) == AsnOidEqual ){
			AsnOctsCopy( &remsys->sysDescr,
				varbind->value->a.simple->a.string );
		} else if( AsnOidCompare(
				&sysObjectID, &varbind->name ) == AsnOidEqual ){
			AsnOctsCopy( &remsys->sysObjectID,
				varbind->value->a.simple->a.string );
		} else if( AsnOidCompare(
				&sysUpTime, &varbind->name ) == AsnOidEqual ){
			remsys->sysUpTime =
				varbind->value->a.simple->a.number;
		} else if( AsnOidCompare(
				&sysContact, &varbind->name ) == AsnOidEqual ){
			AsnOctsCopy( &remsys->sysContact,
				varbind->value->a.simple->a.string );
		} else if( AsnOidCompare(
				&sysName, &varbind->name ) == AsnOidEqual ){
			AsnOctsCopy( &remsys->sysName,
				varbind->value->a.simple->a.string );
		} else if( AsnOidCompare(
				&sysLocation, &varbind->name ) == AsnOidEqual ){
			AsnOctsCopy( &remsys->sysLocation,
				varbind->value->a.simple->a.string );
		} else if( AsnOidCompare(
				&sysServices, &varbind->name ) == AsnOidEqual ){
			remsys->sysServices =
				varbind->value->a.simple->a.number;
		}
	}
	return m;
}

Message *wait_for_attachment_point( int fd, int itf, Msgs *msgs ){
	Message *m = NULL;
	VarBind *varbind;
	VarBindList *vbl;

	m = wait_for_response( fd, itf, msgs, RESPONSE_TIMEOUT );
	if( m == NULL ) return NULL;
	while( GETNEXT_RID( msgs->apointmsg ) != RESP_RID( m )){
		m = wait_for_response( fd, itf, msgs, RESPONSE_TIMEOUT );
		if( m == NULL ) return NULL;
	}

	reset_apoint( newapoint );
	vbl = m->data->a.get_response->variable_bindings;
	FOR_EACH_LIST_ELMT( varbind, vbl ){
		if( AsnOidCompare( &atmfMySystemIdentifier,
					&varbind->name ) == AsnOidEqual ){
			AsnOctsCopy( &(newapoint->atmfMySystemIdentifier),
				varbind->value->a.simple->a.string );
		} else if( AsnOidCompare( &atmfPortMyIdentifier,
					&varbind->name ) == AsnOidEqual ){
			newapoint->atmfPortMyIfIdentifier =
				varbind->value->a.simple->a.number;
		} else if( AsnOidCompare( &sysUpTime, &varbind->name )
					== AsnOidEqual ){
			newapoint->sysUpTime =
				varbind->value->a.simple->a.number;
		} else if( AsnOidCompare( &atmfPortMyIfName, &varbind->name )
					== AsnOidEqual ){
			AsnOctsCopy( &(newapoint->atmfPortMyIfName),
				varbind->value->a.simple->a.string );
		}
	}
	return m;
}

Message *wait_for_config( int fd, int itf, Msgs *msgs ){
	Message *m = NULL;
	VarBind *varbind;
	VarBindList *vbl;

	m = wait_for_response( fd, itf, msgs, RESPONSE_TIMEOUT );
	if( m == NULL ) return NULL;
	if( RESP_RID( m ) != GETNEXT_RID( msgs->config )){
		diag( COMPONENT, DIAG_ERROR,
		"received response with invalid request id" );
	}

	vbl = m->data->a.get_response->variable_bindings;
	FOR_EACH_LIST_ELMT( varbind, vbl ){
		if( AsnOidCompare( &atmfAtmLayerDeviceType, &varbind->name )
			== AsnOidEqual ) config->atmfAtmLayerDeviceType =
					varbind->value->a.simple->a.number;
		else
		if( AsnOidCompare( &atmfAtmLayerUniVersion, &varbind->name )
			== AsnOidEqual ) config->atmfAtmLayerUniVersion =
					varbind->value->a.simple->a.number;
		else
		if( AsnOidCompare( &atmfAtmLayerUniType, &varbind->name )
			== AsnOidEqual ) config->atmfAtmLayerUniType =
					varbind->value->a.simple->a.number;
		else
		if( AsnOidCompare( &atmfAtmLayerMaxVpiBits, &varbind->name )
			== AsnOidEqual ) config->atmfAtmLayerMaxVpiBits =
					varbind->value->a.simple->a.number;
		else
		if( AsnOidCompare( &atmfAtmLayerMaxVciBits, &varbind->name )
			== AsnOidEqual ) config->atmfAtmLayerMaxVciBits =
					varbind->value->a.simple->a.number;
		else
		if( AsnOidCompare( &atmfAddressRegistrationAdminStatus,
			&varbind->name ) == AsnOidEqual )
			config->atmfAddressRegistrationAdminStatus =
			varbind->value->a.simple->a.number;
	}
	return m;
}

Message *wait_for_status( int fd, int itf, Msgs *msgs ){
	Message *m = NULL;
	VarBind *varbind;
	VarBindList *vbl;

	m = wait_for_response( fd, itf, msgs, RESPONSE_TIMEOUT );
	if( m == NULL ) return NULL;

	while( RESP_RID( m ) != GETNEXT_RID( msgs->addrtable )){
		m = wait_for_response( fd, itf, msgs, RESPONSE_TIMEOUT );
		if( m == NULL ) return NULL;
	}

	/* retrieve the lecs address */
	vbl = m->data->a.get_response->variable_bindings;
	FOR_EACH_LIST_ELMT(varbind, vbl) {
		if (AsnOidCompare(&atmfSrvcRegATMAddress,
					&varbind->name) == AsnOidRoot) {
			add_lecs(itf, varbind->value->a.simple->a.string->octs);
		} else if (AsnOidCompare(&atmfAddressStatus,
					&varbind->name) == AsnOidRoot) {
			diag(COMPONENT, DIAG_ERROR,
				"address already registered in switch");
		}
	}

	return m;
}

Message *wait_for_setresponse( int fd, int itf, Msgs *msgs ){
	Message *m = NULL;
	m = wait_for_response( fd, itf, msgs, RESPONSE_TIMEOUT );
	return m;
}

