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
#if !defined(AFX_RULESLIST_H__AEF48733_8D31_11D3_A535_00A0C949ADAC__INCLUDED_)
#define AFX_RULESLIST_H__AEF48733_8D31_11D3_A535_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertiesList.h : header file
//

#define INCLUDEFILE "cdl.hxx"
#include "IncludeSTL.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertiesList window
class CConfigItem;
class CPropertiesList : public CListCtrl
{
// Construction
public:
	CPropertiesList();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertiesList)
	//}}AFX_VIRTUAL

// Implementation
public:
  void Fill(CConfigItem *pti);
	enum FieldType {Type, Value, DefaultValue, Macro, File, URL, Enabled, MAXFIELDTYPE};
  int SetItem (FieldType f, LPCTSTR pszValue);
	virtual ~CPropertiesList();
	void RefreshValue();
	// Generated message map functions
protected:
	int m_nOnSizeRecursionCount;
	int SetItem (LPCTSTR pszItem,LPCTSTR pszValue,int nInsertAs,int nRepeat=1);
	int m_nMaxValueWidth;
	int SetProperty (LPCTSTR pszValue, CdlProperty prop);
  int m_nFirstProperty;
  int SetItemTextGrow(int nItem, LPCTSTR lpszItem);
	static bool PropertyInConflictsList (CdlProperty property, const std::list<CdlConflict> & conflicts);
	static const LPCTSTR FieldTypeImage[MAXFIELDTYPE];
  CPen m_GrayPen;
  static const std::string CPropertiesList::visible_properties [];
  static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) { 
    return ((CPropertiesList*)lParamSort)->CompareFunc(lParam1,lParam2);
  }
  int CompareFunc(LPARAM lParam1, LPARAM lParam2);
  CConfigItem *m_pti;
	double m_fWidth;
  enum {NCOLS=2};
  double m_f[NCOLS]; // relative proportions of columns
	LPARAM m_nLastCol;
  //{{AFX_MSG(CPropertiesList)
  afx_msg void OnTrack(NMHEADER *pNMHeader, LRESULT* pResult) ;
  afx_msg void OnEndTrack(NMHEADER *pNMHeader, LRESULT* pResult) ;
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RULESLIST_H__AEF48733_8D31_11D3_A535_00A0C949ADAC__INCLUDED_)
