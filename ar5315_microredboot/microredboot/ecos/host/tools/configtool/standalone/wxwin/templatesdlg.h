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
// templatesdlg.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/09/27
// Version:     $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/host/tools/configtool/standalone/wxwin/templatesdlg.h#3 $
// Purpose:
// Description: Header file for ecTemplatesDialog
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_TEMPLATESDLG_H_
#define _ECOS_TEMPLATESDLG_H_

#ifdef __GNUG__
#pragma interface "templatesdlg.cpp"
#endif

#include "ecutils.h"

class ecTemplatesDialog : public ecDialog
{
public:
// Ctor(s)
    ecTemplatesDialog(wxWindow* parent);
    ~ecTemplatesDialog();

//// Event handlers

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnDetails(wxCommandEvent& event);
	void OnSelHardwareTemplates(wxCommandEvent& event);
	void OnSelPackageTemplates(wxCommandEvent& event);
	void OnSelPackageVersion(wxCommandEvent& event);

    void ShowDetails(bool show);

//// Operations
    void CreateControls(wxWindow* parent);
    void PopulateControls();
	void UpdateVersionList(const wxString& defaultVersion);
	void UpdateDetails();

//// Accesors
	wxString GetSelectedHardware () const { return m_hardware; }
	wxString GetSelectedTemplate () const { return m_template; }
	wxString GetSelectedTemplateVersion () const { return m_template_version; }
protected:
	wxString    m_hardware;
	wxString    m_template;
	wxString    m_template_version;

	wxString    m_strCdlHardwareDescription;
	wxString    m_strCdlTemplateDescription;
	wxString    m_strCdlTemplatePackages;

private:
    DECLARE_EVENT_TABLE()
};

#define ecID_TEMPLATES_DIALOG_HARDWARE_TEMPLATES 10056
#define ecID_TEMPLATES_DIALOG_HARDWARE_DESCRIPTION 10057
#define ecID_TEMPLATES_DIALOG_PACKAGE_TEMPLATES 10058
#define ecID_TEMPLATES_DIALOG_VERSION 10059
#define ecID_TEMPLATES_DIALOG_PACKAGE_DESCRIPTION 10060
#define ecID_TEMPLATES_DIALOG_DETAILS 10061
#define ecID_TEMPLATES_DIALOG_PACKAGES_MSG 10062
#define ecID_TEMPLATES_DIALOG_PACKAGES 10063

#endif
        // _ECOS_TEMPLATESDLG_H_
