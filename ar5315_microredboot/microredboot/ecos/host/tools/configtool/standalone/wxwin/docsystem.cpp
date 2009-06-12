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
// docsystem.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians, jld
// Contact(s):  julians
// Date:        2001/04/04
// Version:     $Id: docsystem.cpp,v 1.19 2001/12/11 15:59:51 julians Exp $
// Purpose:
// Description: Various classes for the documentation system
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#ifdef __GNUG__
#pragma implementation "docsystem.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/dir.h"
#include "wx/file.h"
#include "wx/wfstream.h"
#include "wx/progdlg.h"
#include "docsystem.h"
#include "htmlparser.h"
#include "configtooldoc.h"
#include "mainwin.h"
#include "shortdescrwin.h"

/*
 * ecHtmlIndexer
 * A class to parse files and generate suitable MS Html Help/wxHTML Help compatible
 * project, contents and keyword files.
 */

ecHtmlIndexer::ecHtmlIndexer(bool useRelativeURLs)
{
#if ecDOCSYSTEM_USE_RELATIVE_URLS
    m_useRelativeURLs = useRelativeURLs;
#else
    m_useRelativeURLs = FALSE;
#endif
    m_useOldDocs = TRUE;

    // Initialise some tables
    AddTutorialDirectory(wxT("arm"), wxT("ARM"));
    AddTutorialDirectory(wxT("am31-33"), wxT("AM31-33"));
    AddTutorialDirectory(wxT("i386pc"), wxT("i386 PC"));
    AddTutorialDirectory(wxT("ppc"), wxT("PowerPC"));
    AddTutorialDirectory(wxT("sh3"), wxT("SH-3"));
    AddTutorialDirectory(wxT("sparclite"), wxT("SPARClite"));
    AddTutorialDirectory(wxT("mips"), wxT("MIPS"));
    AddTutorialDirectory(wxT("v850"), wxT("V850"));

    AddEntityTranslation(wxT("&#8212;"), wxT("--"));
    AddEntityTranslation(wxT("&#38;"), wxT("&"));
    AddEntityTranslation(wxT("&#8220;"), wxT("\""));
    AddEntityTranslation(wxT("&#8221;"), wxT("\""));
    AddEntityTranslation(wxT("&#0091;"), wxT("["));
    AddEntityTranslation(wxT("&#0093;"), wxT("]"));
}

ecHtmlIndexer::~ecHtmlIndexer()
{
    ClearItems();
}

// Append HHC-compatible code to a stream according to the tags found in the
// given HTML file.
/*

Find the name and URL from a chunk of HTML like
the following.

<P CLASS="ChapterTitleTOC">
  <A NAME="pgfId=141252">
  </A>
  <A HREF="ecos-tutorial.4.html#pgfId=1065474" CLASS="Index">
     Documentation Roadmap
  </A>
     <EM CLASS="PageNumber"></EM>
</P>

We need to output something like this:

    <UL>
    <LI> <OBJECT type="text/sitemap"><param name="Name" value="Getting Started with eCos"><param name="Local" value="tutorials/arm/ecos-tutorial.1.html"></OBJECT>
    <LI> <UL>
           <LI> <OBJECT type="text/sitemap"><param name="Name" value="ARM"><param name="Local" value="tutorials/arm/ecos-tutorial.1.html"></OBJECT>
           <LI> <UL>
                  <LI><OBJECT type="text/sitemap"><param name="Name" value="Getting Started with eCos"><param name="Local" value="tutorials/arm/ecos-tutorial.1.html#pgfId=2052424"></OBJECT>
                  <LI><OBJECT type="text/sitemap"><param name="Name" value="Foreword"><param name="Local" value="tutorials/arm/ecos-tutorial.3.html#pgfId=1065235"></OBJECT>
                  <LI><OBJECT type="text/sitemap"><param name="Name" value="Documentation Roadmap"><param name="Local" value="tutorials/arm/ecos-tutorial.4.html#pgfId=1065474"></OBJECT>
                  <LI><UL>
                        <LI><OBJECT type="text/sitemap"><param name="Name" value="Getting Started with eCos"><param name="Local" value="tutorials/arm/ecos-tutorial.4.html#pgfId=1046487"></OBJECT>
                        <LI><OBJECT type="text/sitemap"><param name="Name" value="eCos User's Guide"><param name="Local" value="tutorials/arm/ecos-tutorial.4.html#pgfId=1046592"></OBJECT>
                        <LI><OBJECT type="text/sitemap"><param name="Name" value="eCos Reference Manual"><param name="Local" value="tutorials/arm/ecos-tutorial.4.html#pgfId=1046692"></OBJECT>
                      </UL>
                </UL>
         </UL>
  </UL>

*/

