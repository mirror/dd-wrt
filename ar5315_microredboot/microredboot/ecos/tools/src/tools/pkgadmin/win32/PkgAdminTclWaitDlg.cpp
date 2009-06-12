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
// PkgAdminTclWaitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PkgAdmin.h"
#include "PkgAdminTclWaitDlg.h"
#include "tcl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPkgAdminTclWaitDlg dialog


CPkgAdminTclWaitDlg::CPkgAdminTclWaitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPkgAdminTclWaitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPkgAdminTclWaitDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPkgAdminTclWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPkgAdminTclWaitDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPkgAdminTclWaitDlg, CDialog)
	//{{AFX_MSG_MAP(CPkgAdminTclWaitDlg)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPkgAdminTclWaitDlg message handlers

// The thread function for executing ecosadmin.tcl
UINT CPkgAdminTclWaitDlg::EvalTclThread (LPVOID pvParam)
{
	// invoke the Tcl command specified in the data structure
	EvalTclStruct * pParam = (EvalTclStruct *) pvParam;
	Tcl_Interp * interp = Tcl_CreateInterp ();
	Tcl_Channel outchan = Tcl_OpenFileChannel (interp, "nul", "a+", 777);
	Tcl_SetStdChannel (outchan, TCL_STDOUT); // direct standard output to NUL:
	char * pszStatus = Tcl_SetVar (interp, "argv0", pParam->argv0, NULL);
	pszStatus = Tcl_SetVar (interp, "argv", pParam->argv, NULL);
	pszStatus = Tcl_SetVar (interp, "argc", pParam->argc, NULL);
	pszStatus = Tcl_SetVar (interp, "gui_mode", "1", NULL); // return errors in result string
	pParam->status = Tcl_EvalFile (interp, pParam->argv0);
	pParam->result = Tcl_GetStringResult (interp);
	Tcl_SetStdChannel (NULL, TCL_STDOUT);
	Tcl_UnregisterChannel (interp, outchan);
	Tcl_DeleteInterp (interp);

	 // close the wait dialog to signal completion
	::PostMessage (pParam->hwnd, WM_CLOSE, 0, 0);
	return 0;
}

int CPkgAdminTclWaitDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	etsInfo.hwnd = m_hWnd; // pass the window handle of the wait dialog into the thread
	AfxBeginThread (EvalTclThread, &etsInfo, 0, 0, 0, NULL);	
	return 0;
}
