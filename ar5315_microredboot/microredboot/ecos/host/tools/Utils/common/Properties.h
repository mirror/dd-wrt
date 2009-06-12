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
// Properties.h: interface for the CProperties class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROPERTIES_H__DA938D29_135A_11D3_A50B_00A0C949ADAC__INCLUDED_)
#define AFX_PROPERTIES_H__DA938D29_135A_11D3_A50B_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "eCosStd.h"
#include "Collections.h"

//////////////////////////////////////////////////////////////////////
// This class manages properties and their serialization.
// What you do is this:
//   1. Declare the CProperties object
//   2. Call one or more "Add" functions to associate variables with names
//   3. Call one of the "Load" or "Save" functions to load or save the values
// There are three ways in which the "Load" or "Save" functions operate
//   a. Loading/saving to/from a file [LoadFromFile/SaveToFile]
//      In this case the contents of the file will look like
//        name1=value1
//        name2=value2
//        ...
//   b. Loading/saving to/from a command string [LoadFromCommandString/MakeCommandString]
//      In this case the contents of the string will look like
//        -name1=value1 -name2=value2 ...
//   c. Loading/saving to/from the registry [LoadFromRegistry/SaveToRegistry]
//      In this case the registry will contain a set of values (name1, name2,...) each of whose
//      value data is the corresponding value.
//////////////////////////////////////////////////////////////////////

class CProperties  
{
public:
  CProperties();
  virtual ~CProperties();

  // Declare various types of property.  The functions associate names with 
  // variables, but no values are assigned here.  That comes later when a
  // load function (such as LoadFromRegistry) is called.
  void Add (LPCTSTR pszName,int &n);
  void Add (LPCTSTR pszName,unsigned int &n);
  void Add (LPCTSTR pszName,bool &b);
  void Add (LPCTSTR pszName,char &c);
  void Add (LPCTSTR pszName,unsigned char &c);
  void Add (LPCTSTR pszName,short&s);
  void Add (LPCTSTR pszName,unsigned short&s);
  void Add (LPCTSTR pszName,float&f);
  void Add (LPCTSTR pszName,double&f);
  void Add (LPCTSTR pszName,String &s);
  void Add (LPCTSTR pszName,void *d,unsigned int nLength);
  
  // Load from and save to a command string.
  String MakeCommandString () const; // caller must delete []
  bool LoadFromCommandString (LPCTSTR psz);

  // Load from and save to a given file.  The format is name=value, one per line.
  bool LoadFromFile(LPCTSTR pszFileName);
  bool SaveToFile (LPCTSTR pszFileName) const;

#ifdef _WIN32
  bool LoadFromRegistry (HKEY,LPCTSTR);
  bool SaveToRegistry(HKEY,LPCTSTR) const;
  static bool CreateKey (LPCTSTR pszKey,HKEY hKey=HKEY_CURRENT_USER);
#endif

protected:
  static bool CreatePathToFile(LPCTSTR pszDir);
  
  class CProperty {
  public:
    enum Typetype {Integer, szString, Bool, Char, Short, Float, Double, Void};
    CProperty(LPCTSTR pszName,Typetype type,void *_pData);
    virtual ~CProperty();
    friend class CProperties;
    bool  SetValue(int n);
    bool  SetValue(double n);
    bool  SetValue(void *d,int nLength);
    bool  SetValue(LPCTSTR psz);
    const String GetStringValue() const;
    unsigned long GetValue() const;
    
  protected:
    String strName;
    Typetype Type;
    void *pData;
    unsigned int nLength; // for Void
  };
  CProperty * Lookup (LPCTSTR pszName);
  std::vector<CProperty> ar; // Holds declared properties
};

#endif // !defined(AFX_PROPERTIES_H__DA938D29_135A_11D3_A50B_00A0C949ADAC__INCLUDED_)
