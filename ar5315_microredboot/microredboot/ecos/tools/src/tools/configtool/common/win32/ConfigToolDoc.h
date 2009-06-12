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
// ConfigToolDoc.h
//
/////////////////////////////////////////////////////////////////////////////
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	interface of the CConfigToolDoc class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

#if !defined(AFX_ConfigToolDOC_H__A4845246_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_)
#define AFX_ConfigToolDOC_H__A4845246_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define INCLUDEFILE <string>
#include "IncludeSTL.h"
#define INCLUDEFILE "cdl.hxx"
#include "IncludeSTL.h"

#include "memmap.h" // for mem_map

#include "ConfigTool.h"
#include "FileName.h"

class CConfigItem;

#ifdef PLUGIN
class CProject;
class CProjectManager;
class Project; 
class CConfigToolDoc {
private:
  static LPCTSTR CALLBACK GetFn (void *pObj) { return (LPCTSTR)*(CString *)pObj; }
  static void    CALLBACK PutFn (void *pObj,LPCTSTR psz) { *(CString *)pObj=psz; }
#else
  class CConfigToolDoc : public CDocument {
  protected: 
    DECLARE_DYNCREATE(CConfigToolDoc)
#endif
      
    // Attributes
  public:
    static void CdlParseErrorHandler (std::string message);
    static void CdlParseWarningHandler (std::string message);

    // ctors and dtors: public for benefit of plugin case
    CConfigToolDoc();
    virtual ~CConfigToolDoc();
    
    void RegenerateData();
    
    // Hints.
    // These are mostly relevant to the properties view, which will generally ignore them unless
    // the pHint matches what it is currently displaying (i.e. the last SelChanged message)
    enum {SelChanged=1,IntFormatChanged=2,ValueChanged=3, Clear=4, AllSaved=6, NameFormatChanged=7, ExternallyChanged=8, MemLayoutChanged=9};
    
    enum {Never=0, Immediate=1, Deferred=2, SuggestFixes=4};
    int m_nRuleChecking; // OR of above values
    
    // Configuration items
    CConfigItem * FirstItem(){return ItemCount()==0?NULL:Item(0);}
    CConfigItem *Item(int nIndex){ return (CConfigItem *)m_arItem[nIndex];}
    int ItemCount(){ return m_arItem.GetSize();}
    
    void SelectTemplate (std::string NewTemplate, std::string NewTemplateVersion);
    void SelectHardware (std::string NewTemplate);

    void SelectPackages ();
    CString GetPackageName (const CString & strAlias);
    
    // MLT-related
    mem_map MemoryMap;
    CString strSelectedSection;
    CString strSelectedRegion;
    bool SwitchMemoryLayout (BOOL bNewTargetPlatform);
    bool SaveMemoryMap();

    const CFileName BuildTree()     const { return m_strBuildTree;   }
    const CFileName InstallTree()   const { return m_strInstallTree; }
    const CFileName HeadersDir()    const { return InstallTree()+_T("include"); }
    const CFileName ObjectDir()     const { return BuildTree()+_T("obj"); }
    const CFileName MLTDir ();
    const CFileName DocBase()       const { return Repository()+_T("doc"); }
    
    void SetBuildTree (LPCTSTR pszBuild) { m_strBuildTree=pszBuild; }
    void SetInstallTree (LPCTSTR pszInstall) { m_strInstallTree=pszInstall; }
    
    void UpdateFailingRuleCount();
    bool GetRunPlatform (CString &strTarget);
    
    bool SetEnabled (CConfigItem &ti, bool bEnabled, CdlTransaction transaction=NULL);
    bool SetValue(CConfigItem &ti,ItemIntegerType nValue, CdlTransaction transaction=NULL);
    bool SetValue(CConfigItem &ti,const CString &strValue, CdlTransaction transaction=NULL);
    bool SetValue (CConfigItem &ti, double dValue, CdlTransaction transaction=NULL);
    CString GetDefaultHardware ();
    
