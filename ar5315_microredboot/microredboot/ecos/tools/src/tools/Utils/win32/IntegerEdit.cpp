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
// IntegerEdit.cpp : implementation file
//
//
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
#include "IntegerEdit.h"
#include "CTUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntegerEdit

CIntegerEdit::CIntegerEdit(__int64 nInitialValue):
  CCellEdit(CUtils::IntToStr(nInitialValue,false/*bool bHex*/)),
  m_bInSize(false)
{
}

CIntegerEdit::~CIntegerEdit()
{
}


BEGIN_MESSAGE_MAP(CIntegerEdit, CCellEdit)
	//{{AFX_MSG_MAP(CIntegerEdit)
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntegerEdit message handlers

void CIntegerEdit::OnUpdate()
{
    CString str;
    GetWindowText(str);
    __int64 d;
    if(CUtils::StrToItemIntegerType(str,d)||0==str.CompareNoCase(_T("0X"))||_T("-")==str){
        // reject all illegal strings except [partially] correct ones
        m_strPrevText=str;
    } else {
        MessageBeep(0xFFFFFFFF);
        const CPoint pt(GetCaretPos());
        SetWindowText(m_strPrevText);
        SetCaretPos(pt);
    }
}


BOOL CIntegerEdit::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CCellEdit::PreCreateWindow(cs);
}

int CIntegerEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCellEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
  m_wndSpin.Create(WS_CHILD|UDS_NOTHOUSANDS|UDS_ARROWKEYS|UDS_SETBUDDYINT,CRect(0,0,1,1),GetParent(),1);
  m_wndSpin.SetBuddy(this);
  m_wndSpin.SetRange32(0,0x7fffffff);
  m_wndSpin.SetWindowPos(this,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	return 0;
}
  
void CIntegerEdit::OnSize(UINT /*nType*/, int cx, int cy) 
{
  int cxSpin=::GetSystemMetrics(SM_CXVSCROLL);
  int cySpin=::GetSystemMetrics(SM_CYVSCROLL);
  int cxEdge=::GetSystemMetrics(SM_CXEDGE);
  int cyEdge=::GetSystemMetrics(SM_CYEDGE);
  bool bSpin=(cx>3*cxSpin);
  if(bSpin){
    if(m_bInSize){
      Default();
    } else {
      cx-=(cxSpin+4);
      CRect rect(cx+4,-cyEdge,cx+4+cxSpin,min(cySpin,cy+2*cyEdge));
      ClientToScreen(rect);
      GetParent()->ScreenToClient(rect);
      m_wndSpin.MoveWindow(rect,true);
      m_wndSpin.ShowWindow(SW_SHOW);
      rect=CRect(0,0,cx,cy);
      ClientToScreen(rect);
      GetParent()->ScreenToClient(rect);
      m_bInSize=true;
      rect.InflateRect(cxEdge,cyEdge);
      MoveWindow(rect);
      m_bInSize=false;
    }
  } else {
    m_wndSpin.ShowWindow(SW_HIDE);
    Default();
  }
}
