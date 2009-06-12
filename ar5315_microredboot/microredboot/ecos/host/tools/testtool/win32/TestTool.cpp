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
// RunTests.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "CSHDialog.h"
#include "eCosDialog.h"
#include "eCosTest.h"
#include "FileName.h"
#include "TestTool.h"
#include "RunTestsSheet.h"
#include "Properties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRunTestsApp

BEGIN_MESSAGE_MAP(CRunTestsApp, CWinApp)
	//{{AFX_MSG_MAP(CRunTestsApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	//ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRunTestsApp construction

CRunTestsApp::CRunTestsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRunTestsApp object

CRunTestsApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CRunTestsApp initialization

BOOL CRunTestsApp::InitInstance()
{
  CeCosSocket::Init();
  CeCosTestPlatform::Load();
  CFileName strCSHFile;
  ::GetModuleFileName(::GetModuleHandle(NULL),strCSHFile.GetBuffer(1+MAX_PATH),MAX_PATH);
  strCSHFile.ReleaseBuffer();
  strCSHFile.ReplaceExtension(_T(".chm"));
  CCSHDialog::SetCSHFilePath(strCSHFile);

  extern UINT  arTestToolDialogMap[];
  CeCosDialog::AddDialogMap(arTestToolDialogMap);

    /*
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
    */
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
    /*
	CRunTestsDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
    */
    CRunTestsSheet sheet(_T("Run Tests"), NULL, 0, InitFunc);
	m_pMainWnd = &sheet;
    //sheet.m_psh.hIcon=LoadIcon(IDR_MAINFRAME);
    //sheet.m_psh.dwFlags|=PSH_USEHICON/*|PSH_HASHELP*/;
    sheet.DoModal();
	
    // Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
  CeCosSocket::Term();
  CeCosTestPlatform::RemoveAllPlatforms();

	return FALSE;
}

void CALLBACK CRunTestsApp::InitFunc(CProperties *pProp, bool bSave)
{
    static bool bFirstTime=true;
    if(bSave){
        pProp->SaveToRegistry(HKEY_CURRENT_USER,_T("Software\\Red Hat\\eCos\\RunTests"));
    } else {
        pProp->LoadFromCommandString(GetCommandLine());
        pProp->LoadFromRegistry(HKEY_CURRENT_USER,_T("Software\\Red Hat\\eCos\\RunTests"));
    }
    bFirstTime=false;
}
