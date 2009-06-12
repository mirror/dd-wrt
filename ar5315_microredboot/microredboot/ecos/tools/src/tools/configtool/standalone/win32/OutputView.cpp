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
// OutputView.cpp : implementation file
//
//
//===========================================================================
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the output window view
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
#ifndef PLUGIN
#include "BCMenu.h"
#endif
#include "ConfigTool.h"
#include "OutputView.h"
#include "MainFrm.h"
#include "FindDialog.h"
#include "CTUtils.h"
#include "ConfigTooldoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputView

IMPLEMENT_DYNCREATE(COutputView, CEditView)

COutputView::COutputView()
{
    CConfigTool::SetOutputView(this);
}

COutputView::~COutputView()
{
    CConfigTool::SetOutputView(0);
}

static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);

BEGIN_MESSAGE_MAP(COutputView, CEditView)
	//{{AFX_MSG_MAP(COutputView)
	ON_WM_CONTEXTMENU()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
    ON_COMMAND(ID_EDIT_FINDAGAIN,OnEditFindAgain)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, OnEditFindReplace)
	ON_CONTROL_REFLECT(EN_CHANGE, OnEditChange)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditDelete)
	ON_COMMAND(ID_EDIT_CLEAR_ALL, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR_ALL, OnUpdateEditClear)
	ON_COMMAND(ID_LOG_SAVE, OnFileSave)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_WM_MENUCHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutputView drawing

void COutputView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
	UNUSED_ALWAYS(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// COutputView diagnostics

#ifdef _DEBUG
void COutputView::AssertValid() const
{
	CEditView::AssertValid();
}

void COutputView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COutputView message handlers

void COutputView::AddText(const CString & str)
{
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

	{
		int nStart,nEnd;
		int nLength=GetBufferLength();
		GetEditCtrl().GetSel(nStart,nEnd);
		// Replace selection
		GetEditCtrl().SetSel(nLength,nLength,TRUE);
		GetEditCtrl().ReplaceSel(strText);
		if(GetBufferLength()!=unsigned(nLength+strText.GetLength())){
			// Try again by removing equivalent length from start of buffer.
			// For neatness, remove whole lines
			int nLine=GetEditCtrl().LineFromChar(strText.GetLength()-1);
			int nIndex=GetEditCtrl().LineIndex(nLine+1);
			GetEditCtrl().SetSel(0,nIndex-1,TRUE);
			GetEditCtrl().ReplaceSel(_T(""));
			nLength=GetBufferLength();
			GetEditCtrl().SetSel(nLength,nLength,TRUE);
			GetEditCtrl().ReplaceSel(strText);
		} else if(nStart!=nEnd) {
			GetEditCtrl().SetSel(nStart,nEnd,TRUE);
		}
	}
}

void COutputView::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();
	CConfigToolApp*pApp=(CConfigToolApp*)AfxGetApp();
	if(pApp->m_strBufferedLogMessages){
		AddText(pApp->m_strBufferedLogMessages);
		pApp->m_strBufferedLogMessages=_T("");
	}
}

void COutputView::Clear()
{
	GetEditCtrl().SetSel(0,-1);
	GetEditCtrl().Clear();
}

BOOL COutputView::PreCreateWindow(CREATESTRUCT& cs) 
{
	//cs.style|=ES_MULTILINE|ES_READONLY|WS_VSCROLL;
	cs.style|=ES_MULTILINE|WS_VSCROLL|ES_NOHIDESEL;
	return CEditView::PreCreateWindow(cs);
}

void COutputView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(VK_DELETE==nChar){
		MessageBeep (MB_OK);
	} else {
		CEditView::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void COutputView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	Menu menu;
	menu.LoadMenu(IDR_OUTPUT_CONTEXT);
  menu.LoadToolbar(IDR_MAINFRAME);
	Menu *pPopup=(Menu *)menu.GetSubMenu(0);
	if(point.x<0){
		point=GetCaretPos();
		point.x=max(3,point.x);
		point.y=max(3,point.y);
		ClientToScreen(&point);
	}
	pPopup->TrackPopupMenu(TPM_LEFTALIGN, point.x,point.y,this);
	UNUSED_ALWAYS(pWnd);
}

void COutputView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar){
		case 0x03: // ctrl+c
			SendMessage(WM_COMMAND,ID_EDIT_COPY,0);
			break;
		case 0x01: // ctrl+a
			SendMessage(WM_COMMAND,ID_EDIT_SELECT_ALL,0);
			break;
		default:
			MessageBeep	(MB_OK);
			break;
	}

	UNUSED_ALWAYS(nFlags);
	UNUSED_ALWAYS(nRepCnt);
}

