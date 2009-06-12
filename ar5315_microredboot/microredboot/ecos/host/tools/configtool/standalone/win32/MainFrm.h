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
// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	Interface of the main frame class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_MAINFRM_H__A4845244_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_)
#define AFX_MAINFRM_H__A4845244_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
class CControlView;
class CPropertiesView;
class COutputView;
class CFindDialog;
class CMessageBox;
class CCellView;
class CDescView;
class CConfigToolApp;
class CSplitterWndEx;
#include "BCMenu.h"
#include "Subprocess.h"

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
// Attributes
public:
	//CMFWnd m_wndHelp;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow = -1);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation

  enum {Horz=-3, Vert=-2, Thin=-1};
  struct Split {
    int type;
    double fFrac;
    CSplitterWndEx *pSplit;
    int nLevel;
    Split(int _type,int _nLevel,double _fFrac=0.5) : type(_type), nLevel(_nLevel), fFrac(_fFrac), pSplit(0) {}
    ~Split(){};
  };
  static Split arSplit[];
  
public:
  BCMenu m_menu;
	HMENU NewMenu();
	void OnUpdateFrameTitle(BOOL bAddToTitle);
	void SetFailRulePane(int nCount);
	bool m_bFindInProgress;
	void SetThermometerMax (int nMax);
  enum PaneType { Cell, Control, ShortDesc, Output, Properties, Rules, MLT, PaneTypeMax};
  LPCTSTR GetPaneName(PaneType p) { return arView[p].pszName; }
	CFont &GetPaneFont(PaneType pane);
	bool SetPaneFont (PaneType pane, const LOGFONT &lf);

	void SetIdleMessage (LPCTSTR pszIdleMessage=NULL);
	void UpdateThermometer (int nLines);
	bool m_bExiting;
	CConfigToolApp * GetApp();
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CProgressCtrl m_Progress;
	enum {StatusPane, ThermometerPane, PercentagePane, FailRulePane};
	int m_nFailRulePaneSize;
	int m_nThermometerPaneSize;
	int m_nPercentagePaneSize;

	CStatusBar  m_wndStatusBar;
	afx_msg void OnHelp();
	bool PrepareEnvironment(bool bWithBuildTools = true);
	
  // public so available as a utility function for windows creating popups
	LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);

protected:
	static bool m_arMounted[26];
	static void CygMount(TCHAR c);
	int m_nLogicalLines;
	DWORD m_dwThreadId;

  struct ViewStruct {
    LPCTSTR pszName;
    CSplitterWndEx *pParent;
    bool bVisible;
  	CFont font;
    CRuntimeClass *pClass;
    CView *pView;
    ViewStruct(LPCTSTR _pszName,bool _bVisible,CRuntimeClass *_pClass) : pszName(_pszName), pParent(0), bVisible(_bVisible), pClass(_pClass), pView(0) {}
    ViewStruct(const ViewStruct &o) : pszName(o.pszName), pParent(0), bVisible(o.bVisible), pClass(o.pClass), pView(0) {}
  };
  static ViewStruct arView[PaneTypeMax];

	static DWORD WINAPI ThreadFunc( LPVOID ); 

	static UINT indicators[];
	CToolBar    m_wndToolBar;
  CToolBar    m_wndMLTToolBar;
  bool m_bMLTToolbarVisible;
	CFindDialog * m_pFindReplaceDialog;

	bool m_bStatusBarCreated;

	CString m_strIdleMessage;
	int m_nThermometerMax;
	CString m_strBuildTarget;
	void BumpWindow (int nInc);

	CSubprocess m_sp;
	
	void Build(const CString &strWhat=_T(""));
	
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewProperties();
	afx_msg void OnUpdateViewProperties(CCmdUI* pCmdUI);
	afx_msg void OnViewMLT();
	afx_msg void OnViewShortdesc();
	afx_msg void OnUpdateViewShortdesc(CCmdUI* pCmdUI);
	afx_msg void OnViewOutput();
	afx_msg void OnUpdateViewOutput(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBuildConfigure(CCmdUI* pCmdUI);
	afx_msg void OnBuildStop();
	afx_msg void OnUpdateBuildStop(CCmdUI* pCmdUI);
	afx_msg void OnConfigurationBuild();
	afx_msg void OnDestroy();
	afx_msg void OnBuildTests();
	afx_msg void OnUpdateBuildTests(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileNew(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAppExit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMLT(CCmdUI* pCmdUI);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnUpdateConfigurationRefresh(CCmdUI* pCmdUI);
	afx_msg void OnConfigurationRefresh();
	afx_msg void OnViewSettings();
	afx_msg void OnToolsPaths();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateConfigurationRepository(CCmdUI* pCmdUI);
	afx_msg void OnWindowNext();
	afx_msg void OnWindowPrev();
	afx_msg void OnHelpSubmitPr();
	afx_msg void OnHelpEcos();
	afx_msg void OnHelpRedHatHome();
	afx_msg void OnHelpUitron();
	afx_msg void OnHelpEcoshome();
	afx_msg void OnRunSim();
	afx_msg void OnBuildClean();
	afx_msg void OnUpdateBuildClean(CCmdUI* pCmdUI);
	afx_msg void OnToolsShell();
	afx_msg void OnClose();
	afx_msg void OnBuildOptions();
	afx_msg void OnToolsOptions();
	afx_msg void OnUsertoolsPaths();
	afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateRunSim(CCmdUI* pCmdUI);
	afx_msg void OnViewConflicts();
	afx_msg void OnUpdateViewConflicts(CCmdUI* pCmdUI);
	afx_msg void OnViewMltbar();
	afx_msg void OnUpdateViewMltbar(CCmdUI* pCmdUI);
	afx_msg void OnResolveConflicts();
	afx_msg void OnUpdateResolveConflicts(CCmdUI* pCmdUI);
	afx_msg void OnGoHome();
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnUpdateBuildOptions(CCmdUI* pCmdUI);
	afx_msg LRESULT OnSubprocess(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateToolsAdministration(CCmdUI* pCmdUI);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnEditPlatforms();
  afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
  void OnNavComplete(NMHDR*, LRESULT*);
	DECLARE_MESSAGE_MAP()
  static void CALLBACK SubprocessOutputFunc(void *pParam,LPCTSTR psz);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__A4845244_05EE_11D2_80BE_00A0C949ADAC__INCLUDED_)
