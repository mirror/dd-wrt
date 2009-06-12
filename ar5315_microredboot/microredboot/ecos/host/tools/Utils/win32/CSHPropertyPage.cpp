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
// CSHPropertyPage.cpp : implementation file
//

#include "stdafx.h"
#include "CSHPropertyPage.h"
#include <afxpriv.h> // for WM_COMMANDHELP
#include <htmlhelp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCSHPropertyPage property page
IMPLEMENT_DYNCREATE(CCSHPropertyPage, CPropertyPage)

CCSHPropertyPage::~CCSHPropertyPage(){}

void CCSHPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCSHPropertyPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCSHPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CCSHPropertyPage)
  ON_COMMAND(ID_WHATS_THIS,OnWhatsThis)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
  ON_MESSAGE(WM_COMMANDHELP,OnCommandHelp)
	ON_COMMAND(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCSHPropertyPage message handlers

void CCSHPropertyPage::OnWhatsThis()
{
  DisplayHelp(m_pwndContext->m_hWnd,HelpID(m_pwndContext->m_hWnd),GetInstanceHandle());
}

void CCSHPropertyPage::OnContextMenu(CWnd*pWnd, CPoint pt)
{
  m_pwndContext=WndFromPoint(this,pWnd,pt);
  if(NULL==m_pwndContext || !CCSHCommon::OnContextMenu(this,pt,HelpID(m_pwndContext->GetDlgCtrlID()))){
    CPropertyPage::OnContextMenu(pWnd,pt);
  }
}

BOOL CCSHPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
  DisplayHelp((HWND)pHelpInfo->hItemHandle,HelpID((HWND)pHelpInfo->hItemHandle),GetInstanceHandle());
  return TRUE;
}

// Dummy implementations
UINT CCSHPropertyPage::HelpID(DWORD) const
{
  return 0; // dummy implementation
}


CString CCSHPropertyPage::CSHFile() const 
{
  return CCSHCommon::GetCSHFilePath();
}

void CCSHPropertyPage::OnHelp() 
{
  if(!CSHFile().IsEmpty()){
    HtmlHelp(NULL,GetCSHFilePath()+CSHFile(),HH_DISPLAY_TOPIC,0);
  }
}

HINSTANCE CCSHPropertyPage::GetInstanceHandle()
{
  return AfxGetInstanceHandle();
}

BOOL CCSHPropertyPage::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  if(CCSHCommon::FilterMessage(message, wParam,lParam,pResult)){
    return true; // handled
  }
  return CPropertyPage::OnWndMsg(message, wParam, lParam, pResult);
}
