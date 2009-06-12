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
// CdlTemplatesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Configtool.h"
#include "ConfigtoolDoc.h"
#include "CTUtils.h"
#include "CdlTemplatesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCdlTemplatesDialog dialog


CCdlTemplatesDialog::CCdlTemplatesDialog(CWnd* pParent /*=NULL*/)
	: CeCosDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CCdlTemplatesDialog)
	m_strCdlTemplateDescription = _T("");
	m_strCdlTemplatePackages = _T("");
	//}}AFX_DATA_INIT
}


void CCdlTemplatesDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCdlTemplatesDialog)
	DDX_Control(pDX, IDC_CDL_TEMPLATE_PACKAGES, m_edtCdlTemplatePackages);
	DDX_Control(pDX, IDC_CDL_TEMPLATE, m_cboCdlTemplate);
	DDX_Control(pDX, IDC_CDL_TEMPLATE_VER, m_cboCdlTemplateVersion);
	DDX_Control(pDX, IDC_CDL_HARDWARE, m_cboCdlHardware);
	DDX_Text(pDX, IDC_CDL_HARDWARE_DESC, m_strCdlHardwareDescription);
	DDX_Text(pDX, IDC_CDL_TEMPLATE_DESC, m_strCdlTemplateDescription);
	DDX_Text(pDX, IDC_CDL_TEMPLATE_PACKAGES, m_strCdlTemplatePackages);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCdlTemplatesDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CCdlTemplatesDialog)
	ON_CBN_SELCHANGE(IDC_CDL_HARDWARE, OnSelchangeCdlHardware)
	ON_BN_CLICKED(IDC_DETAILS, OnDetails)
	ON_WM_CANCELMODE()
	ON_CBN_SELCHANGE(IDC_CDL_TEMPLATE, OnSelchangeCdlTemplate)
	ON_CBN_SELCHANGE(IDC_CDL_TEMPLATE_VER, OnSelchangeCdlTemplateVersion)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCdlTemplatesDialog message handlers

