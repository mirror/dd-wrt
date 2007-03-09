//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This program is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
//=================================================================
//
//        eCosSerial.cpp
//
//        Serial test class
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   This class abstracts the serial port for use in the testing infrastructure
// Usage:
//
//####DESCRIPTIONEND####

#include "eCosStd.h"
#include "eCosSerial.h"
#include "eCosThreadUtils.h"
#include "eCosTrace.h"

CeCosSerial::CeCosSerial():
  m_nErr(0),
  m_pHandle(0),
  m_nDataBits(8),
  m_nStopBits(ONE_STOP_BIT),
  m_bXONXOFFFlowControl(0),
  m_bRTSCTSFlowControl(0),
  m_bDSRDTRFlowControl(0),
  m_bParity(false),
  m_nBaud(0),
  m_nTotalReadTimeout(10*1000),
  m_nTotalWriteTimeout(10*1000),
  m_nInterCharReadTimeout(500),
  m_nInterCharWriteTimeout(500),
  m_bBlockingReads(true)
{
}

CeCosSerial::~CeCosSerial()
{
  Close();
}

CeCosSerial::CeCosSerial(LPCTSTR pszPort,int nBaud):
  m_nErr(0),
  m_pHandle(0),
  m_nDataBits(8),
  m_nStopBits(ONE_STOP_BIT),
  m_bXONXOFFFlowControl(0),
  m_bRTSCTSFlowControl(0),
  m_bDSRDTRFlowControl(0),
  m_bParity(false),
  m_nTotalReadTimeout(10*1000),
  m_nTotalWriteTimeout(10*1000),
  m_nInterCharReadTimeout(500),
  m_nInterCharWriteTimeout(500),
  m_bBlockingReads(true)
{
  Open(pszPort,nBaud);
}

