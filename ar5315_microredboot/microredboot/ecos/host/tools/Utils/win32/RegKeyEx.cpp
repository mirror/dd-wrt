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
// RegKeyEx.cpp : implementation file
//

#include "stdafx.h"
#include "RegKeyEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CRegKeyEx::Value::Value () :
  m_dwType(REG_NONE)
{
}

CRegKeyEx::Value::Value (DWORD dw) :
  m_dwType(REG_NONE),
  m_dw(dw)
{
}

CRegKeyEx::Value::Value (LPCTSTR psz) :
  m_dwType(REG_SZ)
{
  m_psz=new TCHAR[1+_tcslen(psz)];
  _tcscpy(m_psz,psz);
}

CRegKeyEx::Value::~Value ()
{
  switch(Type()) {
    case REG_SZ:
    case REG_EXPAND_SZ:
      delete m_psz;
      break;
    default:
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////
// CRegKeyEx

CRegKeyEx::~CRegKeyEx()
{
  if(m_hKey){
    Close();
  }
}

CRegKeyEx::CRegKeyEx(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired)
{
  Create(hKeyParent,lpszKeyName,REG_NONE,REG_OPTION_NON_VOLATILE,samDesired);
}

bool CRegKeyEx::QueryValue(LPCTSTR pszValueName,CString &str)
{
  DWORD dwLen,dwType;
  bool rc=(ERROR_SUCCESS==::RegQueryValueEx(m_hKey,pszValueName,NULL,&dwType,NULL,&dwLen)) && (REG_SZ==dwType||REG_EXPAND_SZ==dwType);
  if(rc){
    rc&=(ERROR_SUCCESS==CRegKey::QueryValue(str.GetBuffer(1+dwLen),pszValueName,&dwLen));
    str.ReleaseBuffer();
  }
  return rc;
}

bool CRegKeyEx::QueryValue(int nIndex,CString &strName,CString &strValue)
{
  DWORD dwType,dwMaxValueNameLen,dwMaxValueLen;
  bool rc=(ERROR_SUCCESS==::RegQueryInfoKey(m_hKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&dwMaxValueNameLen,&dwMaxValueLen,NULL,NULL));
  if(rc){
    dwMaxValueNameLen++;
    dwMaxValueLen++;
    rc&=(ERROR_SUCCESS==::RegEnumValue(m_hKey,nIndex,strName.GetBuffer(dwMaxValueNameLen),&dwMaxValueNameLen,NULL,&dwType,(LPBYTE)strValue.GetBuffer(dwMaxValueLen),&dwMaxValueLen)) && (REG_SZ==dwType||REG_EXPAND_SZ==dwType);
    strName.ReleaseBuffer();
    strValue.ReleaseBuffer();
  }
  return rc;
}

bool CRegKeyEx::QueryValue(LPCTSTR pszValueName,DWORD &dwValue)
{
  DWORD dwType;
  DWORD dwLen=sizeof DWORD;
  return (ERROR_SUCCESS==::RegQueryValueEx(m_hKey,pszValueName,NULL,&dwType,(LPBYTE)&dwValue,&dwLen) && REG_DWORD==dwType);
}

bool CRegKeyEx::QueryValue(int nIndex,CString &strName,DWORD &dwValue)
{
  DWORD dwType,dwMaxValueNameLen;
  DWORD dwValueLen=sizeof(DWORD);
  bool rc=(ERROR_SUCCESS==::RegQueryInfoKey(m_hKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&dwMaxValueNameLen,NULL,NULL,NULL));
  if(rc){
    rc&=(ERROR_SUCCESS==::RegEnumValue(m_hKey,nIndex,strName.GetBuffer(1+dwMaxValueNameLen),&dwMaxValueNameLen,NULL,&dwType,(LPBYTE)&dwValue,&dwValueLen)) && (REG_DWORD==dwType);
    strName.ReleaseBuffer();
  }
  return rc;
}

CRegKeyEx::Value CRegKeyEx::QueryValue(LPCTSTR pszValueName)
{
  DWORD dwType;
  if(ERROR_SUCCESS!=::RegQueryValueEx(m_hKey,pszValueName,NULL,&dwType,NULL,NULL)){
    dwType=REG_NONE;
  }
  switch(dwType){
    case REG_SZ:
      {
        CString strValue;
        return QueryValue(pszValueName,strValue)?Value(strValue):Value();
      }
      break;
    case REG_DWORD:
      {
        DWORD dw;
        return QueryValue(pszValueName,dw)?Value(dw):Value();
      }
      break;
    default:
      return Value();
      break;
  }
}

CRegKeyEx::Value CRegKeyEx::QueryValue(int nIndex,CString &strName)
{
  DWORD dwType;
  if(ERROR_SUCCESS!=::RegEnumValue(m_hKey,nIndex,NULL,NULL,NULL,&dwType,NULL,NULL)){
    dwType=REG_NONE;
  }
  switch(dwType){
    case REG_SZ:
      {
        CString strValue;
        return QueryValue(nIndex,strName,strValue)?Value(strValue):Value();
      }
      break;
    case REG_DWORD:
      {
        DWORD dw;
        return QueryValue(nIndex,strName,dw)?Value(dw):Value();
      }
      break;
    default:
      return Value();
      break;
  }
}

bool CRegKeyEx::QueryKey(int nIndex,CString &strName)
{
  DWORD dwNameLen;
  bool rc=(ERROR_SUCCESS==::RegQueryInfoKey(m_hKey,NULL,NULL,NULL,NULL,&dwNameLen,NULL,NULL,NULL,NULL,NULL,NULL));
  if(rc){
    dwNameLen++;
    rc&=(ERROR_SUCCESS==::RegEnumKey(m_hKey,nIndex,strName.GetBuffer(dwNameLen),dwNameLen));
    strName.ReleaseBuffer();
  }
  return rc;
}