BOOL CCdlTemplatesDialog::OnInitDialog() 
{
	CeCosDialog::OnInitDialog();

	ShowDetails (false); // hide the details initially

	CConfigToolDoc * pDoc = CConfigTool::GetConfigToolDoc ();

	// populate the hardware combo box
	m_hardware = pDoc->GetCdlConfig ()->get_hardware ();
	const std::vector<std::string> & targets = pDoc->GetCdlPkgData ()->get_targets ();
	std::vector<std::string>::const_iterator target_i;
	for (target_i = targets.begin (); target_i != targets.end (); target_i++)
	{
		const std::vector<std::string> & aliases = pDoc->GetCdlPkgData ()->get_target_aliases (* target_i);

		// use the first alias (if any) as the description
		CString strTargetDescription = aliases.size () ? aliases [0].c_str () : target_i->c_str ();
		int nIndex = m_cboCdlHardware.AddString (strTargetDescription);
		m_cboCdlHardware.SetItemData (nIndex, (DWORD) target_i); // store the target iterator
		if (m_hardware == * target_i)            // if current target...
			m_cboCdlHardware.SetCurSel (nIndex); // ...select the string
	}

	if (CB_ERR == m_cboCdlHardware.GetCurSel ()) // if no target selected...
		m_cboCdlHardware.SetCurSel (0);          // ...select the first one

	// populate the template combo box
	m_template = pDoc->GetCdlConfig ()->get_template ();
	const std::vector<std::string> & templates = pDoc->GetCdlPkgData ()->get_templates ();
	std::vector<std::string>::const_iterator template_i;
	for (template_i = templates.begin (); template_i != templates.end (); template_i++)
	{
		CString strTemplateDescription = template_i->c_str ();
		int nIndex = m_cboCdlTemplate.AddString (strTemplateDescription);
		m_cboCdlTemplate.SetItemData (nIndex, (DWORD) template_i); // store the template iterator
		if (m_template == * template_i)          // if current template...
			m_cboCdlTemplate.SetCurSel (nIndex); // ...select the string
	}

	if (CB_ERR == m_cboCdlTemplate.GetCurSel ()) // if no template selected...
		m_cboCdlTemplate.SetCurSel (0);          // ...select the first one

	// display initial target and template descriptions
	OnSelchangeCdlHardware ();
	OnSelchangeCdlTemplate ();

	// populate the template version combo box
	UpdateVersionList (pDoc->GetTemplateVersion ());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCdlTemplatesDialog::OnSelchangeCdlHardware() 
{
	CConfigToolDoc * pDoc = CConfigTool::GetConfigToolDoc ();

	// the target has changed so retrieve the new target description
	const int nIndex = m_cboCdlHardware.GetCurSel ();
	m_hardware = * (std::vector<std::string>::const_iterator) m_cboCdlHardware.GetItemData (nIndex);
	m_strCdlHardwareDescription = pDoc->GetCdlPkgData ()->get_target_description (m_hardware).c_str ();
	m_strCdlHardwareDescription = CUtils::StripExtraWhitespace (m_strCdlHardwareDescription);

    UpdateDetails (); // display new hardware packages in details box
    UpdateData (FALSE); // display new target description
}

void CCdlTemplatesDialog::OnSelchangeCdlTemplate() 
{
	// the template has changed so update the version combo box
	int nIndex = m_cboCdlTemplate.GetCurSel ();
	m_template = * (std::vector<std::string>::const_iterator) m_cboCdlTemplate.GetItemData (nIndex);
	UpdateVersionList (""); // repopulate template versions combo box and select most recent version
}

void CCdlTemplatesDialog::UpdateVersionList(std::string default_version)
{
  // clear the version combo box
  m_cboCdlTemplateVersion.ResetContent ();

  // get the template version information
  CConfigToolDoc * pDoc = CConfigTool::GetConfigToolDoc ();
  const std::vector<std::string>& versions = pDoc->GetCdlPkgData ()->get_template_versions (m_template);
  ASSERT (versions.size () > 0);

  // add the template versions to the version combo box
  for (unsigned int version = 0; version < versions.size (); version++) {
    TRACE (_T("Adding version '%s'\n"), CString (versions [version].c_str ()));
    m_cboCdlTemplateVersion.AddString (CString (versions [version].c_str ()));
  }

  // select the appropriate version in the version combo box
  if ("" == default_version) { // if no default version specified
    m_cboCdlTemplateVersion.SetCurSel (versions.size () - 1); // select the most recent version
  } else { // a default version was specified
    m_cboCdlTemplateVersion.SelectString (-1, CString (default_version.c_str ()));
  }
  OnSelchangeCdlTemplateVersion ();

  // enable the version combo box only if there are multiple versions
  m_cboCdlTemplateVersion.EnableWindow (versions.size () > 1);
}

void CCdlTemplatesDialog::UpdateDetails()
{
  // retrieve the template and target package names
  CConfigToolDoc * pDoc = CConfigTool::GetConfigToolDoc ();
  const std::vector<std::string> & template_packages = pDoc->GetCdlPkgData ()->get_template_packages (m_template, m_template_version);
  std::vector<std::string> packages = pDoc->GetCdlPkgData ()->get_target_packages (m_hardware);
  packages.insert (packages.end (), template_packages.begin (), template_packages.end ());

  // retrieve the zeroth (verbose) package alias for each package
  std::vector<std::string> aliases;
  for (unsigned int i = 0; i < packages.size (); i++)
  {
    if (pDoc->GetCdlPkgData ()->is_known_package (packages [i])) // if the package is installed
    {
      aliases.push_back (pDoc->GetCdlPkgData ()->get_package_aliases (packages [i]) [0]);
    }
    else // package is not installed
    {
      aliases.push_back ("Unknown package " + packages [i]);
    }
  }

  // sort the aliases into alphabetical order
  std::sort (aliases.begin (), aliases.end ());

  // copy the aliases into the details box
  m_strCdlTemplatePackages = _T("");
  for (i = 0; i < aliases.size (); i++)
  {
    m_strCdlTemplatePackages += aliases [i].c_str ();
    m_strCdlTemplatePackages += _T("\r\n"); // add a CRLF between each alias
  }
  m_strCdlTemplatePackages.TrimRight (); // remove the trailing CRLF
}

void CCdlTemplatesDialog::OnSelchangeCdlTemplateVersion()
{
  CString strVersion;
  CConfigToolDoc * pDoc = CConfigTool::GetConfigToolDoc ();
  m_cboCdlTemplateVersion.GetLBText (m_cboCdlTemplateVersion.GetCurSel (), strVersion);
  TRACE (_T("Version '%s' selected\n"), strVersion);
  m_template_version = CUtils::UnicodeToStdStr (strVersion);
  m_strCdlTemplateDescription = pDoc->GetCdlPkgData ()->get_template_description (m_template, m_template_version).c_str ();
  m_strCdlTemplateDescription = CUtils::StripExtraWhitespace (m_strCdlTemplateDescription);

  UpdateDetails (); // display new template packages in details box
  UpdateData (FALSE); // display new template description
}

void CCdlTemplatesDialog::OnDetails() 
{
    ShowDetails (! m_edtCdlTemplatePackages.IsWindowVisible ());
}

void CCdlTemplatesDialog::ShowDetails(bool bShow)
{
    // show or hide the windows
    m_edtCdlTemplatePackages.ShowWindow (bShow ? SW_SHOW : SW_HIDE);
    GetDlgItem (IDC_CDL_TEMPLATE_PACKAGES_STATIC)->ShowWindow (bShow ? SW_SHOW : SW_HIDE);
    GetDlgItem (IDC_DETAILS)->SetWindowText (bShow ? _T("&Details <<") : _T("&Details >>"));
    
    // resize the dialog box
    CRect rect1,rect2;
    GetDlgItem (IDC_DETAILS)->GetWindowRect (rect1);
    m_edtCdlTemplatePackages.GetWindowRect (rect2);
    int delta = rect2.bottom - rect1.bottom;
    CRect rcDlg;
    GetWindowRect (rcDlg);
    rcDlg.bottom += bShow ? delta : - delta;
    MoveWindow (rcDlg);
}
