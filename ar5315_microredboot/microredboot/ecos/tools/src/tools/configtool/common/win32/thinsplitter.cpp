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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	Implementation of a CSplitterWndEx override for control/cell view split
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
#include "ThinSplitter.h"
#include "ControlView.h"
//#include "CellView.h"

BEGIN_MESSAGE_MAP(CThinSplitter, CSplitterWndEx)
	//{{AFX_MSG_MAP(CThinSplitter)
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//IMPLEMENT_DYNCREATE(CThinSplitter, CSplitterWndEx)

CThinSplitter::CThinSplitter():
	m_fColumnRatio(0.75),
	CSplitterWndEx()
{
  m_GrayPen.CreatePen(PS_SOLID,1,RGB(192,192,192));	
}

CThinSplitter::~CThinSplitter()
{
}

void CThinSplitter::OnDrawSplitter( CDC* pDC, ESplitType nType, const CRect& rect )
{
	m_cxSplitterGap=1;
	switch(nType){
		case splitBar:
//TRACE("paint bar pDC=%08x\n",pDC);
			if(pDC){
				int cx=rect.left;
				CPen *pOldPen=pDC->SelectObject(&m_GrayPen);
				//static CPen redpen(PS_SOLID,1,RGB(255,0,0));
				//pDC->FillSolidRect(rect,RGB(192,192,192));
				//CPen *pOldPen=pDC->SelectObject(&redpen);

				pDC->MoveTo(cx,rect.top+m_cyBorder);
				pDC->LineTo(cx,rect.bottom-m_cyBorder);
				pDC->MoveTo(cx+1,rect.top+m_cyBorder);
				pDC->LineTo(cx+1,rect.bottom-m_cyBorder);
				pDC->SelectObject(pOldPen);
				
				return;
			}
			break;
		case splitBorder:
			//if(pDC)
			{
				CRect rcClient;
				GetClientRect(rcClient);
				CSplitterWndEx::OnDrawSplitter( pDC,splitBorder,rcClient);
				return;
			}
		default:
			break;
	}
	
	CSplitterWndEx::OnDrawSplitter( pDC,  nType,rect );
}

void CThinSplitter::DeleteColumn(int colDelete)
{
	// Do nothing - we don't allow deletion of columns
	UNUSED_ALWAYS(colDelete);
}

int CThinSplitter::HitTest(CPoint pt) const
{
	enum HitTestValue
	{
		hSplitterBar1           = 201,
	};
	int rc=CSplitterWndEx::HitTest(pt);
	if(rc==0){
		// Be more lenient about finding the splitter bar
		CRect rect;
		GetClientRect(rect);
		rect.left = m_pColInfo[0].nCurSize-3;
		rect.right = m_pColInfo[0].nCurSize + m_cxSplitterGap + 3;
		if (rect.PtInRect(pt))
		{
			rc=hSplitterBar1;
		}
	}
	return rc;
}

void CThinSplitter::OnSize( UINT nType, int cx, int cy )
{
	//if(m_nCols>1 && m_pColInfo){
	//	SetColumnInfo(0,int(m_fColumnRatio * cx + 0.5),0);
	//}
	CSplitterWndEx::OnSize( nType, cx, cy );
}

// Two reasons for this: 1-the annoying debug warning the MFC version emits and
// 2- an apparent bug whereby the scrollbar gets frozen.
void CThinSplitter::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	for (int col = 0; col < m_nCols; col++)
	{
		GetPane(0, col)->SendMessage(WM_VSCROLL,
			MAKELONG(nSBCode, nPos), (LPARAM)pScrollBar->m_hWnd);
	}
}

BOOL CThinSplitter::SplitColumn(int cxBefore){
	CRect rcClient;
	GetClientRect(rcClient);
	m_pColInfo[0].nCurSize=m_pColInfo[0].nIdealSize;
	//m_pDynamicViewClass = RUNTIME_CLASS(CCellView);
	int rc=CSplitterWndEx::SplitColumn(cxBefore);
	double d0=double(m_pColInfo[0].nCurSize);
	double d1=double(m_pColInfo[1].nCurSize);
	m_fColumnRatio=d0/(d0+d1);
	return rc;
}

// We do this because the CSplitterWndEx implementation assumes both children are derived
// from CScrollView - so here we just let the children handle the message themselves.
BOOL CThinSplitter::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	return TRUE;
	UNUSED_ALWAYS(nFlags);
	UNUSED_ALWAYS(zDelta);
	UNUSED_ALWAYS(pt);
}

void CThinSplitter::RecalcLayout()
{
	CSplitterWndEx::RecalcLayout();
	if (m_nCols==2 && m_pColInfo[0].nCurSize && m_pColInfo[1].nCurSize){
		CRect rect;
		GetClientRect(rect);
		m_fColumnRatio=double(m_pColInfo[0].nCurSize)/double(rect.Width());
	}
}

BOOL CThinSplitter::CreateStatic( CWnd* pParentWnd, int nRows, int nCols, DWORD dwStyle, UINT nID)
{
  bool rc=CSplitterWndEx::CreateStatic(pParentWnd, nRows, nCols, dwStyle, nID);
  m_nMaxRows=1;
  return rc;
}

