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
// RunTestsSheet.cpp : implementation file
//

#include "stdafx.h"

#include "eCosTest.h"
#include "eCosThreadUtils.h"
#include "eCosTrace.h"
#include "PropertiesDialog.h"
#include "ResetAttributes.h"
#include "RunTestsSheet.h"
#include "TestResource.h"
#include "X10.h"

#include <afxpriv.h>

static const UINT arIds []={IDOK,IDCANCEL,ID_APPLY_NOW,IDHELP};


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CRunTestsSheet

IMPLEMENT_DYNAMIC(CRunTestsSheet, CeCosPropertySheet)

CRunTestsSheet::CRunTestsSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage, CBFunc pInitFunc, CRunTestsSheet **ppSheet)
:CeCosPropertySheet(pszCaption, pParentWnd, iSelectPage),
  m_pResource(NULL),
  m_pInitFunc(pInitFunc),
  m_bAllowResizing(false),
  m_cxMin(0),
  m_cyMin(0),
  m_Status(Stopped),
  m_bHideTarget(false),
  m_bHideRemoteControls(false),
  m_bModal(false),
  m_ppSheet(ppSheet),
  m_ep(CeCosTest::ExecutionParameters::RUN),
  m_nTimeout(900),
  m_nDownloadTimeout(120),
  m_nTimeoutType(TIMEOUT_AUTOMATIC),
  m_nDownloadTimeoutType(TIMEOUT_SPECIFIED),
  m_bRemote(false),
  m_bSerial(true),
  m_strPort(_T("COM1")),
  m_nBaud(38400),
  m_nLocalTCPIPPort(1),
  m_nReset(RESET_MANUAL),
  m_nResourcePort(1),
  m_nRemotePort(1),
  m_bFarmed(true)
{
  InitializeCriticalSection(&m_CS);
  AddPage(&executionpage);
  AddPage(&outputpage);
  AddPage(&summarypage);
  
}

CRunTestsSheet::~CRunTestsSheet()
{
  if(m_ppSheet){
    *m_ppSheet=0;
  }
  delete m_pResource;
  DeleteCriticalSection(&m_CS);
}


BEGIN_MESSAGE_MAP(CRunTestsSheet, CeCosPropertySheet)
//{{AFX_MSG_MAP(CRunTestsSheet)
ON_BN_CLICKED(IDOK, OnRun)
ON_BN_CLICKED(ID_APPLY_NOW, OnProperties)
ON_BN_CLICKED(IDCANCEL, OnClose)
ON_MESSAGE(WM_TESTOUTPUT, OnTestOutput)
ON_MESSAGE(WM_RUNCOMPLETE, OnTestsComplete)
ON_WM_SYSCOMMAND()
ON_WM_SIZE()
ON_WM_GETMINMAXINFO()
ON_WM_TIMER()
ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
ON_WM_CREATE()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRunTestsSheet message handlers

