//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
// Copyright (C) 2003 John Dallaway
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
//        eCosTestPlatform.cpp
//
//        platform information implementation
//
//=================================================================
#include "eCosTestPlatform.h"
#include "eCosTestUtils.h"
#include "eCosTrace.h"

std::vector<CeCosTestPlatform> CeCosTestPlatform::arPlatforms;

const CeCosTestPlatform *CeCosTestPlatform::Get(LPCTSTR psz) 
{ 
  for(int i=0;i<(signed)arPlatforms.size();i++){
    const CeCosTestPlatform &t=arPlatforms[i];
    if(0==_tcsicmp(t.Name(),psz)){
      return &t;
    }
  }
  return NULL;
}

CeCosTestPlatform::CeCosTestPlatformProperties::CeCosTestPlatformProperties(CeCosTestPlatform *pti)
{
  Add(_T("platform"),pti->m_strName);
  Add(_T("prefix"),  pti->m_strPrefix);
  Add(_T("commands"),pti->m_strCommands);
  Add(_T("inferior"),pti->m_strInferior);
  Add(_T("prompt"),  pti->m_strPrompt);
  Add(_T("ServerSideGdb"),pti->m_nServerSideGdb);
}

bool CeCosTestPlatform::LoadFromDir(LPCTSTR pszDir)
{
  bool rc=true;
  TRACE(_T("CeCosTestPlatform::LoadFromDir %s\n"),pszDir);
  // Find all the files in directory pszDir and load from each of them
  TCHAR szOrigDir[256];
  _tgetcwd(szOrigDir,sizeof szOrigDir-1);
  if(0==_tchdir(pszDir)){
    String strFile;
    void *pHandle;
    for(bool b=CeCosTestUtils::StartSearch(pHandle,strFile);b;b=CeCosTestUtils::NextFile(pHandle,strFile)){
      if(CeCosTestUtils::IsFile(strFile)){
        CeCosTestPlatform t;
        t.m_strName=strFile;
        CeCosTestPlatformProperties prop(&t);
        if(prop.LoadFromFile(strFile)){
          Add(t);
        } else {
          ERROR(_T("Illegal platform specification in %s%c%s\n"),pszDir,cPathsep,(LPCTSTR)strFile);
          rc=false;
        }
      }
    }
    CeCosTestUtils::EndSearch(pHandle);
  } else {
    ERROR(_T("Failed to change to %s from %s\n"),pszDir,szOrigDir);
  }
  _tchdir(szOrigDir);

  return rc;
}

#ifdef _WIN32
bool CeCosTestPlatform::SaveToRegistry(HKEY hTopKey,LPCTSTR pszKey)
{
  // save target info to the registry

  CProperties::CreateKey(pszKey,hTopKey);
  HKEY hKey;
  bool rc=ERROR_SUCCESS==RegOpenKeyEx (hTopKey, pszKey, 0L, KEY_ALL_ACCESS, &hKey);
  if(rc){
    for(int i=0;i<(signed)arPlatforms.size();i++){
      HKEY hKey2;
      DWORD dwDisp;
      const CeCosTestPlatform &ti=arPlatforms[i];
      rc&=(ERROR_SUCCESS==RegCreateKeyEx(hKey,ti.Name(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey2, &dwDisp));
      if(rc){
        LPCTSTR pszPrefix=ti.Prefix();
        LPCTSTR pszGdb   =ti.GdbCmds();
        LPCTSTR pszInferior=ti.Inferior();
        LPCTSTR pszPrompt=ti.Prompt();
        DWORD dwServerSideGdb=ti.ServerSideGdb()?1l:0l;
        rc&=(ERROR_SUCCESS==RegSetValueEx(hKey2,_T("Prefix"),0,REG_SZ,   (CONST BYTE *)pszPrefix,(1+_tcslen(pszPrefix))*sizeof (TCHAR))) &&
            (ERROR_SUCCESS==RegSetValueEx(hKey2,_T("Commands"),0,REG_SZ,   (CONST BYTE *)pszGdb,(1+_tcslen(pszGdb))*sizeof (TCHAR))) &&
            (ERROR_SUCCESS==RegSetValueEx(hKey2,_T("Inferior"),0,REG_SZ,   (CONST BYTE *)pszInferior,(1+_tcslen(pszInferior))*sizeof (TCHAR))) &&
            (ERROR_SUCCESS==RegSetValueEx(hKey2,_T("Prompt"),0,REG_SZ,   (CONST BYTE *)pszPrompt,(1+_tcslen(pszPrompt))*sizeof (TCHAR))) &&
            (ERROR_SUCCESS==RegSetValueEx(hKey2,_T("ServerSideGdb"),0,REG_DWORD,   (CONST BYTE *)&dwServerSideGdb,sizeof (DWORD)));
      }
      RegCloseKey(hKey2);
    }
    RegCloseKey(hKey);
  }
  return rc;
}

const String CeCosTestPlatform::GetGreatestSubkey (LPCTSTR pszKey)
{
  String strSubkey;
  HKEY hKey;
  
  if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, pszKey, 0L, KEY_READ, &hKey)) {
    DWORD dwIndex = 0;
    TCHAR pszBuffer [MAX_PATH + 1];
    
    while (ERROR_SUCCESS == RegEnumKey (hKey, dwIndex++, (LPTSTR) pszBuffer, sizeof (pszBuffer))) {
      if (strSubkey.compare (pszBuffer) < 0) {
        strSubkey = pszBuffer;
      }
    }
    
    RegCloseKey (hKey);
  }
  
  TRACE (_T("CeCosTestPlatform::GetGreatestSubkey(\"%s\"): %s\n"), pszKey, (LPCTSTR)strSubkey);
  return strSubkey;
}
#endif

