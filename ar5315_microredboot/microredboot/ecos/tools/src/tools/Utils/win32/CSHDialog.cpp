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
// CSHDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CSHDialog.h"
#include <afxpriv.h> // for WM_COMMANDHELP

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <htmlHelp.h>
#include <windowsx.h>
/////////////////////////////////////////////////////////////////////////////
// CCSHDialog dialog

void CCSHDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCSHDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCSHDialog, CDialog)
	//{{AFX_MSG_MAP(CCSHDialog)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
  ON_MESSAGE(WM_COMMANDHELP,OnCommandHelp)
  ON_COMMAND(ID_WHATS_THIS,OnWhatsThis)
	ON_COMMAND(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCSHDialog message handlers
void CCSHDialog::OnContextMenu(CWnd* pWnd, CPoint pt) 
{
  m_pwndContext=WndFromPoint(this,pWnd,pt);
  if(NULL==m_pwndContext || !CCSHCommon::OnContextMenu(this,pt,HelpID(m_pwndContext->GetDlgCtrlID()))){
    CDialog::OnContextMenu(pWnd,pt);
  }
}

void CCSHDialog::OnWhatsThis()
{
  DisplayHelp(m_pwndContext->m_hWnd,HelpID(m_pwndContext->m_hWnd),GetInstanceHandle());
}

BOOL CCSHDialog::OnHelpInfo(HELPINFO* pHelpInfo) 
{
  DisplayHelp((HWND)pHelpInfo->hItemHandle,HelpID((HWND)pHelpInfo->hItemHandle),GetInstanceHandle());
  return TRUE;
}

BOOL CCSHDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  if(0==HelpID(IDOK)){
    // Dynamically add/remove the question mark button to the title bar of the dialog box:
    ModifyStyleEx(WS_EX_CONTEXTHELP,0,0);
  } else {
    ModifyStyleEx(0,WS_EX_CONTEXTHELP,0);
  }

  CWnd *pHelpButton=GetDlgItem(IDHELP);
  if(NULL!=pHelpButton){
    pHelpButton->EnableWindow(!CSHFile().IsEmpty());
  }
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CCSHDialog::CSHFile() const // return the context-sensitive help file 
{
  return _T("");
}

UINT CCSHDialog::HelpID(DWORD) const
{
  return 0; // dummy implementation
}

void CCSHDialog::OnHelp() 
{
  if(!CSHFile().IsEmpty()){
    HtmlHelp(NULL,GetCSHFilePath()+CSHFile(),HH_DISPLAY_TOPIC,0);
  }
}

HINSTANCE CCSHDialog::GetInstanceHandle()
{
  return AfxGetInstanceHandle();
}

BOOL CCSHDialog::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  if(CCSHCommon::FilterMessage(message, wParam,lParam,pResult)){
    return true; // handled
  }
  return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

