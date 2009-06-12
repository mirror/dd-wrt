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
#if !defined(AFX_CSHCOMMON_H__B9FD78FA_DE33_11D3_A53F_00A0C949ADAC__INCLUDED_)
#define AFX_CSHCOMMON_H__B9FD78FA_DE33_11D3_A53F_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CSHCommon.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCSHCommon window

class CCSHCommon
{
// Construction
public:
	CCSHCommon();

// Attributes
public:

// Operations
public:

  // What CSHFile() will probably return something starting with this path
  static void  SetCSHFilePath(LPCTSTR pszCSHFilePath) { m_strCSHFilePath=pszCSHFilePath; }
  static const CString GetCSHFilePath() { return m_strCSHFilePath; }

  // Used by dialogs handling NM_RCLICK notifications to display a context menu to avoid getting two in a row.
  void SuppressNextContextMenuMessage() { m_bSupressContextMenu=true; }

// Implementation
public:
	virtual ~CCSHCommon();

protected:
	bool OnContextMenu (CWnd *pDialog, CPoint pt, UINT idHelp);
  enum {ID_WHATS_THIS=42};

  bool m_bSupressContextMenu;

  static CString m_strCSHFilePath;

	static void DisplayHelp (HWND hCtrl, UINT ids, HINSTANCE hInst);

  static CWnd *WndFromPoint(CWnd *pDialog,CWnd* pWnd,CPoint pt);
	CWnd* m_pwndContext;

  bool FilterMessage(UINT &message, WPARAM &wParam,LPARAM &lParam,LRESULT *&pResult);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSHCOMMON_H__B9FD78FA_DE33_11D3_A53F_00A0C949ADAC__INCLUDED_)
