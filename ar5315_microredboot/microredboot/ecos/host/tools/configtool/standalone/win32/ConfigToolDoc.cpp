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
// ConfigToolDoc.cpp : implementation of the CConfigToolDoc class
//
//
//===========================================================================
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the document class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include "stdafx.h"

#include "CTUtils.h"
#include "CdlTemplatesDialog.h"
#include "CellView.h"
#include "ConfigItem.h"
#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include "eCosTest.h"
#include "IdleMessage.h"
#include "MainFrm.h"
#include "PlatformDialog.h"
#include "PkgAdminDlg.h"
#include "RegKeyEx.h"
#include "RunTestsSheet.h"
#include "Thermometer.h"

#define INCLUDEFILE "build.hxx"
#include "IncludeSTL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// a trivial CDL parse error handler

/////////////////////////////////////////////////////////////////////////////
// CConfigToolDoc

IMPLEMENT_DYNCREATE(CConfigToolDoc, CDocument)

BEGIN_MESSAGE_MAP(CConfigToolDoc, CDocument)
//{{AFX_MSG_MAP(CConfigToolDoc)
ON_COMMAND(ID_CONFIGURATION_REPOSITORY, OnConfigurationRepository)
ON_COMMAND(ID_BUILD_TEMPLATES, OnBuildTemplates)
ON_COMMAND(ID_BUILD_PACKAGES, OnBuildPackages)
ON_COMMAND(ID_TOOLS_ADMINISTRATION, OnToolsAdministration)
ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigToolDoc construction/destruction


CConfigToolDoc::CConfigToolDoc():
  m_bUseCustomViewer(false),
  m_nFindFlags(0),
  m_nFindWhere(InMacro),
  m_bUseExternalBrowser(false),
  m_bAutoExpand(false),
  m_bMacroNames(false),
  m_bHex(false),
  m_CdlConfig(NULL),
  m_CdlPkgData(NULL),
  m_CdlInterp(NULL),
  m_template_version(""),
  m_bRepositoryOpen(false)
{
  CConfigTool::SetDocument(this);
  LoadProfileSettings();
}

CConfigToolDoc::~CConfigToolDoc()
{
  CConfigTool::SetDocument(0);
  SaveProfileSettings();
}

/////////////////////////////////////////////////////////////////////////////
// CConfigToolDoc serialization

void CConfigToolDoc::Serialize(CArchive& ar)
{
  if (ar.IsStoring())
  {
    // TODO: add storing code here
  }
  else
  {
    // TODO: add loading code here
  }
}

/////////////////////////////////////////////////////////////////////////////
// CConfigToolDoc diagnostics

#ifdef _DEBUG
void CConfigToolDoc::AssertValid() const
{
  CDocument::AssertValid();
}

