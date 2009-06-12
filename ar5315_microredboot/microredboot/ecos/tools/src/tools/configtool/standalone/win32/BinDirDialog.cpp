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
// BinDirDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigTool.h"
#include "configtooldoc.h"
#include "BinDirDialog.h"
#include "ConfigItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBinDirDialog dialog


CBinDirDialog::CBinDirDialog(const CStringArray &arstrPaths, const CString &strDefault)
	: CFolderDialog(FALSE, CBinDirDialog::IDD), m_arstrPaths(arstrPaths), m_strDefault(strDefault)
{
	//{{AFX_DATA_INIT(CBinDirDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_strFolder = _T(""); // Internationalization OK
}


void CBinDirDialog::DoDataExchange(CDataExchange* pDX)
{
	CFolderDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBinDirDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBinDirDialog, CFolderDialog)
	//{{AFX_MSG_MAP(CBinDirDialog)
	ON_CBN_EDITCHANGE(IDC_FOLDER, OnEditchangeFolder)
	ON_CBN_SELCHANGE(IDC_FOLDER, OnSelchangeFolder)
	ON_BN_CLICKED(IDC_FOLDER_DIALOG_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBinDirDialog message handlers

BOOL CBinDirDialog::OnInitDialog() 
{
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_FOLDER);
	CFolderDialog::OnInitDialog();

	CDC *pDC=GetDC();
	CFont *pOldFont=pDC->SelectObject(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));

	int nMaxTextLength=0;
	for(int i=0;i<m_arstrPaths.GetSize();i++){
		const CFileName &str=m_arstrPaths[i];
		for(int j=i+1;j<m_arstrPaths.GetSize();j++){
			const CFileName &ostr=m_arstrPaths[j];
			if(ostr.SameFile(str)){
				goto Next;
			}
		}
		if(str.IsDir()&&pCombo->FindString(-1,str)<0){
			pCombo->AddString(str);
			nMaxTextLength=max(nMaxTextLength,pDC->GetTextExtent(str).cx);
		}
		Next:;
	}
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	CRect rcClient;
	pCombo->GetClientRect(rcClient);
	int nExpand=nMaxTextLength-(rcClient.Width()-GetSystemMetrics(SM_CXVSCROLL)-2*GetSystemMetrics(SM_CXBORDER)-5);
	if(nExpand>0){
		static const UINT arids[]={IDC_STATIC_DESC, IDOK,IDCANCEL,IDC_FOLDER_DIALOG_BROWSE,IDC_FOLDER};
		CRect rect;
		for(int i=0;i<sizeof arids/sizeof arids[0];i++){
			UINT id=arids[i];
			GetDlgItem(id)->GetWindowRect(rect);
			ScreenToClient(rect);
			if(IDC_FOLDER!=id && IDC_STATIC_DESC!=id){
				rect.left+=nExpand;
			}
			rect.right+=nExpand;
			GetDlgItem(id)->MoveWindow(rect);
		}

		GetWindowRect(rect);
		rect.right+=nExpand;
		MoveWindow(rect);
	}

	GetDlgItem(IDOK)->EnableWindow(!m_strFolder.IsEmpty());
	for(i=pCombo->GetCount()-1;i>=0;--i){
		CFileName str;
		pCombo->GetLBText(i,str);
		if(str.SameFile(m_strDefault)){
			pCombo->SetCurSel(i);
			GetDlgItem(IDOK)->EnableWindow(TRUE);
		}
	}

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBinDirDialog::OnEditchangeFolder() 
{
	CString str;
	GetDlgItemText(IDC_FOLDER,str);
	GetDlgItem(IDOK)->EnableWindow(!str.IsEmpty());
}

void CBinDirDialog::OnSelchangeFolder() 
{
	GetDlgItem(IDOK)->EnableWindow(-1!=((CComboBox *)GetDlgItem(IDC_FOLDER))->GetCurSel());
}

void CBinDirDialog::OnBrowse() 
{
	CString str;
	CFolderDialog::OnBrowse();
	GetDlgItemText(IDC_FOLDER,str);
	GetDlgItem(IDOK)->EnableWindow(!str.IsEmpty());
}
