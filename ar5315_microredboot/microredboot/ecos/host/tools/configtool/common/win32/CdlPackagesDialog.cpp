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
// CdlPackagesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigTool.h"
#include "ConfigtoolDoc.h"
#include "AddRemoveDialog.h"
#include "CdlPackagesDialog.h"
#include "CTUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCdlPackagesDialog dialog


CCdlPackagesDialog::CCdlPackagesDialog(CWnd* pParent /*=NULL*/)
	: CAddRemoveDialog(IDD_CDL_PACKAGES, pParent)
{
	//{{AFX_DATA_INIT(CCdlPackagesDialog)
	m_strPackageDescription = _T("");
	//}}AFX_DATA_INIT
}


void CCdlPackagesDialog::DoDataExchange(CDataExchange* pDX)
{
	CAddRemoveDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCdlPackagesDialog)
	DDX_Control(pDX, IDC_PACKAGE_VER, m_cboPackageVersion);
	DDX_Text(pDX, IDC_PACKAGE_DESC, m_strPackageDescription);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCdlPackagesDialog, CAddRemoveDialog)
	//{{AFX_MSG_MAP(CCdlPackagesDialog)
	ON_LBN_SELCHANGE(IDC_ADDREMOVE_LIST1, OnSelchangeList1)
	ON_LBN_SELCHANGE(IDC_ADDREMOVE_LIST2, OnSelchangeList2)
	ON_CBN_SELCHANGE(IDC_PACKAGE_VER, OnSelchangePackageVersion)
	ON_BN_CLICKED(IDC_ADDREMOVE_ADD, OnAdd)
	ON_BN_CLICKED(IDC_ADDREMOVE_REMOVE, OnRemove)
	ON_LBN_DBLCLK(IDC_ADDREMOVE_LIST1, OnDblclkList1)
	ON_LBN_DBLCLK(IDC_ADDREMOVE_LIST2, OnDblclkList2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCdlPackagesDialog message handlers

void CCdlPackagesDialog::OnSelchangeList1 ()
{
	CAddRemoveDialog::OnSelchangeList1 ();
	UpdatePackageDescription ();
	UpdateVersionList ();
	UpdateHardwareSelectionFlag ();
}

void CCdlPackagesDialog::OnSelchangeList2 () 
{
	CAddRemoveDialog::OnSelchangeList2 ();
	UpdatePackageDescription ();
	UpdateVersionList ();
	UpdateHardwareSelectionFlag ();
}

void CCdlPackagesDialog::UpdateVersionList ()
{
	m_cboPackageVersion.ResetContent (); // clear the version combo box

	CListBox * pListBox = NULL;
	int nListSelCount = m_List1.GetSelCount ();
	if (nListSelCount)
	{
		pListBox = &m_List1;
	}
	else
	{
		nListSelCount = m_List2.GetSelCount ();
		if (nListSelCount)
			pListBox = &m_List2;
	}

	if (pListBox) // if there are packages selected
	{
		std::list<std::string> common_versions;
		bool bCommonSelectedVersion = true;
		int nCommonVersionIndex=-1;

		// retrieve the list box indices of the selected packages

		int * arnIndices = new int [nListSelCount];
		pListBox->GetSelItems (nListSelCount, arnIndices);
		for (int nIndex = 0; nIndex < nListSelCount; nIndex++) // for each selected package
		{
			// retrieve the first package alias

			CString strPackageAlias;
			pListBox->GetText (arnIndices [nIndex], strPackageAlias);

			// retrieve the dialog item array index for use in
			// comparing current version strings

			const int nVersionIndex = (int) pListBox->GetItemData (arnIndices [nIndex]);

			// retrieve the installed version array

			TRACE (_T("Retrieving versions for '%s'\n"), strPackageAlias);
			CConfigToolDoc * pDoc = CConfigTool::GetConfigToolDoc ();
			const std::vector<std::string>& versions = pDoc->GetCdlPkgData ()->get_package_versions (CUtils::UnicodeToStdStr (pDoc->GetPackageName (strPackageAlias)));

			if (0 == nIndex) // if this is the first selected package
			{
				// use the version array to initialise a linked list of version
				// strings held in common between the selected packages
				for (unsigned int uCount = 0; uCount < versions.size (); uCount++)
				{
					TRACE (_T("Adding common version '%s'\n"), CString (versions [uCount].c_str ()));
					common_versions.push_back (versions [uCount]);
				}
				nCommonVersionIndex = nVersionIndex; // save the item array index
			}
			else // this is not the first selected package
			{
				std::list<std::string>::iterator i_common_versions = common_versions.begin ();
				while (i_common_versions != common_versions.end ()) // iterate through the common versions
				{
					if (versions.end () == std::find (versions.begin (), versions.end (), * i_common_versions)) // if the common version is not in the versions list
					{
						TRACE (_T("Removing common version '%s'\n"), CString (i_common_versions->c_str ()));
						common_versions.erase (i_common_versions++); // remove the version from the common versions list
					}
					else
					{
						i_common_versions++;
					}
				}
				if (bCommonSelectedVersion) // if the selected versions of all preceding packages are identical
				{
					// check if the selected version of this package matches that of the preceding ones
					bCommonSelectedVersion = (m_arstrVersions [nVersionIndex] == m_arstrVersions [nCommonVersionIndex]);
				}
			}
		}

		// add the common versions to the version combo box

		std::list<std::string>::iterator i_common_versions;
		for (i_common_versions = common_versions.begin (); i_common_versions != common_versions.end (); i_common_versions++)
		{
			TRACE (_T("Adding version '%s'\n"), CString (i_common_versions->c_str ()));
			m_cboPackageVersion.AddString (CString (i_common_versions->c_str ()));
		}

		// select the common current version (if any) in the version combo box

		if (bCommonSelectedVersion)
		{
			TRACE (_T("Selecting version '%s'\n"), m_arstrVersions [nCommonVersionIndex]);
			m_cboPackageVersion.SelectString (-1, m_arstrVersions [nCommonVersionIndex]);
		}

		// enable the version combo box only if there are multiple common versions

		m_cboPackageVersion.EnableWindow (common_versions.size () > 1);

		delete [] arnIndices;
	}
	else // there are no packages selected
	{
		m_cboPackageVersion.EnableWindow (FALSE); // disable the version combo box
	}
}

void CCdlPackagesDialog::UpdatePackageDescription ()
{
	CConfigToolDoc * pDoc = CConfigTool::GetConfigToolDoc ();

	CListBox * pListBox = NULL;
	if (1 == m_List1.GetSelCount ())
		pListBox = &m_List1;
	else if (1 == m_List2.GetSelCount ())
		pListBox = &m_List2;

	if (pListBox)
	{
		int nIndex;
		pListBox->GetSelItems (1, &nIndex);
		CString strPackageAlias;
		pListBox->GetText (nIndex, strPackageAlias);
		m_strPackageDescription = pDoc->GetCdlPkgData ()->get_package_description (CUtils::UnicodeToStdStr (pDoc->GetPackageName (strPackageAlias))).c_str ();
		m_strPackageDescription = CUtils::StripExtraWhitespace (m_strPackageDescription);
	}
	else
	{
		m_strPackageDescription = _T("");
	}

	UpdateData (FALSE);
}

void CCdlPackagesDialog::Insert (LPCTSTR pszItem, bool bAdded, LPCTSTR pszDesc /* = NULL */, LPCTSTR pszVersion /* = NULL */)
{
	TRACE (_T("CCdlPackagesDialog::Insert() adding package %s version '%s'\n"), pszItem, pszVersion);
	CAddRemoveDialog::Insert (pszItem, bAdded, pszDesc);
	m_arstrVersions.Add (pszVersion);
}


BOOL CCdlPackagesDialog::OnInitDialog() 
{
    m_arbSel=new int [GetCount ()];
	CeCosDialog::OnInitDialog ();

	// prepare for the measurement of listbox item widths
	int nWidth = 0;
    CDC * pDC = m_List1.GetDC ();
	CFont * pOldFont = pDC->SelectObject (m_List1.GetFont ());

	// add the items to the listboxes and measure their widths in pixels
    for (int i = GetCount () - 1; i >= 0; --i)
	{
		TRACE (_T("Adding item '%s' index %d\n"), m_arstrItems [i], i);
        CListBox & lb = m_arnItems [i] ? m_List2 : m_List1; // determine which listbox
        lb.SetItemData (lb.AddString (m_arstrItems [i]), (DWORD) i); // add the item
		CSize sizeText = pDC->GetTextExtent (m_arstrItems [i]); // measure the width of the item
		nWidth = max (nWidth, sizeText.cx); // record the maximum width of items to date
    }

	// restore the device context following measurements
    pDC->SelectObject (pOldFont);

	// enable horizontal scrolling if necessary, assuming the
	// listboxes have identical widths and accommodating a
	// 2 pixel border at each side of each listbox
	m_List1.SetHorizontalExtent (nWidth + 4);
	m_List2.SetHorizontalExtent (nWidth + 4);

	// enable listboxes only if they have any content
    m_Add.EnableWindow (m_List1.GetSelCount () > 0);
    m_Remove.EnableWindow (m_List2.GetSelCount () > 0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCdlPackagesDialog::OnSelchangePackageVersion() 
{
	if (CB_ERR == m_cboPackageVersion.GetCurSel ()) // if there is no version selection
		return; // do nothing

	CListBox * pListBox = NULL;
	int nListSelCount = m_List1.GetSelCount ();
	if (nListSelCount)
	{
		pListBox = &m_List1;
	}
	else
	{
		nListSelCount = m_List2.GetSelCount ();
		if (nListSelCount)
			pListBox = &m_List2;
	}

	ASSERT (pListBox);

	// retrieve the list box indices of the selected packages

	int * arnIndices = new int [nListSelCount];
	pListBox->GetSelItems (nListSelCount, arnIndices);

	for (int nIndex = 0; nIndex < nListSelCount; nIndex++) // for each selected package
	{
		// set the package version to that specified in the version combo box
		m_cboPackageVersion.GetLBText (m_cboPackageVersion.GetCurSel (), m_arstrVersions [pListBox->GetItemData (arnIndices [nIndex])]);
		TRACE (_T("Version '%s' selected for package %s\n"), m_arstrVersions [pListBox->GetItemData (arnIndices [nIndex])], m_arstrItems [pListBox->GetItemData (arnIndices [nIndex])]);
	}
	delete [] arnIndices;
}

void CCdlPackagesDialog::OnAdd()
{
	if (m_bHardwarePackageSelected)
		HardwarePackageMessageBox ();
	else
		CAddRemoveDialog::OnAdd ();
}

void CCdlPackagesDialog::OnRemove()
{
	if (m_bHardwarePackageSelected)
		HardwarePackageMessageBox ();
	else
		CAddRemoveDialog::OnRemove ();	
}

void CCdlPackagesDialog::OnDblclkList1()
{
	if (m_bHardwarePackageSelected)
		HardwarePackageMessageBox ();
	else
		CAddRemoveDialog::OnDblclkList1 ();
}

void CCdlPackagesDialog::OnDblclkList2()
{
	if (m_bHardwarePackageSelected)
		HardwarePackageMessageBox ();
	else
		CAddRemoveDialog::OnDblclkList2 ();
}

CString CCdlPackagesDialog::GetVersion(LPCTSTR pszItem)
{
    for (int nCount = GetCount () - 1; nCount >= 0; --nCount)
	{
        if (m_arstrItems [nCount] == pszItem)
		{
            return m_arstrVersions [nCount];
        }
    }
    ASSERT (false);
    return _T("");
}

void CCdlPackagesDialog::UpdateHardwareSelectionFlag()
{
	m_bHardwarePackageSelected = false;

	CListBox * pListBox = NULL;
	int nListSelCount = m_List1.GetSelCount ();
	if (nListSelCount)
	{
		pListBox = &m_List1;
	}
	else
	{
		nListSelCount = m_List2.GetSelCount ();
		if (nListSelCount)
			pListBox = &m_List2;
	}

	if (pListBox) // if there are packages selected
	{
		CConfigToolDoc * pDoc = CConfigTool::GetConfigToolDoc ();

		// retrieve the list box indices of the selected packages

		int * arnIndices = new int [nListSelCount];
		pListBox->GetSelItems (nListSelCount, arnIndices);

		for (int nIndex = 0; nIndex < nListSelCount; nIndex++) // for each selected package
		{
			CString strPackageAlias;
			pListBox->GetText (arnIndices [nIndex], strPackageAlias);

			// check if the package is a hardware package

			TRACE (_T("Checking '%s' for hardware status\n"), strPackageAlias);
			if (pDoc->GetCdlPkgData ()->is_hardware_package (CUtils::UnicodeToStdStr (pDoc->GetPackageName (strPackageAlias))))
			{
				m_bHardwarePackageSelected = true;
				break;
			}
		}

		delete [] arnIndices;
	}
}

void CCdlPackagesDialog::HardwarePackageMessageBox()
{
	AfxMessageBox (_T("Add and remove hardware packages by selecting a new hardware template."));
}
