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
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   Holds information related to target reset
// Usage:
//
//####DESCRIPTIONEND####
#include "eCosStd.h"
#include "eCosThreadUtils.h"
#include "eCosTrace.h"
#include "ResetAttributes.h"

const CResetAttributes CResetAttributes::NoReset;

CResetAttributes::CResetAttributes(LPCTSTR psz) : 
  // Default values:
  m_nDelay(1000),
  m_nReadTimeout(10*1000),
  m_nBaud(38400)
{
  // Remove spaces
  while(*psz){
    if(!_istspace(*psz)){
      m_str+=*psz;
    }
    psz++;
  }
}

/*
LPCTSTR CResetAttributes::Image(int nErr)
{
  switch(nErr){
    case RESET_OK:
      return _T("RESET_OK");
      break;
    case RESET_ILLEGAL_DEVICE_CODE:
      return _T("Illegal device code");
      break;
    case RESET_NO_REPLY:
      return _T("No reply from reset unit");
      break;
    case RESET_BAD_CHECKSUM:
      return _T("Bad checksum");
      break;
    case RESET_BAD_ACK:
      return _T("Bad ack");
      break;
    default:
      return _T("Unknown reset error");
      break;
  }
}
*/
void CResetAttributes::SuckThreadFunc()
{
  m_strResetOutput=_T("");
  
  // Board has apparently been powered on.  Suck initial output.
  ResetLog(String::SFormat(_T("Reading board startup output from %s with timeout of %d seconds..."),(LPCTSTR)m_strAuxPort,m_nReadTimeout/1000));

  enum {BUFSIZE=512};
  TCHAR buf[1+BUFSIZE];
  memset(buf,0,BUFSIZE); // safety for string functions in IsValidReset
  do {
    unsigned int dwRead=0;
    // We are working in non-blocking mode
    if(m_Socket.Ok()){
      if(!m_Socket.Peek(dwRead)||!m_Socket.recv(buf,MIN(dwRead,BUFSIZE))){
        break;
      }
    } else if (!m_Serial.Read(buf,BUFSIZE,dwRead)){
      m_Serial.ClearError();
      continue;
    } 
    if(dwRead>0){
      buf[dwRead]=_TCHAR('\0');
      
      // Remove unprintable characters
      String str;
      for(const TCHAR *t=buf;*t;t++){
        if(_istprint(*t)){
          str+=*t;
        }
      }

      if(m_pfnReset){
        ENTERCRITICAL;
        m_pfnReset(m_pfnResetparam,str);
        LEAVECRITICAL;
      }

      ResetLog(str);
      m_strResetOutput+=str;

      if(IsValidReset()){
        break;
      }
//    } else { // Nothing read
//      CeCosThreadUtils::Sleep(50);
    }
  } while (0==m_tResetOccurred || Now()-m_tResetOccurred<m_nReadTimeout);

  if(0==m_strResetOutput.size()){
    ResetLog(_T("No response from board"));
  } else {
    if(m_pfnReset){
      ENTERCRITICAL;
      m_pfnReset(m_pfnResetparam,_T("\n"));
      LEAVECRITICAL;
    }
    TRACE(_T("%s"),(LPCTSTR)m_strResetOutput);
  }
}