BOOL CRunTestsSheet::OnInitDialog() 
{
  if(!m_bHideTarget){
    m_prop.Add(_T("Platform"),m_strTarget);
  }
  m_prop.Add(_T("Active timeout"),m_nTimeout);
  m_prop.Add(_T("Download timeout"),m_nDownloadTimeout);
  m_prop.Add(_T("Active timeout type"),m_nTimeoutType);
  m_prop.Add(_T("Download timeout type"),m_nDownloadTimeoutType);
  m_prop.Add(_T("Remote"),m_bRemote);
  m_prop.Add(_T("Serial"),m_bSerial);
  m_prop.Add(_T("Port"),m_strPort);
  m_prop.Add(_T("Baud"),m_nBaud);
  m_prop.Add(_T("Local TCPIP Host"),m_strLocalTCPIPHost);
  m_prop.Add(_T("Local TCPIP Port"),m_nLocalTCPIPPort);
  m_prop.Add(_T("Reset Type"),m_nReset);
  m_prop.Add(_T("Reset String"),m_strReset);
  m_prop.Add(_T("Resource Host"),m_strResourceHost);
  m_prop.Add(_T("Resource Port"),m_nResourcePort);
  m_prop.Add(_T("Remote Host"),m_strRemoteHost);
  m_prop.Add(_T("Remote Port"),m_nRemotePort);
  m_prop.Add(_T("Recurse"),executionpage.m_bRecurse);
  //m_prop.Add(_T("Loadfromdir"),executionpage.m_strLoaddir);
  m_prop.Add(_T("Farmed"),m_bFarmed);
  m_prop.Add(_T("Extension"),executionpage.m_strExtension);
  
  CeCosTrace::SetOutput(TestOutputCallback,this);
  CeCosTrace::SetError (TestOutputCallback,this);
  
  // m_psh can only be used to set the small icon.  Set the large one here.
  m_psh.hIcon=AfxGetApp()->LoadIcon(IDR_TT_MAINFRAME);
  //sheet.m_psh.dwFlags|=PSH_USEHICON/*|PSH_HASHELP*/;
  if(m_psh.hIcon){
    SetIcon(m_psh.hIcon,FALSE);
    SetIcon(m_psh.hIcon,TRUE);
  }
  GetWindowRect(m_rcPrev);
#ifdef _DEBUG
  CeCosTrace::EnableTracing(CeCosTrace::TRACE_LEVEL_TRACE);
#endif
  CeCosTrace::SetInteractive(true);
  
  if(m_pInitFunc){
    m_pInitFunc(&m_prop,false);
  }
  
  GetDlgItem(IDCANCEL)->SetWindowText(_T("&Close"));
  
  m_nTestsToComplete=0;
  BOOL bResult = CeCosPropertySheet::OnInitDialog();
  SetDlgItemText(IDOK,_T("&Run"));
  SetDlgItemText(ID_APPLY_NOW,_T("&Properties"));
  GetDlgItem(ID_APPLY_NOW)->EnableWindow(TRUE);
  GetDlgItem(IDCANCEL)->EnableWindow(TRUE); // required for modeless case
  
  SetActivePage(&outputpage);
  SetActivePage(&summarypage);
  SetActivePage(&executionpage);
 	
  if(m_pInitFunc){
    m_pInitFunc(&m_prop,false);
    outputpage.UpdateData(FALSE);
    summarypage.UpdateData(FALSE);
    executionpage.UpdateData(FALSE);
  }
  /*
  CString strCaption = _T("Output");
  TC_ITEM tcItem;
  tcItem.mask = TCIF_TEXT;
  tcItem.pszText = (LPTSTR)((LPCTSTR)strCaption);
  GetTabControl()->SetItem(2, &tcItem );
  strCaption=_T("Summary");
  GetTabControl()->SetItem(3, &tcItem );
  */
  // Allow resizing
  
  // WS_OVERLAPPEDWINDOW would preclude caption bar help button
  ModifyStyle(0,WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME ,0);
  
  CRect rect;
  GetWindowRect(rect);
  m_rcPrev=rect;
  
  m_cxMin=rect.Width();
  m_cyMin=rect.Height();
  
  m_bAllowResizing=true;
  
  WINDOWPLACEMENT wndpl;
  if (5==_stscanf(m_strPlacement,_T("%d %d %d %d %d"),&rect.left,&rect.top,&rect.right,&rect.bottom,&wndpl.showCmd)){
    CRect rcMax;
    SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)(RECT *)rcMax, 0);
    
    if(rect.Width()<100 || rect.Height()<100 || rect.Width()>rcMax.Width() || rect.Height()>rcMax.Height()){
      rect=CFrameWnd::rectDefault;
    }
    
    wndpl.length = sizeof(WINDOWPLACEMENT);
    wndpl.flags = 0;
    
    wndpl.ptMinPosition = CPoint(0, 0);
    wndpl.ptMaxPosition =CPoint(-::GetSystemMetrics(SM_CXBORDER),-::GetSystemMetrics(SM_CYBORDER));
    wndpl.rcNormalPosition = rect;
    
    // sets window's position and iconized/maximized status
    SetWindowPlacement(&wndpl);
  }
  
  // Hack: force an initial sizing (without which the tab control is badly sized)
  m_rcOffset.left=m_rcOffset.right=m_rcOffset.top=0;
  
  m_rcOffset.bottom=m_bModal?50:-50;
  MoveWindow(GetTabControl(),Stretch);
  for(int i=0;i<GetPageCount();i++){
    MoveWindow(GetPage(i),Stretch);
  }
#ifdef _DEBUG
    for(CWnd *p=GetWindow(GW_CHILD);p;p=p->GetWindow(GW_HWNDNEXT)){
      TCHAR buf[256];
      ::GetClassName(p->m_hWnd,buf,sizeof buf);
      TRACE(_T("Window %x id=%d class=%s\n"),p,p->GetDlgCtrlID(),buf);
    }
