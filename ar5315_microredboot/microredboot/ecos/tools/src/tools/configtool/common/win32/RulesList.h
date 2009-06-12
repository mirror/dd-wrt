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
#if !defined(AFX_PROPERTIESLIST_H__AEF48733_8D31_11D3_A535_00A0C949ADAC__INCLUDED_)
#define AFX_PROPERTIESLIST_H__AEF48733_8D31_11D3_A535_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertiesList.h : header file
//

#include "TTListCtrl.h"

#define INCLUDEFILE "cdl.hxx"
#include "IncludeSTL.h"
/////////////////////////////////////////////////////////////////////////////
// CRulesList window
class CConfigItem;
class CRulesList : public CTTListCtrl
{
// Construction
public:
	CRulesList();

// Attributes
public:

// Operations
public:
  void AddConflict (const CdlConflict& conflict);
  void AddConflicts (const std::list<CdlConflict>& conflicts);
  void AddConflicts (const std::vector<CdlConflict>& conflicts);
  CConfigItem *AssociatedItem (int nRow,int nCol);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRulesList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRulesList();
	// Generated message map functions
protected:
  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) { 
    return ((CRulesList*)lParamSort)->CompareFunc(lParam1,lParam2);
  }
  int CompareFunc(LPARAM lParam1, LPARAM lParam2);

  double m_fWidth;
  enum {NCOLS=3};
  double m_f[NCOLS]; // relative proportions of columns
	LPARAM m_nLastCol;
  //{{AFX_MSG(CRulesList)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEndTrack(NMHEADER *pNMHeader, LRESULT* pResult) ;
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTIESLIST_H__AEF48733_8D31_11D3_A535_00A0C949ADAC__INCLUDED_)
