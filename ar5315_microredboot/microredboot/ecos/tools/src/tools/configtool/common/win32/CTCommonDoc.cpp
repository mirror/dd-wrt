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
#include "stdafx.h"

#include "ConfigToolDoc.h"

#ifdef PLUGIN
  #include "CTMain.h"
  #include "Project.h"
  #include "ProjectManager.h"
#else
  #include "MainFrm.h"
#endif

#include "CTUtils.h"
#include "CdlPackagesDialog.h"
#include "ConfigItem.h"
#include "ConfigTool.h"
#include "ControlView.h"
#include "FailingRulesDialog.h"
#include "FolderDialog.h"
#include "IdleMessage.h"
#include "MLTView.h"
#include "NotePage.h"
#include "RegionGeneralPage.h"
#include "RegKeyEx.h"
#include "RulesView.h"
#include "SectionGeneralPage.h"
#include "SectionRelocationPage.h"

#include <htmlhelp.h>
#include <shlobj.h>

#define INCLUDEFILE <string>
#include "IncludeSTL.h"
#define INCLUDEFILE "build.hxx"
#include "IncludeSTL.h"

#ifdef _DEBUG
  #define new DEBUG_NEW
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

const CFileName &CConfigToolDoc::DefaultExternalBrowser()
{
  static bool bFirstTime=true;
  if(bFirstTime){
    const CFileName strFile(CFileName::GetTempPath()+_T("index.html"));
    CFile f;
    if(f.Open(strFile,CFile::modeCreate|CFile::modeWrite)){
      f.Close();
      bool rc=((int)FindExecutable(strFile,_T("."),m_strDefaultExternalBrowser.GetBuffer(MAX_PATH))>32);
      m_strDefaultExternalBrowser.ReleaseBuffer();
      if(!rc){
        m_strDefaultExternalBrowser=_T("");
      }
      ::DeleteFile(strFile);
    }
    bFirstTime=false;
  }
  return m_strDefaultExternalBrowser;
}

CFileName CConfigToolDoc::m_strDefaultExternalBrowser;

void CConfigToolDoc::CheckRadios()
{
  for(int nItem=0;nItem<ItemCount();nItem++){
    CConfigItem *pItem=Item(nItem);
    if(pItem->HasRadio () && pItem==pItem->FirstRadio()){
      CString strMsg;
      CConfigItem *pFirstSet=NULL;
      for(CConfigItem *pSibItem=pItem;pSibItem;pSibItem=pSibItem->NextRadio()){
        if(pSibItem->IsEnabled ()){
          if(pFirstSet){
            strMsg+=_T(" ");
            strMsg+=pSibItem->Macro ();
          } else {
            pFirstSet=pSibItem;
          }
        }
      }
      if(!strMsg.IsEmpty()){
        CConfigTool::Log(_T("%s, multiple radio buttons are set: %s%s"),
          pItem->Macro(),pFirstSet->Macro(),strMsg);
      } else if(!pFirstSet){
        CConfigTool::Log(_T("%s, no radio buttons are set"),pItem->Macro());
      }
    }
  }
}

// Find the ConfigItem referencing the given CdlValuable
CConfigItem * CConfigToolDoc::Find (CdlValuable v)
{
  for(int nItem=0;nItem<ItemCount();nItem++){
    CConfigItem *pItem=Item(nItem);
    if(v==pItem->GetCdlValuable()){
      return pItem;
    }
  }
  return NULL;
}

CConfigItem * CConfigToolDoc::Find(const CString & strWhat,WhereType where)
{
  for(int nItem=0;nItem<ItemCount();nItem++){
    CConfigItem *pItem=Item(nItem);
    if(0==pItem->StringValue(where).Compare(strWhat)){
      return pItem;
    }
  }
  return NULL;
}

void CConfigToolDoc::OnMLTNewRegion() 
{
  // create the property sheet
  
  CPropertySheet prsRegionSheet (IDS_NEW_REGION_SHEET_TITLE, CConfigTool::GetMLTView());
  
  // add the property pages
  
  CRegionGeneralPage prpGeneral;
  CNotePage prpNote;
  
  prsRegionSheet.AddPage (&prpGeneral);
  prsRegionSheet.AddPage (&prpNote);
  
  // setup initial values
  
  prpGeneral.m_strRegionName = _T("");
  prpGeneral.m_dwRegionStartAddress = 0;
  prpGeneral.m_dwRegionSize = 0;
  
  // show property sheet
  
  if (prsRegionSheet.DoModal () == IDOK)
  {
    if (int nErrorCode = MemoryMap.create_memory_region (CUtils::UnicodeToStdStr(prpGeneral.m_strRegionName), prpGeneral.m_dwRegionStartAddress, prpGeneral.m_dwRegionSize, (prpGeneral.m_bRegionReadOnly ? read_only : read_write), CUtils::UnicodeToStdStr(prpNote.m_strNote)))
      ErrorBox (IDS_ERR_MEMMAP_REGION_CREATE, IDS_ERR_MEMMAP_BASE + nErrorCode);
    else // no errors
    {
      SetModifiedFlag ();
      UpdateAllViews (NULL, MemLayoutChanged);
    }
  }		
}

void CConfigToolDoc::ErrorBox (UINT uIDSIntro, UINT uIDSError)
{
  CString strIntro, strError;
  strIntro.LoadString (uIDSIntro);
  strError.LoadString (uIDSError);
  AfxMessageBox (strIntro + _T("\n\n") + strError);
}

void CConfigToolDoc::OnMLTNewSection() 
{
  // create the property sheet
  
  CPropertySheet prsSectionSheet (IDS_NEW_SECTION_SHEET_TITLE, CConfigTool::GetMLTView());
  
  // add the property pages
  
  CSectionGeneralPage prpGeneral;
  CSectionRelocationPage prpRelocation;
  CNotePage prpNote;
  
  prsSectionSheet.AddPage (&prpGeneral);
  prsSectionSheet.AddPage (&prpRelocation);
  prsSectionSheet.AddPage (&prpNote);
  
  // initialise for a new section
  
  prpGeneral.m_bNameLinkerDefined = TRUE;
  prpGeneral.m_strNameLinker = _T("");
  prpRelocation.m_bRelocates = FALSE;
  prpRelocation.m_bNewSection = TRUE;
  
  // show property sheet
  
  if (prsSectionSheet.DoModal () == IDOK)
  {
    mem_anchor initial_anchor, final_anchor;
    mem_address initial_address;
    CString strInitialRelativeName = _T("");
    
    if (prpGeneral.m_bFinalAbsolute) // final location is an absolute location
      final_anchor = absolute;
    else // final location is a relative location
      final_anchor = relative;
    
    if (prpRelocation.m_bRelocates) // if the section relocates
    {
      if (prpRelocation.m_bInitialAbsolute) // initial location is an absolute location
      {
        initial_anchor = absolute;
      }
      else // initial location is a relative location
      {
        strInitialRelativeName = prpRelocation.m_strInitialRelativeName;
        initial_anchor = relative;
      }
      initial_address = prpRelocation.m_dwInitialAddress;
    }
    else
    {
      strInitialRelativeName = prpGeneral.m_strFinalRelativeName;
      initial_anchor = final_anchor;
      initial_address = prpGeneral.m_dwFinalAddress;
    }
    
    
    if (int nErrorCode = MemoryMap.create_memory_section (CUtils::UnicodeToStdStr(prpGeneral.m_bNameLinkerDefined ? prpGeneral.m_strNameLinker : prpGeneral.m_strNameUser), prpGeneral.m_dwSectionSize, prpGeneral.m_dwAlignment, initial_anchor, CUtils::UnicodeToStdStr(strInitialRelativeName), initial_address, final_anchor, CUtils::UnicodeToStdStr(prpGeneral.m_strFinalRelativeName), prpGeneral.m_dwFinalAddress, prpRelocation.m_bRelocates, false, prpGeneral.m_bNameLinkerDefined, CUtils::UnicodeToStdStr(prpNote.m_strNote)))
      ErrorBox (IDS_ERR_MEMMAP_SECTION_CREATE, IDS_ERR_MEMMAP_BASE + nErrorCode);
    else // no errors
    {
      SetModifiedFlag ();
      UpdateAllViews (NULL, MemLayoutChanged);
    }
  }		
}

void CConfigToolDoc::OnMLTDelete() 
{
  if (strSelectedSection != _T("")) // a section is selected
  {
    if (MemoryMap.delete_memory_section (CUtils::UnicodeToStdStr(strSelectedSection)))
    {
      strSelectedSection = _T("");
      SetModifiedFlag ();
      UpdateAllViews (NULL, MemLayoutChanged);
    }
    else
      AfxMessageBox (_T("Could not delete memory section.\n\nMake sure there are no other sections defined relative to this one."));
  }
  else if (strSelectedRegion != _T("")) // a region is selected
  {
    if (MemoryMap.delete_memory_region (CUtils::UnicodeToStdStr(strSelectedRegion)))
    {
      strSelectedRegion = _T("");
      SetModifiedFlag ();
      UpdateAllViews (NULL, MemLayoutChanged);
    }
    else
      AfxMessageBox (_T("Could not delete memory region.\n\nMake sure it is not in use.")); // FIXME
  }
}

