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
// Description:	This is the implementation of the configuration item class
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
#include "ConfigItem.h"
#include "ControlView.h"
#include "CTUtils.h"
#ifdef PLUGIN
  #define INCLUDEFILE "ide.common.h" // for setEditLocation
  #include "IncludeSTL.h"
  #include "common/CodeCoordinate.h"
#endif
#include "ConfigToolDoc.h"
#include "ConfigTool.h"

LPCTSTR CConfigItem::TreeItemTypeImage[MaxTreeItemType + 1]={
  _T("None"), _T("Integer"), _T("Enumeration"), _T("String"), _T("Double"), 0}; // Internationalization OK

const CFileName CConfigItem::FileName() const
{
  CFileName strFile;
  const CdlNode node = dynamic_cast<CdlNode> (m_CdlItem);
  if (node){
    // get the package which owns the configuration item
    const CdlPackage package = GetOwnerPackage();
    if (package){
      
      // return the filename of the config header
      strFile=CFileName(CConfigTool::GetConfigToolDoc()->InstallTree()+_T("include\\pkgconf"))+package->get_config_header ().c_str ();
    }
  }
  return strFile;
}

CConfigItem::CConfigItem(CConfigItem *pParent, CdlUserVisible CdlItem):
  m_CdlItem(CdlItem)
{
  CTreeCtrl &tree=CConfigTool::GetControlView()->GetTreeCtrl();
  HTREEITEM hParent;
  if(NULL==CdlItem){
    // This is the root item
    hParent=TVI_ROOT;
    m_Type=None;
  } else {
    hParent=pParent->HItem();
  
    // FIXME: re-implement using CdlValuableBody::get_widget_hint()
    if (IsPackage()) {
      // If a package item, display the package version string
      m_Type=String; 
    } else {
      const CdlValuable valuable = dynamic_cast<CdlValuable> (CdlItem);
      switch (valuable->get_flavor ()){
        case CdlValueFlavor_None:
        case CdlValueFlavor_Bool:
          m_Type=None;
          break;
        case CdlValueFlavor_Data:
        case CdlValueFlavor_BoolData:
          if (! valuable->has_legal_values ()) {
            m_Type=String;
          } else if (0 == valuable->get_legal_values ()->ranges.size ()) {
            m_Type=Enum;
          } else {
            CdlListValue list_value;
            CdlEvalContext context (NULL, valuable, valuable->get_property (CdlPropertyId_LegalValues));
            valuable->get_legal_values ()->eval (context, list_value);
            m_Type=list_value.get_double_ranges ().size () ? Double : Integer;
          }
          break;
        default:
          ASSERT (0); // specified flavor not supported
          break;
      }
    }  
  }
  m_hItem=tree.InsertItem(ItemNameOrMacro(),hParent);
  tree.SetItemData(m_hItem,(DWORD)this);
  CConfigTool::GetControlView()->AdjustItemImage(m_hItem);
}

CConfigItem::~CConfigItem()
{
}

CString CConfigItem::GetURL() const
{
  for(const CConfigItem *pItem=this;pItem;pItem=pItem->Parent()){
    if(pItem->GetCdlItem()){
      CString strURL;
      strURL=pItem->GetCdlItem()->get_doc_url().c_str();
      if(strURL.GetLength()){
        return strURL;
      }
      strURL=pItem->GetCdlItem()->get_doc().c_str();
      if(strURL.GetLength()){
        return strURL;
      }
    }
  }
  return _T("ref/ecos-ref.html"); // the default URL
}

