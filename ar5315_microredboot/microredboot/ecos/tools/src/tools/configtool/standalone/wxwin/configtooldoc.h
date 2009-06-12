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
// configtooldoc.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/10/05
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/configtooldoc.h#3 $
// Purpose:
// Description: Header file for ecConfigToolDoc
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_CONFIGTOOLDOC_H_
#define _ECOS_CONFIGTOOLDOC_H_

#ifdef __GNUG__
#pragma interface "configtooldoc.h"
#endif

#include "cdl.hxx"

#include "configitem.h"

#if ecUSE_MLT
#include "memmap.h"
#endif

class ecConfigItem;

// Tell OnCreate to prompt for a repository instead of just loading it silently
#define ecDOC_PROMPT_FOR_REPOSITORY 8

/*
 * ecConfigToolDoc
 * Holds the saveable configtool information
 */

class ecConfigToolDoc: public wxDocument
{
    DECLARE_DYNAMIC_CLASS(ecConfigToolDoc)
public:
    ecConfigToolDoc();
    ~ecConfigToolDoc();

//// Overrides
    virtual bool OnCreate(const wxString& path, long flags);
    virtual bool OnOpenDocument(const wxString& filename);
    virtual bool OnSaveDocument(const wxString& filename);
    virtual bool OnNewDocument() { return TRUE; }
    virtual bool OnCloseDocument() ;
    virtual bool Save(); // Overridden only to correct bug in wxWindows, docview.cpp

//// Error and other handlers
    static void CdlLoadErrorHandler(std::string message);
    static void CdlLoadWarningHandler(std::string message);
    static void CdlParseErrorHandler (std::string message);
    static void CdlParseWarningHandler (std::string message);
    static void CdlTransactionHandler (const CdlTransactionCallback & data);
    static CdlInferenceCallbackResult CdlInferenceHandler (CdlTransaction data);
    static CdlInferenceCallbackResult CdlGlobalInferenceHandler(CdlTransaction data);

//// Accessors
    CdlPackagesDatabase GetCdlPkgData () { return m_CdlPkgData; }
    CdlConfiguration GetCdlConfig () { return m_CdlConfig; }
    CdlInterpreter GetCdlInterpreter() { return m_CdlInterp; }

    // Absolute path name to root of respository - parent of PackagesDir below
    void SetRepository(const wxString& pszRepository)  { m_strRepository = ecFileName(pszRepository); }
    const ecFileName& GetRepository() const { return m_strRepository; }

    wxString GetDefaultHardware () ;
	const wxString& GetTemplateVersion() const { return m_templateVersion; }
    wxString GetPackageName (const wxString & strAlias);

    // Absolute path name to "packages" directory - i.e. the one under the repository
    // named "packages" in a delivery or "ecc" in CVS:
    const ecFileName& GetPackagesDir() const { return m_strPackagesDir; }   

    const wxString& GetInstallTree() const { return m_strInstallTree; }
    const wxString& GetBuildTree() const { return m_strBuildTree; }
    void SetInstallTree(const wxString& str) { m_strInstallTree = str; }
    void SetBuildTree(const wxString& str) { m_strBuildTree = str; }

    wxList& GetItems() { return m_items; }
    ecConfigItem* GetItem(size_t i) { return (ecConfigItem*) m_items[i]; }

    CdlBuildInfo& GetBuildInfo() { return m_BuildInfo; }

    ecConfigItem* GetFirstItem() { return (m_items.Number() == 0 ? (ecConfigItem*) NULL : (ecConfigItem*) m_items.First()->Data()); }

    const wxString GetCurrentTargetPrefix();

    const wxString HTMLHelpLinkFileName();

    const ecFileName MLTDir ();
    const ecFileName CurrentLinkerScript();
    const wxString CurrentMemoryLayout ();

