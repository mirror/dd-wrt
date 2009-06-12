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
// ControlView.cpp : implementation file
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
// Description:	This is the implementation of the tree (control) view
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
#include "ControlView.h"
#include "ConfigTool.h"
#include "ConfigItem.h"
#include "ConfigToolDoc.h"
#include <stdlib.h>
#include "CTUtils.h"
#include "CellView.h"
#include "MessageBox.h"
#include "FindDialog.h"
#include "CTPropertiesDialog.h"
#include "ConfigViewOptionsDialog.h"
#ifdef PLUGIN
  //#include "ide.common.h"
  #define INCLUDEFILE "ide.guicommon.h"  // for ID_EDIT_FINDAGAIN
  #include "IncludeSTL.h"
  #include "CTMain.h"
#else
  #include "MainFrm.h"
#endif

#include <afxpriv.h> // for WM_COMMANDHELP, WM_HITTEST
#include <htmlhelp.h>

#include "DescView.h" // for testing
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CControlView

IMPLEMENT_DYNCREATE(CControlView, CTreeView)

CControlView::CControlView()
{
  m_bHasVScroll=-1;
  m_nWorkspace=0;
  m_dwDefaultStyle&=(~WS_VSCROLL);
  m_hContext=NULL;
  m_hExpandedForFind=NULL;
  m_GrayPen.CreatePen(PS_SOLID,1,RGB(192,192,192));	
  CConfigTool::SetControlView(this);
}

CControlView::~CControlView()
{
  CConfigTool::SetControlView(0);
}

static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);

BEGIN_MESSAGE_MAP(CControlView, CTreeView)
//{{AFX_MSG_MAP(CControlView)
ON_WM_PAINT()
ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemexpanded)
ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
ON_WM_LBUTTONDOWN()
ON_WM_VSCROLL()
ON_WM_SIZE()
ON_COMMAND(ID_RESTORE_DEFAULTS, OnRestoreDefaults)
ON_COMMAND(ID_POPUP_PROPERTIES, OnPopupProperties)
ON_COMMAND(ID_UNLOAD_PACKAGE, OnUnload)
ON_WM_KEYDOWN()
ON_COMMAND(ID_VIEW_URL, OnViewUrl)
ON_WM_CREATE()
ON_WM_HSCROLL()
ON_WM_MOUSEMOVE()
ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemexpanding)
ON_WM_MOUSEWHEEL()
ON_WM_RBUTTONDOWN()
ON_WM_SYSKEYDOWN()
ON_WM_SYSCHAR()
ON_COMMAND(ID_VIEW_HEADER, OnViewHeader)
ON_REGISTERED_MESSAGE( WM_FINDREPLACE, OnFind)
ON_COMMAND(ID_EDIT_FIND,OnEditFind)
ON_COMMAND(ID_EDIT_FINDAGAIN,OnEditFindAgain)
ON_UPDATE_COMMAND_UI(ID_EDIT_FINDAGAIN, OnUpdateEditFindAgain)
ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, OnUpdateEditFind)
ON_WM_CHAR()
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteitem)
	ON_COMMAND(ID_CV_WHATS_THIS, OnWhatsThis)
ON_MESSAGE(WM_SETFONT,OnSetFont)
	ON_WM_HELPINFO()
	ON_WM_MENUCHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CControlView drawing

void CControlView::OnDraw(CDC* pDC)
{
  UNUSED_ALWAYS(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CControlView diagnostics

#ifdef _DEBUG
void CControlView::AssertValid() const
{
  CTreeView::AssertValid();
}

void CControlView::Dump(CDumpContext& dc) const
{
  CTreeView::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CControlView message handlers

void CControlView::OnPaint() 
{
  CPaintDC dc(this); // device context for painting
  
  // First let the control do its default drawing.
  CWnd::DefWindowProc( WM_PAINT, (WPARAM)dc.m_hDC, 0 );
  
  CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
  if(pDoc->ItemCount()>0)
  {
    // Redraw the disabled labels
    CFont *pOldFont=dc.SelectObject(GetFont());
    dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
    
    for(HTREEITEM h=GetFirstVisibleItem();h;h=GetNextVisibleItem(h))
    {
      const UINT selflag = TVIS_DROPHILITED | TVIS_SELECTED;		
      if(!IsActive(h)&&!(GetItemState( h, selflag ) & selflag )){
        // Do not meddle with selected items or drop highlighted items
        CRect rect;		
        GetItemRect( h, &rect, TRUE );
        dc.TextOut(m_TreeXOffsetAdjustment+rect.left+2, rect.top+1, GetItemText(h));			
      }
    }
    dc.SelectObject(pOldFont);
    
    // Now draw grid lines
    
    CRect rcClient;
    GetClientRect(rcClient);
    CPen *pOldPen=dc.SelectObject(&m_GrayPen);
    int cy=0;
    for(h=GetFirstVisibleItem();h;h=GetNextVisibleItem(h))
    {
      dc.MoveTo(rcClient.left,cy);
      dc.LineTo(rcClient.right,cy);
      cy+=m_nItemHeight;
    }
    dc.MoveTo(rcClient.left,cy);
    dc.LineTo(rcClient.right,cy);
    dc.SelectObject(pOldPen);
    
  }
}

void CControlView::OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult) 
{
  //NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
  SetScrollRangePos();
  //SetHScrollRangePos();
  if(CConfigTool::GetCellView()){
    CConfigTool::GetCellView()->Invalidate();
//sdf1    CConfigTool::GetCellView()->UpdateWindow();
  } 
  
  *pResult = 0;
  UNUSED_ALWAYS(pNMHDR);
}

void CControlView::OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult) 
{
  //NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
  
  *pResult = 0;
  UNUSED_ALWAYS(pNMHDR);
}

void CControlView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
  CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
  if(pDoc->ItemCount()>0){
    // Do nothing if in process of destroying configitems
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    pDoc->UpdateAllViews (this, CConfigToolDoc::SelChanged, (CObject *)&TI(pNMTreeView->itemNew.hItem));
    CConfigItem &ti=TI(pNMTreeView->itemNew.hItem);
    
    CString strDesc(ti.Desc());
    if(strDesc.IsEmpty()){
      strDesc=_T("No context help is available for this item");
    }
    //pMain->m_wndHelp.SetWindowText(strDesc);
    //pMain->m_wndHelp.SetTitle(ti.ItemNameOrMacro());
    SetScrollPos();
    if(CConfigTool::GetCellView()){
      CConfigTool::GetCellView()->Sync();
    }
  }
  *pResult = 0;
}




