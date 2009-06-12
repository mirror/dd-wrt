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
#if !defined(AFX_CDLTEMPLATESDIALOG_H__D0F912EA_2D4C_11D3_BFFE_00A0C9554250__INCLUDED_)
#define AFX_CDLTEMPLATESDIALOG_H__D0F912EA_2D4C_11D3_BFFE_00A0C9554250__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CdlTemplatesDialog.h : header file
//

#define INCLUDEFILE <string>
#include "IncludeSTL.h"

/////////////////////////////////////////////////////////////////////////////
// CCdlTemplatesDialog dialog

#include "resource.h"
#include "eCosDialog.h"

class CCdlTemplatesDialog : public CeCosDialog
{
// Construction
public:
	std::string GetSelectedHardware () const { return m_hardware; }
	std::string GetSelectedTemplate () const { return m_template; }
	std::string GetSelectedTemplateVersion () const { return m_template_version; }
	CCdlTemplatesDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCdlTemplatesDialog)
	enum { IDD = IDD_CDL_TEMPLATES };
	CComboBox	m_cboCdlTemplate;
	CComboBox	m_cboCdlTemplateVersion;
	CComboBox	m_cboCdlHardware;
	CEdit	m_edtCdlTemplatePackages;
	CString	m_strCdlHardwareDescription;
	CString	m_strCdlTemplateDescription;
	CString	m_strCdlTemplatePackages;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCdlTemplatesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCdlTemplatesDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCdlHardware();
	afx_msg void OnSelchangeCdlTemplate();
	afx_msg void OnSelchangeCdlTemplateVersion();
	afx_msg void OnDetails();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void UpdateVersionList (std::string default_version);
	void UpdateDetails ();
private:
	void ShowDetails (bool bShow);
	std::string m_hardware;
	std::string m_template;
	std::string m_template_version;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDLTEMPLATESDIALOG_H__D0F912EA_2D4C_11D3_BFFE_00A0C9554250__INCLUDED_)
