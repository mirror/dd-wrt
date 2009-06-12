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
// ComboEdit.cpp : implementation file
//

#include "stdafx.h"
#include "ComboEdit.h"
#include "cellview.h"
#include "Configtool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CComboEdit

CComboEdit::CComboEdit(LPCTSTR pszInitialValue,const CStringArray &arValues):
  CCell(pszInitialValue)
{
  m_arValues.Copy(arValues);
}

CComboEdit::~CComboEdit()
{
}


BEGIN_MESSAGE_MAP(CComboEdit, CCell)
	//{{AFX_MSG_MAP(CComboEdit)
	ON_WM_CREATE()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComboEdit message handlers


BOOL CComboEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext*) 
{
  return CWnd::CreateEx(WS_EX_CLIENTEDGE,_T("COMBOBOX"), NULL, dwStyle, rect, pParentWnd, nID);
}

int CComboEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCell::OnCreate(lpCreateStruct) == -1)
		return -1;

	SendMessage(WM_SETFONT,(WPARAM)(HFONT)GetStockObject(DEFAULT_GUI_FONT),0);

  CStringArray arEnum;
  for(int i=0;i<m_arValues.GetSize();i++){
    SendMessage(CB_ADDSTRING,0,(LPARAM)(LPCTSTR)m_arValues[i]);
  }
  int nIndex=SendMessage(CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)(LPCTSTR)m_strInitialValue);
  SendMessage(CB_SETCURSEL, nIndex == CB_ERR ? 0 : nIndex, 0);
	
	return 0;
}

void CComboEdit::OnKillFocus(CWnd*)
{
    CConfigTool::GetCellView()->CancelCellEdit();
}

