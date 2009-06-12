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
// conflictsdlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/06
// Version:     $Id: conflictsdlg.cpp,v 1.4 2001/07/09 14:21:32 julians Exp $
// Purpose:
// Description: Implementation file for the ecResolveConflictsDialog
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
#pragma implementation "conflictsdlg.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "configtool.h"
#include "conflictsdlg.h"
#include "conflictwin.h"
#include "solutionswin.h"
#include "configtooldoc.h"
#include "configtoolview.h"
#include "configtree.h"

#include "wx/cshelp.h"

BEGIN_EVENT_TABLE(ecResolveConflictsDialog, ecDialog)
    EVT_BUTTON(ecID_CONFLICTS_ALL, ecResolveConflictsDialog::OnAll)
    EVT_BUTTON(ecID_CONFLICTS_NONE, ecResolveConflictsDialog::OnNone)
    EVT_BUTTON(ecID_CONFLICTS_CONTINUE, ecResolveConflictsDialog::OnContinue)
    EVT_UPDATE_UI(ecID_CONFLICTS_ALL, ecResolveConflictsDialog::OnUpdateAll)
    EVT_UPDATE_UI(ecID_CONFLICTS_NONE, ecResolveConflictsDialog::OnUpdateNone)
    EVT_LIST_ITEM_SELECTED(ecID_CONFLICTS_CONFLICTS, ecResolveConflictsDialog::OnConflictSelected)
    EVT_LIST_ITEM_DESELECTED(ecID_CONFLICTS_CONFLICTS, ecResolveConflictsDialog::OnConflictDeselected)
    EVT_INIT_DIALOG(ecResolveConflictsDialog::OnInitDialog)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// Frame constructor
ecResolveConflictsDialog::ecResolveConflictsDialog(wxWindow* parent, std::list<CdlConflict> conflicts, CdlTransaction transaction, wxList *parConflictsOfInterest):
    m_conflicts(conflicts),
    m_Transaction(transaction),
    m_parConflictsOfInterest(parConflictsOfInterest),
    m_Map(wxKEY_INTEGER)

