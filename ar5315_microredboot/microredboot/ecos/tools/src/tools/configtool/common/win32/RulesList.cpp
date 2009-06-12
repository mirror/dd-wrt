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
// RulesView.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigToolDoc.h"
#include "RulesList.h"

#include "RulesView.h"
#include "ControlView.h"
#include "ConfigItem.h"
#include "ConfigTool.h"

#define INCLUDEFILE <string>
#include "IncludeSTL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRulesList

CRulesList::CRulesList():
  m_nLastCol(0x7fffffff),
  m_fWidth(0.0)
{
  m_f[0]=3.0/7.0;
  m_f[1]=1.0/7.0;
  m_f[2]=3.0/7.0;
}

CRulesList::~CRulesList()
{
}

BEGIN_MESSAGE_MAP(CRulesList, CTTListCtrl)
	//{{AFX_MSG_MAP(CRulesList)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
  ON_NOTIFY(HDN_ENDTRACKW, 0, OnEndTrack)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRulesList drawing

/////////////////////////////////////////////////////////////////////////////
// CRulesList message handlers

void CRulesList::AddConflict (const CdlConflict &conf)
{
  // set the item column string
  CString strMacroName = (conf)->get_node ()->get_name ().c_str ();
  int nIndex = InsertItem (GetItemCount (), strMacroName);
  SetItemData (nIndex, (DWORD) (conf));
  
  // set the conflict column string
  if (0 != dynamic_cast<CdlConflict_Unresolved> (conf)) {// a conflict of type 'unresolved'
    SetItemText (nIndex, 1, _T("Unresolved"));
  } else if (0 != dynamic_cast<CdlConflict_IllegalValue> (conf)) { // a conflict of type 'illegal value'
    SetItemText (nIndex, 1, _T("Illegal"));
  } else if (0 != dynamic_cast<CdlConflict_EvalException> (conf)) { // a conflict of type 'evaluation exception'
    SetItemText (nIndex, 1, _T("Exception"));
  } else if (0 != dynamic_cast<CdlConflict_Requires> (conf)) { // a conflict of type 'goal unsatisfied'
    SetItemText (nIndex, 1, _T("Unsatisfied"));
  } else if (0 != dynamic_cast<CdlConflict_Data> (conf)) { // a conflict of type 'bad data'
    SetItemText (nIndex, 1, _T("Bad data"));
  } else {
    ASSERT (0);
  }
  
	// set the property column string
	CString strProperty = conf->get_property ()->get_property_name ().c_str ();
	strProperty += _T(" ");
	const std::vector<std::string> & argv = conf->get_property ()->get_argv ();
	std::vector<std::string>::const_iterator argv_i;
  for (argv_i = argv.begin (); argv_i != argv.end (); argv_i++) {// for each property argument...
    if (argv_i != argv.begin ())                              // ...except the first
    {
      strProperty += argv_i->c_str (); // add the argument to the string
      strProperty += _T (" "); // separate arguments by a space character
    }
  }
	strProperty.TrimRight (); // remove the trailing space character
	SetItemText (nIndex, 2, strProperty);
}

int CRulesList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  lpCreateStruct->style|=LVS_REPORT;
	if (CTTListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	SetExtendedStyle(LVS_EX_FULLROWSELECT);

	InsertColumn(0,_T("Item"),	 LVCFMT_LEFT,0,0);	
	InsertColumn(1,_T("Conflict"),LVCFMT_LEFT,0,1);	
	InsertColumn(2,_T("Property"),LVCFMT_LEFT,0,2);	

  return 0;
}

void CRulesList::OnSize(UINT nType, int cx, int cy) 
{
 	CTTListCtrl::OnSize(nType, cx, cy);
  m_fWidth=cx;
  for(int i=0;i<NCOLS;i++){
	  SetColumnWidth(i,int(cx*m_f[i]));
  }
}

int CRulesList::CompareFunc(LPARAM lParam1, LPARAM lParam2)
{
	LV_FINDINFO find1 = { LVFI_PARAM, NULL, lParam1, NULL, NULL };
	LV_FINDINFO find2 = { LVFI_PARAM, NULL, lParam2, NULL, NULL };
	const int nIndex1 = FindItem (&find1);
	const int nIndex2 = FindItem (&find2);
	const CString str1 = GetItemText (nIndex1, m_nLastCol & 0x7fffffff);
	const CString str2 = GetItemText (nIndex2, m_nLastCol & 0x7fffffff);
	return (m_nLastCol & 0x80000000) ^ str1.Compare (str2);
}

void CRulesList::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	LPARAM nCol=pNMListView->iSubItem;
	if((nCol&0x7fffffff)==(0x7fffffff&m_nLastCol)){
		nCol=m_nLastCol^0x80000000;
	}
  m_nLastCol=nCol;
	SortItems(CompareFunc,(DWORD)this);
	
	*pResult = 0;
}

void CRulesList::OnEndTrack(NMHEADER *pNMHeader, LRESULT*) 
{
  m_f[pNMHeader->iItem]=pNMHeader->pitem->cxy/m_fWidth;
}

void CRulesList::AddConflicts (const std::list<CdlConflict>& conflicts)
{
  for (std::list<CdlConflict>::const_iterator conf_i=conflicts.begin (); conf_i != conflicts.end (); conf_i++) {
    AddConflict(*conf_i);
  }
}

void CRulesList::AddConflicts (const std::vector<CdlConflict>& conflicts)
{
  for (std::vector<CdlConflict>::const_iterator conf_i=conflicts.begin (); conf_i != conflicts.end (); conf_i++) {
    AddConflict(*conf_i);
  }
}

void CRulesList::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
  if (GetParent ()->IsKindOf (RUNTIME_CLASS (CRulesView))) { // handle double click for conflicts view only
    NM_LISTVIEW * pNMListView = (NM_LISTVIEW *) pNMHDR;
    int nItem = pNMListView->iItem;
    if (-1 != nItem) { // if the double click was on a row of the list
      CConfigItem * pItem = AssociatedItem (nItem, pNMListView->iSubItem); // get the referenced item
      if (pItem) { // if a referenced item was found
        CConfigTool::GetControlView ()->SelectItem (pItem); // select the refreenced item
      }
    }
  }

  *pResult = 0;
  UNUSED_ALWAYS(pNMHDR);
}

CConfigItem *CRulesList::AssociatedItem(int nRow,int nCol)
{
  const CdlConflict conflict = (CdlConflict) GetItemData (nRow);
  ASSERT (conflict);

  CConfigItem *pItem=NULL;
  switch(nCol){
    case 2:
      {
        const CdlGoalExpression goal = dynamic_cast<CdlGoalExpression> (conflict->get_property ());
        if (! goal) // if the item is not a goal expression
          break; // do nothing
        const CdlExpression expression = goal->get_expression ();
        if (1 == expression->references.size ()) // if the property contains a single reference
		{
          // assume that the reference is to another user visible node and try to find it
          const CString strName(expression->references [0].get_destination_name ().c_str());
          pItem = CConfigTool::GetConfigToolDoc ()->Find(strName);
        }
      }
      break;
    case 0:
      pItem = CConfigTool::GetConfigToolDoc ()->Find(CString(conflict->get_node ()->get_name ().c_str()));
      break;
    default:
      break;
  }
  return pItem;
}
