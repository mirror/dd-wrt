#if !defined(COM_SCSI_H)
#define COM_SCSI_H

/*
 * SCSI command
 */
#define SCSI_CMD_INQUIRY				0x12
#define SCSI_CMD_START_STOP_UNIT		0x1B
#define SCSI_CMD_TEST_UNIT_READY		0x00
#define SCSI_CMD_RESERVE_6				0x16
#define SCSI_CMD_RELEASE_6				0x17

#define SCSI_CMD_READ_6					0x08
#define SCSI_CMD_READ_10				0x28
#define SCSI_CMD_READ_12				0xA8
#define SCSI_CMD_READ_16				0x88

#define SCSI_CMD_WRITE_6				0x0A
#define SCSI_CMD_WRITE_10				0x2A
#define SCSI_CMD_WRITE_12				0xAA
#define SCSI_CMD_WRITE_16				0x8A

#define SCSI_CMD_READ_CAPACITY_10		0x25
#define SCSI_CMD_READ_CAPACITY_16		0x9E	/* 9Eh/10h */

#define SCSI_CMD_VERIFY_10				0x2F
#define SCSI_CMD_VERIFY_16				0x8F

#define SCSI_CMD_REQUEST_SENSE			0x03
#define SCSI_CMD_MODE_SENSE_6			0x1A
#define SCSI_CMD_MODE_SENSE_10			0x5A
#define SCSI_CMD_MODE_SELECT_6			0x15
#define SCSI_CMD_MODE_SELECT_10			0x55

#define SCSI_CMD_WRITE_VERIFY_10		0x2E
#define SCSI_CMD_SYNCHRONIZE_CACHE_10	0x35

/* ATAPI CDB */
#define SCSI_CMD_REPORT_LUN						0xA0
#define SCSI_CMD_PREVENT_MEDIUM_REMOVAL			0x1E
#define SCSI_CMD_READ_SUB_CHANNEL				0x42
#define SCSI_CMD_READ_TOC						0x43
#define SCSI_CMD_READ_DISC_STRUCTURE			0xAD
#define SCSI_CMD_READ_CD						0xBE
#define SCSI_CMD_GET_EVENT_STATUS_NOTIFICATION	0x4A

/* MMC */
#define SCSI_CMD_READ_DISC_INFO				0x51

#define SCSI_IS_READ(cmd)			\
	( ((cmd)==SCSI_CMD_READ_6)		\
	|| ((cmd)==SCSI_CMD_READ_10)	\
	|| ((cmd)==SCSI_CMD_READ_12)	\
	|| ((cmd)==SCSI_CMD_READ_16) )

#define SCSI_IS_WRITE(cmd)			\
	( ((cmd)==SCSI_CMD_WRITE_6)		\
	|| ((cmd)==SCSI_CMD_WRITE_10)	\
	|| ((cmd)==SCSI_CMD_WRITE_12)	\
	|| ((cmd)==SCSI_CMD_WRITE_16) )

#define SCSI_IS_MODE_SENSE(cmd)				\
	( ((cmd)==SCSI_CMD_MODE_SENSE_6)		\
	|| ((cmd)==SCSI_CMD_MODE_SENSE_10) )

#define SCSI_IS_REQUEST_SENSE(cmd)			\
	( ((cmd)==SCSI_CMD_REQUEST_SENSE) )

#define SCSI_IS_VERIFY(cmd)			( ((cmd)==SCSI_CMD_VERIFY_10) || ((cmd)==SCSI_CMD_VERIFY_16) )

//TBD: Define for other modules
#define SCSI_CMD_MARVELL_SPECIFIC	0xE1
	#define CDB_CORE_MODULE						0x1
		#define CDB_CORE_IDENTIFY				0x1
		#define CDB_CORE_SET_UDMA_MODE			0x2
		#define CDB_CORE_SET_PIO_MODE			0x3
		#define CDB_CORE_ENABLE_WRITE_CACHE		0x4
		#define CDB_CORE_DISABLE_WRITE_CACHE	0x5
		#define CDB_CORE_ENABLE_SMART			0x6
		#define CDB_CORE_DISABLE_SMART			0x7
		#define CDB_CORE_SMART_RETURN_STATUS	0x8
		#define CDB_CORE_SHUTDOWN				0x9
		#define CDB_CORE_ENABLE_READ_AHEAD		0xA
		#define CDB_CORE_DISABLE_READ_AHEAD		0xB
		#define CDB_CORE_READ_LOG_EXT			0xC
#ifdef CACHE_MODULE_SUPPORT
	#define CDB_CACHE_MODULE						0x2
		#define CBD_CACHE_FLUSH_DEV                     0xD
#endif

#define SCSI_IS_INTERNAL(cmd)		((cmd)==SCSI_CMD_MARVELL_SPECIFIC)

