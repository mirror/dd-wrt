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
// configtoolview.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/10/05
// Version:     $Id: configtoolview.cpp,v 1.11 2001/07/13 15:17:43 julians Exp $
// Purpose:
// Description: Implementation file for the ecConfigToolView class
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
    #pragma implementation "configtoolview.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "configtoolview.h"
#include "configtooldoc.h"
#include "configtree.h"
#include "configtool.h"
#include "shortdescrwin.h"
#include "mainwin.h"
#include "propertywin.h"
#include "ecutils.h"

IMPLEMENT_DYNAMIC_CLASS(ecConfigToolView, wxView)

BEGIN_EVENT_TABLE(ecConfigToolView, wxView)
END_EVENT_TABLE()

ecConfigToolView::ecConfigToolView()
{
    m_expandedForFind = wxTreeItemId();
}

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool ecConfigToolView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
    wxGetApp().GetDocManager()->ActivateView(this, TRUE);

    // Single-window mode
#if 0
    m_frame = GetMainFrame();

    // Associate the appropriate frame with this view.
    SetFrame(m_frame);
    
    // Make sure the document manager knows that this is the
    // current view.
    Activate(TRUE);
    
    // Initialize the edit menu Undo and Redo items
    doc->GetCommandProcessor()->SetEditMenu(((pzMainFrame *)m_frame)->editMenu);
    doc->GetCommandProcessor()->Initialize();    
#endif

    return TRUE;
}

void ecConfigToolView::OnDraw(wxDC *dc)
{
}

void ecConfigToolView::OnUpdate(wxView *WXUNUSED(sender), wxObject *hintObj)
{
    ecConfigToolDoc* pDoc = (ecConfigToolDoc*) GetDocument();
    ecConfigTreeCtrl* treeCtrl = wxGetApp().GetTreeCtrl();

    wxASSERT (pDoc);

    ecConfigItem* selItem = NULL;

    wxTreeItemId sel = treeCtrl->GetSelection();
    if (sel.IsOk())
    {
        ecTreeItemData* data = (ecTreeItemData*) treeCtrl->GetItemData(sel);
        if (data)
            selItem = data->GetConfigItem() ;
    }

    ecConfigToolHint* hint = (ecConfigToolHint*) hintObj;
    int hintOp = ecNoHint;
    if (hint)
        hintOp = hint->m_op;

    switch (hintOp)
    {
    case ecSelChanged:
        
        {
            // Note that we're cheating a bit here, since we're using the tree view
            // to update another view, instead of having a view per control as in the MFC
            // version. However, it doesn't seem to be worth the extra machinery.
                    // Update the description window
            if (selItem)
            {
                wxGetApp().GetMainFrame()->GetShortDescriptionWindow()->SetValue(selItem->GetDescription());
                
                // Update the properties window
                wxGetApp().GetMainFrame()->GetPropertyListWindow()->Fill(selItem);
            }
            else
            {
                wxGetApp().GetMainFrame()->GetShortDescriptionWindow()->Clear();
                wxGetApp().GetMainFrame()->GetPropertyListWindow()->ClearAll();
            }

        }
        break;
    case ecAllSaved:
        
        {
            int nItem;
            for(nItem=0;nItem<pDoc->GetItems().Number();nItem++)
            {
                ecConfigItem *pItem = (ecConfigItem*) pDoc->GetItems()[nItem];
                wxTreeItemId treeItem = pItem->GetTreeItem();
                if(treeItem){
                    treeCtrl->SetItemText(treeItem, pItem->GetItemNameOrMacro());
                    //InvalidateItem(h);
                }
            }

            // Update the properties window
            if (selItem)
            {
                wxGetApp().GetMainFrame()->GetPropertyListWindow()->Fill(selItem);
            }

            wxGetApp().GetMainFrame()->UpdateFrameTitle();

            // Update the value pane
            wxGetApp().GetMainFrame()->GetValueWindow()->Refresh();
        }
        break;
    case ecFilenameChanged:
        
        {
            // Update the properties window
            if (selItem)
            {
                wxGetApp().GetMainFrame()->GetPropertyListWindow()->Fill(selItem);
            }
            wxGetApp().GetMainFrame()->UpdateFrameTitle();
        }
        break;
    case ecNameFormatChanged:
        {
            int nItem;
            for(nItem=0;nItem<pDoc->GetItems().Number();nItem++)
            {
                ecConfigItem *pItem = (ecConfigItem*) pDoc->GetItems()[nItem];
                wxString strName(pItem->GetItemNameOrMacro());
                if(pItem->Modified()){
                    strName+=wxT('*');
                }
                treeCtrl->SetItemText(pItem->GetTreeItem(), strName);
            }
            
            treeCtrl->Refresh();
        }
        break;
    case ecIntFormatChanged:
        {
            if (selItem && selItem->GetOptionType() == ecLong)
            {
                wxGetApp().GetMainFrame()->GetPropertyListWindow()->SetItem(ecPropertyListCtrl::ecValue, selItem->StringValue());
                wxGetApp().GetMainFrame()->GetPropertyListWindow()->SetItem(ecPropertyListCtrl::ecDefaultValue, ecUtils::IntToStr(selItem->DefaultValue(), wxGetApp().GetSettings().m_bHex));
            }

        }
        break;
    case ecClear:		
        {
            m_expandedForFind = wxTreeItemId();

            treeCtrl->DeleteAllItems();

            wxGetApp().GetMainFrame()->GetShortDescriptionWindow()->Clear();
            wxGetApp().GetMainFrame()->GetPropertyListWindow()->Fill(NULL);            
            wxGetApp().GetMainFrame()->GetValueWindow()->Refresh();
            break;
        }

    case ecValueChanged:
        {
            ecConfigItem& ti = *(ecConfigItem *)hint->m_item;

            // Update the tree item
            ti.ConvertFromCdl();
            ti.UpdateTreeItem(* treeCtrl);

            // Update the value cell
            ecValueWindow* valueWindow = wxGetApp().GetMainFrame()->GetValueWindow();
            if (valueWindow)
            {
                wxRect rect = valueWindow->GetItemRect(& ti);
#ifdef __WXMSW__
                valueWindow->Refresh(TRUE, & rect);
#else
                // For some reason, on Linux, the value window gets
                // blanked out if a dialog pops up as a result of the value change.
                valueWindow->Refresh();
#endif
            }

            // Properties window
            if (selItem)
            {
                wxGetApp().GetMainFrame()->GetPropertyListWindow()->RefreshValue();
            }
            wxGetApp().GetMainFrame()->UpdateFrameTitle();
        }		        
        break;

    case ecExternallyChanged:
        {
            int nItem;
            for(nItem=0;nItem<pDoc->GetItems().Number();nItem++)
            {
                ecConfigItem *pItem = (ecConfigItem*) pDoc->GetItems()[nItem];
                pItem->UpdateTreeItem(* treeCtrl);
            }
            wxGetApp().GetMainFrame()->UpdateFrameTitle();
        }
        break;
    default:
        break; // not for us, apparently
  }
}

