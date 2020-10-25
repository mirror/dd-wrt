#ifndef SG_PT_WIN32_H
#define SG_PT_WIN32_H
/*
 * The information in this file was obtained from scsi-wnt.h by
 * Richard Stemmer, rs@epost.de . He in turn gives credit to
 * Jay A. Key (for scsipt.c).
 * The plscsi program (by Pat LaVarre <p.lavarre@ieee.org>) has
 * also been used as a reference.
 * Much of the information in this header can also be obtained
 * from msdn.microsoft.com .
 * Updated for cygwin version 1.7.17 changes 20121026
 */

/* WIN32_LEAN_AND_MEAN may be required to prevent inclusion of <winioctl.h> */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCSI_MAX_SENSE_LEN 64
#define SCSI_MAX_CDB_LEN 16
#define SCSI_MAX_INDIRECT_DATA 16384

typedef struct {
    USHORT          Length;
    UCHAR           ScsiStatus;
    UCHAR           PathId;
    UCHAR           TargetId;
    UCHAR           Lun;
    UCHAR           CdbLength;
    UCHAR           SenseInfoLength;
    UCHAR           DataIn;
    ULONG           DataTransferLength;
    ULONG           TimeOutValue;
    ULONG_PTR       DataBufferOffset;  /* was ULONG; problem in 64 bit */
    ULONG           SenseInfoOffset;
    UCHAR           Cdb[SCSI_MAX_CDB_LEN];
} SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;


