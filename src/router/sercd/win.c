/*
 * sercd Windows support
 * Copyright 2008 Peter Åstrand <astrand@cendio.se> for Cendio AB
 * see file COPYING for license details
 */

#ifdef WIN32
#include "sercd.h"
#include "win.h"

#include <stdio.h>
#include <assert.h>
#include <errno.h>

extern Boolean BreakSignaled;

extern int MaxLogLevel;

/* Initial serial port settings */
static DCB *InitialPortSettings;
static DCB initialportsettings;

static HANDLE SocketEvent = INVALID_HANDLE_VALUE;
static BOOL SocketWritable = FALSE;

static HANDLE DeviceEvent = INVALID_HANDLE_VALUE;
static OVERLAPPED *DeviceOverlapped = NULL;
static OVERLAPPED DeviceOverlapped_struct = { 0 };
static DWORD DeviceCommEvents;
static BOOL DeviceWritable = TRUE;
static DWORD DeviceReadChars = 0;
static BOOL DeviceModemEvents = TRUE;


/* Wrapper for GetCommState which logs errors */
static BOOL
SercdGetCommState(HANDLE hFile, DCB * dcb)
{
    char LogStr[TmpStrLen];

    if (GetCommState(hFile, dcb)) {
	return TRUE;
    }
    else {
	snprintf(LogStr, sizeof(LogStr), "GetCommState failed with error 0x%lx", GetLastError());
	LogStr[sizeof(LogStr) - 1] = '\0';
	LogMsg(LOG_ERR, LogStr);
	return FALSE;
    }
}

/* Wrapper for GetCommModemStatus which logs errors */
static BOOL
SercdGetCommModemStatus(HANDLE hFile, DWORD * lpModemStat)
{
    char LogStr[TmpStrLen];

    if (GetCommModemStatus(hFile, lpModemStat)) {
	return TRUE;
    }
    else {
	snprintf(LogStr, sizeof(LogStr), "GetCommModemStatus failed with error 0x%lx",
		 GetLastError());
	LogStr[sizeof(LogStr) - 1] = '\0';
	LogMsg(LOG_ERR, LogStr);
	return FALSE;
    }
}

/* Convert DCB parity to tncom parity */
static unsigned char
DCB2TncomParity(DCB * dcb)
{
    switch (dcb->Parity) {
    case NOPARITY:
	return TNCOM_NOPARITY;
    case ODDPARITY:
	return TNCOM_ODDPARITY;
    case EVENPARITY:
	return TNCOM_EVENPARITY;
    case MARKPARITY:
	return TNCOM_MARKPARITY;
    case SPACEPARITY:
	return TNCOM_SPACEPARITY;
    default:
	return TNCOM_NOPARITY;
    }
}

/* Convert DCB stop size to tncom size */
static unsigned char
DCB2TncomStopSize(DCB * dcb)
{
    switch (dcb->StopBits) {
    case ONESTOPBIT:
	return TNCOM_ONESTOPBIT;
    case ONE5STOPBITS:
	return TNCOM_ONE5STOPBITS;
    case TWOSTOPBITS:
	return TNCOM_TWOSTOPBITS;
    default:
	return TNCOM_ONESTOPBIT;
    }
}

/* Convert DCB output flow to tncom flow*/
static unsigned char
DCB2TncomOutFlow(DCB * dcb)
{
    if (dcb->fOutX)
	return TNCOM_CMD_FLOW_XONXOFF;
    if (dcb->fOutxCtsFlow)
	return TNCOM_CMD_FLOW_HARDWARE;
    if (dcb->fOutxDsrFlow)
	return TNCOM_CMD_FLOW_DSR;
    /* No support for DCD/RLSD flow control */
    return TNCOM_CMD_FLOW_NONE;
}

/* Convert DCB input flow to tncom flow*/
static unsigned char
DCB2TncomInFlow(DCB * dcb)
{
    if (dcb->fInX)
	return TNCOM_CMD_INFLOW_XONXOFF;
    if (dcb->fOutxCtsFlow)
	return TNCOM_CMD_INFLOW_HARDWARE;
    return TNCOM_CMD_INFLOW_NONE;
}