void CConfigToolDoc::ShowRegionProperties()
{
  // create the property sheet
  
  CString strTitle;
  strTitle.LoadString (IDS_REGION_PROPERTIES_SHEET_TITLE);
  CPropertySheet prsRegionSheet (strSelectedRegion + _T(" - ") + strTitle, CConfigTool::GetMLTView());
  
  // add the property pages
  
  CRegionGeneralPage prpGeneral;
  CNotePage prpNote;
  
  prsRegionSheet.AddPage (&prpGeneral);
  prsRegionSheet.AddPage (&prpNote);
  
  // set up property sheet values
  
  mem_type type;
  std::string note;
  MemoryMap.get_memory_region (CUtils::UnicodeToStdStr(strSelectedRegion), &prpGeneral.m_dwRegionStartAddress, &prpGeneral.m_dwRegionSize, &type, &note);
  prpGeneral.m_strRegionName = strSelectedRegion;
  prpGeneral.m_bRegionReadOnly = (type == read_only);
  prpNote.m_strNote = note.c_str ();
  
  // show property sheet
  
  if (prsRegionSheet.DoModal () == IDOK)
  {
    if (int nErrorCode = MemoryMap.edit_memory_region (CUtils::UnicodeToStdStr(strSelectedRegion), CUtils::UnicodeToStdStr(prpGeneral.m_strRegionName), prpGeneral.m_dwRegionStartAddress, prpGeneral.m_dwRegionSize, (prpGeneral.m_bRegionReadOnly ? read_only : read_write), CUtils::UnicodeToStdStr(prpNote.m_strNote)))
      ErrorBox (IDS_ERR_MEMMAP_REGION_MODIFY, IDS_ERR_MEMMAP_BASE + nErrorCode);
    else // no errors
    {
      SetModifiedFlag ();
      UpdateAllViews (NULL, MemLayoutChanged);
    }
  }		
}

void CConfigToolDoc::ShowSectionProperties()
{
  // create the property sheet
  
  CString strTitle;
  strTitle.LoadString (IDS_SECTION_PROPERTIES_SHEET_TITLE);
  CPropertySheet prsSectionSheet (strSelectedSection + _T(" - ") + strTitle, CConfigTool::GetMLTView());
  
  // add the property pages
  
  CSectionGeneralPage prpGeneral;
  CSectionRelocationPage prpRelocation;
  CNotePage prpNote;
  
  prsSectionSheet.AddPage (&prpGeneral);
  prsSectionSheet.AddPage (&prpRelocation);
  prsSectionSheet.AddPage (&prpNote);
  
  // setup initial values
  
  std::list <mem_section>::iterator section = MemoryMap.find_memory_section (CUtils::UnicodeToStdStr(strSelectedSection));
  prpGeneral.m_bNameLinkerDefined = (section->linker_defined);
  if (prpGeneral.m_bNameLinkerDefined)
    prpGeneral.m_strNameLinker = section->name.c_str ();
  else
  {
    prpGeneral.m_strNameUser = section->name.c_str ();
    prpGeneral.m_dwSectionSize = section->size;
  }
  
  prpGeneral.m_bFinalAbsolute = (section->final_location->anchor == absolute);
  if (prpGeneral.m_bFinalAbsolute)
    prpGeneral.m_dwFinalAddress = section->final_location->address;
  else
  {
    prpGeneral.m_strFinalRelativeName = MemoryMap.find_preceding_section (section, false)->name.c_str ();
    prpGeneral.m_dwAlignment = section->alignment;
  }
  
  prpRelocation.m_bRelocates = section->relocates;
  if (prpRelocation.m_bRelocates)
  {
    prpRelocation.m_bInitialAbsolute = (section->initial_location->anchor == absolute);
    if (prpRelocation.m_bInitialAbsolute)
      prpRelocation.m_dwInitialAddress = section->initial_location->address;
    else
      prpRelocation.m_strInitialRelativeName = MemoryMap.find_preceding_section (section, true)->name.c_str ();
  }
  
  prpNote.m_strNote = section->note.c_str ();
  
  // show property sheet
  
  if (prsSectionSheet.DoModal () == IDOK)
  {
    mem_anchor initial_anchor, final_anchor;
    mem_address initial_address;
    CString strInitialRelativeName = _T("");
    
    if (prpGeneral.m_bFinalAbsolute) // final location is an absolute location
      final_anchor = absolute;
    else // final location is a relative location
      final_anchor = relative;
    
    if (prpRelocation.m_bRelocates) // if the section relocates
    {
      if (prpRelocation.m_bInitialAbsolute) // initial location is an absolute location
      {
        initial_anchor = absolute;
      }
      else // initial location is a relative location
      {
        strInitialRelativeName = prpRelocation.m_strInitialRelativeName;
        initial_anchor = relative;
      }
      initial_address = prpRelocation.m_dwInitialAddress;
    }
    else
    {
      strInitialRelativeName = prpGeneral.m_strFinalRelativeName;
      initial_anchor = final_anchor;
      initial_address = prpGeneral.m_dwFinalAddress;
    }
    
    
    if (int nErrorCode = MemoryMap.edit_memory_section (CUtils::UnicodeToStdStr(strSelectedSection), CUtils::UnicodeToStdStr(prpGeneral.m_bNameLinkerDefined ? prpGeneral.m_strNameLinker : prpGeneral.m_strNameUser), prpGeneral.m_dwSectionSize, prpGeneral.m_dwAlignment, initial_anchor, CUtils::UnicodeToStdStr(strInitialRelativeName), initial_address, final_anchor, CUtils::UnicodeToStdStr(prpGeneral.m_strFinalRelativeName), prpGeneral.m_dwFinalAddress, prpRelocation.m_bRelocates, false, prpGeneral.m_bNameLinkerDefined, CUtils::UnicodeToStdStr(prpNote.m_strNote)))
      ErrorBox (IDS_ERR_MEMMAP_SECTION_MODIFY, IDS_ERR_MEMMAP_BASE + nErrorCode);
    else // no errors
    {
      SetModifiedFlag ();
      UpdateAllViews (NULL, MemLayoutChanged);
    }
  }		
}

void CConfigToolDoc::OnMLTProperties() 
{
  if (strSelectedRegion != _T(""))
    ShowRegionProperties ();
  else if (strSelectedSection != _T(""))
    ShowSectionProperties ();
}

// Choose a default Hal.  Do this using clues the installer may have helpfully left behind (PR 18050)
// or else if there is only one possible choice, by choosing that one :-).
/*
void CConfigToolDoc::ChooseDefaultHal()
{
bool bFound=false;
// Has the installer told us?
LPCTSTR pszRegPath=_T("SOFTWARE\\Red Hat\\eCos\\1.2.8");
HKEY hKey;
if(ERROR_SUCCESS==RegOpenKeyEx (HKEY_LOCAL_MACHINE, pszRegPath, 0L, KEY_READ, &hKey)){
DWORD dwSizePath=MAX_PATH;
CString str;
PTCHAR psz=str.GetBuffer(dwSizePath);
if(ERROR_SUCCESS==RegQueryValueEx(hKey, _T("Default Architecture"), NULL, NULL, (LPBYTE)psz, &dwSizePath)){
str.MakeLower();
for(int nItem=0;nItem<ItemCount();nItem++){
CConfigItem *pItem=Item(nItem);
if(pItem->m_pTarget){
CString strTarget(pItem->m_pTarget->Name());
strTarget.MakeLower();
if(-1!=str.Find(strTarget)){
bFound=true;
dwSizePath=MAX_PATH;
if(ERROR_SUCCESS==::RegQueryValueEx(hKey, _T("Default Build Tools Path"), NULL, NULL, (LPBYTE)psz, &dwSizePath)){
pItem->m_pTarget->SetBinDir(psz);
for(CConfigItem *pSib=pItem->FirstRadio();pSib;pSib=pSib->NextRadio()){
pSib->SetValue((ItemIntegerType) (pSib==pItem));
}
//                            pItem->EnableAncestors();
}
break;
}
}
}
}
RegCloseKey(hKey);
}

  if(!bFound){
  // No - choose the Hal according to toolchain
  for(int nItem=0;nItem<ItemCount();nItem++){
  CConfigItem *pItem=Item(nItem);
  if(pItem->m_pTarget){
  CString strTarget(pItem->m_pTarget->Name());
  strTarget.MakeLower();
  for(int j=0;j<m_arstrToolChainPaths.GetSize();j++){
  CString strTools(m_arstrToolChainPaths[j]);
  strTools.MakeLower();
  if(-1!=strTools.Find(strTarget)){
  for(CConfigItem *pSib=pItem->FirstRadio();pSib;pSib=pSib->NextRadio()){
  pSib->SetValue((ItemIntegerType) (pSib==pItem));
  }
  return;
  }
  }
  }
  }
  }
  }
*/

bool CConfigToolDoc::SetValue (CConfigItem &ti, double dValue, CdlTransaction transaction/*=NULL*/)
{
  ASSERT (ti.Type () == CConfigItem::Double);
  
  // test if the new double value is in range
  const CdlValuable valuable = ti.GetCdlValuable();
  CdlListValue list_value;
  CdlEvalContext context (NULL, ti.GetCdlItem (), ti.GetCdlItem ()->get_property (CdlPropertyId_LegalValues));
  valuable->get_legal_values ()->eval (context, list_value);
  if (! list_value.is_member (dValue))
  {
    if (dValue==valuable->get_double_value(CdlValueSource_Current) || IDNO == CUtils::MessageBoxFT (MB_YESNO, _T("%s is not a legal value for %s.\n\nDo you want to use this value anyway?"),
      CUtils::DoubleToStr (dValue), ti.Macro ()))
      return false;
  }
  
  if (! ti.SetValue (dValue,transaction))
    return false;
  
  SetModifiedFlag ();
  return true;
}
bool CConfigToolDoc::SetValue(CConfigItem &ti,const CString &strValue, CdlTransaction transaction/*=NULL*/)
{
  // warn the user if a modified memory layout is about to be discarded
  if (MemoryMap.map_modified () && (ti.Macro () == _T("CYG_HAL_STARTUP")) &&
    (IDCANCEL == CUtils::MessageBoxFT (MB_OKCANCEL, _T("Changes to the current memory layout will be lost."))))
    return false;
  
  bool rc=false;
  switch(ti.Type()){
		case CConfigItem::None:
      break;
    case CConfigItem::Enum:
    case CConfigItem::String:
      rc=ti.SetValue(strValue,transaction);
      break;
    case CConfigItem::Integer:
      {
        ItemIntegerType n;
        rc=CUtils::StrToItemIntegerType(strValue,n) && SetValue(ti,n,transaction);
      }
      break;
    case CConfigItem::Double:
      {
        double dValue;
        rc = CUtils::StrToDouble (strValue, dValue) && SetValue (ti, dValue,transaction);
      }
      break;
    default:
      ASSERT(FALSE);
      break;
      
  }
  if(rc){
    SetModifiedFlag();
    UpdateAllViews (NULL, CConfigToolDoc::ValueChanged, (CObject *)&ti);
  }
  return rc;
}

