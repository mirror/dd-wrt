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
// Description:   Standard include file for trace functions
// Usage:
//
//####DESCRIPTIONEND####

#include "eCosThreadUtils.h"
#include "eCosTrace.h"

CeCosTrace::TraceLevel CeCosTrace::nVerbosity=CeCosTrace::TRACE_LEVEL_ERRORS;
bool CeCosTrace::bInteractive=false;
LPCTSTR CeCosTrace::arpszDow[7]={_T("Su"),_T("M"),_T("Tu"),_T("W"),_T("Th"),_T("F"),_T("Sa")};

LogFunc *CeCosTrace::pfnOut=StreamLogFunc;
void *CeCosTrace::pOutParam=(void *)stdout;
LogFunc *CeCosTrace::pfnError=StreamLogFunc;
void *CeCosTrace::pErrorParam=(void *)stderr;

CeCosTrace::StreamInfo CeCosTrace::OutInfo(_T(""),stdout);
CeCosTrace::StreamInfo CeCosTrace::ErrInfo(_T(""),stderr);

void CALLBACK CeCosTrace::StreamLogFunc(void *pParam, LPCTSTR psz) 
{
  // Interactive mode
  ENTERCRITICAL;
  _fputts(psz,(FILE *)pParam);
  fflush((FILE *)pParam);
  LEAVECRITICAL;
}

bool CeCosTrace::SetOutput(LPCTSTR pszFilename)
{
  FILE *f=_tfopen(pszFilename,_T("a") MODE_TEXT);
  if(f){
    if(!OutInfo.strFilename.empty()){
      fclose(OutInfo.f);
    }
    if(nVerbosity>=TRACE_LEVEL_TRACE){
      _ftprintf(stderr,_T("Output -> %s (%08x)\n"),pszFilename,(unsigned int)f);
    }
    OutInfo.f=f;
    OutInfo.strFilename=pszFilename;
    SetOutput(StreamInfoFunc,&OutInfo);
  }
  return (NULL!=f);
}

bool CeCosTrace::SetError(LPCTSTR pszFilename)
{
  FILE *f=_tfopen(pszFilename,_T("a") MODE_TEXT);
  if(f){
    if(!ErrInfo.strFilename.empty()){
      fclose(ErrInfo.f);
    }
    ErrInfo.f=f;
    ErrInfo.strFilename=pszFilename;
    SetError(StreamInfoFunc,&ErrInfo);
  }
  return (NULL!=f);
}

void CALLBACK CeCosTrace::StreamInfoFunc(void *pParam, LPCTSTR psz) 
{
  StreamInfo *pInfo=(StreamInfo *)pParam;
  ENTERCRITICAL;
  _fputts(psz,pInfo->f);
  if(!pInfo->strFilename.empty() && Now()-pInfo->tLastReopen>20*1000){
    // SAMBA clients will not honor fflush(), so we do this:
    fclose(pInfo->f);
    do {
      pInfo->f=_tfopen(pInfo->strFilename,_T("a") MODE_TEXT);
      if(NULL==pInfo->f){
        _ftprintf(stderr,_T("Failed to reopen %s\n"),(LPCTSTR)pInfo->strFilename);
        CeCosThreadUtils::Sleep(1000);
      }
    } while (NULL==pInfo->f);
    pInfo->tLastReopen=Now();
  } else {
    fflush(pInfo->f);
  }
  LEAVECRITICAL;
}

void CeCosTrace::TimeStampedErr(LPCTSTR pszFormat,...)
{
  va_list marker;
  va_start (marker, pszFormat);
	String str;
  str.vFormat(pszFormat,marker);
  va_end (marker);

  Err(String::SFormat(_T("%s %s"),(LPCTSTR)Timestamp(),(LPCTSTR)str));
}

const String CeCosTrace::Timestamp()
{
  time_t ltime;
  time(&ltime);
  struct tm *now=localtime( &ltime );
  
  bool bInCriticalSection=CeCosThreadUtils::CS::InCriticalSection();
  TCHAR c1=bInCriticalSection?_TCHAR('<'):_TCHAR('[');
  TCHAR c2=bInCriticalSection?_TCHAR('>'):_TCHAR(']');
  return String::SFormat(_T("%c%3x %s %02d:%02d:%02d%c"),c1,CeCosThreadUtils::GetThreadId(),
    arpszDow[now->tm_wday],now->tm_hour,now->tm_min,now->tm_sec,c2);

}
