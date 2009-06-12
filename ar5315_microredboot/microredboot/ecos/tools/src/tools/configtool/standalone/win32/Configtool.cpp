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
// ConfigTool.cpp : Defines the class behaviors for the application.
//
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the application class
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

#include "ConfigTool.h"
#include "ConfigToolDoc.h"
#include "ControlView.h"
#include "CSHDialog.h"
#include "CTUtils.h"
#include "eCosDialog.h"
#include "eCosTest.h"
#include "eCosSocket.h"
#include "FileName.h"
#include "MainFrm.h"
#include "OutputView.h"
#include "RegKeyEx.h"
#include "Splash.h"

#include <afxadv.h>
#include <afxdisp.h> // for AfxEnableControlContainer()
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString CConfigTool::strHelpFile;

/////////////////////////////////////////////////////////////////////////////
// CConfigToolApp 

BEGIN_MESSAGE_MAP(CConfigToolApp, CWinApp)
//{{AFX_MSG_MAP(CConfigToolApp)
ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
//}}AFX_MSG_MAP
// Standard file based document commands
ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
// Standard print setup command
ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CConfigToolApp construction
CConfigToolApp::CConfigToolApp()
{
  m_GrayPen.CreatePen(PS_SOLID,1,RGB(192,192,192));	
  m_VersionInfo.dwOSVersionInfoSize=sizeof OSVERSIONINFO;
  ::GetVersionEx(&m_VersionInfo);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CConfigToolApp object

CConfigToolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CConfigToolApp initialization

BOOL CConfigToolApp::InitInstance()
{
  CeCosSocket::Init();
  CeCosTestPlatform::Load();
  CFileName strCSHFile;
  ::GetModuleFileName(::GetModuleHandle(NULL),strCSHFile.GetBuffer(1+MAX_PATH),MAX_PATH);
  strCSHFile.ReleaseBuffer();
  strCSHFile.ReplaceExtension(_T(".chm"));
  CConfigTool::strHelpFile=strCSHFile;
  CCSHDialog::SetCSHFilePath(strCSHFile);
  
  extern UINT  arCommonDialogMap[];
  CeCosDialog::AddDialogMap(arCommonDialogMap);

  extern UINT  arStandaloneDialogMap[];
  CeCosDialog::AddDialogMap(arStandaloneDialogMap);

  extern UINT  arPkgAdminDialogMap[];
  CeCosDialog::AddDialogMap(arPkgAdminDialogMap);

  extern UINT  arTestToolDialogMap[];
  CeCosDialog::AddDialogMap(arTestToolDialogMap);

  // CG: The following block was added by the Splash Screen component.
  \
  {
    \
      CCommandLineInfo cmdInfo;
    \
      ParseCommandLine(cmdInfo);
    \
      
      \
      CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);
    \
  }
  AfxEnableControlContainer();
  // Standard initialization
  // If you are not using these features and wish to reduce the size
  //  of your final executable, you should remove from the following
  //  the specific initialization routines you do not need.
  
	int nSize=GetEnvironmentVariable(_T("PATH"), NULL, 0);
	if(nSize>0){
    GetEnvironmentVariable(_T("PATH"),m_strOriginalPath.GetBuffer(1+nSize),nSize);
    m_strOriginalPath.ReleaseBuffer();
  }

#ifdef _AFXDLL
  Enable3dControls();			// Call this when using MFC in a shared DLL
#else
  Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
  SetRegistryKey(IDS_REGKEY);
  LoadStdProfileSettings();  // Load standard INI file options (including MRU)
  
  // Register the application's document templates.  Document templates
  //  serve as the connection between documents, frame windows and views.
  
  CSingleDocTemplate* pDocTemplate;
  pDocTemplate = new CSingleDocTemplate(
    IDR_MAINFRAME,
    RUNTIME_CLASS(CConfigToolDoc),
    RUNTIME_CLASS(CMainFrame),       // main SDI frame window
    RUNTIME_CLASS(CControlView));
  AddDocTemplate(pDocTemplate);
  
  // Parse command line for standard shell commands, DDE, file open
  CCommandLineInfo cmdInfo;
  //ParseCommandLine(cmdInfo);
  
  CString strCmdLine(GetCommandLine());
  CStringArray arArgs;
  int nWords=CUtils::Chop(strCmdLine,arArgs,_TCHAR(' '),/*bObserveStrings=*/true,/*bBackslashQuotes=*/false);
  bool bBuild=false;
  bool bRun=false;
  CConfigToolDoc *pDoc=CConfigTool::GetConfigToolDoc();
  for(int i=1;i<nWords;i++)
  {
    CString &str=arArgs[i];
    if(0==str.Compare(_T("-R")) && i<nWords-1){
      pDoc->SetRepository(arArgs[++i]);
    } else if (0==str.CompareNoCase(_T("-S")) && i<nWords-1){
      // Load settings file
    } else if (0==str.CompareNoCase(_T("-B"))){
      // Build
      bBuild=true;
    } else if (0==str.CompareNoCase(_T("-R"))){
      // run
      bRun=true;
    } else {
      cmdInfo.m_nShellCommand=CCommandLineInfo::FileOpen;
      cmdInfo.m_strFileName=str;
    }
  }
  
  // Dispatch commands specified on the command line
  if (!ProcessShellCommand(cmdInfo))
    return FALSE;
  
  CMenu* pMenu = m_pMainWnd->GetMenu();
  if (pMenu)pMenu->DestroyMenu();
  HMENU hMenu = ((CMainFrame*) m_pMainWnd)->NewMenu();
  pMenu = CMenu::FromHandle( hMenu );
  m_pMainWnd->SetMenu(pMenu);
  ((CMainFrame*)m_pMainWnd)->m_hMenuDefault = hMenu;


  // The one and only window has been initialized, so show and update it.
  m_pMainWnd->ShowWindow(SW_SHOW);
  m_pMainWnd->UpdateWindow();
  
  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
  CAboutDlg();
  
  // Dialog Data
  //{{AFX_DATA(CAboutDlg)
  enum { IDD = IDD_ABOUTBOX };
  CStatic	m_static;
  //}}AFX_DATA
  
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAboutDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  //{{AFX_MSG(CAboutDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
  //{{AFX_DATA_INIT(CAboutDlg)
  //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAboutDlg)
  DDX_Control(pDX, IDC_STATIC_ABOUT, m_static);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//{{AFX_MSG_MAP(CAboutDlg)
ON_WM_LBUTTONDBLCLK()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CConfigToolApp::OnAppAbout()
{
  CAboutDlg aboutDlg;
  aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CConfigToolApp commands


int CConfigToolApp::ExitInstance() 
{
  // Save persistence info
  WriteProfileString(_T("Build"),	 _T("MakeOptions"),m_strMakeOptions);	
  WriteProfileString(CUtils::LoadString(IDS_KEY_USER_DIR), _T("Folder"), m_strUserToolsDir);
  // Write any target bindirs to the registry
  for(POSITION pos = m_arstrBinDirs.GetStartPosition(); pos != NULL; ){
    CString strPrefix,strBinDir;
    m_arstrBinDirs.GetNextAssoc(pos, strPrefix, strBinDir);
    WriteProfileString(CUtils::LoadString(IDS_KEY_TOOLS_DIR),strPrefix,strBinDir);
  }
  ::DeleteFile(CConfigToolDoc::HTMLHelpLinkFileName());
  CeCosSocket::Term();
  CeCosTestPlatform::RemoveAllPlatforms();
  return CWinApp::ExitInstance();
}

void CConfigToolApp::LoadStdProfileSettings()
{
  CWinApp::LoadStdProfileSettings(4);
  
  CString strNotepad;
  FindExecutable(_T("Notepad.exe"),_T(""),strNotepad.GetBuffer(MAX_PATH));
  strNotepad.ReleaseBuffer(); 
  
  m_strMakeOptions=			  GetProfileString(_T("Build"),	_T("MakeOptions"),_T(""));
  if(m_strMakeOptions.IsEmpty()){
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    m_strMakeOptions.Format(_T("-j%d"),SystemInfo.dwNumberOfProcessors);
  }

  CRegKeyEx InstallKey (HKEY_LOCAL_MACHINE, GetInstallVersionKey (), KEY_READ);

  m_strUserToolsDir=			GetProfileString(CUtils::LoadString(IDS_KEY_USER_DIR), _T("Folder"), _T(""));
  if(!m_strUserToolsDir.IsDir()){ // if the user specified user tools dir does not exist
    InstallKey.QueryValue (_T("Default User Tools Path"), m_strUserToolsDir); // use the installer default
    if (!m_strUserToolsDir.IsDir()) { // if the default user tools dir does not exist
      m_strUserToolsDir=_T(""); // force prompting for the user tools dir
    }
  }
  
  // set default build tools binary directories as specified by the installer
  CFileName strDefaultBuildToolsPath;
  if (InstallKey.QueryValue (_T("Default Build Tools Path"), strDefaultBuildToolsPath)) {
    // look for *-gcc.exe in the default build tools directory
    CFileFind finder;
    BOOL bMore=finder.FindFile (strDefaultBuildToolsPath + _T("*-gcc.exe"));
    while (bMore) { // for each file matching the globbing pattern
      bMore = finder.FindNextFile ();
      CFileName strFile (finder.GetFileName ());
      m_arstrBinDirs.SetAt (strFile.Left (strFile.Find (_T("-gcc"))), strDefaultBuildToolsPath);
    }
  }

  CRegKeyEx k;
  k.Attach(GetSectionKey(CUtils::LoadString(IDS_KEY_TOOLS_DIR)));
  CFileName strDir;
  CString strPrefix;
  for(int i=0;k.QueryValue(i,strPrefix,strDir);i++){
    if(strDir.IsDir()){
      m_arstrBinDirs.SetAt(strPrefix,strDir);
    }
  }

  CStringArray arstrToolChainPaths;
  GetRepositoryRegistryClues(arstrToolChainPaths,_T("GNUPro eCos"));
  for(i=0;i<arstrToolChainPaths.GetSize();i++){
    CFileName strDir(arstrToolChainPaths[i]);
    strDir+="H-i686-cygwin32\\bin";
    if(strDir.IsDir()){
      // This is a potential toolchain location. Look for *-gcc.exe
      CFileFind finder;
      BOOL bMore=finder.FindFile(strDir+"*-gcc.exe");
      while (bMore) {
        bMore = finder.FindNextFile();
        CFileName strFile(finder.GetFileName());
        m_arstrBinDirs.SetAt(strFile.Left(strFile.Find(_T("-gcc"))),strDir);
      }
    }
  }

  // Look for GNUPro 00r1 first, since it's the latest and greatest user tools.   
  GetRepositoryRegistryClues(m_arstrUserToolPaths, _T("GNUPro 00r1"));
  if (m_arstrUserToolPaths.GetSize() == 0)
  {
      GetRepositoryRegistryClues(m_arstrUserToolPaths, _T("Cygwin 00r1"));
  }
  if (m_arstrUserToolPaths.GetSize() > 0)
  {
    for(i=0;i<m_arstrUserToolPaths.GetSize();i++){
        CFileName str(m_arstrUserToolPaths[i]);
        str+="H-i686-pc-cygwin\\bin";
        if(str.IsDir()){
        m_arstrUserToolPaths.SetAt(i,str);
        } else {
        m_arstrUserToolPaths.RemoveAt(i);
        i--;
        }
    }
  }
  else
  {   
    GetRepositoryRegistryClues(m_arstrUserToolPaths, _T("GNUPro unsupported"));
    for(i=0;i<m_arstrUserToolPaths.GetSize();i++){
        CFileName str(m_arstrUserToolPaths[i]);
        str+="H-i686-cygwin32\\bin";
        if(str.IsDir()){
        m_arstrUserToolPaths.SetAt(i,str);
        } else {
        m_arstrUserToolPaths.RemoveAt(i);
        i--;
        }
    }
 }
  
  // Include the path in the set of potential user paths
  {
    CString strPath;
    int nSize=GetEnvironmentVariable(CUtils::LoadString(IDS_PATH), NULL, 0);
    if(nSize>0){
      GetEnvironmentVariable(CUtils::LoadString(IDS_PATH), strPath.GetBuffer(nSize), nSize);
      strPath.ReleaseBuffer();
      CStringArray arstrPath;
      CUtils::Chop(strPath,arstrPath,_TCHAR(';'));
      for(int i=arstrPath.GetSize()-1;i>=0;--i){ // Reverse order is important to treat path correctly
        const CFileName &strFolder=arstrPath[i];
        CFileName strFile(strFolder);
        strFile+=_T("ls.exe");
        if(strFile.Exists()){
          m_arstrUserToolPaths.Add(strFolder);
          if(m_strUserToolsDir.IsEmpty()){
            m_strUserToolsDir=strFolder;
          }
        }
      }
    }
  }
}

BOOL CConfigToolApp::OnIdle(LONG lCount) 
{
  if(lCount==0)
  {
    CMainFrame *pMain=CConfigTool::GetMain();
    if(pMain){
      // During startup the main window will be the splash screen
      CDC *pDC=pMain->GetDC();
      CFont *pOldFont=pDC->SelectObject(pMain->m_wndStatusBar.GetFont());
      pDC->SelectObject(pOldFont);
      pMain->ReleaseDC(pDC);
      
    }
  }
  return CWinApp::OnIdle(lCount);
}

BOOL CConfigToolApp::PreTranslateMessage(MSG* pMsg)
{
  // CG: The following lines were added by the Splash Screen component.
  if (CSplashWnd::PreTranslateAppMessage(pMsg))
    return TRUE;
  
  return CWinApp::PreTranslateMessage(pMsg);
}

BOOL CAboutDlg::OnInitDialog() 
{
  CString strVersion;
  strVersion.Format(_T("%s %s"),_T(__DATE__),_T(__TIME__));
  SetDlgItemText(IDC_STATIC_DATETIME,strVersion);
  CDialog::OnInitDialog();
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

bool CConfigToolApp::Launch(const CString & strFileName,const CString &strViewer)
{
  bool rc=false;
  
  if(!strViewer.IsEmpty())//use custom editor
  {
    CString strCmdline(strViewer);
    
    TCHAR *pszCmdLine=strCmdline.GetBuffer(strCmdline.GetLength());
    GetShortPathName(pszCmdLine,pszCmdLine,strCmdline.GetLength());
    strCmdline.ReleaseBuffer();
    
    strCmdline+=_TCHAR(' ');
    strCmdline+=strFileName;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    
    si.cb = sizeof(STARTUPINFO); 
    si.lpReserved = NULL; 
    si.lpReserved2 = NULL; 
    si.cbReserved2 = 0; 
    si.lpDesktop = NULL; 
    si.dwFlags = 0; 
    si.lpTitle=NULL;
    
    if(CreateProcess(
      NULL, // app name
      //strCmdline.GetBuffer(strCmdline.GetLength()),    // command line
      strCmdline.GetBuffer(strCmdline.GetLength()),    // command line
      NULL, // process security
      NULL, // thread security
      TRUE, // inherit handles
      0,
      NULL, // environment
      NULL, // current dir
      &si, // startup info
      &pi)){
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      rc=true;
    } else {
      CUtils::MessageBoxF(_T("Failed to invoke %s.\n"),strCmdline);
    }
    strCmdline.ReleaseBuffer();
  } else {// Use association
    TCHAR szExe[MAX_PATH];
    HINSTANCE h=FindExecutable(strFileName,_T("."),szExe);
    if(int(h)<=32){
      CString str;
      switch(int(h)){
      case 0:  str=_T("The system is out of memory or resources.");break;
      case 31: str=_T("There is no association for the specified file type.");break;
      case ERROR_FILE_NOT_FOUND: str=_T("The specified file was not found.");break;
      case ERROR_PATH_NOT_FOUND: str=_T("The specified path was not found.");break;
      case ERROR_BAD_FORMAT:     str=_T("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image).");break;
      default: break;
      }
      CUtils::MessageBoxF(_T("Failed to open document %s.\r\n%s"),strFileName,str);
    } else {
      
      SHELLEXECUTEINFO sei = {sizeof(sei), 0, AfxGetMainWnd()->GetSafeHwnd(), _T("open"),
        strFileName, NULL, NULL, SW_SHOWNORMAL, AfxGetInstanceHandle( )};
      
      sei.hInstApp=0;
      HINSTANCE hInst=ShellExecute(AfxGetMainWnd()->GetSafeHwnd(),_T("open"), strFileName, NULL, _T("."), 0)/*ShellExecuteEx(&sei)*/;
      if(int(hInst)<=32/*sei.hInstApp==0*/)
      {
        CString str;
        switch(int(hInst))
        {
        case 0 : str=_T("The operating system is out of memory or resources. ");break;
        case ERROR_FILE_NOT_FOUND : str=_T("The specified file was not found. ");break;
        case ERROR_PATH_NOT_FOUND : str=_T("The specified path was not found. ");break;
        case ERROR_BAD_FORMAT : str=_T("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image). ");break;
        case SE_ERR_ACCESSDENIED : str=_T("The operating system denied access to the specified file. ");break;
        case SE_ERR_ASSOCINCOMPLETE : str=_T("The filename association is incomplete or invalid. ");break;
        case SE_ERR_DDEBUSY : str=_T("The DDE transaction could not be completed because other DDE transactions were being processed. ");break;
        case SE_ERR_DDEFAIL : str=_T("The DDE transaction failed. ");break;
        case SE_ERR_DDETIMEOUT : str=_T("The DDE transaction could not be completed because the request timed out. ");break;
        case SE_ERR_DLLNOTFOUND : str=_T("The specified dynamic-link library was not found. ");break;
          //case SE_ERR_FNF : str=_T("The specified file was not found. ");break;
        case SE_ERR_NOASSOC : str=_T("There is no application associated with the given filename extension. ");break;
        case SE_ERR_OOM : str=_T("There was not enough memory to complete the operation. ");break;
          //case SE_ERR_PNF : str=_T("The specified path was not found. ");break;
        case SE_ERR_SHARE : str=_T("A sharing violation occurred. ");break;
        default: str=_T("An unexpected error occurred");break;
        }
        CUtils::MessageBoxF(_T("Failed to open document %s using %s.\r\n%s"),strFileName,szExe,str);
      } else {
        rc=true;
      }
    }
  }
  return rc;
}

/*
void CConfigToolApp::AddToRecentFileList(LPCTSTR lpszPathName) 
{
const CFileName strDir=CFileName(lpszPathName).Head();
if(strDir.IsDir()){
CWinApp::AddToRecentFileList(strDir);
}
}
*/

int CConfigToolApp::GetRepositoryRegistryClues(CStringArray &arstr,LPCTSTR pszPrefix)
{
  // Go looking for potential candidates in SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
  arstr.RemoveAll();	
  CRegKeyEx k1(HKEY_LOCAL_MACHINE,_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths"),KEY_READ);
  CString strKey;
  for(int i=0;k1.QueryKey(i,strKey);i++){
    if(0==strKey.Find(pszPrefix)){
      CRegKeyEx k2((HKEY)k1,strKey,KEY_READ);
      CFileName strDir;
      if(k2.QueryValue(_T("Path"),strDir) && strDir.IsDir()){
        arstr.Add(strDir);
      }
    }
  }   
  return arstr.GetSize();
}


void CAboutDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
  UNUSED_ALWAYS(point);
  UNUSED_ALWAYS(nFlags);
}

//const char CConfigToolApp::s_profileFlags[] = _T("flags");
LPCTSTR  CConfigToolApp::s_profileMax = _T("max");
LPCTSTR  CConfigToolApp::s_profileTool = _T("tool");
LPCTSTR  CConfigToolApp::s_profileStatus = _T("status");
LPCTSTR  CConfigToolApp::s_profileRect = _T("Rect");

void CConfigToolApp::SaveWindowPlacement(CWnd *pWnd, const CString &strKey)
{
  // Our position
  CString strText;
  
  WINDOWPLACEMENT wndpl;
  wndpl.length = sizeof(WINDOWPLACEMENT);
  // gets current window position and
  //  iconized/maximized status
  pWnd->GetWindowPlacement(&wndpl);
  strText.Format(_T("%04d %04d %04d %04d"),
    wndpl.rcNormalPosition.left,
    wndpl.rcNormalPosition.top,
    wndpl.rcNormalPosition.right,
    wndpl.rcNormalPosition.bottom);
  if(!pWnd->IsWindowVisible()){
    wndpl.showCmd=SW_HIDE;
  }
  WriteProfileInt   (strKey,_T("Show"), wndpl.showCmd);
  WriteProfileString(strKey,s_profileRect, strText);
}

BOOL CConfigToolApp::RestoreWindowPlacement(CWnd *pWnd,const CString &strKey,const CRect &rcDefault)
{
  // Set the windows according to registry settings
  CString strText;
  WINDOWPLACEMENT wndpl;
  CRect rect;
  
  strText = GetProfileString(strKey,s_profileRect);
  if (!strText.IsEmpty()) {
    rect.left = _ttoi((LPCTSTR ) strText);
    rect.top =  _ttoi((LPCTSTR ) strText + 5);
    rect.right = _ttoi((LPCTSTR ) strText + 10);
    rect.bottom = _ttoi((LPCTSTR ) strText + 15);
  } else {
    rect = rcDefault;
  }
  
  CRect rcMax;
  SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)(RECT *)rcMax, 0);
  
  if(rect.Width()<100 || rect.Height()<100 || rect.Width()>rcMax.Width() || rect.Height()>rcMax.Height()){
    rect=rcDefault;
  }
  
  wndpl.length = sizeof(WINDOWPLACEMENT);
  wndpl.showCmd = GetProfileInt(strKey,_T("Show"),SW_SHOWNA);
  wndpl.flags = 0;
  
  wndpl.ptMinPosition = CPoint(0, 0);
  wndpl.ptMaxPosition =CPoint(-::GetSystemMetrics(SM_CXBORDER),-::GetSystemMetrics(SM_CYBORDER));
  wndpl.rcNormalPosition = rect;
  
  // sets window's position and iconized/maximized status
  return pWnd->SetWindowPlacement(&wndpl);
}

void CConfigToolApp::LoadFont(const CString & strKey, LOGFONT & lf)
{
  lf.lfHeight			=(LONG)GetProfileInt(strKey,_T("Height"),0);
  if(0==lf.lfHeight){
    CFont font;
    font.Attach(HFONT(GetStockObject(DEFAULT_GUI_FONT)));
    font.GetLogFont(&lf);
  } else {
    lf.lfWidth			=(LONG)GetProfileInt(strKey,_T("Width"),0);
    lf.lfEscapement		=(LONG)GetProfileInt(strKey,_T("Escapement"),0);
    lf.lfOrientation	=(LONG)GetProfileInt(strKey,_T("Orientation"),0);
    lf.lfWeight			=(LONG)GetProfileInt(strKey,_T("Weight"),0);
    lf.lfItalic			=(BYTE)GetProfileInt(strKey,_T("Italic"),0);
    lf.lfUnderline		=(BYTE)GetProfileInt(strKey,_T("Underline"),0);
    lf.lfStrikeOut		=(BYTE)GetProfileInt(strKey,_T("StrikeOut"),0);
    lf.lfCharSet		=(BYTE)GetProfileInt(strKey,_T("CharSet"),0);
    lf.lfOutPrecision	=(BYTE)GetProfileInt(strKey,_T("OutPrecision"),0);
    lf.lfClipPrecision	=(BYTE)GetProfileInt(strKey,_T("ClipPrecision"),0); 
    lf.lfQuality		=(BYTE)GetProfileInt(strKey,_T("Quality"),0);
    lf.lfPitchAndFamily	=(BYTE)GetProfileInt(strKey,_T("PitchAndFamily"),0);
    CString strFaceName =GetProfileString(strKey,_T("FaceName"),_T(""));
    if(strFaceName.GetLength()<=31){
      _tcscpy(lf.lfFaceName,strFaceName);
    } else {
      lf.lfFaceName[0]=_TCHAR('\0');
    }
  }
}

void CConfigToolApp::SaveFont(const CString & strKey, const LOGFONT & lf)
{
  WriteProfileInt(strKey,_T("Height"),		(int)lf.lfHeight);
  WriteProfileInt(strKey,_T("Width"),			(int)lf.lfWidth);
  WriteProfileInt(strKey,_T("Escapement"),	(int)lf.lfEscapement);
  WriteProfileInt(strKey,_T("Orientation"),	(int)lf.lfOrientation);
  WriteProfileInt(strKey,_T("Weight"),		(int)lf.lfWeight);
  WriteProfileInt(strKey,_T("Italic"),		(int)lf.lfItalic);			
  WriteProfileInt(strKey,_T("Underline"),		(int)lf.lfUnderline);
  WriteProfileInt(strKey,_T("StrikeOut"),		(int)lf.lfStrikeOut);	
  WriteProfileInt(strKey,_T("CharSet"),		(int)lf.lfCharSet);			
  WriteProfileInt(strKey,_T("OutPrecision"),	(int)lf.lfOutPrecision);
  WriteProfileInt(strKey,_T("ClipPrecision"),	(int)lf.lfClipPrecision);
  WriteProfileInt(strKey,_T("Quality"),		(int)lf.lfQuality);	
  WriteProfileInt(strKey,_T("PitchAndFamily"),(int)lf.lfPitchAndFamily);
  WriteProfileString(strKey,_T("FaceName"),   lf.lfFaceName);
}

CEditView	      *CConfigTool::m_pEditView=NULL;
COutputView	    *CConfigTool::m_pOutputView=NULL;
CControlView    *CConfigTool::m_pControlView=NULL;
CCellView       *CConfigTool::m_pCellView=NULL;
CDescView       *CConfigTool::m_pDescView=NULL;
CPropertiesView *CConfigTool::m_pPropertiesView=NULL;
CRulesView      *CConfigTool::m_pRulesView=NULL;
CConfigToolDoc  *CConfigTool::m_pConfigToolDoc=NULL;
CMainFrame      *CConfigTool::m_pMain=NULL;
CMLTView        *CConfigTool::m_pMLTView=NULL;

CControlView    *CConfigTool::GetControlView()      { return m_pControlView; }
COutputView     *CConfigTool::GetOutputView()       { return m_pOutputView; }
CCellView       *CConfigTool::GetCellView()         { return m_pCellView; }
CDescView       *CConfigTool::GetDescView()         { return m_pDescView; }
CPropertiesView *CConfigTool::GetPropertiesView()   { return m_pPropertiesView; }
CRulesView      *CConfigTool::GetRulesView()        { return m_pRulesView; }
CMLTView        *CConfigTool::GetMLTView()          { return m_pMLTView; }

void CConfigTool::SetDocument(CConfigToolDoc *pDoc) {m_pConfigToolDoc=pDoc; }
void CConfigTool::SetMain(CMainFrame * pMain) { m_pMain=pMain; }
void CConfigTool::SetControlView(CControlView *pControlView) {m_pControlView=pControlView; }
void CConfigTool::SetOutputView(COutputView *pOutputView) {m_pOutputView=pOutputView; }
void CConfigTool::SetCellView(CCellView *pCellView) {m_pCellView=pCellView; }
void CConfigTool::SetDescView(CDescView *pDescView) {m_pDescView=pDescView; }
void CConfigTool::SetMLTView(CMLTView *pMLTView) {m_pMLTView=pMLTView; }
void CConfigTool::SetPropertiesView(CPropertiesView *pPropertiesView) {m_pPropertiesView=pPropertiesView; }
void CConfigTool::SetRulesView(CRulesView *pRulesView) {m_pRulesView=pRulesView; }
CMainFrame *CConfigTool::GetMain()
{
  CWnd *pWnd=AfxGetMainWnd();
  return (pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)))?(CMainFrame*)pWnd:NULL;
}