// Clean up windows used for displaying the view.
bool ecConfigToolView::OnClose(bool deleteWindow)
{
    ecConfigToolHint hint(NULL, ecClear);
    GetDocument()->UpdateAllViews (NULL, & hint);

    if (!GetDocument()->Close())
        return FALSE;

    wxGetApp().GetDocManager()->ActivateView(this, FALSE);

    // TODO: Set any pointers to this view to NULL

#if 0
    wxString s(wxTheApp->GetAppName());
    if (m_frame)
        m_frame->SetTitle(s);
    
    SetFrame((wxFrame *) NULL);
#endif
    
    Activate(FALSE);
    
    return TRUE;
}

void ecConfigToolView::OnChangeFilename()
{
    if (wxGetApp().GetTopWindow() && GetDocument())
    {
        wxString docTitle(wxFileNameFromPath(GetDocument()->GetFilename()));
        wxStripExtension(docTitle);
        GetDocument()->SetTitle(docTitle);

        wxString name(GetDocument()->GetFilename());
        wxStripExtension(name);

        ((ecConfigToolDoc*) GetDocument())->SetInstallTree(name + wxT("_install"));
        ((ecConfigToolDoc*) GetDocument())->SetBuildTree(name + wxT("_build"));

        wxString title;

        wxString modifiedMarker;
        if (GetDocument()->IsModified())
            modifiedMarker = wxT("*");

        title = docTitle + modifiedMarker + wxString(wxT(" - ")) + wxGetApp().GetSettings().GetAppName();

        ((wxFrame*) wxGetApp().GetTopWindow())->SetTitle(title);
    }
}

// General disabler
void ecConfigToolView::OnUpdateDisable(wxUpdateUIEvent& event)
{
    event.Enable( FALSE );
}

void ecConfigToolView::Refresh(const wxString& macroName)
{
    ecConfigItem * pItem=wxGetApp().GetConfigToolDoc()->Find(macroName);

    if (pItem) // will be NULL if item has been removed
    {
        pItem->ConvertFromCdl();

        if (wxGetApp().GetTreeCtrl())
            pItem->UpdateTreeItem(* wxGetApp().GetTreeCtrl());

        // Update the cell contents.
        if (wxGetApp().GetMainFrame() && wxGetApp().GetMainFrame()->GetValueWindow())
        {
            wxRect rect = wxGetApp().GetMainFrame()->GetValueWindow()->GetItemRect(pItem);
            wxGetApp().GetMainFrame()->GetValueWindow()->Refresh(TRUE, & rect);
        }
    }
}

