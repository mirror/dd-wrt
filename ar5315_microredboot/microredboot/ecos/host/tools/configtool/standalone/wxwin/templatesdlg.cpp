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
// templatesdlg.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/27
// Version:     $Id: templatesdlg.cpp,v 1.10 2001/12/03 16:05:40 julians Exp $
// Purpose:
// Description: Implementation file for ecTemplatesDialog
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
#pragma implementation "templatesdlg.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wx/cshelp.h"
#include "wx/valgen.h"

#include "configtool.h"
#include "configtooldoc.h"
#include "templatesdlg.h"
#include "ecutils.h"

BEGIN_EVENT_TABLE(ecTemplatesDialog, ecDialog)
    EVT_BUTTON(wxID_OK, ecTemplatesDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, ecTemplatesDialog::OnCancel)
    EVT_COMBOBOX(ecID_TEMPLATES_DIALOG_HARDWARE_TEMPLATES, ecTemplatesDialog::OnSelHardwareTemplates)
    EVT_CHOICE(ecID_TEMPLATES_DIALOG_PACKAGE_TEMPLATES, ecTemplatesDialog::OnSelPackageTemplates)
    EVT_CHOICE(ecID_TEMPLATES_DIALOG_VERSION, ecTemplatesDialog::OnSelPackageVersion)
    EVT_BUTTON(ecID_TEMPLATES_DIALOG_DETAILS, ecTemplatesDialog::OnDetails)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// Frame constructor
ecTemplatesDialog::ecTemplatesDialog(wxWindow* parent)
{
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);

    ecDialog::Create(parent, ecID_TEMPLATES_DIALOG, _("Templates"),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

    CreateControls(this);

    Centre(wxBOTH);
}

ecTemplatesDialog::~ecTemplatesDialog()
{
}

