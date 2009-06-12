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
// AddRemoveDialog.cpp : implementation file
//

#include "stdafx.h"

#include "AddRemoveDialog.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddRemoveDialog dialog

void CAddRemoveDialog::AfxDataInit()
{
	//{{AFX_DATA_INIT(CAddRemoveDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAddRemoveDialog::CAddRemoveDialog(CWnd* pParent /*=NULL*/)
	: CeCosDialog(IDD, pParent),
    m_bSort(false),
    m_arbSel(NULL),
    m_bHaveDescs(false),
    m_nHorizontalResizingLimit(0)
{
	AfxDataInit ();
}

CAddRemoveDialog::CAddRemoveDialog(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CeCosDialog(nIDTemplate, pParent),
    m_bSort(false),
    m_arbSel(NULL),
    m_nHorizontalResizingLimit(0)
{
	AfxDataInit ();
}

void CAddRemoveDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddRemoveDialog)
	DDX_Control(pDX, IDC_PACKAGE_DESC, m_wndDesc);
	DDX_Control(pDX, IDC_ADDREMOVE_LIST2, m_List2);
	DDX_Control(pDX, IDC_ADDREMOVE_LIST1, m_List1);
	DDX_Control(pDX, IDC_ADDREMOVE_ADD,   m_Add);
	DDX_Control(pDX, IDC_ADDREMOVE_REMOVE,m_Remove);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddRemoveDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CAddRemoveDialog)
	ON_BN_CLICKED(IDC_ADDREMOVE_ADD, OnAdd)
	ON_BN_CLICKED(IDC_ADDREMOVE_REMOVE, OnRemove)
	ON_LBN_DBLCLK(IDC_ADDREMOVE_LIST1, OnDblclkList1)
	ON_LBN_DBLCLK(IDC_ADDREMOVE_LIST2, OnDblclkList2)
	ON_LBN_SELCHANGE(IDC_ADDREMOVE_LIST1, OnSelchangeList1)
	ON_LBN_SELCHANGE(IDC_ADDREMOVE_LIST2, OnSelchangeList2)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddRemoveDialog message handlers

void CAddRemoveDialog::OnAdd() 
{
    Add(m_List1,m_List2);
    m_Add.EnableWindow(FALSE);
    m_Remove.EnableWindow(TRUE);
}

void CAddRemoveDialog::OnRemove() 
{
    Add(m_List2,m_List1);
    m_Add.EnableWindow(TRUE);
    m_Remove.EnableWindow(FALSE);
}

void CAddRemoveDialog::Insert(LPCTSTR pszItem, bool bAdded, LPCTSTR pszDesc/*=NULL*/)
{
    m_bHaveDescs|=(NULL!=pszDesc);
    m_arnItems.  Add(bAdded);
    m_arstrItems.Add(pszItem);
    m_arstrDescs.Add(pszDesc?pszDesc:_T(""));
}

