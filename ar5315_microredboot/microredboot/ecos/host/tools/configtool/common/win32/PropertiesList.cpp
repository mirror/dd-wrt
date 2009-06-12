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
// PropertiesView2.cpp : implementation file
//
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the properties window view.
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
#include "PropertiesList.h"
#include "ConfigToolDoc.h"
#include "ConfigItem.H"

#include "CTUtils.h"
#include "ControlView.h"
#include "ConfigTool.h"

#define INCLUDEFILE <string>
#include "IncludeSTL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// specify the CDL properties which are to be visible in the properties view
const std::string CPropertiesList::visible_properties [] =
{
	CdlPropertyId_ActiveIf,
	CdlPropertyId_BuildProc,
	CdlPropertyId_Calculated,
	CdlPropertyId_CancelProc,
	CdlPropertyId_CheckProc,
	CdlPropertyId_Compile,
	CdlPropertyId_ConfirmProc,
	CdlPropertyId_DecorationProc,
	CdlPropertyId_DefaultValue,
	CdlPropertyId_Define,
	CdlPropertyId_DefineHeader,
	CdlPropertyId_DefineProc,
//	CdlPropertyId_Description,
	CdlPropertyId_Dialog,
//	CdlPropertyId_Display,
	CdlPropertyId_DisplayProc,
	CdlPropertyId_Doc,
	CdlPropertyId_EntryProc,
	CdlPropertyId_Flavor,
	CdlPropertyId_DefineFormat,
	CdlPropertyId_Group,
	CdlPropertyId_Hardware,
	CdlPropertyId_IfDefine,
	CdlPropertyId_Implements,
	CdlPropertyId_IncludeDir,
	CdlPropertyId_IncludeFiles,
	CdlPropertyId_InitProc,
	CdlPropertyId_InstallProc,
	CdlPropertyId_LegalValues,
	CdlPropertyId_Library,
	CdlPropertyId_LicenseProc,
	CdlPropertyId_Make,
	CdlPropertyId_Makefile,
	CdlPropertyId_MakeObject,
	CdlPropertyId_NoDefine,
	CdlPropertyId_Object,
	CdlPropertyId_Parent,
	CdlPropertyId_Requires,
	CdlPropertyId_Screen,
	CdlPropertyId_Script,
	CdlPropertyId_UpdateProc,
	CdlPropertyId_Wizard
};

/////////////////////////////////////////////////////////////////////////////
// CPropertiesList

CPropertiesList::CPropertiesList() :
  m_pti(NULL),
  m_nFirstProperty(0),
  m_nOnSizeRecursionCount(0)
{
  m_f[0]=0.25;
  m_f[1]=0.75;
  m_GrayPen.CreatePen(PS_SOLID,1,RGB(192,192,192));	
}

CPropertiesList::~CPropertiesList()
{
}
//CListCtrl

BEGIN_MESSAGE_MAP(CPropertiesList, CListCtrl)
	//{{AFX_MSG_MAP(CPropertiesList)
  ON_NOTIFY(HDN_ENDTRACKW, 0, OnEndTrack)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_NOTIFY_REFLECT(HDN_TRACK, OnTrack)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

const LPCTSTR CPropertiesList::FieldTypeImage[MAXFIELDTYPE]=
	{_T("Type"), _T("Value"), _T("Default"), _T("Macro"), _T("File"), _T("URL"), _T("Enabled")};

/////////////////////////////////////////////////////////////////////////////
// CPropertiesList message handlers