// Look in registry to match prefixes CYGPKG_HAL_ and CYGHWR_HAL_:
//
bool CConfigToolDoc::GetRunPlatform(CString &strTarget)
{
  CRegKeyEx k1(HKEY_CURRENT_USER,_T("Software\\Red Hat\\eCos\\Platforms"), KEY_READ);
  CString strKey;
  for(int i=0;k1.QueryKey(i,strKey);i++){
    CRegKeyEx k2((HKEY)k1,strKey,KEY_READ);
    // Subkeys' names are the target image names [one of which we aim to return]
    // Subkeys's values of interest are:
    //      Prefix  String
    //      Sim     DWORD  [optional]
    //      Macro   String 
    DWORD dwSim=0;
    CString strPrefix,strMacro;
    k2.QueryValue(_T("Sim"),dwSim);
    k2.QueryValue(_T("Prefix"),strPrefix);
    k2.QueryValue(_T("Macro"),strMacro);

    CConfigItem *pItem=Find(_T("CYGPKG_HAL_")+strMacro);
    if(pItem && pItem->IsEnabled()){
      // We have found what we are looking for - probably
      pItem=Find(_T("CYGHWR_HAL_")+strMacro+_T("_STARTUP"));
      if(pItem){
        // Two platforms apply - choose this one only if the ram startup attribute fits
        if( (dwSim==0) == (0==pItem->StringValue().CompareNoCase(_T("ram"))) ) {
          strTarget=strKey;
        }
      } else {
        // Only one platform applies
        strTarget=strKey;
      }
      
    }
  }
  return !strTarget.IsEmpty();
}

bool CConfigToolDoc::SetEnabled(CConfigItem &ti, bool bEnabled, CdlTransaction transaction/*=NULL*/)
{
  const bool bStatus = ti.SetEnabled (bEnabled,transaction);
  
  if (bStatus) {
    SetModifiedFlag();
    UpdateAllViews (NULL, CConfigToolDoc::ValueChanged, (CObject *) &ti);
  }
  return bStatus;
}

void CConfigToolDoc::AddContents (const CdlContainer container, CConfigItem *pParent)
{
  
  // determine the container contents
  
  const std::vector<CdlNode>& contents = container->get_contents ();
  std::vector<CdlNode>::const_iterator node_i;
  for (node_i = contents.begin (); node_i != contents.end (); node_i++)
  {
    const CdlNode node = * node_i;
    const CdlPackage pkg = dynamic_cast<CdlPackage> (node);
    const CdlComponent comp = dynamic_cast<CdlComponent> (node);
    const CdlOption opt = dynamic_cast<CdlOption> (node);
    const CdlContainer contnr = dynamic_cast<CdlContainer> (node);
    
    // if the node in the container is a package, component or option
    // then it is visible and should be added to the tree
    if  (0 != pkg) // the node is a package
    {
      CConfigItem * pItem = AddItem (pkg, pParent); // add the package
      AddContents (pkg, pItem); // add the package contents
    }
    else if (0 != comp) // the node is a component
    {
      CConfigItem * pItem = AddItem (comp, pParent); // add the component
      AddContents (comp, pItem); // add the component contents
    }
    else if (0 != opt) // the node is an option
      AddItem (opt, pParent); // add the option
    
    else if (0 != contnr) // if the node is a container
      AddContents (contnr, pParent); // add the container contents
    
    // ignore nodes of any other class
  }
}

// Three hack functions.

const CString CConfigToolDoc::CurrentPlatform()
{
/*
for(int nItem=0;nItem<ItemCount();nItem++){
CConfigItem *pItem=Item(nItem);
if(pItem->IsEnabled() && !pItem->Platform().IsEmpty()){
return pItem->Platform();
}
}
  */
  return _T("");
}

// a trivial CDL parse error handler
void CConfigToolDoc::CdlParseErrorHandler (std::string message)
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  pDoc->m_strCdlErrorMessage = message.c_str ();
  TRACE (_T("CdlParseError: %s\n"), pDoc->m_strCdlErrorMessage);
  CConfigTool::Log (CUtils::StripExtraWhitespace (pDoc->m_strCdlErrorMessage)); // display the message in the output window
};

// a trivial CDL parse warning handler
void CConfigToolDoc::CdlParseWarningHandler (std::string message)
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  pDoc->m_strCdlErrorMessage = message.c_str ();
  TRACE (_T("CdlParseWarning: %s\n"), pDoc->m_strCdlErrorMessage);
  CConfigTool::Log (CUtils::StripExtraWhitespace (pDoc->m_strCdlErrorMessage)); // display the message in the output window
};

// a trivial CDL load error handler
void CConfigToolDoc::CdlLoadErrorHandler (std::string message)
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  pDoc->m_strCdlErrorMessage = message.c_str ();
  TRACE (_T("CdlLoadError: %s\n"), pDoc->m_strCdlErrorMessage);
  CConfigTool::Log (CUtils::StripExtraWhitespace (pDoc->m_strCdlErrorMessage)); // display the message in the output window
};

// a trivial CDL load warning handler
void CConfigToolDoc::CdlLoadWarningHandler (std::string message)
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  pDoc->m_strCdlErrorMessage = message.c_str ();
  TRACE (_T("CdlLoadWarning: %s\n"), pDoc->m_strCdlErrorMessage);
  CConfigTool::Log (CUtils::StripExtraWhitespace (pDoc->m_strCdlErrorMessage)); // display the message in the output window
};

CdlInferenceCallbackResult CConfigToolDoc::CdlGlobalInferenceHandler(CdlTransaction transaction)
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  pDoc->m_ConflictsOutcome=NotDone;  // prepare for the case that there are no solutions
  CdlInferenceCallbackResult rc=CdlInferenceCallbackResult_Continue;
  /*
  std::list<CdlConflict>conflicts1(transaction->get_global_conflicts_with_solutions());
  const std::vector<CdlConflict>&conflicts2=transaction->get_resolved_conflicts();
	for (unsigned int i = 0; i < conflicts2.size (); i++) {
		conflicts1.push_back(conflicts2 [i]);
	}
  if(conflicts1.size()>0){
    CFailingRulesDialog dlg(conflicts1,transaction,&pDoc->m_arConflictsOfInterest);
    rc=(IDOK==dlg.DoModal())?CdlInferenceCallbackResult_Continue:CdlInferenceCallbackResult_Cancel;
    pDoc->m_ConflictsOutcome=(CdlInferenceCallbackResult_Continue==rc)?OK:Cancel;
  }
  */
  const std::list<CdlConflict>& conflicts=pDoc->GetCdlConfig()->get_all_conflicts();  
  CFailingRulesDialog dlg(conflicts,transaction,&pDoc->m_arConflictsOfInterest);
  rc=(IDOK==dlg.DoModal())?CdlInferenceCallbackResult_Continue:CdlInferenceCallbackResult_Cancel;
  pDoc->m_ConflictsOutcome=(CdlInferenceCallbackResult_Continue==rc)?OK:Cancel;

  return rc;
}

CdlInferenceCallbackResult CConfigToolDoc::CdlInferenceHandler (CdlTransaction transaction)
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  CdlInferenceCallbackResult rc=CdlInferenceCallbackResult_Continue;
  const std::list<CdlConflict>&conflicts=transaction->get_new_conflicts();
  if((pDoc->m_nRuleChecking&Immediate) && conflicts.size()>0){
    if(pDoc->m_nRuleChecking&SuggestFixes){
      std::list<CdlConflict> s_conflicts;
      for (std::list<CdlConflict>::const_iterator conf_i= conflicts.begin (); conf_i != conflicts.end (); conf_i++) { // for each conflict
        if((*conf_i)->has_known_solution()){
          s_conflicts.push_back(*conf_i);
        }
      }
      if(s_conflicts.size()>0){
        CFailingRulesDialog dlg(s_conflicts,transaction);
        return(IDOK==dlg.DoModal()?CdlInferenceCallbackResult_Continue:CdlInferenceCallbackResult_Cancel);
      }
    } 
    CString strMsg;
    if(1==conflicts.size()){
      strMsg=(_T("There is 1 unresolved conflict."));
    } else {
      strMsg.Format(_T("There are %d unresolved conflict%s."),conflicts.size(),1==conflicts.size()?_T(""):_T("s"));
    }
    rc=(IDYES==CUtils::MessageBoxFT(MB_YESNO|MB_DEFBUTTON2,_T("%s  Make the change anyway?"),strMsg))?CdlInferenceCallbackResult_Continue:CdlInferenceCallbackResult_Cancel;
  }
  return rc;
}