BOOL CAddRemoveDialog::OnInitDialog() 
{
	CeCosDialog::OnInitDialog();

    m_arbSel=new int[GetCount()];
    
    SetWindowText(m_strCaption);
    SetSortMode(m_List1);
    SetSortMode(m_List2);

	int cxExtent=0;
    CDC *pDC=m_List1.GetDC();
	CFont *pOldFont=pDC->SelectObject(m_List1.GetFont());
    for(int i=GetCount()-1;i>=0;--i) {
        const CString &str=m_arstrItems[i];
		cxExtent=max(cxExtent,pDC->GetTextExtent(str).cx);
        CListBox &lb=m_arnItems[i]?m_List2:m_List1;
        lb.SetItemData(lb.AddString(str),(DWORD)i);
    }
    pDC->SelectObject(pOldFont);

    SetDlgItemText(IDC_ADDREMOVE_TEXT1,m_strMessage1);
    SetDlgItemText(IDC_ADDREMOVE_TEXT2,m_strMessage2);

	CRect rcLb1;
	m_List1.GetWindowRect(rcLb1);
    CRect rcDialog;
    GetWindowRect(rcDialog);

	int nExpand=cxExtent-(rcLb1.Width()-GetSystemMetrics(SM_CXVSCROLL)-2*GetSystemMetrics(SM_CXBORDER)-5);
    nExpand=min(nExpand,m_nHorizontalResizingLimit);
	if(nExpand>0){
		MoveChild(IDOK,2*nExpand,Move);
		MoveChild(IDCANCEL,2*nExpand,Move);
		MoveChild(IDC_ADDREMOVE_ADD,nExpand,Move);
		MoveChild(IDC_ADDREMOVE_REMOVE,nExpand,Move);
		MoveChild(IDC_ADDREMOVE_LIST1,nExpand,Stretch);
		MoveChild(IDC_ADDREMOVE_TEXT2,nExpand,Move);
		MoveChild(IDC_ADDREMOVE_LIST2,nExpand,Move);
		MoveChild(IDC_ADDREMOVE_LIST2,nExpand,Stretch);
		MoveChild(IDC_PACKAGE_DESC,2*nExpand,Stretch);
	    rcDialog.right+=2*nExpand; // MoveWindow call is below
	}

    // enable horizontal scrolling if necessary, assuming the
	// listboxes have identical widths and accommodating a
	// 2 pixel border at each side of each listbox
	m_List1.SetHorizontalExtent (cxExtent + 4);
	m_List2.SetHorizontalExtent (cxExtent + 4);

    m_Add.EnableWindow(m_List1.GetCount()>0);
    m_Remove.EnableWindow(m_List2.GetCount()>0);
	
    if(!m_bHaveDescs){
        // Remove the description pane if not used:
        CRect rcDesc;
        m_wndDesc.GetWindowRect(rcDesc);
        m_wndDesc.ShowWindow(SW_HIDE);
	    int delta=(rcDesc.Height()+(rcDesc.top-rcLb1.bottom));
        MoveChild(IDOK,delta,Up);
        MoveChild(IDCANCEL,delta,Up);
        rcDialog.bottom-=delta;
    }

    // Finally, adjust dialog size according to calculations:
    MoveWindow(rcDialog);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddRemoveDialog::OnDblclkList1() 
{
    OnDblclk(m_List1,m_List2);
    m_Add.EnableWindow(FALSE);
    m_Remove.EnableWindow(TRUE);
}

void CAddRemoveDialog::OnDblclkList2() 
{
    OnDblclk(m_List2,m_List1);
    m_Add.EnableWindow(TRUE);
    m_Remove.EnableWindow(FALSE);
}

int CAddRemoveDialog::Add(CListBox &lb1,CListBox &lb2)
{
    int nSelCount=lb1.GetSelItems(lb1.GetCount(),m_arbSel);
    for(int i=nSelCount-1;i>=0;--i){
        Xfer(lb1,lb2,m_arbSel[i]);
    }
    ClearSel(lb1);
    lb2.SetFocus();
    return nSelCount;
}

void CAddRemoveDialog::OnDblclk(CListBox &lb1,CListBox &lb2)
{
    BOOL bOutside;
    DWORD dwPos=GetMessagePos();
    POINTS pts=MAKEPOINTS(dwPos);
    CRect rc(pts.x,pts.y,0,0);
    lb1.ScreenToClient(rc);
    CPoint pt(rc.left,rc.top);
    int nItem=lb1.ItemFromPoint(pt,bOutside);
    if(!bOutside){
        Xfer(lb1,lb2,nItem);
    }
    ClearSel(lb1);
    lb2.SetFocus();
}

void CAddRemoveDialog::ClearSel(CListBox &lb)
{
    for(int i=lb.GetCount()-1;i>=0;--i){
        lb.SetSel(i,FALSE);
    }
}

void CAddRemoveDialog::OnSelchangeList1()
{
    ClearSel(m_List2);
    m_Add.EnableWindow(m_List1.GetSelCount()>0);
    UpdateDescription(m_List1);
    m_Remove.EnableWindow(FALSE);
}

void CAddRemoveDialog::OnSelchangeList2()
{
    ClearSel(m_List1);
    m_Remove.EnableWindow(m_List2.GetSelCount()>0);
    UpdateDescription(m_List2);
    m_Add.EnableWindow(FALSE);
}

void CAddRemoveDialog::Xfer(CListBox &lb1, CListBox &lb2, int nItem)
{
    CString strItem;
    lb1.GetText(nItem,strItem);
    int nNewItem=lb2.AddString(strItem);
    lb2.SetSel(nNewItem,TRUE);
    DWORD nIndex=lb1.GetItemData(nItem);
    lb2.SetItemData(nNewItem,nIndex);
    m_arnItems[(int)nIndex]^=1;
    lb1.DeleteString(nItem);
}

bool CAddRemoveDialog::IsAdded(LPCTSTR pszItem) const
{
    for(int i=GetCount()-1;i>=0;--i){
        if(m_arstrItems[i]==pszItem){
            return 0!=m_arnItems[i];
        }
    }
    ASSERT(false);
    return false;
}


void CAddRemoveDialog::SetCaption(LPCTSTR pszCaption)
{
    m_strCaption=pszCaption;
}

void CAddRemoveDialog::SetMessage(LPCTSTR pszText1,LPCTSTR pszText2)
{
    m_strMessage1=pszText1;
    m_strMessage2=pszText2;
}

void CAddRemoveDialog::OnDestroy() 
{
	CeCosDialog::OnDestroy();
    deleteZA(m_arbSel);
}

// Add the "sort" property to the listboxes.  Unfortunately this entails
// recreating them.
void CAddRemoveDialog::SetSortMode(CListBox &lb)
{
    DWORD dwStyle=lb.GetStyle();
    if(m_bSort!=(0!=(dwStyle&LBS_SORT))){
        if(m_bSort){
            dwStyle|=LBS_SORT;
        } else {
            dwStyle&=~LBS_SORT;
        }
        dwStyle|=WS_HSCROLL; // For some reason this doesn't come through GetStyle()
        CRect rect;
        lb.GetWindowRect(rect);
        ScreenToClient(rect);
        DWORD dwStyleEx=lb.GetExStyle();
        CFont *pFont=lb.GetFont();
        DWORD id=lb.GetDlgCtrlID();
        lb.DestroyWindow();
        lb.Detach();
        // CreateEx not Create to get ourselves a 3D border
        lb.CreateEx(dwStyleEx,_T("LISTBOX"),NULL,dwStyle,rect,this,id);
        lb.SetFont(pFont);
    }
}

// Implement ctrl/A on listboxes
BOOL CAddRemoveDialog::PreTranslateMessage(MSG* pMsg) 
{
    if(WM_CHAR==pMsg->message && 1==pMsg->wParam){
        if(pMsg->hwnd==m_List1.m_hWnd){
            m_List1.SelItemRange(TRUE,0,m_List1.GetCount()-1);
            ClearSel(m_List2);
            m_Add.EnableWindow(m_List1.GetCount()>0);
            m_Remove.EnableWindow(FALSE);
            return TRUE;
        } else if(pMsg->hwnd==m_List2.m_hWnd){
            m_List2.SelItemRange(TRUE,0,m_List2.GetCount()-1);
            ClearSel(m_List1);
            m_Remove.EnableWindow(m_List2.GetCount()>0);
            m_Add.EnableWindow(FALSE);
            return TRUE;
        }
    }
	
	return CeCosDialog::PreTranslateMessage(pMsg);
}

void CAddRemoveDialog::MoveChild(UINT id,int delta,MoveType how)
{
    CWnd *pWnd=GetDlgItem(id);
    CRect rect;
	pWnd->GetWindowRect(rect);
	ScreenToClient(rect);
    switch(how){
        case Move:
	        rect.left+=delta;
	        rect.right+=delta;
            break;
        case Stretch:
	        rect.right+=delta;
            break;
        case Up:
            rect.top-=delta;
            rect.bottom-=delta;
            break;
    }
    pWnd->MoveWindow(rect);
}

void CAddRemoveDialog::UpdateDescription(CListBox &lb)
{
    CString strDesc;
    // Set the description if the listbox has exactly one item selected
    if(1==lb.GetSelCount()){
	    int nIndex;
	    lb.GetSelItems(1,&nIndex);
        strDesc=m_arstrDescs[lb.GetItemData(nIndex)];
    }
    m_wndDesc.SetWindowText(strDesc);
}

void CAddRemoveDialog::SetHorizontalResizingLimit(int nPercent)
{
    m_nHorizontalResizingLimit=nPercent;
}
