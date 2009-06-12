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
// MessageBox.cpp : implementation file
//
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/10/06
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the messagebox class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

//

#include "stdafx.h"
#include "MessageBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessageBox dialog

// Must be global because InitModalIndirect saves the pointer
CMessageBox::DLGDATA CMessageBox::DlgData = {
	{	DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
		0,		  // No controls yet
		0,0,0,0}, // Fix up size and position later
		0,0,0};   // Default menu, class and title

void CMessageBox::Init()
{
	m_pFont=CFont::FromHandle(HFONT(GetStockObject(DEFAULT_GUI_FONT)));
	m_nFocusButton=-1;
	m_nEscapeButton=-1;
	m_nJustify=SS_LEFT;
	m_nDefaultButton=0;
	m_hIcon=NULL;
	m_pStaticText=NULL;
	m_pStaticIcon=NULL;
	m_bTopMost=false;
	m_bModeless=false;
	m_bDialogCreated=false;
	m_pParentNotify=NULL;
	m_crText=GetSysColor(COLOR_BTNTEXT);
	InitModalIndirect (&DlgData.tmpl,NULL); 
}

CMessageBox::CMessageBox()
	: CDialog()
{
	Init();
	m_strCaption=_T("Error");
}

CMessageBox::CMessageBox(const CString &strText,const CString &strCaption/*=_T("Error")*/,UINT Flag/*=MB_OK*/)
	: CDialog()
{
	Init();

	m_strText=strText;
	m_strCaption=strCaption;
	m_nDefaultButton=((Flag&MB_DEFMASK)>>8);
	m_bTopMost=(0!=(Flag&MB_SYSTEMMODAL));
	// Use flag to select from amongst standard combinations and
	// to select icon.

	switch(Flag&MB_TYPEMASK){
		case MB_OK:
			AddButton(_T("OK"),IDOK);
			break;
		case MB_OKCANCEL:
			AddButton(_T("OK"),IDOK);
			AddButton(_T("Cancel"),IDCANCEL);
			break;
		case MB_ABORTRETRYIGNORE:
			AddButton(_T("&Abort"),IDABORT);
			AddButton(_T("&Retry"),IDRETRY);
			AddButton(_T("&Ignore"),IDIGNORE);
			break;
		case MB_YESNOCANCEL:
			AddButton(_T("&Yes"),IDYES);
			AddButton(_T("&No"),IDNO);
			AddButton(_T("Cancel"),IDCANCEL);
			break;
		case MB_YESNO:
			AddButton(_T("&Yes"),IDYES);
			AddButton(_T("&No"),IDNO);
			break;
		case MB_RETRYCANCEL:
			AddButton(_T("&Retry"),IDRETRY);
			AddButton(_T("Cancel"),IDCANCEL);
			break;
		case MB_YESNOALL: //13
			AddButton(_T("&Yes"),IDYES);
			AddButton(_T("&No"),IDNO);
			AddButton(_T("Yes &All"),IDYESALL);
			AddButton(_T("No A&ll"),IDNOALL);
			break;
		case MB_YESNOALLCANCEL: //14
			AddButton(_T("&Yes"),IDYES);
			AddButton(_T("&No"),IDNO);
			AddButton(_T("Yes &All"),IDYESALL);
			AddButton(_T("No A&ll"),IDNOALL);
			AddButton(_T("Cancel"),IDCANCEL);
			break;
		default:
			ASSERT(FALSE);
	}
	
	if(Flag&MB_HELP){
		AddButton(_T("&Help"),IDHELP);
	}

	switch(Flag&MB_ICONMASK){
		case MB_ICONHAND:
			m_hIcon=AfxGetApp()->LoadStandardIcon(IDI_HAND);
			break;
		case MB_ICONQUESTION:
			m_hIcon=AfxGetApp()->LoadStandardIcon(IDI_QUESTION);
			break;
		case MB_ICONEXCLAMATION:
			m_hIcon=AfxGetApp()->LoadStandardIcon(IDI_EXCLAMATION);
			break;
		case MB_ICONASTERISK:
			m_hIcon=AfxGetApp()->LoadStandardIcon(IDI_ASTERISK);
			break;
		case 0:
			break;
		default:
			ASSERT(FALSE);
			break;
	}
}

BEGIN_MESSAGE_MAP(CMessageBox, CDialog)
	//{{AFX_MSG_MAP(CMessageBox)
	ON_WM_FONTCHANGE()
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED, 1, 0xFFFF, OnButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageBox message handlers

