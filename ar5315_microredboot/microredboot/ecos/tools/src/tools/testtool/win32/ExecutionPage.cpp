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
// ExecutionPage.cpp : implementation file
//
#include "stdafx.h"
#include "ExecutionPage.h"
#include "shlobj.h"
#include "RunTestsSheet.h"
#include <afxpriv.h> // for WM_KICKIDLE
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExecutionPage property page

const UINT arIds []={IDC_TT_SELECT_ALL,IDC_TT_UNSELECT_ALL,IDC_TT_ADD, IDC_TT_FOLDER, IDC_TT_REMOVE};

IMPLEMENT_DYNCREATE(CExecutionPage, CeCosPropertyPage)

CExecutionPage::CExecutionPage() : 
  CeCosPropertyPage(IDD_TT_EXECUTION_PAGE),
  m_strExtension(_T("*.exe"))
{
  GetCurrentDirectory(MAX_PATH,m_strFolder.GetBuffer(MAX_PATH));
  m_strFolder.ReleaseBuffer();
  //{{AFX_DATA_INIT(CExecutionPage)
		// NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}

CExecutionPage::~CExecutionPage()
{
}

void CExecutionPage::DoDataExchange(CDataExchange* pDX)
{
  CeCosPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CExecutionPage)
  DDX_Control(pDX, IDC_TT_RUNTESTS_LIST, m_List);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExecutionPage, CeCosPropertyPage)
//{{AFX_MSG_MAP(CExecutionPage)
ON_BN_CLICKED(IDC_TT_FOLDER, OnFolder)
ON_BN_CLICKED(IDC_TT_SELECT_ALL, OnSelectAll)
ON_BN_CLICKED(IDC_TT_UNSELECT_ALL, OnUnselectAll)
ON_BN_CLICKED(IDC_TT_ADD, OnAdd)
ON_BN_CLICKED(IDC_TT_REMOVE, OnRemove)
ON_WM_SIZE()
ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
ON_WM_CHAR()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExecutionPage message handlers
CExecutionPage *CExecutionPage::pDlg=NULL;
LRESULT CALLBACK CExecutionPage::WindowProcNew(HWND hwnd,UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message ==  WM_COMMAND) {
    switch(wParam){
    case MAKEWPARAM(IDC_TT_RECURSE,BN_CLICKED):
      pDlg->m_bRecurse ^= 1;
      pDlg->m_Button.SetCheck(pDlg->m_bRecurse);
      return 0;
    case MAKEWPARAM(IDC_TT_EXTENSION,EN_CHANGE):
      {
        CString str;
        pDlg->m_Combo.GetWindowText(str);
        pDlg->m_strExtension=(LPCTSTR)str;
      }
      return 0;
    default:
      break;
    }
  }
  return CallWindowProc(pDlg->m_wndProc, hwnd, message, wParam, lParam);
}

int CALLBACK CExecutionPage::CBBrowseCallbackProc( HWND hwnd, 
                                                  UINT uMsg, 
                                                  LPARAM lParam, 
                                                  LPARAM lpData 
                                                  )
{
  pDlg=(CExecutionPage *)lpData;
  switch(uMsg){
  case BFFM_INITIALIZED:
    {
      ::SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)(LPCTSTR)pDlg->m_strFolder);
      CWnd *pWnd=CWnd::FromHandle(hwnd);
      pWnd->SetWindowText(_T("Add Files from Folder"));
      
      // Get rect of IDCANCEL button to the right
      CRect rect,rect1,rect2;
      pWnd->GetDlgItem(IDCANCEL)->GetWindowRect(&rect1);
      pWnd->GetWindowRect(&rect2);
      int nDlgMargin=rect2.right-rect1.right;
      int nButtonHeight=rect1.Height();
      rect.left=rect.top=nDlgMargin;
      rect.right=rect.left+6*nButtonHeight;
      rect.bottom=rect.top+(10*nButtonHeight)/14;
      WPARAM wFont=(WPARAM)GetStockObject(DEFAULT_GUI_FONT);
      pDlg->m_Button.CreateEx(0,_T("BUTTON"),NULL, WS_VISIBLE|WS_CHILD|BS_CHECKBOX, rect, pWnd, IDC_TT_RECURSE);
      pDlg->m_Button.SetWindowText(_T("&Add from subfolders"));
      pDlg->m_Button.SendMessage(WM_SETFONT, wFont, 0);
      pDlg->m_Button.SetCheck(pDlg->m_bRecurse);
      
      rect.left=rect.right+(4*nButtonHeight)/14;
      rect.right=rect.left+3*nButtonHeight;
      rect.bottom+=2*GetSystemMetrics(SM_CYBORDER)+4;
      pDlg->m_Static.Create(_T("Files of type:"),WS_VISIBLE|WS_CHILD|SS_LEFT, rect, pWnd);
      pDlg->m_Static.SendMessage(WM_SETFONT, wFont, 0);
      rect.bottom-=2*GetSystemMetrics(SM_CYBORDER)+4;
      
      rect.left=rect.right+(4*nButtonHeight)/14;
      rect.right=rect.left+3*nButtonHeight;
      //rect.bottom=rect.top+2*nButtonHeight;
      rect.top=rect.bottom-(12*nButtonHeight)/14;
      pDlg->m_Combo.CreateEx(WS_EX_CLIENTEDGE,_T("Edit"),NULL,WS_VISIBLE|WS_CHILD|WS_BORDER|ES_LEFT/*|CBS_DROPDOWN*/, rect, pWnd, IDC_TT_EXTENSION);
      //pDlg->m_Combo.AddString(pDlg->m_strExtension);
      //pDlg->m_Combo.SetCurSel(0);
      pDlg->m_Combo.SetWindowText(pDlg->m_strExtension);
      pDlg->m_Combo.SendMessage(WM_SETFONT, wFont, 0);
      
      pDlg->m_wndProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (long)WindowProcNew);
      
    }
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