void ecTemplatesDialog::CreateControls(wxWindow* parent)
{
    wxSizer *item0 = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *item2 = new wxStaticBox( parent, -1, _("Hardware") );
    wxSizer *item1 = new wxStaticBoxSizer( item2, wxVERTICAL );

    wxString *strs3 = (wxString*) NULL;
    wxComboBox *item3 = new wxComboBox( parent, ecID_TEMPLATES_DIALOG_HARDWARE_TEMPLATES, "", wxDefaultPosition, wxSize(360,-1), 0, strs3, wxCB_DROPDOWN|wxCB_READONLY|wxCB_SORT );
    item1->Add( item3, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxTextCtrl *item4 = new wxTextCtrl( parent, ecID_TEMPLATES_DIALOG_HARDWARE_DESCRIPTION, _(""), wxDefaultPosition, wxSize(90,60), wxTE_MULTILINE|wxTE_READONLY );
    item1->Add( item4, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item0->Add( item1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxSizer *item5 = new wxBoxSizer( wxHORIZONTAL );

    wxStaticBox *item7 = new wxStaticBox( parent, -1, _("Packages") );
    wxSizer *item6 = new wxStaticBoxSizer( item7, wxVERTICAL );

    wxSizer *item8 = new wxBoxSizer( wxHORIZONTAL );

    wxString *strs9 = (wxString*) NULL;
    wxChoice *item9 = new wxChoice( parent, ecID_TEMPLATES_DIALOG_PACKAGE_TEMPLATES, wxDefaultPosition, wxSize(100,-1), 0, strs9, 0 );
    item8->Add( item9, 20, wxALIGN_CENTRE|wxALL, 5 );

    wxString *strs10 = (wxString*) NULL;
    wxChoice *item10 = new wxChoice( parent, ecID_TEMPLATES_DIALOG_VERSION, wxDefaultPosition, wxSize(90,-1), 0, strs10, 0 );
    item8->Add( item10, 0, wxALIGN_CENTRE|wxALL, 5 );

    item6->Add( item8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    wxTextCtrl *item11 = new wxTextCtrl( parent, ecID_TEMPLATES_DIALOG_PACKAGE_DESCRIPTION, _(""), wxDefaultPosition, wxSize(110,70), wxTE_MULTILINE|wxTE_READONLY );
    item6->Add( item11, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    item5->Add( item6, 1, wxALIGN_CENTRE|wxLEFT|wxRIGHT|wxBOTTOM, 5 );

    wxSizer *item12 = new wxBoxSizer( wxVERTICAL );

    wxButton *item13 = new wxButton( parent, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    item12->Add( item13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxButton *item14 = new wxButton( parent, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    item12->Add( item14, 0, wxALIGN_CENTRE|wxALL, 5 );

    item12->Add( 10, 10, 0, wxALIGN_CENTRE|wxALL, 0 );

    wxButton *item15 = new wxButton( parent, ecID_TEMPLATES_DIALOG_DETAILS, _("&Details >>"), wxDefaultPosition, wxDefaultSize, 0 );
    item12->Add( item15, 0, wxALIGN_CENTRE|wxALL, 5 );

    item5->Add( item12, 0, wxALIGN_CENTRE|wxALL, 5 );

    item0->Add( item5, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0 );

    wxStaticText *item16 = new wxStaticText( parent, ecID_TEMPLATES_DIALOG_PACKAGES_MSG, _("&Packages in selected template:"), wxDefaultPosition, wxDefaultSize, 0 );

    wxTextCtrl *item17 = new wxTextCtrl( parent, ecID_TEMPLATES_DIALOG_PACKAGES, _(""), wxDefaultPosition, wxSize(90,100), wxTE_MULTILINE|wxTE_READONLY );

    // Don't add these yet (until press Details)
    //item0->Add( item16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    //item0->Add( item17, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
    item16->Show(FALSE);
    item17->Show(FALSE);

#ifdef __WXGTK__
    wxButton *contextButton = new wxContextHelpButton( parent );
    item12->Add( contextButton, 0, wxALIGN_CENTRE|wxALL, 5 );
#endif

    parent->SetAutoLayout( TRUE );
    parent->SetSizer( item0 );
    parent->Layout();
    item0->Fit( parent );
    //item0->SetSizeHints( parent );

    // Add context-sensitive help text
    parent->FindWindow( ecID_TEMPLATES_DIALOG_HARDWARE_TEMPLATES )->SetHelpText(_("Selects from the set of available hardware templates."));
    parent->FindWindow( ecID_TEMPLATES_DIALOG_HARDWARE_DESCRIPTION )->SetHelpText(_("Gives a brief description of the currently selected hardware template."));
    parent->FindWindow( ecID_TEMPLATES_DIALOG_PACKAGE_TEMPLATES )->SetHelpText(_("Selects from the set of available package templates."));
    parent->FindWindow( ecID_TEMPLATES_DIALOG_VERSION )->SetHelpText(_("Selects the version of the currently selected template."));
    parent->FindWindow( ecID_TEMPLATES_DIALOG_PACKAGE_DESCRIPTION )->SetHelpText(_("Gives a brief description of the currently selected package template."));
    parent->FindWindow( ecID_TEMPLATES_DIALOG_PACKAGES )->SetHelpText(_("Lists the packages contained in the currently selected template."));
    parent->FindWindow( wxID_OK )->SetHelpText(_("Closes the dialog and saves any changes you have made."));
    parent->FindWindow( wxID_CANCEL )->SetHelpText(_("Closes the dialog without saving any changes you have made."));
    parent->FindWindow( ecID_TEMPLATES_DIALOG_DETAILS )->SetHelpText(_("Shows or hides a portion of the dialog that provides details of the contents of the currently selected template."));

#if __WXGTK__
    parent->FindWindow( wxID_CONTEXT_HELP )->SetHelpText(_("Invokes context-sensitive help for the clicked-on window."));
#endif

	// Add validators
	parent->FindWindow( ecID_TEMPLATES_DIALOG_PACKAGE_DESCRIPTION )->SetValidator(wxGenericValidator(& m_strCdlTemplateDescription));
	parent->FindWindow( ecID_TEMPLATES_DIALOG_HARDWARE_DESCRIPTION )->SetValidator(wxGenericValidator(& m_strCdlHardwareDescription));
	parent->FindWindow( ecID_TEMPLATES_DIALOG_PACKAGES)->SetValidator(wxGenericValidator(& m_strCdlTemplatePackages));

    PopulateControls();
}

// function which is called by quick sort
static int wxStringCompareFunction(const void *first, const void *second)
{
  wxString *strFirst = (wxString *)first;
  wxString *strSecond = (wxString *)second;

  return wxStricmp(strFirst->c_str(), strSecond->c_str());
}

void ecTemplatesDialog::PopulateControls()
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();

    m_hardware = doc->GetCdlConfig ()->get_hardware ().c_str();

    wxComboBox* cdlHardwareCtrl = (wxComboBox*)  FindWindow( ecID_TEMPLATES_DIALOG_HARDWARE_TEMPLATES ) ;
    wxChoice* cdlPackageCtrl = (wxChoice*)  FindWindow( ecID_TEMPLATES_DIALOG_PACKAGE_TEMPLATES ) ;


	const std::vector<std::string> & targets = doc->GetCdlPkgData ()->get_targets ();
	std::vector<std::string>::const_iterator target_i;

    // Old code: let the combo box do the sorting. But not all platforms implement this.
#if 0
    // populate the hardware combo box
    int nIndex = 0;
	for (target_i = targets.begin (); target_i != targets.end (); target_i++)
	{
		const std::vector<std::string> & aliases = doc->GetCdlPkgData ()->get_target_aliases (* target_i);

		// use the first alias (if any) as the description
		wxString strTargetDescription = aliases.size () ? aliases [0].c_str () : target_i->c_str ();
		cdlHardwareCtrl->Append(strTargetDescription, (void*) &(*target_i)); // store the target iterator
        std::string str(* (target_i));
		if (m_hardware == str.c_str())            // if current target...
        {
            int sel = 0;
            int i;
            for (i = 0; i <= nIndex; i++)
                if (cdlHardwareCtrl->GetClientData(i) == (void*) &(*target_i))
                    sel = i;
			cdlHardwareCtrl->SetSelection (sel); // ...select the string
        }
        nIndex ++;
	}
#else
    // New code: sort, then add to combobox. How do we keep track of the target iterators?
    // could use hash table, assuming that each string is unique
    wxHashTable ht(wxKEY_STRING);
    wxArrayString ar;

	for (target_i = targets.begin (); target_i != targets.end (); target_i++)
	{
		const std::vector<std::string> & aliases = doc->GetCdlPkgData ()->get_target_aliases (* target_i);

		// use the first alias (if any) as the description
		wxString strTargetDescription = aliases.size () ? aliases [0].c_str () : target_i->c_str ();

        ar.Add(strTargetDescription);
        ht.Put(strTargetDescription, (wxObject*) (void*) &(*target_i));
    }

    ar.Sort((wxArrayString::CompareFunction) & wxStringCompareFunction);

    int nIndex = 0;

    unsigned int i;
    for (i = 0; i < ar.GetCount(); i ++)
    {
        wxString strTargetDescription = ar[i];

        std::string *t_i = (std::string*) (void*) ht.Get(strTargetDescription);

		cdlHardwareCtrl->Append(strTargetDescription, (void*) t_i); // store the target iterator
        std::string str(* (t_i));
		if (m_hardware == str.c_str())            // if current target...
        {
            int sel = 0;
            int i;
            for (i = 0; i <= nIndex; i++)
                if (cdlHardwareCtrl->GetClientData(i) == (void*) &(*t_i))
                    sel = i;
			cdlHardwareCtrl->SetSelection (sel); // ...select the string
        }
        nIndex ++;
	}

#endif

	if (-1 == cdlHardwareCtrl->GetSelection ()) // if no target selected...
		cdlHardwareCtrl->SetSelection (0);          // ...select the first one

	// populate the template combo box
	m_template = doc->GetCdlConfig ()->get_template ().c_str();
	const std::vector<std::string> & templates = doc->GetCdlPkgData ()->get_templates ();
	std::vector<std::string>::const_iterator template_i;
    nIndex = 0;
	for (template_i = templates.begin (); template_i != templates.end (); template_i++)
	{
		wxString strTemplateDescription = template_i->c_str ();
		cdlPackageCtrl->Append(strTemplateDescription, (void*) &(*template_i)); // store the template iterator
		/// m_cboCdlTemplate.SetItemData (nIndex, (DWORD) template_i); // store the template iterator
		std::string str(* (template_i));
		if (m_template == str.c_str())          // if current template...
			cdlPackageCtrl->SetSelection(nIndex); // ...select the string
		nIndex ++;
	}

	if (-1 == cdlPackageCtrl->GetSelection()) // if no template selected...
		cdlPackageCtrl->SetSelection(0);          // ...select the first one

	// display initial target and template descriptions
        wxCommandEvent dummyEvent;
	OnSelHardwareTemplates(dummyEvent);
	OnSelPackageTemplates(dummyEvent);

	// populate the template version combo box
	UpdateVersionList (doc->GetTemplateVersion ());
}

void ecTemplatesDialog::OnCancel(wxCommandEvent& event)
{
    event.Skip();
}

void ecTemplatesDialog::OnOK(wxCommandEvent& event)
{
    event.Skip();
}

void ecTemplatesDialog::OnDetails(wxCommandEvent& event)
{
    wxWindow* win1 = FindWindow(ecID_TEMPLATES_DIALOG_PACKAGES);

    wxASSERT( win1 != NULL );

    bool show = !win1->IsShown();

    ShowDetails(show);
}

void ecTemplatesDialog::ShowDetails(bool show)
{
    wxWindow* win1 = FindWindow(ecID_TEMPLATES_DIALOG_PACKAGES);
    wxWindow* win2 = FindWindow(ecID_TEMPLATES_DIALOG_PACKAGES_MSG);
    wxButton* button = (wxButton*) FindWindow(ecID_TEMPLATES_DIALOG_DETAILS);

    if (show)
    {
        GetSizer()->Add( win2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
        GetSizer()->Add( win1, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5 );
        button->SetLabel("&Details <<");
    }
    else
    {
        GetSizer()->Remove(win1);
        GetSizer()->Remove(win2);
        button->SetLabel("&Details >>");
    }
    win1->Show(show);
    win2->Show(show);

    Layout();
    GetSizer()->Fit( this );
}

void ecTemplatesDialog::OnSelHardwareTemplates(wxCommandEvent& event)
{
    wxComboBox* cdlHardwareCtrl = (wxComboBox*)  FindWindow( ecID_TEMPLATES_DIALOG_HARDWARE_TEMPLATES ) ;
    wxChoice* cdlPackageCtrl = (wxChoice*)  FindWindow( ecID_TEMPLATES_DIALOG_PACKAGE_TEMPLATES ) ;

    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();

	// the target has changed so retrieve the new target description
	const int nIndex = cdlHardwareCtrl->GetSelection ();
	//std::vector<std::string>::const_iterator template_i = (std::vector<std::string>::const_iterator) cdlHardwareCtrl->GetClientData (nIndex);
    std::string* template_i =  (std::string*) cdlHardwareCtrl->GetClientData (nIndex);
	m_hardware = template_i->c_str();

	m_strCdlHardwareDescription = doc->GetCdlPkgData ()->get_target_description ((const wxChar*) m_hardware).c_str ();
	m_strCdlHardwareDescription = ecUtils::StripExtraWhitespace (m_strCdlHardwareDescription);

    UpdateDetails (); // display new hardware packages in details box

    TransferDataToWindow (); // display new target description
}

void ecTemplatesDialog::OnSelPackageTemplates(wxCommandEvent& event)
{
    wxComboBox* cdlHardwareCtrl = (wxComboBox*)  FindWindow( ecID_TEMPLATES_DIALOG_HARDWARE_TEMPLATES ) ;
    wxChoice* cdlPackageCtrl = (wxChoice*)  FindWindow( ecID_TEMPLATES_DIALOG_PACKAGE_TEMPLATES ) ;

	// the template has changed so update the version combo box
	int nIndex = cdlPackageCtrl->GetSelection ();

	//std::vector<std::string>::const_iterator template_i = (std::vector<std::string>::const_iterator) cdlPackageCtrl->GetClientData (nIndex);
    std::string *template_i = (std::string*) cdlPackageCtrl->GetClientData (nIndex);
	m_template = template_i->c_str();
	
	UpdateVersionList (wxT("")); // repopulate template versions combo box and select most recent version
}

void ecTemplatesDialog::OnSelPackageVersion(wxCommandEvent& event)
{
	wxString strVersion;
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();
    wxChoice* cdlVersionCtrl = (wxChoice*)  FindWindow( ecID_TEMPLATES_DIALOG_VERSION ) ;

	strVersion = cdlVersionCtrl->GetStringSelection ();

	//TRACE (_T("Version '%s' selected\n"), strVersion);
	m_template_version = ecUtils::UnicodeToStdStr (strVersion).c_str();
	m_strCdlTemplateDescription = doc->GetCdlPkgData ()->get_template_description (m_template.c_str(), m_template_version.c_str()).c_str ();
	m_strCdlTemplateDescription = ecUtils::StripExtraWhitespace (m_strCdlTemplateDescription);
	
	UpdateDetails (); // display new template packages in details box
    TransferDataToWindow (); // display new template description
}

void ecTemplatesDialog::UpdateVersionList(const wxString& defaultVersion)
{
    wxComboBox* cdlHardwareCtrl = (wxComboBox*)  FindWindow( ecID_TEMPLATES_DIALOG_HARDWARE_TEMPLATES ) ;
    wxChoice* cdlPackageCtrl = (wxChoice*)  FindWindow( ecID_TEMPLATES_DIALOG_PACKAGE_TEMPLATES ) ;
    wxChoice* cdlVersionCtrl = (wxChoice*)  FindWindow( ecID_TEMPLATES_DIALOG_VERSION ) ;

    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();

	// clear the version combo box
	cdlVersionCtrl->Clear ();
	
	// get the template version information
	const std::vector<std::string>& versions = doc->GetCdlPkgData ()->get_template_versions (m_template.c_str());
	wxASSERT (versions.size () > 0);
	
	// add the template versions to the version combo box
	for (unsigned int version = 0; version < versions.size (); version++) {
		// TRACE (_T("Adding version '%s'\n"), CString (versions [version].c_str ()));
		cdlVersionCtrl->Append (versions [version].c_str ());
	}
	
	// select the appropriate version in the version combo box
	if (defaultVersion.IsEmpty()) { // if no default version specified
		cdlVersionCtrl->SetSelection (versions.size () - 1); // select the most recent version
	} else { // a default version was specified
		cdlVersionCtrl->SetStringSelection (defaultVersion);
	}
        wxCommandEvent dummyEvent;

	OnSelPackageVersion(dummyEvent);
	
	// enable the version combo box only if there are multiple versions
	cdlVersionCtrl->Enable (versions.size () > 1);
}

void ecTemplatesDialog::UpdateDetails()
{
    ecConfigToolDoc* doc = wxGetApp().GetConfigToolDoc();

	// retrieve the template and target package names
	const std::vector<std::string> & template_packages = doc->GetCdlPkgData ()->get_template_packages (m_template.c_str(), m_template_version.c_str());
	std::vector<std::string> packages = doc->GetCdlPkgData ()->get_target_packages (m_hardware.c_str());
	packages.insert (packages.end (), template_packages.begin (), template_packages.end ());
	
	// retrieve the zeroth (verbose) package alias for each package
	std::vector<std::string> aliases;
        unsigned int i;
	for (i = 0; i < packages.size (); i++)
	{
		if (doc->GetCdlPkgData ()->is_known_package (packages [i])) // if the package is installed
		{
			aliases.push_back (doc->GetCdlPkgData ()->get_package_aliases (packages [i]) [0]);
		}
		else // package is not installed
		{
			aliases.push_back ("Unknown package " + packages [i]);
		}
	}
	
	// sort the aliases into alphabetical order
	std::sort (aliases.begin (), aliases.end ());
	
	// copy the aliases into the details box
	m_strCdlTemplatePackages = wxT("");
	for (i = 0; i < aliases.size (); i++)
	{
		m_strCdlTemplatePackages += aliases [i].c_str ();
#ifdef __WXMSW__
		m_strCdlTemplatePackages += wxT("\r\n"); // add a CRLF between each alias
#else
		m_strCdlTemplatePackages += wxT("\n"); // add a LF between each alias
#endif
	}
	// TODO: does this work for CRLF?
	m_strCdlTemplatePackages.Trim (TRUE); // remove the trailing CRLF
}
