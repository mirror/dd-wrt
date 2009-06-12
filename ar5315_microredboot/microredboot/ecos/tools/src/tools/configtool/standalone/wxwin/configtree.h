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
// configtree.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/08/24
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/tools/src/tools/configtool/standalone/wxwin/configtree.h#3 $
// Purpose:
// Description: Header file for ecConfigTreeCtrl
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_CONFIGTREE_H_
#define _ECOS_CONFIGTREE_H_

#ifdef __GNUG__
#pragma interface "configtree.h"
#endif

#include "wx/wx.h"
#include "splittree.h"
#include "configitem.h"

#define wxMAX_ICON_STATES 4

/*

The idea behind wxIconStateInfo, wxIconStateInfoDB
is to associate multiple state icons with items in tree controls
(and potentially other controls).

So instead of having to remember a lot of image list ids,
you have a named state info object which contains up to 4 different states
(identified by the integers 0 - 3). Each of these states can
be in a further 2 sub-states - enabled or disabled.

wxIconStateDB holds a list of these state info objects
and has a convenient API. For example, the following adds
icons for a checkbox item that can be: on/enabled, off/enabled,
on/disabled,off/disabled.

    m_iconDB.AddInfo("Checkbox", wxICON(checked), 0, TRUE);
    m_iconDB.AddInfo("Checkbox", wxICON(checked_dis), 0, FALSE);
    m_iconDB.AddInfo("Checkbox", wxICON(unchecked), 1, TRUE);
    m_iconDB.AddInfo("Checkbox", wxICON(unchecked_dis), 1, FALSE);

When you update the item image in response to (e.g.) user interaction,
you can say something like this:

    int iconId = m_iconDB.GetIconId("Checkbox", 0, FALSE);

    treeCtrl.SetItemImage(itemId, iconId, wxTreeItemIcon_Normal);
    treeCtrl.SetItemImage(itemId, iconId, wxTreeItemIcon_Selected);

See configitem.cpp for further examples.

These two classes are prefixed 'wx' because they are generic and may
be used elsewhere.

 */

/*
 * wxIconStateInfo
 * Stores information about the visual state of an item in a tree control
 */

class wxIconStateInfo: public wxObject
{
public:
    wxIconStateInfo(const wxString& name);
    
    // How many states? (Each state
    //  has enabled/disabled state)
    // Max (say) 4 states, each with
    // enabled/disabled
    int GetStateCount() const { return m_maxStates; };

    void SetStateCount(int count) { m_maxStates; }
    int GetIconId(int state, bool enabled = TRUE) const;
    void SetIconId(int state, bool enabled, int iconId);

    const wxString& GetName() const { return m_name; }
    
protected:
    int             m_maxStates;
    int             m_states[wxMAX_ICON_STATES * 2]; // Enabled/disabled
    wxString        m_name; // Name of icon, e.g. "Package"
};

/*
 * wxIconStateInfoDb
 * Contains a list of wxIconStateInfos
 */

class wxIconStateInfoDB: public wxList
{
public:
    wxIconStateInfoDB(wxImageList* imageList = NULL);
    
    void AppendInfo(wxIconStateInfo* info);
    
    // Easy way of initialising both the image list and the
    // info db. It will generate image ids itself while appending the icon.
    // 'state' is an integer from 0 up to the max allowed, representing a different
    // state. There may be only one, or (for a checkbox) there may be two.
    // A folder that can be open or closed would have two states.
    // Enabled/disabled is taken as a special case.
    bool AddInfo(const wxString& name, const wxIcon& icon, int state, bool enabled);
    
    wxIconStateInfo* FindInfo(const wxString& name) const;
    
    int GetIconId(const wxString& name, int state, bool enabled = TRUE) const;
    bool SetIconId(const wxString& name, int state, bool enabled, int iconId) ;
    
    void SetImageList(wxImageList* imageList) { m_imageList = imageList; }
    wxImageList* GetImageList() const { return m_imageList; }
    
protected:
    wxImageList*    m_imageList;    
};

/*
 * ecTreeItemData
 * This holds an association between the tree control item
 * and the ecConfigItem.
 */

class ecConfigItem;
class ecTreeItemData : public wxTreeItemData
{
public:
    ecTreeItemData(ecConfigItem* item) : m_configItem(item) { }
    ~ecTreeItemData() { if (m_configItem) delete m_configItem; }