static void
WinLogPortSettings(DCB * dcb)
{
    unsigned char parity;
    unsigned char stopsize;
    unsigned char outflow, inflow;

    parity = DCB2TncomParity(dcb);
    stopsize = DCB2TncomStopSize(dcb);
    outflow = DCB2TncomOutFlow(dcb);
    inflow = DCB2TncomInFlow(dcb);

    LogPortSettings(dcb->BaudRate, dcb->ByteSize, parity, stopsize, outflow, inflow);
}

/* Retrieves the port speed from PortFd */
unsigned long int
GetPortSpeed(PORTHANDLE PortFd)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);
    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return 0;
    }
    return PortSettings.BaudRate;
}

/* Retrieves the data size from PortFd */
unsigned char
GetPortDataSize(PORTHANDLE PortFd)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);
    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return 0;
    }
    return PortSettings.ByteSize;
}

/* Retrieves the parity settings from PortFd */
unsigned char
GetPortParity(PORTHANDLE PortFd)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);
    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return 0;
    }

    return DCB2TncomParity(&PortSettings);
}

/* Retrieves the stop bits size from PortFd */
unsigned char
GetPortStopSize(PORTHANDLE PortFd)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);
    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return 0;
    }

    return DCB2TncomStopSize(&PortSettings);
}

/* Retrieves the flow control status, including DTR and RTS status,
from PortFd */
unsigned char
GetPortFlowControl(PORTHANDLE PortFd, unsigned char Which)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);
    DWORD MLines;

    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return 0;
    }

    if (!SercdGetCommModemStatus(PortFd, &MLines)) {
	return 0;
    }

    /* Check wich kind of information is requested */
    switch (Which) {
	/* BREAK State  */
    case TNCOM_CMD_BREAK_REQ:
	if (BreakSignaled == True)
	    return TNCOM_CMD_BREAK_ON;
	else
	    return TNCOM_CMD_BREAK_OFF;
	break;

	/* DTR Signal State */
    case TNCOM_CMD_DTR_REQ:
	/* See comment in unix.c */
	if (MLines & MS_DSR_ON)
	    return TNCOM_CMD_DTR_ON;
	else
	    return TNCOM_CMD_DTR_OFF;
	break;

	/* RTS Signal State */
    case TNCOM_CMD_RTS_REQ:
	/* See comment in unix.c */
	if (MLines & MS_CTS_ON)
	    return TNCOM_CMD_RTS_ON;
	else
	    return TNCOM_CMD_RTS_OFF;
	break;

	/* Com Port Flow Control Setting (inbound) */
    case TNCOM_CMD_INFLOW_REQ:
	return DCB2TncomInFlow(&PortSettings);
	break;

	/* Com Port Flow Control Setting (outbound/both) */
    case TNCOM_CMD_FLOW_REQ:
    default:
	return DCB2TncomOutFlow(&PortSettings);
	break;
    }
}

/* Return the status of the modem control lines (DCD, CTS, DSR, RNG) */
unsigned char
GetModemState(PORTHANDLE PortFd, unsigned char PMState)
{
    DWORD MLines;
    unsigned char MState = (unsigned char) 0;

    if (!SercdGetCommModemStatus(PortFd, &MLines)) {
	return 0;
    }

    if ((MLines & MS_RLSD_ON) != 0)
	MState += TNCOM_MODMASK_RLSD;
    if ((MLines & MS_RING_ON) != 0)
	MState += TNCOM_MODMASK_RING;
    if ((MLines & MS_DSR_ON) != 0)
	MState += TNCOM_MODMASK_DSR;
    if ((MLines & MS_CTS_ON) != 0)
	MState += TNCOM_MODMASK_CTS;
    if ((MState & TNCOM_MODMASK_RLSD) != (PMState & TNCOM_MODMASK_RLSD))
	MState += TNCOM_MODMASK_RLSD_DELTA;
    if ((MState & TNCOM_MODMASK_RING) != (PMState & TNCOM_MODMASK_RING))
	MState += TNCOM_MODMASK_RING_TRAIL;
    if ((MState & TNCOM_MODMASK_DSR) != (PMState & TNCOM_MODMASK_DSR))
	MState += TNCOM_MODMASK_DSR_DELTA;
    if ((MState & TNCOM_MODMASK_CTS) != (PMState & TNCOM_MODMASK_CTS))
	MState += TNCOM_MODMASK_CTS_DELTA;

    return (MState);
}