void CControlView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  CConfigTool::GetCellView()->CancelCellEdit();
  {
    UINT nFlags;
    HTREEITEM h=HitTest(point,&nFlags);
    if(h && IsActive(h) && (nFlags&TVHT_ONITEMICON))
    {
      SelectItem(h);
      BumpItem (h, TI (h).HasBool () ? 0 : 1); // toggle boolean or increment data
    }
    else
    {
      CTreeView::OnLButtonDown(nFlags,point);
    }
  }
  
  // Relay to the splitter
  ClientToScreen(&point);
  GetParent()->ScreenToClient(&point);
  GetParent()->SendMessage(WM_LBUTTONDOWN,(WPARAM)nFlags,MAKELPARAM(point.x,point.y));	
}


void CControlView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{ 
  // Round to nearest item and express in units of items for the benefit of CTreeView
  nPos=(nPos+m_nItemHeight/2)/m_nItemHeight;
  DefWindowProc(WM_VSCROLL,MAKELONG(nSBCode, nPos), (LPARAM)pScrollBar->m_hWnd);
  SetScrollPos();
}

/////////////////////////////////////////////////////////////////////////////
// CControlView message handlers

void CControlView::OnInitialUpdate() 
{
  CTreeView::OnInitialUpdate();
#ifdef IE4
  SetItemHeight(20);
#endif
  /*
  #ifndef PLUGIN
  // Do this here, because the framework resets the title after calling OnOpenDocument()
  CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
  CString strTitle(pDoc->BuildTree());
  if(strTitle.IsEmpty())
  {
		strTitle+=_T("Repository ");
    strTitle+=pDoc->Repository();
    }
    pDoc->SetTitle(strTitle);
    #endif
  */
}

void CControlView::OnSize(UINT nType, int cx, int cy) 
{
  //m_nInOnSize++;
  CScrollBar *cv=GetScrollBarCtrl(SB_VERT);
  //int dx=cx-m_Size.cx;
  //int dy=cy-m_Size.cy;
  /*
  TRACE("WM_SIZE[%d] dx=%d dy=%d depth=%d ch=%08x cv=%08x",
		0,
    dx,dy,
    m_nInOnSize,
    ch,cv);
		*/
  int bIsHScrollBarDelta=(cv==NULL);
  //int bIsVScrollBarDelta=(ch==NULL);
  if(bIsHScrollBarDelta/*dx==-16*/){
    cx=m_Size.cx;
  }
  //	if(bIsVScrollBarDelta/*dy==-16*/){
  //		cy=m_Size.cy;
  //	}
  if(cx!=m_Size.cx || cy!=m_Size.cy){
    m_Size=CSize(cx,cy);
    DefWindowProc(WM_SIZE,(WPARAM)nType,MAKELPARAM(cx,cy));
  }
  KillScrollBars();
  SetScrollRangePos();
  //SetHScrollRangePos();
  //m_nInOnSize--;
}

BOOL CControlView::PreCreateWindow(CREATESTRUCT& cs) 
{
  cs.style=TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS |
    WS_CHILD | WS_VISIBLE /*| WS_BORDER*/ | WS_TABSTOP;
  cs.style&=(~WS_VSCROLL);
  m_dwDefaultStyle&=(~WS_VSCROLL);
  return CTreeView::PreCreateWindow(cs);
}

void CControlView::OnRButtonDown(UINT nFlags, CPoint point) 
{
  UNUSED_ALWAYS(nFlags);
  UINT Flags;
  HTREEITEM h=HitTest(point,&Flags);
  if(h){
    SelectItem(h);
  }
  // point is in client coords
  ClientToScreen(&point);
  ShowPopupMenu(h,point);
}

// Button functionality

void CControlView::Refresh (HTREEITEM h) // was Enable()
{
  if(h){
    AdjustItemImage(h);
    // Invalidate the labels of the affected items
    CRect rect;		
    GetItemRect(h, rect, TRUE );
    rect.left+=m_TreeXOffsetAdjustment;
    InvalidateRect(rect);
    // Do the same for the cell view
    CRect rcBuddyClient;
    CConfigTool::GetCellView()->GetClientRect(rcBuddyClient);
    rect.left=rcBuddyClient.left;
    rect.right=rcBuddyClient.right;
    CConfigTool::GetCellView()->InvalidateRect(rect);
  }
}

BOOL CControlView::IsActive(HTREEITEM h) const
{
  const CdlUserVisible vitem = TI (h).GetCdlItem ();
  return vitem ? vitem->is_active () : true; 
}