bool CConfigItem::SetValue(LPCTSTR pszValue, CdlTransaction transaction/*=NULL*/)
{
  ASSERT ((m_Type == String) || (m_Type == Enum));
  const CdlValuable valuable = GetCdlValuable();
  ASSERT (valuable);
  const std::string str=CUtils::UnicodeToStdStr (pszValue);
  if(transaction){
    if (CdlValueFlavor_BoolData == valuable->get_flavor ()){
      // set the user bool to the current bool when changing a booldata
      // value to avoid a possible change in the current bool
      valuable->set_enabled_and_value (transaction, valuable->is_enabled (), str, CdlValueSource_User);
    } else {// CdlValueFlavor_Data
      valuable->set_value (transaction, str, CdlValueSource_User);
    }
  } else {
    if (CdlValueFlavor_BoolData == valuable->get_flavor ()){
      // set the user bool to the current bool when changing a booldata
      // value to avoid a possible change in the current bool
      valuable->set_enabled_and_value (valuable->is_enabled (), str, CdlValueSource_User);
    } else {// CdlValueFlavor_Data
      valuable->set_value (str, CdlValueSource_User);
    }
  }
  
  return true;
}

bool CConfigItem::SetValue (double dValue, CdlTransaction transaction/*=NULL*/)
{
  ASSERT (m_Type == Double);
  const CdlValuable valuable = GetCdlValuable();
  ASSERT (valuable);
  
  if(transaction) {
    if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
      // set the user bool to the current bool when changing a booldata
      // value to avoid a possible change in the current bool
      valuable->set_enabled_and_value (transaction, valuable->is_enabled (), dValue, CdlValueSource_User);
    } else {// CdlValueFlavor_Data
      valuable->set_double_value (transaction, dValue, CdlValueSource_User);
    }
  } else {
    if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
      // set the user bool to the current bool when changing a booldata
      // value to avoid a possible change in the current bool
      valuable->set_enabled_and_value (valuable->is_enabled (), dValue, CdlValueSource_User);
    } else {// CdlValueFlavor_Data
      valuable->set_double_value (dValue, CdlValueSource_User);
    }
  }
  
  return true;
}

CConfigItem *CConfigItem::FirstRadio() const
{
  ASSERT(HasRadio ());
  
  for(CConfigItem *h=Parent()->FirstChild();h;h=h->NextSibling()){
    if(h->HasRadio ()){
      return h;
    }
  }
  // No radio buttons found
  ASSERT(false);
  return false;
}

bool CConfigItem::IsEnabled() const
{
  const CdlValuable valuable = GetCdlValuable();
  return NULL==valuable ||valuable->is_enabled();
}

bool CConfigItem::SetValue (ItemIntegerType nValue, CdlTransaction transaction/*=NULL*/)
{
  ASSERT (m_Type == Integer);
  const CdlValuable valuable = GetCdlValuable();
  ASSERT (valuable);

  if(transaction) {
    if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
      // set the user bool to the current bool when changing a booldata
      // value to avoid a possible change in the current bool
      valuable->set_enabled_and_value (transaction, valuable->is_enabled (), (cdl_int) nValue, CdlValueSource_User);
    } else { // CdlValueFlavor_Data
      valuable->set_integer_value (transaction, nValue, CdlValueSource_User);
    }
  } else {
    if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
      // set the user bool to the current bool when changing a booldata
      // value to avoid a possible change in the current bool
      valuable->set_enabled_and_value (valuable->is_enabled (), (cdl_int) nValue, CdlValueSource_User);
    } else { // CdlValueFlavor_Data
      valuable->set_integer_value (nValue, CdlValueSource_User);
    }
  }
  
  return true;
}

bool CConfigItem::HasModifiedChildren() const
{
  for(CConfigItem *pItem=FirstChild();pItem;pItem=pItem->NextSibling()){
    if(pItem->Modified()||pItem->HasModifiedChildren()){
      return true;
    }
  }
  return false;
}