void CConfigToolDoc::Dump(CDumpContext& dc) const
{
  CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CConfigToolDoc commands

bool CConfigToolDoc::SwitchMemoryLayout (BOOL bNewTargetPlatform)
{
  bool rc=true;
  if (bNewTargetPlatform && ! m_strBuildTree.IsEmpty ()) // the user has changed target/platform within a build tree
  {
    // copy default MLT save files for the selected target/platform from the repository to the build tree if they do not already exist
    
    CFileFind ffFileFind;
    BOOL bLastFile = ffFileFind.FindFile (CFileName (m_strPackagesDir, m_strMemoryLayoutFolder, _T("include\\pkgconf\\*.mlt")));
    while (bLastFile)
    {
      bLastFile = ffFileFind.FindNextFile ();
      if (! CFileName(MLTDir (), ffFileFind.GetFileName ()).Exists () && 
        !CUtils::CopyFile (ffFileFind.GetFilePath (), CFileName(MLTDir (), ffFileFind.GetFileName ()))){
        return false; // message already emitted
      }
    }
  }
  
  if (m_strBuildTree.IsEmpty ()) // load the memory layout from the repository
    rc&=NewMemoryLayout (CFileName (m_strPackagesDir, m_strMemoryLayoutFolder, _T("include\\pkgconf")));
  else // load the memory layout from the build tree
    rc&=NewMemoryLayout (MLTDir ());
  
  return true; // FIXME
}

void CConfigToolDoc::OnCloseDocument() 
{
  MemoryMap.new_memory_layout (); // called to free up memory
  CloseRepository();
  CDocument::OnCloseDocument();
}


BOOL CConfigToolDoc::OnNewDocument()
{
  m_strBuildTree=_T("");
  if (!CDocument::OnNewDocument()){
    return FALSE;
  }
  m_bRepositoryOpen = false;
  if(!OpenRepository()){
    PostQuitMessage (1);
    return FALSE;
  }
  // load the memory layout for the default target-platform-startup from the current repository
  
  MemoryMap.set_map_size (0xFFFFFFFF); // set the maximum memory map size
  NewMemoryLayout (CFileName (m_strPackagesDir, m_strMemoryLayoutFolder, _T("include\\pkgconf")));
  UpdateBuildInfo();
  return TRUE;
}

BOOL CConfigToolDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
  BOOL rc=FALSE; // Assume the worst
  CString strFileName;
  CdlInterpreter NewCdlInterp = NULL;
  CdlConfiguration NewCdlConfig = NULL;
  
  
  CFileName strFolder (lpszPathName);
  strFolder = strFolder.Left (strFolder.ReverseFind (_TCHAR('\\'))); // extract folder from file path
  
  if(CConfigTool::GetMain()){
    CString str;
    str.Format(_T("Opening save file %s"),lpszPathName);
    CConfigTool::GetMain()->m_wndStatusBar.SetPaneText(CMainFrame::StatusPane,str);
    CConfigTool::GetMain()->m_wndStatusBar.SetPaneText(CMainFrame::FailRulePane,_T("")); 
  }
  
  TRACE(_T("###Open document - save file=%s\n"), lpszPathName);
  
  // disable the transaction callback before attempting changes
  
  EnableCallbacks(false);
  
  // load the new configuration
  
  try
  {
    NewCdlInterp = CdlInterpreterBody::make ();
    NewCdlConfig = CdlConfigurationBody::load (CUtils::UnicodeToStdStr (lpszPathName), m_CdlPkgData, NewCdlInterp, &CdlLoadErrorHandler, &CdlLoadWarningHandler);
    rc = TRUE;
  }
  catch (CdlStringException exception)
  {
    CUtils::MessageBoxF(_T("Error opening eCos configuration:\n\n%s"), CString (exception.get_message ().c_str ()));
  }
  catch (...)
  {
    CUtils::MessageBoxF(_T("Error opening eCos configuration"));
  }
  
  if (rc)
  {
    rc=false;
    // check the new configuration
    
    ASSERT (NewCdlConfig->check_this (cyg_extreme));
    
    // switch to the new configuration
    
    deleteZ(m_CdlConfig);
    deleteZ(m_CdlInterp);
    m_CdlInterp = NewCdlInterp;
    m_CdlConfig = NewCdlConfig;
	  SetPathName (lpszPathName, TRUE); // called to ensure that MLTDir() will work in this function
    
    AddAllItems (); // must precede NewMemoryLayout() [CurrentLinkerScript() calls Find()]

    // load the memory layout from the build tree
    NewMemoryLayout (MLTDir ());

    // generate the CConfigItems from their CDL descriptions
  
  
    UpdateAllViews (NULL,AllSaved,0);
    UpdateFailingRuleCount();
    UpdateBuildInfo();
    rc=true;
  }
  
  // re-enable the transaction callback
  
  EnableCallbacks(true);
  
  return rc;
}