// a CDL transaction handler to refresh the configuration tree
void CConfigToolDoc::CdlTransactionHandler (const CdlTransactionCallback & data)
{
static int nNesting=0;
TRACE(_T("Transaction handler: nesting level=%d\n"),nNesting++);
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  std::vector<CdlValuable>::const_iterator val_i;
  std::vector<CdlNode>::const_iterator node_i;
  std::list<CdlConflict>::const_iterator conf_i;
  CControlView *pControlView=CConfigTool::GetControlView();
  for (val_i = data.value_changes.begin(); val_i != data.value_changes.end(); val_i++)
  {
    const CString strName((*val_i)->get_name().c_str());
    TRACE(_T("%s %s : value change\n"), CString ((*val_i)->get_class_name().c_str()), strName);
    pControlView->Refresh(strName);
    if (strName==_T("CYGHWR_MEMORY_LAYOUT")){               // the memory layout has changed...
      pDoc->SwitchMemoryLayout (false); // ...so display a new one
    }
  }
  for (node_i = data.active_changes.begin(); node_i != data.active_changes.end(); node_i++)
  {
    const CString strName((*node_i)->get_name().c_str());
    TRACE(_T("%s %s : this has become active or inactive\n"), CString ((*node_i)->get_class_name().c_str()),
      CString ((*node_i)->get_name().c_str()));
    if (! dynamic_cast<CdlInterface> (*node_i)){ // if not an interface node
      pControlView->Refresh(strName);
    }
  }
  for (val_i = data.legal_values_changes.begin(); val_i != data.legal_values_changes.end(); val_i++)
  {
    const CString strName((*node_i)->get_class_name().c_str());
    TRACE(_T("%s %s : the legal_values list has changed, a new widget may be needed.\n"),
      CString ((*val_i)->get_class_name().c_str()), strName);               
  }
  
  for (val_i = data.value_source_changes.begin(); val_i != data.value_source_changes.end(); val_i++)
  {
    const CString strName((*val_i)->get_name().c_str());
    CdlValueSource source = (*val_i)->get_source();
    TRACE(_T("%s %s : the value source has changed to %s\n"),
      CString ((*val_i)->get_class_name().c_str()), strName,
      CString ((CdlValueSource_Default  == source) ? "default"  :
    (CdlValueSource_Inferred == source) ? "inferred" :
    (CdlValueSource_Wizard   == source) ? "wizard"   : "user"));
    pControlView->Refresh (strName);
  }

  pDoc->UpdateFailingRuleCount();
nNesting--;
}

// Three hack functions.

const CString CConfigToolDoc::CurrentMemoryLayout ()
{
  const CConfigItem * pItem = Find (_T("CYGHWR_MEMORY_LAYOUT"));
#ifdef _DEBUG
  if(NULL==pItem){
    TRACE(_T("Warning - CurrentMemoryLayout() returning NULL\n"));
  }
#endif
  return pItem ? pItem->StringValue () : _T("");
}

const CString CConfigToolDoc::CurrentTestingIdentifier ()
{
  // use the CDL target as the default testing identifier
  // override in the forthcoming testing identifier dialog as necessary
  return GetCdlConfig()->get_hardware ().c_str ();

//  const CConfigItem * pItem = Find (_T("CYGTST_TESTING_IDENTIFIER"));
//#ifdef _DEBUG
//  if(NULL==pItem){
//    TRACE(_T("Warning - CurrentTestingIdentifier() returning NULL\n"));
//  }
//#endif
//  return pItem ? pItem->StringValue () : _T("");
}

const CString CConfigToolDoc::CurrentStartup()
{
  const CConfigItem * pItem = Find (_T("CYG_HAL_STARTUP"));
#ifdef _DEBUG
  if(NULL==pItem){
    TRACE(_T("Warning - CurrentStartup() returning NULL\n"));
  }
#endif
  return pItem ? pItem->StringValue () : _T("");
  
  /*
  for(int nItem=0;nItem<ItemCount();nItem++){
		CConfigItem *pItem=Item(nItem);
    const CString strName(pItem->Name());
    if(0==strName.Compare(_T("Startup type")) && pItem->IsEnabled()){
    return pItem->StringValue();
    }
    }
    return _T("ram"); // FIXME - assume ram startup for now if no startup item
  */
}

void CConfigToolDoc::UpdateFailingRuleCount()
{
  int nCount=0;
  if (GetCdlConfig ()){
    // if configuration information
    
    // calculate the number of conflicts
    nCount =
      //	        GetCdlConfig ()->get_structural_conflicts ().size () +    ignore for now
      GetCdlConfig ()->get_all_conflicts ().size ();
    
    // update the conflicts view
    if (CConfigTool::GetRulesView ()) {
      CConfigTool::GetRulesView ()->FillRules ();
    }
  }
  if(CConfigTool::GetMain()){
    CConfigTool::GetMain()->SetFailRulePane(nCount);
  }
}

void CConfigToolDoc::LogConflicts (const std::list<CdlConflict> & conflicts)
{
  std::list<CdlConflict>::const_iterator conf_i;
  for (conf_i = conflicts.begin (); conf_i != conflicts.end (); conf_i++) // for each conflict
  {
    CString strExplain = (* conf_i)->get_explanation ().c_str (); // get the conflict explanation
    CConfigTool::Log (CUtils::StripExtraWhitespace (strExplain)); // display the message in the output window
  }
}

CString CConfigToolDoc::GetPackageName (const CString & strAlias)
{
  const std::vector<std::string> & packages = m_CdlPkgData->get_packages ();
  std::vector<std::string>::const_iterator package_i;
  for (package_i = packages.begin (); package_i != packages.end (); package_i++)
  {
    const std::vector<std::string> & aliases = m_CdlPkgData->get_package_aliases (* package_i);
    CString strPackageAlias = aliases [0].c_str ();
    if (aliases.size () && (0 == strAlias.Compare (strPackageAlias)))
      return package_i->c_str ();
  }
  return _T("");
}

void CConfigToolDoc::DeleteContents() 
{
  TRACE(_T("###DeleteContents()\n"));
#ifndef PLUGIN
  CDocument::DeleteContents();
#endif
  
  RemoveAllItems();
  TRACE(_T("###DeleteContents done\n"));
}

void CConfigToolDoc::SelectPackages ()
{
  CCdlPackagesDialog dlg;
  // This map holds the CConfigItem pointers for the packages loaded before the dialog is invoked.
  // We cannot use Find(), which traverses all items - potentially those that have been removed
  CMapStringToPtr arLoadedPackages;

  // generate the contents of the add/remove list boxes
  const std::vector<std::string> & packages = m_CdlPkgData->get_packages ();
  std::vector<std::string>::const_iterator package_i;
  for (package_i = packages.begin (); package_i != packages.end (); package_i++)
  {
    //		if (! m_CdlPkgData->is_hardware_package (* package_i)) // do not list hardware packages
    {
      const std::vector<std::string> & aliases = m_CdlPkgData->get_package_aliases (* package_i);
      CString strMacroName = package_i->c_str ();
      
      // use the first alias (if any) as the package identifier
      CString strPackageName = aliases.size () ? aliases [0].c_str () : strMacroName;
      CConfigItem * pItem = Find (strMacroName);
      if (pItem) // if the package is loaded
      {
        arLoadedPackages.SetAt(strMacroName,pItem);
        // pass the currently selected package version string to the dialog box
        const CdlValuable valuable = pItem->GetCdlValuable();
        dlg.Insert (strPackageName, TRUE, NULL, valuable ? (LPCTSTR) CString (valuable->get_value ().c_str ()) : NULL);
      }
      else
      {
        // pass version string of the most latest version to the dialog box
        dlg.Insert (strPackageName, FALSE, NULL, (LPCTSTR) CString (m_CdlPkgData->get_package_versions (* package_i) [0].c_str ()));
      }
    }
  }
  
  if (IDOK == dlg.DoModal ())
  {
    bool bChanged = false; // until proved otherwise

    // determine whether each package has changed loaded/unloaded state
    for (package_i = packages.begin (); package_i != packages.end (); package_i++)
      //			if (! m_CdlPkgData->is_hardware_package (* package_i)) // do not check hardware packages
    {
      const std::vector<std::string> & aliases = m_CdlPkgData->get_package_aliases (* package_i);
      CString strMacroName = package_i->c_str ();
      
      // use the first alias (if any) as the package identifier
      CString strPackageName = aliases.size () ? aliases [0].c_str () : strMacroName;
      
      CConfigItem *pItem=NULL;
      bool bPreviouslyLoaded=arLoadedPackages.Lookup(strMacroName,(void *&)pItem);
      bool bNowLoaded=dlg.IsAdded (strPackageName);
      
      // unload packages which are no longer required before
      // loading new ones to avoid potential duplicate macro definitions
      if (! bNowLoaded && bPreviouslyLoaded){
        // The package was loaded but should now be unloaded:
        bChanged|=pItem->Unload();
      } else if (bNowLoaded) {// if the package should be loaded
        const CString strVersion(dlg.GetVersion (strPackageName));
        if (bPreviouslyLoaded) { // if the package is already loaded
          CdlTransactionCallback::set_callback_fn (NULL); // avoid value refresh attempts during load/unload
          bChanged|=pItem->ChangeVersion(strVersion);
          CdlTransactionCallback::set_callback_fn (CdlTransactionHandler); // restore value refresh
        } else {
          // the package was not loaded but should now be loaded
          TRACE (_T("Loading package %s\n"), strMacroName);
          try
          {
            GetCdlConfig()->load_package (CUtils::UnicodeToStdStr(strMacroName), CUtils::UnicodeToStdStr (strVersion), CConfigToolDoc::CdlParseErrorHandler, CConfigToolDoc::CdlParseWarningHandler);
            bChanged=true;
          }
          catch (CdlStringException exception)
          {
            CUtils::MessageBoxF(_T("Error loading package %s:\n\n%s"), strMacroName, CString (exception.get_message ().c_str ()));
          }
          catch (...)
          {
            CUtils::MessageBoxF(_T("Error loading package %s."), strMacroName);
          }
        }
      }				
    }
    
    if (bChanged) {// at least one package was loaded, unloaded or changed version
      SetModifiedFlag();
      RegenerateData();
    }
  }
}

