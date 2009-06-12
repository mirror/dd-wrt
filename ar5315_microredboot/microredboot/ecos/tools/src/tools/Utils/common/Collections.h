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

// ----------------------------------------------------------------------------
// This file defines some useful collection classes:
//   String (a slightly extended string class, based on TCHAR)
//   StringArray (array of the above)
//   PtrArray (array of pointers)
//   IntArray (array of ints)
//   Buffer (untyped memory)
// ----------------------------------------------------------------------------

#ifndef _ECOS_COLLECTIONS_H
#define _ECOS_COLLECTIONS_H
#ifdef _MSC_VER
  // Some standard warning-suppressions to avoid STL header verbosity:
  #pragma warning (push)
  #pragma warning(disable:4018) // signed/unsigned mismatch
  #pragma warning(disable:4097) // typedef-name 'string' used as synonym for class-name 
  #pragma warning(disable:4100) // unreferenced formal parameter
  #pragma warning(disable:4146) // unary minus operator applied to unsigned type, result still unsigned
  #pragma warning(disable:4189) // local variable is initialized but not referenced
  #pragma warning(disable:4244) // conversion from 'unsigned int' to 'char', possible loss of data
  #pragma warning(disable:4250) // CdlConfigurationBody' : inherits 'CdlToplevelBody::is_active' via dominance
  #pragma warning(disable:4284) // return type for (ie; not a UDT or reference to a UDT.  Will produce errors if applied using infix notation)
  #pragma warning(disable:4290) // C++ Exception Specification ignored
  #pragma warning(disable:4503) // decorated name length exceeded, name was truncated
  #pragma warning(disable:4511) // copy constructor could not be generated
  #pragma warning(disable:4512) // assignment operator could not be generated
  #pragma warning(disable:4663) // C++ language change: to explicitly specialize class template...
#endif

#include <string>
#include <vector>

#include "eCosStd.h"

class String;

// An array of strings:
typedef std::vector<String> StringArray;
// An array of integers:
typedef std::vector<int>    IntArray;

// Some extensions to the STL string class.
// The semantics of the like-named functions is as for the MFC class CString.
// The instantiation of the string class is based on TCHAR (the typedef is just below)
// which of course will be a wide character when building UNICODE on Windows.

typedef std::basic_string<TCHAR> string;
class String : public string {
public:
	void Replace (LPCTSTR psz1,LPCTSTR psz2,bool bObserveEscapes=false);
  // Standard ctors
  String() : string(),m_pszBuf(0){}
  String(const String& rhs) : string(rhs),m_pszBuf(0){}
  String(const String& rhs, size_type pos, size_type n) : string(rhs,pos,n),m_pszBuf(0){}
  String(const TCHAR *s, size_type n) : string(s?s:_T(""),n),m_pszBuf(0){}
  String(const TCHAR *s) : string(s?s:_T("")),m_pszBuf(0){}
  String(size_type n, TCHAR c) : string(n,c),m_pszBuf(0){}
  String(const_iterator first, const_iterator last) : string(first,last),m_pszBuf(0){}
  virtual ~String() { delete [] m_pszBuf; }

  // Comparators
  bool operator==(const String& str) const {return 0==compare(str); }
  bool operator==(const LPCTSTR psz) const {return 0==compare(psz); }
  
  // Implicit conversion to LPCTSTR
	operator LPCTSTR () const { return c_str(); }

  // Access to the buffer
  LPTSTR  GetBuffer (unsigned int nLength=0);
  void ReleaseBuffer();
  
  // Format the contents of a string, as printf would do it:
  void Format(LPCTSTR pszFormat,...);
  static String SFormat(LPCTSTR pszFormat,...); 

  // Tokenize (split into pieces at separator cSep).
  // The bObserveStrings argument controls whether double quotes can be used to group words
  int Chop(StringArray &ar,TCHAR cSep=_TCHAR(' '),bool bObserveStrings=true) const;

  // UNICODE-ANSI conversions:
  char * GetCString () const;
  static String CStrToUnicodeStr(const char *psz);

  void vFormat(LPCTSTR  pszFormat, va_list marker);

protected:

  TCHAR *m_pszBuf;
  int   m_nBufferLength;
};

// Use this class to allocate chunks of untyped memory without needing to worry about memory leaks:
class Buffer {
public:
  Buffer(unsigned int nSize) : m_nSize(nSize), pData(malloc(nSize)) {}
  ~Buffer() { free(pData); }
  void *Data() { return pData; }
  void Resize(int nSize) { pData=realloc(pData,nSize); m_nSize=nSize; }
  unsigned int Size() const { return m_nSize; }
protected:
  unsigned int m_nSize;
  void *pData;
};

// An array of untyped pointers:
typedef std::vector<void *> PtrArray;

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

#endif
