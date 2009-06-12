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
#if !defined(AFX_ADDREMOVEDIALOG_H__6D1D20B4_2BA5_11D3_8567_9AA54FC2F74D__INCLUDED_)
#define AFX_ADDREMOVEDIALOG_H__6D1D20B4_2BA5_11D3_8567_9AA54FC2F74D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddRemoveDialog.h : header file
//
#include "resource.h"
#include "eCosDialog.h"
/////////////////////////////////////////////////////////////////////////////
// CAddRemoveDialog dialog

class CAddRemoveDialog : public CeCosDialog
{
// Construction
private:
	void AfxDataInit();
public:
	void SetHorizontalResizingLimit (int nPercent=100);
	void SetMessage (LPCTSTR pszText1,LPCTSTR pszText2);
	void SetCaption (LPCTSTR pszCaption);
    void SetSortMode (bool bSort) { m_bSort=bSort; }
	
    // Use this function to prepopulate the two list boxes:
    // The optional third parameter sets the descriptions (to be displayed
    // when the given item is the sole selected in a listbox)
	void Insert (LPCTSTR pszItem,bool bAdded,LPCTSTR pszDesc=NULL);
	
    // Use these functions to determine results after the dialog has been run:
    // (will assert for items not previously inserted)
    bool IsAdded(LPCTSTR pszItem) const;
    bool IsAdded(int nIndex) const { return 0!=m_arnItems[nIndex]; }
    
    // Can be used at any time:
    int GetCount() const { return m_arnItems.GetSize(); }

    CAddRemoveDialog(CWnd* pParent = NULL);   // standard constructor
    CAddRemoveDialog(UINT nIDTemplate, CWnd* pParent = NULL); // template constructor

// Dialog Data
	//{{AFX_DATA(CAddRemoveDialog)
	enum { IDD = IDD_ADDREMOVE_DIALOG };
	CEdit   	m_wndDesc;
	CListBox	m_List2;
	CListBox	m_List1;
	CButton 	m_Add;
	CButton 	m_Remove;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddRemoveDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_nHorizontalResizingLimit;
	bool m_bHaveDescs;
	void UpdateDescription (CListBox &lb);
    enum MoveType {Move,Stretch,Up};
	void MoveChild (UINT id,int delta,MoveType how);
	void SetSortMode(CListBox &lb);
	int * m_arbSel;
	CString m_strMessage1,m_strMessage2;
	CString m_strCaption;

    // Move item from lb1 to lb2
	void Xfer (CListBox &lb1,CListBox &lp2,int nItem);
    // Clear all selected items in a list box:
	static void ClearSel (CListBox &lb);
    // I have doubleclicked on lb1.  Item will be moved to lb2:
	void OnDblclk(CListBox &lb1,CListBox &lb2);
    // Move all selected items from lb1 to lb2:
	int Add (CListBox &lb1,CListBox &lb2);

    CStringArray m_arstrItems; // List of items
    CStringArray m_arstrDescs; // List of descriptions
    CDWordArray  m_arnItems;   // Whether selected (1) or not (0)

    bool m_bSort;              // Whether listboxes should be sorted

	// Generated message map functions
	//{{AFX_MSG(CAddRemoveDialog)
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList1();
	afx_msg void OnDblclkList2();
	afx_msg void OnSelchangeList1();
	afx_msg void OnSelchangeList2();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDREMOVEDIALOG_H__6D1D20B4_2BA5_11D3_8567_9AA54FC2F74D__INCLUDED_)
