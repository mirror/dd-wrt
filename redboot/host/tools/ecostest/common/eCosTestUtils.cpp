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
//        eCosTestUtils.cpp
//
//        Utility functions
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   This class contains utility functions for use in the testing infrastructure
// Usage:
//
//####DESCRIPTIONEND####

#include "eCosStd.h"
#include "eCosSocket.h"
#include "eCosTestUtils.h"
#include "eCosThreadUtils.h"
#include "eCosTrace.h"
#include "TestResource.h"

LPCTSTR  const CeCosTestUtils::Tail(LPCTSTR  const pszFile)
{
  LPCTSTR pszTail=_tcsrchr(pszFile,_TCHAR('/'));
  if(0==pszTail){
    pszTail=_tcsrchr(pszFile,_TCHAR('\\'));
  }
  return (0==pszTail)?pszFile:pszTail+1;
}

// File iterator.  Gets next file in directory, avoiding _T(".") and _T("..")
bool CeCosTestUtils::NextFile (void *&pHandle,String &str)
{
#ifdef _WIN32
  WIN32_FIND_DATA fd;
  while(FindNextFile((HANDLE)pHandle,&fd)){
    LPCTSTR pszName=fd.cFileName;
#else // UNIX
    struct dirent *d;
    while((d=readdir((DIR *)pHandle))){
      LPCTSTR pszName=d->d_name;
#endif
      if(pszName[0]!='.'){
        str=pszName;
        return true;
      }
    }
    return false;
  }
  
  // Start file iteration and return first file.
bool CeCosTestUtils::StartSearch (void *&pHandle,String &str)
  {
#ifdef _WIN32
    WIN32_FIND_DATA fd;
    pHandle=(void *)FindFirstFile (_T("*.*"), &fd);
    if(INVALID_HANDLE_VALUE==(HANDLE)pHandle){
      ERROR(_T("Failed to open dir\n"));
      return false;
    } else if (fd.cFileName[0]=='.') {
      return NextFile((HANDLE)pHandle,str);
    } else {
		str=String(fd.cFileName);
      return true;
    }
#else // UNIX
    pHandle=(void *)opendir(_T("."));
    if(0==pHandle){
      ERROR(_T("Failed to open dir\n"));
      return false;
    }
    return NextFile(pHandle,str);
#endif
  }
  
  // End file iteration
void CeCosTestUtils::EndSearch (void *&pHandle)
  {
#ifdef _WIN32
    FindClose((HANDLE)pHandle);
#else // UNIX
    closedir((DIR *)pHandle);
#endif
}
  

// deal with common command-line  actions
bool CeCosTestUtils::CommandLine(int &argc,TCHAR **argv,bool bRequireResourceServer)
{
  LPCTSTR psz=_tgetenv(_T("RESOURCESERVER"));
  if(psz && !CTestResource::SetResourceServer(psz)){
    _ftprintf(stderr,_T("Illegal host:port '%s' defined in RESOURCESERVER environment variable\n"),psz);
    return false;
  }
  
  for(int i=1;i<argc;i++){
    if(_TCHAR('-')==*argv[i]){
      if(0==_tcscmp(argv[i],_T("-v"))){
        CeCosTrace::EnableTracing((CeCosTrace::TraceLevel)MAX(CeCosTrace::TracingEnabled(),CeCosTrace::TRACE_LEVEL_TRACE));
        // Shuffle the command line down to remove that which we have just seen:
        for(TCHAR **a=argv+i;(a[0]=a[1]);a++);
        argc--;i--; // counteract the increment
      } else if(0==_tcscmp(argv[i],_T("-V"))){
        CeCosTrace::EnableTracing((CeCosTrace::TraceLevel)MAX(CeCosTrace::TracingEnabled(),CeCosTrace::TRACE_LEVEL_VTRACE));
        // Shuffle the command line down to remove that which we have just seen:
        for(TCHAR **a=argv+i;(a[0]=a[1]);a++);
        argc--;i--; // counteract the increment
      } else if(0==_tcscmp(argv[i],_T("-o"))){
        if(i+1<argc){
          if(!CeCosTrace::SetOutput(argv[i+1])){
            _ftprintf(stderr,_T("Failed to direct output to %s\n"),argv[i+1]);
          }
          // Shuffle the command line down to remove that which we have just seen:
          for(TCHAR **a=argv+i;(a[0]=a[2]);a++);
          argc-=2;i-=2; // counteract the increment
        } else {
          return false;
        }
      } else if(0==_tcscmp(argv[i],_T("-O"))){
        if(i+1<argc){
          if(!CeCosTrace::SetError(argv[i+1])){
            _ftprintf(stderr,_T("Failed to direct error to %s\n"),argv[i+1]);
          }
          // Shuffle the command line down to remove that which we have just seen:
          for(TCHAR **a=argv+i;(a[0]=a[2]);a++);
          argc-=2;i-=2; // counteract the increment
        } else {
          return false;
        }
      } else if(0==_tcscmp(argv[i],_T("-r"))){
        if(i+1<argc){
          if(!CTestResource::SetResourceServer(argv[i+1])){
            _ftprintf(stderr,_T("Illegal host:port '%s'\n"),argv[i+1]);
            return false;
          }
          // Shuffle the command line down to remove that which we have just seen:
          for(TCHAR **a=argv+i;(a[0]=a[2]);a++);
          argc-=2;i-=2; // counteract the increment
        } else {
          return false;
        }
      } else if(0==_tcscmp(argv[i],_T("-version"))){
        const TCHAR *pszTail=_tcsrchr(argv[0],_TCHAR('/'));
        if(0==pszTail){
          pszTail=_tcsrchr(argv[0],_TCHAR('\\'));
        }
  			_tprintf (_T("%s %s (%s %s)\n"), (0==pszTail)?argv[0]:pszTail+1,ECOS_VERSION,  __DATE__, __TIME__);
        exit(0);
      } 
    }
  }

  if(!CeCosSocket::Init() || !CeCosTestPlatform::Load()){
    return false;
  }

#ifndef _WIN32
  sigset_t mask;
  
  // Clean out all the signals
  sigemptyset(&mask);
  
  // Add our sigpipe
  sigaddset(&mask, SIGPIPE);
  
  sigprocmask(SIG_SETMASK, &mask, NULL);

#endif

  if(CTestResource::ResourceServerSet()){
    if(CTestResource::Load()){
      _ftprintf(stderr,_T("Connected to resource server %s\n"),(LPCTSTR)CTestResource::GetResourceServer());
    } else {
      _ftprintf(stderr,_T("Can't load from resource server %s\n"),(LPCTSTR)CTestResource::GetResourceServer());
      return false;
    }
  } else if (bRequireResourceServer) {
    _ftprintf(stderr,_T("You must specify a resource server using either the -r switch or by setting the RESOURCESERVER environment variable\n"));
    return false;
  }

  return true;
}

void CeCosTestUtils::UsageMessage(bool bRequireResourceServer)
{
  _ftprintf(stderr,
    _T("        -o file      : send standard output to named file\n")
    _T("        -O file      : send standard error  to named file\n"));
  if(bRequireResourceServer){
    _ftprintf(stderr,
    _T("        -r host:port : use this host:port as resourceserver [or set the RESOURCESERVER environment variable]\n")
    );
  }
  _ftprintf(stderr,
    _T("        -v           : vebose mode - trace to stderr\n")
    _T("        -V           : very verbose mode - trace to stderr\n")
    _T("        -version     : print a version string\n")
  );
}

const String CeCosTestUtils::HomeFile (LPCTSTR pszFile)
{
  String strFile;
#ifdef _WIN32
  LPCTSTR psz=_tgetenv(_T("HOMEDRIVE"));
  if(psz){
    strFile=psz;
    psz=_tgetenv(_T("HOMEPATH"));
    if(psz){
      strFile+=psz;
    }
    if(_TCHAR('\\')!=strFile[strFile.size()-1]){
      strFile+=_TCHAR('\\');
    }
    strFile+=pszFile;
  }
#else // UNIX
  LPCTSTR psz=_tgetenv(_T("HOME"));
  if(psz){
    strFile=psz;
    strFile+=_TCHAR('/');
    strFile+=pszFile;
  }
#endif
  return strFile;
}

bool CeCosTestUtils::Exists(LPCTSTR pszFile)
{
  struct _stat buf;
  return (0==_tstat(pszFile,&buf));
}

bool CeCosTestUtils::IsFile(LPCTSTR pszFile)
{
  struct _stat buf;
  return 0==_tstat(pszFile,&buf) && 0==(S_IFDIR&buf.st_mode);
}