BOOL CMessageBox::OnInitDialog() 
{
	// Create buttons as required
	ASSERT(ButtonCount()>0);

	SetWindowText(m_strCaption);

	if(-1==m_nEscapeButton||IDCANCEL!=m_arBInfo[m_nEscapeButton].m_id){
		// No cancel button
		CMenu *pMenu=GetSystemMenu(FALSE);
		pMenu->RemoveMenu(SC_CLOSE,MF_BYCOMMAND);
	}

	CDialog::OnInitDialog();
	CDC *pDC=GetDC();
	CFont *pOldFont=pDC->SelectObject(m_pFont);
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	
	int cxDLU=tm.tmAveCharWidth;
	int cyDLU=tm.tmHeight;
	int nButtonWidth= (60*cxDLU)/4;		// width of a button
	int nButtonHeight=(14*cyDLU)/8;		// height of a button
	int cxButtonSep=   (4*cxDLU)/4;		// horizontal button separation
	int cxTextButtonSep=(10*cxDLU)/4;	// horizontal separation between text and icon
	int cyTextButtonSep=(10*cyDLU)/8;	// vertical separation between text and buttons
	int cxBorder=   (7*cxDLU)/4;		// horizontal separation between buttons and border
	int cyBorder=   (7*cyDLU)/8;		// vertical separation between buttons and border
	int cxIcon=GetSystemMetrics(SM_CXICON); // width of an icon
	int cyIcon=GetSystemMetrics(SM_CYICON); // height of an icon
	int nTotalButtonWidth=(ButtonCount()*nButtonWidth)+(ButtonCount()-1)*cxButtonSep;
	int cxText=max(50,nTotalButtonWidth-(m_hIcon?(cxIcon+cxTextButtonSep):0));
	int cyText=0;

	// Size the text control according to the maximum line length
	LPCTSTR c=m_strText;
	while(*c){
		PTCHAR d=_tcsstr(c,_T("\r\n"));
		int nCount;
		if(d){
			*d=_TCHAR('\0');
			nCount=d-c;
		} else {\
			nCount=_tcslen(c);
		}
		cxText=max(cxText,pDC->GetTextExtent(c,nCount).cx);
		cyText+=tm.tmHeight;
		if(d){
			*d=_TCHAR('\r');
			c=d+2;
		} else {
			break;
		}
	}
	
	// If vertical extent of text is less than that of the icon, difference between the two
	int cyTextExtra= (m_hIcon && cyText<cyIcon)?cyIcon-cyText:0;

	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);	

	// Set dialog box size
	{
		int cx=(2*cxBorder)+cxText+cxButtonSep+2*GetSystemMetrics(SM_CXDLGFRAME);
		if(m_hIcon){
			cx+=cxIcon+cxTextButtonSep;
		}
		int cy=(2*cyBorder)+cyText+cyTextExtra+cyTextButtonSep+nButtonHeight+
   			GetSystemMetrics(SM_CYCAPTION)+2*GetSystemMetrics(SM_CYDLGFRAME);
		UINT flags=SWP_NOMOVE;
		if(!m_bTopMost){
			flags|=SWP_NOZORDER;
		}
		SetWindowPos(&wndTopMost,0,0,cx,cy,flags);
	}

	// Create a static control for the icon
	if(m_hIcon){
		m_pStaticIcon=new CStatic;
		m_pStaticIcon->Create(NULL,WS_CHILD|WS_VISIBLE|SS_ICON,
			CRect(cxBorder,cyBorder,cxBorder+cxIcon,cyBorder+cyIcon), this);
		m_pStaticIcon->SetIcon(m_hIcon);
	}

	// Create a static control for the text
	{
		int cx=m_hIcon?cxIcon+cxTextButtonSep:0;
		m_pStaticText=new CStatic;
		m_pStaticText->Create(m_strText,WS_CHILD|WS_VISIBLE|m_nJustify|SS_NOPREFIX,
			CRect(cxBorder+cx,cyBorder+cyTextExtra/2,cxBorder+cx+cxText,cyBorder+cyText+cyTextExtra/2), this);
		m_pStaticText->SetFont(m_pFont);
	}

	// Create the buttons
	CRect rcClient;
	GetClientRect(rcClient);
	CRect rect;
	rect.left=(rcClient.Width()-nTotalButtonWidth)/2;
	rect.right=rect.left+nButtonWidth;
	rect.bottom=rcClient.bottom-cyBorder;
	rect.top=rect.bottom-nButtonHeight;

	ASSERT(m_nDefaultButton<ButtonCount());

	for(unsigned i=0;i<ButtonCount();i++){
		CButton *pWnd=new CButton;
		m_arBInfo[i].m_pWnd=pWnd;
		UINT id=m_arBInfo[i].m_id;
		UINT style=WS_CHILD|WS_VISIBLE|WS_TABSTOP;
		if(!m_arBInfo[i].m_bEnabled){
			style|=WS_DISABLED;
		}
		if(0==i){
			style|=WS_GROUP;
		}
		style|=(m_nDefaultButton==i)?BS_DEFPUSHBUTTON:BS_PUSHBUTTON;
			
		pWnd->Create(m_arBInfo[i].m_strCaption,style,rect,this,id);
		pWnd->SetFont(m_pFont);
		if(m_nDefaultButton==i){
			pWnd->SetFocus();
		}
		rect.left+=nButtonWidth+cxButtonSep;
		rect.right+=nButtonWidth+cxButtonSep;
	}

	m_nFocusButton=m_nDefaultButton;
	m_bDialogCreated=true;
	return FALSE;
}