ItemIntegerType CConfigItem::Value () const
{
  ASSERT (!IsPackage()); // not a package item
  const CdlValuable valuable = GetCdlValuable();
  ASSERT (valuable);
  ItemIntegerType nValue (0);
  
  switch (valuable->get_flavor ())
  {
    //	case CdlValueFlavor_Bool:
    //		nValue = valuable->is_enabled (CdlValueSource_Current) ? 1 : 0;
    //		break;
    
  case CdlValueFlavor_BoolData:
  case CdlValueFlavor_Data:
    nValue = (ItemIntegerType) valuable->get_integer_value (CdlValueSource_Current);
    break;
    
  default:
    ASSERT (0); // specified flavor not supported
  }
  
  return nValue;
}

const double CConfigItem::DoubleValue (CdlValueSource source /* = CdlValueSource_Current */ ) const
{
  ASSERT (!IsPackage()); // not a package item
  const CdlValuable valuable = GetCdlValuable();
  ASSERT (valuable);
  ASSERT (valuable->has_double_value (source));
  return valuable->get_double_value (source);
}

ItemIntegerType CConfigItem::DefaultValue () const
{
  ItemIntegerType nValue;
  return CUtils::StrToItemIntegerType (StringValue (CdlValueSource_Default), nValue) ? nValue : 0;
}

const CString CConfigItem::StringValue (CdlValueSource source /* = CdlValueSource_Current */ ) const
{
  //	ASSERT (!IsPackage()); // not a package item
  const CdlValuable valuable = GetCdlValuable();
  ASSERT (valuable);
  CString strValue (_T(""));
  
  switch (valuable->get_flavor ())
  {
    case CdlValueFlavor_Data:
    case CdlValueFlavor_BoolData:
    case CdlValueFlavor_None: // a package
      if (m_Type == Integer)
        strValue = CUtils::IntToStr (Value (), CConfigTool::GetConfigToolDoc ()->m_bHex);
      else if (m_Type == Double)
        strValue = CUtils::DoubleToStr (DoubleValue ());
      else
        strValue = valuable->get_value (source).c_str ();
      break;
    
    default:
      ASSERT (0); // specified flavor not supported
  }
  
  return strValue;
}

const CString CConfigItem::StringValue(WhereType where) const
{
  CString str;
  switch(where){
    case InName:
      str=Name();
      break;
    case InMacro:
      str=Macro();
      break;
    case InDesc:
      str=Desc();
      break;
    case InCurrentValue:
      str=CConfigItem::None==Type()?_T(""):StringValue(CdlValueSource_Current);
      break;
    case InDefaultValue:
      str=CConfigItem::None==Type()?_T(""):StringValue(CdlValueSource_Default);
      break;
    default:
      ASSERT(FALSE);
      break;
  }
  return str;
}    

void CConfigItem::DumpItem()
{
  TRACE(_T("Item %08x\n\tDisplay Name='%s'\n\tMacro Name='%s'\n\tType=%s"),
    this,	Name(),           Macro(),    TreeItemTypeImage[m_Type]);
  TRACE(_T("\n\tValue=%s\n\tURL=%s\n\tParent=%08x"),StringValue(), GetURL(), Parent());
  
  TRACE(_T("\n"));
}

CConfigItem * CConfigItem::NextRadio() const
{
  ASSERT(this->HasRadio ());
  for(CConfigItem *pItem=NextSibling();pItem;pItem=pItem->NextSibling()){
    if(pItem->HasRadio()){
      return pItem;
    }
  }
  return NULL;
}

bool CConfigItem::Modified () const
{
  const CdlValuable valuable = GetCdlValuable();
  return 
    valuable        // accommodate the root config item which has no CDL item
    && !IsPackage() // packages are never modified
    && valuable->get_source () != CdlValueSource_Default;
}

CString CConfigItem::ItemNameOrMacro() const
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  return pDoc->m_bMacroNames&&!Macro().IsEmpty()?Macro():Name();
}

bool CConfigItem::IsDescendantOf(CConfigItem * pAncestor)
{
  for(CConfigItem *pItem=Parent();pItem;pItem=pItem->Parent()){
    if(pItem==pAncestor){
      return true;
    }
  }
  return false;
}

