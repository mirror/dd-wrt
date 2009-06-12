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
//        SectionRelocationPage.h
//
//        Memory Layout Tool section relocation property page interface
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
//                relocation section property selection
// See also:      SectionRelocationPage.cpp
// Known bugs:    <UPDATE_ME_AT_RELEASE_TIME>
//
//####DESCRIPTIONEND####

#if !defined(AFX_SECTIONRELOCATIONPAGE_H__FA2F38F6_1FA8_11D2_BFBB_00A0C9554250__INCLUDED_)
#define AFX_SECTIONRELOCATIONPAGE_H__FA2F38F6_1FA8_11D2_BFBB_00A0C9554250__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SectionRelocationPage.h : header file
//

#include "resource.h"
#include "eCosPropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// CSectionRelocationPage dialog

class CSectionRelocationPage : public CeCosPropertyPage
{
	DECLARE_DYNCREATE(CSectionRelocationPage)

// Construction
public:
	CSectionRelocationPage();
	~CSectionRelocationPage();
    BOOL m_bInitialAbsolute; // if initial location is an absolute location (not a relative location)
    DWORD m_dwInitialAddress;
    BOOL m_bNewSection;

// Dialog Data
	//{{AFX_DATA(CSectionRelocationPage)
	enum { IDD = IDD_SECTION_RELOCATION };
	CButton	m_btnRelocates;
	CButton	m_btnInitialRelative;
	CButton	m_btnInitialAbsolute;
	CEdit	m_edtInitialAddress;
	CComboBox	m_cboInitialRelativeName;
	BOOL	m_bRelocates;
	CString	m_strInitialRelativeName;
	CString	m_strInitialAddress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSectionRelocationPage)
	public:
	virtual BOOL OnKillActive();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSectionRelocationPage)
	afx_msg void OnSectionRelocationRelocates();
	afx_msg void OnSectionRelocationInitialType();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SECTIONRELOCATIONPAGE_H__FA2F38F6_1FA8_11D2_BFBB_00A0C9554250__INCLUDED_)
