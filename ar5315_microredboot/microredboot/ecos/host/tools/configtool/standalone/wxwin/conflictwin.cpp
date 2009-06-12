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
// configtree.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/04
// Version:     $Id: conflictwin.cpp,v 1.3 2001/04/24 14:39:13 julians Exp $
// Purpose:
// Description: Implementation file for ecConflictListCtrl
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
    #pragma implementation "conflictwin.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "conflictwin.h"
#include "configtool.h"
#include "configtooldoc.h"
#include "configtree.h"

/*
 * ecConflictListCtrl
 */

IMPLEMENT_CLASS(ecConflictListCtrl, wxListCtrl)

BEGIN_EVENT_TABLE(ecConflictListCtrl, wxListCtrl)
    EVT_RIGHT_DOWN(ecConflictListCtrl::OnRightClick)
    EVT_LEFT_DCLICK(ecConflictListCtrl::OnLeftDClick)

    EVT_MENU(ecID_LOCATE_ITEM, ecConflictListCtrl::OnLocate)
    EVT_MENU(ecID_RESOLVE_ITEM, ecConflictListCtrl::OnResolve)
END_EVENT_TABLE()

ecConflictListCtrl::ecConflictListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
        const wxSize& sz, long style):
        wxListCtrl(parent, id, pt, sz, style)
{
    if (!wxGetApp().GetSettings().GetWindowSettings().GetUseDefaults() &&
         wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Conflicts")).Ok())
    {
        SetFont(wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Conflicts")));
    }

    InsertColumn(0, "Item", wxLIST_FORMAT_LEFT, 200);
    InsertColumn(1, "Conflict", wxLIST_FORMAT_LEFT, 80);
    InsertColumn(2, "Property", wxLIST_FORMAT_LEFT, 200);

    m_contextMenu = new wxMenu;
    m_contextMenu->Append(ecID_WHATS_THIS, _("&What's This?"));
    m_contextMenu->AppendSeparator();
    m_contextMenu->Append(ecID_LOCATE_ITEM, _("&Locate"));
    m_contextMenu->Append(ecID_RESOLVE_ITEM, _("&Resolve"));

    m_contextItem = -1;
    m_contextCol = 0;
}

ecConflictListCtrl::~ecConflictListCtrl()
{
    delete m_contextMenu;
}

void ecConflictListCtrl::OnRightClick(wxMouseEvent& event)
{
    int flags = 0;
    long item = HitTest(wxPoint(event.GetX(), event.GetY()), flags);

    if (item >= 0 && !GetParent ()->IsKindOf (CLASSINFO(wxDialog)))  // this menu for conflicts view only
    {
        m_contextItem = item;

        // Find which column we're on
        m_contextCol = wxListCtrlFindColumn(*this, 3, event.GetX());

        // GetContextMenu()->SetClientData((void*) TRUE);
        PopupMenu(GetContextMenu(), event.GetX(), event.GetY());
    }
    else
    {
        m_contextItem = -1;

        PopupMenu(wxGetApp().GetWhatsThisMenu(), event.GetX(), event.GetY());
    }
}

void ecConflictListCtrl::OnLeftDClick(wxMouseEvent& event)
{
    if (!GetParent ()->IsKindOf (CLASSINFO(wxDialog)))  // handle double click for conflicts view only
    {
        long sel = wxListCtrlGetSelection(* this);
        if (sel >= 0)
        {
            // Find which column we're on
            int col = wxListCtrlFindColumn(*this, 3, event.GetX());

            ecConfigItem * pItem = AssociatedItem (sel, col); // get the referenced item
            
            if (pItem) { // if a referenced item was found
                wxGetApp().GetTreeCtrl()->SelectItem (pItem->GetTreeItem()); // select the refreenced item
            }
        }
    }    
}

void ecConflictListCtrl::OnLocate(wxCommandEvent& event)
{
    if (m_contextItem > -1)
    {
        ecConfigItem * pItem = AssociatedItem (m_contextItem, m_contextCol); // get the referenced item
        
        if (pItem) { // if a referenced item was found
            wxGetApp().GetTreeCtrl()->SelectItem (pItem->GetTreeItem()); // select the refreenced item
        }
    }
}

void ecConflictListCtrl::OnResolve(wxCommandEvent& event)
{
    ecConfigToolDoc *pDoc = wxGetApp().GetConfigToolDoc();

    wxList conflictsOfInterest;

    long n = GetItemCount();
    long i;
    for (i = 0; i < n; i++)
    {
        if (GetItemState(i, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED)
        {
            conflictsOfInterest.Append((wxObject*) (void*) GetItemData(i));
        }
    }

    pDoc->ResolveGlobalConflicts(& conflictsOfInterest);
}


void ecConflictListCtrl::AddConflict (const CdlConflict &conf)
{
    // set the item column string
    wxString strMacroName = (conf)->get_node ()->get_name ().c_str ();
    int nIndex = InsertItem (GetItemCount (), strMacroName);

    nIndex = GetItemCount() - 1;

    SetItemData (nIndex, (long) (conf));
    
    // set the conflict column string
    if (0 != dynamic_cast<CdlConflict_Unresolved> (conf)) {// a conflict of type 'unresolved'
        SetItem (nIndex, 1, wxT("Unresolved"));
    } else if (0 != dynamic_cast<CdlConflict_IllegalValue> (conf)) { // a conflict of type 'illegal value'
        SetItem (nIndex, 1, wxT("Illegal"));
    } else if (0 != dynamic_cast<CdlConflict_EvalException> (conf)) { // a conflict of type 'evaluation exception'
        SetItem (nIndex, 1, wxT("Exception"));
    } else if (0 != dynamic_cast<CdlConflict_Requires> (conf)) { // a conflict of type 'goal unsatisfied'
        SetItem (nIndex, 1, wxT("Unsatisfied"));
    } else if (0 != dynamic_cast<CdlConflict_Data> (conf)) { // a conflict of type 'bad data'
        SetItem (nIndex, 1, wxT("Bad data"));
    } else {
        wxASSERT (0);
    }
    
    // set the property column string
    wxString strProperty = conf->get_property ()->get_property_name ().c_str ();
    strProperty += wxT(" ");
    const std::vector<std::string> & argv = conf->get_property ()->get_argv ();
    std::vector<std::string>::const_iterator argv_i;
    for (argv_i = argv.begin (); argv_i != argv.end (); argv_i++) {// for each property argument...
        if (argv_i != argv.begin ())                              // ...except the first
        {
            strProperty += argv_i->c_str (); // add the argument to the string
            strProperty += wxT (" "); // separate arguments by a space character
        }
    }
    strProperty.Trim (TRUE); // remove the trailing space character
    SetItem (nIndex, 2, strProperty);
}

void ecConflictListCtrl::AddConflicts (const std::list<CdlConflict>& conflicts)
{
    for (std::list<CdlConflict>::const_iterator conf_i=conflicts.begin (); conf_i != conflicts.end (); conf_i++) {
        AddConflict(*conf_i);
    }
}

ecConfigItem *ecConflictListCtrl::AssociatedItem(int nRow,int nCol)
{
    const CdlConflict conflict = (CdlConflict) GetItemData (nRow);
    wxASSERT (conflict != NULL);
    
    ecConfigItem *pItem=NULL;
    switch(nCol)
    {
    case 2:
        {
            const CdlGoalExpression goal = dynamic_cast<CdlGoalExpression> (conflict->get_property ());
            if (! goal) // if the item is not a goal expression
                break; // do nothing
            const CdlExpression expression = goal->get_expression ();
            if (1 == expression->references.size ()) // if the property contains a single reference
            {
                // assume that the reference is to another user visible node and try to find it
                const wxString strName(expression->references [0].get_destination_name ().c_str());
                pItem = wxGetApp().GetConfigToolDoc ()->Find(strName);
            }
        }
        break;
    case 0:
        pItem = wxGetApp().GetConfigToolDoc ()->Find(wxString(conflict->get_node ()->get_name ().c_str()));
        break;
    default:
        break;
    }
    return pItem;
}

// This is taken from CRulesView::FillRules in the MFC version
void ecConflictListCtrl::FillRules() 
{
    CdlConfiguration CdlConfig = wxGetApp().GetConfigToolDoc()->GetCdlConfig ();
    if (CdlConfig)
    { // if configuration information
        int nCount=0;
        bool bRefill=false;
        wxHashTable arMap(wxKEY_INTEGER);
        int i;
        for ( i = 0 ; i < GetItemCount(); i++)
        {
            arMap.Put(GetItemData(i), (wxObject*) i);
        }
        
        std::list<CdlConflict>::const_iterator conf_i;
        
        const std::list<CdlConflict>& conflicts=CdlConfig->get_all_conflicts();  
        for (conf_i = conflicts.begin (); conf_i != conflicts.end (); conf_i++) { // for each conflict
            nCount++;
            if (!arMap.Get((long) * conf_i))
            {
                bRefill=true;
                break;
            }
        }
        //for (conf_i = CdlConfig->get_structural_conflicts().begin (); conf_i != CdlConfig->get_structural_conflicts().end (); conf_i++) { // for each conflict
        //  nCount++;
        //  if(!arMap.Lookup(*conf_i,w)){
        //    bRefill=true;
        //    break;
        //  }
        //}
        if(bRefill || nCount != GetItemCount())
        {
            DeleteAllItems();
            //m_List.AddConflicts(CdlConfig->get_structural_conflicts());
            AddConflicts(CdlConfig->get_all_conflicts());
        }
    }
}

#if 0
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

#endif