    CString m_strFind;
    int m_nFindFlags;
    WhereType m_nFindWhere;
    
    bool m_bAutoExpand;
    bool m_bMacroNames;
    bool m_bHex;
    
    bool ShowURL (LPCTSTR pszURL);
  	static const CFileName &DefaultExternalBrowser ();

    // Absolute path name to root of respository - parent of PackagesDir below
    const CFileName Repository()  const { return m_strRepository; }
    void SetRepository(LPCTSTR pszRepository)  { m_strRepository=CFileName(pszRepository); }
    
    // Absolute path name to "packages" directory - i.e. the one under the repository
    // named "packages" in a delivery or "ecc" in CVS:
    const CFileName PackagesDir() const { return m_strPackagesDir; }
    
    CConfigItem * Find (const CString &strWhat,WhereType where=InMacro);
    CConfigItem * Find (CdlValuable v);
    
    const CFileName CurrentLinkerScript();
    const CString CurrentStartup();
    const CString CurrentTestingIdentifier();
    const CString CurrentPlatform();
    const CString CurrentMemoryLayout ();
    CFileName m_strMemoryLayoutFolder;
    CFileName m_strLinkerScriptFolder;
    const CFileName CurrentPlatformPackageFolder();
    bool OpenRepository(LPCTSTR pszRepository=NULL,bool bPromptInitially=false);
    void CloseRepository();
    CdlPackagesDatabase GetCdlPkgData () { return m_CdlPkgData; }
    CdlConfiguration GetCdlConfig () { return m_CdlConfig; }
    CdlInterpreter GetCdlInterpreter() { return m_CdlInterp; }
    std::string GetTemplateVersion() { return m_template_version; }

    bool UpdateBuildInfo(bool bFirstTime=false);
    CdlBuildInfo &BuildInfo() { return m_BuildInfo; }
  
    enum GlobalConflictOutcome {OK,Cancel,NotDone};
    GlobalConflictOutcome ResolveGlobalConflicts(CPtrArray *parConflictsOfInterest=NULL);

    bool CheckConflictsBeforeSave();
    int GetTestExeNames (CFileNameArray &ar,CFileNameArray &arTestsMissing);
    BOOL IsModified();
    void SetPathName( LPCTSTR pszPath, BOOL bAddToMRU = TRUE );
    static bool ShowHtmlHelp(LPCTSTR pszURL);
	  static const CString HTMLHelpLinkFileName(); // the full path to "link2.htm"
  
  protected:
    void AddAllItems ();
    void RemoveAllItems();
    void LogConflicts (const std::list<CdlConflict> & conflicts);
    void ShowSectionProperties();
    void ShowRegionProperties();
    void ErrorBox (UINT, UINT);

    void EnableCallbacks (bool bEnable=true);

    CdlPackagesDatabase m_CdlPkgData;
    CdlInterpreter m_CdlInterp;
    CdlConfiguration m_CdlConfig;
    std::string m_template_version;
    static void CdlTransactionHandler (const CdlTransactionCallback & data);
    static CdlInferenceCallbackResult CdlInferenceHandler (CdlTransaction data);
    static CdlInferenceCallbackResult CdlGlobalInferenceHandler(CdlTransaction data);
    static void CdlLoadErrorHandler(std::string message);
    static void CdlLoadWarningHandler(std::string message);
    static CFileName m_strDefaultExternalBrowser;
    CdlBuildInfo m_BuildInfo;
    CFileName m_strBuildTree;
    CFileName m_strInstallTree;
    bool CopyMLTFiles();
    CFileName m_strRepository;
    bool m_bRepositoryOpen;
    
    CPtrArray m_arItem;
    
    bool NewMemoryLayout (const CString &strPrefix);
    CFileName m_strPackagesDir;
    void CheckRadios();
    
