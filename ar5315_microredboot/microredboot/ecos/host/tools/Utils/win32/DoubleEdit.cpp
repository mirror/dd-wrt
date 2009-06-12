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
// DoubleEdit.cpp: implementation of the CDoubleEdit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DoubleEdit.h"
#include "CTUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDoubleEdit::CDoubleEdit(double dInitialValue):
  CCellEdit(CUtils::DoubleToStr (dInitialValue))
{
}

CDoubleEdit::~CDoubleEdit()
{

}

BEGIN_MESSAGE_MAP(CDoubleEdit, CCellEdit)
	//{{AFX_MSG_MAP(CDoubleEdit)
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDoubleEdit message handlers

void CDoubleEdit::OnUpdate()
{
    CString strValue;
    GetWindowText (strValue);
    double dValue;
    if (strValue.IsEmpty () || CUtils::StrToDouble (strValue, dValue) || _T("-")==strValue ||
		CUtils::StrToDouble (TrimRightNoCase (strValue, _T("e")), dValue) ||
		CUtils::StrToDouble (TrimRightNoCase (strValue, _T("e+")), dValue) ||
		CUtils::StrToDouble (TrimRightNoCase (strValue, _T("e-")), dValue)) // cell text is legal
	{
        m_strPrevText = strValue;
    }
	else // cell text is not legal so revert to previous text
	{
        MessageBeep (0xFFFFFFFF);
        const CPoint pt (GetCaretPos ());
        SetWindowText (m_strPrevText);
        SetCaretPos (pt);
    }
}

CString CDoubleEdit::TrimRightNoCase (const CString & strInput, LPCTSTR lpszTrimChars)
{
	const CString strTrim = lpszTrimChars;
	if (0 == strInput.Right (strTrim.GetLength ()).CompareNoCase (strTrim))
		return strInput.Left (strInput.GetLength () - strTrim.GetLength ());
	else
		return strInput;
}

BOOL CDoubleEdit::PreCreateWindow(CREATESTRUCT& cs) 
{
    /*
    if(!m_bHex){
        cs.style|=ES_NUMBER;
    }
    */
	return CCellEdit::PreCreateWindow(cs);
}
