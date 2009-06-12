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
// CellEdit.cpp : implementation file
//

#include "stdafx.h"
#include "CellEdit.h"
#include "cellview.h"
#include "Configtool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCellEdit

CCellEdit::CCellEdit(LPCTSTR pszInitialValue):
  CCell(pszInitialValue)
{
}

CCellEdit::~CCellEdit()
{
}


BEGIN_MESSAGE_MAP(CCellEdit, CCell)
	//{{AFX_MSG_MAP(CCellEdit)
	ON_WM_CREATE()
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCellEdit message handlers

BOOL CCellEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext*) 
{
	return CWnd::CreateEx(WS_EX_CLIENTEDGE,_T("EDIT"), NULL, dwStyle, rect, pParentWnd, nID);
}

int CCellEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCell::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	SendMessage(WM_SETFONT,(WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT),0);
  SendMessage(WM_SETTEXT,0,(LPARAM)(LPCTSTR)m_strInitialValue);
	return 0;
}

void CCellEdit::OnKillFocus(CWnd*)
{
    CConfigTool::GetCellView()->CancelCellEdit();
}

void CCellEdit::OnEditCopy   ()
{
 SendMessage(WM_COPY,0,0);
}

void CCellEdit::OnEditDelete ()
{
  SendMessage(WM_CLEAR,0,0);
}

void CCellEdit::OnEditPaste  ()
{
  SendMessage(WM_PASTE,0,0);
}

void CCellEdit::OnEditCut    ()
{
  SendMessage(WM_CUT,0,0);
}

void CCellEdit::OnUpdateEditCopy   (CCmdUI *pCmdUI)
{
  LRESULT l=SendMessage(EM_GETSEL,0,0);
  pCmdUI->Enable(LOWORD(l)!=HIWORD(l));
}

void CCellEdit::OnUpdateEditDelete (CCmdUI *pCmdUI)
{
  LRESULT l=SendMessage(EM_GETSEL,0,0);
  pCmdUI->Enable(LOWORD(l)!=HIWORD(l));
}

void CCellEdit::OnUpdateEditPaste  (CCmdUI *pCmdUI)
{
  if(OpenClipboard()){
    pCmdUI->Enable(NULL!=::GetClipboardData(CF_TEXT));
    CloseClipboard();
  } else {
    pCmdUI->Enable(false);
  }
}

void CCellEdit::OnUpdateEditCut    (CCmdUI *pCmdUI)
{
  LRESULT l=SendMessage(EM_GETSEL,0,0);
  pCmdUI->Enable(LOWORD(l)!=HIWORD(l));
}


void CCellEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar){
		case 0x01: // ctrl+a
			SendMessage(EM_SETSEL,0,-1);
			break;
		case 0x03: // ctrl+c
			SendMessage(WM_COPY,0,0);
			break;
		case 0x16: // ctrl+v
			SendMessage(WM_PASTE,0,0);
			break;
		case 0x18: // ctrl+x
			SendMessage(WM_CUT,0,0);
			break;
		case 0x1a: // ctrl+z
			SendMessage(EM_UNDO,0,0);
			break;
    default:
      Default();
			break;
  }	
	CCell::OnChar(nChar, nRepCnt, nFlags);
}
