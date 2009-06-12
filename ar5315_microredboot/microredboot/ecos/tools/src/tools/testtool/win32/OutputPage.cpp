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
// OutputPage.cpp : implementation file
//

#include "stdafx.h"
#include <time.h>
#include "OutputPage.h"
#include "eCosTest.h"
#include "RunTestsSheet.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputPage property page

IMPLEMENT_DYNCREATE(COutputPage, CeCosPropertyPage)

COutputPage::COutputPage() : CeCosPropertyPage(IDD_TT_OUTPUT_PAGE,0)
{
	//{{AFX_DATA_INIT(COutputPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COutputPage::~COutputPage()
{
}

void COutputPage::DoDataExchange(CDataExchange* pDX)
{
	CeCosPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COutputPage)
	DDX_Control(pDX, IDC_TT_EDIT, m_Edit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COutputPage, CeCosPropertyPage)
	//{{AFX_MSG_MAP(COutputPage)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutputPage message handlers

BOOL COutputPage::OnInitDialog() 
{
	CeCosPropertyPage::OnInitDialog();

    m_Font.CreatePointFont(90,_T("Courier New"));
    m_Edit.SetFont(&m_Font);

    return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COutputPage::AddText(LPCTSTR psz)
{
    const CString str(psz);
	CString strText;
	// Change \n into \r\n
	int nStart=0;
	for(int nIndex=0;nIndex<str.GetLength();nIndex++){
		if(str[nIndex]==_TCHAR('\n')){
			if(nIndex==0||str[nIndex-1]!=_TCHAR('\r')){
				strText+=str.Mid(nStart,nIndex-nStart);
				strText+=_TCHAR('\r');
				nStart=nIndex;
			}
		}
	}

	strText+=str.Mid(nStart,nIndex-nStart);

    /*
    CString strWText;
    m_Edit.GetWindowText(strWText);
    strWText+=strText;
    m_Edit.SetWindowText(strWText);
    */
    
	{
		int nStart,nEnd;
		int nLength=m_Edit.GetWindowTextLength();
		m_Edit.GetSel(nStart,nEnd);
		// Replace selection
		m_Edit.SetSel(nLength,nLength,TRUE);
		m_Edit.ReplaceSel(strText);
		if(m_Edit.GetWindowTextLength()!=nLength+strText.GetLength()){
			// Try again by removing equivalent length from start of buffer.
			// For neatness, remove whole lines
			int nLine=m_Edit.LineFromChar(strText.GetLength()-1);
			int nIndex=m_Edit.LineIndex(nLine+1);
			m_Edit.SetSel(0,nIndex-1,TRUE);
			m_Edit.ReplaceSel(_T(""));
			nLength=m_Edit.GetWindowTextLength();
			m_Edit.SetSel(nLength,nLength,TRUE);
			m_Edit.ReplaceSel(strText);
		} else if(nStart!=nEnd) {
			m_Edit.SetSel(nStart,nEnd,TRUE);
		}
	}
}





BOOL COutputPage::OnSetActive() 
{
	BOOL rc=CeCosPropertyPage::OnSetActive();
	/*
	int nStart,nEnd;
    m_Edit.GetSel(nStart,nEnd);
TRACE(_T("Before: Start=%d end=%d\n"),nStart,nEnd);
	m_Edit.GetSel(nStart,nEnd);
TRACE(_T("After: Start=%d end=%d\n"),nStart,nEnd);
    //m_Edit.SetSel(nStart,nEnd);
    */
    return rc;
}

void COutputPage::OnSize(UINT nType, int cx, int cy) 
{
	CeCosPropertyPage::OnSize(nType, cx, cy);
    CWnd *pWnd=GetDlgItem(IDC_TT_EDIT);
    if(pWnd){
        ((CRunTestsSheet*)GetParent())->MoveWindow(pWnd,CRunTestsSheet::Stretch);
    }
}

void COutputPage::AddLogMsg(LPCTSTR psz)
{
    CString str;
    time_t ltime;
    time(&ltime);
    _tcsftime(str.GetBuffer(80),80,_T("*** %H:%M:%S "),localtime(&ltime));
    str.ReleaseBuffer();
    str+=psz;
    str+=_T("\r\n");
    AddText(str);
}