bool CeCosTestPlatform::Load()
{
  TRACE(_T("CeCosTestPlatform::Load\n"));
  srand( (unsigned)time( NULL ) );
  
#ifdef _WIN32

  // get target info from the registry
  String strPlatformsKey = _T("Software\\eCos Configuration Tool\\Platforms");
//  strPlatformsKey += GetGreatestSubkey (_T("Software\\eCos"));
//  strPlatformsKey += _T("\\Platforms");

  HKEY hKey;
  bool rc=ERROR_SUCCESS==RegOpenKeyEx (HKEY_CURRENT_USER, strPlatformsKey, 0L, KEY_READ, &hKey);
  DWORD dwSubKeys=0;
  if(rc){
    // Found the given key.
    // Subkeys' names are the target image names:
    // Subkeys's values are:
    //      Prefix  String
    //      Type    String 
    //      GdbCmd  String [optional]
    FILETIME ftLastWriteTime;
    DWORD dwMaxSubKeyLen;
    if(ERROR_SUCCESS==RegQueryInfoKey(hKey,NULL,NULL,NULL,&dwSubKeys,&dwMaxSubKeyLen,NULL,NULL,NULL,NULL,NULL,NULL)){
      TCHAR *szName=new TCHAR[1+dwMaxSubKeyLen];
      DWORD dwSizeName=sizeof(TCHAR)*(1+dwMaxSubKeyLen);
      for(DWORD dwIndex=0;ERROR_SUCCESS==RegEnumKeyEx(hKey, dwIndex, szName, &dwSizeName, NULL, NULL, NULL, &ftLastWriteTime); dwIndex++){
        CeCosTestPlatform t;
        if(t.LoadFromRegistry(hKey,szName)){
          t.m_strName=szName;
          CeCosTestPlatform::Add(t);
        }
        dwSizeName=sizeof(TCHAR)*(1+dwMaxSubKeyLen);
      }
      delete [] szName;
    }
    RegCloseKey(hKey);
  }
#endif
  const String strDir(CeCosTestUtils::HomeFile(_T(".eCosPlatforms")));
#ifdef _WIN32
  if(!CeCosTestUtils::Exists(strDir)){
    return true;
  }
#endif
  LoadFromDir(strDir);
  if(0==Count()){
    ERROR(_T("Failed to initialize any targets\n"));
  }
  return true;
}

int CeCosTestPlatform::Add(const CeCosTestPlatform &t)
{
  for(std::vector<CeCosTestPlatform>::iterator it=arPlatforms.begin();it!=arPlatforms.end();){
    if(0==_tcsicmp(it->Name(),t.Name())){
      // Careful - there's already something here with this name:
      ERROR(_T("Warning: duplicate target info %s\n"),it->Name());
      it=arPlatforms.erase(it);
    } else {
      it++;
    }
  }
  arPlatforms.push_back(t);
  return arPlatforms.size()-1;
}

void CeCosTestPlatform::RemoveAllPlatforms()
{
  arPlatforms.clear();
}

bool CeCosTestPlatform::LoadFromCommandString(LPCTSTR psz)
{
  CeCosTestPlatformProperties prop(this);
  return prop.LoadFromCommandString(psz);
}

#ifdef _WIN32
bool CeCosTestPlatform::LoadFromRegistry(HKEY hKey,LPCTSTR pszKey)
{
  CeCosTestPlatformProperties prop(this);
  return prop.LoadFromRegistry(hKey,pszKey);
}
#endif

bool CeCosTestPlatform::Save()
{
  const String strDir(CeCosTestUtils::HomeFile(_T(".eCosPlatforms")));
#ifdef _WIN32
  if(!CeCosTestUtils::Exists(strDir)){
    String strPlatformsKey = _T("Software\\eCos Configuration Tool\\Platforms");
//    strPlatformsKey += GetGreatestSubkey (_T("Software\\eCos"));
//    strPlatformsKey += _T("\\Platforms");

    return SaveToRegistry(HKEY_CURRENT_USER,strPlatformsKey);
  }
#endif
  return SaveToDir(strDir);
}

bool CeCosTestPlatform::SaveToDir (LPCTSTR pszDir)
{
  bool rc=false;
  void *pHandle;
  TCHAR szOrigDir[256];
  _tgetcwd(szOrigDir,sizeof szOrigDir-1);
  if(0==_tchdir(pszDir)){
    // Delete all the files under directory "pszDir"
    String strFile;
    for(bool b=CeCosTestUtils::StartSearch(pHandle,strFile);b;b=CeCosTestUtils::NextFile(pHandle,strFile)){
      if(CeCosTestUtils::IsFile(strFile)){
        _tunlink(strFile);
      }
    }
    CeCosTestUtils::EndSearch(pHandle);
    rc=true;
    // Rewrite the files
    for(int i=0;i<(signed)arPlatforms.size();i++){
      CeCosTestPlatform &t=arPlatforms[i];
      CeCosTestPlatformProperties prop(&t);
      rc&=prop.SaveToFile(t.Name());
    }
  } else {
    ERROR(_T("Failed to change to %s from %s\n"),pszDir,szOrigDir);
  }
  
  return rc;
}