void CExecutionPage::OnFolder() 
{
  
  BROWSEINFO bi;
  bi.hwndOwner = GetSafeHwnd(); 
  bi.pidlRoot = NULL;   
  bi.pszDisplayName = m_strFolder.GetBuffer(MAX_PATH);
  bi.lpszTitle = _T("");
  bi.ulFlags = BIF_RETURNONLYFSDIRS|BIF_STATUSTEXT/*|0x0010 BIF_EDITBOX*/;             
  bi.lpfn = (BFFCALLBACK)CBBrowseCallbackProc;
  bi.lParam = (LPARAM)this;
  
  bool bSaveRecurse=m_bRecurse;
  LPITEMIDLIST iil = SHBrowseForFolder(&bi);
  m_strFolder.ReleaseBuffer();
  if(iil){
    SHGetPathFromIDList(iil,m_strFolder.GetBuffer(MAX_PATH));
    m_strFolder.ReleaseBuffer();
    SetModified();
    FillListBox(m_strFolder);
  } else {
    m_bRecurse=bSaveRecurse;
  }
  
}

/*
void CExecutionPage::OnRefresh() 
{
FillListBox(m_strFolder);
}
*/

void CExecutionPage::FillListBox(LPCTSTR pszFolder)
{
  CWaitCursor wait;
  CString strOldFolder;
  ::GetCurrentDirectory(MAX_PATH,strOldFolder.GetBuffer(MAX_PATH));
  strOldFolder.ReleaseBuffer();
  
  if(::SetCurrentDirectory(pszFolder)){
    int nCount=m_List.GetCount();
    m_List.Dir(DDL_READWRITE,m_strExtension);
    for(int i=nCount;i<m_List.GetCount();i++){
      TCHAR strFull[1+MAX_PATH];
      CString str;
      m_List.GetText(i,str);
      TCHAR *pFile;
      ::GetFullPathName(str,MAX_PATH,strFull,&pFile);
      m_List.DeleteString(i);
      if(LB_ERR==m_List.FindStringExact(-1,strFull)){
        m_List.InsertString(i,strFull);
        m_List.SetCheck(i,TRUE);
      } else {
        --i;
      }
    }
    if(m_bRecurse){
		    CFileFind finder;
        BOOL bMore=finder.FindFile();
        while (bMore)    {
          bMore = finder.FindNextFile();
          if(finder.IsDirectory() && !finder.IsDots()){
            FillListBox(finder.GetFileName());
          }
        }
    }
    ::SetCurrentDirectory(strOldFolder);
  }
  GetDlgItem(IDC_TT_SELECT_ALL)->EnableWindow(m_List.GetCount()>0);
  GetDlgItem(IDC_TT_UNSELECT_ALL)->EnableWindow(m_List.GetCount()>0);
}

void CExecutionPage::OnSelectAll() 
{
  for(int i=0;i<m_List.GetCount();i++){
    m_List.SetCheck(i,TRUE);
  }
}

void CExecutionPage::OnUnselectAll() 
{
  for(int i=0;i<m_List.GetCount();i++){
    m_List.SetCheck(i,FALSE);
  }
}

bool CExecutionPage::IsSelected(int i)
{
  return TRUE==m_List.GetCheck(i);
}

void CExecutionPage::OnAdd() 
{
  CFileDialog dlg(TRUE, _T("exe"), _T("*.exe"), OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST,
    _T("Executables(*.exe)\0*.exe\0"));
  dlg.m_ofn.lpstrTitle=_T("Add");
  if(IDOK==dlg.DoModal()){
    LPCTSTR pszDir=dlg.m_ofn.lpstrFile;
    DWORD dwAttr=GetFileAttributes(pszDir);
    if(0xFFFFFFFF!=dwAttr){
      bool bErr=false;
      if(dwAttr&FILE_ATTRIBUTE_DIRECTORY){
        for(LPCTSTR c=pszDir+_tcslen(pszDir)+1;_TCHAR('\0')!=*c;c+=_tcslen(c)+1){
          CString str(pszDir);
          str+=_TCHAR('\\');
          str+=c;
          if(LB_ERR==m_List.FindStringExact(-1,str)){
            m_List.SetCheck(m_List.AddString(str),TRUE);
          } else {
            bErr=true;
          }
        }
      } else {
        if(LB_ERR==m_List.FindStringExact(-1,pszDir)){
          m_List.SetCheck(m_List.AddString(pszDir),TRUE);
        } else {
          bErr=true;
        }
      }
      if(bErr){
        MessageBox(_T("One or more of the files was already present"));
      }
    }
  }
  GetDlgItem(IDC_TT_SELECT_ALL)->EnableWindow(m_List.GetCount()>0);
  GetDlgItem(IDC_TT_UNSELECT_ALL)->EnableWindow(m_List.GetCount()>0);
}


