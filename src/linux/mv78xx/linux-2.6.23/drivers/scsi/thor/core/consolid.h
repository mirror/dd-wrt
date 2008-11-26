#ifndef _CONSOLIDATE_H
#define _CONSOLIDATE_H

/*
 * Here is the definition for the command consolidate sub module
 * This is only changed when we modify the consolidate algorithm.
 */
#define CONS_MAX_INTERNAL_REQUEST_COUNT	32

#define CONS_MAX_EXTERNAL_REQUEST_SIZE	(1024*64)
#define CONS_SEQUENTIAL_MAX				0x7FFF		/* Avoid overflow. It's determined by Sequential variable size */
#define CONS_SEQUENTIAL_THRESHOLD		64			/* Must bigger than OS outstanding request. Refer to Consolid_RequestCallBack */

#define CONS_MAX_INTERNAL_REQUEST_SIZE	(1024*128)	/* The maximum request size hardware can handle. */
#define CONS_MIN_INTERNAL_REQUEST_SIZE	(1024*128)	/* We'll accumulate the request to this size and then fire. */


typedef struct _Consolidate_Extension
{
	MV_Request	Requests[CONS_MAX_INTERNAL_REQUEST_COUNT];
	List_Head	Free_Queue;
}Consolidate_Extension, *PConsolidate_Extension;

typedef struct _Consolidate_Device
{
	MV_LBA		Last_LBA;				/* last LBA*/
	PMV_Request Holding_Request;		/* Internal request which already consolidate some external requests. */
	MV_U16		Sequential;				/* sequential counter */
	MV_BOOLEAN	Is_Read;				/* The last request is read or write. */
	MV_U8		Reserved0;
	MV_U16		Reserved1[2];			
}Consolidate_Device, *PConsolidate_Device;

void
Consolid_ModuleSendRequest(
	MV_PVOID This,
	PMV_Request pReq
	);

void 
Consolid_InitializeExtension(
	MV_PVOID This
	);

void
Consolid_InitializeDevice(
	MV_PVOID This,
	MV_U16 Device_Id
	);

void
Consolid_PushFireRequest(
	MV_PVOID This,
	MV_U16 Device_Id
	);

#endif

