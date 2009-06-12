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
#if !defined(AFX_ECOSDIALOG_H__900AB3BE_8321_11D3_A534_00A0C949ADAC__INCLUDED_)
#define AFX_ECOSDIALOG_H__900AB3BE_8321_11D3_A534_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// eCosDialog.h : header file
//

#ifdef PLUGIN
  #include "ide.win32.h"
  #include "ide.guicommon.h"
#else
  #include "CSHDialog.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CeCosDialog dialog

class CeCosDialog : public CCSHDialog
{
// Construction
  friend class CeCosPropertyPage;
  friend class CeCosPropertySheet;
public:
  CeCosDialog( LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL ):  CCSHDialog(lpszTemplateName, pParentWnd){}
  CeCosDialog( UINT nIDTemplate, CWnd* pParentWnd = NULL ):CCSHDialog(nIDTemplate, pParentWnd){}
  CeCosDialog( ):CCSHDialog(){}
  static void AddDialogMap  (UINT  *map) { m_arDialogMaps.Add(map); }

// Dialog Data
	//{{AFX_DATA(CeCosDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CeCosDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

  // Implementation
protected:

  static CPtrArray m_arDialogMaps;
  
  virtual CString CSHFile() const;
  static CString CSHFile(UINT dlgID);
  static DWORD arControlMap[];
  static DWORD arDialogMap[];
  virtual UINT HelpID (DWORD dwCtrlID) const;
  static  UINT HelpID (UINT dlgID,DWORD dwCtrlID);
  void OnContextMenu(CWnd* pWnd, CPoint point);
  virtual HINSTANCE GetInstanceHandle();
  BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	// Generated message map functions
	//{{AFX_MSG(CeCosDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ECOSDIALOG_H__900AB3BE_8321_11D3_A534_00A0C949ADAC__INCLUDED_)
