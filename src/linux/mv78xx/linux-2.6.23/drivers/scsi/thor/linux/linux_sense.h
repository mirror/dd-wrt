#if !defined(_LINUX_SENSE_H)
#define _LINUX_SENSE_H

typedef struct _SENSE_DATA {
    MV_U8 ErrorCode:7;
    MV_U8 Valid:1;
    MV_U8 SegmentNumber;
    MV_U8 SenseKey:4;
    MV_U8 Reserved:1;
    MV_U8 IncorrectLength:1;
    MV_U8 EndOfMedia:1;
    MV_U8 FileMark:1;
    MV_U8 Information[4];
    MV_U8 AdditionalSenseLength;
    MV_U8 CommandSpecificInformation[4];
    MV_U8 AdditionalSenseCode;
    MV_U8 AdditionalSenseCodeQualifier;
    MV_U8 FieldReplaceableUnitCode;
    MV_U8 SenseKeySpecific[3];
} SENSE_DATA, *PSENSE_DATA;

void HBA_Translate_Req_Status_To_OS_Status(
	PHBA_Extension pHBA,
	struct scsi_cmnd *pSCmd,
	PMV_Request pReq
	);
	
#endif
