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
// FindDialog.cpp : implementation file
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
// Description:	This overrides CFindDialog functions to customize the dialog
//				for use in the configuration tool.
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
#include "FindDialog.h"
#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include <afxpriv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFindDialog dialog

 
CFindDialog::CFindDialog()
	: CFindReplaceDialog()
{
	//{{AFX_DATA_INIT(CFindDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    #ifdef PLUGIN
    extern HINSTANCE heCosInstance;
    m_fr.hInstance=heCosInstance;
    #endif
	m_fr.Flags |= FR_ENABLETEMPLATE;
	m_fr.lpTemplateName=MAKEINTRESOURCE(IDD_FINDREPLACE);
}

CFindDialog::~CFindDialog()
{
}

void CFindDialog::DoDataExchange(CDataExchange* pDX)
{
	CFindReplaceDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BOOL CFindDialog::OnInitDialog() 
{
	CFindReplaceDialog::OnInitDialog();
  CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_SEARCHCOMBO);
  pCombo->AddString(_T("Macro names"));
  pCombo->AddString(_T("Item names"));
  pCombo->AddString(_T("Short descriptions"));
  pCombo->AddString(_T("Current Values"));
  pCombo->AddString(_T("Default Values"));
  pCombo->SetCurSel(m_nFindPos);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CFindDialog, CFindReplaceDialog)
	//{{AFX_MSG_MAP(CFindDialog)
	ON_CBN_SELCHANGE(IDC_SEARCHCOMBO, OnSelchangeSearchcombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindDialog message handlers

void CFindDialog::OnSelchangeSearchcombo() 
{
  m_nFindPos=(WhereType)((CComboBox *)GetDlgItem(IDC_SEARCHCOMBO))->GetCurSel();
}

BOOL CFindDialog::Create(LPCTSTR lpszFindWhat, DWORD dwFlags, WhereType where, CWnd* pParentWnd)
{
    m_nFindPos=where;
	m_nIDHelp = AFX_IDD_FIND;
	m_fr.Flags |= dwFlags;

	ASSERT_VALID(pParentWnd);
	m_fr.hwndOwner = pParentWnd->m_hWnd;
	ASSERT(m_fr.hwndOwner != NULL); // must have a parent for modeless dialog

	m_fr.wFindWhatLen = sizeof(m_szFindWhat);

    int n=min(sizeof(m_szFindWhat)-1,_tcslen(lpszFindWhat));
	_tcsncpy(m_szFindWhat, lpszFindWhat, n);
    m_szFindWhat[n]=_TCHAR('\0');

	AfxHookWindowCreate(this);
    HWND hWnd = ::FindText(&m_fr);
    if (!AfxUnhookWindowCreate()){
		PostNcDestroy();
    }

	ASSERT(hWnd == NULL || hWnd == m_hWnd);
	return hWnd != NULL;
}
