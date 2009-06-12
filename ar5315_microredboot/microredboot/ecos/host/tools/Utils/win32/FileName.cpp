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
#include "stdafx.h"
#include "FileName.h"
#include <shlwapi.h>

const TCHAR CFileName::cSep=_TCHAR('\\');

CFileName::CFileName(LPCTSTR psz1,LPCTSTR psz2):
CString(psz1)
{
  Normalize();
  operator+=(psz2);
}

CFileName::CFileName(LPCTSTR psz1,LPCTSTR psz2,LPCTSTR psz3):
CString(psz1)
{
  Normalize();
  operator+=(psz2);
  operator+=(psz3);
}

CFileName::CFileName(LPCTSTR psz1,LPCTSTR psz2,LPCTSTR psz3,LPCTSTR psz4):
CString(psz1)
{
  Normalize();
  operator+=(psz2);
  operator+=(psz3);
  operator+=(psz4);
}

CFileName::CFileName(LPCTSTR psz1,LPCTSTR psz2,LPCTSTR psz3,LPCTSTR psz4,LPCTSTR psz5):
CString(psz1)
{
  operator+=(psz2);
  operator+=(psz3);
  operator+=(psz4);
  operator+=(psz5);
}

CFileName AFXAPI operator+(const CFileName& string1, const CFileName& string2)
{
  CFileName s;
  s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
    string2.GetData()->nDataLength, string2.m_pchData);
  return s;
}

CFileName AFXAPI operator+(const CFileName& string, LPCTSTR lpsz)
{
  ASSERT(lpsz == NULL || AfxIsValidString(lpsz));
  CFileName s;
  s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
    CFileName::SafeStrlen(lpsz), lpsz);
  return s;
}

CFileName AFXAPI operator+(LPCTSTR lpsz, const CFileName& string)
{
  ASSERT(lpsz == NULL || AfxIsValidString(lpsz));
  CFileName s;
  s.ConcatCopy(CFileName::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
    string.m_pchData);
  return s;
}

CFileName AFXAPI operator+(const CFileName& string1, TCHAR ch)
{
  CFileName s;
  s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
  return s;
}

CFileName AFXAPI operator+(TCHAR ch, const CFileName& string)
{
  CFileName s;
  s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
  return s;
}

const CFileName& CFileName::operator+=(LPCTSTR lpsz)
{
  ASSERT(lpsz == NULL || AfxIsValidString(lpsz));
  ConcatInPlace(SafeStrlen(lpsz), lpsz);
  return *this;
}

const CFileName& CFileName::operator+=(TCHAR ch)
{
  ConcatInPlace(1, &ch);
  return *this;
}

const CFileName& CFileName::operator+=(const CFileName& string)
{
  ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
  return *this;
}

void CFileName::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
{
  if(nSrcLen>0){
    if(GetLength()>0){
      // Appending a single separator to a non-null string is a no-op
      if(1==nSrcLen && cSep==*lpszSrcData){
        return;
      }
      // Count the intervening separators
      int n=(cSep==m_pchData[GetLength()-1])+(cSep==*lpszSrcData);
      switch(n){
      case 0:
        CString::ConcatInPlace(1, &cSep);
        break;
      case 1:
        break;
      case 2:
        lpszSrcData++;
        nSrcLen--;
        break;
      }
    }
    CString::ConcatInPlace(nSrcLen, lpszSrcData);
  }
}

void CFileName::ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data,
                           int nSrc2Len, LPCTSTR lpszSrc2Data)
{
  int n=1;
  int nNewLen = nSrc1Len + nSrc2Len;
  if(nSrc1Len>0){
    if(1==nSrc2Len && cSep==*lpszSrc2Data){
      // Appending a single separator to a non-null string is a no-op
      lpszSrc2Data++;
      nSrc2Len--;
      nNewLen--;
    } else {
      // Count the intervening separators
      n=(cSep==lpszSrc1Data[nSrc1Len-1])+(cSep==*lpszSrc2Data);
      
      switch(n){
      case 0:
        nNewLen++;
        break;
      case 1:
        break;
      case 2:
        lpszSrc2Data++;
        nSrc2Len--;
        nNewLen--;
        break;
      }
    }
  }
  if (nNewLen != 0)
  {
    AllocBuffer(nNewLen);
    memcpy(m_pchData, lpszSrc1Data, nSrc1Len*sizeof(TCHAR));
    LPTSTR p=m_pchData+nSrc1Len;
    if(0==n){
      *p++=cSep;
    }
    memcpy(p, lpszSrc2Data, nSrc2Len*sizeof(TCHAR));
  }
}

