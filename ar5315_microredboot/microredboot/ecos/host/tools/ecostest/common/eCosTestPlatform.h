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
//        eCosTestPlatform.h
//
//        platform information header
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          2000-04-01
// Description:   eCosTestPlatform
// Usage:
//
//####DESCRIPTIONEND####

#ifndef _CeCosTestPlatform_H
#define _CeCosTestPlatform_H

#include "eCosStd.h"
#include "Collections.h"
#include "Properties.h"
#include <vector>

//=================================================================
// This class holds properties associated with a platform type (i.e. common to all instances of that platform)
// The information is read from a .eCosrc file or from the registry.
// An instance of a platform corresponds to the class CTestResource.
//=================================================================

class CeCosTestPlatform {
  class CeCosTestPlatformProperties : public CProperties {
  public:
    CeCosTestPlatformProperties(CeCosTestPlatform *pti);
    virtual ~CeCosTestPlatformProperties(){}
  protected:
  };
  friend class CeCosTestPlatformProperties;
public:

  static bool Load();
	static bool Save();

  bool IsValid()    const { return NULL!=Get(m_strName); }
  LPCTSTR Name()    const { return m_strName.c_str(); }
  LPCTSTR Prefix()  const { return m_strPrefix.c_str(); }
  LPCTSTR GdbCmds() const { return m_strCommands.c_str(); }
  LPCTSTR Prompt()  const { return m_strPrompt.c_str(); }
  LPCTSTR Inferior()const { return m_strInferior.c_str(); }
  bool ServerSideGdb() const { return 0!=m_nServerSideGdb; }
  CeCosTestPlatform():m_nServerSideGdb(0){}
	bool LoadFromCommandString(LPCTSTR psz);
  CeCosTestPlatform(LPCTSTR pszIm,LPCTSTR pszPre,LPCTSTR pszPrompt,LPCTSTR pszGdb,bool bServerSideGdb,LPCTSTR pszInferior):
    m_strName(pszIm),
    m_strPrefix(pszPre),
    m_strCommands(pszGdb),
    m_strPrompt(pszPrompt),
    m_nServerSideGdb(bServerSideGdb),
    m_strInferior(pszInferior)
  {}
  static int Add (const CeCosTestPlatform &t);

  static unsigned int Count() { return (unsigned)arPlatforms.size(); }
  
  // Get a platform by name:
  static const CeCosTestPlatform *Get(LPCTSTR t);

  // This is only used to enumerate the available platforms:
  static const CeCosTestPlatform *Get(unsigned int i) { return (i<Count())?&arPlatforms[i]:0; }

  static void RemoveAllPlatforms();

  static bool IsValid (LPCTSTR pszTarget) { return NULL!=Get(pszTarget); }

protected:
	static bool LoadFromDir (LPCTSTR pszDir);
	static bool SaveToDir (LPCTSTR pszDir);
#ifdef _WIN32
	bool   LoadFromRegistry(HKEY hKey,LPCTSTR pszKey);
  static const String GetGreatestSubkey (LPCTSTR pszKey);
  static bool SaveToRegistry(HKEY hKey,LPCTSTR pszKey);
#endif
  String m_strName;
  String m_strPrefix;
  String m_strCommands;
  String m_strPrompt;
  int    m_nServerSideGdb;
  String m_strInferior;
  static std::vector<CeCosTestPlatform> arPlatforms;
};

#endif
