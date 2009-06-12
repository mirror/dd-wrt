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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/09/11
// Version:		0.01
// Purpose:		Class to encapsulate filename operations (i.e. broadly on a file which probably do not involve opening
//              it).  Importantly, the + and += operators performs filename segment addition, making sure that only one '\\'
//				comes between each piece.
//              Translated to wxWindows conventions by julians (in progress)
// Description:	Interface of the filename class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_FILENAME_H_
#define _ECOS_FILENAME_H_

#include "wx/wxchar.h"
#include "wx/dir.h"

/*
 * ecFileName
 *
 * This class extends wxString with functionality related
 * to filenames.
 */

class ecFileName : public wxString {
public:
    void ReplaceExtension (const wxString& newExt);
    
    // previous directory is returned:
    static ecFileName SetCurrentDirectory (const wxChar* pszDir);
    const wxString Root() const;
    const wxString Extension() const;
    static ecFileName GetTempPath();
    bool IsAbsolute() const;
    
    static const wxChar cSep;        // The path separator ('\' on windows)
    
    // Standard ctors
    ecFileName():wxString(){}
    ecFileName(const ecFileName& stringSrc):wxString(stringSrc){Normalize();}
    ecFileName(const wxString& stringSrc):wxString(stringSrc){Normalize();}
    ecFileName(wxChar ch, int nRepeat = 1):wxString(ch,nRepeat){Normalize();}

    ecFileName(/*LPCSTR*/ const wxChar* lpsz):wxString(lpsz){Normalize();}

    /* TODO
    ecFileName(LPCWSTR lpsz):wxString(lpsz){Normalize();}
    */
    
    ecFileName(/*LPCSTR*/ const wxChar* lpch, int nLength):wxString(lpch,nLength){Normalize();}

    /* TODO
    ecFileName(LPCWSTR lpch, int nLength):wxString(lpch,nLength){Normalize();}
    */
    ecFileName(const unsigned char* psz):wxString(psz){Normalize();}

    // Construct from path fragments
    ecFileName(const wxChar*,const wxChar*);
    ecFileName(const wxChar*,const wxChar*,const wxChar*);
    ecFileName(const wxChar*,const wxChar*,const wxChar*,const wxChar*);
    ecFileName(const wxChar*,const wxChar*,const wxChar*,const wxChar*,const wxChar*);

    //~ecFileName();

    // catenation operators: exactly one separator is placed between L and R
    const ecFileName& operator+=(const ecFileName& string);
    const ecFileName& operator+=(wxChar ch);
#ifdef _UNICODE
    const ecFileName& operator+=(char ch);
#endif
    const ecFileName& operator+=(const wxChar* lpsz);
    friend ecFileName operator+(const ecFileName& string1,const ecFileName& string2);
    friend ecFileName operator+(const ecFileName& string, wxChar ch);
    friend ecFileName operator+(wxChar ch, const ecFileName& string);
#ifdef _UNICODE
    friend ecFileName operator+(const ecFileName& string, char ch);
    friend ecFileName operator+(char ch, const ecFileName& string);
#endif
    friend ecFileName operator+(const ecFileName& string, const wxChar* lpsz);
    friend ecFileName operator+(const wxChar* lpsz, const ecFileName& string);

    // Textual append - no separator functionality
    const ecFileName& Append (wxChar ch);
    const ecFileName& Append (const wxChar* psz);

    // Utility functions
    const ecFileName FullName() const;   // full path name
    const ecFileName NoSpaceName() const;// sans spaces
    const ecFileName ShortName() const;	// the type with ~s in it
    const ecFileName Tail() const;       // file name sans directory part
    const ecFileName Head() const;       // directory part
    const ecFileName CygPath() const;    // path mangled for CygWin

    static ecFileName GetCurrentDirectory();
    const ecFileName& ExpandEnvironmentStrings();
    static ecFileName ExpandEnvironmentStrings(const wxChar* psz);

    // Form path name relative to given parameter (if NULL, current directory)
    const ecFileName& MakeRelative(const wxChar* pszRelativeTo=0);
    static ecFileName Relative(const wxChar* psz,const wxChar* pszRelativeTo=0);

    bool SameFile (const ecFileName &strOther) const;

    /* TODO
    bool SetFileAttributes (long dwFileAttributes) const;
    
      long Attributes() const { return ::GetFileAttributes(*this); }
    */
    
    bool Exists     () const ;
    bool IsDir      () const ;
    bool IsFile     () const ;
    bool IsReadOnly () const ;
    
    time_t LastModificationTime() const;
    
    bool RecursivelyDelete(); 
    
    bool CreateDirectory (bool bParentsToo=true,bool bFailIfAlreadyExists=false) const;
    
    static int FindFiles (const wxString& pszDir,wxArrayString &ar,const wxString& pszPattern=wxT("*.*"),bool bRecurse=TRUE,long dwExclude=wxDIR_DIRS|wxDIR_HIDDEN);
    
protected:
    
   /* Don't appear to be necessary
    // Helpers for Relative():
    const wxChar* *Chop ();
    static ecFileName Drive(const wxChar* psz);
    */
    
    // Remove trailing '/' (helper for ctors)
    void Normalize();
    
    // Implementating catenation functionality:
    void ConcatInPlace(const wxString& src);
    void ConcatCopy(const wxString& src1, const wxString& src2);
};

class wxSaveExcursion {
    const ecFileName m_strPrevDir;
public:
    wxSaveExcursion(const wxChar* pszDir) : m_strPrevDir(ecFileName::SetCurrentDirectory(pszDir)) {}
    ~wxSaveExcursion() { ecFileName::SetCurrentDirectory(m_strPrevDir); }
    bool Ok() const { return !m_strPrevDir.IsEmpty(); }
};

#endif
// _ECOS_FILENAME_H_