void CPropertiesList::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{

  // Double-clicked the item	
  
  // We do not use the parameters on the OnDblClk handlers in order to preserve compatibility
  // between pre- and post-4.71 comctrl32.dll versions.
  
  int pos=GetMessagePos();
  CPoint pt(GET_X_LPARAM(pos),GET_Y_LPARAM(pos));
  ScreenToClient(&pt);
  int nItem=HitTest(pt,NULL);
  
  if(GetItemData(nItem)>1){
    // This is a property row
    const CdlGoalExpression goal = dynamic_cast<CdlGoalExpression> ((CdlProperty) GetItemData (nItem));
    if (goal){
      // This is a rule row
      const CdlExpression expression = goal->get_expression ();
      if (1 == expression->references.size ()) // if the property contains a single reference
      {
        // assume that the reference is to another user visible node and try to find it
        std::string macro_name = expression->references [0].get_destination_name ();
        CConfigItem * pItem = CConfigTool::GetConfigToolDoc ()->Find (CString (macro_name.c_str ()));
        if (pItem) // the referenced node was found so select it
        {
          CConfigTool::GetControlView()->GetTreeCtrl().SelectItem(pItem->HItem());
        }
      }
    }
  } else {
    const CString strText(GetItemText(nItem,0));
    if(strText==FieldTypeImage[File]){
      m_pti->ViewHeader();
    } else if (strText==FieldTypeImage[URL]) {
      m_pti->ViewURL();
    }
  }

	UNUSED_ALWAYS(pResult);
	UNUSED_ALWAYS(pNMHDR);
}
void CPropertiesList::OnPaint() 
{
	// Default painting
  Default();

	// Draw the grid
	int nItems=GetItemCount();
	if(nItems>0){
		CDC &dc=*GetDC(); // device context for painting
    CRect rcHeader;
    GetHeaderCtrl()->GetClientRect(rcHeader);
    dc.ExcludeClipRect(rcHeader);
		CFont *pOldFont=dc.SelectObject(GetFont());
    CPen *pOldPen=dc.SelectObject(&m_GrayPen);

    CRect rect;
		GetItemRect(0,rect,LVIR_BOUNDS);

    int cy=rect.top-1;
		int dy=rect.Height();

    // This prevents the vertical line leaving shadows when column dragging occurs
    rect.top--;
    rect.bottom=rect.top+1;
    dc.FillSolidRect(rect,GetSysColor(COLOR_WINDOW));

		// Vertical line
		GetItemRect(0,rect,LVIR_LABEL);

    dc.MoveTo(rect.right-1,cy-1);
		dc.LineTo(rect.right-1,cy+dy*nItems);

    GetClientRect(rect);
    int cx=rect.Width();

    for(int i=0;i<nItems;i++){
			cy+=dy;
			dc.MoveTo(0,cy);
			dc.LineTo(cx,cy);
		}

		dc.SelectObject(pOldPen);
		dc.SelectObject(pOldFont);
		
		ReleaseDC(&dc);
	}
}