void COutputView::OnEditClear() 
{
	Clear();	
}

void COutputView::OnEditUndo() 
{
	MessageBeep (MB_OK);
}

void COutputView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint){
		case 0:
		default:
			return;
	}
	UNUSED_ALWAYS(pSender);
	UNUSED_ALWAYS(pHint);
}

void COutputView::OnFileSave() 
{
	CFileDialog dlg( FALSE, _T("log"), _T("ConfigTool"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 		
		_T("Log Files (*.log)|*.log|All Files (*.*)|*.*||"));
	if(IDOK==dlg.DoModal()){
		Save(dlg.GetPathName());
	}
}

void COutputView::OnEditSelectAll() 
{
	GetEditCtrl().SetSel(0,-1);
}

void COutputView::OnEditCopy() 
{
	GetEditCtrl().Copy();	
}

void COutputView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
  int nStart,nEnd;
  GetEditCtrl().GetSel(nStart,nEnd);
	pCmdUI->Enable(nStart!=nEnd);
}

void COutputView::OnUpdateEditClear(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetBufferLength()!=0);
}

void COutputView::Save(const CString & strFile)
{
	TRY
	{    
		CStdioFile f( strFile, CFile::modeCreate | CFile::modeWrite );
		CStringArray arstr;
		GetContents(arstr);
		for(int i=0;i<arstr.GetSize();i++){
			f.WriteString(arstr[i]);
			f.WriteString(_T("\n"));
		}
		f.Close();
	}
	CATCH( CFileException, e )
	{
		CUtils::MessageBoxF(_T("Failed to write to %s - %s"),strFile,CUtils::Explanation(*e));
	}
	END_CATCH

}

void COutputView::GetContents(CStringArray & arstr)
{
	arstr.SetSize(GetEditCtrl().GetLineCount());
	for(int i=0;i<arstr.GetSize();i++){
		CString &str=arstr[i];
		for(int n=256;;n+=256){
			TCHAR *pszBuf=str.GetBuffer(n);
			int nLineLen=GetEditCtrl().GetLine(i,pszBuf,n-1);
			pszBuf[nLineLen]=_TCHAR('\0');
			str.ReleaseBuffer();
			if(nLineLen<n-1){
				break;
			}
		}
	}
}

void COutputView::OnEditFindAgain() 
{
    CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
    if(!FindText(pDoc->m_strFind,pDoc->m_nFindFlags&&FR_DOWN, pDoc->m_nFindFlags&FR_MATCHCASE)){
		CUtils::MessageBoxF(_T("Cannot find '%s'"),pDoc->m_strFind);
    }
}

void COutputView::OnUpdateEditFindAgain(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(!CConfigTool::GetConfigToolDoc()->m_strFind.IsEmpty() && m_hWnd==CWnd::GetFocus()->m_hWnd);
}

void COutputView::OnFindNext( LPCTSTR lpszFind, BOOL bNext, BOOL bCase )
{
    CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
	pDoc->m_strFind=lpszFind;
    pDoc->m_nFindFlags=(bNext?FR_DOWN:0)|(bCase?FR_MATCHCASE:0);
    CEditView::OnFindNext(lpszFind, bNext, bCase );
}

void COutputView::OnTextNotFound( LPCTSTR lpszFind )
{
    CUtils::MessageBoxF(_T("Cannot find '%s'"),lpszFind);
    CEditView::OnTextNotFound(lpszFind);
}

void COutputView::OnEditFind() 
{
    CConfigTool::GetMain()->m_bFindInProgress=true;	
    CEditView::OnEditFind();
}

void COutputView::OnUpdateEditFind(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!CConfigTool::GetMain()->m_bFindInProgress);	
}

LONG COutputView::OnEditFindReplace(WPARAM wParam, LPARAM lParam)
{
	CFindReplaceDialog* pDialog = CFindReplaceDialog::GetNotifier(lParam);
    if(pDialog->IsTerminating()){
        CConfigTool::GetMain()->m_bFindInProgress=false;
    }
    return CEditView::OnFindReplaceCmd(wParam, lParam);
}

void COutputView::OnEditChange ()
{
	// override CEditView::OnEditChange() to prevent
	// setting of the document modified flag
}

void COutputView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(false);
}

void COutputView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(false);
}

void COutputView::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(false);
}

LRESULT COutputView::OnMenuChar(UINT, UINT, CMenu*)
{
  const MSG *pMsg=GetCurrentMessage();
  // punt to the mainframe to deal with shortcuts in popups
  return AfxGetMainWnd()->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
}