int CConfigItem::EvalEnumStrings (CStringArray &arEnumStrings) const
{
  const CdlValuable valuable = GetCdlValuable();
  ASSERT (valuable);
  /*
  if (m_Type == Boolean)
  {
  arEnumStrings.SetSize (2);
  arEnumStrings.SetAt (0, _T("True"));
  arEnumStrings.SetAt (1, _T("False"));
  }
  else
  */
  {
    ASSERT (m_Type == Enum);
    CdlListValue list_value;
    CdlEvalContext context (NULL, m_CdlItem, m_CdlItem->get_property (CdlPropertyId_LegalValues));
    valuable->get_legal_values ()->eval (context, list_value);
    const std::vector<CdlSimpleValue> & table = list_value.get_table ();
    
    // add legal values to the list
    arEnumStrings.SetSize (table.size ());
    for (unsigned int nValue = 0; nValue < table.size (); nValue++)
    {
      arEnumStrings.SetAt (nValue, table [nValue].get_value ().c_str ());
    }
  }
  return arEnumStrings.GetSize();
}

bool CConfigItem::HasBool() const
{
  if (!m_CdlItem) {
    return false;
  } else if (IsPackage()) {
    return false;
  } else {
    const CdlValuable valuable = GetCdlValuable();
    CdlValueFlavor flavor = valuable->get_flavor ();
    return (flavor == CdlValueFlavor_Bool) || (flavor == CdlValueFlavor_BoolData);
  }
}

bool CConfigItem::HasRadio() const
{
  const CdlValuable valuable = GetCdlValuable();
  if (! valuable)
    return false;
  
  CdlWidgetHint hint;
  valuable->get_widget_hint (hint);
  return (CdlBoolWidget_Radio == hint.bool_widget);
}

bool CConfigItem::SetEnabled(bool bEnabled, CdlTransaction current_transaction/*=NULL*/)
{
  const CdlValuable valuable = GetCdlValuable();
  ASSERT (valuable);
  
  // use a transaction object to ensure that all config items are changed together
  CdlTransaction transaction = current_transaction ? current_transaction : CdlTransactionBody::make (CConfigTool::GetConfigToolDoc ()->GetCdlConfig ());
  
  if (HasRadio () && bEnabled) { // if a new radio button has been selected
    for (CConfigItem *pItem = FirstRadio(); pItem; pItem = pItem->NextRadio ()) { // for each radio button in the group
      if (pItem != this) { // if not the newly selected radio button
        pItem->SetEnabled (false, transaction); // disable the radio button
      }
    }
  }
  
  if (CdlValueFlavor_BoolData == valuable->get_flavor ()) {
    // set the user value to the current data value when enabling/disabling
    // a booldata item to avoid a possible change in the current data value
    CdlSimpleValue simple_value = valuable->get_simple_value ();
    valuable->set_enabled_and_value (transaction, bEnabled, simple_value, CdlValueSource_User);
  } else { // CdlValueFlavor_Bool
    valuable->set_enabled (transaction, bEnabled, CdlValueSource_User);
  }
  
  if (! current_transaction) { // if not a recursive call to disable a radio button
    transaction->body (); // commit the transaction
    deleteZ(transaction);
  }
  
  return true;
}

bool CConfigItem::ViewHeader()
{
  bool rc=false;
  const CFileName strFile(FileName());
  if(!strFile.IsEmpty()){
    CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
    if(pDoc->BuildTree().IsEmpty()){
      CUtils::MessageBoxF(_T("Cannot display header file until configuration is saved"));
    } else {
#ifdef PLUGIN
      // Load or activate window and leave
      CodeCoordinate loc((LPCTSTR)strFile, 0, 0, CodeCoordinate::FILE_LINE);
      rc=AppInstance::getAppManager()->getEditorController()->setEditLocation(loc);
#else
      rc=CUtils::Launch(strFile,pDoc->m_strViewer);
#endif
    }
  }
  return rc;
}