bool CeCosSerial::SetBlockingReads(bool b,bool bApplySettingsNow/*=true*/)
{
  m_bBlockingReads=b;
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

bool CeCosSerial:: SetBaud(unsigned int nBaud,bool bApplySettingsNow/*=true*/)
{
  m_nBaud=nBaud;
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

bool CeCosSerial:: SetParity(bool bParityOn,bool bApplySettingsNow/*=true*/)
{
  m_bParity=bParityOn;
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

bool CeCosSerial:: SetDataBits(int n,bool bApplySettingsNow/*=true*/)
{
  m_nDataBits=n;
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

bool CeCosSerial:: SetStopBits(StopBitsType n,bool bApplySettingsNow/*=true*/)
{
  m_nStopBits=n;
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

bool CeCosSerial:: SetXONXOFFFlowControl(bool b,bool bApplySettingsNow/*=true*/)
{
  m_bXONXOFFFlowControl=b;
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

bool CeCosSerial:: SetRTSCTSFlowControl(bool b,bool bApplySettingsNow/*=true*/)
{
  m_bRTSCTSFlowControl=b;
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

bool CeCosSerial:: SetDSRDTRFlowControl(bool b,bool bApplySettingsNow/*=true*/)
{
  m_bDSRDTRFlowControl=b;
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

bool CeCosSerial:: SetReadTimeOuts(int nTotal,int nBetweenChars,bool bApplySettingsNow/*=true*/) // mSec
{
  m_nTotalReadTimeout=nTotal;
  m_nInterCharReadTimeout=nBetweenChars;
  
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

bool CeCosSerial:: SetWriteTimeOuts(int nTotal,int nBetweenChars,bool bApplySettingsNow/*=true*/) // mSec
{
  m_nTotalWriteTimeout=nTotal;
  m_nInterCharWriteTimeout=nBetweenChars;
  return 0==m_pHandle || !bApplySettingsNow || ApplySettings();
}

#ifdef _WIN32
bool CeCosSerial::Open(LPCTSTR pszPort,int nBaud)
{
  bool rc=false;
  m_nBaud=nBaud,
    m_strPort=pszPort;
  HANDLE hCom=::CreateFile(pszPort,GENERIC_READ|GENERIC_WRITE, 0,NULL,OPEN_EXISTING,0,NULL);
  SaveError();
  if (INVALID_HANDLE_VALUE==hCom) { 
    ERROR(_T("Failed to open port %s - %s\n"),pszPort,(LPCTSTR)ErrString());
  } else {
    m_pHandle=(void *)hCom;
    if(ApplySettings()){
      Flush();
      rc=true;
    } else {
      Close();
    }
  }
  return rc;
}

bool CeCosSerial::Close()
{
  bool rc=false;
  if(m_pHandle){
    try {
      rc=(TRUE==CloseHandle((HANDLE)m_pHandle));
    }
    catch(...) {
      TRACE(_T("!!! Exception caught closing serial handle %08x\n"),m_pHandle);
    }
    m_pHandle=0;
  } else {
    rc=true;
  }
  return rc;
}

bool CeCosSerial::ApplySettings()
{
  bool rc=false;
  try {
    DCB dcb;
    
    ZeroMemory(&dcb,sizeof dcb);
    dcb.DCBlength=sizeof dcb;
    dcb.BaudRate=m_nBaud;
    dcb.fBinary=true;
    dcb.fParity=true;
    dcb.Parity=(BYTE) ((m_bParity) ? EVENPARITY : NOPARITY);
    dcb.StopBits=(BYTE)m_nStopBits;
    dcb.ByteSize=(BYTE)m_nDataBits;
    LPCTSTR arpszStopbits[3]={_T("1"),_T("1.5"),_T("2")};
    TRACE(_T("Applysettings baud=%d Parity=%d stopbits=%s databits=%d\n"),
      dcb.BaudRate, 
      dcb.Parity, 
      arpszStopbits[dcb.StopBits],
      dcb.ByteSize);
    
    if (m_bDSRDTRFlowControl) {
        dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
        dcb.fOutxDsrFlow = 1;
    } else {
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        dcb.fOutxDsrFlow = 0;
    }
    
    if (m_bRTSCTSFlowControl) {
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
        dcb.fOutxCtsFlow = 1;
    } else {
        dcb.fRtsControl = RTS_CONTROL_ENABLE;
        dcb.fOutxCtsFlow = 0;
    }

    dcb.fTXContinueOnXoff=1;
    dcb.XonChar=17;
    dcb.XoffChar=19;
    if (m_bXONXOFFFlowControl) {
        dcb.XonLim=512;
        dcb.XoffLim=512;
        dcb.fOutX=1;
        dcb.fInX=1;
    } else {
        dcb.XonLim=2048;
        dcb.XoffLim=512;
        dcb.fOutX=0;
        dcb.fInX=0;
    }
    dcb.fAbortOnError=1;
    
    HANDLE hCom=(HANDLE)m_pHandle;
    if (!SetCommState(hCom, &dcb)) { 
      SaveError();
		    ERROR(_T("Failed to set comm state - port %s handle=%d err=%d\n"),(LPCTSTR)m_strPort,hCom,GetLastError());
    } else {
		    COMMTIMEOUTS commtimeouts;
        if(m_bBlockingReads){		
		        commtimeouts.ReadIntervalTimeout=m_nInterCharReadTimeout;
            commtimeouts.ReadTotalTimeoutMultiplier=0;
            commtimeouts.ReadTotalTimeoutConstant=m_nTotalReadTimeout;
        } else {
		        commtimeouts.ReadIntervalTimeout=MAXDWORD;
            commtimeouts.ReadTotalTimeoutMultiplier=0;
            commtimeouts.ReadTotalTimeoutConstant=0;
        }
        commtimeouts.WriteTotalTimeoutMultiplier=m_nTotalWriteTimeout;
        commtimeouts.WriteTotalTimeoutConstant=m_nInterCharWriteTimeout;
        
        if (SetCommTimeouts(hCom, &commtimeouts)) { 
          rc=true;
        } else {
          SaveError();
          ERROR(_T("Failed to set comm timeouts - port %s\n"),(LPCTSTR)m_strPort);
        }
    }
  }
  catch(...)
  {
    TRACE(_T("!!! Exception caught in CeCosSerial::ApplySettings!!!\n"));
  }
  return rc;
}

bool CeCosSerial::Read (void *pBuf,unsigned int nSize,unsigned int &nRead)
{
  bool rc=(TRUE==ReadFile((HANDLE)m_pHandle,pBuf,nSize,(LPDWORD)&nRead,0));
  SaveError();
  return rc;
}

bool CeCosSerial::Write(void *pBuf,unsigned int nSize,unsigned int &nWritten)
{
  bool rc=(TRUE==WriteFile((HANDLE)m_pHandle,pBuf,nSize,(LPDWORD)&nWritten,0));
  SaveError();
  return rc;
}

bool CeCosSerial::ClearError()
{
  DWORD dwErrors;
  bool rc=(TRUE==ClearCommError(HANDLE(m_pHandle),&dwErrors,0));
  if(dwErrors&CE_BREAK)TRACE(_T("The hardware detected a break condition.\n"));
  if(dwErrors&CE_DNS)TRACE(_T("Windows 95 and Windows 98: A parallel device is not selected.\n"));
  if(dwErrors&CE_FRAME)TRACE(_T("The hardware detected a framing error.\n"));
  if(dwErrors&CE_IOE)TRACE(_T("An I/O error occurred during communications with the device.\n"));
  if(dwErrors&CE_MODE)TRACE(_T("The requested mode is not supported, or the hFile parameter is invalid. If this value is specified, it is the only valid error.\n"));
  if(dwErrors&CE_OOP)TRACE(_T("Windows 95 and Windows 98: A parallel device signaled that it is out of paper.\n"));
  if(dwErrors&CE_OVERRUN)TRACE(_T("A character-buffer overrun has occurred. The next character is lost.\n"));
  if(dwErrors&CE_PTO)TRACE(_T("Windows 95 and Windows 98: A time-out occurred on a parallel device.\n"));
  if(dwErrors&CE_RXOVER)TRACE(_T("An input buffer overflow has occurred. There is either no room in the input buffer, or a character was received after the end-of-file (EOF) character.\n"));
  if(dwErrors&CE_RXPARITY)TRACE(_T("The hardware detected a parity error.\n"));
  if(dwErrors&CE_TXFULL)TRACE(_T("The application tried to transmit a character, but the output buffer was full.\n"));
  return rc;
}

bool CeCosSerial::Flush (void)
{
  bool rc=(TRUE==PurgeComm ((HANDLE)m_pHandle,PURGE_TXCLEAR|PURGE_RXCLEAR));
  SaveError();
  return rc;
}

String CeCosSerial::ErrString() const
{
  String str;
  LPVOID lpMsgBuf;
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    m_nErr,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
    );
  str=(LPCTSTR)lpMsgBuf;
  // Free the buffer.
  LocalFree( lpMsgBuf );
  return str;
}

#else // UNIX

String CeCosSerial::ErrString() const
{
  return strerror(errno);
}

bool CeCosSerial::Close()
{
  bool rc=m_pHandle && (-1!=close((int)m_pHandle));
  m_pHandle=0;
  return rc;
}

bool CeCosSerial::Open(LPCTSTR pszPort,int nBaud)
{
  bool rc=false;
  m_nBaud=nBaud,
    m_strPort=pszPort;
  int fd = open(pszPort,O_RDWR|O_NONBLOCK);
  if (-1==fd) { 
    ERROR(_T("Failed to open port %s\n"),pszPort);
    return false;
  } else {
    m_pHandle=(void *)fd;
    if(ApplySettings()){
      rc=true;
    } else {
      Close();
      ERROR(_T("Failed to apply settings.\n"));
      return false;
    }
  }
  return rc;
}

bool CeCosSerial::ApplySettings()
{
  struct termios buf, buf_verify;
  int rate;
  
  // Clear the two structures so we can make a binary comparison later on.
  memset(&buf, 0, sizeof(buf));
  memset(&buf_verify, 0, sizeof(buf_verify));
  
  LPCTSTR arpszStopbits[3]={_T("1"),_T("1.5"),_T("2")};
  TRACE(_T("Applysettings baud=%d bParity=%d stopbits=%s databits=%d\n"),
    m_nBaud, 
    m_bParity, 
    arpszStopbits[m_nStopBits],
    m_nDataBits);
  
  switch(m_nBaud) {
  case 110:
    rate = B110;
    break;
  case 150:
    rate = B150;
    break;
  case 300:
    rate = B300;
    break;
  case 600:
    rate = B600;
    break;
  case 1200:
    rate = B1200;
    break;
  case 2400:
    rate = B2400;
    break;
  case 4800:
    rate = B4800;
    break;
  case 9600:
    rate = B9600;
    break;
  case 19200:
    rate = B19200;
    break;
  case 38400:
    rate = B38400;
    break;
  case 57600:
    rate = B57600;
    break;
  case 115200:
    rate = B115200;
    break;
  default:
    return false;
  };
  
  TRACE(_T("Changing configuration...\n"));
  
  // Get current settings.
  if (tcgetattr((int) m_pHandle, &buf)) {
    fprintf(stderr, _T("Error: tcgetattr\n"));
    return false;
  }
  
  // Reset to raw.
  buf.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
    |INLCR|IGNCR|ICRNL|IXON);
  buf.c_oflag &= ~OPOST;
  buf.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  buf.c_cflag &= ~(CSIZE|PARENB);
  buf.c_cflag |= CS8;
  
  // Set baud rate.
  cfsetispeed(&buf, rate);
  cfsetospeed(&buf, rate);
  
  // Set data bits.
  {
    int data_bits[9] = {0, 0, 0, 0, 0, CS5, CS6, CS7, CS8};
    
    buf.c_cflag &= ~CSIZE;
    buf.c_cflag |= data_bits[m_nDataBits];
  }
  
  // Set stop bits.
  {
    buf.c_cflag &= ~CSTOPB;
    if (ONE_STOP_BIT != m_nStopBits)
      buf.c_cflag |= CSTOPB;
  }
  
  // Set parity.
  {
    buf.c_cflag &= ~(PARENB | PARODD); // no parity.
    if (m_bParity)                  // even parity.
      buf.c_cflag |= PARENB;
  }

  // Set flow control
  {
      buf.c_iflag &= ~(IXON|IXOFF);
#ifdef CRTSCTS
      buf.c_cflag &= ~CRTSCTS;
#endif
      if ( m_bXONXOFFFlowControl ) {
          buf.c_iflag |= (IXON|IXOFF);
      }
      if ( m_bRTSCTSFlowControl ) {
#ifdef CRTSCTS
          buf.c_cflag |= CRTSCTS;
#else
          return false;
#endif
      }
      if ( m_bDSRDTRFlowControl ) {
#ifdef CDSRDTR
          buf.c_cflag |= CDSRDTR;
#else
          return false;
#endif
      }
      
      
  }
  
  // Set the new settings
  if (tcsetattr((int) m_pHandle, TCSADRAIN, &buf)) {
    fprintf(stderr, _T("Error: tcsetattr\n"));
    return false;
  }
  
  // Now read back the settings. On SunOS tcsetattr only returns
  // error if _all_ settings fail. If just a few settings are not
  // supported, the call returns true while the hardware is set to a
  // combination of old and new settings.
  if (tcgetattr((int) m_pHandle, &buf_verify)) {
    fprintf(stderr, _T("Error: tcgetattr\n"));
    return false;
  }
  if (memcmp(&buf, &buf_verify, sizeof(buf))) {
    fprintf(stderr, _T("Error: termios verify failed\n"));
    return false;
  }
  
  // A slight delay to allow things to settle.
  CeCosThreadUtils::Sleep(10);
  
  TRACE(_T("Done.\n"));
  
  return true;
}

bool CeCosSerial::Flush (void)
{
  return 0==tcflush((int) m_pHandle, TCIOFLUSH);
}

bool CeCosSerial::Read (void *pBuf,unsigned int nSize,unsigned int &nRead)
{
  
  if (!m_bBlockingReads) {
    nRead = 0;
    int n = read((int)m_pHandle, pBuf, nSize);
    if (-1 == n) {
      if (EAGAIN == errno)
        return true;
      ERROR(_T("Read failed: %d\n"), errno);
      return false;
    }
    nRead = n;
#if 0
    if (n>0) {
        unsigned int i;
        fprintf(stderr, "%d:", nRead);
        for (i = 0; i < nRead; i++)
            fprintf(stderr, "%02x!", ((unsigned char *)pBuf)[i]);
        fprintf(stderr, "\n");
    }
#endif
    return true;
  }
  
  // Blocking reads: emulate the Windows semantics:
  //  If m_nTotalReadTimeout elapses before we see the first TCHAR, 
  //   return.
  //  If m_nInterCharReadTimeout elapses after reading any
  //   subsequent TCHAR, return.
  
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET((int)m_pHandle, &rfds);
  
  // Start with total timeout.
  struct timeval tv;
  tv.tv_sec = m_nTotalReadTimeout / 1000;
  tv.tv_usec = (m_nTotalReadTimeout % 1000) * 1000;
  
  unsigned char* pData = (unsigned char*) pBuf;
  nRead = 0;
  while (nSize) {
    switch(select((int)m_pHandle + 1, &rfds, NULL, NULL, &tv)) {
    case 1:
      {
        int n = read((int)m_pHandle, pData, nSize);
        if (-1 == n && EAGAIN != errno) {
          ERROR(_T("Read failed: %d\n"), errno);
          return false;           // FAILED
        }
        else if (n > 0) {
#if 0
            unsigned int i;
            fprintf(stderr, "%d:", nRead);
            for (i = 0; i < nRead; i++)
                fprintf(stderr, "%02x!", ((unsigned char *)pBuf)[i]);
            fprintf(stderr, "\n");
#endif
            nRead += n;
            pData += n;
            nSize -= n;
        }
        
        // Now use inter-char timeout.
        tv.tv_sec = m_nInterCharReadTimeout / 1000;
        tv.tv_usec = (m_nInterCharReadTimeout % 1000) * 1000;
      }
      break;
    case 0:
      return true;                // Timeout 
    case -1:
      ERROR(_T("Select failed: %d\n"), errno);
      return false;
    }
  }
  
  return true;
}

bool CeCosSerial::Write(void *pBuf,unsigned int nSize,unsigned int &nWritten)
{
  bool rc;
  int n=write((int)m_pHandle,pBuf,nSize);
  if(-1==n){
    nWritten=0;
    if (errno == EAGAIN)
      rc = true;
    else
      rc=false;
  } else {
    nWritten=n;
    rc=true;
  }
  return rc;
}

bool CeCosSerial::ClearError()
{
  return false;
}

#endif