void CPropertiesList::Fill(CConfigItem *pti)
{
  if(NULL==pti){
    DeleteAllItems();
    m_nFirstProperty=0;
    m_pti=NULL;
  } else if(pti!=m_pti){
    m_pti=pti;
    m_nMaxValueWidth=0;
    CConfigItem::TreeItemType type=m_pti->Type();
    int i;  

    // Initially flag all items as unnecessary - calls of SetItem or SetProperty will change this
    for(i=GetItemCount()-1;i>=0;--i){
      SetItemData(i,0);
    }
	  if (m_pti->HasBool () || (CConfigItem::None!=type)){
      SetItem(Macro, m_pti->Macro ());
    }
  
    if (m_pti->HasBool ()){
      SetItem(Enabled,m_pti->IsEnabled() ? _T("True") : _T("False"));
    }
    
    if(!m_pti->FileName().IsEmpty()){
      SetItem(File,m_pti->FileName());
    }
    SetItem(URL,m_pti->GetURL());

    if(CConfigItem::None!=type){
      switch(type){
        case CConfigItem::String:
          SetItem(Value,m_pti->StringValue());
          SetItem(DefaultValue,m_pti->StringDefaultValue());
          break;
        case CConfigItem::Integer:
          SetItem(Value,CUtils::IntToStr(m_pti->Value(),CConfigTool::GetConfigToolDoc()->m_bHex));
          SetItem(DefaultValue,CUtils::IntToStr(m_pti->DefaultValue(),CConfigTool::GetConfigToolDoc()->m_bHex));
          break;
        case CConfigItem::Double:
          SetItem(Value,CUtils::DoubleToStr(m_pti->DoubleValue()));
          SetItem(DefaultValue,CUtils::DoubleToStr(m_pti->DoubleDefaultValue()));
          break;
        case CConfigItem::Enum:
          SetItem(Value,m_pti->StringValue());
          SetItem(DefaultValue,m_pti->StringDefaultValue());
          break;
        default:
          ASSERT(FALSE);
          break;
      }
      SetItem(Type,CConfigItem::TreeItemTypeImage[type]);
    }
    
    // List all the properties applicable to me
    const std::string name = CUtils::UnicodeToStdStr (m_pti->Macro ());
    if (name.size () > 0)
    {
      const CdlConfiguration config = CConfigTool::GetConfigToolDoc ()->GetCdlConfig ();
      const CdlNode node = config->find_node (name, true);
      ASSERT (node);
      const std::vector<CdlProperty> & properties = node->get_properties ();
      std::vector<CdlProperty>::const_iterator property_i;
      CMapStringToPtr map; // count of each property name
      for (property_i = properties.begin (); property_i != properties.end (); property_i++) {// for each property
        // get the property name
        const CdlProperty &prop=*property_i;
        const CString strName(prop->get_property_name ().c_str());
        enum {VISIBLE_PROPERTIES_COUNT=sizeof visible_properties/sizeof visible_properties[0]};
        if (std::find (visible_properties, visible_properties + VISIBLE_PROPERTIES_COUNT, CUtils::UnicodeToStdStr(strName)) != visible_properties + VISIBLE_PROPERTIES_COUNT) {// if the property should be displayed
          // set the property arguments
          CString strPropertyArgs;
          const std::vector<std::string> & argv = prop->get_argv ();
          void *p;
          if(!map.Lookup(strName,p)){
            p=0;
          }

          p=(void *)((int)p+1);
          map.SetAt(strName,p);
          
          std::vector<std::string>::const_iterator argv_i;
          for (argv_i = argv.begin (); argv_i != argv.end (); argv_i++){ // for each property argument...
            if (argv_i != argv.begin ()){                              // ...except the first (the property name)
              CString strArg(CUtils::StripExtraWhitespace (CString(argv_i->c_str())));
              if (strPropertyArgs.GetLength () + strArg.GetLength() + 1 > 256) {// if the string is too long for the list control
                break; // no need to add any further arguments
              }
              strPropertyArgs += strArg; // add the argument to the string
              strPropertyArgs += _T (" "); // separate arguments by a space character
            }
          }
          // the list control appears to display a maximum of 256 characters
          int nIndex=SetItem(strName,strPropertyArgs,GetItemCount(),(int)p);
          SetItemData(nIndex,(DWORD)prop);
          
          // display the exclamation icon if the property is in a conflicts list
          bool bConflictItem =
            //					PropertyInConflictsList (* property_i, config->get_structural_conflicts ()) || ignore for now
            PropertyInConflictsList (prop, config->get_all_conflicts ());
          CListCtrl::SetItem (nIndex, 0, LVIF_IMAGE, NULL, bConflictItem ? 1 : 0, 0, 0, 0 );
        }
      }
    }
    
    for(i=GetItemCount()-1;i>=0;--i){
      if(0==GetItemData(i)){
        DeleteItem(i);
        if(i<m_nFirstProperty){
          m_nFirstProperty--;
        }
      }
    }
    CRect rect;
    GetClientRect(rect);
    int nAvailWidth=rect.Width()-GetColumnWidth(0);
    int w=max(m_nMaxValueWidth,nAvailWidth);
    m_f[1]=double(w)/double(rect.Width());
    SetColumnWidth(1,w); 
  }
}


bool CPropertiesList::PropertyInConflictsList (CdlProperty property, const std::list<CdlConflict> & conflicts)
{
	std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i = conflicts.begin (); conf_i != conflicts.end (); conf_i++) // for each conflict
		if (property == (* conf_i)->get_property ())
			return true;

	return false;
}

// set item text in the properties list control, extending the list if necessary
int CPropertiesList::SetItemTextGrow(int nItem, LPCTSTR lpszItem)
{
	while (GetItemCount () < nItem + 1)
	{
    int n=InsertItem (GetItemCount (), _T(""));
		if (-1 == n){
			return -1;
		}
	}
	return SetItemText (nItem, 0, lpszItem);
}