bool CConfigItem::ViewURL()
{
  return CConfigTool::GetConfigToolDoc()->ShowURL(GetURL());
}

// Unload (a package)
bool CConfigItem::Unload()
{
  bool rc=false;
  CdlPackage package=dynamic_cast<CdlPackage>(GetCdlItem());
  ASSERT(package);
  CConfigToolDoc* pDoc=CConfigTool::GetConfigToolDoc();
  // Remove its objects from the view to prevent any painting problems
  CConfigTool::GetControlView()->GetTreeCtrl().DeleteItem(HItem());
  for(int nItem=0;nItem<pDoc->ItemCount();nItem++){
    CConfigItem *pItem=pDoc->Item(nItem);
    if(package==pItem->GetOwnerPackage()){
      //CConfigTool::GetControlView()->GetTreeCtrl().DeleteItem(pItem->HItem());
      pItem->m_hItem=NULL;   // Make sure we can't attempt to paint it
      pItem->m_CdlItem=NULL; // Make sure we can't access stale data
    }
  }
  
  const CString strMacroName(Macro());
  TRACE (_T("Unloading package %s\n"), strMacroName);
  try {
    pDoc->GetCdlConfig()->unload_package (package);
    rc=true;
  }
  catch (CdlStringException exception) {
    CUtils::MessageBoxF(_T("Error unloading package %s:\n\n%s"), strMacroName, CString (exception.get_message ().c_str ()));
  }
  catch (...) {
    CUtils::MessageBoxF(_T("Error unloading package %s"), strMacroName);
  }
  m_hItem=NULL;   // Make sure we can't attempt to paint it
  m_CdlItem=NULL; // Make sure we can't access stale data
  return rc;
}

// Change version (of a package)
bool CConfigItem::ChangeVersion(const CString &strVersion)
{
  bool rc=false;
  CdlPackage package=dynamic_cast<CdlPackage>(GetCdlItem());
  ASSERT(package);
  const CdlValuable valuable = GetCdlValuable();
  ASSERT (valuable);
  const CString strMacroName(Macro());
  if (strVersion != valuable->get_value ().c_str ()) { // if the wrong version is loaded
    TRACE (_T("Changing package %s to version '%s'\n"), strMacroName, strVersion);
    try {
      CConfigTool::GetConfigToolDoc()->GetCdlConfig()->change_package_version (package, CUtils::UnicodeToStdStr (strVersion), CConfigToolDoc::CdlParseErrorHandler, CConfigToolDoc::CdlParseWarningHandler);
      rc=true;
    }
    catch (CdlStringException exception) {
      CUtils::MessageBoxF(_T("Error changing package %s to version '%s':\n\n%s"), strMacroName, strVersion, CString (exception.get_message ().c_str ()));
    }
    catch (...) {
      CUtils::MessageBoxF(_T("Error changing package %s to version '%s'"), strMacroName, strVersion);
    }
  }
  return rc;
}

CConfigItem *CConfigItem::Parent() const 
{ 
  CTreeCtrl &tree=CConfigTool::GetControlView()->GetTreeCtrl();
  HTREEITEM hParent=tree.GetParentItem(HItem());
  return (NULL==hParent||TVI_ROOT==hParent)?NULL:(CConfigItem *)tree.GetItemData(hParent);
}

CConfigItem *CConfigItem::FirstChild() const
{ 
  CTreeCtrl &tree=CConfigTool::GetControlView()->GetTreeCtrl();
  HTREEITEM hChild=tree.GetChildItem(HItem());
  return hChild?(CConfigItem *)tree.GetItemData(hChild):NULL;
}

CConfigItem *CConfigItem::NextSibling() const
{ 
  CTreeCtrl &tree=CConfigTool::GetControlView()->GetTreeCtrl();
  HTREEITEM hSibling=tree.GetNextSiblingItem(HItem());
  return hSibling?(CConfigItem *)tree.GetItemData(hSibling):NULL;
}
