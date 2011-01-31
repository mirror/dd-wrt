#ifndef __ILMID_H
#define __ILMID_H

#define	ECOLDSTART	1
#define	ETIMEOUT	2
#define ESETPREFIX	3
#define	EALARM		4

#include "message.h"
#include "asn1/asn_incl.h"

typedef struct {
	Message *apointmsg;
	Message *coldstart;
	Message *addrtable;
	Message *poll;
	Message *set;
	Message *config;
	Message *sysmsg;
} Msgs;

typedef struct attachment_point {
	AsnOcts atmfPortMyIfName;
	AsnInt atmfPortMyIfIdentifier;
	AsnOcts atmfMySystemIdentifier;
	AsnInt sysUpTime;
//	char sysIdMem[6];
} AttPoint;

typedef struct configuration_info {
	AsnInt atmfAtmLayerUniVersion;
	AsnInt atmfAtmLayerMaxVpiBits;
	AsnInt atmfAtmLayerMaxVciBits;
	AsnInt atmfAtmLayerUniType;
	AsnInt atmfAtmLayerDeviceType;
	AsnInt atmfAddressRegistrationAdminStatus;
} Config;

typedef struct __sysgroup {
	AsnOcts sysDescr;
	AsnOcts sysObjectID;
	AsnInt sysUpTime;
	AsnOcts sysContact;
	AsnOcts sysName;
	AsnOcts sysLocation;
	AsnInt sysServices;
} SysGroup;

typedef enum _State { up, down } State;

#define ATM_REMOTE_SYSNAME_LEN		15
#define ATM_REMOTE_CONTACT_LEN		30
#define ATM_REMOTE_PORTNAME_LEN		10
#define ILMIDIAG_DIR "/tmp/.ilmi"
#define ILMIDIAG_BACKLOG 1
struct ilmi_state {
	unsigned char ilmi_version;	/* ilmi version */
	unsigned char uni_version;	/* negotiated signalling version */
	unsigned char vpi_bits;		/* negotiated vpi bits */
	unsigned char vci_bits;		/* negotiated vci bits */
	unsigned char state;		/* up/down */
	unsigned int remote_portid;	/* id of switch port we're using */
	char remote_sysname[ATM_REMOTE_SYSNAME_LEN];
	char remote_contact[ATM_REMOTE_CONTACT_LEN];
	char remote_portname[ATM_REMOTE_PORTNAME_LEN];
};

#endif

