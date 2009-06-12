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
// OutputEdit.cpp : implementation file
//

#include "stdafx.h"
#include "OutputEdit.h"
#include "TestToolRes.h"		// main symbols

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputEdit

COutputEdit::COutputEdit()
{
}

COutputEdit::~COutputEdit()
{
}


BEGIN_MESSAGE_MAP(COutputEdit, CEdit)
	//{{AFX_MSG_MAP(COutputEdit)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_COMMAND(ID_TT_EDIT_SAVE, OnEditSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutputEdit message handlers

void COutputEdit::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    UNUSED_ALWAYS(pWnd);
    if(GetWindowTextLength()>0){
	    CMenu menu;
	    menu.LoadMenu(IDR_TT_CONTEXTMENU2);

	    CMenu *pPopup=menu.GetSubMenu(0);
        
        int nBeg, nEnd;

        GetSel( nBeg, nEnd );
        
        if(nBeg==nEnd ){
            pPopup->EnableMenuItem(ID_EDIT_COPY,MF_BYCOMMAND|MF_GRAYED);
        }
        pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x,point.y,this);
    }	    
}

void COutputEdit::OnEditSelectAll() 
{
	SetSel(0,-1);
}

void COutputEdit::OnEditCopy() 
{
	Copy();	
}

void COutputEdit::OnEditClear() 
{
	SetWindowText(_T(""));
}

void COutputEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    switch(nChar){
        case 1: // ctrl+A
            OnEditSelectAll();
            break;
        case 3: // ctrl+C
            OnEditCopy();
            break;
        default:
            CEdit::OnChar(nChar, nRepCnt, nFlags);
            break;
    }
}

// Control gets sent WM_SETSEL (0,0xffffffff) when property page is selected
// I don't know why, but this works around it:
void COutputEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
    SetSel(m_dwSel);
	
}

void COutputEdit::OnKillFocus(CWnd* pNewWnd) 
{
    m_dwSel=GetSel();	
	CEdit::OnKillFocus(pNewWnd);
}

void COutputEdit::OnEditSave() 
{
	CFileDialog dlg( FALSE, _T("log"), _T("Output"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 		
		_T("Log Files (*.log)|*.log|All Files (*.*)|*.*||"));
	if(IDOK==dlg.DoModal()){
	    TRY
	    {    
		    CStdioFile f( dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite );
		    CString str;
		    GetWindowText(str);
		    f.WriteString(str);
		    f.Close();
	    }
	    CATCH( CFileException, e )
	    {
		    MessageBox(_T("Failed to write file"));
	    }
	    END_CATCH
	}
	
}