#endif
  for(i=0;i<sizeof(arIds)/sizeof(arIds[0]);i++){
    CWnd *pWnd=GetDlgItem(arIds[i]);
    if(pWnd){
      MoveWindow(pWnd,BottomRight);
      pWnd->ShowWindow(SW_SHOW); // necessary in the modeless case
    } else {
      TRACE(_T("Failed to find window id=%x\n"),arIds[i]);
    }
  }

  // hack to lay buttons out correctly in application case
  if(this==AfxGetMainWnd()){
    CRect rect1,rect2;
    GetDlgItem(IDOK)->GetWindowRect(rect1);
    GetDlgItem(IDCANCEL)->GetWindowRect(rect2);
    CRect rect(rect1);
    rect.left-=(rect2.left-rect1.left);
    rect.right-=(rect2.right-rect1.right);
    ScreenToClient(rect);
    GetDlgItem(ID_APPLY_NOW)->MoveWindow(rect);
    GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_SHOW);
  }

  if(!m_bModal){
    SetTimer(0,100,0);
  }
  BringWindowToTop();
  return bResult;
}

void CRunTestsSheet::OnRun()
{
  
  if(Running==m_Status){
    outputpage.AddLogMsg(_T("Run canceled"));
    m_Status=Stopping;
    EnterCriticalSection(&m_CS);
    m_nNextToSubmit=0x7fffffff;
    LeaveCriticalSection(&m_CS);
    CeCosTest::CancelAllInstances();  
  } else {
    outputpage.UpdateData(TRUE);
    summarypage.UpdateData(TRUE);
    executionpage.UpdateData(TRUE);
    if(0==executionpage.SelectedTestCount()){
      MessageBox(_T("No tests have selected for execution"));
    } else {
      m_ep=CeCosTest::ExecutionParameters(
        CeCosTest::ExecutionParameters::RUN,
        m_strTarget,
        TIMEOUT_NONE==m_nTimeoutType?0x7fffffff:TIMEOUT_AUTOMATIC==m_nTimeoutType?900000:1000*m_nTimeout,
        TIMEOUT_NONE==m_nDownloadTimeoutType?0x7fffffff:TIMEOUT_AUTOMATIC==m_nDownloadTimeoutType?0:1000*m_nDownloadTimeout);
      if(m_bRemote){
        CTestResource::SetResourceServer(CeCosSocket::HostPort(m_strResourceHost,m_nResourcePort));
        if(!CTestResource::Load()){
          MessageBox(_T("Could not connect to resource server"));
          return;
        }
      } else {
        const String strPort(m_bSerial?(LPCTSTR)m_strPort:CeCosSocket::HostPort(m_strLocalTCPIPHost,m_nLocalTCPIPPort));
        if(0==strPort.size()){
          m_pResource=new CTestResource(_T(""),m_ep.PlatformName());
        } else {
          int nBaud=m_bSerial?m_nBaud:0;
          if (RESET_X10!=m_nReset) {
            m_pResource=new CTestResource(_T(""),m_ep.PlatformName(),strPort,nBaud);
          } else {
            m_pResource=new CTestResource(_T(""),m_ep.PlatformName(),strPort,nBaud,m_strReset);
          }
        }
      }
      m_Status=Running;
      SetDlgItemText(IDOK,_T("&Stop"));
      m_nNextToSubmit=0;
      outputpage.AddLogMsg(_T("Run started"));
      SubmitTests();
    }
  }
}

struct Info {
  CRunTestsSheet *pSheet;
  CeCosTest      *pTest;
};

DWORD CRunTestsSheet::X10ThreadFunc (void *pParam)
{
  Info *pInfo=(Info *)pParam;
  String str;
  bool bOk=false;
  CResetAttributes::ResetResult n=pInfo->pSheet->m_pResource->Reset(str);
  if(CResetAttributes::RESET_OK!=n){
    str+=_T(">>> Could not reset target\n");
  }
  str+=_TCHAR('\n');
  LPTSTR pszCopy=new TCHAR[1+str.size()];
  _tcscpy(pszCopy,str);
  pInfo->pSheet->PostMessage(WM_TESTOUTPUT,(WPARAM)pszCopy,0);
  
  if(bOk){
    // we're already in a thread, so we can call the function directly
    pInfo->pTest->RunLocal();
  }
  RunCallback(pInfo);
  return 0;
}

void CALLBACK CRunTestsSheet::RunLocalFunc(void *pParam)
{
  ((Info *)pParam)->pTest->RunLocal();
}

