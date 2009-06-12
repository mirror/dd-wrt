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
//        SectionGeneralPage.h
//
//        Memory Layout Tool section general property page interface
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     John Dallaway
// Contact(s):    jld
// Date:          1998/07/29 $RcsDate$ {or whatever}
// Version:       0.00+  $RcsVersion$ {or whatever}
// Purpose:       Provides a derivation of the MFC CeCosPropertyPage class for
//                general section property selection
// See also:      SectionGeneralPage.cpp
// Known bugs:    <UPDATE_ME_AT_RELEASE_TIME>
//
//####DESCRIPTIONEND####

#if !defined(AFX_SECTIONGENERALPAGE_H__FA2F38F5_1FA8_11D2_BFBB_00A0C9554250__INCLUDED_)
#define AFX_SECTIONGENERALPAGE_H__FA2F38F5_1FA8_11D2_BFBB_00A0C9554250__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SectionGeneralPage.h : header file
//

#include "resource.h"
#include "eCosPropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// CSectionGeneralPage dialog

class CSectionGeneralPage : public CeCosPropertyPage
{
	DECLARE_DYNCREATE(CSectionGeneralPage)

// Construction
public:
	CSectionGeneralPage();
	~CSectionGeneralPage();
    bool m_bFinalAbsolute; // if final location is an absolute location (not a relative location)
    bool m_bNameLinkerDefined; // if the section is linker-defined (not user-defined)
    DWORD m_dwAlignment; // the section alignment
    DWORD m_dwFinalAddress; // the final section address 
    DWORD m_dwSectionSize; // the section size (0 == unknown)

// Dialog Data
	//{{AFX_DATA(CSectionGeneralPage)
	enum { IDD = IDD_SECTION_GENERAL };
	CComboBox	m_cboAlignment;
	CEdit	m_edtNameUser;
	CComboBox	m_cboNameLinker;
	CEdit	m_edtFinalAddress;
	CComboBox	m_cboFinalRelativeName;
	CButton	m_btnSectionSizeKnown;
	CEdit	m_edtSectionSize;
	CButton	m_btnFinalRelative;
	CButton	m_btnFinalAbsolute;
	CButton	m_btnNameLinkerDefined;
	CButton	m_btnNameUserDefined;
	CString	m_strFinalRelativeName;
	CString	m_strNameUser;
	CString	m_strNameLinker;
	int		m_nAlignment;
	CString	m_strFinalAddress;
	CString	m_strSectionSize;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSectionGeneralPage)
	public:
	virtual BOOL OnKillActive();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSectionGeneralPage)
	afx_msg void OnSectionSizeKnown();
	afx_msg void OnSectionGeneralFinalType();
	virtual BOOL OnInitDialog();
	afx_msg void OnSectionGeneralNameType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SECTIONGENERALPAGE_H__FA2F38F5_1FA8_11D2_BFBB_00A0C9554250__INCLUDED_)
