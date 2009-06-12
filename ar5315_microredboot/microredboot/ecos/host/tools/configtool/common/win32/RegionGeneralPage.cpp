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
//=================================================================
//
//        RegionGeneralPage.cpp
//
//        Memory Layout Tool region general property page class
//
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     John Dallaway
// Contact(s):    jld
// Date:          1998/07/29 $RcsDate$ {or whatever}
// Version:       0.00+  $RcsVersion$ {or whatever}
// Purpose:       Provides a derivation of the MFC CeCosPropertyPage class for
//                general region property selection
// See also:      RegionGeneralPage.h
// Known bugs:    <UPDATE_ME_AT_RELEASE_TIME>
//
//####DESCRIPTIONEND####

#include "stdafx.h"
#include "RegionGeneralPage.h"
#include "ConfigTool.h"
#include "memmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRegionGeneralPage property page

IMPLEMENT_DYNCREATE(CRegionGeneralPage, CeCosPropertyPage)

CRegionGeneralPage::CRegionGeneralPage() : CeCosPropertyPage(CRegionGeneralPage::IDD)
{
	//{{AFX_DATA_INIT(CRegionGeneralPage)
	m_strRegionName = _T("");
	m_bRegionReadOnly = FALSE;
	m_strRegionStartAddress = _T("");
	m_strRegionSize = _T("");
	//}}AFX_DATA_INIT
}

CRegionGeneralPage::~CRegionGeneralPage()
{
}

void CRegionGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CeCosPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegionGeneralPage)
	DDX_Control(pDX, IDC_REGION_GENERAL_SIZE, m_edtRegionSize);
	DDX_Control(pDX, IDC_REGION_GENERAL_START_ADDRESS, m_edtRegionStartAddress);
	DDX_Control(pDX, IDC_REGION_GENERAL_NAME, m_edtRegionName);
	DDX_Text(pDX, IDC_REGION_GENERAL_NAME, m_strRegionName);
	DDX_Check(pDX, IDC_REGION_GENERAL_READ_ONLY, m_bRegionReadOnly);
	DDX_Text(pDX, IDC_REGION_GENERAL_START_ADDRESS, m_strRegionStartAddress);
	DDX_Text(pDX, IDC_REGION_GENERAL_SIZE, m_strRegionSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegionGeneralPage, CeCosPropertyPage)
	//{{AFX_MSG_MAP(CRegionGeneralPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegionGeneralPage message handlers

BOOL CRegionGeneralPage::OnKillActive() 
{
    if (! UpdateData (TRUE))
        return FALSE;

    if ((m_strRegionName == _T("")) || (m_strRegionName.FindOneOf (LD_ILLEGAL_CHARS) != -1))
    {
        AfxMessageBox (IDS_VALIDATE_REGION_NAME);
        m_edtRegionName.SetFocus ();
        return FALSE;
    }

    // convert address and size information to a DWORD representation

    TCHAR strDummy [2]; // holds any stray character following a hex value

    if (_stscanf (m_strRegionStartAddress, _T("%lx%1s"), &m_dwRegionStartAddress, strDummy) != 1)
    {
        AfxMessageBox (IDS_VALIDATE_REGION_START_ADDRESS);
        m_edtRegionStartAddress.SetFocus ();
        m_edtRegionStartAddress.SetSel (0, -1); // select all text
        return FALSE;
    }

    if ((_stscanf (m_strRegionSize, _T("%lx%1s"), &m_dwRegionSize, strDummy) != 1) || (m_dwRegionSize < 1))
    {
        AfxMessageBox (IDS_VALIDATE_REGION_SIZE);
        m_edtRegionSize.SetFocus ();
        m_edtRegionSize.SetSel (0, -1); // select all text
        return FALSE;
    }

    return CeCosPropertyPage::OnKillActive();
}

BOOL CRegionGeneralPage::OnSetActive() 
{
    // generate hex strings for display

    if (m_dwRegionSize == 0) // a new region
    {
        m_strRegionStartAddress = _T("");
        m_strRegionSize = _T("");
    }
    else // modify an existing region
    {
        m_strRegionStartAddress.Format (_T("%08lX"), m_dwRegionStartAddress);
        m_strRegionSize.Format (_T("%lX"), m_dwRegionSize);
    }

    if (! UpdateData (FALSE))
        return FALSE;
	
	return CeCosPropertyPage::OnSetActive();
}

