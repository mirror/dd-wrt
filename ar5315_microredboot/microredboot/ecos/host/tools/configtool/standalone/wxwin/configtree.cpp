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
// Date:        2000/08/24
// Version:     $Id: configtree.cpp,v 1.8 2001/04/24 14:39:13 julians Exp $
// Purpose:
// Description: Implementation file for ecConfigTreeCtrl
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
#pragma implementation "configtree.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/cshelp.h"

// Include XPM icons
#if !defined(__WXMSW__)
#include "bitmaps/closedfolder.xpm"
#include "bitmaps/closedfolder_dis.xpm"
#include "bitmaps/package_closed.xpm"
#include "bitmaps/package_closed_dis.xpm"
#include "bitmaps/package_open.xpm"
#include "bitmaps/package_open_dis.xpm"
#include "bitmaps/checked.xpm"
#include "bitmaps/checked_dis.xpm"
#include "bitmaps/unchecked.xpm"
#include "bitmaps/unchecked_dis.xpm"
#include "bitmaps/integer.xpm"
#include "bitmaps/integer_dis.xpm"
#include "bitmaps/integer2.xpm"
#include "bitmaps/integer2_dis.xpm"
#include "bitmaps/enumerated.xpm"
#include "bitmaps/enumerated_dis.xpm"
#include "bitmaps/radioon.xpm"
#include "bitmaps/radioon_dis.xpm"
#include "bitmaps/radiooff.xpm"
#include "bitmaps/radiooff_dis.xpm"
#include "bitmaps/text.xpm"
#include "bitmaps/text_dis.xpm"
#endif

#include "configtree.h"
#include "configpropdlg.h"
#include "configtooldoc.h"
#include "configtoolview.h"

/*
 * ecConfigTreeCtrl
 */

IMPLEMENT_CLASS(ecConfigTreeCtrl, wxRemotelyScrolledTreeCtrl)

BEGIN_EVENT_TABLE(ecConfigTreeCtrl, wxRemotelyScrolledTreeCtrl)
    EVT_PAINT(ecConfigTreeCtrl::OnPaint)
    EVT_MOUSE_EVENTS(ecConfigTreeCtrl::OnMouseEvent)
    EVT_CHAR(ecConfigTreeCtrl::OnKeyDown)
    EVT_TREE_SEL_CHANGED(-1, ecConfigTreeCtrl::OnSelChanged)
    EVT_TREE_ITEM_EXPANDED(-1, ecConfigTreeCtrl::OnCollapseExpand)
    EVT_TREE_ITEM_COLLAPSED(-1, ecConfigTreeCtrl::OnCollapseExpand)
    EVT_HELP(-1, ecConfigTreeCtrl::OnHelp)
END_EVENT_TABLE()

ecConfigTreeCtrl::ecConfigTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
                                   const wxSize& sz, long style):