void CFileName::Normalize()
{
  // Remove any trailing slash
  int &n=GetData()->nDataLength;
  if(n>1 && (cSep==m_pchData[n-1])){
    n--;
    m_pchData[n] = _TCHAR('\0');
  }
}


const CFileName CFileName::FullName() const
{
  PTCHAR pFile;
  DWORD dwSize=::GetFullPathName (*this, 0, NULL, &pFile);
  if(dwSize>0){
    CString strCopy;
    ::GetFullPathName (*this, 1+dwSize, strCopy.GetBuffer(1+dwSize), &pFile);
    strCopy.ReleaseBuffer();
    return strCopy;
  } else {
    return *this;
  }
}

const CFileName CFileName::ShortName() const
{
  DWORD dwSize=::GetShortPathName (*this, NULL, 0);
  if(dwSize>0){
    CString strCopy;
    ::GetShortPathName (*this, strCopy.GetBuffer(1+dwSize), 1+dwSize);
    strCopy.ReleaseBuffer();
    return strCopy;
  } else {
    return *this;
  }
}

const CFileName CFileName::NoSpaceName() const// sans spaces
{
  CStringArray ar1,ar2;
  const CString str2(ShortName());
  
  LPCTSTR pc,pcStart;
  
  pcStart=*this;
  for(pc=*this;*pc;pc++){
    if(_TCHAR('\\')==*pc){
      ar1.Add(CString(pcStart,pc-pcStart));
      pcStart=pc;
    }
  }
  ar1.Add(CString(pcStart,pc-pcStart));
  
  pcStart=str2;
  for(pc=str2;*pc;pc++){
    if(_TCHAR('\\')==*pc){
      ar2.Add(CString(pcStart,pc-pcStart));
      pcStart=pc;
    }
  }
  ar2.Add(CString(pcStart,pc-pcStart));
  
  ASSERT(ar1.GetSize()==ar2.GetSize());
  
  CString rc;
  for(int i=0;i<ar1.GetSize();i++){
    rc+=(-1==ar1[i].Find(_TCHAR(' ')))?ar1[i]:ar2[i];
  }
  return rc;
}

//
const CFileName CFileName::Tail() const
{
  TCHAR *ch=_tcsrchr(m_pchData,cSep);
  return ch?ch+1:m_pchData;
}

const CFileName CFileName::Head() const
{
  TCHAR *ch=_tcsrchr(m_pchData,cSep);
  return ch?CFileName(m_pchData,ch-m_pchData):m_pchData;
}

