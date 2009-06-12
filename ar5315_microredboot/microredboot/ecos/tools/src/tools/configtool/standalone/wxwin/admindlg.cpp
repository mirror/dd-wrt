//####COPYRIGHTBEGIN####
//
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
// Copyright (C) 2003 John Dallaway
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
// admindlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians,jld
// Contact(s):  julians
// Date:        2000/09/28
// Version:     $Id: admindlg.cpp,v 1.6 2001/08/22 16:50:32 julians Exp $
// Purpose:
// Description: Implementation file for ecAdminDialog
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
#pragma implementation "admindlg.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

// For registry access functions in GetUserToolsPath
#ifdef __WXMSW__
#include <windows.h>
#include "wx/msw/winundef.h"
#endif

#include "wx/cshelp.h"
#include "wx/filedlg.h"
#include "wx/file.h"
#include "wx/filefn.h"
#include "wx/progdlg.h"

#include "configtool.h"
#include "admindlg.h"
#include "configtooldoc.h"
#include "licensedlg.h"
#include "ecutils.h"

#ifdef __WXGTK__
#include "bitmaps/package_open.xpm"
#include "bitmaps/package_version.xpm"
#endif

BEGIN_EVENT_TABLE(ecAdminDialog, ecDialog)
    EVT_BUTTON(wxID_OK, ecAdminDialog::OnClose)
    EVT_BUTTON(ecID_ADMIN_DIALOG_ADD, ecAdminDialog::OnAdd)
    EVT_BUTTON(ecID_ADMIN_DIALOG_REMOVE, ecAdminDialog::OnRemove)
    EVT_INIT_DIALOG(ecAdminDialog::OnInitDialog)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// Frame constructor