bool ecHtmlIndexer::CreateHHCByExaminingClass(const wxString& title, const wxString& topURL, const wxString& htmlFile, const wxString& docDir, wxOutputStream& stream, int startIndent)
{
    if (!wxFileExists(htmlFile))
        return FALSE;

    wxString sep(wxFILE_SEP_PATH);
    wxString pathPrefix(wxPathOnly(htmlFile));
    if (!wxIsAbsolutePath(htmlFile))
    {
        pathPrefix = pathPrefix.Mid(docDir.Length() + 1);
    }

    wxString topURL1(topURL);
    if (!UseRelativeURLs() && !wxIsAbsolutePath(topURL1))
        topURL1 = docDir + sep + topURL1;

    wxSimpleHtmlParser parser;
    if (parser.ParseFile(htmlFile))
    {
        stream << "<LI> <OBJECT type=\"text/sitemap\"><param name=\"Name\" value=\"";
        stream << TranslateEntities(title) << wxString("\">");
        
        if (!topURL.IsEmpty())
        {
            stream << "<param name=\"Local\" value=\"";
            stream << topURL1 << "\"></OBJECT>";
        }
        stream << "\n";
        int indent = startIndent;
        wxSimpleHtmlTag* tag = parser.GetTopLevelTag()->GetChildren();

        while (tag)
        {
            if (tag->GetType() == wxSimpleHtmlTag_Open)
            {
                wxSimpleHtmlAttribute* attr = NULL;
                if ((tag->NameIs("P") || tag->NameIs("DIV") ||
                     tag->NameIs("H1") || tag->NameIs("H2") || tag->NameIs("H3") || tag->NameIs("H4"))
                     &&
                     (attr = tag->FindAttribute("CLASS")))
                {
                    int level = -1;
                    if (attr->HasValue("Level1IX") ||
                        attr->HasValue("SectionTitleTOC") ||
                        attr->HasValue("IndexTitleTOC"))
                        level = 1;

                    else if (attr->HasValue("Level2IX") ||
                        attr->HasValue("ChapterTitleTOC") ||
                        attr->HasValue("IntroTitleTOC"))
                        level = 2;

                    else if (attr->HasValue("Heading1TOC"))
                        level = 3;

                    else if (attr->HasValue("Heading2TOC"))
                        level = 4;

                    if (level > -1)
                    {
                        wxSimpleHtmlTag* aTag = tag->FindTag("A", "HREF");
                        if (aTag)
                            CreateHHCOutputItem(aTag, level, indent, pathPrefix, docDir, stream);
                    }
                }
            }
            tag = tag->GetNext();
        }

        // Close any remaining levels
        int i;
        while (indent > startIndent)
        {
            indent --;
            for (i = 0; i < indent*2; i++) stream << " ";
            stream << "</UL>\n";
        }

        return TRUE;
    }
    else
        return FALSE;    
}

// Append HHC-compatible code to a stream according to the tags found in the
// given HTML file. Use level of <DL> to determine contents heading level.

/*

Find the name and URL from a chunk of HTML like
the following.

<DL>
  <DT>
  <B>Table of Contents</B>
  </DT>
  <DT>
  1. <A HREF="overview.html">Overview</A>
  </DT>
  <DD>
    <DL>
      <DT>
      <A HREF="overview.html#OVERVIEW.TERMINOLOGY">Terminology</A>
      </DT>
      <DD>
        <DL>
          <DT>
          <A HREF="overview.html#CONCEPTS.TERMINOLOGY.FRAMEWORK">Component Framework</A>
          </DT>

  ...

    <UL>
    <LI> <OBJECT type="text/sitemap"><param name="Name" value="Getting Started with eCos"><param name="Local" value="tutorials/arm/ecos-tutorial.1.html"></OBJECT>
    <LI> <UL>
           <LI> <OBJECT type="text/sitemap"><param name="Name" value="ARM"><param name="Local" value="tutorials/arm/ecos-tutorial.1.html"></OBJECT>
           <LI> <UL>
                  <LI><OBJECT type="text/sitemap"><param name="Name" value="Getting Started with eCos"><param name="Local" value="tutorials/arm/ecos-tutorial.1.html#pgfId=2052424"></OBJECT>
                  <LI><OBJECT type="text/sitemap"><param name="Name" value="Foreword"><param name="Local" value="tutorials/arm/ecos-tutorial.3.html#pgfId=1065235"></OBJECT>
                  <LI><OBJECT type="text/sitemap"><param name="Name" value="Documentation Roadmap"><param name="Local" value="tutorials/arm/ecos-tutorial.4.html#pgfId=1065474"></OBJECT>
                  <LI><UL>
                        <LI><OBJECT type="text/sitemap"><param name="Name" value="Getting Started with eCos"><param name="Local" value="tutorials/arm/ecos-tutorial.4.html#pgfId=1046487"></OBJECT>
                        <LI><OBJECT type="text/sitemap"><param name="Name" value="eCos User's Guide"><param name="Local" value="tutorials/arm/ecos-tutorial.4.html#pgfId=1046592"></OBJECT>
                        <LI><OBJECT type="text/sitemap"><param name="Name" value="eCos Reference Manual"><param name="Local" value="tutorials/arm/ecos-tutorial.4.html#pgfId=1046692"></OBJECT>
                      </UL>
                </UL>
         </UL>
  </UL>

*/

