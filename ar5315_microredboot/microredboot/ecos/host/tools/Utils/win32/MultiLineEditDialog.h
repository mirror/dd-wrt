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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	Interface of the popup control used by the control view
//				for in-cell editing of multi-line items
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_MULTILINEEDITDIALOG_H__DEBF31D5_224E_11D2_80C1_00A0C949ADAC__INCLUDED_)
#define AFX_MULTILINEEDITDIALOG_H__DEBF31D5_224E_11D2_80C1_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MultiLineEditDialog.h : header file
//

#include "eCosDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CMultiLineEditDialog dialog

class CMultiLineEditDialog : public CeCosDialog
{
// Construction
public:
	CMultiLineEditDialog(CWnd* pParent = NULL,UINT idEdit=42);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMultiLineEditDialog)
	CString	m_strText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiLineEditDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	UINT m_idEdit;
  struct DlgItem {
    DLGITEMTEMPLATE dit;
    WORD arclass1, arclass2;
    WORD artitle;
    WORD ardata;
    WORD align;
  };

  struct DlgData {
    DLGTEMPLATE dtdlg;
    WORD armenu;
    WORD wndclass;
    WORD artitle;
    DlgItem ctrl[3];
  };
  static const DlgData data;
  DlgData m_data;
	// Generated message map functions
	//{{AFX_MSG(CMultiLineEditDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTILINEEDITDIALOG_H__DEBF31D5_224E_11D2_80C1_00A0C949ADAC__INCLUDED_)