void CControlView::AdjustItemImage(HTREEITEM h)
{
  CConfigItem &ti=TI(h);
  CString str(ti.ItemNameOrMacro());
  if(ti.Modified()){
    str+=_TCHAR('*');
  }
  //#ifndef PLUGIN
  //	CConfigTool::GetConfigToolDoc()->SetTitle(_T("rt")); // Parameter is ignored
  //#endif
  SetItemText(h,str);
  
  int nImage=0;
  enum {FolderIcon=0, CheckBoxIcon=2, RadioIcon=6, IntegerIcon=10, StringIcon=12, EnumIcon=14, PackageIcon=16, DoubleIcon=20};
  if (ti.HasBool ())
  {
    if (ti.IsPackage ())
      nImage = PackageIcon;
    else
      nImage = ti.HasRadio () ? RadioIcon : CheckBoxIcon;
    
    if (ti.IsEnabled ())
      nImage += 2;
  }
  else
  {
    switch(ti.Type())
    {
    /*
    case CConfigItem::Boolean:
    nImage=ti.IsPackage()?PackageIcon:CheckBoxIcon;
    // These images have all four states
    if(ti.Value())
    {
				nImage+=2;
        }
        break;
        case CConfigItem::Radio:
        nImage=RadioIcon;
        // Has four states
        if(ti.Value())
        {
        nImage+=2;
        }
        break;
      */
    case CConfigItem::Double:
      nImage=DoubleIcon;
      break;
    case CConfigItem::Integer:
      nImage=IntegerIcon;
      break;
    case CConfigItem::Enum:
      nImage=EnumIcon;
      break;
    case CConfigItem::None:
      nImage = FolderIcon;
      break;
    case CConfigItem::String:
      // if a package use the enabled package icon else use the string icon
      nImage = ti.IsPackage () ? PackageIcon + 2 : StringIcon;
      break;
    default:
      ASSERT(FALSE);
      break;
    }
  }
  // All items have an enabled alternative
  if(!IsActive(h))
  {
    nImage+=1;
  }
  //	else if (ti.Type () == CConfigItem::Boolean)
  else if ((ti.Type () != CConfigItem::None) || ti.HasBool ())
  {
    // grey icon if the option is not modifiable or is inactive
    const CdlValuable valuable = ti.GetCdlValuable();
    if (valuable && ! (valuable->is_modifiable () && valuable->is_active ()))
      nImage++;
  }
  BOOL b=SetItemImage(h,nImage,nImage);
  ASSERT(b);
}

ItemIntegerType CControlView::Value(HTREEITEM h) const
{
  ItemIntegerType rc;
  CConfigItem &ti=TI(h);
  if(CConfigTool::GetCellView()->ActiveCell()==h){
    rc=CConfigTool::GetCellView()->GetCellValue();
  } else 
  {
    rc=ti.Value();
  }
  return rc;
}

void CControlView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
  CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
  switch(lHint)
  {
		case CConfigToolDoc::AllSaved:
      
      {
        for(int nItem=0;nItem<pDoc->ItemCount();nItem++)
        {
          CConfigItem *pItem=pDoc->Item(nItem);
          HTREEITEM h=pItem->HItem();
          if(h){
            SetItemText(h,pItem->ItemNameOrMacro());
            InvalidateItem(h);
          }
        }
      }
      break;
    case CConfigToolDoc::NameFormatChanged:
      {
        for(int nItem=0;nItem<pDoc->ItemCount();nItem++)
        {
          CConfigItem *pItem=pDoc->Item(nItem);
          CString strName(pItem->ItemNameOrMacro());
          if(pItem->Modified()){
            strName+=_TCHAR('*');
          }
          SetItemText(pItem->HItem(),strName);
        }
        
        Invalidate();
      }
      break;
    case CConfigToolDoc::IntFormatChanged:
      break;
    case CConfigToolDoc::Clear:		
      m_hExpandedForFind=NULL;
      m_hContext=NULL;
      DeleteAllItems();
      break;
    case CConfigToolDoc::ValueChanged:
      {
        CConfigItem &ti=*(CConfigItem *)pHint;
        HTREEITEM h=ti.HItem();
        AdjustItemImage(h);
        switch(ti.Type()){
        case CConfigItem::Integer:
          //		            case CConfigItem::Boolean:
          //		            case CConfigItem::Radio:
          {
            ItemIntegerType n=ti.Value();
            Refresh (h);
            for(HTREEITEM hc=GetChildItem(h);hc;hc=GetNextSiblingItem (hc)){
              Refresh (hc);
            }
            /*
            // In the case of radio buttons, do the same thing to my siblings:
            if(ti.Type()==CConfigItem::Radio && n==1){
            for(HTREEITEM hs=ti.FirstRadio()->HItem();hs;hs=GetNextSiblingItem(hs)){
            if(TI(hs).Type()==CConfigItem::Radio && hs!=h){
            AdjustItemImage(hs);
            // Enable or disable their children
            for(HTREEITEM hn=GetChildItem(hs);hn;hn=GetNextSiblingItem(hn)){
            Refresh (hn);
            }
            }
            }
            }
            */
            if(pDoc->m_bAutoExpand){
              if ( (0==(GetItemState(h,TVIS_EXPANDED) & TVIS_EXPANDED)) != (0==n) ){
                Expand(h,n?TVE_EXPAND:TVE_COLLAPSE);
                SetScrollRangePos();
              }
              
              //SetHScrollRangePos();
              if(CConfigTool::GetCellView()){
                CConfigTool::GetCellView()->Invalidate();
              }
            }
          }
          break;
        default:
          break;
        }
        CRect rect;
        GetItemRect(h,rect,FALSE);
        InvalidateRect(rect);
      }		        
      break;
    case CConfigToolDoc::ExternallyChanged:
      {
        for(int nItem=0;nItem<pDoc->ItemCount();nItem++){
          CConfigItem *pItem=pDoc->Item(nItem);
          AdjustItemImage(pItem->HItem());
        }
      }
      break;
    case 0:
      if(pDoc->ItemCount()>0)
      {
        CRect rect;
        GetItemRect(GetFirstVisibleItem(),rect,FALSE);
        m_nItemHeight=rect.Height();
        m_nWorkspace=m_nItemHeight;
        SetScrollRangePos();
        SetHScrollRangePos(); 
#ifdef _DEBUG
        int nCP=0;
        for(int nItem=0;nItem<pDoc->ItemCount();nItem++)
        {
          CConfigItem *pItem=pDoc->Item(nItem);
          ASSERT(pItem->HItem());
          switch(pItem->Type()){
          case CConfigItem::None:
            break;
          case CConfigItem::String:
          case CConfigItem::Enum:
          case CConfigItem::Integer:
          case CConfigItem::Double:
            //		case CConfigItem::Boolean:
            nCP++;
            break;
            //		case CConfigItem::Radio:
            //			nCP+=(pItem==pItem->FirstRadio());
            //			break;
          default:
            ASSERT(FALSE);
            break;
          }
        }
        TRACE(_T("### done creating - %d config items created: %d configuration points\n"),GetCount(),nCP);
#endif
        
        SetScrollRangePos();
        
      }
    default:
      break; // not for us, apparently
  }
  UNUSED_ALWAYS(pSender);
}

