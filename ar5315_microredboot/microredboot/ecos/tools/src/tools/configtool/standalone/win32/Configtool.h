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
// ConfigTool.h : 
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	main header file for the ConfigTool application
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

#if !defined(AFX_ConfigTool_H__A4845240_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_)
#define AFX_ConfigTool_H__A4845240_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include "stdafx.h"       // main symbols
#include "FileName.h"
#include "FindDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CConfigToolApp:
// See ConfigTool.cpp for the implementation of this class
//

class CConfigToolDoc;
class COutputView;
class CControlView;
class CCellView;
class CDescView;
class CMLTView;
class CPropertiesView;
class CRulesView;
class CMainFrame;

class CConfigTool {
public:
  static void DismissSplash();
  static void SetDocument(CConfigToolDoc *pDoc);
  static void SetMain(CMainFrame * pMain);
  static void SetControlView(CControlView *pControlView);
  static void SetOutputView(COutputView *pOutputView);
  static void SetCellView(CCellView *pCellView);
  static void SetDescView(CDescView *pDescView);
  static void SetMLTView(CMLTView *pMLTView);
  static void SetPropertiesView(CPropertiesView *pPropertiesView);
  static void SetRulesView(CRulesView *pRulesView);
  
  static CConfigToolDoc  * GetConfigToolDoc() { return m_pConfigToolDoc; }
  
  static CControlView    * GetControlView();
  static CCellView       * GetCellView();
  static CDescView       * GetDescView();
  static COutputView     * GetOutputView();
  static CPropertiesView * GetPropertiesView();
  static CRulesView      * GetRulesView();
  static CMLTView        * GetMLTView();
  static CMainFrame      * GetMain();
  
  static int Log(LPCTSTR,...);
  static void CALLBACK OutputWindowTextCB(LPCTSTR pszMsg);

  static CString strHelpFile;

protected:
  static CEditView	   *m_pEditView;
  static COutputView	   *m_pOutputView;
  static CControlView    *m_pControlView;
  static CCellView       *m_pCellView;
  static CDescView       *m_pDescView;
  static CPropertiesView *m_pPropertiesView;
  static CMLTView        *m_pMLTView;
  static CRulesView      *m_pRulesView;
  static CConfigToolDoc  *m_pConfigToolDoc;
  static CMainFrame      *m_pMain;
};

class CConfigToolApp : public CWinApp
{
public:
	CString m_strOriginalPath;
  
  CFileName m_strUserToolsDir;
  void SaveWindowPlacement(CWnd *pWnd, const CString &strKey);
  void SaveFont (const CString &strKey,const LOGFONT &lf);
  void LoadFont (const CString &strKey,LOGFONT &lf);
  BOOL RestoreWindowPlacement(CWnd *pWwnd, const CString &strKey,const CRect &rcDefault);
  CStringArray m_arstrUserToolPaths;
  CString m_strMakeOptions;
  OSVERSIONINFO m_VersionInfo;
  bool Launch (const CString &strFileName,const CString &strViewer=_T(""));
  CPen m_GrayPen;
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  
  CConfigToolApp();
  CString m_strBufferedLogMessages;
  int GetRepositoryRegistryClues (CStringArray &arstrRepository,LPCTSTR pszPrefix);
  CMapStringToString m_arstrBinDirs;
  CString GetInstallVersionKey ();
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CConfigToolApp)
public:
  virtual BOOL InitInstance();
  virtual int ExitInstance();
  virtual BOOL OnIdle(LONG lCount);
  //	virtual void AddToRecentFileList(LPCTSTR lpszPathName);
  //}}AFX_VIRTUAL
  
  // Implementation
  
  //{{AFX_MSG(CConfigToolApp)
  afx_msg void OnAppAbout();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
protected:
  void LoadStdProfileSettings();
private:
  static LPCTSTR s_profileRect;
  static LPCTSTR s_profileIcon;
  static LPCTSTR s_profileMax;
  static LPCTSTR s_profileTool;
  static LPCTSTR s_profileStatus;
  
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ConfigTool_H__A4845240_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_)
