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
#include "Cell.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCell

CCell::CCell(LPCTSTR pszInitialValue):
  m_strInitialValue(pszInitialValue)
{
}

CCell::~CCell()
{
}


BEGIN_MESSAGE_MAP(CCell, CWnd)
	//{{AFX_MSG_MAP(CCell)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCell message handlers

BOOL CCell::PreTranslateMessage(MSG* pMsg) 
{
  if(pMsg->message==WM_KEYDOWN){
    switch(pMsg->wParam){
      case VK_ESCAPE:
        GetParent()->SendMessage(WM_CANCEL_EDIT,0);
        return TRUE;
      case VK_RETURN:
        GetParent()->SendMessage(WM_CANCEL_EDIT,1);
        return TRUE;
      default:
        break;
    }
  }
	
	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CCell::Create(DWORD, const RECT&, CWnd*, UINT, CCreateContext*)
{
  ASSERT(false);
  return false;
}

void CCell::OnChar(UINT nChar, UINT, UINT ) 
{
  switch(nChar){
    case VK_RETURN:
      GetParent()->SendMessage(WM_CANCEL_EDIT,1);
      break;
    case VK_ESCAPE:
      GetParent()->SendMessage(WM_CANCEL_EDIT,0);
      break;
    default:
      // Prevent beep: do not pass message back
      break;
  }
}
