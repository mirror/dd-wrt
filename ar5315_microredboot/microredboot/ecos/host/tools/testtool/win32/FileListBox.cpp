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
// FileListBox.cpp : implementation file
//

#include "stdafx.h"
#include "FileListBox.h"
#include "TestToolRes.h"		// main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileListBox

CFileListBox::CFileListBox()
{
}

CFileListBox::~CFileListBox()
{
}


BEGIN_MESSAGE_MAP(CFileListBox, CCheckListBox)
	//{{AFX_MSG_MAP(CFileListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDC_TT_REMOVE, OnRemove)
	ON_COMMAND(IDC_TT_ADD, OnAdd)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileListBox message handlers

void CFileListBox::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    UNUSED_ALWAYS(pWnd);
	CMenu menu;
	menu.LoadMenu(IDR_TT_CONTEXTMENU);
	CMenu *pPopup=menu.GetSubMenu(0);
    /*
    ScreenToClient(&point);
    BOOL bOutside;
    m_nIndex=ItemFromPoint(point,bOutside);
    CRect rect;
    GetItemRect(m_nIndex,rect);
    if(bOutside || !rect.PtInRect(point)){
        pPopup->EnableMenuItem(IDC_TT_REMOVE,MF_BYCOMMAND|MF_GRAYED);
    }
    ClientToScreen(&point);
    */
    for(int i=0;i<GetCount();i++){
        if(GetSel(i)){
            break;
        }
    }
    if(i==GetCount()){
        pPopup->EnableMenuItem(IDC_TT_REMOVE,MF_BYCOMMAND|MF_GRAYED);
    }

	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x,point.y,this);
}

void CFileListBox::OnRemove() 
{
    bool *arb=new bool[GetCount()];
    for(int i=0;i<GetCount();i++){
        arb[i]=(GetSel(i)>0);
    }
    for(i=GetCount()-1;i>=0;--i){
        if(arb[i]){
            DeleteString(i);
        }
    }
    delete [] arb;
}

void CFileListBox::OnAdd() 
{
    GetParent()->SendMessage(WM_COMMAND,IDC_TT_ADD,0);
}

void CFileListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if(VK_DELETE==nChar){
        SendMessage(WM_COMMAND,IDC_TT_REMOVE,0);
    } else {
	    CCheckListBox::OnKeyDown(nChar, nRepCnt, nFlags);
    }
}