void CControlView::OnRestoreDefaults() 
{
  if(TI(m_hContext).HasModifiedChildren())
  {
    switch(CUtils::MessageBoxFT(MB_YESNOCANCEL,_T("Restore defaults for nested items?")))
    {
    case IDYES:
      RestoreDefault(m_hContext,TRUE);
      break;
    case IDNO:
      RestoreDefault(m_hContext,FALSE);
      break;
    case IDCANCEL:
      break;
    default:
      ASSERT(FALSE);
      break;
    }
  } else 
  {
    RestoreDefault(m_hContext,FALSE);
  }
  
  // current values may have changed so refresh the other views
  //CConfigTool::GetConfigToolDoc ()->UpdateFailingRuleCount ();
  if (TI (m_hContext).Type () != CConfigItem::None)
    CConfigTool::GetConfigToolDoc ()->UpdateAllViews (NULL, CConfigToolDoc::ValueChanged, (CObject *) GetItemData (m_hContext));
}

void CControlView::OnPopupProperties() 
{
  if(NULL==m_hContext){
    m_hContext=GetSelectedItem();
  }
  if(NULL!=m_hContext){
    CCTPropertiesDialog dlg(TI(m_hContext));
    dlg.DoModal();
    m_hContext=NULL;
  }
}


void CControlView::RestoreDefault(HTREEITEM h, BOOL bRecurse /* = FALSE */, BOOL bTopLevel /* = TRUE */)
{
  CConfigItem &ti = TI (h);
  const CdlValuable valuable = ti.GetCdlValuable();
  if (valuable && (CdlValueFlavor_None != valuable->get_flavor ())) // skip the root node and nodes without a value
    valuable->set_source (CdlValueSource_Default);
  SetItemText (h, ti.ItemNameOrMacro ()); // remove asterisk in control view
  
  if (bTopLevel && ti.HasRadio ()) // if user-specified item is a radio button
  {
    for (CConfigItem * pItem = ti.FirstRadio (); pItem; pItem = pItem->NextRadio ())
    {
      if (&ti != pItem)
      {
        const CdlValuable valuable = pItem->GetCdlValuable();
        ASSERT (valuable);
        valuable->set_source (CdlValueSource_Default); // restore default for each sibling radio button
        SetItemText (pItem->HItem (), pItem->ItemNameOrMacro ()); // remove asterisk in control view
      }
      
    }
  }
  
  if (bRecurse)
  {
    for (h = GetChildItem (h); h; h = GetNextSiblingItem (h))
    {
      RestoreDefault (h, TRUE, FALSE);
    }
  }
}

BOOL CControlView::IsChanged(HTREEITEM h, BOOL bRecurse)
{
  BOOL rc = TI (h).Modified ();
  //	CConfigItem &ti=TI(h);
  //	BOOL rc=(0!=ti.StringDefaultValue().Compare(ti.StringValue()));		
  if(!rc && bRecurse)
  {
    for(h=GetChildItem(h);h;h=GetNextSiblingItem(h))
    {
      if(IsChanged(h,TRUE))
      {
        rc=TRUE;
        break;
      }
    }
  }
  return rc;
}


void CControlView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if(nChar==VK_APPS){
    HTREEITEM h=GetSelectedItem();
    ShowPopupMenu(h);
    return;
  }
  CTreeView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CControlView::InvalidateItem(HTREEITEM h)
{
  if(CConfigTool::GetCellView()->ActiveCell()==h)
  {
    //m_pwndCell->Invalidate();
  } 
  else 
  {
    CRect rect;
    GetItemRect(h,rect,FALSE);
    InvalidateRect(rect);
  }
}

/*
void CControlView::OnViewSwitches() 
{
m_bSwitches ^= 1;	
CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
for(int nItem=0;nItem<pDoc->ItemCount();nItem++)
{
CConfigItem *pItem=pDoc->Item(nItem);
HTREEITEM h=pItem->HItem();
if(pItem->m_Icon==CConfigItem::IconCheckbox)
{
pItem->m_Icon=CConfigItem::IconSwitch;
} else if(pItem->m_Icon==CConfigItem::IconSwitch)
{
pItem->m_Icon=CConfigItem::IconCheckbox;
}
AdjustItemImage(h);
}

  Invalidate();
  
    }
    
      void CControlView::OnUpdateViewSwitches(CCmdUI* pCmdUI) 
      {
      pCmdUI->SetCheck(m_bSwitches);
      }
      
        void CControlView::OnViewCheckboxes() 
        {
        OnViewSwitches();
        }
        void CControlView::OnUpdateViewCheckboxes(CCmdUI* pCmdUI) 
        {
        pCmdUI->SetCheck(!m_bSwitches);
        }
*/
void CControlView::OnViewUrl() 
{
  TI(m_hContext).ViewURL();
}

