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
// ttlistctrl.cpp : implementation file
//

#include "stdafx.h"
#include "ttlistctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTTListCtrl

CTTListCtrl::CTTListCtrl()
{
}

CTTListCtrl::~CTTListCtrl()
{
}


BEGIN_MESSAGE_MAP(CTTListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CTTListCtrl)
	//}}AFX_MSG_MAP
  ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTTListCtrl message handlers

int CTTListCtrl::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
{
  CTTListCtrl &l=(CTTListCtrl &)*this;
  LVHITTESTINFO info;
  info.pt=point;
  if(-1!=l.SubItemHitTest(&info)){
    CRect rect;
    l.GetSubItemRect(info.iItem,info.iSubItem,LVIR_LABEL,rect);

    CDC *pDC=l.GetDC();
    CFont *pOldFont=pDC->SelectObject(GetFont());
    int cx=pDC->GetTextExtent(GetItemText(info.iItem,info.iSubItem)).cx;
    pDC->SelectObject(pOldFont);
    l.ReleaseDC(pDC);
    if(cx>GetColumnWidth(info.iSubItem)-10){

      pTI->hwnd = m_hWnd;
	    pTI->uId = 1+MAKELONG(info.iItem,info.iSubItem);
 	    pTI->lpszText = LPSTR_TEXTCALLBACK;
	    pTI->rect = rect;

	    return pTI->uId;
    }
  }
  return -1;
}

BOOL CTTListCtrl::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	id=pNMHDR->idFrom;

  if(0==id--){	  	// Notification in NT from automatically
		return FALSE;   	// created tooltip
  } else {
	  const CString strTipText(GetItemText(LOWORD(id),HIWORD(id)));
#ifndef _UNICODE
	  if (pNMHDR->code == TTN_NEEDTEXTA)
		  lstrcpyn(pTTTA->szText, strTipText, sizeof pTTTA->szText/sizeof TCHAR - 1);
	  else
		  _mbstowcsz(pTTTW->szText, strTipText, sizeof pTTTW->szText/sizeof TCHAR - 1);
#else
	  if (pNMHDR->code == TTN_NEEDTEXTA)
		  _wcstombsz(pTTTA->szText, strTipText, sizeof pTTTA->szText/sizeof TCHAR - 1);
	  else
		  lstrcpyn(pTTTW->szText, strTipText, sizeof pTTTW->szText/sizeof TCHAR - 1);
#endif
    CRect rect;
    GetSubItemRect(LOWORD(id),HIWORD(id),LVIR_LABEL,rect);
    ClientToScreen(rect);
    //::SetWindowPos(pNMHDR->hwndFrom,NULL,rect.left,rect.top,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
    *pResult = 0;

	  return TRUE;    // message was handled
  }
}

void CTTListCtrl::PreSubclassWindow() 
{
	CListCtrl::PreSubclassWindow();
  EnableToolTips(TRUE);
}