bool ecHtmlIndexer::CreateHHCByExaminingList(const wxString& title, const wxString& topURL, const wxString& htmlFile, const wxString& docDir, wxOutputStream& stream, int startIndent)
{
    if (!wxFileExists(htmlFile))
        return FALSE;

    // The path prefix is the path relative to the doc dir
    wxString sep(wxFILE_SEP_PATH);
    wxString pathPrefix(wxPathOnly(htmlFile));
    if (!wxIsAbsolutePath(htmlFile))
    {
        pathPrefix = pathPrefix.Mid(docDir.Length() + 1);
    }

    wxString topURL1(topURL);
    if (!UseRelativeURLs() && !wxIsAbsolutePath(topURL1))
        topURL1 = docDir + sep + topURL1;

    wxSimpleHtmlParser parser;
    if (parser.ParseFile(htmlFile))
    {
        stream << "<LI> <OBJECT type=\"text/sitemap\"><param name=\"Name\" value=\"";
        stream << TranslateEntities(title) << wxString("\">");
        
        if (!topURL.IsEmpty())
        {
            stream << "<param name=\"Local\" value=\"";
            stream << topURL1 << "\"></OBJECT>";
        }
        stream << "\n";
        int indent = startIndent; int level = 0;
        wxSimpleHtmlTag* tag = parser.GetTopLevelTag()->GetChildren();

        while (tag)
        {
            if (tag->GetType() == wxSimpleHtmlTag_Open && tag->NameIs("DL")) level ++ ;
            if (tag->GetType() == wxSimpleHtmlTag_Close && tag->NameIs("DL")) level -- ;
            if (tag->GetType() == wxSimpleHtmlTag_Open && tag->NameIs("A") && tag->HasAttribute("HREF") && level > 0)
                CreateHHCOutputItem(tag, level, indent, pathPrefix, docDir, stream);

            // If we get to list of figures/tables/examples, finish
            if (tag->GetType() == wxSimpleHtmlTag_Text)
            {
                if (tag->GetText() == wxT("List of Figures") ||
                    tag->GetText() == wxT("List of Tables") ||
                    tag->GetText() == wxT("List of Examples"))
                {
                    tag = NULL;
                }
            }

            if (tag)
                tag = tag->GetNext();
        }

        // Close any remaining levels
        int i;
        while (indent > startIndent)
        {
            indent --;
            for (i = 0; i < indent*2; i++) stream << " ";
            stream << "</UL>\n";
        }
        return TRUE;
    }
    else
        return FALSE;   
}

// Just add the given contents item without parsing
bool ecHtmlIndexer::CreateHHCItem(const wxString& title, const wxString& topURL, const wxString& docDir, wxOutputStream& stream, int indent)
{
    wxString sep(wxFILE_SEP_PATH);
    wxString topURL1(topURL);
    if (!UseRelativeURLs())
        topURL1 = docDir + sep + topURL1;

    int i;
    for (i = 0; i < indent*2; i++) stream << " ";
    
    stream << "<LI> <OBJECT type=\"text/sitemap\"><param name=\"Name\" value=\"";
    stream << TranslateEntities(title) << "\"><param name=\"Local\" value=\"";
    stream << topURL1 << "\"></OBJECT>\n";

    return TRUE;
}

void ecHtmlIndexer::CreateHHCOutputItem(wxSimpleHtmlTag* tag, int level, int& indent, const wxString& pathPrefix, const wxString& docDir, wxOutputStream& stream)
{
    wxString url, text;
    tag->GetAttributeValue(url, "HREF");
    tag->FindTextUntilTagClose(text, "A");
    text.Trim(TRUE); // Trim spaces from right of string
    text.Replace("\n", " "); // Remove newlines

    // Need to adjust the URL to give the path relative to where the index is
    url = pathPrefix + wxString(wxT("/")) + url;

    wxString sep(wxFILE_SEP_PATH);
    wxString url1(url);
    if (!UseRelativeURLs() && !wxIsAbsolutePath(url))
        url1 = docDir + sep + url;

    int i;
    while (level > indent)
    {
        for (i = 0; i < indent*2; i++) stream << " ";

        stream << "<UL>";
        indent ++;

        // If we're skipping one or more levels, we need to insert
        // a dummy node.
        if (level > indent)
        {
            stream << "\n";
            for (i = 0; i < indent*2; i++) stream << " ";

            stream << "<LI> <OBJECT type=\"text/sitemap\"><param name=\"Name\" value=\"";
            stream << TranslateEntities(text) << "\"><param name=\"Local\" value=\"";
            stream << url1 << "\"></OBJECT>";
        }
        stream << "\n";
    }

    while (level < indent)
    {
        indent--;
        for (i = 0; i < indent*2; i++) stream << " ";

        stream << "</UL>\n";
    }

    for (i = 0; i < indent*2; i++) stream << " ";

    stream << "<LI> <OBJECT type=\"text/sitemap\"><param name=\"Name\" value=\"";
    stream << TranslateEntities(text) << "\"><param name=\"Local\" value=\"";
    stream << url1 << "\"></OBJECT>\n";
}

void ecHtmlIndexer::CreateHHCWriteHeader(wxOutputStream& stream)
{
    stream << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n";
    stream << "<HTML>\n<HEAD>\n<meta name=\"GENERATOR\" content=\"Microsoft&reg; HTML Help Workshop 4.1\">\n";
    stream << "<!-- Sitemap 1.0 -->\n</HEAD><BODY>\n";
    stream << "<UL>\n";
}