    // CDL interface
    void AddContents (const CdlContainer container, CConfigItem * pParent);
    CConfigItem * AddItem (const CdlUserVisible visible, CConfigItem * pParent);
    CString m_strCdlErrorMessage;
    bool OpenRepository (const CFileName strNewRepository,CdlPackagesDatabase &NewCdlPkgData,CdlInterpreter &NewCdlInterp,CdlConfiguration &NewCdlConfig,CFileName &strNewPackagesDir);
    bool GenerateHeaders();
    
    // This data supports communication information to the failing rules dialog invoked by CdlGlobalInferenceHandler():
    GlobalConflictOutcome m_ConflictsOutcome;
    CPtrArray m_arConflictsOfInterest; // used by CRulesView::OnResolve 

    bool QualifyDocURL(CString &strURL);

#ifdef PLUGIN
  protected:
    CProject * m_peCosProject;

  public:
    bool CreateTestProjects();
    CProjectManager *m_pm; // A pointer so as to reduce dependencies on this header file
    void SeteCosProject(CProject *peCosProject){m_peCosProject=peCosProject; }
    CProject *eCosProject(){return m_peCosProject; }
    // Here are the declarations that make us look like a CDocument-derived class:
  public:
	  bool UnloadPackage (CConfigItem *pItem);
    void OnMLTNewRegion();
    void OnMLTNewSection();
    void OnMLTDelete();
    void OnMLTProperties();
	void OnFileExport();
	void OnFileImport();
    
    // Making the class look as if it is derived from CDocument:
    BOOL OnNewDocument();
    BOOL OnOpenDocument(LPCTSTR lpszPathName);
    void OnCloseDocument();
    BOOL OnSaveDocument(LPCTSTR lpszPathName);
    void DeleteContents();
    void SetModifiedFlag( BOOL bModified = TRUE ) { m_bModified=bModified; }
    void UpdateAllViews( CView* pSender, LPARAM lHint = 0L, CObject* pHint = NULL );
    const CString& GetPathName( ) const { return m_strPathName; }

    bool PostOpenDocument();
  protected:
    CString m_strPathName;
    BOOL m_bModified;
#else
    //{{AFX_VIRTUAL(CConfigToolDoc)
  public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual void OnCloseDocument();
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
    virtual void DeleteContents();
  protected:
    //	    virtual BOOL SaveModified();
    //}}AFX_VIRTUAL
  public:
    // Standlone-specific:
    void RunTests();
    enum BrowserType { Internal, AssociatedExternal, CustomExternal };
    BOOL Reload();
    CStringArray m_arstrUserToolPaths;
    bool m_bUseCustomViewer;
    bool m_bUseExternalBrowser;
    CString m_strBrowser;
    CFileName m_strViewer;
    BrowserType m_eUseCustomBrowser;
    const CString CurrentTargetPrefix();
    int GetCompilationCount(LPCTSTR psz);

  protected:

    CString m_strBufferedLogMessages;
    // Access to header files:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
  public:
    void SetModifiedFlag (BOOL bModified = TRUE) { CDocument::SetModifiedFlag (bModified); UpdateFrameCounts (); }
    //	    void SetTitle(LPCTSTR lpszTitle);
    
    void SaveProfileSettings();
    void LoadProfileSettings();
    //{{AFX_MSG(CConfigToolDoc)
    afx_msg void OnBuildConfigure();
    afx_msg void OnConfigurationRepository();
    afx_msg void OnBuildTemplates();
    afx_msg void OnBuildPackages();
    afx_msg void OnMLTNewRegion();
    afx_msg void OnMLTNewSection();
    afx_msg void OnMLTDelete();
    afx_msg void OnMLTProperties();
    afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditNewSection(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditProperties(CCmdUI* pCmdUI);
  	afx_msg void OnToolsAdministration();
    afx_msg void OnFileExport();
    afx_msg void OnFileImport();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
#endif
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ConfigToolDOC_H__A4845246_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_)
