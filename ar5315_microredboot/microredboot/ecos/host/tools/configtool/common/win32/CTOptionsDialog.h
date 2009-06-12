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
// Description:	Interface of the Configuration -> Options View tab
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_OUTPUTPAGE_H__3B88C71B_2008_11D2_80C1_00A0C949ADAC__INCLUDED_)
#define AFX_OUTPUTPAGE_H__3B88C71B_2008_11D2_80C1_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OutputPage.h : header file
//

#include "resource.h"
#include "eCosDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CToolsOptionsDialog dialog

class CToolsOptionsDialog : public CeCosDialog
{

// Construction
public:
	CToolsOptionsDialog();
	~CToolsOptionsDialog();

// Dialog Data
	//{{AFX_DATA(CToolsOptionsDialog)
	enum { IDD = IDD_TOOLS_OPTIONS_DIALOG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CToolsOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetButtons();
	// Generated message map functions
	//{{AFX_MSG(CToolsOptionsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDeferred();
	afx_msg void OnImmediate();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTPAGE_H__3B88C71B_2008_11D2_80C1_00A0C949ADAC__INCLUDED_)