void CALLBACK CRunTestsSheet::RunRemoteFunc(void *pParam)
{
  ((Info *)pParam)->pTest->RunRemote(NULL);
}

void CRunTestsSheet::SubmitTests()
{
  int nResources=m_bRemote?max(1,CTestResource::GetMatchCount (m_ep)):1;
  if(nResources>CeCosTest::InstanceCount){
    if(m_nNextToSubmit>=executionpage.SelectedTestCount()){
      return;
    }
    Info *pInfo=new Info;
    pInfo->pTest=new CeCosTest(m_ep, executionpage.SelectedTest(m_nNextToSubmit++));
    pInfo->pSheet=this;
    m_nTestsToComplete++;
    if(m_bRemote){
      CeCosThreadUtils::RunThread(RunRemoteFunc,pInfo,RunCallback,_T("RunRemoteFunc"));
    } else {
      bool bRun=false;
      switch((ResetType)m_nReset){
        case RESET_NONE:
          bRun=true;
          break;
        case RESET_X10:
          // Resetting can take a while, so spawn a thread
          bRun=false;
          {
            DWORD dwID;
            CloseHandle(CreateThread(0,0,X10ThreadFunc, pInfo, 0, &dwID));
          }
          break;
        case RESET_MANUAL:
          bRun=(IDOK==MessageBox(_T("Press OK when target is reset - cancel to abort run"),NULL,MB_OKCANCEL));
          if(!bRun){
            m_nNextToSubmit=executionpage.SelectedTestCount();
            RunCallback(pInfo);
          }
          break;
      }
      if(bRun){
        CeCosThreadUtils::RunThread(RunLocalFunc,pInfo,RunCallback,_T("RunLocalFunc"));
      }
    }
  }
}

void CRunTestsSheet::RunCallback(void *pParam)
{
  Info *pInfo=(Info *)pParam;
  CRunTestsSheet *pSheet=pInfo->pSheet;
  if(::IsWindow(pSheet->m_hWnd)){ //FIXME
    CeCosTest *pTest=pInfo->pTest;
    EnterCriticalSection(&pSheet->m_CS);
  
    pSheet->summarypage.AddResult(pTest);
    delete pTest;
  
    pSheet->m_nTestsToComplete--;
    pSheet->SubmitTests();   
  
    if(0==pSheet->m_nTestsToComplete){
      // It would be nice to do this in the handler for WM_RUNCOMPLETE, but we must be in the CS
      delete pSheet->m_pResource;
      pSheet->m_pResource=0;
      pSheet->PostMessage(WM_RUNCOMPLETE,0,0);
    }
    LeaveCriticalSection(&pSheet->m_CS);
  }
  delete pInfo;
}

LRESULT CRunTestsSheet::OnTestsComplete(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(wParam);
  UNUSED_ALWAYS(lParam);
  m_Status=Stopped;
  SetDlgItemText(IDOK,_T("&Run"));
  outputpage.AddLogMsg(_T("Run complete"));
  return 0;
}

void CALLBACK CRunTestsSheet::TestOutputCallback(void *pParam,LPCTSTR psz)
{
  CRunTestsSheet*pWnd=(CRunTestsSheet*)pParam;
  if(::IsWindow(pWnd->m_hWnd)){ //FIXME
    LPTSTR pszCopy=new TCHAR[1+_tcslen(psz)];
    _tcscpy(pszCopy,psz);
    pWnd->PostMessage(WM_TESTOUTPUT,(WPARAM)pszCopy,0);
  }
}

LRESULT CRunTestsSheet::OnTestOutput(WPARAM wParam, LPARAM lParam)
{
  UNUSED_ALWAYS(lParam);
  LPTSTR psz=(LPTSTR)wParam;
  outputpage.AddText(psz);
  delete [] psz;
  return 0;
}

