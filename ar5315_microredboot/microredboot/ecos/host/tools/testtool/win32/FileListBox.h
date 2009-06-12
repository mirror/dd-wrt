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
#if !defined(AFX_FILELISTBOX_H__AC331EDD_1201_11D3_A507_00A0C949ADAC__INCLUDED_)
#define AFX_FILELISTBOX_H__AC331EDD_1201_11D3_A507_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileListBox window

class CFileListBox : public CCheckListBox
{
// Construction
public:
	CFileListBox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileListBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFileListBox();

	// Generated message map functions
protected:
	UINT m_nIndex;
	//{{AFX_MSG(CFileListBox)
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRemove();
	afx_msg void OnAdd();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILELISTBOX_H__AC331EDD_1201_11D3_A507_00A0C949ADAC__INCLUDED_)