bool CControlView::BumpItem(HTREEITEM h,int nInc)
{
  bool rc=false;
  // Take an action for clicking on the icon
  CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
  CConfigItem &ti=TI(h);
  
  // do not modify the option value if it is inactive or not modifiable
  const CdlValuable valuable = ti.GetCdlValuable();
  if (!valuable || (valuable->is_modifiable () && valuable->is_active ())) {
    if (0 == nInc) // if a toggle request
    {
      if (ti.HasBool () && ! (ti.HasRadio () && ti.IsEnabled ())) { // only enable (not disable) a radio button
        rc=pDoc->SetEnabled (ti, ! ti.IsEnabled ()); // toggle enabled/disabled state
      }
    } else if (ti.IsEnabled ()) { // the item is enabled...
      switch(ti.Type())
      {
      case CConfigItem::None:
      case CConfigItem::String:
      case CConfigItem::Double:
        break;
      case CConfigItem::Enum:
        {
          CStringArray arEnum;
          ti.EvalEnumStrings (arEnum); // calculate legal values just in time
          if (0==arEnum.GetSize()) // if no legal values...
            break;           // ...do nothing
          int nIndex = -1;
          const CString strCurrent = ti.StringValue ();
          for (int nEnum = 0; (nEnum < arEnum.GetSize()) && (nIndex == -1); nEnum++)
            if (0 == arEnum[nEnum].Compare (strCurrent))
						        nIndex = nEnum; // the index of the current value
            
				        if (nIndex != -1) // if the current value is still legal
                  nIndex += (nInc < 0 ? -1 : 1); // increment/decrement the index
                else
                  nIndex = 0; // otherwise select the first enum
                
                if (nIndex < 0) // if the new index is negative
                  nIndex = arEnum.GetSize()-1; // make it positive
                
                rc=pDoc->SetValue (ti, arEnum[nIndex % arEnum.GetSize()]);
        }
        break;
      case CConfigItem::Integer:
        {
          ItemIntegerType nOldValue=Value(h);
          if(nInc==1 && nOldValue==ItemIntegerType (-1)){
            nOldValue=0;
          } else if(nInc==-1 && nOldValue==0){
            nOldValue=ItemIntegerType (-1);
          } else {
            nOldValue+=nInc;
          }
          rc=pDoc->SetValue(ti,nOldValue);
          break;
        }
        
        break;
        /*
        case CConfigItem::Boolean:
        
          {
          ItemIntegerType nOldValue=Value(h);
          pDoc->SetValue(ti,nOldValue^1);
          }
          break;
		        case CConfigItem::Radio:
            
              if(0==Value(h)){
              pDoc->SetValue(ti, (ItemIntegerType) 1);
              }
              break;
        */
      default:
        ASSERT(FALSE);
        break;
      }
    }
  }
  return rc;
}

int CControlView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  if (CTreeView::OnCreate(lpCreateStruct) == -1)
    return -1;
  
  BOOL b=m_il.Create(IDB_BITMAP2,16,1,RGB(0,128,128));
  
  // This code is necessary because of what appears to be an inconsistency between
  // different versions of the common control dll.  In some the text rectangles seem
  // not to take into account the offset introduced by the item image of the root item.
  // The offset is used in the OnPaint handler to make sure the greyed text is in the
  // right place.
  
  // Begin hack
  {
    HTREEITEM h=InsertItem(_T("Foo"));
    CRect rect1,rect2;
    GetItemRect(h,rect1,TRUE);
    
    SetImageList(&m_il,TVSIL_NORMAL);
    GetItemRect(h,rect2,TRUE);
    m_TreeXOffsetAdjustment=rect1.left==0?(rect2.left-rect1.left):0;
    DeleteItem(h);
  }
  // End hack
  
  ASSERT(b);
  
  CRect rcClient;
  GetClientRect(rcClient);	
  CSplitterWnd *pSplitter=(CSplitterWnd*)GetParent();
  pSplitter->SetColumnInfo(1,rcClient.Width()/4,0);
  
  return 0;
}

void CControlView::KillScrollBars()
{
  CSplitterWnd *pSplit=(CSplitterWnd *)GetParent();
  int min,curX,curY;
  const int col=0;
  pSplit->GetColumnInfo(col,curX,min);
  if(-1==curX){
    return;
  }
  pSplit->GetRowInfo   (0,curY,min);
  if(-1==curY){
    return;
  }
  CRect rcClient;
  GetClientRect(rcClient);
  /*
  TRACE("splitter[%d]=(%d,%d) view=(%d,%d)",
		col,
    curX,curY,
    rcClient.Width(),rcClient.Height());
		*/
  if(curX>rcClient.Width()&&curY>rcClient.Height()){
    //sdf ShowScrollBar(SB_BOTH,FALSE);
    ShowScrollBar(SB_VERT,FALSE);
  } else {
    if(curX>rcClient.Width()){
      //TRACE(" -VERT\n");
      ShowScrollBar(SB_VERT,FALSE);
    }
    if(curY>rcClient.Height()){
      //TRACE(" -HORZ\n");
      //sdf ShowScrollBar(SB_HORZ,FALSE);
    }
  }
  //TRACE("\n");
}

