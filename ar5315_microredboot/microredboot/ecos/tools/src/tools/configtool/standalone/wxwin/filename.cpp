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

#ifdef __GNUG__
#pragma implementation "filename.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/filefn.h"
#include "wx/confbase.h" // For wxExpandEnvVars

#include "filename.h"

#ifdef __WXMSW__
//#include <direct.h>
//#include <dos.h>

#include <windows.h>
#include <shlwapi.h>

#include "wx/msw/winundef.h"
#ifdef CreateDirectory
#undef CreateDirectory
#endif
#ifdef ExpandEnvironmentStrings
#undef ExpandEnvironmentStrings
#endif
#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#endif
#ifdef SetCurrentDirectory
#undef SetCurrentDirectory
#endif
#ifdef GetTempPath
#undef GetTempPath
#endif

#else

// #include <dir.h>
#endif

#include <sys/stat.h>

#define wxTChar wxT

const wxChar ecFileName::cSep=wxFILE_SEP_PATH;

ecFileName::ecFileName(const wxChar* psz1,const wxChar* psz2):
wxString(psz1)
{
    Normalize();
    operator+=(psz2);
}

ecFileName::ecFileName(const wxChar* psz1,const wxChar* psz2,const wxChar* psz3):
wxString(psz1)
{
    Normalize();
    operator+=(psz2);
    operator+=(psz3);
}

ecFileName::ecFileName(const wxChar* psz1,const wxChar* psz2,const wxChar* psz3,const wxChar* psz4):
wxString(psz1)
{
    Normalize();
    operator+=(psz2);
    operator+=(psz3);
    operator+=(psz4);
}

ecFileName::ecFileName(const wxChar* psz1,const wxChar* psz2,const wxChar* psz3,const wxChar* psz4,const wxChar* psz5):
wxString(psz1)
{
    operator+=(psz2);
    operator+=(psz3);
    operator+=(psz4);
    operator+=(psz5);
}

/*
ecFileName::~ecFileName()
{
    // Unfortunately we can't do this since GetStringData is private in wxString.
    // So for now, we may memory leaks since ~wxString is not virtual.
    //GetStringData()->Unlock();
}
*/

ecFileName operator+(const ecFileName& string1, const ecFileName& string2)
{
    ecFileName s;
    s.ConcatCopy(string1, string2);
    return s;
}

ecFileName operator+(const ecFileName& string, const wxChar* lpsz)
{
    if (lpsz == NULL)
        return string;

    ecFileName s;
    s.ConcatCopy(string, wxString(lpsz));
    return s;
}

ecFileName operator+(const wxChar* lpsz, const ecFileName& string)
{
    if (lpsz == NULL)
        return string;

    ecFileName s;
    s.ConcatCopy(wxString(lpsz), string);
    return s;
}

ecFileName operator+(const ecFileName& string1, wxChar ch)
{
    ecFileName s;
    s.ConcatCopy(string1, wxString(ch));
    return s;
}

ecFileName operator+(wxChar ch, const ecFileName& string)
{
    ecFileName s;
    s.ConcatCopy(wxString(ch), string);
    return s;
}

const ecFileName& ecFileName::operator+=(const wxChar* lpsz)
{
    if (lpsz == NULL)
        return *this;

    ConcatInPlace(wxString(lpsz));
    return *this;
}

const ecFileName& ecFileName::operator+=(wxChar ch)
{
    ConcatInPlace(wxString(ch));
    return *this;
}

const ecFileName& ecFileName::operator+=(const ecFileName& string)
{
    ConcatInPlace(string);
    return *this;
}

void ecFileName::ConcatInPlace(const wxString& src)
{
    ConcatCopy(* this, src);
}

void ecFileName::ConcatCopy(const wxString& src1,
                            const wxString& src2)
{
    int nSrc1Len = src1.Len();
    int nSrc2Len = src2.Len();
    int n=1;
    int nNewLen = nSrc1Len + nSrc2Len;
    if(nSrc1Len>0)
    {
        if(1==nSrc2Len && cSep==src2[0])
        {
            // Appending a single separator to a non-null string is a no-op
            return;
        } else {
            // Count the intervening separators
            n=(cSep==src1[nSrc1Len-1])+(cSep==src2[0]);
            
            switch(n){
            case 0:
                (*this) = src1 + wxString(cSep) + src2;
                break;
            case 1:
                (*this) = src1 + src2;
                break;
            case 2:
                (*this) = src1 + src2.Mid(1);
                break;
            }
            return;
        }
    }
}

void ecFileName::Normalize()
{
    // Remove any trailing seperator
    int len = Len();
    if (len > 0 && (*this)[(size_t)(len - 1)] == cSep)
        (*this) = Mid(0, len - 1);
}