void ecConfigToolView::Refresh (wxTreeItemId h)
{
    if(h)
    {
        // TODO Not sure if we need this
#if 0
        AdjustItemImage(h);
        // Invalidate the labels of the affected items
        CRect rect;		
        GetItemRect(h, rect, TRUE );
        rect.left+=m_TreeXOffsetAdjustment;
        InvalidateRect(rect);
        // Do the same for the cell view
        CRect rcBuddyClient;
        CConfigTool::GetCellView()->GetClientRect(rcBuddyClient);
        rect.left=rcBuddyClient.left;
        rect.right=rcBuddyClient.right;
        CConfigTool::GetCellView()->InvalidateRect(rect);
#endif
    }
}

ecConfigItem *ecConfigToolView::DoFind(const wxString& what, wxWindow* parent)
{
    ecConfigToolDoc *pDoc = wxGetApp().GetConfigToolDoc();
    if (!pDoc)
        return NULL;

    int nCount = pDoc->GetItems().Number();

    // static LPCTSTR arWhereImage[]={_T("name"),_T("macro"),_T("description string"),_T("current value"),_T("default value")};
    
    wxString strFind(what);

    if(! wxGetApp().GetSettings().m_findMatchCase)
    {
        strFind.MakeLower();
    }
    
    wxTreeItemId h = wxGetApp().GetTreeCtrl()->GetSelection();

    int nItem;  
    if(!h){
        nItem=0;
    } else {
        for (nItem=nCount-1;nItem>=0;--nItem) {
            if(h==pDoc->GetItem(nItem)->GetTreeItem())
            {
                break;
            }
        }
        wxASSERT(nItem>=0);
    }
    
    ecConfigItem *pItem = NULL;

    int i;
    for (i=0 ; i < nCount ; i++)
    {
        if(wxGetApp().GetSettings().m_findDirection)
        {
            nItem=(nItem+1)%nCount;
        } else {
            nItem=(nCount+nItem-1)%nCount;
        }
        pItem = pDoc->GetItem(nItem);

        ecWhereType where = ecConfigItem::WhereStringToType(wxGetApp().GetSettings().m_findSearchWhat);
        
        wxString strName (pItem->StringValue(where));

        if (!wxGetApp().GetSettings().m_findMatchCase)
        {
            strName.MakeLower();
        }

        int nIndex = strName.Find(strFind);
        if(-1!=nIndex)
        {
            if (wxGetApp().GetSettings().m_findMatchWholeWord)
            {
                // Enforce whole-word semantics: to left and right
                if(nIndex>0 && IsWordChar(strName[(unsigned) (nIndex-1)])){
                    continue;
                }
                nIndex += strFind.Length();
                if (nIndex < strName.Length() && IsWordChar(strName[(unsigned) nIndex])){
                    continue;
                }
            }		
            break;
        }
    }

    if (i < nCount)
    {
        if(m_expandedForFind)
        {
            wxGetApp().GetTreeCtrl()->Collapse(m_expandedForFind);
        }

        wxTreeItemId h=pItem->GetTreeItem();
        // Is h visible?
        wxTreeItemId hv;
        for(hv=wxGetApp().GetTreeCtrl()->GetFirstVisibleItem();hv;hv=wxGetApp().GetTreeCtrl()->GetNextVisible(hv))
        {
            if(hv==h)
            {
                break;
            }
        }
        if (0==hv)
        {
            // we want to record the highest unexpanded item
            for(hv=wxGetApp().GetTreeCtrl()->GetParent(h);hv;hv=wxGetApp().GetTreeCtrl()->GetParent(hv))
            {
                if (!wxGetApp().GetTreeCtrl()->IsExpanded( hv))
                {
                    m_expandedForFind = hv;
                }
            }
        }
        wxGetApp().GetTreeCtrl()->EnsureVisible(h);
        wxGetApp().GetTreeCtrl()->SelectItem(h);
        
    } else
    {
        wxString msg;
        msg.Printf(_("Cannot find '%s' in %s"), (const wxChar*) what, (const wxChar*) wxGetApp().GetSettings().m_findSearchWhat );
        wxMessageBox(msg, _("Find"), wxOK|wxICON_INFORMATION, parent);
    }

    return pItem;
}

bool ecConfigToolView::IsWordChar(wxChar c)
{
  return wxIsalnum(c) || wxT('_')==c;
}


