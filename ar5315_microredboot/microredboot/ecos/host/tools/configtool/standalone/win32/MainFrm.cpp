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
// MainFrm.cpp : implementation of the CMainFrame class:
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the main frame class
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

#define WM_SUBPROCESS (WM_USER+42)

#include "BinDirDialog.h"
#include "BuildOptionsDialog.h"
#include "CTOptionsDialog.h"
#include "CTUtils.h"
#include "CellView.h"
#include "ConfigItem.h"
#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include "ControlView.h"
#include "DescView.h"
#include "FindDialog.h"
#include "FolderDialog.h"
#include "MLTFrame.h"
#include "MainFrm.h"
#include "MessageBox.h"
#include "PlatformsDialog.h"
#include "NotePage.h"
#include "OutputView.h"
#include "PropertiesView.h"
#include "RegionGeneralPage.h"
#include "RulesView.h"
#include "SectionGeneralPage.h"
#include "SectionRelocationPage.h"
#include "Splash.h"
#include "SplitterWndEx.h"
#include "Subprocess.h"
#include "TestResource.h"
#include "ThinSplitter.h"
#include "ViewOptions.h"

#define INCLUDEFILE "build.hxx"
#include "IncludeSTL.h"

#include <afxpriv.h>
#include <htmlhelp.h>
#include <windowsx.h> // for GET_X_LPARAM, GET_Y_LPARAM

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)
//UINT WM_FINDREPLACE = RegisterWindowMessage(FINDMSGSTRING);

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
  //{{AFX_MSG_MAP(CMainFrame)
  ON_WM_CREATE()
  ON_COMMAND(ID_VIEW_PROPERTIES, OnViewProperties)
  ON_UPDATE_COMMAND_UI(ID_VIEW_PROPERTIES, OnUpdateViewProperties)
  ON_COMMAND(ID_VIEW_MLT, OnViewMLT)
  ON_COMMAND(ID_VIEW_SHORTDESC, OnViewShortdesc)
  ON_UPDATE_COMMAND_UI(ID_VIEW_SHORTDESC, OnUpdateViewShortdesc)
  ON_COMMAND(ID_VIEW_OUTPUT, OnViewOutput)
  ON_UPDATE_COMMAND_UI(ID_VIEW_OUTPUT, OnUpdateViewOutput)
  ON_UPDATE_COMMAND_UI(ID_BUILD_CONFIGURE, OnUpdateBuildConfigure)
  ON_COMMAND(ID_BUILD_STOP, OnBuildStop)
  ON_UPDATE_COMMAND_UI(ID_BUILD_STOP, OnUpdateBuildStop)
  ON_COMMAND(ID_BUILD_CONFIGURE, OnConfigurationBuild)
  ON_WM_DESTROY()
  ON_COMMAND(ID_BUILD_TESTS, OnBuildTests)
  ON_UPDATE_COMMAND_UI(ID_BUILD_TESTS, OnUpdateBuildTests)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
  ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
  ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFileNew)
  ON_UPDATE_COMMAND_UI(ID_APP_EXIT, OnUpdateAppExit)
  ON_UPDATE_COMMAND_UI(ID_VIEW_MLT, OnUpdateViewMLT)
  ON_WM_SYSCOMMAND()
  ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_REFRESH, OnUpdateConfigurationRefresh)
  ON_COMMAND(ID_CONFIGURATION_REFRESH, OnConfigurationRefresh)
  ON_COMMAND(ID_MENU_VIEW_SETTINGS, OnViewSettings)
  ON_COMMAND(ID_TOOLS_PATHS, OnToolsPaths)
  ON_WM_SIZE()
  ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_REPOSITORY, OnUpdateConfigurationRepository)
  ON_COMMAND(ID_WINDOW_NEXT, OnWindowNext)
  ON_COMMAND(ID_WINDOW_PREV, OnWindowPrev)
  ON_COMMAND(ID_HELP_SUBMIT_PR, OnHelpSubmitPr)
  ON_COMMAND(ID_HELP_ECOS, OnHelpEcos)
  ON_COMMAND(ID_HELP_RED_HATONTHEWEB, OnHelpRedHatHome)
  ON_COMMAND(ID_HELP_UITRON, OnHelpUitron)
  ON_COMMAND(ID_HELP_ECOSHOME, OnHelpEcoshome)
  ON_COMMAND(ID_RUN_SIM, OnRunSim)
  ON_COMMAND(ID_BUILD_CLEAN, OnBuildClean)
  ON_UPDATE_COMMAND_UI(ID_BUILD_CLEAN, OnUpdateBuildClean)
  ON_COMMAND(ID_TOOLS_SHELL, OnToolsShell)
  ON_WM_CLOSE()
  ON_COMMAND(ID_BUILD_OPTIONS, OnBuildOptions)
  ON_COMMAND(ID_TOOLS_OPTIONS, OnToolsOptions)
  ON_COMMAND(ID_USERTOOLS_PATHS, OnUsertoolsPaths)
  ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
  ON_UPDATE_COMMAND_UI(ID_RUN_SIM, OnUpdateRunSim)
  ON_COMMAND(ID_VIEW_CONFLICTS, OnViewConflicts)
  ON_UPDATE_COMMAND_UI(ID_VIEW_CONFLICTS, OnUpdateViewConflicts)
  ON_COMMAND(ID_VIEW_MLTBAR, OnViewMltbar)
  ON_UPDATE_COMMAND_UI(ID_VIEW_MLTBAR, OnUpdateViewMltbar)
  ON_COMMAND(ID_RESOLVE_CONFLICTS, OnResolveConflicts)
  ON_UPDATE_COMMAND_UI(ID_RESOLVE_CONFLICTS, OnUpdateResolveConflicts)
  ON_COMMAND(ID_GO_HOME, OnGoHome)
  ON_WM_MEASUREITEM()
  ON_WM_INITMENUPOPUP()
  ON_UPDATE_COMMAND_UI(ID_BUILD_OPTIONS, OnUpdateBuildOptions)
  ON_MESSAGE(WM_SUBPROCESS,OnSubprocess)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_ADMINISTRATION, OnUpdateToolsAdministration)
	ON_WM_HELPINFO()
	ON_COMMAND(ID_EDIT_PLATFORMS, OnEditPlatforms)
  ON_WM_MENUCHAR()
  ON_COMMAND(ID_HELP, OnHelp)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
  ON_NOTIFY(HHN_NAVCOMPLETE,  ID_HHNOTIFICATION, OnNavComplete)
  ON_NOTIFY(HHN_TRACK,        ID_HHNOTIFICATION, OnNavComplete)
  ON_NOTIFY(HHN_WINDOW_CREATE,ID_HHNOTIFICATION, OnNavComplete)
