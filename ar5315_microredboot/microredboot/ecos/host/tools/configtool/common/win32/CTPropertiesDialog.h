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
// Description:	Interface of the Properties Dialog
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_PROPERTIESDIALOG_H__45994E31_3B59_11D2_80C1_00A0C949ADAC__INCLUDED_)
#define AFX_PROPERTIESDIALOG_H__45994E31_3B59_11D2_80C1_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PropertiesDialog.h : header file
//
/////////////////////////////////////////////////////////////////////////////
class CConfigItem;
#include "resource.h"
#include "PropertiesList.h"
#include "eCosDialog.h"

// CCTPropertiesDialog dialog
class CCTPropertiesDialog : public CeCosDialog
{
// Construction
public:
	CCTPropertiesDialog(CConfigItem &ti);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCTPropertiesDialog)
	enum { IDD = IDD_VIEW_PROPERTIES_DIALOG };
	CPropertiesList	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCTPropertiesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  CConfigItem &m_ti;
	// Generated message map functions
	//{{AFX_MSG(CCTPropertiesDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTIESDIALOG_H__45994E31_3B59_11D2_80C1_00A0C949ADAC__INCLUDED_)
