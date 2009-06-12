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
#if !defined(AFX_CONNECTIONPAGE_H__44CEA287_11C4_11D3_A505_00A0C949ADAC__INCLUDED_)
#define AFX_CONNECTIONPAGE_H__44CEA287_11C4_11D3_A505_00A0C949ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConnectionPage.h : header file
//
#include "eCosDialog.h"
#include "eCosTest.h"
#include "TestToolRes.h"
/////////////////////////////////////////////////////////////////////////////
// CPropertiesDialog dialog

class CPropertiesDialog : public CeCosDialog
{
//	DECLARE_DYNCREATE(CPropertiesDialog)

// Construction
public:
	CString m_strTarget;
	UINT m_nRemotePort;
	CString m_strRemoteHost;
	bool m_bFarmed;
	CString m_strPort;
	CString m_strReset;
	bool m_bRemote;
	bool m_bSerial;
	int		m_nBaud;
	CString	m_strLocalTCPIPHost;
	UINT	m_nLocalTCPIPPort;
	int		m_nReset;
	CString	m_strResourceHost;
	UINT	m_nResourcePort;

	CPropertiesDialog(bool bHideTarget=false,bool bHideRemoteControls=false);
	~CPropertiesDialog();

    // Dialog Data
	//{{AFX_DATA(CPropertiesDialog)
	UINT	m_nDownloadTimeout;
	UINT	m_nTimeout;
	int		m_nDownloadTimeoutType;
	int		m_nTimeoutType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropertiesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bHideRemoteControls;
	bool m_bHideTarget;
	bool m_bConnectionModified;
	CStringArray m_arstrPorts;
    #ifdef POPUP_PROPERTIES
    bool m_bModified;
    #endif

	void SetButtons();
	// Generated message map functions
	//{{AFX_MSG(CPropertiesDialog)
	afx_msg void OnRadioLocal();
	afx_msg void OnRadioRemote();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePlatform();
	afx_msg void OnSettings();
	virtual void OnCancel();
	afx_msg void OnSelchangeDownloadtimeoutCombo();
	afx_msg void OnSelchangeTimeoutCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTIONPAGE_H__44CEA287_11C4_11D3_A505_00A0C949ADAC__INCLUDED_)