END_MESSAGE_MAP()

UINT CMainFrame::indicators[] =
{
  ID_SEPARATOR,           // status line indicator
    ID_THERMOMETER,
    ID_SEPARATOR,           // status line indicator
    ID_SEPARATOR
};

bool CMainFrame::m_arMounted[26]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame():
  m_pFindReplaceDialog(NULL),
    m_strIdleMessage(),
    m_nThermometerMax(0),
    m_bFindInProgress(false),
    m_bStatusBarCreated(false)
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  DragAcceptFiles();
  
  if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  
  if (!m_wndToolBar.Create(this,WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,AFX_IDW_TOOLBAR) ||
    !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
  {
    TRACE0("Failed to create toolbar\n");
    return -1;      // fail to create
  }
  
  if (!m_wndMLTToolBar.Create(this,WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,AFX_IDW_TOOLBAR+1) ||
    !m_wndMLTToolBar.LoadToolBar(IDR_MLTBAR))
  {
    TRACE0("Failed to create MLT toolbar\n");
    return -1;      // fail to create
  }
  m_bMLTToolbarVisible=true;
  
  if (!m_wndStatusBar.Create(this) ||
    !m_wndStatusBar.SetIndicators(indicators,
    sizeof(indicators)/sizeof(UINT)))
  {
    TRACE0("Failed to create status bar\n");
    return -1;      // fail to create
  }
  m_bStatusBarCreated=true;
  CDC *pDC=GetDC();
  CFont *pOldFont=pDC->SelectObject(m_wndStatusBar.GetFont());
  m_nThermometerPaneSize=100;
  m_nPercentagePaneSize=pDC->GetTextExtent(_T("100%")).cx;
  m_nFailRulePaneSize=pDC->GetTextExtent(_T("333 conflicts")).cx;
  pDC->SelectObject(pOldFont);
  ReleaseDC(pDC);
  
  CRect rect;
  m_wndStatusBar.GetItemRect(ThermometerPane, rect);
  
  m_Progress.Create(WS_VISIBLE|WS_CHILD|PBS_SMOOTH, rect,&m_wndStatusBar, ID_THERMOMETER);
  m_Progress.SetRange(0,100);
  m_Progress.SetPos(0);
  
  //CConfigTool::GetConfigToolDoc()->UpdateFailingRuleCount();
  // TODO: Delete these three lines if you don't want the toolbar to
  //  be dockable
  m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
  m_wndMLTToolBar.EnableDocking(CBRS_ALIGN_ANY);
  
  EnableDocking(CBRS_ALIGN_ANY);
  DockControlBar(&m_wndToolBar);
  DockControlBar(&m_wndMLTToolBar);
  
  CSplashWnd::ShowSplashScreen(this);
  
  return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  //cs.style|=~FWS_ADDTOTITLE;
  return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
  CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
  CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
CMainFrame::ViewStruct CMainFrame::arView[PaneTypeMax]={
  CMainFrame::ViewStruct(_T("Cell"),              true,RUNTIME_CLASS(CCellView)),
  CMainFrame::ViewStruct(_T("Configuration"),     true,RUNTIME_CLASS(CControlView)),
  CMainFrame::ViewStruct(_T("Short Description"), true,RUNTIME_CLASS(CDescView)),
  CMainFrame::ViewStruct(_T("Output"),            true,RUNTIME_CLASS(COutputView)),
  CMainFrame::ViewStruct(_T("Properties"),        true,RUNTIME_CLASS(CPropertiesView)),
  CMainFrame::ViewStruct(_T("Rules"),             false,RUNTIME_CLASS(CRulesView)),
  CMainFrame::ViewStruct(_T("Memory Layout"),     false,RUNTIME_CLASS(CMLTView))
};

// This data defines the layout of the nested splitters
CMainFrame::Split CMainFrame::arSplit[]={
  CMainFrame::Split(Horz,1,0.8),
    CMainFrame::Split(Horz,2,0.7),
      CMainFrame::Split(Vert,3,0.5),
        CMainFrame::Split(Thin,4,0.4),
          CMainFrame::Split(Control,5),
          CMainFrame::Split(Cell,5),
        CMainFrame::Split(Horz,4,0.6),
          CMainFrame::Split(Rules,5),
          CMainFrame::Split(Horz,5,0.5),
            CMainFrame::Split(Properties,6),
            CMainFrame::Split(ShortDesc,6),
      CMainFrame::Split(MLT,3),
    CMainFrame::Split(Output,2)
};

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
  for(int i=0;i<sizeof arView/sizeof arView[0];i++){
    ViewStruct &v=arView[(PaneType)i];
    v.bVisible=GetApp()->GetProfileInt(_T("Windows"),v.pszName,v.bVisible);
  }
  CRect rcClient;
  GetClientRect(rcClient);
  
  BOOL rc=TRUE;
  
  //    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //    x         |         |                           x
  //    x Tree (T)| Cell(T) |                           x
  //    x ^^^^^^^^^^^^^^^^^^|           Rules (2)       x
  //    x         1         |                           x
  //    x         |         |---------------------------x
  //    x         |         |                           x
  //    x         |         |           Properties (2)  x
  //    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //    x           MLT                                 x
  //    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  //    x                                               x
  //    x           Output (0)                          x
  //    x                                               x
  //    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
  
  for(i=0;i<sizeof arSplit/sizeof arSplit[0];i++){
    const CSize szDefault(50,50);
    Split &s=arSplit[i];
    CSplitterWndEx *pParent=NULL;
    bool bSecond=false;
    int nRowParent=0;
    int nColParent=0;
    // Search backwards for our parent.  This is the first entry back of level one higher.
    for(int j=i-1;j>=0;--j){
      Split &sp=arSplit[j];
      if(sp.nLevel==s.nLevel-1){
        pParent=sp.pSplit;
        nRowParent=(bSecond && Horz==sp.type);
        nColParent=(bSecond && Horz!=sp.type);
        break;
      } else if(sp.nLevel==s.nLevel){
        bSecond=true;
      }
    }
    if(s.type>=0 /* a view */){
      TRACE(_T("Create view index=%d [%s]\n"),i,arView[s.type].pszName);
      VERIFY(pParent->CreateView (nRowParent, nColParent, arView[s.type].pClass, szDefault, pContext));
      arView[s.type].pView=(CView *)pParent->GetPane(nRowParent,nColParent);
      arView[s.type].pParent=pParent;
    } else {
      int nRows=1+(Horz==s.type);
      int nCols=1+(Horz!=s.type);
      UINT id;
      CWnd *pwndParent;
      if(NULL==pParent){
        pwndParent=this;
        id=AFX_IDW_PANE_FIRST;
      } else {
        id=pParent->IdFromRowCol(nRowParent,nColParent);
        pwndParent=pParent;
      }
      if(Thin==s.type){
        TRACE(_T("Create thin splitter index=%d\n"),i);
        s.pSplit=new CThinSplitter;
        VERIFY(s.pSplit->CreateStatic (pParent, 1, 2, WS_CHILD|WS_VISIBLE|WS_BORDER, id));
        s.pSplit->SetScrollStyle(WS_VSCROLL);
      } else {
        TRACE(_T("Create splitter index=%d\n"),i);
        s.pSplit=new CSplitterWndEx;
        VERIFY(s.pSplit->CreateStatic (pwndParent, nRows, nCols, WS_CHILD|WS_VISIBLE|WS_BORDER, id));
      }
    }
  };

  for(i=0;i<PaneTypeMax;i++){
    LOGFONT lf;
    ViewStruct &v=arView[(PaneType)i];
    GetApp()->LoadFont(v.pszName,lf);
    if(!arView[(PaneType)i].font.CreateFontIndirect(&lf)){
      CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT))->GetLogFont(&lf);
    }
    SetPaneFont((PaneType)i,lf);
    //v.pParent->ShowPane(v.pView,v.bVisible);
  }

  for(i=0;i<sizeof arSplit/sizeof arSplit[0];i++){
    Split &s=arSplit[i];
    if(s.pSplit){
      s.pSplit->RecalcLayout();
    }
  }

  UNUSED_ALWAYS(lpcs); // avoid compiler warning
  return rc;
}