const ecFileName ecFileName::FullName() const
{
#ifdef __WXMSW__
    TCHAR* pFile;
    long dwSize=::GetFullPathName (*this, 0, NULL, &pFile);
    if(dwSize>0){
        wxString strCopy;
        ::GetFullPathName (*this, 1+dwSize, strCopy.GetWriteBuf(1+dwSize), &pFile);
        strCopy.UngetWriteBuf();
        return strCopy;
    } else {
        return *this;
    }
#else
    return wxGetCwd() + wxString(cSep) + wxString(*this);
#endif
}

const ecFileName ecFileName::ShortName() const
{
#ifdef __WXMSW__
    long dwSize=::GetShortPathName (*this, NULL, 0);
    if(dwSize>0){
        wxString strCopy;
        ::GetShortPathName (*this, strCopy.GetWriteBuf(1+dwSize), 1+dwSize);
        strCopy.UngetWriteBuf();
        return strCopy;
    } else {
        return *this;
    }
#else
    return *this;
#endif
}

const ecFileName ecFileName::NoSpaceName() const// sans spaces
{
#ifndef __WXMSW__
    return *this;
#else
    // Only replace components with spaces with FAT names.
    wxArrayString ar1,ar2;
    const wxString str2(ShortName());
    
    size_t i,pcStart;
    
    unsigned int len = Len();
    pcStart = 0;
    for (i = 0; i < len; i++)
    {
        if(wxTChar('\\')==(*this)[i])
        {
            ar1.Add(this->Mid(pcStart, i - pcStart + 1));
            pcStart = i + 1;
        }
    }
    ar1.Add(this->Mid(pcStart, i - pcStart + 1));

    len = str2.Len();
    pcStart = 0;
    for (i = 0; i < len; i++)
    {
        if(wxTChar('\\')==str2[i])
        {
            ar2.Add(str2.Mid(pcStart, i - pcStart + 1));
            pcStart = i + 1;
        }
    }
    ar2.Add(str2.Mid(pcStart, i - pcStart + 1));    
    
    wxASSERT(ar1.Count()==ar2.Count());
    
    wxString rc;
    for(i=0;i<ar1.Count();i++){
        rc+=(-1==ar1[i].Find(wxTChar(' ')))?ar1[i]:ar2[i];
    }
    return rc;
#endif
}

//
const ecFileName ecFileName::Tail() const
{
    return AfterLast(cSep);
}

const ecFileName ecFileName::Head() const
{
    return BeforeLast(cSep);
}

time_t ecFileName::LastModificationTime() const
{
    return wxFileModificationTime(* this);
    
#if 0
    // GetFileAttributes is not available in Win95 so we test the water first
    static HINSTANCE hInst=LoadLibrary(_T("kernel32.dll"));
    wxASSERT(hInst);
    
#ifdef _UNICODE
#define GETFILEATTRIBUTESNAME "GetFileAttributesExW"
#else
#define GETFILEATTRIBUTESNAME "GetFileAttributesExA"
#endif // !UNICODE
    
    typedef BOOL (WINAPI *GetFileAttributesP)(const wxChar*,GET_FILEEX_INFO_LEVELS,LPVOID);
    
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
#endif
}

/* TODO
bool ecFileName::SetFileAttributes(long dwFileAttributes) const
{
return ::SetFileAttributes (*this, dwFileAttributes)!=0;
}
*/

bool ecFileName::Exists     () const
{
    return IsFile() || IsDir() ;
}

bool ecFileName::IsDir      () const
{
    return wxPathExists(* this);
}

bool ecFileName::IsFile     () const
{
#if defined(__WXMAC__)
    struct stat stbuf;
    if (filename && stat (wxUnix2MacFilename(filename), &stbuf) == 0 )
        return TRUE;
    return FALSE ;
#else
    wxStructStat st;
    if ((*this) != wxT("") && wxStat (wxFNSTRINGCAST fn_str(), &st) == 0 && (st.st_mode & S_IFREG))
        return TRUE;

    return FALSE;
#endif
}

bool ecFileName::IsReadOnly () const
{
#ifdef __WXMSW__
    long a=GetFileAttributes(* this); return 0xFFFFFFFF!=a && (0!=(a&FILE_ATTRIBUTE_READONLY ));
#else
    wxFAIL_MSG("ecFileName::IsReadOnly not supported on this platform.");
    return FALSE;
#endif
}

bool ecFileName::SameFile(const ecFileName &o) const
{
#ifdef __WXMSW__
    return 0==ShortName().CmpNoCase(o.ShortName());
#else
    // On most other platforms, case is important.
    return o == (*this);
#endif
}