typedef struct {
    USHORT          Length;
    UCHAR           ScsiStatus;
    UCHAR           PathId;
    UCHAR           TargetId;
    UCHAR           Lun;
    UCHAR           CdbLength;
    UCHAR           SenseInfoLength;
    UCHAR           DataIn;
    ULONG           DataTransferLength;
    ULONG           TimeOutValue;
    PVOID           DataBuffer;
    ULONG           SenseInfoOffset;
    UCHAR           Cdb[SCSI_MAX_CDB_LEN];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;


typedef struct {
    SCSI_PASS_THROUGH spt;
    /* plscsi shows a follow on 16 bytes allowing 32 byte cdb */
    ULONG           Filler;
    UCHAR           ucSenseBuf[SCSI_MAX_SENSE_LEN];
    UCHAR           ucDataBuf[SCSI_MAX_INDIRECT_DATA];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;


typedef struct {
    SCSI_PASS_THROUGH_DIRECT spt;
    ULONG           Filler;
    UCHAR           ucSenseBuf[SCSI_MAX_SENSE_LEN];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;



typedef struct {
    UCHAR           NumberOfLogicalUnits;
    UCHAR           InitiatorBusId;
    ULONG           InquiryDataOffset;
} SCSI_BUS_DATA, *PSCSI_BUS_DATA;


typedef struct {
    UCHAR           NumberOfBusses;
    SCSI_BUS_DATA   BusData[1];
} SCSI_ADAPTER_BUS_INFO, *PSCSI_ADAPTER_BUS_INFO;


typedef struct {
    UCHAR           PathId;
    UCHAR           TargetId;
    UCHAR           Lun;
    BOOLEAN         DeviceClaimed;
    ULONG           InquiryDataLength;
    ULONG           NextInquiryDataOffset;
    UCHAR           InquiryData[1];
} SCSI_INQUIRY_DATA, *PSCSI_INQUIRY_DATA;


typedef struct {
    ULONG           Length;
    UCHAR           PortNumber;
    UCHAR           PathId;
    UCHAR           TargetId;
    UCHAR           Lun;
} SCSI_ADDRESS, *PSCSI_ADDRESS;

/*
 * Standard IOCTL define
 */
#ifndef CTL_CODE
#define CTL_CODE(DevType, Function, Method, Access)             \
        (((DevType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

/*
 * file access values
 */
#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS         0
#endif
#ifndef FILE_READ_ACCESS
#define FILE_READ_ACCESS        0x0001
#endif
#ifndef FILE_WRITE_ACCESS
#define FILE_WRITE_ACCESS       0x0002
#endif

// IOCTL_STORAGE_QUERY_PROPERTY

#define FILE_DEVICE_MASS_STORAGE    0x0000002d
#define IOCTL_STORAGE_BASE          FILE_DEVICE_MASS_STORAGE
#define FILE_ANY_ACCESS             0

// #define METHOD_BUFFERED             0

#define IOCTL_STORAGE_QUERY_PROPERTY \
    CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)


#ifndef _DEVIOCTL_
typedef enum _STORAGE_BUS_TYPE {
    BusTypeUnknown      = 0x00,
    BusTypeScsi         = 0x01,
    BusTypeAtapi        = 0x02,
    BusTypeAta          = 0x03,
    BusType1394         = 0x04,
    BusTypeSsa          = 0x05,
    BusTypeFibre        = 0x06,
    BusTypeUsb          = 0x07,
    BusTypeRAID         = 0x08,
    BusTypeiScsi        = 0x09,
    BusTypeSas          = 0x0A,
    BusTypeSata         = 0x0B,
    BusTypeSd           = 0x0C,
    BusTypeMmc          = 0x0D,
    BusTypeVirtual             = 0xE,
    BusTypeFileBackedVirtual   = 0xF,
    BusTypeSpaces       = 0x10,
    BusTypeNvme         = 0x11,
    BusTypeSCM          = 0x12,
    BusTypeUfs          = 0x13,
    BusTypeMax          = 0x14,
    BusTypeMaxReserved  = 0x7F
} STORAGE_BUS_TYPE, *PSTORAGE_BUS_TYPE;

typedef enum _STORAGE_PROTOCOL_TYPE {
    ProtocolTypeUnknown = 0,
    ProtocolTypeScsi,
    ProtocolTypeAta,
    ProtocolTypeNvme,
    ProtocolTypeSd
} STORAGE_PROTOCOL_TYPE;

typedef enum _STORAGE_PROTOCOL_NVME_DATA_TYPE {
    NVMeDataTypeUnknown = 0,
    NVMeDataTypeIdentify,
    NVMeDataTypeLogPage,
    NVMeDataTypeFeature
} STORAGE_PROTOCOL_NVME_DATA_TYPE;

typedef struct _STORAGE_PROTOCOL_SPECIFIC_DATA {
    STORAGE_PROTOCOL_TYPE ProtocolType;
    ULONG DataType;
    ULONG ProtocolDataRequestValue;
    ULONG ProtocolDataRequestSubValue;
    ULONG ProtocolDataOffset;
    ULONG ProtocolDataLength;
    ULONG FixedProtocolReturnData;
    ULONG Reserved[3];
} STORAGE_PROTOCOL_SPECIFIC_DATA;


typedef struct _STORAGE_DEVICE_DESCRIPTOR {
    ULONG Version;
    ULONG Size;
    UCHAR DeviceType;
    UCHAR DeviceTypeModifier;
    BOOLEAN RemovableMedia;
    BOOLEAN CommandQueueing;
    ULONG VendorIdOffset;       /* 0 if not available */
    ULONG ProductIdOffset;      /* 0 if not available */
    ULONG ProductRevisionOffset;/* 0 if not available */
    ULONG SerialNumberOffset;   /* -1 if not available ?? */
    STORAGE_BUS_TYPE BusType;
    ULONG RawPropertiesLength;
    UCHAR RawDeviceProperties[1];
} STORAGE_DEVICE_DESCRIPTOR, *PSTORAGE_DEVICE_DESCRIPTOR;

#define STORAGE_PROTOCOL_STRUCTURE_VERSION 0x1

#define IOCTL_STORAGE_PROTOCOL_COMMAND \
        CTL_CODE(IOCTL_STORAGE_BASE, 0x04F0, METHOD_BUFFERED, \
                FILE_READ_ACCESS | FILE_WRITE_ACCESS)

typedef struct _STORAGE_PROTOCOL_COMMAND {
    DWORD         Version;        /* STORAGE_PROTOCOL_STRUCTURE_VERSION */
    DWORD         Length;
    STORAGE_PROTOCOL_TYPE   ProtocolType;
    DWORD         Flags;
    DWORD         ReturnStatus;
    DWORD         ErrorCode;
    DWORD         CommandLength;
    DWORD         ErrorInfoLength;
    DWORD         DataToDeviceTransferLength;
    DWORD         DataFromDeviceTransferLength;
    DWORD         TimeOutValue;
    DWORD         ErrorInfoOffset;
    DWORD         DataToDeviceBufferOffset;
    DWORD         DataFromDeviceBufferOffset;
    DWORD         CommandSpecific;
    DWORD         Reserved0;
    DWORD         FixedProtocolReturnData;
    DWORD         Reserved1[3];
    BYTE          Command[1];     /* has CommandLength elements */
} STORAGE_PROTOCOL_COMMAND, *PSTORAGE_PROTOCOL_COMMAND;

#endif          /* _DEVIOCTL_ */

typedef struct _STORAGE_DEVICE_UNIQUE_IDENTIFIER {
    ULONG  Version;
    ULONG  Size;
    ULONG  StorageDeviceIdOffset;
    ULONG  StorageDeviceOffset;
    ULONG  DriveLayoutSignatureOffset;
} STORAGE_DEVICE_UNIQUE_IDENTIFIER, *PSTORAGE_DEVICE_UNIQUE_IDENTIFIER;

// Use CompareStorageDuids(PSTORAGE_DEVICE_UNIQUE_IDENTIFIER duid1, duid2)
// to test for equality

#ifndef _DEVIOCTL_
typedef enum _STORAGE_QUERY_TYPE {
    PropertyStandardQuery = 0,
    PropertyExistsQuery,
    PropertyMaskQuery,
    PropertyQueryMaxDefined
} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

typedef enum _STORAGE_PROPERTY_ID {
    StorageDeviceProperty = 0,
    StorageAdapterProperty,
    StorageDeviceIdProperty,
    StorageDeviceUniqueIdProperty,
    StorageDeviceWriteCacheProperty,
    StorageMiniportProperty,
    StorageAccessAlignmentProperty,
    /* Identify controller goes to adapter; Identify namespace to device */
    StorageAdapterProtocolSpecificProperty = 49,
    StorageDeviceProtocolSpecificProperty = 50
} STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;

typedef struct _STORAGE_PROPERTY_QUERY {
    STORAGE_PROPERTY_ID PropertyId;
    STORAGE_QUERY_TYPE QueryType;
    UCHAR AdditionalParameters[1];
} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;

typedef struct _STORAGE_PROTOCOL_DATA_DESCRIPTOR {
    DWORD  Version;
    DWORD  Size;
    STORAGE_PROTOCOL_SPECIFIC_DATA ProtocolSpecificData;
} STORAGE_PROTOCOL_DATA_DESCRIPTOR, *PSTORAGE_PROTOCOL_DATA_DESCRIPTOR;

// Command completion status
// The "Phase Tag" field and "Status Field" are separated in spec. We define
// them in the same data structure to ease the memory access from software.
//
typedef union {
    struct {
        USHORT  P           : 1;        // Phase Tag (P)

        USHORT  SC          : 8;        // Status Code (SC)
        USHORT  SCT         : 3;        // Status Code Type (SCT)
        USHORT  Reserved    : 2;
        USHORT  M           : 1;        // More (M)
        USHORT  DNR         : 1;        // Do Not Retry (DNR)
    } DUMMYSTRUCTNAME;
    USHORT AsUshort;
} NVME_COMMAND_STATUS, *PNVME_COMMAND_STATUS;

// Information of log: NVME_LOG_PAGE_ERROR_INFO. Size: 64 bytes
//
typedef struct {
    ULONGLONG  ErrorCount;
    USHORT     SQID;           // Submission Queue ID
    USHORT     CMDID;          // Command ID
    NVME_COMMAND_STATUS Status; // Status Field: This field indicates the
                                // Status Field for the command that
                                // completed. The Status Field is located in
                                // bits 15:01, bit 00 corresponds to the Phase
                                // Tag posted for the command.
    struct {
        USHORT  Byte        : 8;   // Byte in command that contained error
        USHORT  Bit         : 3;   // Bit in command that contained error
        USHORT  Reserved    : 5;
    } ParameterErrorLocation;

    ULONGLONG  Lba;            // LBA: This field indicates the first LBA
                               // that experienced the error condition, if
                               // applicable.
    ULONG      NameSpace;      // Namespace: This field indicates the nsid
                               // that the error is associated with, if
                               // applicable.
    UCHAR      VendorInfoAvailable;    // Vendor Specific Information Available
    UCHAR      Reserved0[3];
    ULONGLONG  CommandSpecificInfo;    // This field contains command specific
                                       // information. If used, the command
                                       // definition specifies the information
                                       // returned.
    UCHAR      Reserved1[24];
} NVME_ERROR_INFO_LOG, *PNVME_ERROR_INFO_LOG;

typedef struct {

    ULONG   DW0;
    ULONG   Reserved;

    union {
        struct {
            USHORT  SQHD;               // SQ Head Pointer (SQHD)
            USHORT  SQID;               // SQ Identifier (SQID)
        } DUMMYSTRUCTNAME;

        ULONG   AsUlong;
    } DW2;

    union {
        struct {
            USHORT              CID;    // Command Identifier (CID)
            NVME_COMMAND_STATUS Status;
        } DUMMYSTRUCTNAME;

        ULONG   AsUlong;
    } DW3;

} NVME_COMPLETION_ENTRY, *PNVME_COMPLETION_ENTRY;


// Bit-mask values for STORAGE_PROTOCOL_COMMAND - "Flags" field.
//
// Flag indicates the request targeting to adapter instead of device.
#define STORAGE_PROTOCOL_COMMAND_FLAG_ADAPTER_REQUEST    0x80000000

//
// Status values for STORAGE_PROTOCOL_COMMAND - "ReturnStatus" field.
//
#define STORAGE_PROTOCOL_STATUS_PENDING                 0x0
#define STORAGE_PROTOCOL_STATUS_SUCCESS                 0x1
#define STORAGE_PROTOCOL_STATUS_ERROR                   0x2
#define STORAGE_PROTOCOL_STATUS_INVALID_REQUEST         0x3
#define STORAGE_PROTOCOL_STATUS_NO_DEVICE               0x4
#define STORAGE_PROTOCOL_STATUS_BUSY                    0x5
#define STORAGE_PROTOCOL_STATUS_DATA_OVERRUN            0x6
#define STORAGE_PROTOCOL_STATUS_INSUFFICIENT_RESOURCES  0x7

#define STORAGE_PROTOCOL_STATUS_NOT_SUPPORTED           0xFF

// Command Length for Storage Protocols.
//
// NVMe commands are always 64 bytes.
#define STORAGE_PROTOCOL_COMMAND_LENGTH_NVME            0x40

// Command Specific Information for Storage Protocols - CommandSpecific field
//
#define STORAGE_PROTOCOL_SPECIFIC_NVME_ADMIN_COMMAND    0x01
#define STORAGE_PROTOCOL_SPECIFIC_NVME_NVM_COMMAND 0x02

#endif          /* _DEVIOCTL_ */


// NVME_PASS_THROUGH

#ifndef STB_IO_CONTROL
typedef struct _SRB_IO_CONTROL {
    ULONG HeaderLength;
    UCHAR Signature[8];
    ULONG Timeout;
    ULONG ControlCode;
    ULONG ReturnCode;
    ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;
#endif

#ifndef NVME_PASS_THROUGH_SRB_IO_CODE

#define NVME_SIG_STR "NvmeMini"
#define NVME_STORPORT_DRIVER 0xe000

#define NVME_PASS_THROUGH_SRB_IO_CODE \
  CTL_CODE(NVME_STORPORT_DRIVER, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma pack(1)

/* Following is pre-Win10; used with DeviceIoControl(IOCTL_SCSI_MINIPORT),
 * in Win10 need DeviceIoControl(IOCTL_STORAGE_PROTOCOL_COMMAND) for pure
 * pass-through. Win10 also has "Protocol specific queries" for things like
 * Identify and Get feature. */
typedef struct _NVME_PASS_THROUGH_IOCTL
{
    SRB_IO_CONTROL SrbIoCtrl;
    ULONG VendorSpecific[6];
    ULONG NVMeCmd[16];      /* Command DW[0...15] */
    ULONG CplEntry[4];      /* Completion DW[0...3] */
    ULONG Direction;        /* 0=None, 1=Out, 2=In, 3=I/O */
    ULONG QueueId;          /* 0=AdminQ */
    ULONG DataBufferLen;    /* sizeof(DataBuffer) if Data In */
    ULONG MetaDataLen;
    ULONG ReturnBufferLen;  /* offsetof(DataBuffer), plus
                             * sizeof(DataBuffer) if Data Out */
    UCHAR DataBuffer[1];
} NVME_PASS_THROUGH_IOCTL;
#pragma pack()

#endif // NVME_PASS_THROUGH_SRB_IO_CODE


/*
 * method codes
 */
#define METHOD_BUFFERED         0
#define METHOD_IN_DIRECT        1
#define METHOD_OUT_DIRECT       2
#define METHOD_NEITHER          3


#define IOCTL_SCSI_BASE    0x00000004

/*
 * constants for DataIn member of SCSI_PASS_THROUGH* structures
 */
#define SCSI_IOCTL_DATA_OUT             0
#define SCSI_IOCTL_DATA_IN              1
#define SCSI_IOCTL_DATA_UNSPECIFIED     2

#define IOCTL_SCSI_PASS_THROUGH         CTL_CODE(IOCTL_SCSI_BASE, 0x0401, \
        METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_MINIPORT             CTL_CODE(IOCTL_SCSI_BASE, 0x0402, \
        METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_GET_INQUIRY_DATA     CTL_CODE(IOCTL_SCSI_BASE, 0x0403, \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_GET_CAPABILITIES     CTL_CODE(IOCTL_SCSI_BASE, 0x0404, \
        METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE(IOCTL_SCSI_BASE, 0x0405, \
        METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_SCSI_GET_ADDRESS          CTL_CODE(IOCTL_SCSI_BASE, 0x0406, \
        METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifdef __cplusplus
}
#endif

#endif          /* SG_PT_WIN32_H */