void CMainFrame::OnViewProperties() 
{
  arView[Properties].bVisible^=1;
  arView[Properties].pParent->ShowPane(arView[Properties].pView,arView[Properties].bVisible);
}

void CMainFrame::OnUpdateViewProperties(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(arView[Properties].bVisible);
}

void CMainFrame::OnViewMLT() 
{
  arView[MLT].bVisible^=1;
  arView[MLT].pParent->ShowPane(arView[MLT].pView,arView[MLT].bVisible);
  ShowControlBar(&m_wndMLTToolBar,arView[MLT].bVisible&m_bMLTToolbarVisible,false);
}

void CMainFrame::OnViewShortdesc() 
{
  arView[ShortDesc].bVisible^=1;
  arView[ShortDesc].pParent->ShowPane(arView[ShortDesc].pView,arView[ShortDesc].bVisible);
}

void CMainFrame::OnUpdateViewShortdesc(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(arView[ShortDesc].bVisible);
}

void CMainFrame::OnViewOutput() 
{
  arView[Output].bVisible^=1;
  arView[Output].pParent->ShowPane(arView[Output].pView,arView[Output].bVisible);
}

void CMainFrame::OnUpdateViewOutput(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(arView[Output].bVisible);
}
	
void CMainFrame::OnUpdateBuildConfigure(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::OnBuildStop() 
{
  m_sp.Kill(); // leave the rest to OnSubprocessComplete()
}

void CMainFrame::OnUpdateBuildStop(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(m_sp.ProcessAlive());
}

void CMainFrame::OnConfigurationBuild() 
{
  Build();
}

DWORD CMainFrame::ThreadFunc(LPVOID param)
{
  CMainFrame *pMain=(CMainFrame *)param;
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  CString strCmd(_T("make -n "));
  strCmd+=pMain->m_strBuildTarget;
  SetCurrentDirectory(pDoc->BuildTree());
  String strOut;
  CSubprocess sp;
  sp.Run(strOut,strCmd);
  // Don't attempt to change the thermometer itself - not safe from a separate thread
  pMain->m_nThermometerMax=pDoc->GetCompilationCount(strOut);
  
  return 0;
}

void CMainFrame::Build(const CString &strWhat/*=_T("")*/)
{
  ASSERT(!m_sp.ProcessAlive());
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  
  if(!arView[Output].bVisible){
    SendMessage(WM_COMMAND,ID_VIEW_OUTPUT,0);
  }
  
  if(pDoc->IsModified()||pDoc->BuildTree().IsEmpty()){
    SendMessage (WM_COMMAND, ID_FILE_SAVE);
  }
  
  if(!(pDoc->IsModified()||pDoc->BuildTree().IsEmpty())){ // verify the save worked
    CString strCmd (_T("make"));
    if(!strWhat.IsEmpty()){
      strCmd+=_TCHAR(' ');
      strCmd+=strWhat;
    }
    if(!GetApp()->m_strMakeOptions.IsEmpty()){
      strCmd+=_TCHAR(' ');
      strCmd+=GetApp()->m_strMakeOptions;
    }

    strCmd += _T(" --directory ");
    
    // Quoting the name may not mix with the 'sh' command on Unix, so only do it
    // under Windows where it's more likely there will be spaces needing quoting.
#ifdef _WINDOWS
    CString buildDir(pDoc->BuildTree());
    
#if 1 // ecUSE_ECOS_X_NOTATION
    std::string cPath = cygpath(std::string(pDoc->BuildTree()));
    buildDir = cPath.c_str();
#endif
    strCmd += CString(_T("\"")) + buildDir + CString(_T("\""));
#else
    strCmd += CString(pDoc->BuildTree()) ;
#endif

    if(PrepareEnvironment()){
      m_strBuildTarget=strWhat;
      SetThermometerMax(250); // This is just a guess.  The thread we are about to spawn will work out the correct answer
      m_nLogicalLines=0;
      UpdateThermometer(0);
      CloseHandle(CreateThread(NULL, 0, ThreadFunc, this, 0 ,&m_dwThreadId));
      CString strMsg;
      strMsg.Format(_T("Building %s"),strWhat);
      SetIdleMessage(strMsg);
      
      SetTimer(42,1000,0); // This timer checks for process completion
      SetCurrentDirectory(pDoc->BuildTree());
      m_sp.Run(SubprocessOutputFunc, this, strCmd, false);
      SetIdleMessage();
    }
  }
}


CConfigToolApp * CMainFrame::GetApp()
{
  return (CConfigToolApp *)AfxGetApp();
}

void CMainFrame::ActivateFrame(int nCmdShow) 
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  CRect rcDefault;
  SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)(RECT *)rcDefault, 0);
  // Insist on nothing more than 1000 width, with the golden ratio
  rcDefault.DeflateRect(max(0,(rcDefault.Width()-1000)/2),max(0,(rcDefault.Height()-618)/2));
  
  GetApp()->RestoreWindowPlacement (this, _T("Window size"),rcDefault);
  LoadBarState(_T("DockState"));
  
  ShowControlBar(&m_wndMLTToolBar,arView[MLT].bVisible,false);
  CFrameWnd::ActivateFrame(nCmdShow); // must be before we muck about with the splitters
  
  CRect rcClient;
  GetClientRect(rcClient);
  for(int j=0;j<sizeof arSplit/sizeof arSplit[0];j++){
    Split &s=arSplit[j];
    if(s.pSplit){
      CString strKey;
      strKey.Format(_T("Split %d"),j);
      const CString strSplit(GetApp()->GetProfileString(_T("Split"),strKey));
      double fSplit=s.fFrac;
      _tscanf(_T("%f"),&fSplit);
      s.pSplit->SetSplitPoint(fSplit);
      s.pSplit->RecalcLayout();
    }
  }

  for(int i=0;i<PaneTypeMax;i++){
    ViewStruct &v=arView[(PaneType)i];
    v.pParent->ShowPane(v.pView,v.bVisible);
  }

  RecalcLayout();
  pDoc->UpdateFailingRuleCount();
  
}

