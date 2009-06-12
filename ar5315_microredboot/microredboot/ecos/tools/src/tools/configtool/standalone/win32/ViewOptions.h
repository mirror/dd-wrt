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
#if !defined(AFX_VIEWOPTIONS_H__3B88C71C_2008_11D2_80C1_00A0C949ADAC__INCLUDED_)
#define AFX_VIEWOPTIONS_H__3B88C71C_2008_11D2_80C1_00A0C949ADAC__INCLUDED_

#include "FileName.h"	// Added by ClassView
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ViewOptions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewOptions dialog
#include "MainFrm.h"
#include "resource.h"
#include "eCosDialog.h"

class CViewOptions : public CeCosDialog
{

// Construction
public:
	CViewOptions();
	~CViewOptions();

// Dialog Data
	//{{AFX_DATA(CViewOptions)
	enum { IDD = IDD_VIEW_SETTINGS_DIALOG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CViewOptions)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	static CFileName HeaderFileAssociation();
	CMainFrame::PaneType m_FontChosen;
	LOGFONT m_lf;
	static const CFileName strHeaderfileAssociation;
	// Generated message map functions
	//{{AFX_MSG(CViewOptions)
	afx_msg void OnRadioAssociated();
	afx_msg void OnRadioCustom();
	afx_msg void OnBrowse();
	virtual BOOL OnInitDialog();
	afx_msg void OnRadioInternalBrowser();
	afx_msg void OnRadioAssociatedBrowser();
	afx_msg void OnRadioCustomBrowser();
	afx_msg void OnBrowseBrowser();
	virtual void OnOK();
	afx_msg void OnFont();
	afx_msg void OnSelchangePanecombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWOPTIONS_H__3B88C71C_2008_11D2_80C1_00A0C949ADAC__INCLUDED_)