/* Set the serial port data size */
void
SetPortDataSize(PORTHANDLE PortFd, unsigned char DataSize)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);

    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return;
    }

    PortSettings.ByteSize = DataSize;

    if (!SetCommState(PortFd, &PortSettings)) {
	LogMsg(LOG_NOTICE, "SetPortDataSize: Unable to configure port.");
    }
    WinLogPortSettings(&PortSettings);
}

/* Set the serial port parity */
void
SetPortParity(PORTHANDLE PortFd, unsigned char Parity)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);

    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return;
    }

    switch (Parity) {
    case TNCOM_ODDPARITY:
	PortSettings.Parity = ODDPARITY;
	break;

    case TNCOM_EVENPARITY:
	PortSettings.Parity = EVENPARITY;
	break;

    case TNCOM_MARKPARITY:
	PortSettings.Parity = MARKPARITY;
	break;

    case TNCOM_SPACEPARITY:
	PortSettings.Parity = SPACEPARITY;
	break;

    case TNCOM_NOPARITY:
    default:
	PortSettings.Parity = NOPARITY;
	break;
    }

    if (!SetCommState(PortFd, &PortSettings)) {
	LogMsg(LOG_NOTICE, "SetPortDataSize: Unable to configure port.");
    }
    WinLogPortSettings(&PortSettings);
}

/* Set the serial port stop bits size */
void
SetPortStopSize(PORTHANDLE PortFd, unsigned char StopSize)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);

    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return;
    }

    switch (StopSize) {
    case TNCOM_TWOSTOPBITS:
	PortSettings.StopBits = TWOSTOPBITS;
	break;
    case TNCOM_ONE5STOPBITS:
	PortSettings.StopBits = ONE5STOPBITS;
	break;
    case TNCOM_ONESTOPBIT:
    default:
	PortSettings.StopBits = ONESTOPBIT;
	break;
    }

    if (!SetCommState(PortFd, &PortSettings)) {
	LogMsg(LOG_NOTICE, "SetPortStopSize: Unable to configure port.");
    }
    WinLogPortSettings(&PortSettings);
}

/* Set the port flow control and DTR and RTS status */
void
SetPortFlowControl(PORTHANDLE PortFd, unsigned char How)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);

    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return;
    }

    /* Check which settings to change */
    switch (How) {
	/* No Flow Control (outbound/both) */
    case TNCOM_CMD_FLOW_NONE:
	PortSettings.fOutxCtsFlow = FALSE;
	PortSettings.fOutxDsrFlow = FALSE;
	PortSettings.fRtsControl = RTS_CONTROL_DISABLE;
	PortSettings.fOutX = FALSE;
	PortSettings.fInX = FALSE;
	break;
	/* XON/XOFF Flow Control (outbound/both) */
    case TNCOM_CMD_FLOW_XONXOFF:
	PortSettings.fOutxCtsFlow = FALSE;
	PortSettings.fOutxDsrFlow = FALSE;
	PortSettings.fRtsControl = RTS_CONTROL_DISABLE;
	PortSettings.fOutX = TRUE;
	PortSettings.fInX = TRUE;
	break;
	/* HARDWARE Flow Control (outbound/both) */
    case TNCOM_CMD_FLOW_HARDWARE:
	PortSettings.fOutxCtsFlow = TRUE;
	PortSettings.fOutxDsrFlow = FALSE;
	PortSettings.fRtsControl = RTS_CONTROL_HANDSHAKE;
	PortSettings.fOutX = TRUE;
	PortSettings.fInX = TRUE;
	break;

	/* DTR Signal State ON */
    case TNCOM_CMD_DTR_ON:
	PortSettings.fDtrControl = DTR_CONTROL_ENABLE;
	break;
	/* DTR Signal State OFF */
    case TNCOM_CMD_DTR_OFF:
	PortSettings.fDtrControl = DTR_CONTROL_DISABLE;
	break;
	/* RTS Signal State ON */
    case TNCOM_CMD_RTS_ON:
	PortSettings.fRtsControl = RTS_CONTROL_ENABLE;
	break;
	/* RTS Signal State OFF */
    case TNCOM_CMD_RTS_OFF:
	PortSettings.fRtsControl = RTS_CONTROL_DISABLE;
	break;

	/* INBOUND FLOW CONTROL is ignored */
    case TNCOM_CMD_INFLOW_NONE:
    case TNCOM_CMD_INFLOW_XONXOFF:
    case TNCOM_CMD_INFLOW_HARDWARE:
	LogMsg(LOG_WARNING, "Inbound flow control ignored.");
	break;

    case TNCOM_CMD_FLOW_DCD:
	LogMsg(LOG_WARNING, "DCD Flow Control ignored.");
	break;

    case TNCOM_CMD_INFLOW_DTR:
	LogMsg(LOG_WARNING, "DTR Flow Control ignored.");
	break;

    case TNCOM_CMD_FLOW_DSR:
	LogMsg(LOG_WARNING, "DSR Flow Control ignored.");
	break;

    default:
	LogMsg(LOG_WARNING, "Requested invalid flow control.");
	break;
    }

    if (!SetCommState(PortFd, &PortSettings)) {
	LogMsg(LOG_NOTICE, "SetPortFlowControl: Unable to configure port.");
    }
    WinLogPortSettings(&PortSettings);
}