ecFileName ecFileName::ExpandEnvironmentStrings(const wxChar* psz)
{
    // wxExpandEnvVars is from confbase.h

    ecFileName f = wxExpandEnvVars(psz);
    return f;
}

const ecFileName& ecFileName::ExpandEnvironmentStrings()
{
    *this=ecFileName::ExpandEnvironmentStrings(*this);
    return *this;
}

#if 0
// Helper for Relative()  psz is in full format.
ecFileName ecFileName::Drive(const wxChar* psz)
{
    if(wxIsalpha(psz[0])){
        return psz[0];
    } else if(cSep==psz[0]&&cSep==psz[1]){
        wxChar *c=_tcschr(psz+2,cSep);
        if(c){
            c=_tcschr(c+1,cSep);
            if(c){
                return wxString(psz,c-psz);
            }
        }
    }
    return _T("");
}
#endif

ecFileName ecFileName::Relative(const wxChar* compare,const wxChar* current)
{
#ifdef __WXMSW__
    wxString rc;
    bool b=(TRUE==PathRelativePathTo(rc.GetWriteBuf(1+MAX_PATH),current,FILE_ATTRIBUTE_DIRECTORY,compare,0));
    rc.UngetWriteBuf();
    return b?(ecFileName)rc:(ecFileName)compare;
#else
    wxFAIL_MSG("ecFileName::Relative not implemented on this platform.");
    return ecFileName();
#endif
} 

const ecFileName& ecFileName::MakeRelative(const wxChar* pszRelativeTo)
{
    *this=ecFileName::Relative(*this,pszRelativeTo);
    return *this;
}


ecFileName ecFileName::GetCurrentDirectory()
{
    ecFileName f;
    f = wxGetCwd();
    f.Normalize();

    return f;
}


const ecFileName& ecFileName::Append(const wxChar* lpsz)
{
    if (lpsz)
        return *this;

    //wxString::ConcatInPlace(lpsz);
    wxString::Append(lpsz);
    return *this;
    
}

const ecFileName& ecFileName::Append(wxChar ch)
{
    ConcatInPlace(wxString(ch));
    return *this;
}

bool ecFileName::IsAbsolute() const
{
    int nLength=Len();
    const wxString& psz=*this;
    return 
        (nLength>0 && (cSep==psz[0]))|| // starts with '\'
        (nLength>1 && (
        (wxIsalpha(psz[0]) && wxTChar(':')==psz[1]) ||  // starts with [e.g.] "c:\"
        (cSep==psz[0] && cSep==psz[1])));              // UNC
}

// TODO (?)
#if 0
// Return an array of filename pieces.  Plugs '\0's into 'this', which
// is therefore subsequently only usable as referenced by the returned array.
const wxChar* *ecFileName::Chop()
{
    wxChar *c;
    // Count the separators
    int nSeps=0;
    for(c=_tcschr(m_pchData,cSep);c;c=_tcschr(c+1,cSep)){
        nSeps++;
    }
    const wxChar* *ar=new const wxChar*[2+nSeps]; // +1 for first, +1 for terminating 0
    ar[0]=m_pchData;
    int i=1;
    for(c=_tcschr(m_pchData,cSep);c;c=_tcschr(c+1,cSep)){
        ar[i++]=c+1;
        *c=wxTChar('\0');
    }
    ar[i]=0;
    return ar;
}
#endif

ecFileName ecFileName::GetTempPath()
{
    ecFileName f;
#ifdef __WXMSW__
#ifdef _UNICODE
    ::GetTempPathW(1+MAX_PATH,f.GetWriteBuf(1+MAX_PATH));
#else
    ::GetTempPathA(1+MAX_PATH,f.GetWriteBuf(1+MAX_PATH));
#endif
  f.UngetWriteBuf();
#elif defined(__WXGTK__)
#else
    wxFAIL("ecFileName::GetTempPath() not implemented on this platform.");
#endif
    f.Normalize();
    return f;    
}

const ecFileName ecFileName::CygPath () const 
{
#ifdef __WXMSW__
    ecFileName rc = ShortName();
    if(wxIsalpha(rc[(size_t)0]) && wxTChar(':')==rc[(size_t)1])
    {
        // Convert c:\ to /cygdrive/c/
        wxString s = wxString(wxT("/cygdrive/")) + wxString(rc[(size_t)0]) + rc.Mid(2);
        rc = s;
    }
    size_t i;
    for (i = 0; i < rc.Len(); i++)
    {
        if (rc[i] == wxTChar('\\'))
            rc[i] = wxTChar('/');
    }
#else
    const ecFileName& rc = * this;
#endif

    return rc;
}

