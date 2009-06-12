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
// CSHPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "CSHPropertySheet.h"
#include <afxpriv.h> // for WM_COMMANDHELP
#include <htmlhelp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCSHPropertySheet property page
IMPLEMENT_DYNCREATE(CCSHPropertySheet, CPropertySheet)

CCSHPropertySheet::~CCSHPropertySheet(){}

void CCSHPropertySheet::DoDataExchange(CDataExchange* pDX)
{
	CPropertySheet::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCSHPropertySheet)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCSHPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CCSHPropertySheet)
  ON_COMMAND(ID_WHATS_THIS,OnWhatsThis)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
  ON_MESSAGE(WM_COMMANDHELP,OnCommandHelp)
	ON_COMMAND(IDHELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCSHPropertySheet message handlers

void CCSHPropertySheet::OnWhatsThis()
{
  DisplayHelp(m_pwndContext->m_hWnd,HelpID(m_pwndContext->m_hWnd),GetInstanceHandle());
}

void CCSHPropertySheet::OnContextMenu(CWnd*pWnd, CPoint pt)
{
  m_pwndContext=WndFromPoint(this,pWnd,pt);
  if(NULL==m_pwndContext || !CCSHCommon::OnContextMenu(this,pt,HelpID(m_pwndContext->GetDlgCtrlID()))){
    CPropertySheet::OnContextMenu(pWnd,pt);
  }
}

BOOL CCSHPropertySheet::OnHelpInfo(HELPINFO* pHelpInfo) 
{
  DisplayHelp((HWND)pHelpInfo->hItemHandle,HelpID((HWND)pHelpInfo->hItemHandle),GetInstanceHandle());
  return TRUE;
}

BOOL CCSHPropertySheet::OnInitDialog() 
{
  CPropertySheet::OnInitDialog();
  if(0==HelpID(IDOK)){
    // Dynamically add/remove the question mark button to the title bar of the dialog box:
    ModifyStyleEx(WS_EX_CONTEXTHELP,0,0);
  } else {
    ModifyStyleEx(0,WS_EX_CONTEXTHELP,0);
  }
  
  if(CSHFile().IsEmpty()){
    // All of the following mess happens because Microsoft decided to show a help button by 
    // default to every Property sheet constructed by Visual C++ 6.In the following lines, 
    // I hide the help button and move the other three buttons to where they should be.
    CWnd *pwndHelpButton=GetDlgItem(IDHELP);
    if(pwndHelpButton){
      static const int _afxPropSheetButtons[] = { IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };
    
      for (int i = 0; i < sizeof _afxPropSheetButtons/sizeof _afxPropSheetButtons[0] - 1; i++){
        // Shunt button   
        CRect rect;
        GetDlgItem(_afxPropSheetButtons[i+1])->GetWindowRect(&rect);
        ScreenToClient(&rect);
        GetDlgItem(_afxPropSheetButtons[i])->MoveWindow(&rect);
      }
      pwndHelpButton->DestroyWindow();    
    }    
  }
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Dummy implementations
UINT CCSHPropertySheet::HelpID(DWORD) const
{
  return 0; // dummy implementation
}


CString CCSHPropertySheet::CSHFile() const 
{
  return m_strCSHFilePath;
}

void CCSHPropertySheet::OnHelp() 
{
  if(!CSHFile().IsEmpty()){
    HtmlHelp(NULL,GetCSHFilePath()+CSHFile(),HH_DISPLAY_TOPIC,0);
  }
}

HINSTANCE CCSHPropertySheet::GetInstanceHandle()
{
  return AfxGetInstanceHandle();
}

BOOL CCSHPropertySheet::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
  if(CCSHCommon::FilterMessage(message, wParam,lParam,pResult)){
    return true; // handled
  }
  return CPropertySheet::OnWndMsg(message, wParam, lParam, pResult);
}
