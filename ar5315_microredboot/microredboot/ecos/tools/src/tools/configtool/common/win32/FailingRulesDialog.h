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
#if !defined(AFX_FAILINGRULESDIALOG_H__E264B972_8875_11D2_BF54_00A0C949ADAC__INCLUDED_)
#define AFX_FAILINGRULESDIALOG_H__E264B972_8875_11D2_BF54_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FailingRulesDialog.h : header file
//

#include "resource.h"
#include "RulesList.h"
#include "eCosDialog.h"
#include "TTListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CFailingRulesDialog dialog
class CConfigItem;
class CdlConflictBody;
class CdlTransactionBody;
class CFailingRulesDialog : public CeCosDialog
{
// Construction
public:
	CFailingRulesDialog (std::list<CdlConflict> conflicts,CdlTransaction=NULL,CPtrArray *parConflictsOfInterest=NULL);
  virtual ~CFailingRulesDialog();
  bool IsSelected (int nIndex);
// Dialog Data
	//{{AFX_DATA(CFailingRulesDialog)
	enum { IDD = IDD_RESOLVE_CONFLICTS_DIALOG };
	CTTListCtrl m_List;
	CRulesList  m_RulesList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFailingRulesDialog)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetButtons();
  struct SolutionInfo {
    int nCount;
    enum {CHECKED=-1,UNCHECKED=-2};
    int arItem[1]; // real size==nCount.
    // Each element of nItem==item index (if selected) or accept bool (if not)
  };
	SolutionInfo & Info (const CdlConflict conflict);
	UINT m_idTimer;
	void SetAll (bool bOnOff);
	CMapPtrToPtr m_Map; // maps conficts to bool array representing fixes
	void AddConflictSolutions (CdlConflict conflict);
	void RemoveConflictSolutions (CdlConflict conflict);
  CStringArray m_arValues;
  const std::list<CdlConflict> m_conflicts;
  CdlTransactionBody *m_Transaction;
  CPtrArray *m_parConflictsOfInterest;
  void OnLocate();
  int m_nContextItem;
  int m_nContextRow;
  BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );

	// Generated message map functions
	//{{AFX_MSG(CFailingRulesDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
  afx_msg void OnReset();
  afx_msg BOOL OnItemChanged(UINT, LPNMLISTVIEW pnmv, LRESULT* pResult);
  afx_msg BOOL OnClick(UINT,LPNMLISTVIEW pnmv, LRESULT* pResult);
  afx_msg BOOL OnSolutionItemChanged(UINT,LPNMLISTVIEW pnmv, LRESULT* pResult);
	afx_msg void OnConflictsNone();
  afx_msg BOOL OnRClick(UINT,LPNMITEMACTIVATE pnmv, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FAILINGRULESDIALOG_H__E264B972_8875_11D2_BF54_00A0C949ADAC__INCLUDED_)
