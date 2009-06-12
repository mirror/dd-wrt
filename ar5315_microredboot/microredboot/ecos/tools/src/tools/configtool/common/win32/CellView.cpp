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
// Description:	Implementation of a the cell window view class
// Requires:	
// Provides:	
// See also:#include	
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include "stdafx.h"
#include "ConfigTool.h"
#include "CellView.h"
#include "ControlView.h"
#include "ConfigItem.h"
#include "IntegerEdit.h"
#include "DoubleEdit.h"
#include "StringEdit.h"
#include "ComboEdit.h"

#include "CTUtils.h"
#include "ConfigToolDoc.h"

#include "resource.h"

#ifdef PLUGIN
  #define INCLUDEFILE "ide.guicommon.h" // for ID_EDIT_FINDAGAIN
  #include "IncludeSTL.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCellView

IMPLEMENT_DYNCREATE(CCellView, CView)

CCellView::CCellView():
  m_bComboSellPending(false)
{
  m_hInCell=NULL;
  m_pwndCell=NULL;
  m_GrayPen.CreatePen(PS_SOLID,1,RGB(192,192,192));	
  CConfigTool::SetCellView(this);
}

CCellView::~CCellView()
{
    deleteZ(m_pwndCell);
    CConfigTool::SetCellView(0);
}


BEGIN_MESSAGE_MAP(CCellView, CView)
	//{{AFX_MSG_MAP(CCellView)
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
    ON_COMMAND(ID_EDIT_FIND,OnEditFind)
    ON_COMMAND(ID_EDIT_FINDAGAIN,OnEditFindAgain)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FINDAGAIN, OnUpdateEditFindAgain)
	ON_WM_RBUTTONDOWN()
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_EDIT_COPY,   OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT,    OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE,  OnEditPaste)
	ON_COMMAND(ID_EDIT_CLEAR,  OnEditDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY,   OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT,    OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE,  OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR,  OnUpdateEditDelete)
	ON_WM_HELPINFO()
    ON_MESSAGE(WM_CANCEL_EDIT,OnCancelEdit)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCellView drawing

void CCellView::OnInitialUpdate()
{
	m_nFirstVisibleItem=0;
	CView::OnInitialUpdate();
}

