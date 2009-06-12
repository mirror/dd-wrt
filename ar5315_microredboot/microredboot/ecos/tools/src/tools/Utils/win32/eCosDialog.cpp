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
// eCosDialog.cpp : implementation file
//

#include "stdafx.h"
#include "eCosDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CeCosDialog dialog

void CeCosDialog::DoDataExchange(CDataExchange* pDX)
{
	CCSHDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CeCosDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CeCosDialog, CCSHDialog)
	//{{AFX_MSG_MAP(CeCosDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CeCosDialog message handlers
  
CPtrArray CeCosDialog::m_arDialogMaps;

CString CeCosDialog::CSHFile() const
{
  return CSHFile((UINT)m_lpszTemplateName);
}

CString CeCosDialog::CSHFile(UINT nID)
{
  if(nID){
    for(int j=m_arDialogMaps.GetSize()-1;j>=0;--j){
      UINT *map=(UINT *)m_arDialogMaps[j];
      for(int i=0;map[i];i+=2){
        if(nID==map[i]){
          CString strFile;
          if(strFile.LoadString(map[i+1])){
            return m_strCSHFilePath+strFile;
          } else {
            TRACE(_T("CCSHDialog::Failed to load help file id %d for dialog id=%d\n"),map[i+1],nID);
          }
        }
      }
    }
  }
  TRACE(_T("CCSHDialog::Failed to find help file for dialog id=%d\n"),nID);
  return _T("");
}

UINT CeCosDialog::HelpID (DWORD dwCtrlID) const
{
  return HelpID((UINT)m_lpszTemplateName,dwCtrlID);
}

UINT CeCosDialog::HelpID (UINT dlgID, DWORD dwCtrlID) 
{
  UNUSED_ALWAYS(dlgID); // because we have a single namespace for all control IDs
  LPCTSTR id;
  switch(dwCtrlID){
    case IDOK:
      id=_T("Closes the dialog and saves any changes you have made.");
      break;
    case IDCANCEL:
      id=_T("Closes the dialog without saving any changes you have made.");
      break;
    case ID_APPLY_NOW:
      id=_T("Saves all the changes you have made without closing this dialog box.");
      break;
    case ID_HELP:
    case IDHELP:
      id=_T("Click this to display an overview of the dialog box.");
      break;
    default:
      id=(LPCTSTR)dwCtrlID;
      break;
  }
  return (UINT)id;
}

HINSTANCE CeCosDialog::GetInstanceHandle()
{
#ifdef PLUGIN
  extern HINSTANCE heCosInstance;
  return heCosInstance;
#else
  return AfxGetInstanceHandle();
#endif
}