void CRunTestsSheet::OnProperties()
{
  CPropertiesDialog dlg(m_bHideTarget,m_bHideRemoteControls);
  dlg.m_strTarget=m_strTarget;
  dlg.m_nTimeout=m_nTimeout;
  dlg.m_nDownloadTimeout=m_nDownloadTimeout;
  dlg.m_nTimeoutType=m_nTimeoutType;
  dlg.m_nDownloadTimeoutType=m_nDownloadTimeoutType;
  dlg.m_bRemote=m_bRemote;
  dlg.m_bSerial=m_bSerial;
  dlg.m_strPort=m_strPort;
  dlg.m_nBaud=m_nBaud;
  dlg.m_strLocalTCPIPHost=m_strLocalTCPIPHost;
  dlg.m_nLocalTCPIPPort=m_nLocalTCPIPPort;
  dlg.m_nReset=m_nReset;
  dlg.m_strReset=m_strReset;
  dlg.m_strResourceHost=m_strResourceHost;
  dlg.m_nResourcePort=m_nResourcePort;
  dlg.m_strRemoteHost=m_strRemoteHost;
  dlg.m_nRemotePort=m_nRemotePort;
  dlg.m_strPort=m_strPort;
  dlg.m_bFarmed=m_bFarmed;
  if(IDOK==dlg.DoModal()){
    m_strTarget=(LPCTSTR)dlg.m_strTarget;
    m_nTimeout=dlg.m_nTimeout;
    m_nDownloadTimeout=dlg.m_nDownloadTimeout;
    m_nTimeoutType=dlg.m_nTimeoutType;
    m_nDownloadTimeoutType=dlg.m_nDownloadTimeoutType;
    m_bRemote=dlg.m_bRemote;
    m_bSerial=dlg.m_bSerial;
    m_strPort=(LPCTSTR)dlg.m_strPort;
    m_nBaud=dlg.m_nBaud;
    m_strLocalTCPIPHost=(LPCTSTR)dlg.m_strLocalTCPIPHost;
    m_nLocalTCPIPPort=dlg.m_nLocalTCPIPPort;
    m_nReset=dlg.m_nReset;
    m_strReset=(LPCTSTR)dlg.m_strReset;
    m_strResourceHost=(LPCTSTR)dlg.m_strResourceHost;
    m_nResourcePort=dlg.m_nResourcePort;
    m_strRemoteHost=(LPCTSTR)dlg.m_strRemoteHost;
    m_nRemotePort=dlg.m_nRemotePort;
    m_bFarmed=dlg.m_bFarmed;
    if(m_pInitFunc){
      m_pInitFunc(&m_prop,true);
    }
  }
}

void CRunTestsSheet::OnClose()
{
  if(m_pInitFunc){
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(&wndpl);
    if(!IsWindowVisible()){
		    wndpl.showCmd=SW_HIDE;
    }
    m_strPlacement.Format(_T("%d %d %d %d %d"),
      wndpl.rcNormalPosition.left,
      wndpl.rcNormalPosition.top,
      wndpl.rcNormalPosition.right,
      wndpl.rcNormalPosition.bottom,
      wndpl.showCmd);
    m_pInitFunc(&m_prop,true);
  }
  if(m_bModal){
    EndDialog(IDOK);
  } else {
    DestroyWindow();
  }
}



void CRunTestsSheet::OnSysCommand(UINT nID, LPARAM lParam) 
{
  if(SC_CLOSE==nID){
    OnClose();
  } else {
    CeCosPropertySheet::OnSysCommand(nID, lParam);
  }
}

void CRunTestsSheet::OnSize(UINT nType, int cx, int cy) 
{
  // WS_OVERLAPPEDWINDOW would preclude caption bar help button
  ModifyStyle(0,WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME ,0);
  CeCosPropertySheet::OnSize(nType, cx, cy);
  if(SIZE_MINIMIZED!=nType){
    CRect rect;
    GetWindowRect(rect);
    TRACE(_T("OnSize(%d) left=%d top=%d right=%d bottom=%d ar=%d\n"),nType,rect.left,rect.top,rect.right,rect.bottom,m_bAllowResizing);
    
    m_rcOffset.left  =rect.left-m_rcPrev.left;
    m_rcOffset.right =rect.right-m_rcPrev.right;
    m_rcOffset.top   =rect.top-m_rcPrev.top;
    m_rcOffset.bottom=rect.bottom-m_rcPrev.bottom;
    
    m_rcPrev=rect;
    
    if(m_bAllowResizing){
      cx=max(m_cxMin,cx); 
      cy=max(m_cyMin,cy);
      MoveWindow(GetTabControl(),Stretch);
      CRect rc[sizeof(arIds)/sizeof(arIds[0])];
      CRect rcSheet;
      GetWindowRect(rcSheet);
      for(int i=0;i<sizeof(arIds)/sizeof(arIds[0]);i++){
        CWnd *pWnd=GetDlgItem(arIds[i]);
        if(pWnd){
          pWnd->GetWindowRect(rc[i]);
          ScreenToClient(rc[i]);
          MoveWindow(pWnd,BottomRight,FALSE);
        }
      }

      for(i=0;i<sizeof(arIds)/sizeof(arIds[0]);i++){
        CWnd *pWnd=GetDlgItem(arIds[i]);
        if(pWnd){
          pWnd->Invalidate();
          InvalidateRect(rc[i]);
        } else {
          TRACE(_T("Failed to find window id=%x\n"),arIds[i]);
        }
      }
      
      for(i=0;i<GetPageCount();i++){
        MoveWindow(GetPage(i),Stretch);
      }
      
    }
  }
}