bool CConfigToolDoc::SetValue(CConfigItem &ti,ItemIntegerType nValue, CdlTransaction transaction/*=NULL*/)
{
  switch(ti.Type()){
		case CConfigItem::Enum:
    case CConfigItem::Integer:
      //		case CConfigItem::Boolean:
      //		case CConfigItem::Radio:
      break;
    case CConfigItem::None:
    case CConfigItem::String:
    default:
      ASSERT(FALSE);
      break;
  }
  
  bool rc=false;
  bool bChangingMemmap=MemoryMap.map_modified () && ((ti.Macro ().Compare (_T ("CYG_HAL_STARTUP")) == 0));
  
  if(nValue==ti.Value()){
    return true;
  }
  
  // test if the new integer value is in range
  if (CConfigItem::Integer == ti.Type ())
 	{
    const CdlValuable valuable = ti.GetCdlValuable();
    CdlListValue list_value;
    CdlEvalContext context (NULL, ti.GetCdlItem (), ti.GetCdlItem ()->get_property (CdlPropertyId_LegalValues));
    valuable->get_legal_values ()->eval (context, list_value);
    if (! list_value.is_member ((cdl_int) nValue))
    {
      if (nValue==(ItemIntegerType) valuable->get_integer_value (CdlValueSource_Current) || IDNO == CUtils::MessageBoxFT (MB_YESNO, _T("%s is not a legal value for %s.\n\nDo you want to use this value anyway?"),
        CUtils::IntToStr (nValue, CConfigTool::GetConfigToolDoc ()->m_bHex), ti.Macro ()))
        goto Exit;
    };
 	}
  
  //    if(!ti.CanSetValue(nValue))
  //	{
  //        if(bInteractive){
  //			CString strExplanation;
  //			if(CConfigItem::Integer==ti.Type()){
  //				strExplanation.Format(_T(" [value must lie in the range %d..%d]"),
  //					CUtils::IntToStr(ti.Min(),m_bHex), 
  //                    CUtils::IntToStr(ti.Max()),m_bHex);
  //			}
  //			CUtils::MessageBoxF(
  //				_T("Cannot set '%s' to %s%s"),
  //				ti.ItemNameOrMacro(), CUtils::IntToStr(nValue,m_bHex), strExplanation);
  //        }
  //		goto Exit;
  //	}
  
  // warn the user if the current memory layout has been changed and will be lost
  // this will happen when the layout has been modified and the target-platform-startup is changed
  
  if (bChangingMemmap && IDCANCEL==CUtils::MessageBoxFT(MB_OKCANCEL,_T("Changes to the current memory layout will be lost."))){
    goto Exit;
  }
  
  // Save state
  if(!ti.SetValue(nValue,transaction)){
    // CanSetValue above should have caught this
    CUtils::MessageBoxF(_T("Cannot set '%s' to %d"),ti.ItemNameOrMacro(), nValue);
    goto Exit;
  } 
  
  rc=true;
Exit:
  if(rc){
    SetModifiedFlag();
    UpdateFailingRuleCount ();
    UpdateAllViews (NULL, CConfigToolDoc::ValueChanged, (CObject *)&ti);
    // switch to new memory layout when target, platform or startup changes
    // but ignore cases where a target or platform is being deactivated (PR 19363)
    
    //		if (((CurrentTarget () != pOldTarget) || (CurrentPlatform () != strOldPlatform) || (ti.Name ().Compare (_T ("Startup type")) == 0)) &&
    //			((ti.Type () != CConfigItem::Radio) || (nValue)))
    //			SwitchMemoryLayout ((CurrentTarget () != pOldTarget) || (CurrentPlatform () != strOldPlatform));
  }
  return rc;
}

bool CConfigToolDoc::OpenRepository(LPCTSTR pszRepository/*=NULL*/,bool bPromptInitially/*=false*/)
{
  if(!m_bRepositoryOpen){
    CWaitCursor wait;
    CIdleMessage IM(_T("Opening repository"));
    UpdateFailingRuleCount();
    
    CFileName strNewRepository;
    while(!m_bRepositoryOpen){
      
      if(bPromptInitially){
        CFolderDialog dlg(/*BOOL bAllowCreation=*/false);
        dlg.m_strDesc=_T("Please specify the root of the eCos repository tree.");
        dlg.m_strTitle=_T("Choose folder for eCos repository");
        dlg.m_strFolder=strNewRepository;
        CConfigTool::DismissSplash();
        if(IDCANCEL==dlg.DoModal()){
          return false;
        }
        strNewRepository=dlg.m_strFolder;
      } else {
        // Use what came in as parameter or what was found in registry
        strNewRepository=pszRepository?pszRepository:m_strRepository;
        bPromptInitially=true;
      }
      
      IM.Set (_T("Opening repository ") + (CString) strNewRepository);
      
      CdlPackagesDatabase NewCdlPkgData = NULL;
      CdlInterpreter      NewCdlInterp  = NULL;
      CdlConfiguration    NewCdlConfig  = NULL;
      CFileName strNewPackagesDir;
      
      EnableCallbacks(false); // disable transaction callbacks until the config tree is regenerated
      
      if(OpenRepository(strNewRepository,NewCdlPkgData,NewCdlInterp,NewCdlConfig,strNewPackagesDir)){
        // Success
        
        // select the "default" template if it exists
        // otherwise select the first available template
        std::string default_template = "default";
        if (! NewCdlPkgData->is_known_template (default_template))
        {
          const std::vector<std::string>& templates = NewCdlPkgData->get_templates ();
          if (templates.size () != 0)
            default_template = templates [0];
        }
        
        m_template_version = "";
        try
        {
          const std::vector<std::string>& template_versions = NewCdlPkgData->get_template_versions (default_template);
          NewCdlConfig->set_template (default_template, template_versions [0], &CdlParseErrorHandler, &CdlParseWarningHandler);
          m_template_version = template_versions [0];
        }
        catch (CdlStringException exception) {
          CUtils::MessageBoxF(_T("Error loading package template '%s':\n\n%s"), CString (default_template.c_str ()), CString (exception.get_message ().c_str ()));
        }
        catch (...) {
          CUtils::MessageBoxF(_T("Error loading package template '%s'."), CString (default_template.c_str ()));
        }
        
        // check the configuration
        ASSERT (NewCdlConfig->check_this (cyg_extreme));
        
        // use the new package database, interpreter and configuration
        deleteZ(m_CdlConfig); // delete the previous configuration
        deleteZ(m_CdlInterp); // delete the previous interpreter
        deleteZ(m_CdlPkgData); // delete the previous package database
        
        m_CdlPkgData = NewCdlPkgData;
        m_CdlInterp  = NewCdlInterp;
        m_CdlConfig  = NewCdlConfig;
        
        // save the repository location
        
        SetRepository(strNewRepository);
        m_strPackagesDir = strNewPackagesDir;
        
#ifndef PLUGIN
        // clear the previously specified document file name (if any),
        // OnNewDocument() calls DeleteContents() so must be called
        // before AddAllItems()
        CDocument::OnNewDocument ();
#endif
        
        // generate the CConfigItems from their CDL descriptions
        AddAllItems ();
        
        m_bRepositoryOpen=true;
      } else {
        // failure
        deleteZ(NewCdlConfig);
        deleteZ(NewCdlInterp);
        deleteZ(NewCdlPkgData);
        
      }
      
      // install a transaction handler callback function now that the tree exists
      EnableCallbacks(true);
    }
    
    }
    return m_bRepositoryOpen;
}

// Helper fn for namesake above
bool CConfigToolDoc::OpenRepository (const CFileName strNewRepository,CdlPackagesDatabase &NewCdlPkgData,CdlInterpreter &NewCdlInterp,CdlConfiguration &NewCdlConfig,CFileName &strNewPackagesDir)
{
  bool rc=false;
  TRACE(_T("###Open repository %s\n"),strNewRepository);
  if(!strNewRepository.IsEmpty()){
    // Now strNewRepository is guaranteed non-empty, but does it exist?
    if(!strNewRepository.IsDir()) {
      CUtils::MessageBoxF(_T("Cannot open repository - the folder %s does not exist"), strNewRepository);
    } else {
    
      // Ok so it exists, but does it look right?
      strNewPackagesDir=strNewRepository+_T("ecc");
      if(!strNewPackagesDir.IsDir()){
        strNewPackagesDir=strNewRepository+_T("packages");
      }
    
      if(!strNewPackagesDir.IsDir()){
        // Don't mention the ecc\ attempt
        CUtils::MessageBoxF(_T("%s does not seem to be a source repository: the folder %s does not exist"),
          strNewRepository,strNewPackagesDir);
      } else {
      
        const CFileName strDatabase = strNewPackagesDir + _T("ecos.db");
        if(!strDatabase.Exists()) {
          CUtils::MessageBoxF(_T("%s does not seem to be a source repository: %s does not exist"), strNewRepository, strDatabase);
        } else {
        
          // create a CDL repository, interpreter and configuration
        
          try {// create a new package database, interpreter and configuration
            NewCdlPkgData = CdlPackagesDatabaseBody::make (CUtils::UnicodeToStdStr(strNewPackagesDir), &CdlParseErrorHandler, &CdlParseWarningHandler);
            NewCdlInterp = CdlInterpreterBody::make ();
            NewCdlConfig = CdlConfigurationBody::make ("eCos", NewCdlPkgData, NewCdlInterp);
          }
          catch (CdlStringException exception) {
            CUtils::MessageBoxF(_T("Error opening eCos repository:\n\n%s"), CString (exception.get_message ().c_str ()));
            return false;
          }
          catch (...) {
            CUtils::MessageBoxF(_T("Error opening eCos repository."));
            return false;
          }
        
          // select the default target if specified in the registry
          // otherwise select the first available target
          std::string default_hardware = CUtils::UnicodeToStdStr (GetDefaultHardware ());
          if (! NewCdlPkgData->is_known_target (default_hardware)) {
            const std::vector<std::string>& targets = NewCdlPkgData->get_targets ();
            if (targets.size () == 0){
              CUtils::MessageBoxF (_T("Error opening eCos repository:\n\nno hardware templates"));
              return false;
            } else {
              default_hardware = targets [0];
            }
          }
        
          try {
            m_strCdlErrorMessage = _T("");
            NewCdlConfig->set_hardware (default_hardware, &CdlParseErrorHandler, &CdlParseWarningHandler);
          }
          catch (CdlStringException exception) {
            if (m_strCdlErrorMessage.IsEmpty ())
            {
              CUtils::MessageBoxF (_T("Error loading the default hardware template '%s':\n\n%s"), CString (default_hardware.c_str ()), CString (exception.get_message ().c_str ()));
            }
            else // report the most recent parser message in the message box since there may be no output pane in which to view it
            {
              CUtils::MessageBoxF (_T("Error loading the default hardware template '%s':\n\n%s\n\nParser message:\n\n%s"), CString (default_hardware.c_str ()), CString (exception.get_message ().c_str ()), m_strCdlErrorMessage);
            }
            return false;
          }
          catch (...) {
            CUtils::MessageBoxF (_T("Error loading the default hardware template '%s':\n\n%s"), CString (default_hardware.c_str ()), m_strCdlErrorMessage);
            return false;
          }
          rc=true;
        }
      }
    }
  }
  return rc;
}    