void CMainFrame::OnDestroy() 
{
  
  GetApp()->SaveWindowPlacement (this, _T("Window size"));

  for(int i=0;i<PaneTypeMax;i++){
    LOGFONT lf;
    ViewStruct &v=arView[(PaneType)i];
    v.font.GetLogFont(&lf);
    GetApp()->SaveFont(v.pszName,lf);
  }
  for(int j=0;j<sizeof arSplit/sizeof arSplit[0];j++){
    Split &s=arSplit[j];
    if(s.pSplit){
      CString strSplit;
      strSplit.Format(_T("%f"),s.pSplit->GetSplitPoint());
      CString strKey;
      strKey.Format(_T("%d"),j);
      GetApp()->WriteProfileString(_T("Split"),strKey,strSplit);
    }
  }
  for(j=0;j<sizeof arSplit/sizeof arSplit[0];j++){
    Split &s=arSplit[j];
    if(s.pSplit){
      deleteZ(s.pSplit);
    }
  }
  
  if(m_sp.ProcessAlive()){
    m_sp.Kill();
  }
  
  for(i=0;i<sizeof arView/sizeof arView[0];i++){
    GetApp()->WriteProfileInt(_T("Windows"), arView[i].pszName, arView[i].bVisible);
  }

  CFrameWnd::OnDestroy();
}

void CMainFrame::OnBuildTests() 
{
  Build("tests");
}