void CCellView::OnDraw(CDC* pDC)
{
	CTreeCtrl &Tree=CConfigTool::GetControlView()->GetTreeCtrl();

	CConfigToolDoc* pDoc=CConfigTool::GetConfigToolDoc();
	if(pDoc->ItemCount()>0)
	{
		CRect rect;
		Tree.GetItemRect(Tree.GetRootItem(),rect,TRUE);
		m_nFirstVisibleItem=rect.top;

		CRect rcClient;
		GetClientRect(rcClient);
		CPen *pOldPen=pDC->SelectObject(&m_GrayPen);
		CFont *pOldFont=pDC->SelectObject(CConfigTool::GetControlView()->GetFont());
		pDC->SetBkMode(TRANSPARENT);

		CRect rcClip;
		pDC->GetClipBox(rcClip);
		
		CPtrArray arItems;
		int dy=CConfigTool::GetControlView()->GetItemHeight();
		int cy=0;
		for(HTREEITEM h=Tree.GetFirstVisibleItem();h;h=Tree.GetNextVisibleItem(h))
		{
			if(cy>rcClip.bottom){
				break;
			}

			CRect rcEdit(0,cy,rcClient.right,cy+dy);
			cy+=dy;

			pDC->MoveTo(rcClient.left,rcEdit.top);
			pDC->LineTo(rcClient.right,rcEdit.top);

			CConfigItem &ti=TI(h);
			if(h!=m_hInCell){
        switch(ti.Type()){
		      case CConfigItem::Enum:
						// Using combobox
						rcEdit.left+=2;
						rcEdit.top+=2;
            // fall through
          case CConfigItem::Integer:
          case CConfigItem::Double:
          case CConfigItem::String:
						// Using editbox
						rcEdit.top+=2;
						rcEdit.left+=3;
            {
				      CString str(ti.StringValue());
				      // cell contents is greyed if the option is not both active and modifiable
				      // or if a booldata item is not enabled
				      const CdlValuable valuable = ti.GetCdlValuable();
				      // check for a package explicitly because is_modifiable() returns true for a package
				      pDC->SetTextColor (GetSysColor ((! valuable) || (valuable->is_modifiable () && valuable->is_active () && ((! ti.HasBool ()) || ti.IsEnabled ()) && ! ti.IsPackage ()) ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
				      pDC->TextOut(rcEdit.left,rcEdit.top,str);
            }
            break;
          default:
            break;
        }
			}
		}
		pDC->MoveTo(rcClient.left,cy);
		pDC->LineTo(rcClient.right,cy);
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldFont);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCellView diagnostics

#ifdef _DEBUG
void CCellView::AssertValid() const
{
	CView::AssertValid();
}

void CCellView::Dump(CDumpContext&	dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCellView message handlers

void CCellView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint)
	{
		case CConfigToolDoc::IntFormatChanged:
      {
			  for(HTREEITEM h=CConfigTool::GetControlView()->GetFirstVisibleItem();h;h=CConfigTool::GetControlView()->GetNextVisibleItem(h)){
				  CConfigItem &ti=TI(h);
          if(ti.Type()==CConfigItem::Integer) {
					  CRect rect;
					  GetItemRect(h,rect);
					  InvalidateRect(rect);
				  }
			  }
        if(m_pwndCell && TI(m_hInCell).Type()==CConfigItem::Integer) {
				  CString strData;
				  m_pwndCell->GetWindowText(strData);
				  ItemIntegerType n;
				  CUtils::StrToItemIntegerType(strData,n);
          m_pwndCell->SetWindowText(CUtils::IntToStr(n,CConfigTool::GetConfigToolDoc()->m_bHex));
			  }
      }
			break;
		case CConfigToolDoc::Clear:	
			deleteZ(m_pwndCell);
			Invalidate();
			UpdateWindow(); // This prevents cell view half of config pane still being displayed
      break;
		case CConfigToolDoc::ValueChanged:
			{
				CConfigItem *pti=(CConfigItem *)pHint;
				CRect rect;
				GetItemRect(pti->HItem(),rect);
				InvalidateRect(rect);
			}
			break;
		case 0:
			Invalidate();
			break;
		default:
			break;
	}	
	UNUSED_ALWAYS(pSender);
}

void CCellView::GetItemRect(HTREEITEM h,CRect & rect) const
{
	CRect rcClient;
	GetClientRect(rcClient);

	CConfigTool::GetControlView()->GetItemRect( h, &rect, FALSE );
	rect.left=rcClient.left;
	rect.right=rcClient.right;
}

void CCellView::GetInCellRect(HTREEITEM h, CRect & rect, BOOL bDropped) const
{
	CConfigItem &ti=TI(h);
	GetItemRect(h,rect);
  switch(ti.Type()){
		case CConfigItem::Enum:
			// Using combobox
      if(bDropped) {
        CStringArray arEnum;
        ti.EvalEnumStrings(arEnum);
        rect.bottom += (2+(int)arEnum.GetSize()-1)*rect.Height();
      }
      break;
    case CConfigItem::Integer:  
    case CConfigItem::Double:
    case CConfigItem::String:
			// Using editbox
			//rect.bottom++;
			//rect.DeflateRect(2,2); // To allow room  for the border we draw ourselves
      //rect.InflateRect(2,2);
      break;
    default:
      return;
  }
}

ItemIntegerType CCellView::GetCellValue() const
{
  // If the item is being edited in-cell, we'll get the data from the control
  ItemIntegerType rc=0;
  switch(TI(m_hInCell).Type()){
		case CConfigItem::Integer:
      {
        CString strData;
        m_pwndCell->GetWindowText(strData);
        if(!CUtils::StrToItemIntegerType(strData,rc)){
          rc=0;
        }
      }
      break;
    case CConfigItem::Enum:
    /*
    case CConfigItem::Boolean:
    case CConfigItem::Radio:
    //rc=((CTreeComboBox *)m_pwndCell)->GetCurSel();
    rc=((CComboEdit *)m_pwndCell)->GetCurSel();
    break;
      */
    case CConfigItem::String:
    default:
      int type=TI(m_hInCell).Type();
      UNUSED_ALWAYS(type);
      ASSERT(FALSE);
      break;
  }
  return rc;
}

void CCellView::CancelCellEdit(bool bApplyChanges)
{
  if(m_hInCell){
    CConfigItem &ti=TI(m_hInCell);
    if(bApplyChanges){
      CString strValue;
      m_pwndCell->GetWindowText(strValue);
      // Ignore empty strings in integer or floating cells - these are legal as intermediate values but not now
      if(strValue!=ti.StringValue() && (!strValue.IsEmpty() || (ti.Type()!=CConfigItem::Integer && ti.Type()!=CConfigItem::Double))){
        CConfigTool::GetConfigToolDoc()->SetValue (ti, strValue);
      }
    }

    deleteZ(m_pwndCell);
    m_hInCell=NULL;
  }
}

BOOL CCellView::InCell(HTREEITEM h)
{
  CancelCellEdit();
  if(h && TI(h).IsEnabled()){
    CConfigItem &ti=TI(h);
    // edit cell only if option is both active and modifiable
    const CdlValuable valuable = ti.GetCdlValuable();
    // check packages explicitly because is_modifiable() returns true for a package
    if ((! valuable) || (valuable->is_modifiable () && valuable->is_active () && ! ti.IsPackage ())){
      CRect rcEdit;
      GetItemRect(h,rcEdit);
      rcEdit.bottom++;
      switch(ti.Type()){
        case CConfigItem::Double:
          {
            double d;
            CUtils::StrToDouble(ti.StringValue(),d);
            m_pwndCell=new CDoubleEdit(d);
            m_pwndCell->Create(WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, rcEdit, this, IDC_CELL);
          }
          break;
        case CConfigItem::Integer:
          {
            ItemIntegerType i;
            CUtils::StrToItemIntegerType(ti.StringValue(),i);
            m_pwndCell=new CIntegerEdit(i);
          }
          m_pwndCell->Create(WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, rcEdit, this, IDC_CELL);
          break;
        case CConfigItem::String:
          {
            CStringEdit *pStringEdit=new CStringEdit(ti.StringValue());
            m_pwndCell=pStringEdit;
            m_pwndCell->Create(WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, rcEdit, this, IDC_CELL);
            pStringEdit->EnableDoubleClickEdit(true,IDC_CT_EDIT);
          }

          break;
        case CConfigItem::Enum:
          {
            CStringArray arEnum;
            ti.EvalEnumStrings(arEnum);
            rcEdit.bottom += (2+(int)arEnum.GetSize()-1)*rcEdit.Height();
            m_pwndCell=new CComboEdit(ti.StringValue(),arEnum);
            m_pwndCell->Create(WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST, rcEdit, this, IDC_CELL);
          }
          break;
        default:
          return 0;
          break;
      }
      m_hInCell=h;
      m_pwndCell->SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));
      m_pwndCell->SetFocus();
      m_pwndCell->GetWindowText(m_strInitialCell);
    }
  }
  return NULL!=m_hInCell;
}