wxRemotelyScrolledTreeCtrl(parent, id, pt, sz, style)
{
    LoadIcons();
    
    m_propertiesMenu = new wxMenu;
    
    m_propertiesMenu->Append(ecID_WHATS_THIS, _("&What's This?"));
    m_propertiesMenu->AppendSeparator();
    m_propertiesMenu->Append(ecID_TREE_PROPERTIES, _("P&roperties"));
    m_propertiesMenu->Append(ecID_TREE_RESTORE_DEFAULTS, _("Restore &Defaults"));
    m_propertiesMenu->Append(ecID_TREE_VISIT_DOC, _("Visit Documentation"));
    m_propertiesMenu->Append(ecID_TREE_VIEW_HEADER, _("View Header"));
    m_propertiesMenu->AppendSeparator();
    m_propertiesMenu->Append(ecID_TREE_UNLOAD_PACKAGE, _("&Unload Package..."));

    if (!wxGetApp().GetSettings().GetWindowSettings().GetUseDefaults() &&
         wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Configuration")).Ok())
    {
        SetFont(wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Configuration")));
    }
}

// Load the icons and initialize the tree
void ecConfigTreeCtrl::LoadIcons()
{
    m_imageList = new wxImageList(16, 16, TRUE);
    m_iconDB.SetImageList(m_imageList);
    SetImageList(m_imageList);
    
    // We associate names and states so we can easily get the appropriate
    // icon for a given tree item. This gets used in ecConfigItem::UpdateTreeIcon.
    
    m_iconDB.AddInfo("Container", wxICON(closedfolder), 0, TRUE);
    m_iconDB.AddInfo("Container", wxICON(closedfolder_dis), 0, FALSE);
    
    m_iconDB.AddInfo("Package", wxICON(package_open), 0, TRUE);
    m_iconDB.AddInfo("Package", wxICON(package_open_dis), 0, FALSE);
    
    m_iconDB.AddInfo("Checkbox", wxICON(checked), 0, TRUE);
    m_iconDB.AddInfo("Checkbox", wxICON(checked_dis), 0, FALSE);
    m_iconDB.AddInfo("Checkbox", wxICON(unchecked), 1, TRUE);
    m_iconDB.AddInfo("Checkbox", wxICON(unchecked_dis), 1, FALSE);
    
    m_iconDB.AddInfo("Radiobox", wxICON(radioon), 0, TRUE);
    m_iconDB.AddInfo("Radiobox", wxICON(radioon_dis), 0, FALSE);
    m_iconDB.AddInfo("Radiobox", wxICON(radiooff), 1, TRUE);
    m_iconDB.AddInfo("Radiobox", wxICON(radiooff_dis), 1, FALSE);
    
    m_iconDB.AddInfo("Text", wxICON(text), 0, TRUE);
    m_iconDB.AddInfo("Text", wxICON(text_dis), 0, FALSE);
    
    m_iconDB.AddInfo("Enumerated", wxICON(enumerated), 0, TRUE);
    m_iconDB.AddInfo("Enumerated", wxICON(enumerated_dis), 0, FALSE);
    
    m_iconDB.AddInfo("Integer", wxICON(integer), 0, TRUE);
    m_iconDB.AddInfo("Integer", wxICON(integer_dis), 0, FALSE);
    
#if 0
    // Add some dummy items
    ecConfigItem* item = NULL;
    wxTreeItemId rootId = AddRoot(_(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(NULL, _("Configuration"), ecContainer)));
    
    item->SetTreeItem(rootId);
    item->UpdateTreeItem(* this);
    
    wxTreeItemId childId1 = AppendItem(rootId, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("My Package"), ecPackage, ecFlavorData, ecString)));
    item->SetTreeItem(childId1);
    item->GetValue() = _("v1_4_1");
    item->UpdateTreeItem(* this);
    
    wxTreeItemId childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 1"), ecOption, ecFlavorData, ecString)));
    item->SetTreeItem(childId2);
    item->GetValue() = _("The value for this option");
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 2"), ecOption, ecFlavorData, ecLong)));
    item->SetTreeItem(childId2);
    item->GetValue() = (long) 176343;
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId2, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 3"), ecOption, ecFlavorBool, ecOptionTypeNone, TRUE, TRUE, ecHintCheck)));
    item->SetTreeItem(childId2);
    item->GetValue() = (bool) TRUE;
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 4"), ecOption, ecFlavorBool, ecOptionTypeNone, TRUE, FALSE, ecHintCheck)));
    item->SetTreeItem(childId2);
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 5"), ecOption, ecFlavorBool, ecOptionTypeNone, TRUE, TRUE, ecHintRadio)));
    item->SetTreeItem(childId2);
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 6"), ecOption, ecFlavorBool, ecOptionTypeNone, TRUE, FALSE, ecHintRadio)));
    item->SetTreeItem(childId2);
    item->GetValue() = (bool) TRUE ;
    item->UpdateTreeItem(* this);
    
    // Another branch
    childId1 = AppendItem(rootId, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("My Container"), ecContainer)));
    item->SetTreeItem(childId1);
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 1"), ecOption, ecFlavorData, ecString, FALSE)));
    item->SetTreeItem(childId2);
    item->GetValue() = _("The value for this option");
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 2"), ecOption, ecFlavorData, ecLong, FALSE)));
    item->SetTreeItem(childId2);
    item->GetValue() = (long) 176343;
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 3"), ecOption, ecFlavorBool, ecOptionTypeNone, FALSE, TRUE, ecHintCheck)));
    item->SetTreeItem(childId2);
    item->GetValue() = (bool) TRUE;
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 4"), ecOption, ecFlavorBool, ecOptionTypeNone, FALSE, FALSE, ecHintCheck)));
    item->SetTreeItem(childId2);
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 5"), ecOption, ecFlavorBool, ecOptionTypeNone, FALSE, TRUE, ecHintRadio)));
    item->SetTreeItem(childId2);
    item->UpdateTreeItem(* this);
    
    childId2 = AppendItem(childId1, _(""), -1, -1, new ecTreeItemData(item = new ecConfigItem(item, _("Option 6"), ecOption, ecFlavorBool, ecOptionTypeNone, FALSE, FALSE, ecHintRadio)));
    item->SetTreeItem(childId2);
    item->GetValue() = (bool) TRUE ;
    item->UpdateTreeItem(* this);
    
    Expand(rootId);