bool CResetAttributes::Reset(Action action,bool bCheckOutput)
{
  m_tResetOccurred=0;
  m_strResetOutput=_T("");
  bool rc=false;
  CeCosSocket sock;
  String strStatus;
  strStatus.Format(_T("Reset target using %s %s port=%s(%d) read timeout=%d delay=%d"),
    (LPCTSTR)m_strHostPort,(LPCTSTR)m_strControl, 
    (LPCTSTR)m_strAuxPort, m_nBaud, m_nReadTimeout, m_nDelay);
  if(bCheckOutput){
    strStatus+=_T(" expect(");
    for(unsigned int i=0;i<m_arValidResetStrings.size();i++){
      if(i>0){
        strStatus+=_TCHAR(',');
      }
      strStatus+=m_arValidResetStrings[i];
    }
    strStatus+=_T(")");
  }
  ResetLog(strStatus);

  // Open up communication to port whence we read the board startup
  bool bThreadDone=false;
  bCheckOutput&=(m_strAuxPort.size()>0);
  if(bCheckOutput){
    TRACE(_T("Opening %s\n"),(LPCTSTR)m_strAuxPort);
    if(CeCosSocket::IsLegalHostPort(m_strAuxPort)){
      // tcp/ip port
      if(!m_Socket.Connect(m_strAuxPort,m_nReadTimeout)){
        ResetLog(String::SFormat(_T("Failed to open %s - %s"),(LPCTSTR)m_strAuxPort,(LPCTSTR)m_Socket.SocketErrString()));
        return false;
      }
    } else {
      // Comms device
      m_Serial.SetBlockingReads(false);
      if(m_Serial.Open(m_strAuxPort,m_nBaud)){
        m_Serial.Flush();
      } else {
        ResetLog(String::SFormat(_T("Failed to open comms port %s - %s"),(LPCTSTR)m_strAuxPort,(LPCTSTR)m_Serial.ErrString()));
        return false;
      }
    }
    CeCosThreadUtils::RunThread(SSuckThreadFunc,this,&bThreadDone,_T("SSuckThreadFunc"));
  } else {
    ResetLog(_T("[not checking output]"));
  }
  
  // This will be true if we need to talk to a reset server, false to talk down a local port
  bool bRemote=CeCosSocket::IsLegalHostPort(m_strHostPort);
  if(bRemote){
    if(sock.Connect(m_strHostPort,10*1000)){
      // Write the message to the socket
      int nDelay=(action==ON_OFF || action==OFF_ON)?m_nDelay:0;
      TRACE(_T("-Control=%s -Action=%d -Delay=%d"),(LPCTSTR)m_strControl,action,nDelay);
      if(sock.sendString(String::SFormat(_T("-Control=%s -Action=%d -Delay=%d"),(LPCTSTR)m_strControl,action,nDelay),_T("Reset control codes"), 10*1000)){
        // Wait for an acknowledgement
        String strResponse;
        if(sock.recvString(strResponse, _T("Response"), nDelay+20*1000)){
          rc=(0==strResponse.size());
          if(!rc && m_pfnReset){
            ResetLog(String::SFormat(_T("Reset server reports error '%s'"),(LPCTSTR)strResponse));
          }
        } else {
          ResetLog(String::SFormat(_T("Failed to read response from reset server %s - %s"),(LPCTSTR)m_strHostPort,(LPCTSTR)sock.SocketErrString()));
        }
      } else {
        ResetLog(String::SFormat(_T("Failed to contact reset server %s - %s"),(LPCTSTR)m_strHostPort,(LPCTSTR)sock.SocketErrString()));
      }
      m_tResetOccurred=Now();
      if(bCheckOutput){
        if(!rc){
          // force thread to time out
          m_tResetOccurred=Now()-m_nReadTimeout;
        }
        CeCosThreadUtils::WaitFor(bThreadDone); // do not apply a timeout - the thread has one
        rc=IsValidReset();
        ResetLog(rc?_T("Reset output valid"):_T("Reset output INVALID"));
      } 
    } else {
      ResetLog(String::SFormat(_T("Failed to contact reset server %s - %s"),(LPCTSTR)m_strHostPort,(LPCTSTR)sock.SocketErrString()));
    }
  } else {
    // Sending something locally
    m_tResetOccurred=Now();
    unsigned int nWritten;
    m_Serial.Write((void *)(LPCTSTR)m_strControl,1,nWritten);
    if(bCheckOutput){
      CeCosThreadUtils::WaitFor(bThreadDone);  // do not apply a timeout - the thread has one
      rc=IsValidReset();
      ResetLog(rc?_T("Reset output valid"):_T("Reset output INVALID"));
    }
  }

  if(m_Socket.Ok()){
    m_Socket.Close();
  } else {
    m_Serial.Close();
  }
  return rc && bCheckOutput;
}

// We expect to be passed a string that starts with "xxx(yyy)"
// and the task is to extract xxx into strID and yyy into strArg
const TCHAR *CResetAttributes::GetIdAndArg (LPCTSTR psz,String &strID,String &strArg)
{
  const TCHAR *cEnd=_tcschr(psz,_TCHAR('('));
  if(cEnd){
    strID=String(psz,cEnd-psz);
    int nNest=0;
    for(const TCHAR *c=cEnd;*c;c++){
      if(_TCHAR('(')==*c){
        nNest++;
      } else if(_TCHAR(')')==*c){
        nNest--;
        if(0==nNest){
          strArg=String(cEnd+1,c-(cEnd+1));
          return c+1;
        }
      }
    }
    assert(false);
  }
  return 0;
}

// Do the reset
CResetAttributes::ResetResult CResetAttributes::Reset (LogFunc *pfnLog, void *pfnLogparam,bool bCheckOnly)
{
  m_pfnReset=pfnLog;
  m_pfnResetparam=pfnLogparam;

  // First we clean up the reset string so as to make subsequent parsing less complicated.
  // Spaces have already been removed in the ctor

  // Check paren matching:
  int nNest=0;
  for(const TCHAR *c=m_str;*c;c++){
    if(_TCHAR('(')==*c){
      nNest++;
    } else if(_TCHAR(')')==*c){
      nNest--;
      if(nNest<0){
        ResetLog(_T("Too many right parentheses"));
        return INVALID_STRING;
      }
    }
  }
  if(nNest>0){
    ResetLog(_T("Too many left parentheses"));
    return INVALID_STRING;
  }

  return Parse(m_str,bCheckOnly);
}

