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
// mainwin.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/08/24
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/mainwin.h#3 $
// Purpose:
// Description: Header file for the ConfigTool main window
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_MAINWIN_H_
#define _ECOS_MAINWIN_H_

#ifdef __GNUG__
#pragma interface "mainwin.h"
#endif

#include "wx/wx.h"
#include "wx/docview.h"

#include "splittree.h"

/*
 * Status bar panes
 */

#define ecStatusPane            0
#define ecFailRulePane          3

class ecValueWindow;
class ecOutputWindow;
class ecConfigTreeCtrl;
class ecShortDescriptionWindow;
class ecPropertyListCtrl;
class ecConflictListCtrl;
class ecFindDialog;
class WXDLLEXPORT wxSashLayoutWindow;

// Define a new frame type: this is going to be our main frame
class ecMainFrame : public wxDocParentFrame
{
    friend class ecFindDialog;

public:
// Ctor(s)
    ecMainFrame(wxDocManager *manager, const wxString& title, const wxPoint& pos, const wxSize& size);

//// Event handlers

    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnFind(wxCommandEvent& event);
    void OnFindNext(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnIdle(wxIdleEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
    void OnHelpEcos(wxCommandEvent& event);
    void OnHelpConfigtool(wxCommandEvent& event);
    void OnHelpContext(wxCommandEvent& event);
    void OnResolveConflicts(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnPlatforms(wxCommandEvent& event);
    void OnBuildOptions(wxCommandEvent& event);
    void OnTemplates(wxCommandEvent& event);
    void OnAdmin(wxCommandEvent& event);
    void OnPackages(wxCommandEvent& event);
    void OnRunTests(wxCommandEvent& event);
    void OnChooseRepository(wxCommandEvent& event);
    void OnBuildToolsPath(wxCommandEvent& event);
    void OnUserToolsPath(wxCommandEvent& event);
    void OnWhatsThis(wxCommandEvent& event);
    void OnSaveOutput(wxCommandEvent& event);
    void OnImport(wxCommandEvent& event);
    void OnExport(wxCommandEvent& event);
    void OnBuildLibrary(wxCommandEvent& event);
    void OnBuildTests(wxCommandEvent& event);
    void OnStopBuild(wxCommandEvent& event);
    void OnClean(wxCommandEvent& event);
    void OnShell(wxCommandEvent& event);
    void OnGenerateBuildTree(wxCommandEvent& event);

    void OnNewRegion(wxCommandEvent& event);
    void OnNewSection(wxCommandEvent& event);
    void OnDeleteRegionOrSection(wxCommandEvent& event);
    void OnRegionOrSectionProperties(wxCommandEvent& event);

    void OnWebRedHatHome(wxCommandEvent& event);
    void OnWebEcos(wxCommandEvent& event);
    void OnWebNetRelease(wxCommandEvent& event);
    void OnWebUitron(wxCommandEvent& event);
    void OnRepositoryInfo(wxCommandEvent& event);
    void OnIndexDocs(wxCommandEvent& event);

    void OnUpdatePlatforms(wxUpdateUIEvent& event);
    void OnUpdateBuildOptions(wxUpdateUIEvent& event);
    void OnUpdateBuildToolsPath(wxUpdateUIEvent& event);
    void OnUpdateUserToolsPath(wxUpdateUIEvent& event);
    void OnUpdateTemplates(wxUpdateUIEvent& event);
    void OnUpdateAdmin(wxUpdateUIEvent& event);
    void OnUpdatePackages(wxUpdateUIEvent& event);
    void OnUpdateRunTests(wxUpdateUIEvent& event);
    void OnUpdateChooseRepository(wxUpdateUIEvent& event);
    void OnUpdateResolveConflicts(wxUpdateUIEvent& event);
    void OnUpdateSelectAll(wxUpdateUIEvent& event);
    void OnUpdateClear(wxUpdateUIEvent& event);
    void OnUpdateImport(wxUpdateUIEvent& event);
    void OnUpdateExport(wxUpdateUIEvent& event);
    void OnUpdateBuildLibrary(wxUpdateUIEvent& event);
    void OnUpdateBuildTests(wxUpdateUIEvent& event);
    void OnUpdateStopBuild(wxUpdateUIEvent& event);
    void OnUpdateClean(wxUpdateUIEvent& event);
    void OnUpdateRepositoryInfo(wxUpdateUIEvent& event);
    void OnUpdateShell(wxUpdateUIEvent& event);
    void OnUpdateFind(wxUpdateUIEvent& event);
    void OnUpdateFindNext(wxUpdateUIEvent& event);
    void OnUpdateIndexDocs(wxUpdateUIEvent& event);
    void OnUpdateGenerateBuildTree(wxUpdateUIEvent& event);

    void OnUpdateNewRegion(wxUpdateUIEvent& event);
    void OnUpdateNewSection(wxUpdateUIEvent& event);
    void OnUpdateDeleteRegionOrSection(wxUpdateUIEvent& event);
    void OnUpdateRegionOrSectionProperties(wxUpdateUIEvent& event);

  // Toggle windows
    void OnToggleWindow(wxCommandEvent& event);
    void OnUpdateToggleWindow(wxUpdateUIEvent& event);
    void OnToggleToolbar(wxCommandEvent& event);
    void OnUpdateToggleToolbar(wxUpdateUIEvent& event);
    void OnUpdateDisable(wxUpdateUIEvent& event);

//// Operations
	// (Re)create the toolbar
	void RecreateToolbar();

    // Create the windows
    void CreateWindows();

    // Respond to a sash drag operation
    void OnSashDrag(wxSashEvent& event);

    // Enumerate the visible 'minor' sash windows,
    // i.e. those in the top-right segment of the frame
    void GetMinorWindows(wxList& list);

    // Get all visible sash windows
    void GetSashWindows(wxList& list);

    // Divide the given space evenly amongst some windows
    void DivideSpaceEvenly(wxList& list, const wxSize& space, int orient);

    // Restore the sash window default size from the actual window size
    void RestoreDefaultWindowSizes(wxList& list);

    void SetFailRulePane(int nCount);

    // Toggle the given window on or off
    void ToggleWindow(int windowId);

    // Update the title, either via the document's view or explicitly if no doc
    void UpdateFrameTitle();

//// Accessors
    ecOutputWindow* GetOutputWindow() const { return m_outputWindow; }
    ecConfigTreeCtrl* GetTreeCtrl() const { return m_tree; }
    ecValueWindow* GetValueWindow() const { return m_valueWindow; }
    ecShortDescriptionWindow* GetShortDescriptionWindow() const { return m_shortDescrWindow; }
    ecPropertyListCtrl* GetPropertyListWindow() const { return m_propertyListWindow; }
    ecConflictListCtrl* GetConflictsWindow() const { return m_conflictsWindow; }
    ecMemoryLayoutWindow* GetMemoryLayoutWindow() const { return m_mltWindow; }

//// Overrides

    virtual bool ProcessEvent(wxEvent& event);

protected:
    ecConfigTreeCtrl*               m_tree;
    wxThinSplitterWindow*           m_splitter;
    wxSplitterScrolledWindow*       m_scrolledWindow;
    ecValueWindow*                  m_valueWindow;
    ecOutputWindow*                 m_outputWindow;
    ecShortDescriptionWindow*       m_shortDescrWindow;
    ecPropertyListCtrl*             m_propertyListWindow;
    ecConflictListCtrl*             m_conflictsWindow;
    ecFindDialog*                   m_findDialog;
    ecMemoryLayoutWindow*           m_mltWindow;

    // Sash layout windows, that contain the 'real' windows
    wxSashLayoutWindow*             m_configSashWindow;
    wxSashLayoutWindow*             m_conflictsSashWindow;
    wxSashLayoutWindow*             m_propertiesSashWindow;
    wxSashLayoutWindow*             m_memorySashWindow;
    wxSashLayoutWindow*             m_shortDescrSashWindow;
    wxSashLayoutWindow*             m_outputSashWindow;

private:
    DECLARE_EVENT_TABLE()
};


#endif
        // _ECOS_MAINWIN_H_
