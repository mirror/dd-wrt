#if !defined(CORE_API_H)
#define CORE_API_H

#ifdef CORE_SUPPORT_API

#define HD_WRITECACHE_OFF		0
#define HD_WRITECACHE_ON		1

MV_VOID
Core_GetHDInfo(
	IN MV_PVOID extension,
	IN MV_U16 HDId,
    OUT PHD_Info pHD );

MV_VOID
Core_GetExpInfo(
	IN MV_PVOID extension,
	IN MV_U16 ExpId,
    OUT PExp_Info pExp 
	);

MV_VOID
Core_GetPMInfo(
	IN MV_PVOID extension,
	IN MV_U16 PMId,
    OUT PPM_Info pPM 
	);

MV_VOID
Core_GetHDConfig(
	IN MV_PVOID extension,
	IN MV_U16 HDId,
    OUT PHD_Config pHD 
	);

MV_BOOLEAN 
Core_pd_command(
	IN MV_PVOID extension, 
	IN PMV_Request pReq
	);

#endif

#endif

