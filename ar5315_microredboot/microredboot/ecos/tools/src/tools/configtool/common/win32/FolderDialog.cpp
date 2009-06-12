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
// FolderDialog.cpp : implementation file
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
// Description:	This is the implementation of the _T("browse for folder") dialog class
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
#include "FolderDialog.h"
#include "shlobj.h"
#include "CTUtils.h"
#include "NewFolderDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFolderDialog dialog

CFolderDialog::CFolderDialog(BOOL bAllowCreation/*=TRUE*/,UINT id/*=0*/)
    : CeCosDialog(id?id:IDD_FOLDER_DIALOG, NULL),
	m_bAllowCreation(bAllowCreation),
	m_pButton(NULL)
{
	//{{AFX_DATA_INIT(CFolderDialog)
	//}}AFX_DATA_INIT
	m_strTitle=_T("Choose Folder");
	GetCurrentDirectory(MAX_PATH,m_strFolder.GetBuffer(MAX_PATH));
	m_strFolder.ReleaseBuffer();
}


void CFolderDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFolderDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFolderDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CFolderDialog)
	ON_BN_CLICKED(IDC_FOLDER_DIALOG_BROWSE, OnBrowse)
	ON_EN_CHANGE(IDC_FOLDER, OnChangeFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFolderDialog message handlers

void CFolderDialog::OnBrowse() 
{
    m_wndProc=NULL;
	GetDlgItemText(IDC_FOLDER,m_strFolder);

	BROWSEINFO bi;
    bi.hwndOwner = GetSafeHwnd(); 
    bi.pidlRoot = NULL;   
    bi.pszDisplayName = m_strFolder.GetBuffer(MAX_PATH);
    bi.lpszTitle = _T("");
    bi.ulFlags = BIF_RETURNONLYFSDIRS/*|0x0010 BIF_EDITBOX*/;             
    bi.lpfn = (BFFCALLBACK)CBBrowseCallbackProc;
    bi.lParam = (LPARAM)this;

	static const UINT arids[]={IDOK,IDCANCEL,IDC_FOLDER_DIALOG_BROWSE,IDC_FOLDER};
	BOOL arbEnabled[4];

	ASSERT(sizeof arids/sizeof arids[0] == sizeof arbEnabled/sizeof arbEnabled[0]);
	
	for(int i=0;i<sizeof arids/sizeof arids[0];i++){
		UINT id=arids[i];
		arbEnabled[i]=GetDlgItem(id)->IsWindowEnabled();
		GetDlgItem(id)->EnableWindow(FALSE);
	}
    LPITEMIDLIST iil = SHBrowseForFolder(&bi);
	m_strFolder.ReleaseBuffer();
	if(iil)
	{
		SHGetPathFromIDList(iil,m_strFolder.GetBuffer(MAX_PATH));
        m_strFolder.ReleaseBuffer();
        SetDlgItemText(IDC_FOLDER,m_strFolder);
	}

	for(i=0;i<sizeof arids/sizeof arids[0];i++){
		UINT id=arids[i];
		GetDlgItem(id)->EnableWindow(arbEnabled[i]);
	}
	GetDlgItem(IDOK)->EnableWindow(!m_strFolder.IsEmpty());

	if(m_bAllowCreation){
		deleteZ(m_pButton);
	}
}

// Use callback to expand folder in edit box at time of browse
int CALLBACK CFolderDialog::CBBrowseCallbackProc( HWND hwnd, 
	UINT uMsg, 
	LPARAM lParam, 
	LPARAM lpData 
	)
{
    CFolderDialog *pDlg=(CFolderDialog *)lpData;
    switch(uMsg){
        case BFFM_INITIALIZED:
	        if(pDlg->m_bAllowCreation){
		        CWnd *pWnd=CWnd::FromHandle(hwnd);
		        pDlg->m_pButton=new CButton();
		        // Get rect of IDOK button
		        CRect rect1;
		        pWnd->GetDlgItem(IDOK)->GetWindowRect(&rect1);
		        pWnd->ScreenToClient(&rect1);

		        // Get rect of IDCANCEL button to the right
		        CRect rect2;
		        pWnd->GetDlgItem(IDCANCEL)->GetWindowRect(&rect2);
		        pWnd->ScreenToClient(&rect2);
		        int nXDiff=rect2.left-rect1.left;

		        CRect rect(rect1);
		        rect.left-=nXDiff;
		        rect.right-=nXDiff;
		        pDlg->m_pButton->Create(_T("&New..."), WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON, rect, pWnd, ID_NEW_FOLDER);
		        pDlg->m_pButton->SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
		        m_wndProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (long)WindowProcNew);
	        }
            ::SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)(LPCTSTR)pDlg->m_strFolder);
        case BFFM_SELCHANGED:
            {
                // Change the cwd such that if the New button is used, we know where we are to start from
                ITEMIDLIST *iil=(ITEMIDLIST *)lParam;
                CString strFolder;
		        SHGetPathFromIDList(iil,strFolder.GetBuffer(MAX_PATH));
                strFolder.ReleaseBuffer();
                SetCurrentDirectory(strFolder);
            }
            break;
        default:
            ;
    }

	return 0;
}