void CControlView::SetScrollPos()
{
  if(m_bHasVScroll==1){
    CRect rect;
    GetTreeCtrl().GetItemRect(GetTreeCtrl().GetRootItem(),rect,FALSE);
    CScrollBar *cv=(CScrollBar *)(GetParent()->GetDlgItem(AFX_IDW_VSCROLL_FIRST));
    //BOOL cv=TRUE;
    if(cv && m_bHasVScroll){
      //TRACE("SetScrollPos %d\n",-rect.top);
      cv->SetScrollPos(-rect.top);
      //SetScrollInfo(SB_VERT,&si,SIF_POS);
    }
  }
}

void CControlView::SetScrollRangePos()
{
  RecalcWorkSpaceSize();
  if(m_nWorkspace>0){
    CScrollBar *cv=(CScrollBar *)(GetParent()->GetDlgItem(AFX_IDW_VSCROLL_FIRST));
    //BOOL cv=TRUE;
    
    if(cv){
      CRect rcClient;
      GetClientRect(rcClient);
      SCROLLINFO si;
      si.cbSize=sizeof SCROLLINFO;
      memset(&si,0,sizeof SCROLLINFO);
      si.nPage=rcClient.Height();
      si.fMask=SIF_PAGE|SIF_RANGE|SIF_POS;
      si.nMin=0;
      CRect rect;
      GetTreeCtrl().GetItemRect(GetTreeCtrl().GetRootItem(),rect,FALSE);
      si.nPos=-rect.top;
      BOOL bHasVScroll=(si.nPage<(unsigned)m_nWorkspace);
      if(bHasVScroll){
        si.nMax=m_nWorkspace/*-si.nPage*/;
        cv->SetScrollInfo(&si,SIF_PAGE|SIF_RANGE|SIF_POS);
        //TRACE("SetScrollInfo range=(%d,%d) pos=%d page=%d\n",si.nMin,si.nMax,si.nPos,si.nPage);
      }
      if(bHasVScroll!=m_bHasVScroll){
        CSplitterWnd *pSplitter=(CSplitterWnd *)GetParent();
        DWORD style=(pSplitter->GetScrollStyle()&(~WS_VSCROLL))
          |(bHasVScroll*WS_VSCROLL);
        pSplitter->SetScrollStyle(style);
        pSplitter->RecalcLayout();
        m_bHasVScroll=bHasVScroll;
      }
    }
  }
}

void CControlView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  CTreeView::OnHScroll(nSBCode, nPos, pScrollBar);
  SetHScrollPos();
}

void CControlView::SetHScrollRangePos()
{
  return;//sdf
}

void CControlView::SetHScrollPos()
{
  return;//sdf
  CRect rcClient;
  GetClientRect(rcClient);
  SCROLLINFO si;
  si.cbSize=sizeof SCROLLINFO;
  memset(&si,0,sizeof SCROLLINFO);
  si.fMask=SIF_POS;
  CRect rect;
  GetTreeCtrl().GetItemRect(GetTreeCtrl().GetRootItem(),rect,FALSE);
  si.nPos=-rect.left;
  const int nCol=0;
  CScrollBar *cv=(CScrollBar *)(GetParent()->GetDlgItem(AFX_IDW_HSCROLL_FIRST+nCol));
  //BOOL cv=TRUE;
  if(cv && m_bHasHScroll){
    cv->SetScrollInfo(&si,SIF_POS);
    //SetScrollInfo(SB_HORZ,&si,SIF_POS);
  }
  
}

void CControlView::OnMouseMove(UINT nFlags, CPoint point) 
{
  CTreeView::OnMouseMove(nFlags, point) ;
  
  // Relay the mouse event to the splitter
  ClientToScreen(&point);
  GetParent()->ScreenToClient(&point);
  GetParent()->SendMessage(WM_MOUSEMOVE,(WPARAM)nFlags,MAKELPARAM(point.x,point.y));	
}

BOOL CControlView::OnMouseWheel(UINT, short zDelta, CPoint) 
{
  UINT nScrollCode=((zDelta<0)?SB_LINEDOWN:SB_LINEUP);
  LPARAM lParam=(LPARAM)GetScrollBarCtrl(SB_VERT)->GetSafeHwnd();
  if(lParam){
    for(int i=0;i<abs(zDelta)/WHEEL_DELTA;i++){
      SendMessage(WM_VSCROLL,MAKEWPARAM(nScrollCode,0),lParam);
      CConfigTool::GetCellView()->Sync();
    }
  }
  return TRUE;
}

void CControlView::RecalcWorkSpaceSize()
{
  //TRACE("Control view recalcworkspace\n");
  CRect rcFirst,rcLast;
  CConfigToolDoc* pDoc = CConfigTool::GetConfigToolDoc();
  if(pDoc->ItemCount()>0){
    HTREEITEM h=pDoc->FirstItem()->HItem();
    if(h){
      GetItemRect(h,rcFirst,FALSE);
      if(pDoc->ItemCount()>0 && pDoc->Item(pDoc->ItemCount()-1)->HItem()){
        for(HTREEITEM h=pDoc->Item(pDoc->ItemCount()-1)->HItem();h;h=GetPrevVisibleItem(h)){
          if(GetItemRect(h,rcLast,FALSE)){
            m_nWorkspace=rcLast.bottom-rcFirst.top;
            break;
          }
        }
      }
    }
  }
}

