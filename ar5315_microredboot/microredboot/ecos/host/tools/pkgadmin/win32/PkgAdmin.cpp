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
// PkgAdmin.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "CSHDialog.h"
#include "eCosDialog.h"
#include "FileName.h"
#include "PkgAdmin.h"
#include "PkgAdminDlg.h"
#include "RegKeyEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPkgAdminApp

BEGIN_MESSAGE_MAP(CPkgAdminApp, CWinApp)
	//{{AFX_MSG_MAP(CPkgAdminApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPkgAdminApp construction

CPkgAdminApp::CPkgAdminApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPkgAdminApp object

CPkgAdminApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPkgAdminApp initialization

BOOL CPkgAdminApp::InitInstance()
{
  CFileName strCSHFile;
  ::GetModuleFileName(::GetModuleHandle(NULL),strCSHFile.GetBuffer(1+MAX_PATH),MAX_PATH);
  strCSHFile.ReleaseBuffer();
  strCSHFile.ReplaceExtension(_T(".chm"));
  CCSHDialog::SetCSHFilePath(strCSHFile);

  extern UINT  arPkgAdminDialogMap[];
  CeCosDialog::AddDialogMap(arPkgAdminDialogMap);

	SetRegistryKey (_T("Red Hat\\eCos"));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
  LPCTSTR pszRepositoryKeyName=_T("Software\\Red Hat\\eCos\\Common\\Repository");
  LPCTSTR pszRepositoryValueName=_T("Folder");

  CFileName strRepository;
  {
    CRegKeyEx k(HKEY_CURRENT_USER,pszRepositoryKeyName, KEY_READ);
    if(k.QueryValue(pszRepositoryValueName, strRepository)){
      if ((strRepository + _T("ecc")).IsDir () && ! (strRepository + _T("packages")).IsDir ()) {
        strRepository += _T("ecc");
      } else {
        strRepository += _T("packages");
      }
    }
  }

  CPkgAdminDlg dlg(strRepository);
  dlg.m_hIcon = LoadIcon(IDR_PKGADMIN_MAINFRAME);
  m_pMainWnd = &dlg;
  
  int nResponse = dlg.DoModal();

  dlg.m_strRepository.Replace(_TCHAR('/'),_TCHAR('\\'));
  int nIndex=dlg.m_strRepository.ReverseFind(_TCHAR('\\'));
  if(-1!=nIndex){
    CRegKeyEx k(HKEY_CURRENT_USER,pszRepositoryKeyName, KEY_WRITE);
    k.SetValue(dlg.m_strRepository.Left(nIndex), pszRepositoryValueName);
  }

  // Since the dialog has been closed, return FALSE so that we exit the
  //  application, rather than start the application's message pump.
  return FALSE;
}