void CConfigToolDoc::CloseRepository()
{
  if(m_bRepositoryOpen){
    // delete the libCDL objects with the document
    EnableCallbacks(false); // first disable the transaction handler
    deleteZ(m_CdlConfig);
    deleteZ(m_CdlInterp);
    deleteZ(m_CdlPkgData);
    m_bRepositoryOpen=false;
  }
}

void CConfigToolDoc::SelectTemplate (std::string NewTemplate, std::string NewTemplateVersion)
{
  if ((NewTemplate != m_CdlConfig->get_template()) || (NewTemplateVersion != m_template_version)){

    CWaitCursor wait; // this may take a little while
    RemoveAllItems();
    m_template_version = "";
    try
    {
      m_CdlConfig->set_template (NewTemplate, NewTemplateVersion, CdlParseErrorHandler, CdlParseWarningHandler);
      m_template_version = NewTemplateVersion;
    }
    catch (CdlStringException exception)
    {
      CUtils::MessageBoxF(_T("Error loading package template '%s':\n\n%s"), CString (NewTemplate.c_str ()), CString (exception.get_message ().c_str ()));
    }
    catch (...)
    {
      CUtils::MessageBoxF(_T("Error loading package template '%s'."), CString (NewTemplate.c_str ()));
    }
    RegenerateData();  

    if (!GetPathName().IsEmpty()){ // not a new document
      CopyMLTFiles (); // copy new MLT files to the build tree as necessary
    }
    SetModifiedFlag();
  }
}

void CConfigToolDoc::RegenerateData()
{
  CWaitCursor wait; // This may take a little while
  AddAllItems (); // regenerate all the config items since the topology may have changed
  if (m_strLinkerScriptFolder.IsEmpty ())
  {
    CUtils::MessageBoxF(_T("The eCos linker script macro CYGBLD_LINKER_SCRIPT is not defined."));
  }
  if (m_strMemoryLayoutFolder.IsEmpty ())
  {
    CUtils::MessageBoxF(_T("The eCos memory layout macro CYGHWR_MEMORY_LAYOUT is not defined."));
  }
  SwitchMemoryLayout (true); // the hardware template may have changed
  UpdateBuildInfo();
  CConfigTool::GetControlView()->SelectItem(Item(0));
}

void CConfigToolDoc::SelectHardware (std::string NewTemplate)
{
  const std::string OldTemplate=m_CdlConfig->get_hardware();
  if (NewTemplate != OldTemplate){
    RemoveAllItems();

    try
    {
      m_CdlConfig->set_hardware (NewTemplate, CdlParseErrorHandler, CdlParseWarningHandler);
    }
    catch (CdlStringException exception)
    {
      CUtils::MessageBoxF(_T("Error loading hardware template '%s':\n\n%s"), CString (NewTemplate.c_str ()), CString (exception.get_message ().c_str ()));
      m_CdlConfig->set_hardware (OldTemplate, CdlParseErrorHandler, CdlParseWarningHandler);
    }
    catch (...)
    {
      CUtils::MessageBoxF(_T("Error loading hardware template '%s'."), CString (NewTemplate.c_str ()));
      m_CdlConfig->set_hardware (OldTemplate, CdlParseErrorHandler, CdlParseWarningHandler);
    }
  
    RegenerateData();
    //EnableCallbacks(true); // re-enable the transaction handler
    if (!GetPathName().IsEmpty()){
      CopyMLTFiles (); // copy new MLT files to the build tree as necessary
    }
    SetModifiedFlag();
  }
}

bool CConfigToolDoc::SaveMemoryMap()
{
  const CString strSuffix(_T("mlt_") + CurrentMemoryLayout ());
  const CFileName strMLTInstallPkgconfDir(InstallTree() + _T("include\\pkgconf"));
  bool rc=false;
  if(strMLTInstallPkgconfDir.CreateDirectory(true)){
    const CString strMLTInstallBase(strMLTInstallPkgconfDir+CFileName(strSuffix));
    const CFileName strMLTDir (MLTDir());
    if(strMLTDir.CreateDirectory (true)){
      const CString strMLTBase (strMLTDir + CFileName (strSuffix));
      TRACE(_T("Saving memory layout to %s\n"), strMLTBase + _T(".mlt"));
      if(MemoryMap.save_memory_layout (strMLTBase + _T(".mlt"))){
        TRACE(_T("Exporting memory layout to %s\n"), strMLTInstallPkgconfDir);
        rc=MemoryMap.export_files (strMLTInstallBase + _T(".ldi"), strMLTInstallBase + _T(".h"));
      }
    }
  }    
  return rc;
}

bool CConfigToolDoc::CopyMLTFiles()
{
  // copy default MLT files for the selected target/platform from the repository if they do not already exist

  TRACE (_T("Looking for MLT files at %s\n"), PackagesDir() + m_strMemoryLayoutFolder + _T("include\\pkgconf\\mlt_*.*"));
  const CFileName strInstallDestination(InstallTree () + _T("include\\pkgconf"));
  const CFileName strMLTDestination (MLTDir ());
  TRACE (_T("Copying .ldi and .h files to %s\n"), strInstallDestination);
  TRACE (_T("Copying .mlt files to %s\n"), strMLTDestination);
  bool rc=strInstallDestination.CreateDirectory (true) && strMLTDestination.CreateDirectory (true);
  if(rc){
    CFileFind ffFileFind;
    BOOL bLastFile = ffFileFind.FindFile (PackagesDir() + m_strMemoryLayoutFolder + _T("\\include\\pkgconf\\mlt_*.*"));
    while (bLastFile)
    {
      bLastFile = ffFileFind.FindNextFile ();
      if (_T(".mlt") == ffFileFind.GetFileName ().Right (4)) // if a .mlt file
      {
        if (! CFileName (strMLTDestination, CFileName (ffFileFind.GetFileName ())).Exists ())
        {
          if(!CUtils::CopyFile (ffFileFind.GetFilePath (), strMLTDestination + CFileName (ffFileFind.GetFileName ()))){
            return false; // message already emitted
          }
        }
      }
      else // a .h or .ldi file
      {
        if (! CFileName (strInstallDestination, CFileName (ffFileFind.GetFileName ())).Exists () && 
          !CUtils::CopyFile (ffFileFind.GetFilePath (), strInstallDestination + CFileName (ffFileFind.GetFileName ()))){
          return false; // message already emitted
        }
      }
    }
  }
  return rc; //FIXME
}

CString CConfigToolDoc::GetDefaultHardware ()
{
  CString strKey = _T("SOFTWARE\\Red Hat\\eCos");
  CString strVersionKey = _T("");
  CString rc = _T("");
  TCHAR pszBuffer [MAX_PATH + 1];
  HKEY hKey;
  LONG lStatus;
  
  // get the greatest eCos version subkey
  if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, strKey, 0L, KEY_READ, &hKey))
  {
    DWORD dwIndex = 0;
    while (ERROR_SUCCESS == RegEnumKey (hKey, dwIndex++, (LPTSTR) pszBuffer, sizeof (pszBuffer)))
    {
      if (strVersionKey.Compare (pszBuffer) < 0)
        strVersionKey = pszBuffer;
    }
    RegCloseKey (hKey);
    if (! strVersionKey.IsEmpty ())
    {
      TRACE (_T("CConfigToolDoc::GetDefaultHardware(): version subkey = '%s'\n"), strVersionKey);
      
      // get the default hardware value
      strKey +=  _T("\\") + strVersionKey;
      if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, strKey, 0L, KEY_READ, &hKey))
      {
        DWORD dwBufferSize = sizeof (pszBuffer);
        lStatus = RegQueryValueEx (hKey, _T("Default Hardware"), NULL, NULL, (LPBYTE) pszBuffer, &dwBufferSize);
        RegCloseKey (hKey);
        if (ERROR_SUCCESS == lStatus)
        {
          TRACE (_T("CConfigToolDoc::GetDefaultHardware(): default hardware = '%s'\n"), pszBuffer);
          rc=pszBuffer;
        }
      }
    }
  }
  return rc;
}