/* Set the serial port speed */
void
SetPortSpeed(PORTHANDLE PortFd, unsigned long BaudRate)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);

    if (!SercdGetCommState(PortFd, &PortSettings)) {
	return;
    }

    PortSettings.BaudRate = BaudRate;

    if (!SetCommState(PortFd, &PortSettings)) {
	LogMsg(LOG_NOTICE, "SetPortSpeed: Unable to configure port.");
    }
    WinLogPortSettings(&PortSettings);
}

void
SetBreak(PORTHANDLE PortFd, Boolean on)
{
    if (on) {
	if (!SetCommBreak(PortFd)) {
	    LogMsg(LOG_NOTICE, "SetBreak:SetCommBreak failed.");
	}
    }
    else {
	if (!ClearCommBreak(PortFd)) {
	    LogMsg(LOG_NOTICE, "SetBreak:ClearCommBreak failed.");
	}
    }
}

void
SetFlush(PORTHANDLE PortFd, int selector)
{
    DWORD flags;

    switch (selector) {
	/* Inbound flush */
    case TNCOM_PURGE_RX:
	flags = PURGE_RXCLEAR;
	break;
	/* Outbound flush */
    case TNCOM_PURGE_TX:
	flags = PURGE_TXCLEAR;
	break;
	/* Inbound/outbound flush */
    case TNCOM_PURGE_BOTH:
    default:
	flags = PURGE_RXCLEAR | PURGE_TXCLEAR;
	break;
    }

    if (!PurgeComm(PortFd, flags)) {
	LogMsg(LOG_NOTICE, "SetFlush:PurgeComm failed.");
    }
}

void
PlatformInit()
{
    WORD winsock_ver;
    WSADATA wsadata;

    /* init winsock */
    winsock_ver = MAKEWORD(2, 2);
    if (WSAStartup(winsock_ver, &wsadata)) {
	fprintf(stderr, "Unable to initialise WinSock\n");
	exit(1);
    }
    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
	fprintf(stderr, "WinSock version is incompatible with 2.2\n");
	WSACleanup();
	exit(1);
    }
}

/* Some day, we might want to support logging to Windows event log */
void
LogMsg(int LogLevel, const char *const Msg)
{
    if (LogLevel <= MaxLogLevel) {
	fprintf(stderr, "%s\n", Msg);
	fflush(stderr);
    }
}