#endif
}

ecConfigTreeCtrl::~ecConfigTreeCtrl()
{
    delete m_propertiesMenu;
    
    SetImageList(NULL);
    delete m_imageList;
}

// Draw the lines on top of the tree
void ecConfigTreeCtrl::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    
    wxTreeCtrl::OnPaint(event);
    
    // Reset the device origin since it may have been set
    dc.SetDeviceOrigin(0, 0);
    
    wxSize sz = GetClientSize();
    
    wxPen pen(wxSystemSettings::GetSystemColour(wxSYS_COLOUR_3DLIGHT), 1, wxSOLID);
    dc.SetPen(pen);
    dc.SetBrush(* wxTRANSPARENT_BRUSH);
    
    wxSize clientSize = GetClientSize();
    wxRect itemRect;
    int cy=0;
    wxTreeItemId h, lastH;
    for(h=GetFirstVisibleItem();h;h=GetNextVisible(h))
    {
        if (GetBoundingRect(h, itemRect))
        {
            cy = itemRect.GetTop();
            dc.DrawLine(0, cy, clientSize.x, cy);
            lastH = h;
        }
    }
    
    if (lastH && lastH.IsOk() && GetBoundingRect(lastH, itemRect))
    {
        cy = itemRect.GetBottom();
        dc.DrawLine(0, cy, clientSize.x, cy);
    }
}

void ecConfigTreeCtrl::OnSelChanged(wxTreeEvent& event)
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    if (doc)
    {
        ecConfigToolHint hint(NULL, ecSelChanged);
        doc->UpdateAllViews(NULL, & hint);
    }
}

void ecConfigTreeCtrl::OnMouseEvent(wxMouseEvent& event)
{
    int flags = 0;
    wxTreeItemId item = HitTest(wxPoint(event.GetX(), event.GetY()), flags);
    
    if (item == 0 || !item.IsOk())
    {
        if (event.RightDown())
            PopupMenu(wxGetApp().GetWhatsThisMenu(), event.GetX(), event.GetY());
        return;
    }
    
    if (event.LeftDown())
    {
        if (flags & wxTREE_HITTEST_ONITEMICON)
        {
            ecTreeItemData* data = (ecTreeItemData*) GetItemData(item);
            data->GetConfigItem()->OnIconLeftDown(* this);
        }
    }
    else if (event.RightDown())
    {
        if ((flags & wxTREE_HITTEST_ONITEMBUTTON) ||
            (flags & wxTREE_HITTEST_ONITEMICON) ||
            (flags & wxTREE_HITTEST_ONITEMINDENT) ||
            (flags & wxTREE_HITTEST_ONITEMLABEL))
        {
            SelectItem(item);
            GetPropertiesMenu()->SetClientData((void*) TRUE);
            PopupMenu(GetPropertiesMenu(), event.GetX(), event.GetY());
        }
        else
        {
            wxGetApp().GetWhatsThisMenu()->SetClientData((void*) FALSE);
            PopupMenu(wxGetApp().GetWhatsThisMenu(), event.GetX(), event.GetY());
        }
        return;
    }
    event.Skip();
}

void ecConfigTreeCtrl::OnKeyDown(wxKeyEvent& event)
{
    wxTreeItemId id = GetSelection();
    if (event.GetKeyCode() == WXK_F10 && event.ShiftDown())
    {
        if (id.IsOk())
        {
            wxRect rect;
            if (GetBoundingRect(id, rect))
            {
                GetPropertiesMenu()->SetClientData((void*) TRUE);
                PopupMenu(GetPropertiesMenu(), 100, rect.GetTop() + 4);
            }
        }
    }
    else if (event.GetKeyCode() == '<')
    {
        if (id.IsOk())
        {
            ecConfigItem* item = ((ecTreeItemData*) GetItemData(id))->GetConfigItem();
            item->BumpItem(-1);
        }
    }
    else if (event.GetKeyCode() == '>')
    {
        if (id.IsOk())
        {
            ecConfigItem* item = ((ecTreeItemData*) GetItemData(id))->GetConfigItem();
            item->BumpItem(+1);
        }        
    }
    else if (event.GetKeyCode() == WXK_SPACE)
    {
        if (id.IsOk())
        {
            ecConfigItem* item = ((ecTreeItemData*) GetItemData(id))->GetConfigItem();
            item->BumpItem(0);
        }
    }
    else if (event.GetKeyCode() == WXK_RETURN && event.AltDown())
    {
        if (id.IsOk())
        {
            ecConfigItem* item = ((ecTreeItemData*) GetItemData(id))->GetConfigItem();
            
            ecConfigPropertiesDialog dialog(wxGetApp().GetTopWindow(), item);
            dialog.SetTitle(item->GetName());
            dialog.ShowModal();
        }
    }
    else
    {
        event.Skip();
    }
}