void CControlView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  switch(nChar){
		case VK_F10:
      if(::GetKeyState(VK_SHIFT)&0x8000){
        // Shift key down
        ShowPopupMenu(GetSelectedItem());
      }
      break;
    case VK_RETURN:
      if(nFlags&(1<<13)){
        // Alt key down
        m_hContext=GetSelectedItem();
        OnPopupProperties();
      }
      break;
  }
  CTreeView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CControlView::ShowPopupMenu(HTREEITEM h,CPoint point/*=CPoint(-1,-1)*/)
{
  // Display a context menu
  if(NULL!=h){
    Menu menu;
    menu.LoadMenu(IDR_CONTROLVIEW_POPUP);
    Menu *pPopup=(Menu *)menu.GetSubMenu(0);

    if(point.x<0){
      CRect rcItem;
      GetItemRect(h,rcItem,TRUE);
      point=rcItem.CenterPoint();
      ClientToScreen(&point);
    }
    m_hContext=h;
    CConfigItem &ti=TI(m_hContext);
    CConfigItem::TreeItemType type=ti.Type();

    #ifndef PLUGIN
    menu.LoadToolbar(IDR_MISCBAR);
    #endif
    if(!IsChanged(m_hContext,true)){
      pPopup->EnableMenuItem(ID_RESTORE_DEFAULTS,MF_BYCOMMAND|MF_GRAYED);
    }

    if((CConfigItem::None!=type) || ti.HasBool ()){
    		const CString strURL(ti.GetURL());
        if(strURL.IsEmpty()){
          pPopup->EnableMenuItem(ID_VIEW_URL,MF_BYCOMMAND|MF_GRAYED);
        }
    }

    const CFileName strFile(TI(h).FileName());
    if(strFile.IsEmpty() || CConfigTool::GetConfigToolDoc()->BuildTree().IsEmpty()){
       pPopup->EnableMenuItem(ID_VIEW_HEADER,MF_BYCOMMAND|MF_GRAYED);
    }

    if(!TI(h).IsPackage()){
      int n=pPopup->GetMenuItemCount();
      pPopup->RemoveMenu(n-1,MF_BYPOSITION);
      pPopup->RemoveMenu(n-2,MF_BYPOSITION);
    }
    pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x,point.y,this);
  }
}

void CControlView::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  // TODO: Add your message handler code here and/or call default
  
  CTreeView::OnSysChar(nChar, nRepCnt, nFlags);
}

CConfigToolDoc * CControlView::GetDocument()
{
  return CConfigTool::GetConfigToolDoc();
}

void CControlView::OnViewHeader() 
{
  TI(m_hContext).ViewHeader();
}

void CControlView::OnEditFind() 
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  if(!CConfigTool::GetMain()->m_bFindInProgress){
    (new CFindDialog())->Create(pDoc->m_strFind,pDoc->m_nFindFlags&(FR_WHOLEWORD|FR_MATCHCASE|FR_DOWN),pDoc->m_nFindWhere, this);
    CConfigTool::GetMain()->m_bFindInProgress=true;
  }
}

void CControlView::OnEditFindAgain() 
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  DoFind(pDoc->m_strFind,pDoc->m_nFindFlags,pDoc->m_nFindWhere);
}

LONG CControlView::OnFind(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  CFindDialog *pDlg=(CFindDialog *)CFindDialog::GetNotifier(lParam);
  FINDREPLACE &fr=pDlg->m_fr;
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  if(0==(fr.Flags&FR_DIALOGTERM)){
    pDoc->m_strFind=fr.lpstrFindWhat;
    pDoc->m_nFindFlags=fr.Flags;
    pDoc->m_nFindWhere=pDlg->m_nFindPos;
    
    CConfigItem *pItem=DoFind(pDoc->m_strFind,pDoc->m_nFindFlags,pDoc->m_nFindWhere);
    if(pItem){
		    // Is the find window on top of the item?
		    CRect rect1,rect2;
        GetItemRect(pItem->HItem(),rect1,TRUE);
        ClientToScreen(rect1);
        pDlg->GetWindowRect(rect2); // screen coords
        rect1=rect1 & rect2;
        if(rect1.Height()|rect1.Width()){
          int nWidth=rect2.Width();
          rect2.left=rect1.right;
          rect2.right=rect2.left+nWidth;
          pDlg->MoveWindow(rect2);
        }
    }
  } else {
    CConfigTool::GetMain()->m_bFindInProgress=false;
  }
  
  return 0;
}

CConfigItem * CControlView::DoFind(LPCTSTR pszWhat,DWORD dwFlags,WhereType where)
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  int nCount=pDoc->ItemCount();
  static LPCTSTR arWhereImage[]={_T("name"),_T("macro"),_T("description string"),_T("current value"),_T("default value")};
  
  CString strFind(pszWhat);
  if(!(dwFlags&FR_MATCHCASE)){
    strFind.MakeLower();
  }
  
  HTREEITEM h=GetSelectedItem();
  int nItem;  
  if(!h){
    nItem=0;
  } else {
    for(nItem=nCount-1;nItem>=0;--nItem) {
      if(h==pDoc->Item(nItem)->HItem()){
        break;
      }
    }
    ASSERT(nItem>=0);
  }

  CConfigItem *pItem=NULL;
  for(int i=0;i<nCount;i++){
    if(dwFlags&FR_DOWN){
      nItem=(nItem+1)%nCount;
    } else {
      nItem=(nCount+nItem-1)%nCount;
    }
    pItem=pDoc->Item(nItem);
    
    CString strName(pItem->StringValue(where));
    if(0==(dwFlags&FR_MATCHCASE)){
      strName.MakeLower();
    }
    int nIndex=strName.Find(strFind);
    if(-1!=nIndex){
      if(dwFlags&FR_WHOLEWORD){
        // Enforce whole-word semantics: to left and right
        if(nIndex>0 && IsWordChar(strName[nIndex-1])){
          continue;
        }
        nIndex+=strFind.GetLength();
        if(nIndex<strName.GetLength() && IsWordChar(strName[nIndex])){
          continue;
        }
      }		
      break;
    }
  }
  
  if(i<nCount){
    if(m_hExpandedForFind){
      Expand(m_hExpandedForFind,TVE_COLLAPSE);
    }
    HTREEITEM h=pItem->HItem();
    // Is h visible?
    for(HTREEITEM hv=GetFirstVisibleItem();hv;hv=GetNextVisibleItem(hv)){
      if(hv==h){
        break;
      }
    }
    if(NULL==hv){
      // we want to record the highest unexpanded item
      for(HTREEITEM hv=GetParentItem(h);hv;hv=GetParentItem(hv)){
        const UINT selflag = TVIS_EXPANDED;		
        if(0==(GetItemState( hv, selflag ) & selflag )){
          m_hExpandedForFind=hv;
        }
      }
    }
    EnsureVisible(h);
    SelectItem(h);
    
  } else {
    CUtils::MessageBoxF(_T("Cannot find the %s '%s'"),arWhereImage[where],pszWhat);
  }
  return pItem;
}