void ecHtmlIndexer::CreateHHCWriteFooter(wxOutputStream& stream)
{
    stream << "</UL>\n";
    stream << "</BODY></HTML>";
}

bool ecHtmlIndexer::CreateHHCStartSection(const wxString& title, const wxString& topURL, const wxString& docDir, wxOutputStream& stream)
{
    wxString sep(wxFILE_SEP_PATH);
    wxString url1(topURL);
    if (!UseRelativeURLs() && !wxIsAbsolutePath(url1))
        url1 = docDir + sep + url1;

    stream << "<LI> <OBJECT type=\"text/sitemap\"><param name=\"Name\" value=\"";
    stream << TranslateEntities(title) << wxString("\">");
    
    if (!topURL.IsEmpty())
    {
        stream << "<param name=\"Local\" value=\"";
        stream << url1 << "\">" ;
    }
    stream << "</OBJECT>\n<UL>\n" ;
    return TRUE;
}

bool ecHtmlIndexer::CreateHHCEndSection(wxOutputStream& stream)
{
    stream << "</UL>\n";
    return TRUE;
}

// Write the project file
bool ecHtmlIndexer::WriteHHP(const wxString& filename, const wxString& docDir)
{
    wxFileOutputStream stream(filename);

    wxString sep(wxFILE_SEP_PATH);
    wxString path, name, ext;
    wxSplitPath(filename, & path, & name, & ext);

    wxString compiledFile(name + wxT(".chm"));
    wxString contentsFile(name + wxT(".hhc"));
    wxString keywordFile(name + wxT(".hhk"));

    stream << "[OPTIONS]\n\
Auto Index=Yes\n\
Binary Index=No\n\
Compatibility=1.1 or later\n\
Compiled file=";
    stream << compiledFile << "\nContents file=" << contentsFile << "\n";
    stream << 
"Default Window=mainwin\n\
Default topic=";
    if (!UseRelativeURLs())
        stream << docDir + sep;
    stream << wxT("index.html") << "\n\
Display compile progress=Yes\n\
Full-text search=Yes\n" <<

// Index file=" << keywordFile << "\n

"Language=0x409 English (United States)\n\
Title=eCos\n";

    stream << 
"[WINDOWS]\n\
mainwin=\"eCos Documentation\",\"eCos.hhc\",,,\"index.html\",\"http://sources.redhat.com/ecos/\",\"Net Release\",\"http://www.redhat.com/products/ecos/\",\"eCos Product\",0x40060420,,0xc287e,[0,0,762,400],,,,,,,0\n\
\n\
[FILES]\n\
index.html\n\
\n\
[INFOTYPES]\n" ;

    // When we have the ability to generate a hhk, replace above line with:
    // mainwin=\"eCos Documentation\",\"eCos.hhc\",\"eCos.hhk\",,\"index.html\",\"http://sources.redhat.com/ecos/\",\"Net Release\",\"http://www.redhat.com/products/ecos/\",\"eCos Product\",0x40060420,,0xc287e,[0,0,762,400],,,,,,,0\n\

    return TRUE;

}

// Create a section for all the Packages in the system, using the current document/repository.
// TODO: check each URL for redirection.
bool ecHtmlIndexer::CreateHHCPackagesSection(const wxString& title, const wxString& topURL, wxOutputStream& stream, const wxString& htmlPath)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (!doc)
        return FALSE;

    // If we have multiple tutorials or whatever, then we need to repeat this line ONCE
    // and then generate the multiple files. Otherwise we'll be repeating the same "Getting Started with eCos"
    // line. I.e. it'll only look right if we only have one tutorial.
    stream << "<LI> <OBJECT type=\"text/sitemap\"><param name=\"Name\" value=\"";
    stream << TranslateEntities(title) << wxString("\">");
    
    if (!topURL.IsEmpty())
    {
        stream << "<param name=\"Local\" value=\"";
        stream << topURL << "\">" ;
    }
    stream << "</OBJECT>\n<UL>\n" ;

    // generate the contents of the add/remove list boxes
    const std::vector<std::string> & packages = doc->GetCdlPkgData ()->get_packages ();

    std::vector<std::string>::const_iterator package_i;
    for (package_i = packages.begin (); package_i != packages.end (); package_i++)
    {
        //		if (! m_CdlPkgData->is_hardware_package (* package_i)) // do not list hardware packages
        {
            const std::vector<std::string> & aliases = doc->GetCdlPkgData ()->get_package_aliases (* package_i);
            wxString strMacroName = package_i->c_str ();

            // use the first alias (if any) as the package identifier
            wxString strPackageName = aliases.size () ? aliases [0].c_str () : strMacroName.c_str();
            ecConfigItem * pItem = doc->Find (strMacroName);
            if (pItem) // if the package is loaded
            {
                // TODO: what if the package is not loaded? how do we access the URL??

                wxString url(pItem->GetURL());

                url = htmlPath + wxString(wxFILE_SEP_PATH) + Redirect(htmlPath, url);

                stream << "<LI> <OBJECT type=\"text/sitemap\"><param name=\"Name\" value=\"";
                stream << TranslateEntities(strPackageName) << wxString("\">");
                
                if (!url.IsEmpty())
                {
                    stream << "<param name=\"Local\" value=\"";
                    stream << url << "\">" ;
                }
                stream << "</OBJECT>\n" ;
            }
        }
    }
    
    stream << "</UL>\n";
    return TRUE;
}