BOOL CConfigToolDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
  const CString strOldPath(GetPathName());
  bool bSaveAs=(lpszPathName!=strOldPath);
  if(!bSaveAs && !IsModified()){
    return true;
  }

  bool rc=false;
  if(CheckConflictsBeforeSave()){ // errors already emitted
  
    const CFileName strPathName(lpszPathName);
  
    TRACE(_T("###Save document %s\n"),lpszPathName);
  
    CString str;
    str.Format(_T("Saving configuration %s"), lpszPathName);
    CIdleMessage IM(str);
    if(CConfigTool::GetCellView()){
      CConfigTool::GetCellView()->CancelCellEdit();
    }
  
    // check the configuration
  
    ASSERT (m_CdlConfig->check_this (cyg_extreme));
  
    // save the configuration
  
    try
    {
      m_CdlConfig->save (CUtils::UnicodeToStdStr (lpszPathName));
      rc=true;
    }
  
    catch (CdlStringException exception)
    {
      CUtils::MessageBoxF(_T("Error saving eCos configuration:\n\n%s"), CString (exception.get_message ().c_str ()));
    }
  
    catch (...)
    {
      CUtils::MessageBoxF(_T("Error saving eCos configuration"));
    }
 
    if(rc){
      rc=false;
      SetPathName (lpszPathName, FALSE); // called to ensure that MLTDir() will work in this function
  
      // save the memory layout files to the build tree and copy to the install tree
  
      if (bSaveAs || MemoryMap.map_modified ()) {
        SaveMemoryMap();
      } else {
        TRACE(_T("Memory layout not modified\n"));
      }
  
      UpdateAllViews (NULL,AllSaved,0);
  
      if(!m_strBuildTree.CreateDirectory() || !m_strInstallTree.CreateDirectory()){
        CUtils::MessageBoxF(_T("Failed to save %s - %s"),lpszPathName,CUtils::GetLastErrorMessageString());
      } else if(GenerateHeaders() && CopyMLTFiles()){ // in each case errors already emitted
         // copy new MLT files to the build tree as necessary
        rc=generate_build_tree (GetCdlConfig(), CUtils::UnicodeToStdStr(m_strBuildTree), CUtils::UnicodeToStdStr(m_strInstallTree));
      }
    }
  }  
  if(rc){
    SetModifiedFlag (false);
    SetPathName (lpszPathName, TRUE); // add to MRU
  } else {
    SetPathName (strOldPath, FALSE);
  }
  return rc;
}

BOOL CConfigToolDoc::Reload()
{
  TRACE(_T("###ReOpen document - build tree=%s\n"),m_strBuildTree);
  UpdateAllViews (NULL,AllSaved,0);
  UpdateAllViews (NULL,ExternallyChanged,0);
  UpdateFailingRuleCount();
  
  return TRUE;
}

void CConfigToolDoc::OnConfigurationRepository() 
{
  SaveModified ();
  m_bRepositoryOpen=false;
  if(OpenRepository(Repository(),true)){
    // reset the document title as shown in the frame window
    GetDocTemplate ()->SetDefaultTitle (this);
    
    // load the memory layout for the default target-platform-startup from the new repository
    NewMemoryLayout (CFileName (m_strPackagesDir, m_strMemoryLayoutFolder, _T("include\\pkgconf")));
    UpdateAllViews(NULL);
    UpdateFailingRuleCount();
  } else {
    m_bRepositoryOpen=true; // keep what we had
  }
  
}

//////////////////////////////////////////////////////////////
// MLT-specific member functions

void CConfigToolDoc::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
  // enable the 'delete' menu item and toolbar button only if there is a region or section selected
  
  pCmdUI->Enable ((strSelectedRegion != _T("")) || (strSelectedSection != _T("")));
}


void CConfigToolDoc::OnUpdateEditNewSection(CCmdUI* pCmdUI) 
{
  // enable the 'new section' menu item and toolbar button only if there is at least one memory region defined
  
  pCmdUI->Enable (MemoryMap.region_list.size () > 0);
}

void CConfigToolDoc::OnUpdateEditProperties(CCmdUI* pCmdUI) 
{
  // enable the 'properties' menu item and toolbar button if there is a memory region or section selected	
  pCmdUI->Enable ((strSelectedRegion != _T("")) || (strSelectedSection != _T("")));
}

