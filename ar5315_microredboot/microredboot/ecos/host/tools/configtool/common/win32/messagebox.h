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
//===========================================================================
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/10/06
// Version:		0.01
// Purpose:	
// Description:	This is the interface of the messagebox class
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================
#if !defined(AFX_MessageBox_H__A7F011B1_4736_11D2_B377_444553540000__INCLUDED_)
#define AFX_MessageBox_H__A7F011B1_4736_11D2_B377_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
//
// CMessageBox dialog
// Simon FitzMaurice, October 1998
//
// CMessageBox replicates the functionality of CWnd::MessageBox(), but also
// allows the creation of custom and modeless messageboxes.
//
// Use the 3-parameter constructor and DoModal() to obtain the usual results.
// [Create() may be used to invoke a modeless dialog]
// All flags described in "Message-Box Styles" are supported with the
// exception of MB_TASKMODAL (not useful within an MFC app).
// In addition, two new styles are available: MB_YESNOALL and MB_YESNOALLCANCEL.  These
// add "Yes All" and "No All" buttons to the normal MB_YESNO and MB_YESNOCANCEL,
// respectively (with ids of IDYESALL and IDNOALL).  The MB_DEFBUTTON(n) macro 
// may be used to extend extend the series MB_DEFBUTTON1, MB_DEFBUTTON2, 
// MB_DEFBUTTON3 ...
//
// Use the parameterless constructor to customize the messagebox.  Buttons must
// be added [using AddButton()] and other attributes changed before calling 
// DoModal() or Create().
//
// In the case of Create(), the CMessageBox object must be allocated on the heap;
// it deletes itself when the dialog is dismissed.  The parameters of Create()
// may be used to specify that the parent be sent a notification message when 
// this happens.
//
/////////////////////////////////////////////////////////////////////////////

#define MB_YESNOALL 13
#define MB_YESNOALLCANCEL 14

// Extend the series MB_DEFBUTTON1, MB_DEFBUTTON2, MB_DEFBUTTON3 ...
#define MB_DEFBUTTON(n) ((n<<8)&MB_DEFMASK)

#define IDYESALL 14
#define IDNOALL 15

#include <afxtempl.h>

class CMessageBox : public CDialog
{
	// Construction
public:
	
	// Ctor to build standard messagebox:
	CMessageBox(const CString &strText,const CString &strCaption=_T("Error"),UINT Flag=MB_OK);  // standard constructor
	
	// Use this ctor if using custom buttons:
	CMessageBox();

	virtual  ~CMessageBox();

	// Run the messagebox as a modeless dialog.  If msg is non-zero, post pWnd 
	// msg when the dialog is dismissed.  The loword of wParam of msg will 
	// contain the id of the button clicked and the hiword wParamHigh.  By 
	// specifying msg as WM_COMMAND and wParamHigh as BN_CLICKED (the default 
	// values), ON_BN_CLICKED handlers may be used.
	BOOL Create(CWnd *pWnd=NULL,UINT msg=0,WORD wParamHigh=BN_CLICKED);

	// Set the caption
	void SetCaption (const CString &strCaption);

	// Set static text color (this function does not affect the color of button text)
	void SetTextColor(COLORREF cr){m_crText=cr;}
	
	// Make the messagebox topmost
	void SetTopMost() { m_bTopMost=true; }
	
	// Set the static text
	void SetText (const CString &str){m_strText=str;}

	// This member is public to allow the caller to manipulate it directly
	// (e.g. by means of CString::Format):
	CString m_strText;

	// Set the font (for static text and buttons)
	void SetFont (CFont *pFont){m_pFont=pFont;}

	// Justify text left, center or right
	void SetTextJustification (UINT n) { ASSERT(SS_LEFT==n||SS_CENTER==n||SS_RIGHT==n); m_nJustify=n; }
	
	// Add a button with given caption, id and whether enabled:
	void AddButton (const CString &strCaption,UINT id, bool bEnabled=true);
	
	// Set the nth button as default
	void SetDefaultButton (UINT nIndex);

	// Count of buttons created
	UINT ButtonCount() const { return (UINT)m_arBInfo.GetSize(); }
	
	// Look up a button by its id.  If the button is not found, -1 is returned.
	int IndexOf (UINT id);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessageBox)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bDialogCreated;
	CWnd *m_pParentNotify; // Window to tell about dismissal
	UINT m_nParentNotifcationMessage;	 // Message to send
	WORD m_nParentNotifcationwParamHigh; // hi of wParam
	bool m_bModeless;
	COLORREF m_crText; // Text and button color

	void Init();
	
	// Dialog Data
	typedef struct {
		DLGTEMPLATE tmpl;
		short wMenu[1];
		short wClass[1];
		short wTitle[1];
	} DLGDATA;
	static DLGDATA DlgData;
	
	// Per-button information
	struct CButtonInfo {
		UINT m_id;			
		bool m_bEnabled;		
		CString m_strCaption;
		CButton *m_pWnd;
	public:
		CButtonInfo (UINT id=0,bool bEnabled=false,const CString &strCaption=_T("")):
			m_id(id),
			m_bEnabled(bEnabled),
			m_strCaption(strCaption),
			m_pWnd(NULL)
			{}
	};
	CArray<CButtonInfo,CButtonInfo&> m_arBInfo;

	int m_nEscapeButton;
	int m_nJustify; // SS_LEFT,SS_CENTER or SS_RIGHT
	CButton &Button(int nIndex) const{return *(m_arBInfo[nIndex].m_pWnd);}
	HICON m_hIcon;
	bool m_bTopMost;
	CString m_strCaption;
	int m_nFocusButton;
	UINT m_nDefaultButton;
	
	CStatic * m_pStaticText;
	CStatic * m_pStaticIcon;
	
	CFont   * m_pFont;

	// Generated message map functions
	//{{AFX_MSG(CMessageBox)
	virtual BOOL OnInitDialog();
	afx_msg void OnFontChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg void OnButton(UINT);
	DECLARE_MESSAGE_MAP()
};

int MessageBoxEx (const CString &strText,const CString &strCaption,UINT Flag);

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MessageBox_H__A7F011B1_4736_11D2_B377_444553540000__INCLUDED_)