void CCellView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    UNUSED_ALWAYS(nFlags);
	CTreeCtrl &Tree=CConfigTool::GetControlView()->GetTreeCtrl();
	CancelCellEdit();
	HTREEITEM h=HitTest();
	if(h){
		InCell(h);
		Tree.SelectItem(h);
	}

	// Relay to the splitter
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);
	GetParent()->SendMessage(WM_LBUTTONDOWN,(WPARAM)nFlags,MAKELPARAM(point.x,point.y));	
}

void CCellView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	if(m_hInCell){
//sdf1		UpdateWindow();
		CRect rect;
    GetItemRect(m_hInCell,rect);
		m_pwndCell->MoveWindow(rect,TRUE);
	}
	
}

int CCellView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}



BOOL CCellView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style &= (~WS_BORDER);
	return CView::PreCreateWindow(cs);
}

void CCellView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CView::OnVScroll(nSBCode, nPos, pScrollBar);
	Sync();
}

void CCellView::Sync()
{
	CTreeCtrl &t=CConfigTool::GetControlView()->GetTreeCtrl();
	CRect rect;
	t.GetItemRect(t.GetRootItem(),rect,TRUE);
	int pos=rect.top;

	if(pos!=m_nFirstVisibleItem){
		ScrollWindow(0,pos-m_nFirstVisibleItem);
    UpdateWindow();
	}
}


void CCellView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Relay the mouse event to the splitter
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);
	GetParent()->SendMessage(WM_MOUSEMOVE,(WPARAM)nFlags,MAKELPARAM(point.x,point.y));	
}


BOOL CCellView::OnMouseWheel(UINT, short zDelta, CPoint) 
{
  UINT nScrollCode=((zDelta<0)?SB_LINEDOWN:SB_LINEUP);
  LPARAM lParam=(LPARAM)GetScrollBarCtrl(SB_VERT)->GetSafeHwnd();
  if(lParam){
    for(int i=0;i<abs(zDelta)/WHEEL_DELTA;i++){
      CConfigTool::GetControlView()->SendMessage(WM_VSCROLL,MAKEWPARAM(nScrollCode,0),lParam);
      Sync();
    }
  }
  return TRUE;
}

void CCellView::OnEditFind() 
{
    CConfigTool::GetControlView()->OnEditFind();
}

void CCellView::OnEditFindAgain() 
{
    CConfigTool::GetControlView()->OnEditFindAgain();
}

void CCellView::OnUpdateEditFindAgain(CCmdUI* pCmdUI) 
{
    CConfigTool::GetControlView()->OnUpdateEditFindAgain(pCmdUI);
}

