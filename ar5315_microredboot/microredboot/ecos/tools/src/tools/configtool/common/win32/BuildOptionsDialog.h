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
#if !defined(AFX_BUILDOPTIONSDIALOG_H__92CF151B_8318_11D3_A534_00A0C949ADAC__INCLUDED_)
#define AFX_BUILDOPTIONSDIALOG_H__92CF151B_8318_11D3_A534_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BuildOptionsDialog.h : header file
//
class CConfigItem;
/////////////////////////////////////////////////////////////////////////////
// CBuildOptionsDialog dialog

#include "eCosDialog.h"
#define INCLUDEFILE "cdl.hxx"
#include "IncludeSTL.h"
#include "resource.h"

class CBuildOptionsDialog : public CeCosDialog
{
// Construction
public:
	CBuildOptionsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBuildOptionsDialog)
	enum { IDD = IDD_BUILD_OPTIONS_DIALOG };
	CListBox	m_List;
	CTreeCtrl	m_Tree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBuildOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HTREEITEM m_hCurrent;
	void Redisplay (HTREEITEM h);
	CImageList m_il;
  void Create(CConfigItem *pti,HTREEITEM hParent);
  typedef std::vector<CdlBuildInfo_Loadable> EntriesArray;
  const EntriesArray &arEntries;
  
	// Generated message map functions
	//{{AFX_MSG(CBuildOptionsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUILDOPTIONSDIALOG_H__92CF151B_8318_11D3_A534_00A0C949ADAC__INCLUDED_)
