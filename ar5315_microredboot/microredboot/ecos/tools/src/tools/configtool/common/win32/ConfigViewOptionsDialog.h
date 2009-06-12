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
#if !defined(AFX_CONFIGVIEWOPTIONSDIALOG_H__ECFB5BD6_09E2_11D3_BF7C_00A0C949ADAC__INCLUDED_)
#define AFX_CONFIGVIEWOPTIONSDIALOG_H__ECFB5BD6_09E2_11D3_BF7C_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigViewOptionsDialog.h : header file
//

#include "resource.h"
#include "eCosDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CConfigViewOptionsDialog dialog

class CConfigViewOptionsDialog : public CeCosDialog
{
// Construction
public:
	CConfigViewOptionsDialog();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigViewOptionsDialog)
	//enum { IDD = IDD_CONFIGURATION_VIEW_OPTIONS_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigViewOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigViewOptionsDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGVIEWOPTIONSDIALOG_H__ECFB5BD6_09E2_11D3_BF7C_00A0C949ADAC__INCLUDED_)