void CConfigToolDoc::LoadProfileSettings()
{
  CConfigToolApp *pApp=(CConfigToolApp *)AfxGetApp();
  m_eUseCustomBrowser=(BrowserType)pApp->GetProfileInt   (_T("View"),	_T("CustomBrowser"), Internal);
  m_bHex=						      pApp->GetProfileInt   (_T("Format"),_T("Hex"),	0);
  m_bMacroNames=			    pApp->GetProfileInt   (_T("Format"),_T("MacroNames"),	0);
  m_nFindFlags=				    pApp->GetProfileInt   (_T("Find"),	_T("Flags"), FR_DOWN);
  m_strFind=					    pApp->GetProfileString(_T("Find"),	_T("String"));
  m_nFindWhere=(WhereType)pApp->GetProfileInt   (_T("Find"),	_T("Where"),0);
  m_bUseCustomViewer=	    pApp->GetProfileInt   (_T("View"),	_T("CustomViewer"), 0);
  m_bAutoExpand=			    pApp->GetProfileInt   (_T("View"),	_T("Auto-expand"),	1);
  m_nRuleChecking=        pApp->GetProfileInt   (_T("Rule"),	_T("Checking"), Immediate|Deferred|SuggestFixes);
  
  CRegKeyEx k(HKEY_CURRENT_USER,CUtils::LoadString(IDS_REPOSITORY_REGKEY), KEY_READ);

  if(!k.QueryValue(CUtils::LoadString(IDS_REPOSITORY_REGVALUE), m_strRepository)){
    CStringArray arstr;
    switch(pApp->GetRepositoryRegistryClues(arstr,_T("eCos"))){
      case 0:
        break;
      case 1:
      default:
        SetRepository(arstr[0]);
        break;
    }
  }
}

void CConfigToolDoc::SaveProfileSettings()
{
  CConfigToolApp *pApp=(CConfigToolApp *)AfxGetApp();
  pApp->WriteProfileInt   (_T("Find"),	 _T("Where"),m_nFindWhere);
  pApp->WriteProfileString(_T("View"),	 _T("Viewer"), m_strViewer);
  pApp->WriteProfileInt   (_T("View"),	 _T("CustomViewer"), m_bUseCustomViewer);
  pApp->WriteProfileInt   (_T("View"),	 _T("CustomBrowser"), m_eUseCustomBrowser);
  pApp->WriteProfileInt   (_T("View"),	 _T("Auto-expand"),	m_bAutoExpand);
  
  pApp->WriteProfileInt   (_T("Format"), _T("Hex"),	m_bHex);
  pApp->WriteProfileInt   (_T("Format"), _T("MacroNames"),	m_bMacroNames);
  
  pApp->WriteProfileInt   (_T("Find"),	 _T("Flags"), m_nFindFlags);
  pApp->WriteProfileString(_T("Find"),	 _T("String"), m_strFind);
  
  pApp->WriteProfileInt   (_T("Rule"),   _T("Checking"), m_nRuleChecking);
  
  CRegKeyEx k(HKEY_CURRENT_USER,CUtils::LoadString(IDS_REPOSITORY_REGKEY), KEY_WRITE);
  k.SetValue(m_strRepository, CUtils::LoadString(IDS_REPOSITORY_REGVALUE));
}

void CConfigToolDoc::OnBuildTemplates () 
{
  CCdlTemplatesDialog dlg (FALSE);
  
  if (IDOK == dlg.DoModal ()){
    SelectHardware(dlg.GetSelectedHardware());
    SelectTemplate(dlg.GetSelectedTemplate(), dlg.GetSelectedTemplateVersion());
  }
}

void CConfigToolDoc::OnBuildPackages() 
{
  SelectPackages ();
}


bool CConfigToolDoc::ShowURL(LPCTSTR pszURL)
{
  bool rc=true;

  CString strURL(pszURL);
  if(!QualifyDocURL(strURL)){
    return false; // error message already output
  }
    
  CConfigToolApp*pApp=(CConfigToolApp*)AfxGetApp();
  
  switch(m_eUseCustomBrowser){
    case Internal:
      rc=CConfigToolDoc::ShowHtmlHelp(strURL);  
      break;
    case AssociatedExternal:
      {
        // A bit of a problem here: we can't execute the document, because this doesn't work with "file" schemes or
        // with URL fragments (#...).  So we'll launch the associated program and hope there aren't too many interesting
        // flags specified with the association
        CString strBrowser;
        if(DefaultExternalBrowser().IsEmpty()){
          CUtils::MessageBoxF(_T("Failed to to determine associated external browser to open %s"),strURL);
        } else {
          pApp->Launch(strURL,DefaultExternalBrowser());
        }
      }
      break;
    case CustomExternal:
      pApp->Launch(strURL,m_strBrowser);
      break;
    default:
      ASSERT(FALSE);
  }
  return rc;
}