int CPropertiesList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  lpCreateStruct->style|=WS_HSCROLL|WS_VSCROLL|LVS_REPORT|LVS_REPORT|LVS_SINGLESEL;
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

  //GetHeaderCtrl()->ModifyStyle(HDS_FULLDRAG,0,0); // remove HDS_FULLDRAG style from header
	
	ListView_SetExtendedListViewStyle(GetSafeHwnd(),/*LVS_EX_GRIDLINES|*/LVS_EX_FULLROWSELECT/*|LVS_EX_ONECLICKACTIVATE*//*|LVS_EX_TRACKSELECT*/);
	InsertColumn(0,_T("Property"),LVCFMT_LEFT,0,0);	
	InsertColumn(1,_T("Value"),LVCFMT_LEFT,0,1);	

	return 0;
}


void CPropertiesList::OnSize(UINT nType, int cx, int cy) 
{
	CListCtrl::OnSize(nType, cx, cy);
  if(0==m_nOnSizeRecursionCount++){//prevent recursion
    m_fWidth=cx;
    for(int i=0;i<NCOLS;i++){
	    SetColumnWidth(i,int(cx*m_f[i]));
    }
  }
  m_nOnSizeRecursionCount--;
}

void CPropertiesList::OnEndTrack(NMHEADER *pNMHeader, LRESULT*) 
{
  m_f[pNMHeader->iItem]=pNMHeader->pitem->cxy/m_fWidth;
}

void CPropertiesList::OnTrack(NMHEADER*, LRESULT*) 
{
  CRect rect;
  GetItemRect(0,rect,LVIR_BOUNDS);
  rect.bottom=rect.top+2;
  InvalidateRect(rect);
}

void CPropertiesList::RefreshValue()
{
  if (m_pti->HasBool ()){
		SetItem(CPropertiesList::Enabled, m_pti->IsEnabled () ? _T("True") : _T("False"));
  }
  if (m_pti->Type () != CConfigItem::None){
		SetItem(CPropertiesList::Value,m_pti->StringValue());
  }
	for (int nItem = m_nFirstProperty; nItem < GetItemCount (); nItem++)
	{
		CdlProperty property = (CdlProperty) GetItemData (nItem);
		ASSERT (property);

		// display the exclamation icon if the property is in a conflicts list
		const CdlConfiguration config = CConfigTool::GetConfigToolDoc ()->GetCdlConfig ();
		bool bConflictItem =
//						PropertyInConflictsList (property, config->get_structural_conflicts ()) || ignore for now
			PropertyInConflictsList (property, config->get_all_conflicts ());
    CListCtrl::SetItem (nItem, 0, LVIF_IMAGE, NULL, bConflictItem ? 1 : 0, 0, 0, 0 );
	}

}

int CPropertiesList::SetItem(FieldType f, LPCTSTR pszValue)
{
  int nIndex=SetItem(FieldTypeImage[f],pszValue,m_nFirstProperty);
  if(nIndex==m_nFirstProperty){
    m_nFirstProperty++;
  }
  SetItemData(nIndex,1);
  return nIndex;
}

int CPropertiesList::SetItem(LPCTSTR pszItem, LPCTSTR pszValue, int nInsertAs,int nRepeat)
{
  ASSERT(nInsertAs<=GetItemCount());
  LVFINDINFO info;
  info.flags =LVFI_STRING;
  info.psz   =pszItem;
  info.vkDirection=VK_DOWN;
  int nIndex=-1;
  do {
    nIndex=FindItem(&info,nIndex);
  } while (--nRepeat>0 && nIndex!=-1);

  if(-1==nIndex){
    nIndex=InsertItem(nInsertAs,pszItem);
  } 

  SetItemText(nIndex,1,pszValue);
  CDC *pDC=GetDC();
  CFont *pOldFont=pDC->SelectObject(GetFont());
  m_nMaxValueWidth=max(m_nMaxValueWidth,pDC->GetTextExtent(pszValue).cx);
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);
  return nIndex;

}