// Keeping looking for redirection until there's none.
wxString ecHtmlIndexer::Redirect(const wxString& baseName, const wxString& url)
{
    wxString path(baseName);
    path += wxFILE_SEP_PATH;
    path += url;

    wxString relativePath(wxPathOnly(url));
    
    wxSimpleHtmlParser parser;
    if ((path.Find(wxT('#')) == -1) && wxFileExists(path) && parser.ParseFile(path))
    {
        wxSimpleHtmlTag* tag = parser.GetTopLevelTag()->GetChildren();
        
        wxSimpleHtmlTag* refreshTag = tag->FindTag(wxT("meta"), wxT("http-equiv"));
        if (refreshTag)
        {
            wxString value;
            if (refreshTag->GetAttributeValue(value, wxT("content")))
            {
                if (!value.AfterFirst(wxT('=')).IsEmpty())
                    value = value.AfterFirst(wxT('=')) ;

                wxString newURL(relativePath + wxString(wxFILE_SEP_PATH) + value);
                return Redirect(baseName, newURL);
            }
        }
    }
    return url;
}

bool ecHtmlIndexer::DoIndexDocs(const wxString& reposDir, wxString& projectFile, bool force)
{
    wxString sep(wxFILE_SEP_PATH);
    
    wxString docDir(reposDir + sep + wxString(wxT("doc"))) ;
    
    // The CVS repository has an HTML subdirectory, but the
    // packaged version doesn't
    if (wxDirExists(docDir + sep + wxT("html")))
        docDir = docDir + sep + wxString(wxT("html"));

    wxString projectDir = FindIndexFilesDir(reposDir);

    projectFile = projectDir + sep + wxT("eCos.hhp");
    wxString contentsFile = projectDir + sep + wxT("eCos.hhc");
    wxString keywordFile = projectDir + sep + wxT("eCos.hhk");

    // See if it's already been generated
    if (wxFileExists(projectFile) && !force)
        return TRUE;
    
    // Project file
    if (!WriteHHP(projectFile, docDir))
        return FALSE;
    
    wxFileOutputStream stream(contentsFile);

    if (!stream.Ok())
        return FALSE;

    // Pop up a progress dialog
    wxProgressDialog dialog(wxGetApp().GetSettings().GetAppName(),
        _("Compiling documentation index..."), m_indexItems.Number(),
        wxGetApp().GetTopWindow());
    
    CreateHHCWriteHeader(stream);

    int count = 1;
    wxNode* node = m_indexItems.First();
    while (node)
    {
        dialog.Update(count);
        count ++;

        ecIndexItem* item = (ecIndexItem*) node->Data();
        wxString filename(item->m_urlToExamine);
        wxString urlFilename(item->m_urlToShow);

        if (!filename.IsEmpty())
        {
            if (!wxIsAbsolutePath(filename))
                filename = docDir + sep + filename;
#ifdef __WXMSW__
            filename.Replace(wxT("/"), wxT("\\"));
#endif
        }

        // Check that the URL we're going to show is available,
        // otherwise don't output it
        bool isOk = TRUE;
        if (!urlFilename.IsEmpty())
        {
            if (!wxIsAbsolutePath(urlFilename))
                urlFilename = docDir + sep + urlFilename;
#ifdef __WXMSW__
            urlFilename.Replace(wxT("/"), wxT("\\"));
#endif
            // Remove # part if there is any
            if (urlFilename.Find(wxT('#')) != -1)
                urlFilename = urlFilename.BeforeLast(wxT('#'));
            if (!wxFileExists(urlFilename))
                isOk = FALSE;
        }

        if (isOk)
        {
            switch (item->m_type)
            {
            case ecIndexByClass:
                {
                    CreateHHCByExaminingClass(item->m_title, item->m_urlToShow, filename, docDir, stream, item->m_startLevel);
                    break;
                }
            case ecIndexByList:
                {
                    CreateHHCByExaminingList(item->m_title, item->m_urlToShow, filename, docDir, stream, item->m_startLevel);
                    break;
                }
            case ecIndexNoParse:
                {
                    CreateHHCItem(item->m_title, item->m_urlToShow, docDir, stream, item->m_startLevel);
                    break;
                }
            case ecIndexStartSection:
                {
                    CreateHHCStartSection(item->m_title, item->m_urlToShow, docDir, stream);
                    break;
                }
            case ecIndexEndSection:
                {
                    CreateHHCEndSection(stream);
                    break;
                }
            default:
                {
                    wxASSERT( FALSE );
                    break;
                }
            }
        }

        node = node->Next();
    }
    
//    CreateHHCPackagesSection(wxT("Packages"), wxEmptyString, stream, docDir);
    
    CreateHHCWriteFooter(stream);

#if 0 // def __WXGTK__
    // Hack to restore correct colour to short description window
    ecShortDescriptionWindow* sdw = wxGetApp().GetMainFrame()->GetShortDescriptionWindow();

    wxColour oldColour = sdw->GetBackgroundColour();
    if (oldColour.Ok())
    {
        sdw->SetBackgroundColour(*wxBLACK);
        sdw->SetBackgroundColour(oldColour);
    }
#endif

    return TRUE;
}

