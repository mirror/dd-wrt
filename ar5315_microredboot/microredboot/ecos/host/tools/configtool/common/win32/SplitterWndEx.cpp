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
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the extended splitter window class.
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
#include "SplitterWndEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////
//
// CSplitterWndEx

CSplitterWndEx::CSplitterWndEx():
	m_fRowColumnRatio(0.0)
{
	m_bParentHide=FALSE;
	m_pwnd1=m_pwnd2=NULL;
}

BEGIN_MESSAGE_MAP(CSplitterWndEx, CSplitterWnd)
//{{AFX_MSG_MAP(CSplitterWndEx)
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSplitterWndEx::ShowPane(CWnd * pWnd,BOOL bShow)
{
	if(m_pwnd1==NULL){
		m_pwnd1=GetPane(0,0);
		m_pwnd2=GetPane(!IsHorizontal(),IsHorizontal());
	}
	
	ASSERT(pWnd==m_pwnd1 || pWnd==m_pwnd2);

	int &nRowCols=IsHorizontal()?m_nCols:m_nRows;

	if(bShow){
		if(!pWnd->IsWindowVisible()){
			if(m_bParentHide){
				((CSplitterWndEx *)GetParent())->ShowPane(this,TRUE);
				// Ensure the correct pane reappears
				if(GetPane(0,0)!=pWnd){
					// Need to swap the entries
					SwapRowColInfo();
				}
				m_bParentHide=FALSE;
			} else { // maybe nothing to do
				nRowCols++;
				if(pWnd==m_pwnd1){
					// Need to swap the entries
					SwapRowColInfo();
				}
			}

			pWnd->ShowWindow(SW_SHOW);
		}
		if(pWnd==m_pwnd1){
			m_pwnd1->SetDlgCtrlID(AFX_IDW_PANE_FIRST);
			m_pwnd2->SetDlgCtrlID(AFX_IDW_PANE_FIRST+(IsHorizontal()?1:16));
		}
	} else {
		if(pWnd->IsWindowVisible()){

			pWnd->ShowWindow(SW_HIDE);
			if(nRowCols == 1) {
				m_bParentHide=TRUE;
				((CSplitterWndEx *)GetParent())->ShowPane(this,FALSE);
			} else {
				// Both panes currently active
				int rowActive, colActive;
				if (GetActivePane(&rowActive, &colActive) != NULL && pWnd==GetPane(rowActive, colActive)){
          int bHorz=IsHorizontal();
					SetActivePane((!bHorz)&(rowActive^1), (bHorz)&(colActive^1));
				}
				if(pWnd==m_pwnd1){
					SwapRowColInfo();
					m_pwnd2->SetDlgCtrlID(AFX_IDW_PANE_FIRST);
					m_pwnd1->SetDlgCtrlID(AFX_IDW_PANE_FIRST+1);
				}
				nRowCols--;
			}
		}
	}
	RecalcLayout();
}

void CSplitterWndEx::SwapRowColInfo()
{
	CRowColInfo*& pRowColInfo=IsHorizontal()?m_pColInfo:m_pRowInfo;

	// Need to swap the entries
	CRowColInfo save;
	memcpy(&save,&pRowColInfo[0],sizeof CRowColInfo);
	memcpy(&pRowColInfo[0],&pRowColInfo[1],sizeof CRowColInfo);
	memcpy(&pRowColInfo[1],&save,sizeof CRowColInfo);
}


// We do this because the CSplitterWnd implementation assumes both children are derived
// from CScrollView - so here we just let the children handle the message themselves.
BOOL CSplitterWndEx::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	return TRUE;
	UNUSED_ALWAYS(nFlags);
	UNUSED_ALWAYS(zDelta);
	UNUSED_ALWAYS(pt);
}

void CSplitterWndEx::OnSize(UINT nType, int cx, int cy) 
{
	int &nRowCols=IsHorizontal()?m_nCols:m_nRows;
	CRowColInfo *&RowColInfo=IsHorizontal()?m_pColInfo:m_pRowInfo;
	if(nRowCols>1 && (cx|cy) && m_fRowColumnRatio!=0.0){
		RowColInfo[0].nIdealSize=int(0.5 + m_fRowColumnRatio * double(IsHorizontal()?cx:cy));
	}
	CSplitterWnd::OnSize( nType, cx, cy );
}

void CSplitterWndEx::RecalcLayout()
{
	CSplitterWnd::RecalcLayout();
	CRowColInfo *&RowColInfo=IsHorizontal()?m_pColInfo:m_pRowInfo;
	int &nRowCols=IsHorizontal()?m_nCols:m_nRows;
	if (nRowCols==2 && RowColInfo[0].nCurSize && RowColInfo[1].nCurSize){
		CRect rect;
		GetClientRect(rect);
		m_fRowColumnRatio=double(RowColInfo[0].nCurSize)/double(IsHorizontal()?rect.Width():rect.Height());
	}
}

double CSplitterWndEx::GetSplitPoint()
{
  return m_fRowColumnRatio;
}

void CSplitterWndEx::SetSplitPoint(double fSplit)
{
  m_fRowColumnRatio=fSplit;
  CRect rc;
  GetClientRect(rc);
  OnSize(SIZE_RESTORED,rc.Width(),rc.Height());
}