void CMessageBox::OnButton(UINT id)
{
	if(-1!=IndexOf(id)){
		if(m_bModeless){
			if(NULL!=m_pParentNotify){
				m_pParentNotify->PostMessage(m_nParentNotifcationMessage,MAKEWPARAM(id,m_nParentNotifcationwParamHigh),0);
				DestroyWindow();
			}
		} else {
			EndDialog(id);
		}
	}
}

BOOL CMessageBox::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message==WM_KEYDOWN){
		switch(pMsg->wParam){
			case VK_ESCAPE:
				if(-1!=m_nEscapeButton){
					OnButton(m_arBInfo[m_nEscapeButton].m_id);
				}
				return TRUE;
			default:
				break;
		}
	}
    if( IsDialogMessage( pMsg ) )        
		return TRUE;    
	else
        return CDialog::PreTranslateMessage( pMsg );
}

void CMessageBox::AddButton(const CString & strCaption, UINT id, bool bEnabled/*=true*/)
{
#ifdef _DEBUG
	ASSERT(-1==IndexOf(id));
	for(unsigned int i=0;i<ButtonCount();i++){
		if(0==m_arBInfo[i].m_strCaption.Compare(strCaption)){
			ASSERT(FALSE);
		}
	}
#endif
	if(bEnabled){
		if(IDCANCEL==id || (IDOK==id && -1==m_nEscapeButton)){
			m_nEscapeButton=ButtonCount();
		} 
	}
	CButtonInfo info(id,bEnabled,strCaption);
	m_arBInfo.Add(info);
}

CMessageBox::~CMessageBox()
{
	for(unsigned int i=0;i<ButtonCount();i++){
		deleteZ(m_arBInfo[i].m_pWnd);
	}
	deleteZ(m_pStaticText);
	deleteZ(m_pStaticIcon);
}

void CMessageBox::SetDefaultButton(UINT nIndex)
{
	ASSERT(nIndex<ButtonCount());
	m_nDefaultButton=nIndex; 
}

void CMessageBox::OnFontChange() 
{
	CDialog::OnFontChange();
	
	m_pFont=CFont::FromHandle(HFONT(GetStockObject(DEFAULT_GUI_FONT)));	
	for(unsigned int i=0;i<ButtonCount();i++){
		Button(i).SetFont(m_pFont);
	}
	m_pStaticText->SetFont(m_pFont);
}



HBRUSH CMessageBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr=CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(nCtlColor){
		case CTLCOLOR_STATIC:
			pDC->SetTextColor(m_crText);
			break;
		default:
			break;
	}
	return hbr;
}


BOOL CMessageBox::Create(CWnd *pWnd,UINT msg,WORD wParamHigh)
{
	m_bModeless=true;
	if(0!=msg){
		ASSERT(NULL!=pWnd);
		m_pParentNotify=pWnd;
		m_nParentNotifcationMessage=msg;
		m_nParentNotifcationwParamHigh=wParamHigh;
	}
	return CreateIndirect (&DlgData.tmpl,pWnd); 
}

int CMessageBox::IndexOf(UINT id)
{
	for(unsigned int i=0;i<ButtonCount();i++){
		if(m_arBInfo[i].m_id==id){
			return (signed)i;
		}
	}
	return -1;
}

void CMessageBox::PostNcDestroy() 
{
	if(m_bModeless){
		delete this;
	} else {
		CDialog::PostNcDestroy();
	}
}

void CMessageBox::OnClose() 
{
	OnButton(IDCANCEL);
}

void CMessageBox::SetCaption (const CString &strCaption) 
{
	m_strCaption=strCaption;
	if(m_bDialogCreated){
		SetWindowText(strCaption);
	}
}