void ecConfigTreeCtrl::OnCollapseExpand(wxTreeEvent& event)
{
    if (GetCompanionWindow())
        GetCompanionWindow()->Refresh();
}

// show help for this window
void ecConfigTreeCtrl::OnHelp(wxHelpEvent& event)
{
    wxPoint pt = ScreenToClient(event.GetPosition());
    int flags = 0;
    wxTreeItemId id = HitTest(pt, flags);
    wxHelpProvider *helpProvider = wxHelpProvider::Get();
    if ( helpProvider && id > 0)
    {
        ecConfigItem* item = ((ecTreeItemData*) GetItemData(id))->GetConfigItem();

        if (item)
        {
            wxGetApp().GetHelpController().DisplayTextPopup(item->GetDescription(), event.GetPosition());
            return;
	}
    }

    event.Skip();
}

/*
* ecValueWindow
*/

IMPLEMENT_CLASS(ecValueWindow, wxTreeCompanionWindow)

BEGIN_EVENT_TABLE(ecValueWindow, wxTreeCompanionWindow)
    EVT_PAINT(ecValueWindow::OnPaint)
    EVT_MOUSE_EVENTS(ecValueWindow::OnMouseEvent)
    EVT_SCROLLWIN(ecValueWindow::OnScroll)
    EVT_TREE_ITEM_EXPANDED(-1, ecValueWindow::OnExpand)
    EVT_TREE_ITEM_COLLAPSED(-1, ecValueWindow::OnExpand)
END_EVENT_TABLE()

ecValueWindow::ecValueWindow(wxWindow* parent, wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& sz,
                             long style):