int
OpenPort(const char *DeviceName, const char *LockFileName, PORTHANDLE * PortFd)
{
    DCB PortSettings;
    PortSettings.DCBlength = sizeof(DCB);
    char LogStr[TmpStrLen];

    *PortFd = CreateFile(DeviceName, GENERIC_READ | GENERIC_WRITE,
			 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (*PortFd == INVALID_HANDLE_VALUE) {
	return Error;
    }

    /* Get the actual port settings */
    InitialPortSettings = &initialportsettings;
    InitialPortSettings->DCBlength = sizeof(DCB);
    GetCommState(*PortFd, InitialPortSettings);
    WinLogPortSettings(InitialPortSettings);

    /* We set all parameters that cannot be set through the
       network. */
    GetCommState(*PortFd, &PortSettings);
    PortSettings.fBinary = TRUE;
    PortSettings.fParity = FALSE;
    PortSettings.fDsrSensitivity = FALSE;
    PortSettings.fTXContinueOnXoff = TRUE;
    PortSettings.fErrorChar = FALSE;
    PortSettings.fNull = FALSE;
    PortSettings.fAbortOnError = FALSE;
    PortSettings.XonLim = 2048;
    PortSettings.XoffLim = 512;
    PortSettings.XonChar = 17;
    PortSettings.XoffChar = 19;
    PortSettings.ErrorChar = 0;
    PortSettings.EofChar = 0;
    PortSettings.EvtChar = 0;

    /* Write the port settings to device */
    if (!SetCommState(*PortFd, &PortSettings)) {
	snprintf(LogStr, sizeof(LogStr), "Unable to configure port %s", DeviceName);
	LogStr[sizeof(LogStr) - 1] = '\0';
	LogMsg(LOG_NOTICE, LogStr);
	return Error;
    }

    /* Set event mask */
    if (!SetCommMask(*PortFd,
		     EV_BREAK | EV_CTS | EV_DSR | EV_RING | EV_RLSD | EV_RXCHAR | EV_TXEMPTY)) {
	snprintf(LogStr, sizeof(LogStr), "SetCommMask failed.");
	LogStr[sizeof(LogStr) - 1] = '\0';
	LogMsg(LOG_NOTICE, LogStr);
	return Error;
    }

    /* Event destroyed by ClosePort */
    DeviceEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    return NoError;
}

void
ClosePort(PORTHANDLE PortFd, const char *LockFileName)
{
    /* Cancel pending WaitCommEvent */
    if (DeviceOverlapped) {
	DWORD undefined;
	if (!SetCommMask(PortFd, 0)) {
	    LogMsg(LOG_CRIT, "DropConnection:SetCommMask failed");
	    /* The overlapped WaitCommEvent might still be
	       running. We cannot continue. */
	    exit(1);
	}
	/* Ignore errors */
	GetOverlappedResult(PortFd, DeviceOverlapped, &undefined, FALSE);
	DeviceOverlapped = NULL;
    }
    CloseHandle(DeviceEvent);
    DeviceEvent = INVALID_HANDLE_VALUE;

    /* Restores initial port settings */
    if (InitialPortSettings)
	SetCommState(PortFd, InitialPortSettings);

    /* Closes the device */
    CloseHandle(PortFd);
}

int
SercdSelect(PORTHANDLE * DeviceIn, PORTHANDLE * DeviceOut, PORTHANDLE * ModemState,
	    SERCD_SOCKET * SocketOut, SERCD_SOCKET * SocketIn,
	    SERCD_SOCKET * SocketListen, long PollInterval)
{
    DWORD waitret, objects = 0;
    HANDLE ghEvents[2];
    int ret = 0;
    int SocketListenIndex = -1;
    int DeviceIndex = -1;
    WSANETWORKEVENTS events;
    BOOL waitcomm;
    PORTHANDLE *Device = NULL;
    SERCD_SOCKET *Socket = NULL;
    char LogStr[TmpStrLen];

    if (DeviceIn && DeviceOut) {
	assert(DeviceIn == DeviceOut);
    }
    if (DeviceOut && ModemState) {
	assert(DeviceOut == ModemState);
    }
    if (DeviceIn && ModemState) {
	assert(DeviceIn == ModemState);
    }
    if (DeviceIn) {
	Device = DeviceIn;
    }
    if (DeviceOut) {
	Device = DeviceOut;
    }
    if (ModemState) {
	Device = ModemState;
    }

    if (SocketOut && SocketIn) {
	assert(SocketOut == SocketIn);
    }
    if (SocketOut) {
	Socket = SocketOut;
    }
    if (SocketIn) {
	Socket = SocketIn;
    }


    if (Device) {
	if (!DeviceOverlapped) {
	    /* Fire overlapped WaitCommEvent */
	    DeviceOverlapped = &DeviceOverlapped_struct;
	    DeviceOverlapped->hEvent = DeviceEvent;
	    waitcomm = WaitCommEvent(*Device, &DeviceCommEvents, DeviceOverlapped);
	    if (waitcomm) {
		/* Immediate result. */
		DeviceOverlapped = NULL;
		/* Both EV_RXCHAR and EV_TXEMPTY are edge-triggered */
		if (DeviceCommEvents & EV_RXCHAR) {
		    DWORD ComErrors;
		    COMSTAT Stat;
		    if (!ClearCommError(*Device, &ComErrors, &Stat)) {
			snprintf(LogStr, sizeof(LogStr), "Communications Error: 0x%lx\n",
				 ComErrors);
			LogStr[sizeof(LogStr) - 1] = '\0';
			LogMsg(LOG_INFO, LogStr);
		    }
		    DeviceReadChars = Stat.cbInQue;
		}
		if (DeviceCommEvents & EV_TXEMPTY) {
		    DeviceWritable = TRUE;
		}
		if (DeviceCommEvents & (EV_CTS | EV_DSR | EV_RING | EV_RLSD)) {
		    DeviceModemEvents = TRUE;
		}
	    }
	    else if (GetLastError() != ERROR_IO_PENDING) {
		errno = EINVAL;
		return -1;
	    }
	}
	if (DeviceOverlapped) {
	    /* Pending WaitCommEvent */
	    DeviceIndex = objects++;
	    ghEvents[DeviceIndex] = DeviceEvent;
	}
    }

    if (SocketListen) {
	SocketListenIndex = objects++;
	ghEvents[SocketListenIndex] = SocketEvent;
    }

    /* Need to wait? */
    if ((SocketOut && SocketWritable) || (DeviceOut && DeviceWritable) ||
	(DeviceIn && DeviceReadChars) || (ModemState && DeviceModemEvents)) {
	PollInterval = 0;
    }

    waitret = WaitForMultipleObjects(objects, ghEvents, FALSE, PollInterval);
    switch (waitret) {
    case WAIT_OBJECT_0 + 0:
    case WAIT_OBJECT_0 + 1:
    case WAIT_TIMEOUT:
	break;

    default:
	errno = EINVAL;
	return -1;
	break;
    }

    if (waitret == WAIT_OBJECT_0 + DeviceIndex) {
	DWORD undefined;
	waitcomm = GetOverlappedResult(*Device, DeviceOverlapped, &undefined, TRUE);
	DeviceOverlapped = NULL;
	if (!waitcomm) {
	    errno = EINVAL;
	    return -1;
	}
	if (DeviceCommEvents & EV_RXCHAR) {
	    DWORD ComErrors;
	    COMSTAT Stat;
	    if (!ClearCommError(*Device, &ComErrors, &Stat)) {
		snprintf(LogStr, sizeof(LogStr), "Communications Error: 0x%lx\n", ComErrors);
		LogStr[sizeof(LogStr) - 1] = '\0';
		LogMsg(LOG_INFO, LogStr);
	    }
	    DeviceReadChars = Stat.cbInQue;
	}
	if (DeviceCommEvents & EV_TXEMPTY) {
	    DeviceWritable = TRUE;
	}
	if (DeviceCommEvents & (EV_CTS | EV_DSR | EV_RING | EV_RLSD)) {
	    DeviceModemEvents = TRUE;
	}
    }

    if (waitret == WAIT_OBJECT_0 + SocketListenIndex) {
	/* Check events on the listening socket */
	if (WSAEnumNetworkEvents(*SocketListen, NULL, &events)) {
	    /* error */
	    errno = EINVAL;
	    return -1;
	}

	/* Level-triggered */
	if (events.lNetworkEvents & FD_ACCEPT) {
	    ret |= SERCD_EV_SOCKETCONNECT;
	}

	/* Check events on the connection socket. Reset event. */
	if (Socket && WSAEnumNetworkEvents(*Socket, SocketEvent, &events)) {
	    /* error */
	    errno = EINVAL;
	    return -1;
	}

	/* Level-triggered */
	if (SocketIn && events.lNetworkEvents & FD_READ) {
	    ret |= SERCD_EV_SOCKETIN;
	}
	/* Level-triggered */
	if (Socket && events.lNetworkEvents & FD_CLOSE) {
	    ret |= SERCD_EV_SOCKETIN;
	}

	/* Edge-triggered, sort of */
	if (events.lNetworkEvents & FD_WRITE) {
	    SocketWritable = TRUE;
	}
    }

    if (DeviceIn && DeviceReadChars) {
	ret |= SERCD_EV_DEVICEIN;
    }
    if (DeviceOut && DeviceWritable) {
	ret |= SERCD_EV_DEVICEOUT;
    }
    if (ModemState && DeviceModemEvents) {
	ret |= SERCD_EV_MODEMSTATE;
    }
    if (SocketOut && SocketWritable) {
	ret |= SERCD_EV_SOCKETOUT;
    }

    return ret;
}


void
NewListener(SERCD_SOCKET LSocketFd)
{
    /* We are keeping the event until exit. This event is shared by
       the listening socket and the client connection. */
    SocketEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    WSAEventSelect(LSocketFd, SocketEvent, FD_ACCEPT | FD_WRITE | FD_READ | FD_CLOSE);
}


/* Drop client connection and close serial port */
void
DropConnection(PORTHANDLE * DeviceFd, SERCD_SOCKET * InSocketFd, SERCD_SOCKET * OutSocketFd,
	       const char *LockFileName)
{
    if (DeviceFd) {
	ClosePort(*DeviceFd, LockFileName);
    }

    if (InSocketFd) {
	closesocket(*InSocketFd);
    }

    if (OutSocketFd) {
	closesocket(*OutSocketFd);
    }
}

/* Workaround for the fact that blocking ReadFile is not possible if
   the file is opened as overlapped. Stupid Windows. */
static BOOL
ReadFileOverlapped(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
		   LPDWORD lpNumberOfBytesRead)
{
    BOOL ret;
    OVERLAPPED ov;
    static HANDLE ev = INVALID_HANDLE_VALUE;

    if (ev == INVALID_HANDLE_VALUE) {
	ev = CreateEvent(NULL, TRUE, FALSE, NULL);
	assert(ev);
    }

    ov.Internal = 0;
    ov.InternalHigh = 0;
    ov.Offset = 0;
    ov.OffsetHigh = 0;
    ov.hEvent = ev;

    ret = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, &ov);
    if (!ret && GetLastError() == ERROR_IO_PENDING) {
	ret = GetOverlappedResult(hFile, &ov, lpNumberOfBytesRead, TRUE);
    }
    return ret;
}


