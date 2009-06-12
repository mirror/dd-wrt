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
//        eCosSerial.h
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
// Description:   This class abstracts the serial port 
// Usage:
//
//
//####DESCRIPTIONEND####

#ifndef _CECOSSERIAL_H
#define _CECOSSERIAL_H
#include "eCosStd.h"
#include "eCosSocket.h"
#include "Collections.h"
//=================================================================
// This class is a host-independent interface to a serial port
//=================================================================
class CeCosSerial {
    friend CeCosSocket::SSReadResult CeCosSocket::SSRead (CeCosSerial &serial,CeCosSocket &socket,void *pBuf,unsigned int nSize,unsigned int &nRead,bool *pbStop);

public:
    enum StopBitsType { ONE_STOP_BIT, ONE_POINT_FIVE_STOP_BITS, TWO_STOP_BITS };
    CeCosSerial(LPCTSTR pszPort,int nBaud); // ctor and open all in one go
    CeCosSerial();                          // Call Open() later
    virtual ~CeCosSerial();

    // Open the port with given baud rate.  Result indicates how successful we've been
    bool Open(LPCTSTR pszPort,int nBaud);

    // Set various line characteristics.  This can be done with the line open or closed.
    // In each case the "bApplySettingsNow" argument indicates whether to perform the action now,
    // or to hold off until a call of ApplySettings().

    bool SetBaud(unsigned int nBaud,bool bApplySettingsNow=true);
    bool SetParity(bool bParityOn,bool bApplySettingsNow=true);
    bool SetDataBits(int n,bool bApplySettingsNow=true);
    bool SetStopBits(StopBitsType n,bool bApplySettingsNow=true);
    bool SetXONXOFFFlowControl(bool b,bool bApplySettingsNow=true);
    bool SetRTSCTSFlowControl(bool b,bool bApplySettingsNow=true);
    bool SetDSRDTRFlowControl(bool b,bool bApplySettingsNow=true);
    bool SetReadTimeOuts(int nTotal,int nBetweenChars,bool bApplySettingsNow=true);  // Times are in mSec
    bool SetWriteTimeOuts(int nTotal,int nBetweenChars,bool bApplySettingsNow=true); // Times are in mSec

    bool ApplySettings();
    
    // Query the settings:
    int  GetParity() const { return m_bParity; }
    int  GetDataBits() const { return m_nDataBits; }
    StopBitsType GetStopBits() const { return m_nStopBits; }
    bool GetXONXOFFFlowControl() const { return m_bXONXOFFFlowControl; }
    bool GetRTSCTSFlowControl() const { return m_bRTSCTSFlowControl; }
    bool GetDSRDTRFlowControl() const { return m_bDSRDTRFlowControl; }
    unsigned int GetBaud() const { return m_nBaud; }
    bool GetReadTimeOuts(int &nTotal,int &nBetweenChars) const {nTotal=m_nTotalReadTimeout; nBetweenChars=m_nInterCharReadTimeout; return true; }// mSec
    bool GetWriteTimeOuts(int &nTotal,int &nBetweenChars) const {nTotal=m_nTotalWriteTimeout; nBetweenChars=m_nInterCharWriteTimeout; return true; }// mSec
    bool GetBlockingReads() const { return m_bBlockingReads; }
    bool Close();

    // Clear the serial buffer:
    bool Flush (void);

    // Use to test success after opening with the ctor:
    bool Ok() const { return 0!=m_pHandle; }

    // Will read up to the length provided:
    bool Read (void *pBuf,unsigned int nSize,unsigned int &nRead);
    bool Write(void *pBuf,unsigned int nSize,unsigned int &nWritten);

    // Use in the event of an error that needs to be cleared before the next operation:
    bool ClearError();

    // Set blocking/non-blocking
    bool SetBlockingReads(bool b,bool bApplySettingsNow=true);

    // Return last error
    int Error() const { return m_nErr; }

    // Return last error, translated to a string
    String ErrString() const;

protected:
    // The last error:
    int m_nErr
      ;
    // Remember the error
    void SaveError() { 
        #ifdef _WIN32
        m_nErr=WSAGetLastError();
        #else // UNIX
        m_nErr=errno;
        #endif
    }

    // Line characteristics:
    void *m_pHandle;
    int m_nDataBits;
    StopBitsType m_nStopBits;
    bool m_bXONXOFFFlowControl;
    bool m_bRTSCTSFlowControl;
    bool m_bDSRDTRFlowControl;
    bool m_bParity;
    unsigned int m_nBaud;
    int m_nTotalReadTimeout,m_nTotalWriteTimeout;
    int m_nInterCharReadTimeout,m_nInterCharWriteTimeout; 
    bool m_bBlockingReads;
    String m_strPort;
};
#endif