    ecConfigItem *GetConfigItem() const { return m_configItem; }
    void SetConfigItem(ecConfigItem *item) { m_configItem = item; }

private:
    ecConfigItem*   m_configItem;
};


/*
 * ecConfigTreeCtrl
 * This control represents the configuration hierarchy.
 * It's derived from wxRemotelyScrolledTreeCtrl because we want
 * to synchronize the tree scrolling with the value window scrolling,
 * and have the scrollbar on the right-hand-side of the value window
 * (not the tree control).
 */

class ecValueWindow;
class ecConfigTreeCtrl: public wxRemotelyScrolledTreeCtrl
{
    DECLARE_CLASS(ecConfigTreeCtrl)
public:
    ecConfigTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt = wxDefaultPosition,
        const wxSize& sz = wxDefaultSize, long style = wxTR_HAS_BUTTONS);
    ~ecConfigTreeCtrl();

//// Event handlers    
    void OnPaint(wxPaintEvent& event);
    void OnMouseEvent(wxMouseEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void OnCollapseExpand(wxTreeEvent& event);
    void OnHelp(wxHelpEvent& event);
    void OnKeyDown(wxKeyEvent& event);

//// Accessors
    wxIconStateInfoDB& GetIconDB() { return m_iconDB; }
    wxMenu* GetPropertiesMenu() const { return m_propertiesMenu; }

//// Operations
    void LoadIcons();

protected:
    wxImageList*        m_imageList;
    wxIconStateInfoDB   m_iconDB;
    wxMenu*             m_propertiesMenu;

    DECLARE_EVENT_TABLE()
};

/*
 * ecValueWindow
 * This window represents the values associated with the configuration tree.
 * It scrolls synchronously with the tree control.
 */

class ecValueWindow: public wxTreeCompanionWindow
{
public:
    ecValueWindow(wxWindow* parent, wxWindowID id = -1,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& sz = wxDefaultSize,
        long style = 0);
    
    //// Overrides
    virtual void DrawItem(wxDC& dc, wxTreeItemId id, const wxRect& rect);
    
    //// Events
    void OnPaint(wxPaintEvent& event);    
    void OnMouseEvent(wxMouseEvent& event);
    void OnScroll(wxScrollWinEvent& event);
    void OnExpand(wxTreeEvent& event);

    //// Accessors
    wxWindow* GetEditWindow() const { return m_editWindow; }

    //// Operations

    bool BeginEditing(ecConfigItem* item);
    bool EndEditing();
    void PositionEditWindow();
    wxRect GetItemRect(ecConfigItem* item);

    ecConfigItem*   GetCurrentConfigItem() const { return m_configItem; }
    
    //// Data members
protected:
    wxWindow*       m_editWindow; // Edit control
    ecConfigItem*   m_configItem; // Item being edited
    
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS(ecValueWindow)
};

/*
 * ecSplitterScrolledWindow
 * This window holds the tree and value windows.
 * We derive a new class mainly to intercept popup menu commands from both child windows
 * without resorting to placing their handlers in the main frame.
 */

class ecSplitterScrolledWindow: public wxSplitterScrolledWindow
{
public:
    ecSplitterScrolledWindow(wxWindow* parent, wxWindowID id, const wxPoint& pt = wxDefaultPosition,
        const wxSize& sz = wxDefaultSize, long style = wxVSCROLL):
        wxSplitterScrolledWindow(parent, id, pt, sz, style)
    {
    }

//// Event handlers
    void OnWhatsThis(wxCommandEvent& event);
    void OnProperties(wxCommandEvent& event);
    void OnRestoreDefaults(wxCommandEvent& event);
    void OnVisitDoc(wxCommandEvent& event);
    void OnViewHeader(wxCommandEvent& event);
    void OnUnloadPackage(wxCommandEvent& event);

    void OnUpdateRestoreDefaults(wxUpdateUIEvent& event);
    void OnUpdateVisitDoc(wxUpdateUIEvent& event);
    void OnUpdateViewHeader(wxUpdateUIEvent& event);
    void OnUpdateUnloadPackage(wxUpdateUIEvent& event);

//// Helpers
    void RestoreDefault(wxTreeItemId h, bool bRecurse = FALSE, bool bTopLevel = TRUE);
    bool IsChanged(wxTreeItemId h, bool bRecurse);
protected:
    DECLARE_EVENT_TABLE()
};


#endif
// _ECOS_CONFIGTREE_H_