// This function parses the reset string, whose form is something like:
//     expect($T05) 3(off(ginga:5000,a1) delay(2000) on(ginga:5000,a1,com1,38400,10000))
// It is recursive (which is another reason elementary syntax checking was carried out above)
// and calls itself to perform repeats [e.g. 3(...)]
CResetAttributes::ResetResult CResetAttributes::Parse (LPCTSTR psz,bool bCheckOnly)
{
  enum {ARGSEP=_TCHAR(',')};
  bool bCheck=false;
  for(const TCHAR *c=psz;*c;){
    String strID,strArg;
    c=GetIdAndArg(c,strID,strArg);
    if(0==c){
      ResetLog(_T("Invalid reset string"));
      return INVALID_STRING;
    }

    if(isdigit(*(LPCTSTR)strID)){
      // Process a repeat-until-reset.  Syntax is n(resetstring)
      int nRepeat=_ttoi(strID);
      if(0==nRepeat){
        ResetLog(_T("Invalid reset string"));
        return INVALID_STRING;
      }
      if(bCheckOnly){
        return Parse(strArg,true);
      } else {
        while(nRepeat--){
          ResetResult r=Parse(strArg);
          if(RESET_OK==r||INVALID_STRING==r){
            return r;
          }
        }
      }
    } else if (_T("expect")==strID) {
      //   Expected string(s).  e.g. expect(str1,str2,...).
      strArg.Chop(m_arValidResetStrings,ARGSEP,true);
    } else if (_T("port")==strID) {
      // Port information.      e.g. port(com1,38400,1000)
      // This information will apply to all subsequent actions until overwritten.
      // Specifically args are:
      //   0. Port
      //   1. Baud
      //   2. Read timeout
      StringArray ar;
      int nArgs=strArg.Chop(ar,ARGSEP,true);
      if(nArgs>0){
        m_strAuxPort=ar[0];
      }
      if(nArgs>1){
        m_nBaud=_ttoi(ar[1]);
      }
      if(nArgs>2){
        m_nReadTimeout=_ttoi(ar[2]);
      }
    } else if (_T("off")==strID || _T("on")==strID || _T("on_off")==strID || _T("off_on")==strID) {
      // Action information.      e.g. off(ginga:500,A4,com1,38400,10000,1000)
      // Specifically args are:
      //   0. Reset host:port
      //   1. Control string
      //   2. Port
      //   3. Baud
      //   4. Read timeout
      //   5. Delay
      StringArray ar;
      int nArgs=strArg.Chop(ar,ARGSEP,true);
      if(nArgs>0){
        m_strHostPort=ar[0];  
      }
      if(nArgs>1){
        m_strControl=ar[1];
      }
      if(nArgs>2){
        m_strAuxPort=ar[2];
      }
      if(nArgs>3){
        m_nBaud=_ttoi(ar[3]);
      }
      if(nArgs>4){
        m_nReadTimeout=_ttoi(ar[4]);
      }
      if(nArgs>5){
        m_nDelay=_ttoi(ar[5]);
      }

      if(0==m_strHostPort.size()){
        ResetLog(_T("Failed to specify reset host:port"));
        return INVALID_STRING;
      }

      Action action=ON; // prevent compiler warning
      if(_T("on")==strID){
        action=ON;
      } else if(_T("off")==strID){
        action=OFF;
      } else if(_T("on_off")==strID){
        action=ON_OFF;
      } else if(_T("off_on")==strID){
        action=OFF_ON;
      } 

      if(!bCheckOnly && Reset(action,bCheck||action==ON_OFF||action==OFF_ON)){
        return RESET_OK;
      }
      bCheck ^= 1;
    } else if (_T("delay")==strID) {
      // Delay for a given time right now.      e.g. delay(1000)
      // Specifically args are:
      //   0. msec to delay
      TRACE(_T("CeCosThreadUtils::Sleep %d\n"),_ttoi(strArg));
      if(!bCheckOnly){
        CeCosThreadUtils::Sleep(_ttoi(strArg));
      }
    } else {
      ResetLog(String::SFormat(_T("Unrecognized command '%s'"),(LPCTSTR)strID));
      return INVALID_STRING;
    }
  }
  ResetLog(_T("Target reset not verified"));
  return NOT_RESET;
}

// Log some output to the reset log function.
void CResetAttributes::ResetLog(LPCTSTR psz)
{
  if(m_pfnReset){
    ENTERCRITICAL;
    m_pfnReset(m_pfnResetparam,String::SFormat(_T("%s >>> %s\n"),(LPCTSTR)CeCosTrace::Timestamp(),psz));
    TRACE(_T("%s"),psz);
    LEAVECRITICAL;
  }
}

bool CResetAttributes::IsValidReset()
{
  unsigned int n=0;
  ENTERCRITICAL;
  for(int i=m_arValidResetStrings.size()-1;i>=0;--i){
    if(_tcsstr(m_strResetOutput,m_arValidResetStrings[i])){
      n++;
    }
  }
  LEAVECRITICAL;
  return n==m_arValidResetStrings.size();
}