/*
 * SCSI status
 */
#define SCSI_STATUS_GOOD					0x00			/* Good */
#define SCSI_STATUS_CHECK_CONDITION			0x02			/* Sense data is valid */
#define SCSI_STATUS_CONDITION_MET			0x04			/* We don't use it */
#define SCSI_STATUS_BUSY					0x08			/* Busy */
#define SCSI_STATUS_INTERMEDIATE			0x10			/* We don't use it */
#define SCSI_STATUS_INTERMEDIATE_MET		0x14			/* We don't use it */
#define SCSI_STATUS_RESERVATION_CONFLICT	0x18			/* We don't use it */
#define SCSI_STATUS_FULL					0x28			/* Task set full */
#define SCSI_STATUS_ACA_ACTIVE				0x30			/* We don't use it */
#define SCSI_STATUS_ABORTED					0x40			/* We don't use it */

/*
 * SCSI sense key
 */
#define SCSI_SK_NO_SENSE					0x00	/* No sense key */
#define SCSI_SK_RECOVERED_ERROR				0x01	/* Command completed with recovery action */
#define SCSI_SK_NOT_READY					0x02	/* Not ready */
#define SCSI_SK_MEDIUM_ERROR				0x03	/* Finally failed even with recovery */
#define SCSI_SK_HARDWARE_ERROR				0x04	/* Hardware error */
#define SCSI_SK_ILLEGAL_REQUEST				0x05	/* Invalid CDB */
#define SCSI_SK_UNIT_ATTENTION				0x06	/* Unit needs attention */
#define SCSI_SK_DATA_PROTECT				0x07	/* We don't use it */
#define SCSI_SK_BLANK_CHECK					0x08	/* We don't use it */
#define SCSI_SK_VENDOR_SPECIFIC				0x09	/* We don't use it */
#define SCSI_SK_COPY_ABORTED				0x0A	/* We don't use it */
#define SCSI_SK_ABORTED_COMMAND				0x0B	/* We don't use it */
#define SCSI_SK_VOLUME_OVERFLOW				0x0D	/* We don't use it */
#define SCSI_SK_MISCOMPARE					0x0E	/* We don't use it */

/*
 * SCSI additional sense code
 */
#define SCSI_ASC_NO_ASC						0x00
#define SCSI_ASC_LUN_NOT_READY				0x04
#define SCSI_ASC_ECC_ERROR					0x10
#define SCSI_ASC_ID_ADDR_MARK_NOT_FOUND		0x12
#define SCSI_ASC_INVALID_OPCODE				0x20
#define SCSI_ASC_LBA_OUT_OF_RANGE			0x21
#define SCSI_ASC_INVALID_FEILD_IN_CDB		0x24
#define SCSI_ASC_LOGICAL_UNIT_NOT_SUPPORTED	0x25
#define SCSI_ASC_INVALID_FIELD_IN_PARAMETER	0x26
#define SCSI_ASC_INTERNAL_TARGET_FAILURE	0x44

/*
 * SCSI additional sense code qualifier
 */
#define SCSI_ASCQ_NO_ASCQ					0x00
#define SCSI_ASCQ_INTERVENTION_REQUIRED		0x03
#define SCSI_ASCQ_MAINTENANCE_IN_PROGRESS	0x80

/* SCSI command CDB helper functions. */
#define SCSI_CDB10_GET_LBA(cdb)	((MV_U32)(((MV_U32)cdb[2]<<24) | ((MV_U32)cdb[3]<<16) | ((MV_U32)cdb[4]<<8) | (MV_U32)cdb[5]))
#define SCSI_CDB10_SET_LBA(cdb, lba)	{\
	cdb[2] = (MV_U8)(lba >> 24);	\
	cdb[3] = (MV_U8)(lba >> 16);	\
	cdb[4] = (MV_U8)(lba >> 8);	\
	cdb[5] = (MV_U8)lba;	\
}
#define SCSI_CDB10_GET_SECTOR(cdb)	((cdb[7]<<8) | cdb[8])
#define SCSI_CDB10_SET_SECTOR(cdb, sector) {\
	cdb[7] = (MV_U8)(sector >> 8);	\
	cdb[8] = (MV_U8)sector;	\
}

#define MV_SCSI_RESPONSE_CODE			0x70
#define MV_SCSI_DIRECT_ACCESS_DEVICE    0x00

typedef struct _MV_Sense_Data
{
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
}MV_Sense_Data, *PMV_Sense_Data;

MV_VOID MV_SetSenseData(
	IN PMV_Sense_Data pSense,
	IN MV_U8 SenseKey,
    IN MV_U8 AdditionalSenseCode,
	IN MV_U8 ASCQ
	);

#endif