int CConfigTool::Log(LPCTSTR pszFormat, ...)
{
  va_list marker;
  va_start (marker, pszFormat);
  
  for(int nLength=100;nLength;) {
    TCHAR *buf=new TCHAR[1+nLength];
    int n=_vsntprintf(buf, nLength, pszFormat, marker ); 
    if(-1==n){
      nLength*=2;  // NT behavior
    } else if (n<nLength){
      CWnd *pMain=CConfigTool::GetMain();
      if(pMain){
        // During startup the main window will be the splash screen
        COutputView *pView=CConfigTool::GetOutputView();
        if(pView){
          pView->AddText(buf);
          pView->AddText(_T("\r\n"));
        }
      }
      ((CConfigToolApp *)AfxGetApp())->m_strBufferedLogMessages+=buf;
      ((CConfigToolApp *)AfxGetApp())->m_strBufferedLogMessages+=_T("\r\n");
      nLength=0;   // trigger exit from loop
    } else {
      nLength=n+1; // UNIX behavior generally, or NT behavior when buffer size exactly matches required length
    }
    delete [] buf;
  }
  
  va_end (marker);
  
  return 0;
}

void CConfigTool::DismissSplash()
{
  CSplashWnd::EnableSplashScreen(FALSE);
}

CString CConfigToolApp::GetInstallVersionKey ()
{
  CString strKey = _T("SOFTWARE\\Red Hat\\eCos");
  CString strVersionKey = _T("");
  CString rc = _T("");
  TCHAR pszBuffer [MAX_PATH + 1];
  HKEY hKey;
  
  // get the greatest eCos version subkey
  if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, strKey, 0L, KEY_READ, &hKey)) {
    DWORD dwIndex = 0;
    while (ERROR_SUCCESS == RegEnumKey (hKey, dwIndex++, (LPTSTR) pszBuffer, sizeof (pszBuffer))) {
      if (strVersionKey.Compare (pszBuffer) < 0) {
        strVersionKey = pszBuffer;
      }
    }
    RegCloseKey (hKey);
  }
  return strKey + _T("\\") + strVersionKey;
}