CConfigItem * CConfigToolDoc::AddItem (const CdlUserVisible vitem, CConfigItem * pParent)
{
  CConfigItem * pItem = new CConfigItem (pParent, vitem);

  m_arItem.Add(pItem);
  pItem->m_strDesc = CUtils::StripExtraWhitespace (CString (vitem->get_description ().c_str ()));

  if (vitem->get_name () == "CYGHWR_MEMORY_LAYOUT")
  {
    ASSERT (m_strMemoryLayoutFolder.IsEmpty ());
    m_strMemoryLayoutFolder = vitem->get_owner ()->get_directory().c_str ();
    m_strMemoryLayoutFolder.Replace(_TCHAR('/'),_TCHAR('\\'));
    TRACE (_T("Found memory layout folder: %s\n"), m_strMemoryLayoutFolder);
  }
  
  if (vitem->get_name () == "CYGBLD_LINKER_SCRIPT")
  {
    ASSERT (m_strLinkerScriptFolder.IsEmpty ());
    m_strLinkerScriptFolder = vitem->get_owner ()->get_directory().c_str ();
    m_strLinkerScriptFolder.Replace(_TCHAR('/'),_TCHAR('\\'));
    TRACE (_T("Found linker script folder: %s\n"), m_strLinkerScriptFolder);
    
    // the CDL hardware template name will eventually become the target name,
    // but for now we must deduce the target name from the linker script file name
    
    const CdlValuable valuable = dynamic_cast<CdlValuable> (vitem);
    CFileName strLinkerScript (m_strPackagesDir, m_strLinkerScriptFolder, CString (valuable->get_value ().c_str ()));
    strLinkerScript.Replace (_TCHAR('/'), _TCHAR('\\'));
    if(!strLinkerScript.Exists ()){
        CConfigTool::Log(_T("%s does not exist\n"),strLinkerScript);
    }
    TRACE (_T("Target '%s' selected\n"), strLinkerScript.Tail ().Root (), pItem->Macro());
    //CFileName strBinDir = AfxGetApp () -> GetProfileString (CUtils::LoadString (IDS_KEY_TOOLS_DIR), pTarget->Name (), _T(""));
    //if (! strBinDir.IsEmpty () && strBinDir.IsDir ())
    //{
    //  pTarget->SetBinDir (strBinDir);
    //}
  }	
  
  //TRACE(_T("Created new item from cdl: "));
  //pItem->DumpItem();
  return pItem;
}

void CConfigToolDoc::AddAllItems ()
{
  // remove any old items and packages
  DeleteContents ();
  if(NULL!=CConfigTool::GetControlView()){ // may not be the case for plugin

    // add the root item
    CConfigItem * pItem = new CConfigItem (NULL, NULL);
    m_arItem.Add(pItem);
    pItem->m_strDesc = _T("The root node for all configurable items");
  
    // add the remaining items using their CDL descriptions
    m_strMemoryLayoutFolder = _T("");
    m_strLinkerScriptFolder = _T("");
    AddContents (m_CdlConfig, FirstItem ());
  
    // check that exactly one radio button in each group is enabled
    CheckRadios ();
  
    // update the rules (conflicts) view
    UpdateFailingRuleCount ();
  
    if(NULL==CConfigTool::GetRulesView() || !CConfigTool::GetRulesView()->IsWindowVisible()){
      // log all conflicts
      //	LogConflicts (m_CdlConfig->get_structural_conflicts ()); // relating to package availability - ignore for now
      LogConflicts (m_CdlConfig->get_all_conflicts ());
    }

    CConfigTool::GetControlView()->SelectItem(Item(0));
    UpdateAllViews (NULL, SelChanged, (CObject *)Item(0)); // refresh the display
    if(ItemCount()>0){
      CConfigTool::GetControlView()->GetTreeCtrl().Expand(Item(0)->HItem(),TVE_EXPAND);
    }
    CConfigTool::GetControlView()->SetFocus();
  }
}

const CFileName CConfigToolDoc::CurrentLinkerScript()
{
  const CConfigItem * pItem = Find (_T("CYGBLD_LINKER_SCRIPT"));
  return pItem ? CFileName (m_strPackagesDir, m_strLinkerScriptFolder, pItem->StringValue ()) : _T("");
}

bool CConfigToolDoc::GenerateHeaders()
{
// Copy non-config header files from the repository to the
// install tree for plugin only. The make system used by
// the standalone config tool does this for us.
#ifdef PLUGIN
  typedef std::vector<CdlBuildInfo_Loadable> EntriesArray;
  const EntriesArray &arEntries=BuildInfo().entries;
  typedef std::vector<CdlBuildInfo_Header> HeaderArray;
  for(EntriesArray::size_type j=0;j<arEntries.size();j++){
    const CdlBuildInfo_Loadable &e=arEntries[j];
    const CFileName strDir(CUtils::WPath(e.directory));
    const HeaderArray &arHeaders=e.headers;
    for(HeaderArray::size_type i=0;i<arHeaders.size();i++){
      const CdlBuildInfo_Header &h=arHeaders[i];
      const CFileName strSource(PackagesDir()+strDir+CUtils::WPath(h.source));
      const CFileName strDest(HeadersDir()/*+CUtils::WPath(e.directory)*/+CUtils::WPath(h.destination));
    
      if(!strDest.Head().CreateDirectory(true)){
        CUtils::MessageBoxF(_T("Failed to create %s - %s"),strDest.Head(),CUtils::GetLastErrorMessageString());
        return false;
      }
      if(!CUtils::CopyFile(strSource,strDest)){ 
        return false; // message already emitted
      }
    }
  }
#endif

  // Generate headers
  try {
    CFileName strPkfConfDir(InstallTree()+_T("include\\pkgconf"));
    if(!strPkfConfDir.CreateDirectory()){
      CUtils::MessageBoxF(_T("Failed to create %s - %s"),strPkfConfDir,CUtils::GetLastErrorMessageString());
      return false;
    }
    GetCdlConfig()->generate_config_headers(CUtils::UnicodeToStdStr(strPkfConfDir.ShortName()));
  }
  catch (CdlInputOutputException e) {
    const CString strMsg(e.get_message().c_str());
    TRACE(_T("!!! Exception thrown calling generate_config_headers - %s"),strMsg);
    CUtils::MessageBoxF(_T("Failed to generate header files - %s"),strMsg);
    return false;
  }
  return true;
}

const CFileName CConfigToolDoc::MLTDir ()
{
	CString strPathName (GetPathName ());
	ASSERT (! strPathName.IsEmpty ());
	return strPathName.Left (strPathName.ReverseFind (_TCHAR('.'))) + _T("_mlt");
}

bool CConfigToolDoc::UpdateBuildInfo(bool bFirstTime)
{
  try {
    GetCdlConfig()->get_build_info(m_BuildInfo);
    #ifdef PLUGIN
    // FIXME: this means anything not mentioned by AddFile in our scan below will be removed - including user-added items
    m_peCosProject->ClearItemFlags();    
    m_peCosProject->UpdateeCosProject(bFirstTime);
    m_peCosProject->RemoveAllUnflaggedItems();
    #else
    UNUSED_ALWAYS(bFirstTime);
    generate_build_tree (GetCdlConfig(), CUtils::UnicodeToStdStr(BuildTree()), CUtils::UnicodeToStdStr(InstallTree()));
    #endif
    return true;
  }
  catch(...){
    return false;
  }
}

void CConfigToolDoc::RemoveAllItems()
{
  for(int nItem=0;nItem<ItemCount();nItem++){
    CConfigItem *pItem=Item(nItem);
    deleteZ(pItem);
  }
  m_arItem.RemoveAll();

  UpdateAllViews(NULL,Clear,0);
}

void CConfigToolDoc::EnableCallbacks (bool bEnable/*=true*/)
{
  CdlTransactionCallback::set_callback_fn(bEnable?&CdlTransactionHandler:0);
  CdlTransactionBody::set_inference_callback_fn(bEnable?&CdlInferenceHandler:0);
  CdlTransactionBody::set_inference_override(CdlValueSource_Invalid);
}

int CConfigToolDoc::GetTestExeNames (CFileNameArray &arTestExes,CFileNameArray &arMissing)
{
  arTestExes.RemoveAll();
  arMissing.RemoveAll();
  
  typedef std::vector<CdlBuildInfo_Loadable> EntriesArray;
  const EntriesArray &arEntries=BuildInfo().entries;
  for(EntriesArray::size_type j=0;j<arEntries.size();j++){
    const CdlBuildInfo_Loadable &e=arEntries[j];
    CStringArray ar;
    int n=CUtils::Chop(CString(get_tests(GetCdlConfig(),e).c_str()),ar);
    for(int i=0;i<n;i++){
      CFileName strFile;
      strFile.Format(_T("%s\\tests\\%s\\%s.exe"),InstallTree(),CString(e.directory.c_str()),ar[i]); 
      strFile.Replace(_TCHAR('/'),_TCHAR('\\'));
      if(strFile.Exists()){
        arTestExes.Add(strFile);
      } else {
        arMissing.Add(strFile);
      }
    }
  }
  return arTestExes.GetSize();
}