void CMainFrame::OnUpdateBuildTests(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::OnUpdateFileSaveAs(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::OnUpdateFileOpen(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::OnUpdateFileNew(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::OnUpdateAppExit(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam) 
{
  CFrameWnd::OnSysCommand(nID, lParam);
}


void CMainFrame::OnUpdateConfigurationRefresh(CCmdUI* pCmdUI) 
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  pCmdUI->Enable(!m_sp.ProcessAlive() && !pDoc->BuildTree().IsEmpty());
}

void CMainFrame::OnConfigurationRefresh() 
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  pDoc->Reload();
}

void CMainFrame::OnViewSettings() 
{
  CViewOptions dlg;
  dlg.DoModal();
}

void CMainFrame::OnHelp() // Help->Configuration Tool Help
{
  CConfigTool::GetConfigToolDoc()->ShowURL(CUtils::LoadString(IDS_CONFIGTOOL_HELP));
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
  CFrameWnd::OnSize(nType, cx, cy);
  if(m_wndStatusBar){
    cx=max(0,cx-m_nFailRulePaneSize-m_nThermometerPaneSize-m_nPercentagePaneSize);
    m_wndStatusBar.SetPaneInfo(StatusPane, 0, SBPS_STRETCH   , 0);	
    m_wndStatusBar.SetPaneInfo(ThermometerPane, 0, SBPS_NORMAL, m_nThermometerPaneSize);	
    m_wndStatusBar.SetPaneInfo(PercentagePane, 0, SBPS_NORMAL, m_nPercentagePaneSize);	
    m_wndStatusBar.SetPaneInfo(FailRulePane, 0, SBPS_NORMAL, m_nFailRulePaneSize);	
    CRect rc;	
    m_wndStatusBar.GetItemRect(ThermometerPane, rc);
    // Reposition the progress control correctly!
    m_Progress.SetWindowPos(&wndTop, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0); 
  }
}

void CMainFrame::OnUpdateConfigurationRepository(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::OnWindowNext() 
{
  BumpWindow(+1);
}

void CMainFrame::OnWindowPrev() 
{
  BumpWindow(-1);
}

void CMainFrame::BumpWindow(int nInc)
{
  CWnd *pWnd=CWnd::GetFocus();
  if(pWnd){
    int nWindows=sizeof arView/sizeof arView[0];
    for(int i=0;i<nWindows;i++){
      CWnd *pThisWnd=arView[i].pView;
      if(pWnd->m_hWnd==pThisWnd->m_hWnd){
        for(int j=((i+nInc+nWindows)%nWindows);j!=i;j=((j+nInc+nWindows)%nWindows)){
          if(arView[j].bVisible){
            arView[j].pView->SetFocus();
            return;
          }
        }
      }
    }
  }
}

void CMainFrame::OnHelpSubmitPr() 
{
  CString strURL;
  if(strURL.LoadString(IDS_ECOS_PR_URL)){
    CConfigTool::GetConfigToolDoc()->ShowURL(strURL);
  }
}

void CMainFrame::OnHelpEcos() 
{
  CString strURL;
  if(strURL.LoadString(IDS_ECOS_SOURCEWARE_URL)){
    CConfigTool::GetConfigToolDoc()->ShowURL(strURL);
  }
}

void CMainFrame::OnHelpEcoshome() 
{
  CString strURL;
  if(strURL.LoadString(IDS_ECOS_HOME_URL)){
    CConfigTool::GetConfigToolDoc()->ShowURL(strURL);
  }
}

void CMainFrame::OnHelpRedHatHome() 
{
  CString strURL;
  if(strURL.LoadString(IDS_RED_HAT_HOME_URL)){
    CConfigTool::GetConfigToolDoc()->ShowURL(strURL);
  }
}

void CMainFrame::OnHelpUitron() 
{
  CString strURL;
  if(strURL.LoadString(IDS_UITRON_HOME_URL)){
    CConfigTool::GetConfigToolDoc()->ShowURL(strURL);
  }
}

void CMainFrame::OnBuildClean() 
{
  Build("clean");
}

void CMainFrame::OnUpdateBuildClean(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::OnToolsShell() 
{
  if(PrepareEnvironment()){
    
    CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
    const CFileName strOldFolder(CFileName::SetCurrentDirectory(pDoc->BuildTree()));
    CUtils::Launch(_T(""),CUtils::LoadString(IDS_SHELL_NAME));
    if(!strOldFolder.IsEmpty()){ // if the current directory was changed
      SetCurrentDirectory(strOldFolder); // restore the previous current directory
    }
  }
}

bool CMainFrame::PrepareEnvironment(bool bWithBuildTools /* = true */)
{
  
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  CConfigToolApp *pApp=(CConfigToolApp *)GetApp();
  ::SetEnvironmentVariable(_T("PATH"),pApp->m_strOriginalPath);

  const CString strPrefix(pDoc->CurrentTargetPrefix());
  CFileName strBinDir;

  bool rc=(! bWithBuildTools) || pApp->m_arstrBinDirs.Lookup(strPrefix,strBinDir);
  if(!rc){
    SendMessage(WM_COMMAND,ID_TOOLS_PATHS,0);
    rc=pApp->m_arstrBinDirs.Lookup(strPrefix,strBinDir);
  }
  
  if(rc){
    
    CFileName strUserBinDir(pApp->m_strUserToolsDir);
    if(strUserBinDir.IsEmpty()){
      if(1==pDoc->m_arstrUserToolPaths.GetSize()){
        pApp->m_strUserToolsDir=pDoc->m_arstrUserToolPaths[0];
      } else {
        SendMessage(WM_COMMAND,ID_USERTOOLS_PATHS,0);
      }
      strUserBinDir=pApp->m_strUserToolsDir;
    }
    if(!strUserBinDir.IsEmpty()){

      // calculate the directory of the host tools from this application's module name
      CFileName strHostToolsBinDir;
      ::GetModuleFileName (::GetModuleHandle (NULL), strHostToolsBinDir.GetBuffer (1 + MAX_PATH), MAX_PATH);
      strHostToolsBinDir.ReleaseBuffer ();
      strHostToolsBinDir = strHostToolsBinDir.Head ();
#ifdef _DEBUG
      strHostToolsBinDir = _T("C:\\Progra~1\\Redhat~1\\eCos");
#endif

      // tools directories are in the order host-tools, user-tools, comp-tools, install/bin (if present), contrib-tools (if present) on the path
      const CFileName strContribBinDir(strUserBinDir,_T("..\\contrib\\bin"));
      CFileName strUsrBinDir(strUserBinDir,_T("..\\usr\\bin"));
      const CFileName strInstallBinDir(pDoc->InstallTree (),_T("bin"));

      // In case strUserBinDir is e.g. c:\program files\red hat\cygwin-00r1\usertools\h-i686-pc-cygwin\bin
      if (!strUsrBinDir.IsDir ())
          strUsrBinDir = CFileName(strUserBinDir + _T("..\\..\\..\\H-i686-pc-cygwin\\bin"));

      if (
        (strUsrBinDir.IsDir ()     && ! CUtils::AddToPath (strUsrBinDir)) || 
        (strContribBinDir.IsDir () && ! CUtils::AddToPath (strContribBinDir)) || 
        (strInstallBinDir.IsDir () && ! CUtils::AddToPath (strInstallBinDir)) || 
        (bWithBuildTools && ! CUtils::AddToPath (strBinDir)) || 
        ! CUtils::AddToPath (strUserBinDir) || 
        ! CUtils::AddToPath (strHostToolsBinDir)) {
        CUtils::MessageBoxF(_T("Failed to set PATH environment variable - rc=%d"),GetLastError());
        rc=false;
      } else {
        if(!SetEnvironmentVariable(_T("MAKE_MODE"),_T("unix"))){
          CUtils::MessageBoxF(_T("Failed to set MAKE_MODE environment variable - rc=%d"),GetLastError());
          rc=false;
        } else {
          SetEnvironmentVariable(_T("GDBTK_LIBRARY"),NULL);
          SetEnvironmentVariable(_T("GCC_EXEC_PREFIX"),NULL);
          SetEnvironmentVariable(_T("ECOS_REPOSITORY"),pDoc->PackagesDir()); // useful for ecosconfig

          if (! pDoc->BuildTree().IsEmpty())
            CygMount(pDoc->BuildTree()[0]);
          if (! pDoc->InstallTree().IsEmpty())
            CygMount(pDoc->InstallTree()[0]);
          if (! pDoc->Repository().IsEmpty())
            CygMount(pDoc->Repository()[0]);

#ifdef _DEBUG
          TCHAR buf[512];
          ::GetEnvironmentVariable(_T("PATH"), buf, 512);
          CConfigTool::Log(buf);
#endif

        }
      }
    }
  }
  return rc;
}


void CMainFrame::OnClose() 
{
  if(m_sp.ProcessAlive()){
    if(IDNO==CUtils::MessageBoxFT(MB_YESNO|MB_DEFBUTTON2,_T("A build is in progress: exit anyway?"))){
      return;
    }
    m_sp.Kill();
  }
  SaveBarState(_T("DockState"));
  CFrameWnd::OnClose();
}

void CMainFrame::OnBuildOptions() 
{
  CBuildOptionsDialog dlg;
  dlg.DoModal();
}

void CMainFrame::OnUpdateBuildOptions(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(TRUE);	
}

void CMainFrame::OnToolsOptions() 
{
  CToolsOptionsDialog dlg;
  dlg.DoModal();
}

void CMainFrame::OnUpdateViewMLT(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(arView[MLT].bVisible);
}

void CMainFrame::OnViewConflicts() 
{
  arView[Rules].bVisible^=1;
  arView[Rules].pParent->ShowPane(arView[Rules].pView,arView[Rules].bVisible);
}

void CMainFrame::OnUpdateViewConflicts(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(arView[Rules].bVisible);
}

void CMainFrame::OnToolsPaths() 
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  CConfigToolApp *pApp=(CConfigToolApp *)GetApp();
  
  CString strDefault;  
  const CString strPrefix(pDoc->CurrentTargetPrefix());

  CStringArray arstrPaths;
  for(POSITION pos = pApp->m_arstrBinDirs.GetStartPosition(); pos != NULL; ){
    CString strP,strB;
    pApp->m_arstrBinDirs.GetNextAssoc(pos, strP, strB);
    
    arstrPaths.Add(strB);
    if(strP==strPrefix){
      strDefault=strB;
    }
  }

  CBinDirDialog dlg(arstrPaths,strDefault);

  dlg.m_strDesc.Format(CUtils::LoadString(IDS_COMPTOOLS_DLG_DESC),strPrefix.IsEmpty()?_T("native"):strPrefix,strPrefix.IsEmpty()?_T(""):strPrefix+_T("-"));  
  dlg.m_strTitle=_T("Build Tools");
  
  if(IDOK==dlg.DoModal()){  
    CFileName strExe;
    strExe.Format(_T("%s\\%s%sgcc.exe"),dlg.m_strFolder,strPrefix,strPrefix.IsEmpty()?_T(""):_T("-"));
    if(strExe.Exists()||
      IDYES==CUtils::MessageBoxFT(MB_YESNO,_T("%s does not appear to contain the build tools - use this folder anyway?"),dlg.m_strFolder)){
      pApp->m_arstrBinDirs.SetAt(strPrefix,dlg.m_strFolder);
    }
  }
}

void CMainFrame::OnUsertoolsPaths() 
{
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  CConfigToolApp * pApp = (CConfigToolApp *) GetApp ();
  
  CStringArray arstrPaths;
  arstrPaths.Copy(pDoc->m_arstrUserToolPaths);
  if(!pApp->m_strUserToolsDir.IsEmpty()){
    arstrPaths.Add(pApp->m_strUserToolsDir);
  }
  CBinDirDialog dlg(arstrPaths, pApp->m_strUserToolsDir);
  
  dlg.m_strDesc.LoadString(IDS_USERTOOLS_DLG_DESC);
  dlg.m_strTitle=_T("User Tools");
  
  if(IDOK==dlg.DoModal()){
    CFileName strFile(dlg.m_strFolder);
    strFile+=_T("ls.exe");
    if(strFile.Exists()||
      IDYES==CUtils::MessageBoxFT(MB_YESNO,_T("%s does not appear to contain the user tools - use this folder anyway?"),dlg.m_strFolder)){
      pApp->m_strUserToolsDir=dlg.m_strFolder;
    }
  }
}

void CMainFrame::UpdateThermometer(int nLines)
{
  if(nLines>=0 && 0!=m_nThermometerMax){	
    int nPercentage=min(100,(100*nLines)/m_nThermometerMax);
    m_Progress.SetPos(nPercentage);
    CString str;
    if(nLines>0){
      str.Format(_T("%d%%"),nPercentage);
    }
    m_wndStatusBar.SetPaneText(PercentagePane,str);
  }
}

void CMainFrame::SetThermometerMax (int nMax)
{
  if(0==nMax){
    UpdateThermometer(0);
  }
  m_nThermometerMax=nMax;
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
  if (AFX_IDS_IDLEMESSAGE==wParam){
    wParam=0;
    lParam=(LPARAM)(LPCTSTR )m_strIdleMessage;
  }
  return CFrameWnd::OnSetMessageString(wParam, lParam);
}

void CMainFrame::SetIdleMessage (LPCTSTR pszIdleMessage)
{
  if(NULL==pszIdleMessage){
    m_strIdleMessage.LoadString(AFX_IDS_IDLEMESSAGE);
  } else {
    m_strIdleMessage=pszIdleMessage;
  }
  SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE, 0);
}	

void CMainFrame::OnRunSim()
{
  CConfigTool::GetConfigToolDoc()->RunTests();
}

bool CMainFrame::SetPaneFont(PaneType pane,const LOGFONT &lf)
{
  CFont &font=arView[pane].font;
  font.Detach();
  font.CreateFontIndirect(&lf);
  switch(pane){
		case Output:
      CConfigTool::GetOutputView()->SendMessage(WM_SETFONT, (WPARAM)(HFONT)font, 0);
      break;
    case ShortDesc:
      CConfigTool::GetDescView()->SendMessage(WM_SETFONT, (WPARAM)(HFONT)font, 0);
      CConfigTool::GetDescView()->Invalidate();
      break;
    case Control:
    case Cell:
      CConfigTool::GetControlView()->SendMessage(WM_SETFONT, (WPARAM)(HFONT)font, 0);
      CConfigTool::GetCellView()->SendMessage(WM_SETFONT, (WPARAM)(HFONT)font, 0);
      CConfigTool::GetControlView()->Invalidate();
      CConfigTool::GetCellView()->Invalidate();
      break;
    case Properties:
      CConfigTool::GetPropertiesView()->SendMessage(WM_SETFONT, (WPARAM)(HFONT)font, 0);
      CConfigTool::GetPropertiesView()->Invalidate();
      break;
    case Rules:
      CConfigTool::GetRulesView()->SendMessage(WM_SETFONT, (WPARAM)(HFONT)font, 0);
      CConfigTool::GetRulesView()->Invalidate();
      break;
    case MLT:
      CConfigTool::GetRulesView()->SendMessage(WM_SETFONT, (WPARAM)(HFONT)font, 0);
      CConfigTool::GetRulesView()->Invalidate();
      break;
    default:
      ASSERT(FALSE);
      break;
  }
  return true;
}

CFont &CMainFrame::GetPaneFont(PaneType pane)
{
  return arView[pane].font;
}

void CMainFrame::OnUpdateRunSim(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!CConfigTool::GetConfigToolDoc()->InstallTree().IsEmpty() && !m_sp.ProcessAlive());
}

void CMainFrame::SetFailRulePane(int nCount)
{
  CString strCount;
  switch (nCount)
  {
		case 0:
      strCount = _T("No conflicts");
      break;
    case 1:
      strCount = _T("1 conflict");
      break;
    default:
      strCount.Format (_T("%d conflicts"), nCount);
      break;
  }
  if(m_bStatusBarCreated){
    m_wndStatusBar.SetPaneText (FailRulePane, strCount);
  }
}

void CMainFrame::OnUpdateFrameTitle (BOOL bAddToTitle)
{
  if ((GetStyle() & FWS_ADDTOTITLE) == 0)
    return;     // leave it alone!
                /*
                #ifndef _AFX_NO_OLE_SUPPORT
                // allow hook to set the title (used for OLE support)
                if (m_pNotifyHook != NULL && m_pNotifyHook->OnUpdateFrameTitle())
                return;
                #endif
  */
  CDocument* pDoc=CConfigTool::GetConfigToolDoc();
  if (bAddToTitle && pDoc != NULL)
    UpdateFrameTitleForDocument(pDoc->GetTitle() + (pDoc->IsModified () ? "*" : ""));
  else
    UpdateFrameTitleForDocument(NULL);
}

void CMainFrame::OnViewMltbar() 
{
  m_bMLTToolbarVisible^=1;
  ShowControlBar(&m_wndMLTToolBar,m_bMLTToolbarVisible,false);
}

void CMainFrame::OnUpdateViewMltbar(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(arView[MLT].bVisible);
  pCmdUI->SetCheck(m_bMLTToolbarVisible);
}

void CMainFrame::OnResolveConflicts() 
{
  if(CConfigToolDoc::NotDone==CConfigTool::GetConfigToolDoc()->ResolveGlobalConflicts()){
    // Global inference handler was never invoked.  Say something
    CUtils::MessageBoxF(_T("No solutions can be automatically determined for the current set of conflicts."));
  }
}

void CMainFrame::OnUpdateResolveConflicts(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(CConfigTool::GetConfigToolDoc()->GetCdlInterpreter()->get_toplevel()->get_all_conflicts().size()>0);
}

void CMainFrame::OnGoHome() // Help->eCos Documentation
{
  CConfigTool::GetConfigToolDoc()->ShowURL(CUtils::LoadString(IDS_ECOS_HELP));
}

HMENU CMainFrame::NewMenu()
{
  // Load the menu from the resources
  m_menu.LoadMenu(IDR_MAINFRAME);  

  m_menu.LoadToolbar(IDR_MAINFRAME);
  m_menu.LoadToolbar(IDR_MISCBAR);

  m_menu.ModifyODMenu(NULL,ID_HELP,IDB_HELP);
  m_menu.ModifyODMenu(NULL,ID_GO_HOME,IDB_HELP);
  m_menu.ModifyODMenu(NULL,ID_HELP_RED_HATONTHEWEB,IDB_HTML);
  m_menu.ModifyODMenu(NULL,ID_HELP_ECOSHOME,IDB_HTML);
  m_menu.ModifyODMenu(NULL,ID_HELP_ECOS,IDB_HTML);
  m_menu.ModifyODMenu(NULL,ID_HELP_UITRON,IDB_HTML);

  return(m_menu.Detach());

}

//This handler ensure that the popup menu items are drawn correctly
void CMainFrame::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
  BOOL setflag=FALSE;
  if(lpMeasureItemStruct->CtlType==ODT_MENU){
    if(IsMenu((HMENU)lpMeasureItemStruct->itemID)){
      CMenu* cmenu=CMenu::FromHandle((HMENU)lpMeasureItemStruct->itemID);
      if(BCMenu::IsMenu(cmenu)){
        m_menu.MeasureItem(lpMeasureItemStruct);
        setflag=TRUE;
      }
    }
  }
  if(!setflag)CFrameWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
 
//This handler ensures that keyboard shortcuts work
LRESULT CMainFrame::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
{
  LRESULT lresult;
  if(BCMenu::IsMenu(pMenu))
    lresult=BCMenu::FindKeyboardShortcut(nChar, nFlags, pMenu);
  else
    lresult=CFrameWnd::OnMenuChar(nChar, nFlags, pMenu);
  return(lresult);
}
 
//This handler updates the menus from time to time
void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
  CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
  if(!bSysMenu){
    if(BCMenu::IsMenu(pPopupMenu))BCMenu::UpdateMenu(pPopupMenu);
  }
}