{
    // Stop values from being changed by other mechanisms during the
    // duration of this dialog.
    wxGetApp().LockValues();

    m_conflictsCtrl = NULL;
    m_solutionsCtrl = NULL;

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    ecDialog::Create(parent, ecID_RESOLVE_CONFLICTS_DIALOG, _("Resolve conflicts"),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    std::list<CdlConflict>::const_iterator conf_i;
    for (conf_i= m_conflicts.begin (); conf_i != m_conflicts.end (); conf_i++)
    { // for each conflict
        int nSolutions = (*conf_i)->get_solution().size();
        SolutionInfo *pInfo = (SolutionInfo *) malloc(sizeof(SolutionInfo)+(nSolutions-1)*sizeof(int));
        pInfo->nCount = nSolutions;
        int i;
        for ( i = 0; i < nSolutions; i++)
        {
            pInfo->arItem[i] = SolutionInfo::CHECKED;
        }
        m_Map.Put((long) *conf_i, (wxObject*) pInfo);
    }

    CreateControls(this);

    Centre(wxBOTH);
}

ecResolveConflictsDialog::~ecResolveConflictsDialog()
{
    m_Map.BeginFind();
    wxNode* node = NULL;
    while ((node = m_Map.Next()))
    {
        SolutionInfo *pInfo = (SolutionInfo*) node->Data();
        free(pInfo);
    }
    m_Map.Clear();

    // OK to change values again
    wxGetApp().UnlockValues();
}

void ecResolveConflictsDialog::CreateControls(wxWindow* parent)
{
    // Create custom windows first
    m_conflictsCtrl = new ecConflictListCtrl(parent, ecID_CONFLICTS_CONFLICTS, wxDefaultPosition, wxSize(470, 110), wxLC_REPORT|wxCLIP_CHILDREN|wxSUNKEN_BORDER);
    m_solutionsCtrl = new ecSolutionListCtrl(parent, ecID_CONFLICTS_SOLUTIONS, wxDefaultPosition, wxSize(470, 110), wxLC_REPORT|wxCLIP_CHILDREN|wxSUNKEN_BORDER);

    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxSizer *item1 = new wxBoxSizer( wxHORIZONTAL );

    wxButton *item2 = new wxButton( parent, ecID_CONFLICTS_CONTINUE, _("&Continue"), wxDefaultPosition, wxDefaultSize, 0 );
    item2->SetDefault();
    item1->Add( item2, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item3 = new wxButton( parent, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item3, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item1, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxWindow *item4 = parent->FindWindow( ecID_CONFLICTS_CONFLICTS );
    wxASSERT( item4 );
    item0->Add( item4, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item5 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText *item6 = new wxStaticText( parent, ecID_CONFLICTS_MSG, _("Proposed Solutions:"), wxDefaultPosition, wxSize(250,-1), 0 );
    item5->Add( item6, 0, wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALL, 0 );

    item5->Add( 30, 20, 1, wxALIGN_CENTRE|wxALL, 0 );

    wxButton *item7 = new wxButton( parent, ecID_CONFLICTS_NONE, _("&None"), wxDefaultPosition, wxDefaultSize, 0 );
    item5->Add( item7, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item8 = new wxButton( parent, ecID_CONFLICTS_ALL, _("&All"), wxDefaultPosition, wxDefaultSize, 0 );
    item5->Add( item8, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item5, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxWindow *item9 = parent->FindWindow( ecID_CONFLICTS_SOLUTIONS );
    wxASSERT( item9 );
    item0->Add( item9, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item1->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );

    // N.B. I find I have to call Layout, then Fit, to make this
    // work.
    parent->Layout();
    item0->Fit( parent );
    item0->SetSizeHints( parent );

    // Add context-sensitive help text
    parent->FindWindow( ecID_CONFLICTS_CONFLICTS )->SetHelpText(_("Displays the set of conflicts for which the fixes are offered."));
    parent->FindWindow( ecID_CONFLICTS_SOLUTIONS )->SetHelpText(_("Displays fixes for the currently selected conflict. Use the checkboxes to enable or disable each fix."));
    parent->FindWindow( ecID_CONFLICTS_CONTINUE )->SetHelpText(_("Continues the current transaction, applying the selected solutions."));
    parent->FindWindow( wxID_CANCEL )->SetHelpText(_("Cancels the current transaction, without applying any solutions."));
    parent->FindWindow( ecID_CONFLICTS_NONE )->SetHelpText(_("Resets all fix checkboxes to the unchecked state."));
    parent->FindWindow( ecID_CONFLICTS_ALL )->SetHelpText(_("Resets all fix checkboxes to the checked state."));

#if __WXGTK__
    parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif
}

void ecResolveConflictsDialog::OnInitDialog(wxInitDialogEvent& event)
{
    wxDialog::OnInitDialog(event);

    // Select the first item and fill the solution set
    m_conflictsCtrl->AddConflicts(m_conflicts);
    
    if (m_parConflictsOfInterest && m_parConflictsOfInterest->Number()>0)
    {
        wxList &arConflictsOfInterest = *m_parConflictsOfInterest;
        int i, j;
        for ( i = m_conflictsCtrl->GetItemCount() - 1; i >= 0; --i )
        {
            for ( j = arConflictsOfInterest.Number() - 1; j>=0; --j )
            {
                CdlConflict conflict = (CdlConflict)m_conflictsCtrl->GetItemData(i);
                if ( ((CdlConflict) arConflictsOfInterest[j]) == conflict )
                {
                    m_conflictsCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                    m_conflictsCtrl->EnsureVisible(i);
                    arConflictsOfInterest.DeleteObject(arConflictsOfInterest[j]);
                    break;
                }
            }
        }
    } else
    {
        for ( int i = m_conflictsCtrl->GetItemCount()-1; i>=0; --i )
        {
            m_conflictsCtrl->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    }
    m_conflictsCtrl->SetFocus();
}

void ecResolveConflictsDialog::OnContinue(wxCommandEvent& event)
{
    // Ensure we have the current conflict check array
    int i;
    for (i = 0; i < m_conflictsCtrl->GetItemCount(); i++)
    {
        if (m_conflictsCtrl->GetItemState(i, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED)
            RemoveConflictSolutions((CdlConflict) m_conflictsCtrl->GetItemData(i));
    }

    // Dismiss the window
    EndModal(wxID_OK);

    std::list<CdlConflict>::const_iterator conf_i;

    for (conf_i= m_conflicts.begin (); conf_i != m_conflicts.end (); conf_i++) // for each conflict
    {
        CdlConflict conflict=*conf_i;
        //int nSolutions=conflict->get_solution().size();
        SolutionInfo &info=Info(conflict);
        int nIndex=0;
        const std::vector<std::pair<CdlValuable, CdlValue> >&Solution=conflict->get_solution();
        for (std::vector<std::pair<CdlValuable, CdlValue> >::const_iterator soln_i = Solution.begin();soln_i != Solution.end(); soln_i++) {
            if(SolutionInfo::CHECKED==info.arItem[nIndex++]){
                CdlValuable valuable  = soln_i->first;
                CdlValue value=soln_i->second;
                CdlValueFlavor flavor = valuable->get_flavor();
                const wxString strName(valuable->get_name().c_str());
                const wxString strValue(value.get_value().c_str());
                bool rc = TRUE;
                wxString str;
                try
                {
                    switch(flavor)
                    {
                    case CdlValueFlavor_None :
                        str = wxT("set CdlValueFlavor_None");
                        rc = FALSE;
                        break;
                    case CdlValueFlavor_Bool :
                        str.Printf(_("%s %s\n"), (const wxChar*) (value.is_enabled()?_("disable"):_("enable")), (const wxChar*) strName);
                        valuable->set_enabled (m_Transaction, value.is_enabled(), CdlValueSource_User);
                        break;
                    case CdlValueFlavor_BoolData :
                        {
                            bool bEnabled=value.is_enabled();
                            str.Printf(_("%s %s and set value to %s\n"), (const wxChar*) (bEnabled? _("disable"):_("enable")), (const wxChar*) strName, (const wxChar*) strValue);
                            // Surely this is wrong - we don't want to set the same value, we want to
                            // set a NEW value.
                            // CdlSimpleValue simple_value = valuable->get_simple_value ();
                            //valuable->set_enabled_and_value (m_Transaction, bEnabled, simple_value, CdlValueSource_User);
                            valuable->set_enabled_and_value (m_Transaction, bEnabled, ecUtils::UnicodeToStdStr (strValue), CdlValueSource_User);
                        }
                        break;
                    case CdlValueFlavor_Data :
                        str.Printf(_("set %s to %s\n"), (const wxChar*) strName, (const wxChar*) strValue);
                        valuable->set_value (m_Transaction, ecUtils::UnicodeToStdStr (strValue), CdlValueSource_User);
                        break;
                    }
                }
                catch(...)
                {
                    rc = FALSE;
                }
                if(rc)
                {
                    wxGetApp().GetConfigToolDoc()->Modify(TRUE);
                } else
                {
                    wxString msg;
                    msg.Printf(_("Failed to %s\n"), (const wxChar*) str);
                    wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
                }
            }
        }
    }
}

void ecResolveConflictsDialog::OnAll(wxCommandEvent& event)
{
    SetAll(TRUE);
}

void ecResolveConflictsDialog::OnNone(wxCommandEvent& event)
{
    SetAll(FALSE);
}

void ecResolveConflictsDialog::SetAll(bool bOnOff)
{
    long i;
    for ( i = m_solutionsCtrl->GetItemCount()-1; i >= 0; --i)
    {
        m_solutionsCtrl->SetChecked(i, bOnOff);
    }
}

// Update All button
void ecResolveConflictsDialog::OnUpdateAll(wxUpdateUIEvent& event)
{
    if (!m_solutionsCtrl)
        return;
    
    int nCheckCount=0;
    int nItemCount = m_solutionsCtrl->GetItemCount();
    int i;
    for (i=nItemCount-1;i>=0;--i)
    {
        nCheckCount+=m_solutionsCtrl->IsChecked(i);
    }
    event.Enable(nItemCount>0 && nCheckCount<nItemCount);
}

// Update None button
void ecResolveConflictsDialog::OnUpdateNone(wxUpdateUIEvent& event)
{
    if (!m_solutionsCtrl)
        return;
    
    int nCheckCount=0;
    int nItemCount=m_solutionsCtrl->GetItemCount();
    int i;
    for (i=nItemCount-1;i>=0;--i){
        nCheckCount+=m_solutionsCtrl->IsChecked(i);
    }
    event.Enable(nItemCount>0 && nCheckCount>0);
}

// Currently there is no 'locate' button so this is not called from anywhere.
// However, the intention in the MFC configtool must have been to use it,
// although it wasn't in the Resolve Conflicts dialog.
void ecResolveConflictsDialog::OnLocate()
{
    ecConfigItem *pItem = m_conflictsCtrl->AssociatedItem(m_nContextItem, m_nContextRow);
    if (pItem) {
        if (wxGetApp().GetTreeCtrl())
            wxGetApp().GetTreeCtrl()->SelectItem(pItem->GetTreeItem());
    }
}

void ecResolveConflictsDialog::RemoveConflictSolutions(CdlConflict conflict)
{
    SolutionInfo &info=Info(conflict);
    int i, j, k;
    for ( i = 0; i < info.nCount; i++ )
    {
        int nItem=info.arItem[i];
        wxASSERT(nItem>=0);
        info.arItem[i] = (m_solutionsCtrl->IsChecked(nItem) ? SolutionInfo::CHECKED:SolutionInfo::UNCHECKED);
        int nRefs = m_solutionsCtrl->GetItemData(nItem);
        if (1 == nRefs)
        {
            m_solutionsCtrl->DeleteItem(nItem);
            for ( k = 0; k < m_conflictsCtrl->GetItemCount(); k++ )
            {
                SolutionInfo &info2 = Info((CdlConflict)m_conflictsCtrl->GetItemData(k));
                for ( j = 0; j < info2.nCount; j++)
                {
                    if (info2.arItem[j] > nItem)
                    {
                        info2.arItem[j] --;
                    }
                }
            }
        } else
        {
            m_solutionsCtrl->SetItemData(nItem, nRefs-1);
        }
    }
}

void ecResolveConflictsDialog::AddConflictSolutions(CdlConflict conflict)
{
    // SolutionInfo allows each conflict to know which solutions have been found for it
    SolutionInfo &info=Info(conflict);
    
    const std::vector<std::pair<CdlValuable, CdlValue> >&Solution=conflict->get_solution();
    
    int i=0;
    for (std::vector<std::pair<CdlValuable, CdlValue> >::const_iterator soln_i = Solution.begin();
         soln_i != Solution.end(); soln_i++)
    {
        CdlValuable valuable = soln_i->first;
        CdlValue value = soln_i->second;
        CdlValueFlavor flavor = valuable->get_flavor();
        
        wxString strValue;
        switch(flavor)
        {
        case CdlValueFlavor_None :
            break;
        case CdlValueFlavor_Bool :
            strValue = value.is_enabled() ? _("Enabled") : _("Disabled");
            break;
        case CdlValueFlavor_BoolData :
            strValue.Printf(wxT("%s, %s"), (const wxChar*) (value.is_enabled() ? _("Enabled") : _("Disabled")), (const wxChar*) value.get_value().c_str());
            break;
        case CdlValueFlavor_Data :
            strValue = value.get_value().c_str();
            break;
        }
        
        const wxString strName(soln_i->first->get_name().c_str());

        long nIndex = m_solutionsCtrl->FindItem(0, strName);
        wxListItem listItem;
        listItem.m_mask = wxLIST_MASK_TEXT;
        listItem.m_itemId = nIndex;
        listItem.m_col = 1;
        if (nIndex != -1)
            m_solutionsCtrl->GetItem(listItem);

        if (-1 == nIndex || strValue != listItem.m_text)
        {
            // We don't have an existing solution that matches this one
            nIndex = m_solutionsCtrl->GetItemCount();
            m_solutionsCtrl->InsertItem(nIndex, strName);
            m_solutionsCtrl->SetItemData(nIndex, 1);
            m_solutionsCtrl->SetItem(nIndex, 1, strValue);

            wxASSERT(info.arItem[i]<0);

            m_solutionsCtrl->SetChecked(nIndex, SolutionInfo::CHECKED==info.arItem[i]);
        } else {
            // We do - to avoid duplicates, increment the "ref count"
            m_solutionsCtrl->SetItemData(nIndex, m_solutionsCtrl->GetItemData(nIndex)+1);
        }
        info.arItem[i++]=nIndex; 

    }
    wxStaticText* staticCtrl = (wxStaticText*) FindWindow(ecID_CONFLICTS_MSG);

    if(0==i){
        staticCtrl->SetLabel(_("No solution is available for this conflict"));
        m_solutionsCtrl->Show(FALSE);
    } else {
        staticCtrl->SetLabel(_("Proposed solution:"));
        m_solutionsCtrl->Show(TRUE);
        // TODO (if necessary)
#if 0
        m_List.SetColumnWidth(0,LVSCW_AUTOSIZE);
        CRect rect;
        m_List.GetClientRect(rect);
        m_List.SetColumnWidth(1,rect.Width()-m_List.GetColumnWidth(0));
#endif
    }
}

ecResolveConflictsDialog::SolutionInfo & ecResolveConflictsDialog::Info(const CdlConflict conflict)
{
  SolutionInfo *pInfo = (SolutionInfo*) m_Map.Get((long) conflict);
  return * pInfo;
}

void ecResolveConflictsDialog::OnConflictSelected(wxListEvent& event) 
{
    CdlConflict conflict=(CdlConflict) m_conflictsCtrl->GetItemData(event.GetIndex());

    if (1 == m_solutionsCtrl->GetSelectedItemCount())
    {
        // TODO ??
        // GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
        m_solutionsCtrl->Show(TRUE);
    }
    AddConflictSolutions(conflict);
}

void ecResolveConflictsDialog::OnConflictDeselected(wxListEvent& event) 
{
    CdlConflict conflict=(CdlConflict) m_conflictsCtrl->GetItemData(event.GetIndex());
    
    RemoveConflictSolutions(conflict);
}


#if 0
// TODO?

// We need to use this because the OnItemChanged handler successive receives "not selected" followed by "selected"
// notifications.  The result is that the "Select one or more conflicts to display available solutions" message
// would be seen briefly when clicking from one selection to another.
BOOL ecResolveConflictsDialog::OnClick(UINT,LPNMLISTVIEW pnmv, LRESULT* pResult) 
{
  if(-1==pnmv->iItem && 0==m_List.GetSelectedCount()){
    SetDlgItemText(IDC_STATIC1,_T("Select one or more conflicts to display available solutions"));
    m_List.ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC1)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_RESET)->EnableWindow(false);
    GetDlgItem(IDC_CONFLICTS_NONE)->EnableWindow(false);
  }
  *pResult = 0;
  return false; // not handled
}

// TODO
BOOL ecResolveConflictsDialog::OnRClick(UINT, LPNMITEMACTIVATE pnmv, LRESULT* pResult) 
{
  DWORD dwPos=GetMessagePos();
  CPoint pt(GET_X_LPARAM(dwPos),GET_Y_LPARAM(dwPos));
  m_nContextItem=pnmv->iItem;
  m_nContextRow=pnmv->iSubItem;
  if(-1!=m_nContextItem){
    //m_RulesList.SetItemState(m_nContextItem,LVIS_SELECTED,LVIS_SELECTED);
  	Menu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(1==m_RulesList.GetSelectedCount() && m_RulesList.AssociatedItem(m_nContextItem,m_nContextRow)?MF_STRING:(MF_STRING|MF_GRAYED),ID_LOCATE,_T("&Locate"));
#ifndef PLUGIN
    SuppressNextContextMenuMessage();
#endif
    menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x,pt.y,this);
  }

  *pResult = 0;
  return TRUE; // handled
}

#endif