CConfigToolDoc::GlobalConflictOutcome CConfigToolDoc::ResolveGlobalConflicts(CPtrArray *parConflictsOfInterest)
{
  m_ConflictsOutcome=NotDone;
  m_arConflictsOfInterest.RemoveAll();
  if(parConflictsOfInterest){
    m_arConflictsOfInterest.Copy(*parConflictsOfInterest);
  }
  CdlInferenceCallback fn=CdlTransactionBody::get_inference_callback_fn();
  CdlTransactionBody::set_inference_callback_fn(CdlGlobalInferenceHandler);
  GetCdlInterpreter()->get_toplevel()->resolve_all_conflicts();
  CdlTransactionBody::set_inference_callback_fn(fn);
  if(NotDone==m_ConflictsOutcome){
    // No solutions were available, but we'll run the dialog anyway
    const std::list<CdlConflict>& conflicts=GetCdlConfig()->get_all_conflicts();  
    CFailingRulesDialog dlg(conflicts,NULL,&m_arConflictsOfInterest);
    m_ConflictsOutcome=(IDOK==dlg.DoModal())?OK:Cancel;
  }
  return m_ConflictsOutcome;
}

bool CConfigToolDoc::CheckConflictsBeforeSave()
{
  if(GetCdlInterpreter()->get_toplevel()->get_all_conflicts().size()>0){
    if(Deferred&m_nRuleChecking){
      if((SuggestFixes&m_nRuleChecking)&&(Cancel==ResolveGlobalConflicts())){
        return false;
      }
      int nConflicts=GetCdlInterpreter()->get_toplevel()->get_all_conflicts().size();
      switch(nConflicts){
      case 0:
        break;
      case 1:
        if(IDNO==CUtils::MessageBoxFT(MB_YESNO|MB_DEFBUTTON2,_T("There is 1 unresolved conflict.  Save anyway?"))){
          return false;
        }
      default:
        if(IDNO==CUtils::MessageBoxFT(MB_YESNO|MB_DEFBUTTON2,_T("There are %d unresolved conflicts.  Save anyway?"),nConflicts)){
          return false;
        }
      }
    }
  }
  return true;
}

BOOL CConfigToolDoc::IsModified() 
{ 
  return m_bModified || MemoryMap.map_modified();
}

void CConfigToolDoc::OnFileExport()
{
  CFileDialog dlg (FALSE, _T("ecm"), NULL, OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, _T("eCos Minimal Configuration (*.ecm)|*.ecm||"), AfxGetMainWnd ());
  TCHAR szBuffer [MAX_PATH * 16] = _T("");
  dlg.m_ofn.lpstrFile = szBuffer;
  dlg.m_ofn.nMaxFile = MAX_PATH * 16;
  dlg.m_ofn.lpstrTitle = _T("Export eCos Minimal Configuration");
  if (IDOK == dlg.DoModal ()) {
    try {
      TRACE (_T("Exporting eCos minimal configuration '%s'\n"), dlg.GetPathName ());
      m_CdlConfig->save (CUtils::UnicodeToStdStr (dlg.GetPathName ()), /* minimal = */ true);
    }
    catch (CdlStringException exception) {
      CUtils::MessageBoxF (_T("Error exporting eCos minimal configuration:\n\n%s"), CString (exception.get_message ().c_str ()));
    }
    catch (...) {
      CUtils::MessageBoxF (_T("Error exporting eCos minimal configuration."));
    }
  }
}

void CConfigToolDoc::OnFileImport()
{
  CFileDialog dlg (TRUE, _T("ecm"), NULL, OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, _T("eCos Minimal Configurations (*.ecm)|*.ecm||"), AfxGetMainWnd ());
  TCHAR szBuffer [MAX_PATH * 16] = _T("");
  dlg.m_ofn.lpstrFile = szBuffer;
  dlg.m_ofn.nMaxFile = MAX_PATH * 16;
  dlg.m_ofn.lpstrTitle = _T("Import eCos Minimal Configuration");
  if (IDOK == dlg.DoModal ()) {
    try {
      TRACE (_T("Importing eCos minimal configuration '%s'\n"), dlg.GetPathName ());
      m_CdlConfig->add (CUtils::UnicodeToStdStr (dlg.GetPathName ()), CConfigToolDoc::CdlParseErrorHandler, CConfigToolDoc::CdlParseWarningHandler);
    }
    catch (CdlStringException exception) {
      CUtils::MessageBoxF (_T("Error importing eCos minimal configuration:\n\n%s"), CString (exception.get_message ().c_str ()));
    }
    catch (...) {
      CUtils::MessageBoxF (_T("Error importing eCos minimal configuration."));
    }

    CWaitCursor wait;
    AddAllItems (); // regenerate all the config items since the topology may have changed
    if (m_strLinkerScriptFolder.IsEmpty ()) {
      CUtils::MessageBoxF (_T("The eCos linker script macro CYGBLD_LINKER_SCRIPT is not defined."));
    }
    if (m_strMemoryLayoutFolder.IsEmpty ()) {
      CUtils::MessageBoxF (_T("The eCos memory layout macro CYGHWR_MEMORY_LAYOUT is not defined."));
    }
    SwitchMemoryLayout (true); // the hardware template may have changed
    UpdateBuildInfo ();
    CConfigTool::GetControlView()->SelectItem (Item (0));
    SetModifiedFlag ();
  }
}

bool CConfigToolDoc::QualifyDocURL(CString &strURL)
{
  if(-1==strURL.Find(_T("://"))){
    strURL.Replace(_TCHAR('/'),_TCHAR('\\'));
    if (! CFileName (strURL).IsFile ()) { // if not an absolute filepath
      strURL = DocBase () + CFileName (strURL); // prepend the doc directory path
    }
    strURL=_T("file://")+strURL;
  }

  if(0==strURL.Find(_T("file://"))){
    CFileName strFile(strURL.Right(strURL.GetLength()-7));
    int nIndex=strFile.ReverseFind(_TCHAR('#'));
    if(-1!=nIndex){
      strFile=strFile.Left(nIndex);
    }
    strFile.Replace(_TCHAR('/'),_TCHAR('\\'));
    if(!strFile.Exists()){
      CUtils::MessageBoxF(_T("Cannot locate the file %s"),strFile);
      return false;
    }
  }
  return true;
}

bool CConfigToolDoc::NewMemoryLayout (const CString &strPrefix)
{
  CFileName strFileName = CurrentLinkerScript ();
  TRACE(_T("Reading linker-defined memory sections from %s\n"), strFileName);
  MemoryMap.new_memory_layout (); // delete the old memory layout regardless
  if (! strFileName.IsEmpty ())
    MemoryMap.import_linker_defined_sections (strFileName); // read the linker-defined section names from the repository (failure is silent)

  CString strMemoryLayoutFileName = strPrefix + _T("\\mlt_") + CurrentMemoryLayout () + _T(".mlt");
  TRACE(_T("Reading memory layout from %s\n"), strMemoryLayoutFileName);
  MemoryMap.load_memory_layout (strMemoryLayoutFileName); // load the new memory layout (failure is silent)
  strSelectedSection = _T("");
  strSelectedRegion = _T("");
  UpdateAllViews (NULL, MemLayoutChanged);
  return true; // FIXME
}

void CConfigToolDoc::SetPathName( LPCTSTR pszPath, BOOL bAddToMRU /*= TRUE*/ )
{
  if(_TCHAR('\0')==*pszPath){ // called like this after failed saves to put things back as they were
	  // CDocument::SetPathName would assert given an empty string
	  m_strPathName = _T("");
#ifndef PLUGIN
    SetTitle(_T("Untitled"));
#endif
    m_strBuildTree   = _T("");
    m_strInstallTree = _T("");
  } else {
#ifdef PLUGIN
    m_strPathName = CFileName (pszPath);
#else
    CDocument::SetPathName (pszPath, bAddToMRU);
#endif
    CString strFolder (pszPath);
    strFolder = strFolder.Left (strFolder.ReverseFind (_TCHAR('.'))); // extract folder from file path
    m_strBuildTree   = CFileName(strFolder + _T("_build")); 
    m_strInstallTree = CFileName(strFolder + _T("_install")); 
  }
}

bool CConfigToolDoc::ShowHtmlHelp (LPCTSTR pszURL)
{

  HWND hwndCaller=AfxGetMainWnd()->CWnd::GetDesktopWindow()->GetSafeHwnd();
  bool rc=false;
  const CFileName strFile(HTMLHelpLinkFileName());
  CStdioFile f;
  if(!CFileName(CConfigTool::strHelpFile).Exists()) {
    CUtils::MessageBoxF(_T("Cannot display help - %s does not exist\n"),CConfigTool::strHelpFile);
  } else if(!f.Open(strFile,CFile::typeText|CFile::modeCreate|CFile::modeWrite)){
    CUtils::MessageBoxF(_T("Cannot display help - error creating %s\n"),strFile);
  } else {
    CString str;
    str.Format(_T("<meta HTTP-EQUIV=\"refresh\" CONTENT=\"0;URL=%s\">"),pszURL);
    f.WriteString(str);
    f.Close();
    if(0==HtmlHelp(hwndCaller, CConfigTool::strHelpFile, HH_DISPLAY_TOPIC, 0)){
      CUtils::MessageBoxF(_T("Cannot display %s"),pszURL);
    } else {
      // FIXME: Do this the first time only?
      HH_WINTYPE WinType;
      HWND wnd;
      HH_WINTYPE *pWinType=NULL;
      wnd = HtmlHelp(hwndCaller, CConfigTool::strHelpFile+_T(">mainwin"), HH_GET_WIN_TYPE, (DWORD) &pWinType);
      WinType=*pWinType;
      WinType.hwndCaller=hwndCaller;
      WinType.fsWinProperties|=HHWIN_PROP_TRACKING;
      WinType.idNotify = ID_HHNOTIFICATION;
      wnd = HtmlHelp(hwndCaller, CConfigTool::strHelpFile, HH_SET_WIN_TYPE, (DWORD) &WinType);
      rc=true;
    }
    //::DeleteFile(strFile);
  }
  return rc;
}

const CString CConfigToolDoc::HTMLHelpLinkFileName()
{
  return CFileName(CConfigTool::strHelpFile).Head()+"link2.htm";
}