bool ecFileName::CreateDirectory(bool bParentsToo,bool bFailIfAlreadyExists) const
{
    if(bParentsToo)
    {
        // Create intermediate directories

        // 'rest' will progressively have its separators replaced by underscores in order
        // to find the next separator
        wxString rest = * this;
        size_t lastPos = 0;
        int len = rest.Len();

#ifdef __WXMSW__
        // If the path is a network path, ignore the first part of the path
        if (len > 2 && (rest.GetChar(0) == wxT('\\') || rest.GetChar(0) == wxT('/')) && (rest.GetChar(1) == wxT('\\') || rest.GetChar(1) == wxT('/')))
        {
            rest.SetChar(0,wxT('_')); rest.SetChar(1,wxT('_'));
            lastPos = rest.Find(cSep);
            if (lastPos != -1 && lastPos >= 0)
                rest.SetChar(lastPos,wxT('_'));
        }
#endif
        
        while (lastPos != -1)
        {
            lastPos = rest.Find(cSep);
            if (lastPos != -1 && lastPos >= 0)
            {
                rest[lastPos] = wxT('_'); // So we find the NEXT separator

                // don't attempt to create "C: or /"
                if (lastPos > 0 && (*this)[lastPos-1] == wxT(':'))
                    continue;
                else if (lastPos == 0)
                    continue;
            }

            // Fail if any of the dirs exist already
            wxString str(this->Mid(0, lastPos));
            bool alreadyExists = wxDirExists(str);
            if (alreadyExists && bFailIfAlreadyExists)
                return FALSE;
            
            if (!alreadyExists)
                if (!wxMkdir(str))
                    return FALSE;
        }
    }


    return IsDir()? (!bFailIfAlreadyExists) : (TRUE==wxMkdir(*this));
}


const wxString ecFileName::Extension() const
{
    wxString path, name, ext;

    wxSplitPath(*this, & path, & name, & ext);
    return ext;
}

const wxString ecFileName::Root() const
{
    return wxPathOnly(*this);
}

ecFileName ecFileName::SetCurrentDirectory(const wxChar* pszDir)
{
    const ecFileName strPwd=wxGetCwd();
    if (::wxSetWorkingDirectory(pszDir))
        return strPwd;
    else
        return wxT("");
}

bool ecFileName::RecursivelyDelete()
{
    wxArrayString ar;
    int i;
    for(i=FindFiles(*this,ar)-1;i>=0;--i){
        wxRemoveFile(ar[i]);
    }
    for(i=FindFiles(*this,ar,wxT("*.*"),TRUE,0)-1;i>=0;--i){
        wxRmdir(ar[i]);
    }
    return TRUE==wxRmdir(*this);
}

// TODO: take account of dwExclude
int ecFileName::FindFiles (const wxString& pszDir,wxArrayString &ar,const wxString& pszPattern/*=wxT("*.*")*/,bool bRecurse/*=TRUE*/,long dwExclude/*=wxDIR_DIRS|wxDIR_HIDDEN*/)
{
    ar.Clear();

    // Scoping for wxDir
    {
        wxDir dir(pszDir);
        
        wxString fileName;
        bool bMore = dir.GetFirst(& fileName, pszPattern, wxDIR_FILES);
        while (bMore)
        {
            if (fileName != wxT(".") && fileName != wxT(".."))
            {
                // Add full path
                ecFileName path(pszDir);
                path += (const wxChar*) fileName;
                ar.Add(path);
            }
            bMore = dir.GetNext(& fileName);
        }
    }

    if (bRecurse)
    {
        // Since wxDir isn't rentrant, we need to gather all the directories
        // first
        wxArrayString ar2;

        // Scoping
        {
            wxDir dir(pszDir);
            
            wxString fileName;
            
            bool bMore = dir.GetFirst(& fileName, wxT("*"), wxDIR_DIRS);
            while (bMore)
            {
                if (fileName != wxT(".") && fileName != wxT(".."))
                {
                    // Add full path
                    ecFileName path(pszDir);
                    path += (const wxChar*) fileName;
                    ar2.Add(path);
                }
                bMore = dir.GetNext(& fileName);
            }
        }

        unsigned int i;
        for (i = 0; i < ar2.Count(); i++)
        {
            wxString f(ar2[i]);
            FindFiles(f, ar, pszPattern, bRecurse, dwExclude);
        }
    }

    return ar.Count();
}

void ecFileName::ReplaceExtension(const wxString& newExt)
{
    wxString ext = newExt;
    if (ext[(unsigned) 0] == wxT('.'))
        ext = ext.Mid(1);

    wxStripExtension(* this);
    this->wxString::Append(wxT("."));
    this->wxString::Append(ext);
}