static bool ecDirectoryWriteable(const wxString& dir)
{
    // See if we can write to it
    wxString sep(wxFILE_SEP_PATH);
    wxString testFile = dir + sep + wxT("_test.tmp");

    bool ok = FALSE;

    {
        wxLogNull log;
        wxFile file;
        ok = file.Create(testFile) ;
        if (ok)
        {
            file.Close();
            wxRemoveFile(testFile);
        }
    }

    return ok;
}

// Find appropriate destination directory for writing files to
wxString ecHtmlIndexer::FindIndexFilesDir(const wxString& reposDir)
{
    wxString sep(wxFILE_SEP_PATH);

    // First try install dir
    wxString dir = reposDir;

#ifdef __WXMSW__
    if (!ecDirectoryWriteable(dir))
    {
        // Try temp directory as a last resort
        if (!wxGetEnv(wxT("TEMP"), & dir))
        {
            dir = wxT("c:\\temp");
        }
        return dir;
    }
    else
    {
        return dir;
    }

#else
    if (!ecDirectoryWriteable(dir))
    {
        dir = wxGetHomeDir();
        if (!ecDirectoryWriteable(dir))
        {
            // Try temp directory as a last resort
            if (!wxGetEnv(wxT("TEMP"), & dir))
            {
                dir = wxT("/tmp");
            }
        }
        else
        {
            wxString ecosDir = dir + sep + wxT(".eCosDocs");
            if (!wxDirExists(ecosDir))
            {
                wxMkdir(ecosDir);
            }
            wxString name(ecMakeNameFromPath(reposDir));
            wxString ecosVerDir = ecosDir + sep + name;
            if (!wxDirExists(ecosVerDir))
            {
                wxMkdir(ecosVerDir);
            }
            dir = ecosVerDir;
        }
        return dir;
    }
    else
    {
        return dir;
    }

#endif
}

//// Operations on items
void ecHtmlIndexer::AddIndexByClass(const wxString& title, const wxString& urlToShow, const wxString& urlToExamine, int startIndent)
{
    m_indexItems.Append(new ecIndexItem(ecIndexByClass, title, urlToShow, urlToExamine, startIndent));
}

void ecHtmlIndexer::AddIndexByList(const wxString& title, const wxString& urlToShow, const wxString& urlToExamine, int startIndent)
{
    m_indexItems.Append(new ecIndexItem(ecIndexByList, title, urlToShow, urlToExamine, startIndent));
}

void ecHtmlIndexer::AddIndexItem(const wxString& title, const wxString& urlToShow, int startIndent)
{
    m_indexItems.Append(new ecIndexItem(ecIndexNoParse, title, urlToShow, wxEmptyString, startIndent));
}

void ecHtmlIndexer::AddStartSection(const wxString& title, const wxString& urlToShow)
{
    m_indexItems.Append(new ecIndexItem(ecIndexStartSection, title, urlToShow, wxEmptyString));
}

void ecHtmlIndexer::AddEndSection()
{
    m_indexItems.Append(new ecIndexItem(ecIndexEndSection, wxEmptyString, wxEmptyString, wxEmptyString));
}

void ecHtmlIndexer::ClearItems()
{
    wxNode* node = m_indexItems.First();
    while (node)
    {
        ecIndexItem* item = (ecIndexItem*) node->Data();
        delete item;
        node = node->Next();
    }
    m_indexItems.Clear();
}

// Set m_useOldDocs to TRUE if we find old-style docs
bool ecHtmlIndexer::CheckDocEra(const wxString& reposDir)
{
    // We look for tutorials/arm/ecos-tutorial.1.html to see if it's old-style
    wxString sep(wxFILE_SEP_PATH);
    
    wxString docDir(reposDir + sep + wxString(wxT("doc"))) ;
    
    // The CVS repository has an HTML subdirectory, but the
    // packaged version doesn't
    if (wxDirExists(docDir + sep + wxT("html")))
        docDir = docDir + sep + wxString(wxT("html"));
    
    wxString armTutorial = docDir + sep + wxString(wxT("tutorials")) + sep +
        wxString(wxT("arm")) + sep + wxString(wxT("ecos-tutorial.1.html")) ;

    m_useOldDocs = wxFileExists(armTutorial);
    return m_useOldDocs;
}


