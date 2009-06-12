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
// MultiLineEditDialog.cpp : implementation file
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
// Description:	This is the implementation of the popup dialog for in-cell editing
//			    of multi-line string items
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
#include "MultiLineEditDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiLineEditDialog dialog

const CMultiLineEditDialog::DlgData CMultiLineEditDialog::data={
  {DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU, 0,  3, 0, 0,  294,  170}, // # items
    
  0x0000, // menu
  0x0000, // class 
  0x0000, // title
  {  
    {
      {WS_VISIBLE | WS_TABSTOP | WS_CHILD | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,0,183,7,50,14,IDOK},
      0xffff, 0x0080, // class (button)
      0x0000,         // title
      0x0000,         // creation data
      0x0000          // alignment
    },

    {
      {WS_VISIBLE | WS_TABSTOP | WS_CHILD | BS_PUSHBUTTON,0,  237,  7,  50,  14,  IDCANCEL},
      0xffff, 0x0080, // class (button)
      0x0000,         // title
      0x0000,         // creation data 
      0x0000          // alignment
    },

    {
      {WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL,0,  7,  24,  280,  139,  42 /* replaced by m_idEdit */},
      0xffff, 0x0081, // class (edit)
      0x0000,         // title
      0x0000,         // creation data
      0x0000          // alignment
    }
  }
};

CMultiLineEditDialog::CMultiLineEditDialog(CWnd* pParent /*=NULL*/,UINT idEdit)
	: CeCosDialog(),
  m_idEdit(idEdit)
{
  m_data=data;
  m_data.ctrl[2].dit.id=(WORD)m_idEdit; // set the id of the edit control according to parent's wishes (typically to allow CSH to work)
  InitModalIndirect(&m_data.dtdlg,pParent);
	//{{AFX_DATA_INIT(CMultiLineEditDialog)
	m_strText = _T("");
	//}}AFX_DATA_INIT
}

void CMultiLineEditDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiLineEditDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiLineEditDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CMultiLineEditDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiLineEditDialog message handlers

BOOL CMultiLineEditDialog::OnInitDialog() 
{
	CeCosDialog::OnInitDialog();
  SetWindowText(_T("String Edit"));
	SetDlgItemText(IDOK,_T("OK"));
	SetDlgItemText(IDCANCEL,_T("Cancel"));
  SetDlgItemText(m_idEdit,m_strText);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMultiLineEditDialog::OnOK()
{
  GetDlgItemText(m_idEdit,m_strText);
  CeCosDialog::OnOK();
}