void CCellView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    UNUSED_ALWAYS(nFlags);
    // Make the r-click have an effect equivalent to that when on the control view part
    CControlView *pv=CConfigTool::GetControlView();
	HTREEITEM h=HitTest();
	if(h){
		pv->SelectItem(h);
    }
	// point is in client coords
	ClientToScreen(&point);
	pv->ShowPopupMenu(h,point);
}

void CCellView::OnUpdateEditFind(CCmdUI* pCmdUI) 
{
    CConfigTool::GetControlView()->OnUpdateEditFind(pCmdUI);
}

BOOL CCellView::OnEraseBkgnd(CDC* pDC) 
{
    /*
    static int x=3;
    const MSG *pMsg=GetCurrentMessage();
	WNDCLASS wndcls;
    if (::GetClassInfo(NULL, _T("AfxFrameOrView42ud"), &wndcls)){
        TRACE(_T("proc=%08x hbrBackground=%08x "),wndcls.lpfnWndProc,wndcls.hbrBackground);
    }
   
    TRACE(_T("msg=%d hWnd=%08x wParam=%08x lParam=%08x m_pfnSuper=%08x super=%08x\n"),
        pMsg->message,pMsg->hwnd, pMsg->lParam,pMsg->wParam,m_pfnSuper,	GetSuperWndProcAddr());
    if(x){
        return CView::OnEraseBkgnd(pDC);
    }
    */

    // Work around bug apparently caused by SlickEdit
	CRect rcClient;
	pDC->GetClipBox(rcClient);
	pDC->FillSolidRect(rcClient,GetSysColor(COLOR_WINDOW));
		
	return TRUE;
}

BOOL CCellView::PreTranslateMessage(MSG* pMsg) 
{
	CTreeCtrl &Tree=CConfigTool::GetControlView()->GetTreeCtrl();
    if(WM_KEYDOWN==pMsg->message){
		switch(pMsg->wParam){
            case VK_TAB:
                {
                 	HTREEITEM h=Tree.GetSelectedItem();
                    if(h){
			            if(0==(::GetKeyState(VK_SHIFT)&0x8000)){
				            // Shift key not down
                            h=Tree.GetNextVisibleItem(h);
                        }
                        if(h){
                            CancelCellEdit();
                    		Tree.SelectItem(h);
                            Tree.SetFocus();
                        } else {
                            MessageBeep (0xFFFFFFFF);
                        }
                        // Always handle this message to keep focus within the tree control
                        return true;
                    }
                }
		    }
    }
	return CView::PreTranslateMessage(pMsg);
}

HTREEITEM CCellView::HitTest()
{
	// Can use the control view's hittest because all it's interested in is the y coord

	TVHITTESTINFO tvi;
    DWORD dwPos=GetMessagePos();
	CTreeCtrl &Tree=CConfigTool::GetControlView()->GetTreeCtrl();
    tvi.pt.y=GET_Y_LPARAM(dwPos);
    Tree.ScreenToClient(&tvi.pt);
    tvi.pt.x=0;
	return Tree.HitTest(&tvi);

}

void CCellView::OnEditCopy() 
{
  if(m_pwndCell){
    m_pwndCell->OnEditCopy();
  }
}

void CCellView::OnEditCut() 
{
  if(m_pwndCell){
    m_pwndCell->OnEditCut();
  }
}

void CCellView::OnEditPaste() 
{
  if(m_pwndCell){
    m_pwndCell->OnEditPaste();
  }
}

void CCellView::OnEditDelete() 
{
  if(m_pwndCell){
    m_pwndCell->OnEditDelete();
  }
}

void CCellView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
  if(m_pwndCell){
    m_pwndCell->OnUpdateEditCopy(pCmdUI);
  } else {
    pCmdUI->Enable(false);
  }
}

void CCellView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
  if(m_pwndCell){
    m_pwndCell->OnUpdateEditCut(pCmdUI);
  } else {
    pCmdUI->Enable(false);
  }
}

void CCellView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
  if(m_pwndCell){
    m_pwndCell->OnUpdateEditPaste(pCmdUI);
  } else {
    pCmdUI->Enable(false);
  }
}

void CCellView::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
  if(m_pwndCell){
    m_pwndCell->OnUpdateEditDelete(pCmdUI);
  } else {
    pCmdUI->Enable(false);
  }
}

BOOL CCellView::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	return CConfigTool::GetControlView()->OnHelpInfo(pHelpInfo);
}

LRESULT CCellView::OnCancelEdit(WPARAM wParam, LPARAM)
{
  CancelCellEdit(wParam);
  return 0;
}
