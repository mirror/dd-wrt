#ifndef COM_EVENT_DRIVER_H
#define COM_EVENT_DRIVER_H

#include "com_define.h"

#define MAX_EVENTS				20
#define MAX_EVENT_PARAMS		4
#define MAX_EVENTS_RETURNED		6

#ifndef _OS_BIOS
#pragma pack(8)	//TBD
#endif

typedef struct _DriverEvent
{
	MV_U32		TimeStamp;
	MV_U32		SequenceNo;	// Event sequence number (contiguous in a single adapter)
	MV_U32		EventID;	// 1st 16 bits - Event class
							// last 16 bits - Event code of this particular Event class
	MV_U8		Severity;
	MV_U8		AdapterID;
	MV_U16		DeviceID;	// Device ID relate to the event class (HD ID, LD ID etc) 
	MV_U32		Params[MAX_EVENT_PARAMS];	// Additional information if ABSOLUTELY necessary.
} DriverEvent, * PDriverEvent;

typedef struct _EventRequest
{
	MV_U8		Count;		// [OUT] # of actual events returned
	MV_U8		Reserved[3];
	DriverEvent	Events[MAX_EVENTS_RETURNED]; 
} EventRequest, * PEventRequest;

#ifndef _OS_BIOS
#pragma pack()
#endif

#endif