void CALLBACK CMainFrame::SubprocessOutputFunc(void *pParam,LPCTSTR psz) 
{
  LPTSTR pszCopy=new TCHAR[1+_tcslen(psz)];
  _tcscpy(pszCopy,psz);
  // Post a message to the mainframe because it wouldn't be safe to manipulate controls from a different thread
  ((CMainFrame *)pParam)->PostMessage(WM_SUBPROCESS,(WPARAM)pszCopy);
}

LRESULT CMainFrame::OnSubprocess(WPARAM wParam, LPARAM)
{
  LPTSTR psz=(LPTSTR)wParam;
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  m_nLogicalLines+=pDoc->GetCompilationCount(psz);
  UpdateThermometer (m_nLogicalLines);
  CConfigTool::GetOutputView()->AddText(psz);
  deleteZA(psz);
  return 0;
}

void CMainFrame::OnUpdateToolsAdministration(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_sp.ProcessAlive());
}

void CMainFrame::CygMount(TCHAR c)
{
  // May not be alpha if it's e.g. a UNC network path
  if (!_istalpha(c))
      return;

  c=towlower(c);
  if(!m_arMounted[c-_TCHAR('a')]){
    m_arMounted[c-_TCHAR('a')]=true;
    CString strCmd;
    String strOutput;

    strCmd.Format(_T("mount %c: /ecos-%c"),c,c);
    CSubprocess sub;
    sub.Run(strOutput,strCmd);
  }
}