// GetFileAttributes is not available in Win95 so we test the water first
FILETIME CFileName::LastModificationTime() const
{
  static HINSTANCE hInst=LoadLibrary(_T("kernel32.dll"));
  ASSERT(hInst);
  
#ifdef _UNICODE
#define GETFILEATTRIBUTESNAME "GetFileAttributesExW"
#else
#define GETFILEATTRIBUTESNAME "GetFileAttributesExA"
#endif // !UNICODE
  
  typedef BOOL (WINAPI *GetFileAttributesP)(LPCTSTR,GET_FILEEX_INFO_LEVELS,LPVOID);
  
  static GetFileAttributesP p=(GetFileAttributesP)GetProcAddress(hInst,GETFILEATTRIBUTESNAME);
  if(p){
    WIN32_FILE_ATTRIBUTE_DATA data;
    
    if((*p)(*this, GetFileExInfoStandard, (LPVOID)&data)){
      return data.ftLastWriteTime;
    }
  } else {
    HANDLE h=CreateFile(*this,0,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    FILETIME ft;
    if(INVALID_HANDLE_VALUE!=h){
      BOOL b=::GetFileTime(h,NULL,NULL,&ft);
      ::CloseHandle(h);
      if(b){
        return ft;
      }
    }
  }
  static FILETIME ftNull={0,0};
  return ftNull;
}

bool CFileName::SetFileAttributes(DWORD dwFileAttributes) const
{
  return ::SetFileAttributes (*this, dwFileAttributes)!=0;
}

bool CFileName::SameFile(const CFileName &o) const
{
  return 0==ShortName().CompareNoCase(o.ShortName());
}

CFileName CFileName::ExpandEnvironmentStrings(LPCTSTR psz)
{
  // First call simply determines the size
  CFileName f;
  DWORD dwSize=::ExpandEnvironmentStrings(psz, NULL, 0);
  if(dwSize>0){
    ::ExpandEnvironmentStrings(psz, f.GetBuffer(dwSize), dwSize);
    f.ReleaseBuffer();
  } else {
    f=psz;
  }
  return f;
}

const CFileName& CFileName::ExpandEnvironmentStrings()
{
  *this=CFileName::ExpandEnvironmentStrings(*this);
  return *this;
}

// Helper for Relative()  psz is in full format.
CFileName CFileName::Drive(LPCTSTR psz)
{
  if(_istalpha(psz[0])){
    return psz[0];
  } else if(cSep==psz[0]&&cSep==psz[1]){
    TCHAR *c=_tcschr(psz+2,cSep);
    if(c){
      c=_tcschr(c+1,cSep);
      if(c){
        return CString(psz,c-psz);
      }
    }
  }
  return _T("");
}

CFileName CFileName::Relative(LPCTSTR compare,LPCTSTR current)
{
  CString rc;
  bool b=(TRUE==PathRelativePathTo(rc.GetBuffer(1+MAX_PATH),current,FILE_ATTRIBUTE_DIRECTORY,compare,0));
  rc.ReleaseBuffer();
  //TRACE(_T("compare=%s current=%s result=%s (b=%d)\n"),compare,current,b?rc:compare,b);
  return b?rc:compare;
} 
  
const CFileName& CFileName::MakeRelative(LPCTSTR pszRelativeTo)
{
  *this=CFileName::Relative(*this,pszRelativeTo);
  return *this;
}


CFileName CFileName::GetCurrentDirectory()
{
  CFileName f;
  ::GetCurrentDirectory(1+_MAX_PATH,f.GetBuffer(1+_MAX_PATH));
  f.ReleaseBuffer();
  f.Normalize();
  return f;
}


const CFileName& CFileName::Append(LPCTSTR lpsz)
{
  ASSERT(lpsz == NULL || AfxIsValidString(lpsz));
  CString::ConcatInPlace(SafeStrlen(lpsz), lpsz);
  return *this;
  
}

const CFileName& CFileName::Append(TCHAR ch)
{
  ConcatInPlace(1, &ch);
  return *this;
}

bool CFileName::IsAbsolute() const
{
  int nLength=GetLength();
  LPCTSTR psz=*this;
  return 
    (nLength>0 && (cSep==psz[0]))|| // starts with '\'
    (nLength>1 && (
    (_istalpha(psz[0]) && _TCHAR(':')==psz[1]) ||  // starts with [e.g.] "c:\"
    (cSep==psz[0] && cSep==psz[1])));              // UNC
}

// Return an array of filename pieces.  Plugs '\0's into 'this', which
// is therefore subsequently only usable as referenced by the returned array.
LPCTSTR *CFileName::Chop()
{
  TCHAR *c;
  // Count the separators
  int nSeps=0;
  for(c=_tcschr(m_pchData,cSep);c;c=_tcschr(c+1,cSep)){
    nSeps++;
  }
  LPCTSTR *ar=new LPCTSTR[2+nSeps]; // +1 for first, +1 for terminating 0
  ar[0]=m_pchData;
  int i=1;
  for(c=_tcschr(m_pchData,cSep);c;c=_tcschr(c+1,cSep)){
    ar[i++]=c+1;
    *c=_TCHAR('\0');
  }
  ar[i]=0;
  return ar;
}

CFileName CFileName::GetTempPath()
{
  CFileName f;
  ::GetTempPath(1+_MAX_PATH,f.GetBuffer(1+_MAX_PATH));
  f.ReleaseBuffer();
  f.Normalize();
  return f;
  
}

const CFileName CFileName::CygPath () const 
{
  TCHAR buf[2+MAX_PATH];
  LPCTSTR rc=buf+1;
  if(!GetShortPathName(m_pchData,1+buf,MAX_PATH)){
    _tcscpy(1+buf,m_pchData);
  }
  if(_istalpha(*rc)&&_TCHAR(':')==rc[1]){
    // Convert c:\ to //c/ [this is the bit that requires the first char of buf]
    buf[0]=_TCHAR('/');
    buf[2]=buf[1];
    buf[1]=_TCHAR('/');
    rc=buf;
  }
  for(TCHAR *c=buf+1;*c;c++){
    if(_TCHAR('\\')==*c){
      *c=_TCHAR('/');
    }
  }
  
  return rc;
}

bool CFileName::CreateDirectory(bool bParentsToo,bool bFailIfAlreadyExists) const
{
  LPCTSTR pszDir=m_pchData;
  if(bParentsToo){
    // Create intermediate directories
    for(LPCTSTR c=_tcschr(pszDir,_TCHAR('\\'));c;c=_tcschr(c+1,_TCHAR('\\'))){
      if(c==pszDir+2 && _istalpha(pszDir[0]) && _TCHAR(':')==pszDir[1]){
        continue; // don't attempt to create "C:"
      }
      const CFileName strDir(pszDir,c-pszDir);
      if(!(strDir.IsDir()? (!bFailIfAlreadyExists) : ::CreateDirectory(strDir,NULL))){
        return false;
      }
    }
  }
  return IsDir()? (!bFailIfAlreadyExists) : (TRUE==::CreateDirectory(pszDir,NULL));
}


const CString CFileName::Extension() const
{
  LPCTSTR ch=_tcsrchr(m_pchData,_TCHAR('.'));
  if(ch && !_tcschr(ch,cSep)){
    return CString(ch+1);
  } else {
    return _T("");
  }
}

const CString CFileName::Root() const
{
  LPCTSTR ch=_tcsrchr(m_pchData,_TCHAR('.'));
  if(ch && !_tcschr(ch,cSep)){
    return CString(m_pchData,ch-m_pchData);
  } else {
    return m_pchData;
  }
}

CFileName CFileName::SetCurrentDirectory(LPCTSTR pszDir)
{
  const CFileName strPwd=GetCurrentDirectory();
  return ::SetCurrentDirectory(pszDir)?strPwd:_T("");
}

bool CFileName::RecursivelyDelete()
{
  CFileNameArray ar;
  for(int i=FindFiles(*this,ar)-1;i>=0;--i){
    ::DeleteFile(ar[i]);
  }
  for(i=FindFiles(*this,ar,_T("*.*"),true,0)-1;i>=0;--i){
    ::RemoveDirectory(ar[i]);
  }
  return TRUE==::RemoveDirectory(*this);
}

int CFileName::FindFiles (LPCTSTR pszDir,CFileNameArray &ar,LPCTSTR pszPattern/*=_T("*.*")*/,bool bRecurse/*=true*/,DWORD dwExclude/*=FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_HIDDEN*/)
{
  ar.RemoveAll();
  CFileFind finder;
  BOOL bMore=finder.FindFile(CFileName(pszDir)+pszPattern);
  while (bMore)    {
    bMore = finder.FindNextFile();
    if(!finder.IsDots() && !finder.MatchesMask(dwExclude)){
      CFileName strFile(finder.GetFilePath());
      ar.Add(strFile);
    }
  }
  if(bRecurse){
    CFileFind finder;
    BOOL bMore=finder.FindFile(CFileName(pszDir)+_T("*.*"));
    while (bMore)    {
      bMore = finder.FindNextFile();
      if(!finder.IsDots() && finder.IsDirectory()){
        CFileNameArray ar2;
        FindFiles(finder.GetFilePath(),ar2,pszPattern,bRecurse,dwExclude);
        ar.Append(ar2);
      }
    }
  }
  return ar.GetSize();
}

void CFileName::ReplaceExtension(LPCTSTR pszNewExt)
{
  ASSERT(pszNewExt);
  if(_TCHAR('.')==*pszNewExt){
    // Be tolerant of whether '.' is included in what we are passed:
    pszNewExt++;
  }
  LPTSTR pch=GetBuffer(2+GetLength()+_tcslen(pszNewExt));
  LPTSTR pcExt=_tcsrchr(pch,_TCHAR('.'));
  if(NULL==pcExt || _tcschr(pcExt,cSep)){
    // No existing extension
    pcExt=pch+GetLength();
    *pcExt++=_TCHAR('.');
  }
  _tcscpy(pcExt+1,pszNewExt);
  ReleaseBuffer();
}
