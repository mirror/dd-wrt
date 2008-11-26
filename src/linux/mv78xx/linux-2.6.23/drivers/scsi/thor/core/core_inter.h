#if !defined(CORE_MAIN_H)
#define CORE_MAIN_H

#include "core_thor.h"

#ifdef SUPPORT_CONSOLIDATE
#include "consolid.h"
#endif

#ifdef _OS_WINDOWS
#define CPU_TO_LE_16(x) x
#define CPU_TO_LE_32(x) x
#endif /* _OS_WINDOWS  */

struct _Domain_Port;
typedef struct _Domain_Port Domain_Port, *PDomain_Port;

struct _Domain_Device;
typedef struct _Domain_Device Domain_Device, *PDomain_Device;

#define CORE_STATE_IDLE			0
#define CORE_STATE_STARTED		1

/* Flag definition for Fast Boot Skip */
#define FLAG_SKIP_PATA_PORT		MV_BIT(0)
#define FLAG_SKIP_PATA_DEVICE	MV_BIT(1)
#define FLAG_SKIP_PM			MV_BIT(2)

typedef struct _Core_Driver_Extension
{
	MV_LPVOID	Mmio_Base;						/* Memory IO base address */
	MV_U16		Vendor_Id;
	MV_U16		Device_Id;
	MV_U8		State;
	MV_U8		Revision_Id;
	MV_U8		VS_Reg_Saved;
	MV_U8		Flag_Fastboot_Skip;

	MV_U32		Capacity;						
	MV_U32		Port_Map;
	MV_U8		Port_Num;						/* How much ports we have? */
	MV_U8		SATA_Port_Num;
	MV_U8		PATA_Port_Num;
	MV_U8		Adapter_State;					/* Adatper state */
	MV_U8		Is_Dump;						/* Is during dump */
	MV_U8		Need_Reset;						/* Need_Reset == 1 means controller need reset. Lily 3/7/2006*/
	MV_U8		Resetting;
#ifdef _OS_BIOS
	MV_U8		host_reseting;
#else
	MV_U8		Reserved1;
#endif

	MV_U8		Total_Device_Count;
	MV_U8		Reserved2[3];

	MV_LPVOID	Base_Address[MAX_BASE_ADDRESS];	/* Base Address */
	Domain_Port Ports[MAX_PORT_NUMBER];			/* Domain Ports */

	List_Head	Waiting_List; 					/* Waiting Request Queue */
	List_Head	Internal_Req_List;				/* Internal Request Queue */

#ifdef SUPPORT_CONSOLIDATE
	PConsolidate_Extension	pConsolid_Extent;	
	PConsolidate_Device		pConsolid_Device;
#endif
}Core_Driver_Extension, *PCore_Driver_Extension;

#endif /* CORE_MAIN_H */