void CRunTestsSheet::MoveWindow(CWnd *pWnd, AffinityType Affinity,bool bRepaint)
{
  if(m_bAllowResizing&&pWnd){
    TRACE(_T("MoveWindow left=%d top=%d right=%d bottom=%d\n"),m_rcOffset.left,m_rcOffset.top,m_rcOffset.right,m_rcOffset.bottom);
    CRect rect;
    pWnd->GetWindowRect(rect);
    pWnd->GetParent()->ScreenToClient(rect);
    TRACE(_T("           left=%d top=%d right=%d bottom=%d"),rect.left,rect.top,rect.right,rect.bottom);
    int nHeight=rect.Height();
    int nWidth=rect.Width();
    
    switch(Affinity){
    case BottomRight:
      rect.right +=m_rcOffset.Width();
      rect.bottom+=m_rcOffset.Height();
      rect.top =rect.bottom-nHeight;
      rect.left=rect.right -nWidth;
      break;
    case TopRight:
      rect.right +=m_rcOffset.Width();
      rect.left=rect.right -nWidth;
      break;
    case BottomLeft:
      rect.top +=m_rcOffset.Height();
      rect.right=rect.left+nWidth;
      break;
    case TopLeft:
      break;
    case Stretch:
      rect.right +=m_rcOffset.Width();
      rect.bottom+=m_rcOffset.Height();
      break;
    }
    TRACE(_T("        -> left=%d top=%d right=%d bottom=%d\n"),rect.left,rect.top,rect.right,rect.bottom);
    pWnd->MoveWindow(rect,bRepaint);
  }
}

void CRunTestsSheet::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
  lpMMI->ptMinTrackSize.x = m_cxMin;
  lpMMI->ptMinTrackSize.y = 175;
  CeCosPropertySheet::OnGetMinMaxInfo(lpMMI);
}

LRESULT CRunTestsSheet::OnKickIdle(WPARAM, LPARAM)
{
  // We could use WM_KICKIDLE to drive CMDUI messages, but in fact we just
  // use an OnKickIdle handler directly (here and in the pages) to control
  // button greying.
  bool bEnableRunStop=false;
  bool bEnableProperties=false;
  switch(m_Status){
  case Running:
    bEnableRunStop=true;
    break;
  case Stopping:
    bEnableProperties=true;
    break;
  case Stopped:
    bEnableRunStop=executionpage.SomeTestsSelected();
    bEnableProperties=true;
    break;
  }
  GetDlgItem(IDOK)->EnableWindow(bEnableRunStop);
  GetDlgItem(ID_APPLY_NOW)->EnableWindow(bEnableProperties);
  SendMessageToDescendants(WM_KICKIDLE, 0, 0, FALSE, FALSE);
  return 0;
}

BOOL CRunTestsSheet::PreTranslateMessage(MSG* pMsg) 
{
  if(WM_KEYDOWN==pMsg->message && VK_ESCAPE==pMsg->wParam){
    return TRUE;// escape character handled
  }
  return CeCosPropertySheet::PreTranslateMessage(pMsg);
}

void CRunTestsSheet::SetTarget(LPCTSTR pszTarget)
{
  m_strTarget=pszTarget;
  m_bHideTarget=true;
}

void CRunTestsSheet::HideRemoteControls()
{
  m_bHideRemoteControls=true;
}

void CRunTestsSheet::Populate(LPCTSTR pszFile,bool bSelect/*=true*/)
{
  executionpage.m_arstrPreLoad.SetAt(pszFile,bSelect?this:0);
}


int CRunTestsSheet::DoModal() 
{
  m_bModal=true;
  return CeCosPropertySheet::DoModal();
}

void CRunTestsSheet::PostNcDestroy() 
{
  if(!m_bModal){
    delete this;
  } else {
    CeCosPropertySheet::PostNcDestroy();
  }
}

void CRunTestsSheet::OnTimer(UINT nIDEvent) 
{
  SendMessage(WM_KICKIDLE,0,0);	
  CeCosPropertySheet::OnTimer(nIDEvent);
}