    const ecFileName GetDocBase() const { return GetRepository() + wxT("doc"); }

//// Operations
    bool OpenRepository(const wxString& pszRepository = wxEmptyString, bool bPromptInitially=FALSE) ;
    bool OpenRepository (ecFileName& strNewRepository, CdlPackagesDatabase &NewCdlPkgData,CdlInterpreter &NewCdlInterp,CdlConfiguration &NewCdlConfig, wxString &strNewPackagesDir);
    void CloseRepository() ;
    // Find a valid repository given a directory name
    bool FindRepository (ecFileName& repositoryIn, ecFileName& repositoryOut) const;
    void EnableCallbacks (bool bEnable=TRUE);
    void AddContents (const CdlContainer container, ecConfigItem *pParent);
    void AddAllItems();
    ecConfigItem* AddItem (const CdlUserVisible vitem, ecConfigItem * pParent);
    void DeleteItems();

    // Use the wxHtmlHelpController
    bool ShowInternalHtmlHelp (const wxString& strURL);

    // Use an external browser
    bool ShowExternalHtmlHelp (const wxString& strURL);

    bool ShowURL(const wxString& strURL);
    // If prefix is TRUE, add file:// to beginning
    bool QualifyDocURL(wxString &strURL, bool prefix = TRUE);
    void CheckRadios();
    bool GenerateHeaders();
    bool ExportFile();
    bool ImportFile();
    void RunTests();

    // A standalone method for generating a build tree without saving first
    bool GenerateBuildTree();
    // Can we generate the build tree yet?
    bool CanGenerateBuildTree() ;

    void SelectTemplate (const wxString& newTemplate, const wxString& newTemplateVersion);
    void SelectHardware (const wxString& newTemplate);
    void SelectPackages ();
	void RegenerateData();
    bool UpdateBuildInfo(bool bFirstTime=FALSE);
    int GetTestExeNames (wxArrayString& arTestExes, wxArrayString& arMissing) ;
    bool SaveMemoryMap();
    bool CopyMLTFiles();
    bool SwitchMemoryLayout (bool bNewTargetPlatform);
    bool NewMemoryLayout (const wxString &strPrefix);

	// Find the ecConfigItem referencing the given CdlValuable
	ecConfigItem * Find (CdlValuable v);
	ecConfigItem * Find(const wxString & strWhat, ecWhereType where = ecInMacro);

    enum GlobalConflictOutcome {OK,Cancel,NotDone};
    // Resolve conflicts
    GlobalConflictOutcome ResolveGlobalConflicts(wxList *parConflictsOfInterest=NULL);
    bool CheckConflictsBeforeSave();
    void LogConflicts (const std::list<CdlConflict> & conflicts);
    void UpdateFailingRuleCount();

    bool SetValue(ecConfigItem &ti, long nValue, CdlTransaction transaction = NULL);
    bool SetValue(ecConfigItem &ti, const wxString &strValue, CdlTransaction transaction = NULL);
    bool SetValue (ecConfigItem &ti, double dValue, CdlTransaction transaction = NULL);
    bool SetEnabled(ecConfigItem &ti, bool bEnabled, CdlTransaction transaction = NULL);

    // Rebuild the .hhc, .hhp, .hhk files and reinitialize the help controller
    bool RebuildHelpIndex(bool force = TRUE);

protected:
    CdlPackagesDatabase m_CdlPkgData;
    CdlInterpreter      m_CdlInterp;
    CdlConfiguration    m_CdlConfig;
    wxString            m_templateVersion;
    ecFileName          m_strPackagesDir;
    wxString            m_strCdlErrorMessage;
    wxString            m_strMemoryLayoutFolder;
    wxString            m_strLinkerScriptFolder;
    ecFileName          m_strBuildTree;
    ecFileName          m_strInstallTree;
    wxList              m_items;
    CdlBuildInfo		m_BuildInfo;

    // This data supports communication of information to the failing rules dialog invoked by CdlGlobalInferenceHandler():
    GlobalConflictOutcome m_ConflictsOutcome;
    wxList              m_arConflictsOfInterest; // used by CRulesView::OnResolve (TODO change comment)

public:
    ecFileName          m_strRepository;
    bool                m_bRepositoryOpen;

#if ecUSE_MLT
    mem_map             m_memoryMap;
    wxString            m_strSelectedSection;
    wxString            m_strSelectedRegion;
#endif
};


#endif
        // _ECOS_CONFIGTOOLDOC_H_