bool CControlView::IsWordChar(TCHAR c)
{
  return _istalnum(c) || _TCHAR('_')==c;
}

void CControlView::OnUpdateEditFindAgain(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!CConfigTool::GetMain()->m_bFindInProgress && !CConfigTool::GetConfigToolDoc()->m_strFind.IsEmpty() && CWnd::GetFocus() && m_hWnd==CWnd::GetFocus()->m_hWnd);	
}

bool CControlView::SelectItem(const CConfigItem *pItem)
{
#ifdef PLUGIN
  CConfigTool::GetMain()->ShoweCosBar();
#else
#endif
  return SelectItem(pItem->HItem());
}

void CControlView::OnUpdateEditFind(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!CConfigTool::GetMain()->m_bFindInProgress);	
}

BOOL CControlView::PreTranslateMessage(MSG* pMsg) 
{
  // We handle WM_KEYDOWN here to avoid the problem of the keyboard buffer not being emptied
  if(WM_KEYDOWN==pMsg->message){
    HTREEITEM h=GetSelectedItem();
    if(h){
		    switch(pMsg->wParam){
        case VK_SPACE:
          if(BumpItem(h,0)){ // toggle boolean
				        return true;
          }
          break;
        case VK_TAB:
          {
            bool bShift=(::GetKeyState(VK_SHIFT)&0x8000);// Shift key down
            if(bShift){
              h=GetPrevVisibleItem(h);
            }
            if(h){
              SelectItem(h);
              if(!CConfigTool::GetCellView()->InCell(h) && !bShift){
                h=GetNextVisibleItem(h);
                if(h){
                  SelectItem(h);
                }
              }
            }
            if(NULL==h){
              MessageBeep (0xFFFFFFFF);
            }
            // Always handle this message to keep focus within the tree control
            return true;
          }
        }
    }
  }
  return CTreeView::PreTranslateMessage(pMsg);
}

void CControlView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  HTREEITEM h=GetSelectedItem();
  if(h){
    switch(nChar){
    case _TCHAR('>'):
      if(!BumpItem(h,+1)){
        MessageBeep(0xffff);
      }
      return;
    case _TCHAR('<'):
      if(!BumpItem(h,-1)){
        MessageBeep(0xffff);
      }
      return;
    }
  }
  
  CTreeView::OnChar(nChar, nRepCnt, nFlags);
}

LRESULT CControlView::OnSetFont(WPARAM, LPARAM)
{
  LRESULT rc=Default();
  CRect rect;
  GetItemRect(GetFirstVisibleItem(),rect,FALSE);
  m_nItemHeight=rect.Height();
  RecalcWorkSpaceSize();
  return rc;
}

void CControlView::OnUnload()
{
  if(IDYES==CUtils::MessageBoxFT(MB_YESNO|MB_DEFBUTTON2,_T("Are you sure you wish to unload this package?"))){
    TI(m_hContext).Unload();
    CConfigTool::GetConfigToolDoc()->RegenerateData();
  }
}

void CControlView::OnDeleteitem(NMHDR*, LRESULT* pResult) 
{
  if(CConfigTool::GetCellView()){
    CConfigTool::GetCellView()->Invalidate();
  } 
	*pResult = 0;
}

void CControlView::OnWhatsThis() 
{
  DWORD dwPos=GetMessagePos();
  
  HH_POPUP hhp;
  hhp.cbStruct=sizeof(hhp);
  hhp.idString=0;
  hhp.hinst=AfxGetInstanceHandle();
  hhp.pszText=TI(m_hContext).Desc();
  hhp.pt.x=GET_X_LPARAM(dwPos);
  hhp.pt.y=GET_Y_LPARAM(dwPos);
  hhp.clrForeground=(COLORREF)-1; //default 
  hhp.clrBackground=GetSysColor(COLOR_INFOBK); 
  hhp.rcMargins=CRect(-1,-1,-1,-1);
  hhp.pszFont=NULL;

  HtmlHelp( 
    m_hWnd,
    NULL,
    HH_DISPLAY_TEXT_POPUP, //HH_TP_HELP_CONTEXTMENU, 
    (DWORD)&hhp
    ); 
	
}

void CControlView::Refresh(LPCTSTR pszMacroName)
{
  const CConfigItem * pItem=CConfigTool::GetConfigToolDoc()->Find(pszMacroName);
  if(pItem){ // will be NULL if item has been removed
    Refresh(pItem->HItem());
  }
}

BOOL CControlView::OnHelpInfo(HELPINFO*) 
{
  return CConfigTool::GetConfigToolDoc()->ShowURL(CUtils::LoadString(IDS_CONTROL_VIEW_HELP));	
}

LRESULT CControlView::OnMenuChar(UINT, UINT, CMenu*)
{
  const MSG *pMsg=GetCurrentMessage();
  // punt to the mainframe to deal with shortcuts in popups
  return AfxGetMainWnd()->SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
}
