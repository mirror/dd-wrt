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
#if !defined(AFX_RUNTESTSSHEET_H__44CEA289_11C4_11D3_A505_00A0C949ADAC__INCLUDED_)
#define AFX_RUNTESTSSHEET_H__44CEA289_11C4_11D3_A505_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RunTestsSheet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRunTestsSheet
#include "stdafx.h"
#include "ExecutionPage.h"
#include "OutputPage.h"
#include "SummaryPage.h"
#include "Properties.h"
#include "eCosTest.h"
#include "eCosPropertySheet.h"

class CTestResource;
class CRunTestsSheet : public CeCosPropertySheet
{
	DECLARE_DYNAMIC(CRunTestsSheet)
    enum Status {Running, Stopping, Stopped};
    Status m_Status;
// Construction
public:

    typedef void (CALLBACK CBFunc)(CProperties*,bool bSave);
	CRunTestsSheet(LPCTSTR pszCaption=_T("Run Tests"), CWnd* pParentWnd = NULL, UINT iSelectPage = 0, CBFunc *pInitFunc=0, CRunTestsSheet **ppSheet=0);

// Attributes
public:
protected:
    CeCosTest::ExecutionParameters m_ep;
    String m_strTarget;
    int m_nTimeout;
    int m_nDownloadTimeout;
    int m_nTimeoutType;
    int m_nDownloadTimeoutType;
    bool m_bRemote;
    bool m_bSerial;
    int m_nBaud;
    String m_strLocalTCPIPHost;
    int m_nLocalTCPIPPort;
    int m_nReset;
    String m_strResourceHost;
    int m_nResourcePort;
    String m_strReset;
    static void CALLBACK RunLocalFunc(void *pParam);
    static void CALLBACK RunRemoteFunc(void *pParam);

// Operations
public:
    void SetTarget(LPCTSTR pszTarget);
    void SetResetNone() { m_nReset = RESET_NONE; };
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRunTestsSheet)
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual int DoModal();
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	CProperties m_prop;
	virtual ~CRunTestsSheet();

    enum AffinityType { BottomRight, TopLeft, TopRight, BottomLeft, Stretch};
	void MoveWindow(CWnd *pWnd,AffinityType Affinity,bool bRepaint=true);
	CRect GetWindowOffset(CWnd *pWnd);
	// Generated message map functions
protected:
	CRunTestsSheet ** m_ppSheet;
	bool m_bModal;
	bool m_bHideRemoteControls;
  static DWORD CALLBACK X10ThreadFunc (void *pParam);
	CString m_strPlacement;
	bool m_bHideTarget;
	UINT m_nRemotePort;
	bool m_bFarmed;
	String m_strPort;
	int m_cyMin;
	int m_cxMin;
	CRect m_rcPrev,m_rcOffset;
	bool m_bAllowResizing;
	CBFunc* m_pInitFunc;
	void * m_InitFuncParam;
	static void CALLBACK TestOutputCallback(void *pParam,LPCTSTR psz);
	CTestResource * m_pResource;
	CRITICAL_SECTION m_CS;
	int m_nTestsToComplete;
	static void CALLBACK RunCallback(void *pParam);
	int m_nNextToSubmit;
	void SubmitTests();
    CExecutionPage  executionpage;
public:
	void Populate (LPCTSTR pszFile,bool bSelect=true);
	void HideRemoteControls();
	String m_strRemoteHost;
private:
    COutputPage outputpage;
    CSummaryPage summarypage;
	//{{AFX_MSG(CRunTestsSheet)
	afx_msg void OnRun();   
	afx_msg void OnProperties();
	afx_msg void OnClose();
    afx_msg LRESULT OnTestOutput(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTestsComplete(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
    afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);	
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RUNTESTSSHEET_H__44CEA289_11C4_11D3_A505_00A0C949ADAC__INCLUDED_)