/* Workaround for the fact that blocking WriteFile is not possible if
   the file is opened as overlapped. Stupid Windows. */
static BOOL
WriteFileOverlapped(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
		    LPDWORD lpNumberOfBytesWritten)
{
    BOOL ret;
    OVERLAPPED ov;
    static HANDLE ev = INVALID_HANDLE_VALUE;

    if (ev == INVALID_HANDLE_VALUE) {
	ev = CreateEvent(NULL, TRUE, FALSE, NULL);
	assert(ev);
    }

    ov.Internal = 0;
    ov.InternalHigh = 0;
    ov.Offset = 0;
    ov.OffsetHigh = 0;
    ov.hEvent = ev;

    ret = WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, &ov);
    if (!ret && GetLastError() == ERROR_IO_PENDING) {
	ret = GetOverlappedResult(hFile, &ov, lpNumberOfBytesWritten, TRUE);
    }
    return ret;
}


ssize_t
WriteToDev(PORTHANDLE port, const void *buf, size_t count)
{
    ssize_t iobytes;
    if (!WriteFileOverlapped(port, buf, count, &iobytes)) {
	iobytes = -1;
    }

    if (iobytes > 0) {
	DeviceWritable = FALSE;
    }
    return iobytes;
}

ssize_t
ReadFromDev(PORTHANDLE port, void *buf, size_t count)
{
    ssize_t iobytes;

    count = MIN(count, DeviceReadChars);

    if (!ReadFileOverlapped(port, buf, count, &iobytes)) {
	iobytes = -1;
    }

    char *charbuf;
    charbuf = buf;
    charbuf[iobytes] = '\0';

    if (iobytes > 0) {
	assert(iobytes <= DeviceReadChars);
	DeviceReadChars -= iobytes;
    }

    return iobytes;
}

ssize_t
WriteToNet(SERCD_SOCKET sock, const void *buf, size_t count)
{
    ssize_t iobytes;
    iobytes = send(sock, buf, count, 0);
    if (iobytes < 0 && WSAGetLastError() == WSAEWOULDBLOCK) {
	SocketWritable = FALSE;
    }
    return iobytes;
}

ssize_t
ReadFromNet(SERCD_SOCKET sock, void *buf, size_t count)
{
    return recv(sock, buf, count, 0);
}

void
ModemStateNotified()
{
    DeviceModemEvents = FALSE;
}

#endif /* WIN32 */
