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
// StringEdit.cpp : implementation file
//
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the masked edit control as used 
//				in integer in-cell edits by the control view.
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include "stdafx.h"
#include "StringEdit.h"
#include "MultiLineEditDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStringEdit

CStringEdit::CStringEdit(LPCTSTR pszInitialValue):
  CCellEdit(pszInitialValue),
  m_bDoubleClickEdit(false)
  //,CI(_T("Double-click to edit in a dialog"))
{
}

CStringEdit::~CStringEdit()
{
}


BEGIN_MESSAGE_MAP(CStringEdit, CCellEdit)
	//{{AFX_MSG_MAP(CStringEdit)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStringEdit message handlers

void CStringEdit::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
  if(m_bDoubleClickEdit){
    UNUSED_ALWAYS(nFlags);
    UNUSED_ALWAYS(point);
    CMultiLineEditDialog dlg(NULL,m_idEdit);
    GetWindowText(dlg.m_strText);
    // Change /n to /r/n endings
    dlg.m_strText.Replace(_T("\n"),_T("\r\n"));
    //CI.Reset();
    if(IDOK==dlg.DoModal()){
      // Change /r/n to /n endings
      dlg.m_strText.Replace(_T("\r\n"),_T("\n"));
      SendMessage(WM_SETTEXT,0,(LPARAM)(LPCTSTR)dlg.m_strText);			
    }
    //CI.Set(_T("Double-click to edit in a dialog"));
    SetFocus();
  } else {
    Default();
  }
}

int CStringEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCellEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