WNDPROC CFolderDialog::m_wndProc=NULL;
LRESULT CALLBACK CFolderDialog::WindowProcNew(HWND hwnd,UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message ==  WM_COMMAND && wParam==MAKEWPARAM(ID_NEW_FOLDER,BN_CLICKED)){
		CNewFolderDialog dlg;
        // Start from the cwd (maintained by BFFM_SELCHANGED above)
		GetCurrentDirectory(MAX_PATH,dlg.m_strFolder.GetBuffer(MAX_PATH));
		dlg.m_strFolder.ReleaseBuffer();

		if(IDOK==dlg.DoModal()){
            // Feed the new directory back as the current selection in the browse dialog
			::SendMessage (hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(LPCTSTR )dlg.m_strFolder);
		}
		return 0;
	} else {
		return CallWindowProc(m_wndProc, hwnd, message, wParam, lParam);
	}
}
 
void CFolderDialog::OnOK() 
{
	GetDlgItemText(IDC_FOLDER,m_strFolder);
	if(!m_strFolder.IsDir()){
		if(m_bAllowCreation){
			if(!m_strFolder.CreateDirectory()){
				CUtils::MessageBoxF(_T("Could not create folder %s"),m_strFolder);
				return;
			}
		} else {
			CUtils::MessageBoxF(_T("Folder %s does not exist"),m_strFolder);
			return;
		}
	} else if (m_bAllowCreation && IDNO==CUtils::MessageBoxFT(MB_YESNO|MB_DEFBUTTON2, _T("Folder %s already exists - overwrite?"),m_strFolder)){
		return;
	}
	CeCosDialog::OnOK();
}

BOOL CFolderDialog::OnInitDialog() 
{
	CeCosDialog::OnInitDialog();
	if(m_bAllowCreation&&m_strDesc.IsEmpty()){
		m_strDesc.LoadString(IDS_DEFAULT_FOLDER_DIALOG_DESC);
	}
	SetDlgItemText(IDC_STATIC_DESC,m_strDesc);
	SetDlgItemText(IDC_FOLDER,m_strFolder);
	SetWindowText(m_strTitle);
	SetCurrentDirectory(m_strFolder);
	SetDlgItemText(IDC_FOLDER,m_strFolder);
	GetDlgItem(IDC_FOLDER)->SetFocus();
	GetDlgItem(IDC_FOLDER)->SendMessage(EM_SETSEL,0,-1);
	GetDlgItem(IDOK)->EnableWindow(!m_strFolder.IsEmpty());
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFolderDialog::OnCancel() 
{
	CeCosDialog::OnCancel();
}



void CFolderDialog::OnChangeFolder() 
{
	CString str;
	GetDlgItemText(IDC_FOLDER,str);
	GetDlgItem(IDOK)->EnableWindow(!str.IsEmpty());
}