ecAdminDialog::ecAdminDialog(wxWindow* parent, const wxString& repository, const wxString& userTools):
    m_imageList(16, 16, 1)
{
    m_strRepository = repository;
    m_strUserTools = userTools;
	m_CdlPkgData = NULL;

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    ecDialog::Create(parent, ecID_ADMIN_DIALOG, _("Administration"),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    CreateControls(this);

    m_imageList.Add(wxICON(package_open));
    m_imageList.Add(wxICON(package_version));
    m_treeCtrl->SetImageList(& m_imageList);

    Centre(wxBOTH);
}

ecAdminDialog::~ecAdminDialog()
{
	ClearPackageTree ();

	// free memory allocated to the CDL database

	if (m_CdlPkgData)
		delete m_CdlPkgData;

    m_treeCtrl->SetImageList(NULL);
}

void ecAdminDialog::OnInitDialog(wxInitDialogEvent& event)
{
    // setup the path to the user tools (tar and gunzip)
    
    if ((! m_strUserTools.IsEmpty()) || FindUserToolsPath ()) // if the user tools can be located
    {
        wxString path;
        wxGetEnv(wxT("PATH"), & path);
        
        // TODO: this may not work on all platforms
        path = path + wxString(wxPATH_SEP) + m_strUserTools;
        wxSetEnv(wxT("PATH"), path);
    }
    
    // populate the package tree

    if (!PopulatePackageTree (m_strRepository))
    {
        m_strRepository = wxT("");
        // TODO
        // OnPkgadminRepository (); // prompt the user for the repository location
    }
}

void ecAdminDialog::CreateControls(wxWindow* parent)
{
    m_treeCtrl = new wxTreeCtrl(parent, ecID_ADMIN_DIALOG_TREE,
        wxDefaultPosition, wxSize(380, 290), wxTR_HAS_BUTTONS | wxSUNKEN_BORDER);

    wxSizer *item0 = new wxBoxSizer( wxHORIZONTAL );

    wxSizer *item1 = new wxBoxSizer( wxVERTICAL );

    wxStaticText *item2 = new wxStaticText( parent, wxID_STATIC, _("&Installed packages:"), wxDefaultPosition, wxDefaultSize, 0 );
    item1->Add( item2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxWindow *item3 = parent->FindWindow( ecID_ADMIN_DIALOG_TREE );
    wxASSERT( item3 );
    item1->Add( item3, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item1, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 5 );

    wxSizer *item4 = new wxBoxSizer( wxVERTICAL );

    wxButton *item5 = new wxButton( parent, ecID_ADMIN_DIALOG_ADD, _("&Add..."), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item5, 0, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item6 = new wxButton( parent, ecID_ADMIN_DIALOG_REMOVE, _("&Remove"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item6, 0, wxALIGN_CENTRE|wxALL, 5 );

    item4->Add( 20, 20, 1, wxALIGN_CENTRE|wxALL, 5 );

    wxButton *item7 = new wxButton( parent, wxID_OK, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    item4->Add( item7, 0, wxALIGN_CENTRE|wxALL, 5 );

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item4->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    item0->Add( item4, 0, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxTOP|wxBOTTOM, 5 );
    
    item7->SetDefault(); // Make Close the default button

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    //item0->SetSizeHints( parent );

    // Add context-sensitive help text
    parent->FindWindow( ecID_ADMIN_DIALOG_TREE)->SetHelpText(_("Displays the set of packages currently in the eCos component repository."));
    parent->FindWindow( ecID_ADMIN_DIALOG_ADD)->SetHelpText(_("Adds the contents of an eCos package file to the eCos component repository."));
    parent->FindWindow( ecID_ADMIN_DIALOG_REMOVE)->SetHelpText(_("Removes the currently selected package from the eCos component repository."));
    parent->FindWindow( wxID_OK )->SetHelpText(_("Closes the dialog and saves any changes you have made."));

#if __WXGTK__
    parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif

}

void ecAdminDialog::OnAdd(wxCommandEvent& event)
{
    wxString defaultDir; // TODO
    wxString defaultFile;
    wxString wildcard = wxT("eCos Package Files (*.epk)|*.epk");
    wxFileDialog dlg(this, _("Open eCos Package Files"), defaultDir, defaultFile, wildcard, wxOPEN|wxMULTIPLE);

    if (wxID_OK == dlg.ShowModal ())
    {
        bool bRepositoryChanged = FALSE;
        //POSITION posPathName = dlg.GetStartPosition ();
        wxArrayString filenames;
        dlg.GetPaths(filenames);
        size_t i;
        for (i = (size_t) 0; i < filenames.GetCount(); i++)
        {
            wxString strPathName(filenames[i]);

            if (!wxFileExists(strPathName))
            {
                wxString msg;
                msg.Printf(_("Cannot open %s"), (const wxChar*) strPathName);
                wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
            }
            else
            {
                
                // get an eCos package distribution file
                
                // extract the licence file

                wxString strCommand;
                strCommand.Printf(wxT("add %s --extract_license"), (const wxChar*) strPathName);
                strCommand.Replace(wxT("\\"), wxT("/")); // backslashes -> forward slashes for Tcl_EvalFile
                EvalTclFile (3, strCommand, _("Adding package"));

                wxString strLicenseFile = m_strRepository + wxString(wxFILE_SEP_PATH) + wxT("pkgadd.txt");
#ifdef __WXMSW__
                strLicenseFile.Replace (wxT("/"), wxT("\\")); // forward slashes -> backslashes for Win32
#endif
                // read the license file

                wxFile fileLicenseFile;
                if (fileLicenseFile.Exists (strLicenseFile) && fileLicenseFile.Open (strLicenseFile, wxFile::read))
                {
                    //TRACE (_T("License file found at %s\n"), strLicenseFile);
                    const off_t dwLicenseLength = fileLicenseFile.Length ();
                    char* pszBuffer = new char [dwLicenseLength + 1]; // allocate a buffer
                    fileLicenseFile.Read ((void*) pszBuffer, dwLicenseLength);
                    fileLicenseFile.Close ();
                    wxRemoveFile (strLicenseFile); // delete the license file when read
                    pszBuffer [dwLicenseLength] = 0; // terminate the string in the buffer
                    wxString strLicenseText (pszBuffer); // copy into a wxString to convert to Unicode
                    delete [] pszBuffer;
#ifdef __WXMSW__
                    if (-1 == strLicenseText.Find (wxT("\r\n"))) // if the file has LF line endings...
                        strLicenseText.Replace (_T("\n"), _T("\r\n")); // ... replace with CRLF line endings
#else
                    strLicenseText.Replace (_T("\r"), wxEmptyString); // remove CR characters
#endif
                    // display the license text

                    ecLicenseDialog dlgLicense (strLicenseText, this, ecID_LICENSE_DIALOG, strPathName + _(" - Add Packages"));
                    if (wxID_OK != dlgLicense.ShowModal ()) // if license not accepted by user
                        continue; // try the next file
                }
                
                // add the contents of the package distribution file
                
                strCommand.Printf (wxT("add %s --accept_license"), (const wxChar*) strPathName);
                strCommand.Replace (wxT("\\"), wxT("/")); // backslashes -> forward slashes for Tcl_EvalFile
                if (! EvalTclFile (3, strCommand, _("Adding package")))  // if not successful
                {
                    // try the next file
                }
                else
                {                
                    bRepositoryChanged = TRUE;
                }
            }
        }
        
        // refresh the package tree only if necessary
        
        if (bRepositoryChanged && ! PopulatePackageTree (m_strRepository))
        {
        }
    }
}

void ecAdminDialog::OnRemove(wxCommandEvent& event)
{
    wxTreeCtrl* treeCtrl = (wxTreeCtrl*) FindWindow( ecID_ADMIN_DIALOG_TREE) ;

    const wxTreeItemId hTreeItem = treeCtrl->GetSelection ();
    if (! hTreeItem || !hTreeItem.IsOk())
        return;
    
    if (wxYES != wxMessageBox (_("The selected package will be deleted from the repository. Core eCos packages may be restored only by reinstalling eCos.\n\nDo you wish to continue?"),
        _("Remove Package"), wxYES_NO | wxICON_EXCLAMATION))
        return;

    ecAdminItemData* data = (ecAdminItemData*) treeCtrl->GetItemData (hTreeItem);

    if (data) // if a package node is selected
    {
        // remove all package version nodes

        wxString pstrPackage(data->m_string);
        
        bool bStatus = TRUE;
        long cookie;
        wxTreeItemId hChildItem = treeCtrl->GetFirstChild (hTreeItem, cookie);
        while (hChildItem && bStatus)
        {
            const wxTreeItemId hNextChildItem = treeCtrl->GetNextSibling (hChildItem);			
            bStatus = RemovePackageVersion (hChildItem);
            hChildItem = hNextChildItem;
        }
        
        // remove the package node
        
        if (bStatus)
        {
            treeCtrl->Delete (hTreeItem);
        }
    }
    else // a version node is selected
    {
        // remove the version node
        
        const wxTreeItemId hParentItem = treeCtrl->GetParent (hTreeItem);
        wxASSERT (hParentItem && hParentItem.IsOk() );
        if (RemovePackageVersion (hTreeItem) && ! treeCtrl->ItemHasChildren (hParentItem)) // if the only version was deleted
        {
            // remove the package node
            
            treeCtrl->Delete (hParentItem); 
        }
    }
}

void ecAdminDialog::OnClose(wxCommandEvent& event)
{
    event.Skip();
}

bool ecAdminDialog::FindUserToolsPath()
{
#ifdef __WXMSW__
    HKEY hKey;
    if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_CURRENT_USER, _T("Software\\eCos Configuration Tool\\Paths\\UserToolsDir"), 0, KEY_READ, &hKey))
        return FALSE;
    
    TCHAR szBuffer [MAX_PATH + 1];
    DWORD dwBufferLength = MAX_PATH + 1;
    LONG lStatus = RegQueryValueEx (hKey, _T("Folder"), NULL, NULL, (LPBYTE) szBuffer, &dwBufferLength);
    RegCloseKey (hKey);
    if (ERROR_SUCCESS != lStatus)
        return FALSE;
    
    m_strUserTools = szBuffer;
    // TRACE (_T("User tools found at %s\n"), m_strUserTools);
    return ! m_strUserTools.IsEmpty ();
#else
    // wxMessageBox("Sorry, ecAdminDialog::FindUserToolsPath not implemented for this platform.");
    return FALSE;
#endif
}

bool ecAdminDialog::RemovePackageVersion (wxTreeItemId hTreeItem)
{
    wxTreeCtrl* treeCtrl = (wxTreeCtrl*) FindWindow( ecID_ADMIN_DIALOG_TREE) ;

    const wxTreeItemId hParentItem = treeCtrl->GetParent (hTreeItem);
    wxASSERT (hParentItem);

    ecAdminItemData* data = (ecAdminItemData*) treeCtrl->GetItemData (hParentItem);

    wxASSERT( data );

    if (!data)
        return FALSE;

    wxString pstrPackage = data->m_string ;

    wxString strCommand;
    wxString itemText(treeCtrl->GetItemText (hTreeItem));
    strCommand.Printf (wxT("remove %s --version %s"), (const wxChar*) pstrPackage, (const wxChar*) itemText);
    if (! EvalTclFile (3, strCommand, wxT("Removing package"))) // if not successful
        return false;
    
    treeCtrl->Delete (hTreeItem); // remove the selected item from the tree

    return TRUE;
}

void ecAdminDialog::ClearPackageTree ()
{
    wxTreeCtrl* treeCtrl = (wxTreeCtrl*) FindWindow( ecID_ADMIN_DIALOG_TREE) ;

    wxTreeItemId hPackage = treeCtrl->GetRootItem ();
    if (! hPackage.IsOk()) // if no packages in the tree...
        return;     // ...nothing to do
    
    while (hPackage.IsOk())
    {
        const wxTreeItemId hNextPackage = treeCtrl->GetNextSibling(hPackage);
        treeCtrl->Delete (hPackage);
        hPackage = hNextPackage;
    }
}

// Trivial handlers; otherwise CdlPackagesDatabaseBody::make asserts.
static void CdlErrorHandler (std::string message)
{
};

static void CdlWarningHandler (std::string message)
{
};


bool ecAdminDialog::PopulatePackageTree (const wxString& packageDatabase)
{
    wxTreeCtrl* treeCtrl = (wxTreeCtrl*) FindWindow( ecID_ADMIN_DIALOG_TREE) ;

    // delete any existing CDL database
    
    if (m_CdlPkgData)
    {
        delete m_CdlPkgData;
        m_CdlPkgData = NULL;
    }
    
    // load the package database
    
    try
    {
        // Cdl asserts unless the handlers are present.
        m_CdlPkgData = CdlPackagesDatabaseBody::make (ecUtils::UnicodeToStdStr (packageDatabase), &CdlErrorHandler, &CdlWarningHandler);
    }
    catch (CdlStringException exception)
    {
        wxString strMessage;
        strMessage.Printf (_("Error loading database:\n\n%s"), (const wxChar*) wxString (exception.get_message ().c_str ()));
        wxMessageBox(strMessage, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        return FALSE;
    }
    catch (...)
    {
        wxMessageBox(_("Error loading database"), (const wxChar*) wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
        return FALSE;
    }
    
    // clear the old package tree
    
    ClearPackageTree ();
    
    // Add a root item
    wxTreeItemId rootId = m_treeCtrl->AddRoot(_("Packages"), 0, -1);

    // populate the new package tree
    
    const std::vector<std::string>& packages = m_CdlPkgData->get_packages ();
    for (std::vector<std::string>::const_iterator package = packages.begin (); package != packages.end (); package++)
    {
        // add a package node
        
        wxTreeItemId hPackage = treeCtrl->AppendItem (treeCtrl->GetRootItem(), wxString (m_CdlPkgData->get_package_aliases (*package) [0].c_str ()));
        treeCtrl->SetItemData (hPackage, new ecAdminItemData(wxString (package->c_str ())));
        treeCtrl->SetItemImage (hPackage, 0, wxTreeItemIcon_Normal);
        treeCtrl->SetItemImage (hPackage, 0, wxTreeItemIcon_Selected);
        treeCtrl->SetItemImage (hPackage, 0, wxTreeItemIcon_Expanded);
        treeCtrl->SetItemImage (hPackage, 0, wxTreeItemIcon_SelectedExpanded);
        
        const std::vector<std::string>& versions = m_CdlPkgData->get_package_versions (* package);
        for (std::vector<std::string>::const_iterator version = versions.begin (); version != versions.end (); version++)
        {
            // add a version node
            const wxTreeItemId hVersion = treeCtrl->AppendItem ( hPackage, wxString (version->c_str ()));
            treeCtrl->SetItemImage (hVersion, 1, wxTreeItemIcon_Normal);
            treeCtrl->SetItemImage (hVersion, 1, wxTreeItemIcon_Selected);
            treeCtrl->SetItemImage (hVersion, 1, wxTreeItemIcon_Expanded);
            treeCtrl->SetItemImage (hVersion, 1, wxTreeItemIcon_SelectedExpanded);
        }
        treeCtrl->SortChildren (hPackage); // sort the version nodes
    }
    
    treeCtrl->SortChildren (treeCtrl->GetRootItem()); // sort the package nodes
    treeCtrl->Expand(treeCtrl->GetRootItem());
    
    return TRUE;
}


bool ecAdminDialog::EvalTclFile(int nargc, const wxString& Argv, const wxString& msg)
{
    wxProgressDialog dlgWait(msg, _("Please wait..."), 100, this);

    dlgWait.Update(50);

//TRACE (_T("Evaluating ecosadmin.tcl %s\n"), pszArgv);

    // set up the data structure which is passed to the Tcl thread

    wxString strArgc;
    strArgc.Printf (wxT("%d"), nargc);
    std::string argv0 = ecUtils::UnicodeToStdStr (m_strRepository) + "/ecosadmin.tcl";
    std::string argv = ecUtils::UnicodeToStdStr (Argv);
    std::string argc = ecUtils::UnicodeToStdStr (strArgc);

    Tcl_Interp * interp = Tcl_CreateInterp ();

#ifdef __WXMSW__
    Tcl_Channel outchan = Tcl_OpenFileChannel (interp, "nul", "a+", 777);
    Tcl_SetStdChannel (outchan, TCL_STDOUT); // direct standard output to NUL:
#endif

    const char * pszStatus = Tcl_SetVar (interp, "argv0", (char*) argv0.c_str(), 0);
    pszStatus = Tcl_SetVar (interp, "argv", (char*) argv.c_str(), 0);
    pszStatus = Tcl_SetVar (interp, "argc", (char*) argc.c_str(), 0);
    pszStatus = Tcl_SetVar (interp, "gui_mode", "1", 0); // return errors in result string
    int nStatus = Tcl_EvalFile (interp, (char*) argv0.c_str());
    const char* result = Tcl_GetStringResult (interp);

#ifdef __WXMSW__
    Tcl_SetStdChannel (NULL, TCL_STDOUT);
    Tcl_UnregisterChannel (interp, outchan);
#endif

    Tcl_DeleteInterp (interp);

    wxString strErrorMessage (result);

    // report any error
    if (! strErrorMessage.IsEmpty ())
    {
        wxString msg (_("Command execution error:\n\n") + strErrorMessage);
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
	return FALSE;
    }
    else if (TCL_OK != nStatus)
    {
        wxString msg (_("Command execution error"));
        wxMessageBox(msg, wxGetApp().GetSettings().GetAppName(), wxICON_EXCLAMATION|wxOK);
    return FALSE;
    }

    return TRUE;
}