wxTreeCompanionWindow(parent, id, pos, sz, style)
{
    SetBackgroundColour(wxSystemSettings::GetSystemColour(wxSYS_COLOUR_LISTBOX));

    if (!wxGetApp().GetSettings().GetWindowSettings().GetUseDefaults() &&
         wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Configuration")).Ok())
    {
        SetFont(wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Configuration")));
    }
    else
        SetFont(wxSystemSettings::GetSystemFont(wxSYS_DEFAULT_GUI_FONT));

    
    m_editWindow = NULL;
    m_configItem = NULL;
}

void ecValueWindow::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    
    if (!m_treeCtrl)
        return;
    
    wxPen pen(wxSystemSettings::GetSystemColour(wxSYS_COLOUR_3DLIGHT), 1, wxSOLID);
    dc.SetPen(pen);
    dc.SetBrush(* wxTRANSPARENT_BRUSH);
    wxFont font(GetFont());
    dc.SetFont(font);
    //dc.SetTextForeground(wxSystemSettings::GetSystemColour(wxSYS_COLOUR_WINDOWTEXT));
    dc.SetBackgroundMode(wxTRANSPARENT);
    
    wxSize clientSize = GetClientSize();
    wxRect itemRect;
    int cy=0;
    wxTreeItemId h, lastH;
    for(h=m_treeCtrl->GetFirstVisibleItem();h;h=m_treeCtrl->GetNextVisible(h))
    {
        if (m_treeCtrl->GetBoundingRect(h, itemRect))
        {
            cy = itemRect.GetTop();
            wxRect drawItemRect(0, cy, clientSize.x, itemRect.GetHeight());
            
            lastH = h;
            
            // Draw the actual item
            DrawItem(dc, h, drawItemRect);
            dc.DrawLine(0, cy, clientSize.x, cy);
        }
    }
    if (lastH && lastH.IsOk() && m_treeCtrl->GetBoundingRect(lastH, itemRect))
    {
        cy = itemRect.GetBottom();
        dc.DrawLine(0, cy, clientSize.x, cy);
    }
}

// Returns the rectangle which will enclose the value for this tree item
wxRect ecValueWindow::GetItemRect(ecConfigItem* item)
{
    if (!m_treeCtrl)
        return wxRect(0, 0, 0, 0);
    
    wxSize clientSize = GetClientSize();
    wxRect itemRect;
    int cy=0;
    wxTreeItemId h;
    for(h=m_treeCtrl->GetFirstVisibleItem();h;h=m_treeCtrl->GetNextVisible(h))
    {
        ecTreeItemData* data = (ecTreeItemData*) m_treeCtrl->GetItemData(h);
        if (data->GetConfigItem() == item)
        {
            if (m_treeCtrl->GetBoundingRect(h, itemRect))
            {
                cy = itemRect.GetTop();
                wxRect drawItemRect(0, cy, clientSize.x, itemRect.GetHeight());
                return drawItemRect;
            }
            else
                return wxRect(0, 0, 0, 0);
        }
    }
    return wxRect(0, 0, 0, 0);
}

void ecValueWindow::OnMouseEvent(wxMouseEvent& event)
{
    if (event.LeftDown() && (event.GetX() > 2))
    {
        // Find if this corresponds to a tree item
        int flags = 0;
        wxTreeItemId item = m_treeCtrl->HitTest(wxPoint(4, event.GetY()), flags);
        if (item.IsOk())
        {
            m_treeCtrl->SelectItem(item);
            ecConfigItem* configItem = ((ecTreeItemData*) m_treeCtrl->GetItemData(item))->GetConfigItem();
            if (configItem->CanEdit())
                BeginEditing(configItem);            
        }
    }
    else if (event.RightDown())
    {
        // Find if this corresponds to a tree item
        int flags = 0;
        wxTreeItemId item = m_treeCtrl->HitTest(wxPoint(4, event.GetY()), flags);
        if (item != 0)
        {
            ecConfigItem* configItem = ((ecTreeItemData*) m_treeCtrl->GetItemData(item))->GetConfigItem();
            m_treeCtrl->SelectItem(item);
            ((ecConfigTreeCtrl*) m_treeCtrl)->GetPropertiesMenu()->SetClientData((void*) TRUE);
            PopupMenu(((ecConfigTreeCtrl*) m_treeCtrl)->GetPropertiesMenu(), event.GetX(), event.GetY());
        }
        else
        {
            wxGetApp().GetWhatsThisMenu()->SetClientData((void*) FALSE);
            PopupMenu(wxGetApp().GetWhatsThisMenu(), event.GetX(), event.GetY());
        }
    }
}

void ecValueWindow::DrawItem(wxDC& dc, wxTreeItemId id, const wxRect& rect)
{
    if (m_treeCtrl)
    {
        ecConfigItem* item = ((ecTreeItemData*) m_treeCtrl->GetItemData(id))->GetConfigItem();
        if (!item)
            return;
        wxString text = item->GetDisplayValue();
        
        if (text.IsEmpty())
            return;
        
        static wxColour normalColour = wxSystemSettings::GetSystemColour(wxSYS_COLOUR_WINDOWTEXT);
        static wxColour disabledColour = wxSystemSettings::GetSystemColour(wxSYS_COLOUR_GRAYTEXT);
        dc.SetTextForeground( (item->GetActive() && (item->GetConfigType() != ecPackage)) ? normalColour : disabledColour );
        
        int textW, textH;
        dc.GetTextExtent(text, & textW, & textH);
        
        int x = 2;
        int y = rect.GetY() + wxMax(0, (rect.GetHeight() - textH) / 2);
        
        dc.DrawText(text, x, y);
    }
}

bool ecValueWindow::BeginEditing(ecConfigItem* item)
{
    if (m_configItem)
        EndEditing();
    
    m_configItem = item;
    
    m_editWindow = item->CreateEditWindow(this);
    if (m_editWindow)
    {
        m_editWindow->Show(FALSE);
        
        item->TransferDataToWindow(m_editWindow);
        
        // Position the control
        PositionEditWindow();
        
        m_editWindow->Show(TRUE);
        m_editWindow->SetFocus();
        
        return TRUE;
    }
    else
    {
        m_configItem = NULL;
        return FALSE;
    }
}

// Position the control
void ecValueWindow::PositionEditWindow()
{
    if (!m_configItem || !m_editWindow)
        return;
    
    // Position the control
    wxSize clientSize = GetClientSize();
    wxRect itemRect;
    m_treeCtrl->GetBoundingRect(m_configItem->GetTreeItem(), itemRect);

    wxSize sz = m_editWindow->GetSize();

    // m_editWindow->SetSize(2, itemRect.y+1, clientSize.x, itemRect.GetHeight() /* -2 */);
    m_editWindow->SetSize(0, itemRect.y, clientSize.x, sz.y);
}

bool ecValueWindow::EndEditing()
{
    if (m_configItem)
    {
        if (m_editWindow && !wxGetApp().GetValuesLocked())
            m_configItem->TransferDataFromWindow(m_editWindow);
        m_configItem = NULL;
    }
    
    if (m_editWindow)
    {
        delete m_editWindow;
        m_editWindow = NULL;
    }
    
    return TRUE;
}

void ecValueWindow::OnScroll(wxScrollWinEvent& event)
{
    wxTreeCompanionWindow::OnScroll(event);
    
    PositionEditWindow();
}

void ecValueWindow::OnExpand(wxTreeEvent& event)
{
    wxTreeCompanionWindow::OnExpand(event);
    
    EndEditing();
}

/*
* wxIconStateInfo
*/

wxIconStateInfo::wxIconStateInfo(const wxString& name)
{
    m_maxStates = 0;
    m_name = name;
    int i;
    for (i = 0; i < wxMAX_ICON_STATES; i++)
        m_states[i] = 0;
}

int wxIconStateInfo::GetIconId(int state, bool enabled) const
{
    wxASSERT ( state < (wxMAX_ICON_STATES * 2) );
    wxASSERT ( state < m_maxStates );
    
    return m_states[state * 2 + (enabled ? 0 : 1)];
}

void wxIconStateInfo::SetIconId(int state, bool enabled, int iconId)
{
    wxASSERT ( state < (wxMAX_ICON_STATES * 2) );
    if (state+1 > m_maxStates)
        m_maxStates = state+1;
    
    m_states[state * 2 + (enabled ? 0 : 1)] = iconId;
}

/*
* wxIconStateInfoDb
* Contains a list of wxIconStateInfos
*/

wxIconStateInfoDB::wxIconStateInfoDB(wxImageList* imageList)
{
    m_imageList = imageList;
    DeleteContents(TRUE);
}

void wxIconStateInfoDB::AppendInfo(wxIconStateInfo* info)
{
    Append(info);
}

// Easy way of initialising both the image list and the
// info db. It will generate image ids itself while appending the icon.
bool wxIconStateInfoDB::AddInfo(const wxString& name, const wxIcon& icon, int state, bool enabled)
{
    wxASSERT (m_imageList != NULL);
    
    wxIconStateInfo* info = FindInfo(name);
    if (!info)
    {
        info = new wxIconStateInfo(name);
        Append(info);
    }
    info->SetIconId(state, enabled, m_imageList->Add(icon));
    return TRUE;
}

wxIconStateInfo* wxIconStateInfoDB::FindInfo(const wxString& name) const
{
    wxNode* node = First();
    while (node)
    {
        wxIconStateInfo* info = (wxIconStateInfo*) node->Data();
        if (info->GetName() == name)
            return info;
        node = node->Next();
    }
    return NULL;
}

int wxIconStateInfoDB::GetIconId(const wxString& name, int state, bool enabled) const
{
    wxIconStateInfo* info = FindInfo(name);
    if (!info)
        return -1;
    return info->GetIconId(state, enabled);
}

bool wxIconStateInfoDB::SetIconId(const wxString& name, int state, bool enabled, int iconId)
{
    wxIconStateInfo* info = FindInfo(name);
    if (!info)
        return FALSE;
    info->SetIconId(state, enabled, iconId);
    return TRUE;
}

#if 0
wxIcon wxIconStateInfoDB::GetIcon(const wxString& name, int state, bool enabled) const
{
    wxASSERT( m_imageList != NULL );
    
    wxIconStateInfo* info = FindInfo(name);
    if (!info)
        return wxNullIcon;
    int id = info->GetIconId(state, enabled);
    if (id < 0)
        return wxNullIcon;
    else
        return m_imageList->GetImage(id); // Doesn't exist
}
#endif

/*
* ecSplitterScrolledWindow
* This window holds the tree and value windows.
* We derive a new class mainly to intercept popup menu commands from both child windows
* without resorting to placing their handlers in the main frame.
*/

BEGIN_EVENT_TABLE(ecSplitterScrolledWindow, wxSplitterScrolledWindow)
    EVT_MENU(ecID_WHATS_THIS, ecSplitterScrolledWindow::OnWhatsThis)
    EVT_MENU(ecID_TREE_PROPERTIES, ecSplitterScrolledWindow::OnProperties)
    EVT_MENU(ecID_TREE_RESTORE_DEFAULTS, ecSplitterScrolledWindow::OnRestoreDefaults)
    EVT_MENU(ecID_TREE_VISIT_DOC, ecSplitterScrolledWindow::OnVisitDoc)
    EVT_MENU(ecID_TREE_VIEW_HEADER, ecSplitterScrolledWindow::OnViewHeader)
    EVT_MENU(ecID_TREE_UNLOAD_PACKAGE, ecSplitterScrolledWindow::OnUnloadPackage)

    EVT_UPDATE_UI(ecID_TREE_RESTORE_DEFAULTS, ecSplitterScrolledWindow::OnUpdateRestoreDefaults)
    EVT_UPDATE_UI(ecID_TREE_VISIT_DOC, ecSplitterScrolledWindow::OnUpdateVisitDoc)
    EVT_UPDATE_UI(ecID_TREE_VIEW_HEADER, ecSplitterScrolledWindow::OnUpdateViewHeader)
    EVT_UPDATE_UI(ecID_TREE_UNLOAD_PACKAGE, ecSplitterScrolledWindow::OnUpdateUnloadPackage)
END_EVENT_TABLE()

static bool ecIsMenuForItem(wxCommandEvent& event)
{
    wxObject* obj = event.GetEventObject();
    wxMenu* menu = NULL;

    if (obj->IsKindOf(CLASSINFO(wxMenu)))
    {
        // Menu has some client data which tells us if the menu was clicked over an item
        // or not
        menu = (wxMenu*) obj;
        return (bool) (menu->GetClientData() != 0) ;
    }
    else
        return FALSE;
}

void ecSplitterScrolledWindow::OnWhatsThis(wxCommandEvent& event)
{
    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;

    wxTreeItemId id = treeCtrl->GetSelection();
    if (id != 0 && ecIsMenuForItem(event))
    {
        ecConfigItem* item = ((ecTreeItemData*) treeCtrl->GetItemData(id))->GetConfigItem();
        
        wxPoint pt = wxGetMousePosition();
        wxString msg;
        msg.Printf("Popping at %d x %d", pt.x, pt.y);
        wxLogDebug(msg);

        wxGetApp().GetHelpController().DisplayTextPopup(item->GetDescription(), wxGetMousePosition());
    }
    else
        event.Skip();
}

void ecSplitterScrolledWindow::OnProperties(wxCommandEvent& event)
{
    wxObject* obj = event.GetEventObject();
    wxMenu* menu = NULL;

    if (obj->IsKindOf(CLASSINFO(wxMenu)))
    {
        // Menu has some client data which tells us if the menu was clicked over an item
        // or not
        menu = (wxMenu*) obj;
    }

    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;
    
    wxTreeItemId id = treeCtrl->GetSelection();
    if (id != 0 && ecIsMenuForItem(event))
    {
        ecConfigItem* item = ((ecTreeItemData*) treeCtrl->GetItemData(id))->GetConfigItem();
        
        ecConfigPropertiesDialog dialog(wxGetApp().GetTopWindow(), item);
        dialog.SetTitle(item->GetName());
        dialog.ShowModal();
    }
}

void ecSplitterScrolledWindow::OnRestoreDefaults(wxCommandEvent& event)
{
    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;
    
    wxTreeItemId id = treeCtrl->GetSelection();
    if (id != 0 && ecIsMenuForItem(event))
    {
        ecConfigItem* item = ((ecTreeItemData*) treeCtrl->GetItemData(id))->GetConfigItem();
        if (item->HasModifiedChildren())
        {
            int ans = wxMessageBox(_("Restore defaults for nested items?"), wxGetApp().GetSettings().GetAppName(), wxYES_NO|wxCANCEL|wxICON_QUESTION);
            switch (ans)
            {
            case wxYES:
                RestoreDefault(id, TRUE);
                break;
            case wxNO:
                RestoreDefault(id, FALSE);
                break;
            case wxCANCEL:
                break;
            default:
                wxASSERT(FALSE);
                break;
            }
        }
        else
        {
            RestoreDefault(id, FALSE);
        }
        // current values may have changed so refresh the other views
        wxGetApp().GetConfigToolDoc ()->UpdateFailingRuleCount ();
        
        if (item->GetOptionType () != ecOptionTypeNone)
        {
            ecConfigToolHint hint(item, ecValueChanged);
            wxGetApp().GetConfigToolDoc ()->UpdateAllViews (NULL, & hint);
        }
    }
}

void ecSplitterScrolledWindow::RestoreDefault(wxTreeItemId h, bool bRecurse /* = FALSE */, bool bTopLevel /* = TRUE */)
{
    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;

    ecConfigItem* ti = ((ecTreeItemData*) treeCtrl->GetItemData(h))->GetConfigItem();

    const CdlValuable valuable = ti->GetCdlValuable();

    if (valuable && (CdlValueFlavor_None != valuable->get_flavor ())) // skip the root node and nodes without a value
        valuable->set_source (CdlValueSource_Default);
    treeCtrl->SetItemText (h, ti->GetItemNameOrMacro ()); // remove asterisk in control view
    
    if (bTopLevel && ti->HasRadio ()) // if user-specified item is a radio button
    {
        for (ecConfigItem * pItem = ti->FirstRadio (); pItem; pItem = pItem->NextRadio ())
        {
            if (ti != pItem)
            {
                const CdlValuable valuable = pItem->GetCdlValuable();
                wxASSERT (valuable);
                valuable->set_source (CdlValueSource_Default); // restore default for each sibling radio button
                treeCtrl->SetItemText (pItem->GetTreeItem(), pItem->GetItemNameOrMacro ()); // remove asterisk in control view
            }
            
        }
    }
    
    if (bRecurse)
    {
        long cookie;
        for (h = treeCtrl->GetFirstChild(h, cookie); h; h = treeCtrl->GetNextSibling(h))
        {
            RestoreDefault (h, TRUE, FALSE);
        }
    }
}

void ecSplitterScrolledWindow::OnVisitDoc(wxCommandEvent& event)
{
    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;

    wxTreeItemId id = treeCtrl->GetSelection();
    if (id != 0 && ecIsMenuForItem(event))
    {
        ecConfigItem* item = ((ecTreeItemData*) treeCtrl->GetItemData(id))->GetConfigItem();
        
        item->ViewURL();
    }
    else
        event.Skip();
}

void ecSplitterScrolledWindow::OnViewHeader(wxCommandEvent& event)
{
    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;

    wxTreeItemId id = treeCtrl->GetSelection();
    if (id != 0 && ecIsMenuForItem(event))
    {
        ecConfigItem* item = ((ecTreeItemData*) treeCtrl->GetItemData(id))->GetConfigItem();
        
        item->ViewHeader();
    }
    else
        event.Skip();
}

void ecSplitterScrolledWindow::OnUnloadPackage(wxCommandEvent& event)
{
    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;

    wxTreeItemId id = treeCtrl->GetSelection();
    if (id != 0)
    {
        ecConfigItem* item = ((ecTreeItemData*) treeCtrl->GetItemData(id))->GetConfigItem();
        
        if (wxYES == wxMessageBox(_("Are you sure you wish to unload this package?"), wxGetApp().GetSettings().GetAppName(),
                wxYES_NO|wxICON_QUESTION))
        {
            item->Unload();
            wxGetApp().GetConfigToolDoc()->RegenerateData();
        }
    }
}

void ecSplitterScrolledWindow::OnUpdateRestoreDefaults(wxUpdateUIEvent& event)
{
    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;

    wxTreeItemId id = treeCtrl->GetSelection();
    if (id != 0)
    {
        ecConfigItem* item = ((ecTreeItemData*) treeCtrl->GetItemData(id))->GetConfigItem();
        
        event.Enable( IsChanged(id, TRUE ) );
    }
}

void ecSplitterScrolledWindow::OnUpdateVisitDoc(wxUpdateUIEvent& event)
{
    event.Enable( TRUE );
}

void ecSplitterScrolledWindow::OnUpdateViewHeader(wxUpdateUIEvent& event)
{
    ecConfigToolDoc *pDoc=wxGetApp().GetConfigToolDoc();
    event.Enable( pDoc && !pDoc->GetBuildTree().IsEmpty() ) ;
}

void ecSplitterScrolledWindow::OnUpdateUnloadPackage(wxUpdateUIEvent& event)
{
    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;

    wxTreeItemId id = treeCtrl->GetSelection();
    if (id != 0)
    {
        ecConfigItem* item = ((ecTreeItemData*) treeCtrl->GetItemData(id))->GetConfigItem();
        
        event.Enable( item->IsPackage() );
    }
}

bool ecSplitterScrolledWindow::IsChanged(wxTreeItemId id, bool bRecurse)
{
    ecConfigTreeCtrl* treeCtrl = (ecConfigTreeCtrl*) FindWindow(ecID_TREE_CTRL);
    wxASSERT (treeCtrl != NULL) ;

    ecConfigItem* item = ((ecTreeItemData*) treeCtrl->GetItemData(id))->GetConfigItem();
    bool rc = item->Modified ();

    if(!rc && bRecurse)
    {
        long cookie;
        for (id=treeCtrl->GetFirstChild(id, cookie);id;id=treeCtrl->GetNextSibling(id))
        {
            if (IsChanged(id,TRUE))
            {
                rc=TRUE;
                break;
            }
        }
    }
    return rc;
}