int CExecutionPage::SelectedTestCount()
{
  int n=0;
  if(IsWindow(m_List.m_hWnd)){
    for(int i=0;i<m_List.GetCount();i++){
      n+=m_List.GetCheck(i);
    }
  }
  return n;
}

CString CExecutionPage::SelectedTest(int nIndex)
{
  CString str;
  for(int i=0;i<m_List.GetCount();i++){
    if(m_List.GetCheck(i)){
      if(0==nIndex--){
        m_List.GetText(i,str);
        break;
      }
    }
  }
  return str;
  
}

BOOL CExecutionPage::OnInitDialog() 
{
  CeCosPropertyPage::OnInitDialog();
  for(POSITION pos = m_arstrPreLoad.GetStartPosition(); pos != NULL; ){
    CString strFile;
    void *p;
    m_arstrPreLoad.GetNextAssoc( pos, strFile,p);
    m_List.SetCheck(m_List.AddString(strFile),0!=p);
  }
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CExecutionPage::OnSize(UINT nType, int cx, int cy) 
{
  CRect rect;
  GetClientRect(rect);
  CeCosPropertyPage::OnSize(nType, cx, cy);
  
  CWnd *pWnd=GetDlgItem(IDC_TT_RUNTESTS_LIST);
  if(pWnd){
    ((CRunTestsSheet*)GetParent())->MoveWindow(pWnd,CRunTestsSheet::Stretch);
    const int nWnds=sizeof(arIds)/sizeof(arIds[0]);
    CRect rc[nWnds];
    
    CWnd *pWnd0=GetDlgItem(arIds[0]);
    ((CRunTestsSheet*)GetParent())->MoveWindow(pWnd0,CRunTestsSheet::TopLeft);
    CWnd *pWndn=GetDlgItem(arIds[nWnds-1]);
    ((CRunTestsSheet*)GetParent())->MoveWindow(pWndn,CRunTestsSheet::TopRight);
    
    CRect rect0;
    pWnd0->GetWindowRect(rect0);
    ScreenToClient(rect0);
    
    CRect rectn;
    pWndn->GetWindowRect(rectn);
    ScreenToClient(rectn);
    
    int nSpacing=(rectn.left-rect0.left)/(nWnds-1);
    for(int i=1;i<nWnds-1;i++){
      CRect rect;
      rect.left=rect0.left+i*nSpacing;
      rect.right=rect.left+rect0.Width();
      rect.top=rect0.top;
      rect.bottom=rect0.bottom;
      pWnd=GetDlgItem(arIds[i]);
      pWnd->GetWindowRect(rc[i]);
      ScreenToClient(rc[i]);
      pWnd->MoveWindow(rect);
    }
    
    for(i=0;i<sizeof(arIds)/sizeof(arIds[0]);i++){
      InvalidateRect(rc[i]);
      GetDlgItem(arIds[i])->Invalidate();
    }
    
  }	
}

LRESULT CExecutionPage::OnKickIdle(WPARAM, LPARAM)
{
  if(IsWindow(m_List.m_hWnd)){
    int n=m_List.GetCount();
    bool bSelectAll=false;
    bool bUnSelectAll=false;
    if(n>0){
      bool bPrev=false;
      for(int i=0;i<n;i++){
        bool bCheck=(TRUE==m_List.GetCheck(i));
        if(bCheck){
          bUnSelectAll=true;
        } else {
          bSelectAll=true;
        }
        if(i>0 && bCheck!=bPrev){
          break;
        }
        bPrev=bCheck;
      }
    }
    GetDlgItem(IDC_TT_SELECT_ALL)->EnableWindow(bSelectAll);
    GetDlgItem(IDC_TT_UNSELECT_ALL)->EnableWindow(bUnSelectAll);
  }
  return 0;
}

bool CExecutionPage::SomeTestsSelected()
{
  bool b=0;
  if(IsWindow(m_List.m_hWnd)){
    for(int i=0;i<m_List.GetCount();i++){
      if(m_List.GetCheck(i)){
        b=true;
        break;
      }
    }
  }
  return b;
  
}

void CExecutionPage::OnRemove() 
{
  m_List.SendMessage(WM_COMMAND,IDC_TT_REMOVE,0);
}