const CString CConfigToolDoc::CurrentTargetPrefix()
{
  CConfigItem *pItem=Find(_T("CYGBLD_GLOBAL_COMMAND_PREFIX"));
  return pItem?pItem->StringValue():_T("");
}

int CConfigToolDoc::GetCompilationCount(LPCTSTR psz)
{
  int rc=0;
  CStringArray arstr;
  const CString strPrefix(CurrentTargetPrefix());
  for(int i=CUtils::Chop(psz,arstr,_T("\n"))-1;i>=0;--i){
    if(-1!=arstr[i].Find(strPrefix)){
      rc++;
    }
  }
	return rc;
}

void CConfigToolDoc::RunTests()
{
  const CString strTarget(CurrentTestingIdentifier());
  TRACE (_T("OnRunSim(): test target ID = '%s'\n"), strTarget);
  
  if (NULL==CeCosTestPlatform::Get(strTarget)) {
    if(IDNO==CUtils::MessageBoxFT(MB_YESNO,_T("%s is not a recognized platform - do you wish to add it?"),strTarget)){
      return;
    }
    CPlatformDialog dlg;
    dlg.m_strPlatform=strTarget;
    dlg.m_strPrefix=CurrentTargetPrefix();
    dlg.m_strCaption=_T("New Platform");
    if(IDCANCEL==dlg.DoModal()){
      return;
    }
    CeCosTestPlatform::Add(CeCosTestPlatform(dlg.m_strPlatform,dlg.m_strPrefix,dlg.m_strPrompt,dlg.m_strGDB,dlg.m_bServerSideGdb,dlg.m_strInferior));  
    CeCosTestPlatform::Save();
  }

  CFileNameArray ar;
  CFileNameArray arTestsMissing;
  int nTests=GetTestExeNames(ar,arTestsMissing);
  CRunTestsSheet sheet(_T("Run Tests"), NULL, 0, 0);
  sheet.SetTarget(strTarget);
  const CeCosTestPlatform * etPlatform = CeCosTestPlatform::Get(strTarget);
  ASSERT (NULL != etPlatform);
  if (-1 != CString (etPlatform->GdbCmds ()).Find (_T("cyg_test_is_simulator"))) { // if a simulator target
    sheet.SetResetNone(); // disable 'reset hardware' message box
  }
  sheet.HideRemoteControls();
  if(arTestsMissing.GetSize()){
    if(IDYES==CUtils::MessageBoxFT(MB_YESNO,_T("Not all tests are built.  Do you wish to build them now?"))){
      CConfigTool::GetMain()->SendMessage(WM_COMMAND,ID_BUILD_TESTS);
      return;
    }
  }
  if(CConfigTool::GetMain()->PrepareEnvironment()){
    for(int i=0;i<nTests;i++){
      sheet.Populate(ar[i],true);
    }
    for(i=0;i<arTestsMissing.GetSize();i++){
      sheet.Populate(arTestsMissing[i],false);
    }
    sheet.DoModal();
  }
}

void CConfigToolDoc::OnToolsAdministration() 
{
  if(IDYES==CUtils::MessageBoxFT(MB_YESNO,_T("This command will close the current document.\n\nDo you wish to continue?"))){
    // ensure that the user tools are on the path for use by ecosadmin.tcl
    if(CConfigTool::GetMain()->PrepareEnvironment(/* bWithBuildTools = */ false)){
      CConfigToolApp *pApp=(CConfigToolApp *)AfxGetApp();
      CPkgAdminDlg dlg(PackagesDir(),pApp->m_strUserToolsDir); // make sure we use doc data before the doc is destroyed
      CMainFrame *pMain=CConfigTool::GetMain();
      if(IDOK==dlg.DoModal()){
        pMain->PostMessage(WM_COMMAND,ID_FILE_NEW);
      }
    }
  }
}
