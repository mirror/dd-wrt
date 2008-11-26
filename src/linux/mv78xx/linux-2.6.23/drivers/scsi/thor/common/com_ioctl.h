#if !defined(COM_IOCTL_H)
#define COM_IOCTL_H

#if defined (_OS_WINDOWS)
#include <ntddscsi.h>
#elif defined(_OS_LINUX)

#endif

/* private IOCTL commands */
#define MV_IOCTL_CHECK_DRIVER \
	    CTL_CODE( FILE_DEVICE_CONTROLLER, \
				  0x900, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)	

/* IOCTL signature */
#define MV_IOCTL_DRIVER_SIGNATURE			"mv61xxsg"
#define MV_IOCTL_DRIVER_SIGNATURE_LENGTH	8

/* IOCTL command status */
#define IOCTL_STATUS_SUCCESS				0
#define IOCTL_STATUS_INVALID_REQUEST		1
#define IOCTL_STATUS_ERROR					2

#ifndef _OS_BIOS
#pragma pack(8)
#endif

typedef struct _MV_IOCTL_BUFFER
{
	SRB_IO_CONTROL Srb_Ctrl;
	MV_U8 Data_Buffer[32];
} MV_IOCTL_BUFFER, *PMV_IOCTL_BUFFER;

#ifndef _OS_BIOS
#pragma pack()
#endif

#endif

