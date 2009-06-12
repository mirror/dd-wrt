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
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	Implementation of the Properties Dialog
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
#include "CTPropertiesDialog.h"
#include "ConfigItem.h"
#include "ConfigToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CCTPropertiesDialog dialog


CCTPropertiesDialog::CCTPropertiesDialog(CConfigItem &ti)
	: CeCosDialog(IDD, NULL),
	m_ti(ti)
{
	//{{AFX_DATA_INIT(CCTPropertiesDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCTPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCTPropertiesDialog)
	//DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCTPropertiesDialog, CeCosDialog)
	//{{AFX_MSG_MAP(CCTPropertiesDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCTPropertiesDialog message handlers



BOOL CCTPropertiesDialog::OnInitDialog() 
{
  CeCosDialog::OnInitDialog();
  SetWindowText(m_ti.ItemNameOrMacro());
  CRect rect;
  GetDlgItem(IDC_VIEW_PROPERTIES_LIST)->GetWindowRect(rect);
  ScreenToClient(rect);
  m_List.CreateEx(WS_EX_CLIENTEDGE,WC_LISTVIEW,NULL,WS_HSCROLL|WS_VSCROLL|WS_VISIBLE|WS_CHILD,rect,this,IDC_VIEW_PROPERTIES_LIST);
  m_List.Fill(&m_ti);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
