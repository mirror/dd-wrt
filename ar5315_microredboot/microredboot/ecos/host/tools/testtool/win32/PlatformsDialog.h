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
#if !defined(AFX_PLATFORMSDIALOG_H__75B523E9_E498_11D3_A546_00A0C949ADAC__INCLUDED_)
#define AFX_PLATFORMSDIALOG_H__75B523E9_E498_11D3_A546_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PlatformsDialog.h : header file
//
#include "testtoolres.h"
#include "eCosDialog.h"
#include "eCosTest.h"

/////////////////////////////////////////////////////////////////////////////
// CPlatformsDialog dialog

class CPlatformsDialog : public CeCosDialog
{
// Construction
  static const LPCTSTR arpszTypes[];
public:
  CeCosTestPlatform *Platform (int i) { return (CeCosTestPlatform *)m_arTargetInfo[i]; }
  unsigned int PlatformCount() const { return m_arTargetInfo.GetSize(); }
	CPlatformsDialog(CWnd* pParent = NULL);   // standard constructor
  void Add(const CeCosTestPlatform &ti);

// Dialog Data
	//{{AFX_DATA(CPlatformsDialog)
	enum { IDD = IDD_TT_PLATFORMS_DIALOG };
	CListCtrl	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlatformsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CPtrArray m_arTargetInfo;

	// Generated message map functions
	//{{AFX_MSG(CPlatformsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddPlatform();
	afx_msg void OnDeletePlatform();
	afx_msg void OnDblclkPlatformList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnModifyPlatform();
	afx_msg void OnKeydownPlatformList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedPlatformList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLATFORMSDIALOG_H__75B523E9_E498_11D3_A546_00A0C949ADAC__INCLUDED_)
