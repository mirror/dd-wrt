/****************************************************************************
*
*	Name:			OsDefs.h
*
*	Description:	
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 7/09/02 9:37a $
****************************************************************************/

#ifndef _OSDEFS_H_
#define _OSDEFS_H_

extern int		errno;
typedef long int				STATUS;

#ifdef INT
#undef INT
#endif

typedef int						INT;
typedef unsigned int			UINT;
typedef unsigned char			UCHAR;
typedef unsigned short			USHORT;
typedef unsigned long			ULONG;
typedef unsigned long *			PULONG;
typedef void					VOID;
typedef void *					PVOID;

typedef unsigned short			WORD;
typedef unsigned long			DWORD;
typedef unsigned char			BYTE;
typedef signed char				CHAR;
typedef signed short int		SHORT;
typedef signed long int			LONG;
typedef long long int			LONGLONG;
typedef unsigned long long int	ULONGLONG;
typedef long long int			LARGE_INTEGER;
typedef unsigned char *			PUCHAR;

typedef UINT					BOOL;
typedef UINT					BOOLEAN;
typedef BOOLEAN *				PBOOLEAN;

typedef void *					PFILE_OBJECT;

typedef struct					sk_buff SK_BUFF_T;
typedef struct					sk_buff_head SK_BUFF_QUEUE_T;
typedef	struct					atm_vcc ATM_VCC_T;


#define KdPrint( x )		printk x

#define TRUE				1
#define FALSE				0

// #define LONG_MIN	(-LONG_MAX - 1L)
#ifndef INT_MIN
#define INT_MIN				(-INT_MAX - 1)
#endif


#define IN
#define OUT
#define INOUT
#define I_O
#define GLOBAL

#define ASSERT( X )
#define ASSERTMSG( X, Y )

//#define NDIS_STRING_CONST( x )	x
//typedef char *					NDIS_STRING;
//typedef char *					PNDIS_STRING;
//typedef DWORD					NDIS_PARAMETER_TYPE;
//#define	NdisParameterInteger	1
//#define	NdisParameterString		2

enum ConnectStatus
{
    MediaNotConnected,
    MediaConnected
};

#define TRAP()					asm("int3")

//#define NdisWriteErrorLogEntry( Handle, Code, NumValues... )



/////////////////////////////////////////////////////////////////////
// Error Codes
/////////////////////////////////////////////////////////////////////
#define EFAIL				1024
#define ERR_NOFLOAT			-2048
#define ERR_FORMAT			-2049
#define ERR_SHUTDOWN		-2050

#define	EALREADYLOADED		(-STATUS_IMAGE_ALREADY_LOADED)				/* device already loaded */
#define	EINVALIDTYPE		(-STATUS_BAD_MASTER_BOOT_RECORD)			/* invalid record type */
#define	ETIMEOUT			(-STATUS_IO_TIMEOUT)						/* command timeout */
#define	ENOTREADY			(-STATUS_DEVICE_NOT_READY)					/* device not ready */
#define	EBADLENGTH			(-STATUS_INVALID_BLOCK_LENGTH)				/* invalid length */
#define EDEVICEFAIL			(-STATUS_IO_DEVICE_ERROR)					/* requested IO failed */
#define ENOINTERRUPT		(-STATUS_BIOS_FAILED_TO_CONNECT_INTERRUPT)	/* requested interrupt unavailable */
#define ESVCINVAL			(-STATUS_INVALID_DEVICE_REQUEST)			/* invalid service */
#define COMMAND_PENDING		(-STATUS_PENDING)							/* command is pending */
#define STATUS_UNSUPPORTED	(-STATUS_NOT_SUPPORTED)						/* command is not supported */
//#define STATUS_INVALID_PARAMETER	(-STATUS_INVALID_PARAMETER)			/*invalid parameter passed in to function */


#define STATUS_SUCCESS							((STATUS)0x00000000L)	
#define STATUS_TIMEOUT							((STATUS)0x00000102L)	
#define STATUS_PENDING							((STATUS)0x00000103L)	
#define STATUS_UNSUCCESSFUL						((STATUS)0xC0000001L)
#define STATUS_FAILURE							((STATUS)0xC0000001L)
#define STATUS_NOT_IMPLEMENTED						((STATUS)0xC0000002L)
#define STATUS_INVALID_PARAMETER					((STATUS)0xC000000DL)

#endif //ifndef _OSDEFS_H_










