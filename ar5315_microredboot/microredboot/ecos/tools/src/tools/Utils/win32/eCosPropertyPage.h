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
#if !defined(AFX_ECOSPROPERTYPAGE_H__900AB3BE_8321_11D3_A534_00A0C949ADAC__INCLUDED_)
#define AFX_ECOSPROPERTYPAGE_H__900AB3BE_8321_11D3_A534_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// eCosPropertyPage.h : header file
//

#ifdef PLUGIN
  #include "ide.win32.h"
  #include "ide.guicommon.h"
#else
  #include "CSHPropertyPage.h"
#endif

#include "eCosDialog.h"
/////////////////////////////////////////////////////////////////////////////
// CeCosPropertyPage dialog

class CeCosPropertyPage : public CCSHPropertyPage
{
	DECLARE_DYNCREATE(CeCosPropertyPage)
// Construction
public:
  CeCosPropertyPage():CCSHPropertyPage(){};
  CeCosPropertyPage(UINT nIDTemplate, UINT nIDCaption = 0 ):CCSHPropertyPage(nIDTemplate, nIDCaption){}
  CeCosPropertyPage( LPCTSTR lpszTemplateName, UINT nIDCaption = 0 ):CCSHPropertyPage(lpszTemplateName, nIDCaption){}

// PropertyPage Data
	//{{AFX_DATA(CeCosPropertyPage)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CeCosPropertyPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  virtual UINT HelpID (DWORD dwCtrlID) const;
  virtual CString CSHFile() const;
  void OnContextMenu(CWnd* pWnd, CPoint point);
  BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	// Generated message map functions
	//{{AFX_MSG(CeCosPropertyPage)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ECOSPROPERTYPAGE_H__900AB3BE_8321_11D3_A534_00A0C949ADAC__INCLUDED_)