BOOL CMainFrame::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CFrameWnd::OnHelpInfo(pHelpInfo);
}

void CMainFrame::OnNavComplete(NMHDR* pHdr, LRESULT*)
{
  TRACE(_T("HH Notify\n"));
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
  if(WM_NOTIFY==pMsg->message){
    LPNMHDR pHdr=(LPNMHDR)pMsg->lParam;
    TRACE(_T("WM_NOTIFY id=%d idFrom=%d code=%d\n"),pMsg->wParam,pHdr->idFrom,pHdr->code);
  }
	
	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnEditPlatforms() 
{
  CPlatformsDialog dlg;	
  if(IDOK==dlg.DoModal()){
    CeCosTestPlatform::RemoveAllPlatforms();
    for(unsigned int i=0;i<dlg.PlatformCount();i++){
      CeCosTestPlatform::Add(*dlg.Platform(i));
    }
    CeCosTestPlatform::Save();
  }
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
  if(!m_sp.ProcessAlive()){
    KillTimer(nIDEvent);
  
    TRACE(_T("m_nThermometerMax=%d m_nLogicalLines=%d\n"),m_nThermometerMax,m_nLogicalLines);
    if(0==m_sp.GetExitCode){
      UpdateThermometer(m_nThermometerMax);
      Sleep(250); // Allow user to see it
    }
    UpdateThermometer(0);
    SetThermometerMax(0);
  }
	CFrameWnd::OnTimer(nIDEvent);
}
