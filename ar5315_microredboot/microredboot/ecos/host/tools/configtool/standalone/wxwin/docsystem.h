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
// Author(s): 	julians
// Contact(s):	julians
// Date:		2001/04/04
// Version:		0.01
// Purpose:	
// Description:	Classes for the Configtool documentation system
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//  
//===========================================================================

#ifndef _EC_DOCSYSTEM_H_
#define _EC_DOCSYSTEM_H_

#ifdef __GNUG__
#pragma interface "docsystem.cpp"
#endif

#include "htmlparser.h"

enum ecIndexType { ecIndexByClass, ecIndexByList, ecIndexNoParse, ecIndexStartSection, ecIndexEndSection } ;

/*
 * ecHtmlIndexer
 * A class to parse files and generate suitable MS Html Help/wxHTML Help compatible
 * project, contents and keyword files.
 */

class ecHtmlIndexer: public wxObject
{
public:
    ecHtmlIndexer(bool useRelativeURLs = TRUE);
    ~ecHtmlIndexer();

//// Operations
    // Top-level function: generate appropriate index files
    // and place them either in the install directory or if that is read-only,
    // in the user's .eCos directory
    // Returns TRUE and the created project file if successful
    bool IndexDocs(const wxString& reposDir, wxString& projectFile, bool force = TRUE);

    // Creates the project, contents and keywords files using the ecIndexItems
    // we previously added.
    bool DoIndexDocs(const wxString& reposDir, wxString& projectFile, bool force = TRUE);

    // If tag is an <A HREF...>, write the item by looking at the text between <A> and </A>
    void CreateHHCOutputItem(wxSimpleHtmlTag* tag, int level, int& indent, const wxString& pathPrefix, const wxString& docDir, wxOutputStream& stream);

    // Append HHC-compatible code to a stream according to the tags found in the
    // given HTML file: use P CLASS attributes to determine level.
    bool CreateHHCByExaminingClass(const wxString& title, const wxString& topURL, const wxString& htmlFile, const wxString& docDir, wxOutputStream& stream, int startIndent = 0);
    
    // Append HHC-compatible code to a stream according to the tags found in the
    // given HTML file: use <DL> </DL> level
    bool CreateHHCByExaminingList(const wxString& title, const wxString& topURL, const wxString& htmlFile, const wxString& docDir, wxOutputStream& stream, int startIndent = 0);

    // Just add the given contents item without parsing
    bool CreateHHCItem(const wxString& title, const wxString& topURL, const wxString& docDir, wxOutputStream& stream, int startIndent = 0);

    void CreateHHCWriteHeader(wxOutputStream& stream);
    void CreateHHCWriteFooter(wxOutputStream& stream);

    // Start a section, which may or may not contain multiple subsections, one per architecture
    bool CreateHHCStartSection(const wxString& title, const wxString& topURL, const wxString& docDir, wxOutputStream& stream);
    bool CreateHHCEndSection(wxOutputStream& stream);

    // Create a section for all the Packages in the system, using the current document/repository
    bool CreateHHCPackagesSection(const wxString& title, const wxString& topURL, wxOutputStream& stream, const wxString& htmlPath);

    // Convert redirection to actual filename
    static wxString Redirect(const wxString& baseName, const wxString& url);

    // Write the project file
    bool WriteHHP(const wxString& filename, const wxString& docDir);

    // Find appropriate destination directory for writing files to
    wxString FindIndexFilesDir(const wxString& reposDir) ;

//// Operations on items
    void AddIndexByClass(const wxString& title, const wxString& urlToShow, const wxString& urlToExamine, int startIndent = 0);
    void AddIndexByList(const wxString& title, const wxString& urlToShow, const wxString& urlToExamine, int startIndent = 0);
    void AddIndexItem(const wxString& title, const wxString& urlToShow, int startIndent = 0);
    void AddStartSection(const wxString& title, const wxString& urlToShow = wxEmptyString);
    void AddEndSection();
    void ClearItems();

//// Symbol tables

    // Some things should be translated in the contents
    void AddEntityTranslation(const wxString& entity, const wxString& translation);
    // Apply all translations to this string
    wxString TranslateEntities(const wxString& toTranslate);

    // Mapping from directory to user-viewable name
    void AddTutorialDirectory(const wxString& dirName, const wxString& title);
    wxString TranslateTutorialDirectory(const wxString& dirName);

//// Accessors
    wxList& GetIndexItems() { return m_indexItems; }
    bool UseRelativeURLs() const { return m_useRelativeURLs; }
    bool UseOldDocs() const { return m_useOldDocs; }

//// Helpers

    // Set m_useOldDocs to TRUE if we find old-style docs
    bool CheckDocEra(const wxString& reposDir) ;

private:
    // List of ecIndexItems
    wxList  m_indexItems;
    bool    m_useRelativeURLs;
    bool    m_useOldDocs; // if TRUE, we have the old-style docs generated by FrameMaker
    wxArrayString m_entityTableNames; // Translations for awkward symbols not processed by wxHTML or MS HTML Help such as &#8212; (--)
    wxArrayString m_entityTableValues; // values for the above
    wxArrayString m_tutorialTableNames; // Directory -> title mapping e.g. arm -> ARM, sparclite -> Fujitsu SPARClite
    wxArrayString m_tutorialTableValues;
};

class ecIndexItem: public wxObject
{
public:
    ecIndexItem(ecIndexType type, const wxString& title, const wxString& urlToShow,
        const wxString& urlToExamine, int startLevel = 0)
    {
        m_type = type; m_title = title; m_urlToShow = urlToShow;
        m_urlToExamine = urlToExamine; m_startLevel = startLevel;
    }

    ecIndexType     m_type;
    wxString        m_title;
    wxString        m_urlToShow;
    wxString        m_urlToExamine;
    int             m_startLevel;
};

#endif