// Top-level function: generate appropriate index files
// and place them either in the install directory or if that is read-only,
// in the user's .eCos directory.
// Returns TRUE and the created project file if successful
bool ecHtmlIndexer::IndexDocs(const wxString& reposDir, wxString& projectFile, bool force)
{
    CheckDocEra(reposDir);

    if (UseOldDocs())
    {
        // Old-style docs, where HTML is mostly generated from PageMaker

        AddStartSection(wxT("Getting Started with eCos"), wxT(""));
        AddIndexByClass(wxT("AM31-33"), wxT("tutorials/am31-33/ecos-tutorial.1.html"), wxT("tutorials/am31-33/ecos-tutorial.2.html"));
        AddIndexByClass(wxT("ARM"), wxT("tutorials/arm/ecos-tutorial.1.html"), wxT("/tutorials/arm/ecos-tutorial.2.html"));
        AddIndexByClass(wxT("i386 PC"), wxT("tutorials/i386pc/ecos-tutorial.1.html"), wxT("tutorials/i386pc/ecos-tutorial.2.html"));
        AddIndexByClass(wxT("PowerPC"), wxT("tutorials/ppc/ecos-tutorial.1.html"), wxT("tutorials/ppc/ecos-tutorial.2.html"));
        AddIndexByClass(wxT("SH-3"), wxT("tutorials/sh3/ecos-tutorial.1.html"), wxT("tutorials/sh3/ecos-tutorial.2.html"));
        AddIndexByClass(wxT("SPARClite"), wxT("tutorials/sparclite/ecos-tutorial.1.html"), wxT("tutorials/sparclite/ecos-tutorial.2.html"));
        AddIndexByClass(wxT("MIPS"), wxT("tutorials/mips/ecos-tutorial.1.html"), wxT("tutorials/mips/ecos-tutorial.2.html"));
        AddIndexByClass(wxT("V850"), wxT("tutorials/v850/ecos-tutorial.1.html"), wxT("tutorials/v850/ecos-tutorial.2.html"));
        AddEndSection();
        
        AddIndexByClass(wxT("eCos User's Guide"), wxT("guides/user-guides.1.html"), wxT("guides/user-guides.2.html"));
        
        AddIndexByList(wxT("RedBoot User's Guide"), wxT("redboot/redboot.html"), wxT("redboot/redboot.html"));
        
        AddIndexByClass(wxT("Linux Configuration Tool Guide"), wxGetApp().GetFullAppPath(wxT("manual/user-guides.4.html")), wxGetApp().GetFullAppPath(wxT("manual/user-guides.2.html")));
        
        AddIndexByList(wxT("eCos Component Writer's Guide"), wxT("cdl/cdl-guide.html"), wxT("cdl/cdl-guide.html"));
        
        AddIndexByClass(wxT("eCos Reference Manual"), wxT("ref/ecos-ref.1.html"), wxT("ref/ecos-ref.2.html"));
        
        AddIndexByClass(wxT("eCos-EL/IX Compatibility Guide"), wxT("ecos-elix/ecos-elix.html"), wxT("ecos-elix/ecos-elix.1.html"), 1);
        
        AddStartSection(wxT("GNUPro Toolkit Reference Manual"));
        // Start at indent 1 to avoid a spurious level
        AddIndexByClass(wxT("ARM"), wxT("ref/gnupro-ref/arm/ARM_COMBO_front.html"), wxT("ref/gnupro-ref/arm/ARM_COMBOTOC.html"), 1);
        AddIndexByClass(wxT("Fujitsu SPARClite"), wxT("ref/gnupro-ref/sparclite/index.html"), wxT("ref/gnupro-ref/sparclite/index.html"));
        AddIndexByClass(wxT("Matsushita MN10300"), wxT("ref/gnupro-ref/mn10300/am33_front.html"), wxT("ref/gnupro-ref/mn10300/am33toc.html"), 1);
        AddIndexByClass(wxT("PowerPC"), wxT("ref/gnupro-ref/powerpc/index.html"), wxT("ref/gnupro-ref/powerpc/index.html"));
        AddIndexByClass(wxT("Toshiba MIPS TX39"), wxT("/gnupro-ref/tx39/index.html"), wxT("/gnupro-ref/tx39/index.html"));
        
        // Don't parse HTML, just add this item, if the page exists.
        // Presumably the HTML can't be parsed for some reason.
        AddIndexItem(wxT("Toshiba MIPS TX49"), wxT("ref/gnupro-ref/tx49/tx49_ref.html"));
        
        AddIndexByClass(wxT("Hitachi SuperH"), wxT("ref/gnupro-ref/sh/SH_front.html"), wxT("ref/gnupro-ref/sh/SHTOC.html"), 1);
        
        AddIndexItem(wxT("NEC V850"), wxT("ref/gnupro-ref/v850/v850_ref_3.html"));
        AddIndexByClass(wxT("NEC VR4300"), wxT("ref/gnupro-ref/vr4300/Vr43REF_front.html"), wxT("ref/gnupro-ref/vr4300/Vr43REFTOC.html"), 1);
        AddEndSection();
    }
    else
    {
        // NEW-STYLE DOCUMENTATION (HTML is generated from SGML)

        // Get a list of all tutorials

        wxArrayString tutorials;

        wxString sep(wxFILE_SEP_PATH);

        wxString docDir(reposDir + sep + wxString(wxT("doc"))) ;

        // The CVS repository has an HTML subdirectory, but the
        // packaged version doesn't
        if (wxDirExists(docDir + sep + wxT("html")))
            docDir = docDir + sep + wxString(wxT("html"));

        docDir += sep ;
        docDir += wxString(wxT("tutorials"));

        wxLogNull log;
        wxDir dir(docDir);

        if (dir.IsOpened())
        {
            wxString filename;
            bool cont = dir.GetFirst(& filename, wxT("*"), wxDIR_DIRS);
            while (cont)
            {
                if (filename != wxT(".") && filename != wxT(".."))
                    tutorials.Add(filename);

                cont = dir.GetNext(& filename);
            }
        }

//        AddStartSection(wxT("Getting Started with eCos"), wxT(""));
        size_t i;
        for (i = 0; i < tutorials.GetCount(); i++)
        {
            wxString tutorial(tutorials[i]);
            wxString tutorialRelativePath = wxT("tutorials/") + tutorial + wxT("/ecos-tutorial.html");

            // Use a more friendly name than just the directory if it's available
            AddIndexByList(TranslateTutorialDirectory(tutorial), tutorialRelativePath, tutorialRelativePath);
        }
//        AddEndSection();

        AddIndexByList(wxT("User Guide"), wxT("user-guide/ecos-user-guide.html"), wxT("user-guide/ecos-user-guide.html"));
//        AddIndexByList(wxT("RedBoot User's Guide"), wxT("redboot/redboot.html"), wxT("redboot/redboot.html"));
#ifdef __WXGTK__
        // FIXME: wxHtmlParser (version 2.4.0) doesn't like the eCos Reference HTML on Linux so just index the initial page for now
        AddIndexItem(wxT("eCos Reference"), wxT("ref/ecos-ref.html"));
#else
        AddIndexByList(wxT("eCos Reference"), wxT("ref/ecos-ref.html"), wxT("ref/ecos-ref.html"));
#endif
        AddIndexByList(wxT("Component Writer's Guide"), wxT("cdl-guide/cdl-guide.html"), wxT("cdl-guide/cdl-guide.html"));
//        AddIndexByList(wxT("eCos-EL/IX Compatibility Guide"), wxT("ecos-elix/elix-compatibility.html"), wxT("ecos-elix/elix-compatibility.html"));

        //// TOOLCHAIN REFERENCE MANUALS
//        AddStartSection(wxT("GNUPro Toolkit Reference Manual"));
        // Start at indent 1 to avoid a spurious level
//        AddIndexByClass(wxT("ARM"), wxT("ref/gnupro-ref/arm/ARM_COMBO_front.html"), wxT("ref/gnupro-ref/arm/ARM_COMBOTOC.html"), 1);
//        AddIndexByClass(wxT("Fujitsu SPARClite"), wxT("ref/gnupro-ref/sparclite/index.html"), wxT("ref/gnupro-ref/sparclite/index.html"), 1);
//        AddIndexByClass(wxT("Matsushita MN10300"), wxT("ref/gnupro-ref/mn10300/am33_front.html"), wxT("ref/gnupro-ref/mn10300/am33toc.html"), 1);
//        AddIndexByClass(wxT("PowerPC"), wxT("ref/gnupro-ref/powerpc/index.html"), wxT("ref/gnupro-ref/powerpc/index.html"));
//        AddIndexByClass(wxT("Toshiba MIPS TX39"), wxT("/gnupro-ref/tx39/index.html"), wxT("/gnupro-ref/tx39/index.html"));

        // Don't parse HTML, just add this item, if the page exists.
        // Presumably the HTML can't be parsed for some reason.
//        AddIndexItem(wxT("Toshiba MIPS TX49"), wxT("ref/gnupro-ref/tx49/tx49_ref.html"));

//        AddIndexByClass(wxT("Hitachi SuperH"), wxT("ref/gnupro-ref/sh/SH_front.html"), wxT("ref/gnupro-ref/sh/SHTOC.html"), 1);

//        AddIndexItem(wxT("NEC V850"), wxT("ref/gnupro-ref/v850/v850_ref_3.html"));
//        AddIndexByClass(wxT("NEC VR4300"), wxT("ref/gnupro-ref/vr4300/Vr43REF_front.html"), wxT("ref/gnupro-ref/vr4300/Vr43REFTOC.html"), 1);
//        AddEndSection();
    }

    DoIndexDocs(reposDir, projectFile, force);

    return TRUE;
}

// Some things should be translated in the contents
void ecHtmlIndexer::AddEntityTranslation(const wxString& entity, const wxString& translation)
{
    m_entityTableNames.Add(entity);
    m_entityTableValues.Add(translation);
}

// Apply all translations to this string
wxString ecHtmlIndexer::TranslateEntities(const wxString& toTranslate)
{
    wxString result(toTranslate);
    size_t i;
    for (i = 0; i < m_entityTableNames.GetCount(); i++)
    {
        result.Replace(m_entityTableNames[i], m_entityTableValues[i]);
    }
    return result;
}

// Mapping from directory to user-viewable name
void ecHtmlIndexer::AddTutorialDirectory(const wxString& dirName, const wxString& title)
{
    m_tutorialTableNames.Add(dirName);
    m_tutorialTableValues.Add(title);
}

wxString ecHtmlIndexer::TranslateTutorialDirectory(const wxString& dirName)
{
    int i = m_tutorialTableNames.Index(dirName);
    if (i >= 0)
        return m_tutorialTableValues[i];
    else
        return dirName;
}

