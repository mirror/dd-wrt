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
// PkgAdminDlg.cpp : implementation file
//

#include "stdafx.h"

#include "PkgAdmin.h"
#include "PkgAdminDlg.h"
#include "PkgAdminLicenseDlg.h"
#include "PkgAdminTclWaitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPkgadminAboutDlg dialog used for App About

class CPkgadminAboutDlg : public CDialog
{
public:
	CPkgadminAboutDlg();

// Dialog Data
	//{{AFX_DATA(CPkgadminAboutDlg)
	enum { IDD = IDD_PKGADMIN_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPkgadminAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CPkgadminAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CPkgadminAboutDlg::CPkgadminAboutDlg() : CDialog(CPkgadminAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CPkgadminAboutDlg)
	//}}AFX_DATA_INIT
}

void CPkgadminAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPkgadminAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPkgadminAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CPkgadminAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPkgAdminDlg dialog

CPkgAdminDlg::CPkgAdminDlg(LPCTSTR pszRepository,LPCTSTR pszUserTools)
	: CeCosDialog(CPkgAdminDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CPkgAdminDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  if(pszRepository){
	  m_strRepository=pszRepository;
  }
  if(pszUserTools){
	  m_strUserTools=pszUserTools;
  }
	m_CdlPkgData = NULL;
}

void CPkgAdminDlg::DoDataExchange(CDataExchange* pDX)
{
	CeCosDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPkgAdminDlg)
	DDX_Control(pDX, IDC_PKGADMIN_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_PKGADMIN_TREE, m_ctrlPackageTree);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPkgAdminDlg, CeCosDialog)
	//{{AFX_MSG_MAP(CPkgAdminDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PKGADMIN_REMOVE, OnPkgadminRemove)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_PKGADMIN_ADD, OnPkgadminAdd)
	ON_BN_CLICKED(IDCLOSE, OnClose)
	ON_BN_CLICKED(IDC_PKGADMIN_REPOSITORY, OnPkgadminRepository)
	ON_NOTIFY(TVN_SELCHANGED, IDC_PKGADMIN_TREE, OnSelchangedPkgadminTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPkgAdminDlg message handlers

BOOL CPkgAdminDlg::OnInitDialog()
{
	CeCosDialog::OnInitDialog();

  if(this==AfxGetApp()->m_pMainWnd){ // only if the dialog is the application
	  // Add "About..." menu item to system menu.

	  // IDM_ABOUTBOX must be in the system command range.
	  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	  ASSERT(IDM_ABOUTBOX < 0xF000);

	  CMenu* pSysMenu = GetSystemMenu(FALSE);
	  if (pSysMenu != NULL)
	  {
		  CString strAboutMenu;
		  strAboutMenu.LoadString(IDS_ABOUTBOX);
		  if (!strAboutMenu.IsEmpty())
		  {
			  pSysMenu->AppendMenu(MF_SEPARATOR);
			  pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		  }
	  }

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	  SetIcon(m_hIcon, TRUE);			// Set big icon
	
  // The following AppWizard-generated call was causing the 32x32 icon to
	//  be resized for use as a 16x16 icon. Removing the call causes the
	//  correct 16x16 icon to be displayed.
//	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	// setup the repsoitory location

	  if (m_strRepository.IsEmpty()) // if the repository cannot be located
	  {
		  OnPkgadminRepository (); // prompt the user for the repository location
		  if (m_strRepository.IsEmpty ())
		  {
			  PostMessage (WM_COMMAND,IDCANCEL);
			  return TRUE;
		  }
	  }
  } else {
    GetDlgItem(IDC_PKGADMIN_REPOSITORY)->ShowWindow(SW_HIDE);
  }

	// setup the path to the user tools (tar and gunzip)

	if ((! m_strUserTools.IsEmpty()) || FindUserToolsPath ()) // if the user tools can be located
	{
		// add the user tools to the PATH environment variable
		const DWORD nLength = GetEnvironmentVariable (_T("PATH"), NULL, 0) + 1;
		TCHAR * pszOldPath  = new TCHAR [nLength];
		GetEnvironmentVariable (_T("PATH"), pszOldPath, nLength);
		SetEnvironmentVariable (_T("PATH"), CString (pszOldPath) + _T(";") + m_strUserTools);
		delete [] pszOldPath;
	}

	// setup the package tree image list

	m_ilTreeIcons.Create (IDB_PKGADMIN_TREEICONS, 16, 1, RGB (0,128,128));
	m_ctrlPackageTree.SetImageList (&m_ilTreeIcons, TVSIL_NORMAL);

	// populate the package tree

	while (! PopulatePackageTree (m_strRepository))
	{
		m_strRepository = _T("");
		OnPkgadminRepository (); // prompt the user for the repository location
		if (m_strRepository.IsEmpty ()) // if dialog was cancelled
		{
			PostQuitMessage (1);
			return TRUE;
		}
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPkgAdminDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CPkgadminAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CeCosDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPkgAdminDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CeCosDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPkgAdminDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// Trivial handlers; otherwise CdlPackagesDatabaseBody::make asserts.
void CdlErrorHandler (std::string message)
{
};

void CdlWarningHandler (std::string message)
{
};


bool CPkgAdminDlg::PopulatePackageTree(LPCTSTR pszPackagesPath)
{
	// delete any existing CDL database

	if (m_CdlPkgData)
	{
		delete m_CdlPkgData;
		m_CdlPkgData = NULL;
	}

	// load the package database

	try
	{
        // Cdl asserts unless the handlers are present.
#if 1
        m_CdlPkgData = CdlPackagesDatabaseBody::make (UnicodeToStdStr (pszPackagesPath), &CdlErrorHandler, &CdlWarningHandler);
#else
        m_CdlPkgData = CdlPackagesDatabaseBody::make (UnicodeToStdStr (pszPackagesPath));
#endif
	}
	catch (CdlStringException exception)
	{
		CString strMessage;
		strMessage.Format (_T("Error loading database:\n\n%s"), CString (exception.get_message ().c_str ()));
		AfxMessageBox (strMessage);
		return false;
	}
	catch (...)
	{
		AfxMessageBox (_T("Error loading database"));
		return false;
	}

	// clear the old package tree

	ClearPackageTree ();

	// populate the new package tree

	const std::vector<std::string>& packages = m_CdlPkgData->get_packages ();
	for (std::vector<std::string>::const_iterator package = packages.begin (); package != packages.end (); package++)
	{
		// add a package node

		TRACE (_T("Adding package %s:"), CString (package->c_str ()));
		HTREEITEM hPackage = m_ctrlPackageTree.InsertItem (CString (m_CdlPkgData->get_package_aliases (*package) [0].c_str ()));
		m_ctrlPackageTree.SetItemData (hPackage, (DWORD) new CString (package->c_str ()));
		m_ctrlPackageTree.SetItemImage (hPackage, 0, 0);

		const std::vector<std::string>& versions = m_CdlPkgData->get_package_versions (* package);
		for (std::vector<std::string>::const_iterator version = versions.begin (); version != versions.end (); version++)
		{
			// add a version node

			TRACE (_T(" %s"), CString (version->c_str ()));
			const HTREEITEM hVersion = m_ctrlPackageTree.InsertItem (CString (version->c_str ()), hPackage);
			m_ctrlPackageTree.SetItemImage (hVersion, 1, 1);
		}
		TRACE (_T("\n"));
		m_ctrlPackageTree.SortChildren (hPackage); // sort the version nodes
	}

	m_ctrlPackageTree.SortChildren (NULL); // sort the package nodes

  if(this==AfxGetApp()->m_pMainWnd){ // if the dialog is the application
	  // update the caption bar
	  CString strCaption (m_strRepository);
	  strCaption.Replace (_TCHAR('/'), _TCHAR('\\'));
	  strCaption += _T(" - eCos Package Administration Tool");
	  SetWindowText (strCaption);
  }

	return true;
}

void CPkgAdminDlg::OnPkgadminRemove() 
{
	const HTREEITEM hTreeItem = m_ctrlPackageTree.GetSelectedItem ();
	if (! hTreeItem)
		return;

	if (IDYES != CWnd::MessageBox (_T("The selected package will be deleted from the repository. Core eCos packages may be restored only by reinstalling eCos.\n\nDo you wish to continue?"),
		_T("Remove Package"), MB_YESNO | MB_ICONEXCLAMATION))
		return;

	const CString * pstrPackage = (const CString *) m_ctrlPackageTree.GetItemData (hTreeItem);
	if (pstrPackage) // if a package node is selected
	{
		// remove all package version nodes

		bool bStatus = true;
		HTREEITEM hChildItem = m_ctrlPackageTree.GetChildItem (hTreeItem);
		while (hChildItem && bStatus)
		{
			const HTREEITEM hNextChildItem = m_ctrlPackageTree.GetNextSiblingItem (hChildItem);			
			bStatus = RemovePackageVersion (hChildItem);
			hChildItem = hNextChildItem;
		}

		// remove the package node

		if (bStatus)
		{
			delete pstrPackage;
			m_ctrlPackageTree.DeleteItem (hTreeItem);
		}
	}
	else // a version node is selected
	{
		// remove the version node

		const HTREEITEM hParentItem = m_ctrlPackageTree.GetParentItem (hTreeItem);
		ASSERT (hParentItem);
		if (RemovePackageVersion (hTreeItem) && ! m_ctrlPackageTree.ItemHasChildren (hParentItem)) // if the only version was deleted
		{
			// remove the package node

			delete pstrPackage;
			m_ctrlPackageTree.DeleteItem (hParentItem); 
		}
	}
}

void CPkgAdminDlg::ClearPackageTree()
{
	HTREEITEM hPackage = m_ctrlPackageTree.GetRootItem ();
	if (! hPackage) // if no packages in the tree...
		return;     // ...nothing to do

	while (hPackage)
	{
		const HTREEITEM hNextPackage = m_ctrlPackageTree.GetNextSiblingItem (hPackage);
		TRACE (_T("Deleting package %s\n"), * ((CString *) m_ctrlPackageTree.GetItemData (hPackage)));
		delete (CString *) m_ctrlPackageTree.GetItemData (hPackage);
		m_ctrlPackageTree.DeleteItem (hPackage);
		hPackage = hNextPackage;
	}
}

void CPkgAdminDlg::OnDestroy() 
{
	CeCosDialog::OnDestroy();
	
	// free memory allocated to the tree item data CStrings

	ClearPackageTree ();

	// free memory allocated to the CDL database

	if (m_CdlPkgData)
		delete m_CdlPkgData;

}

void CPkgAdminDlg::OnPkgadminAdd() 
{
	CFileDialog dlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, _T("eCos Package Files (*.epk)|*.epk||"), GetParent ());
	TCHAR szBuffer [MAX_PATH * 16] = _T("");
	dlg.m_ofn.lpstrFile = szBuffer;
	dlg.m_ofn.nMaxFile = MAX_PATH * 16;
	dlg.m_ofn.lpstrTitle = _T("Open eCos Package Files");
	if (IDOK == dlg.DoModal ())
	{
		bool bRepositoryChanged = false;
		POSITION posPathName = dlg.GetStartPosition ();
		while (posPathName)
		{
			// get an eCos package distribution file

			CString strPathName = dlg.GetNextPathName (posPathName);

			// extract the licence file

			CString strCommand;
			strCommand.Format (_T("add %s --extract_license"), strPathName);
			strCommand.Replace (_TCHAR('\\'), _TCHAR('/')); // backslashes -> forward slashes for Tcl_EvalFile
			EvalTclFile (3, strCommand);
			CString strLicenseFile = m_strRepository + _T("/pkgadd.txt");
			strLicenseFile.Replace (_TCHAR('/'), _TCHAR('\\')); // forward slashes -> backslashes for Win32

			// read the license file

			CFile fileLicenseFile;
			if (fileLicenseFile.Open (strLicenseFile, CFile::modeRead))
			{
				TRACE (_T("License file found at %s\n"), strLicenseFile);
				const DWORD dwLicenseLength = fileLicenseFile.GetLength ();
				char * pszBuffer = new char [dwLicenseLength + 1]; // allocate a buffer
				fileLicenseFile.Read (pszBuffer, dwLicenseLength);
				fileLicenseFile.Close ();
				CFile::Remove (strLicenseFile); // delete the license file when read
				pszBuffer [dwLicenseLength] = NULL; // terminate the string in the buffer
				CString strLicenseText (pszBuffer); // copy into a CString to convert to Unicode
				delete [] pszBuffer;
				if (-1 == strLicenseText.Find (_T("\r\n"))) // if the file has LF line endings...
					strLicenseText.Replace (_T("\n"), _T("\r\n")); // ... replace with CRLF line endings

				// display the license text

				CPkgAdminLicenseDlg dlgLicense (this);
				dlgLicense.m_strLicense = strLicenseText;
				dlgLicense.SetCaption (strPathName + _T(" - Add Packages"));
				if (IDOK != dlgLicense.DoModal ()) // if license not accepted by user
					continue; // try the next file
			}

			// add the contents of the package distribution file

			strCommand.Format (_T("add %s --accept_license"), strPathName);
			strCommand.Replace (_TCHAR('\\'), _TCHAR('/')); // backslashes -> forward slashes for Tcl_EvalFile
			if (! EvalTclFile (3, strCommand))  // if not successful
				continue; // try the next file

			bRepositoryChanged = true;
		}

		// refresh the package tree only if necessary

		if (bRepositoryChanged && ! PopulatePackageTree (m_strRepository))
			DestroyWindow ();
	}
}

bool CPkgAdminDlg::EvalTclFile(int nArgc, LPCTSTR pszArgv)
{
	CPkgAdminTclWaitDlg dlgWait;

	TRACE (_T("Evaluating ecosadmin.tcl %s\n"), pszArgv);

	// set up the data structure which is passed to the Tcl thread

	CString strArgc;
	strArgc.Format (_T("%d"), nArgc);
	std::string argv0 = UnicodeToStdStr (m_strRepository) + "/ecosadmin.tcl";
	std::string argv = UnicodeToStdStr (pszArgv);
	std::string argc = UnicodeToStdStr (strArgc);
	dlgWait.etsInfo.argv0 = (char *) argv0.c_str ();
	dlgWait.etsInfo.argv = (char *) argv.c_str ();
	dlgWait.etsInfo.argc = (char *) argc.c_str ();

	// display the 'please wait' dialog
	// the Tcl command is invoked from CPkgAdminTclWaitDlg::OnCreate()

	CWaitCursor curWait;
	dlgWait.DoModal ();
	curWait.Restore ();

	// retrieve status information from the data structure

	int nStatus = dlgWait.etsInfo.status;
	CString strErrorMessage (dlgWait.etsInfo.result);

	// report any error

	if (! strErrorMessage.IsEmpty ())
	{
		AfxMessageBox (_T("Command execution error:\n\n") + strErrorMessage);
		return false;
	}
	else if (TCL_OK != nStatus)
	{
		AfxMessageBox (_T("Command execution error"));
		return false;
	}

	return true;
}

bool CPkgAdminDlg::RemovePackageVersion(HTREEITEM hTreeItem)
{
	const HTREEITEM hParentItem = m_ctrlPackageTree.GetParentItem (hTreeItem);
	ASSERT (hParentItem);
	CString * pstrPackage = (CString *) m_ctrlPackageTree.GetItemData (hParentItem);
	ASSERT (pstrPackage);
	CString strCommand;
	strCommand.Format (_T("remove %s --version %s"), * pstrPackage, m_ctrlPackageTree.GetItemText (hTreeItem));
	if (! EvalTclFile (3, strCommand)) // if not successful
		return false;
	
	m_ctrlPackageTree.DeleteItem (hTreeItem); // remove the selected item from the tree
	return true;
}

void CPkgAdminDlg::OnPkgadminRepository() 
{
	CFileDialog dlg (TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, _T("eCos Package Database Files (ecos.db)|ecos.db||"), GetParent ());
	dlg.m_ofn.lpstrTitle = _T("Open eCos Package Database File");

	int nStatus;
	do
	{
		nStatus = dlg.DoModal ();
		if (IDOK == nStatus)
		{
			const CString strPathName = dlg.GetPathName ();
			const int nPathNameIndex = strPathName.ReverseFind (_TCHAR('\\'));
			ASSERT (nPathNameIndex != -1);
			m_strRepository = strPathName.Mid (0, nPathNameIndex);
			m_strRepository.Replace (_TCHAR('\\'), _TCHAR('/'));
		}
	}
	while ((IDOK == nStatus) && ! PopulatePackageTree (m_strRepository));
}

void CPkgAdminDlg::OnSelchangedPkgadminTree(NMHDR*, LRESULT* pResult) 
{
	//NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	// enable the remove button only if a node is selected
	m_btnRemove.EnableWindow (NULL != m_ctrlPackageTree.GetSelectedItem ());

	*pResult = 0;
}

std::string CPkgAdminDlg::UnicodeToStdStr(LPCTSTR str)
{
	int nLength = 1 + _tcslen (str);
	char * pszString = new char [nLength];

	#ifdef _UNICODE
	WideCharToMultiByte (CP_ACP, 0, str, -1, pszString, nLength, NULL, NULL);
	#else
	strcpy (pszString, str);
	#endif

	std::string stdstr = std::string (pszString);
	delete [] pszString;
	return stdstr;
}

bool CPkgAdminDlg::FindUserToolsPath()
{
	HKEY hKey;
	if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_CURRENT_USER, _T("Software\\Red Hat\\eCos\\Configuration Tool\\User Tools"), 0, KEY_READ, &hKey))
		return false;
	
	TCHAR szBuffer [MAX_PATH + 1];
	DWORD dwBufferLength = MAX_PATH + 1;
	LONG lStatus = RegQueryValueEx (hKey, _T("Folder"), NULL, NULL, (LPBYTE) szBuffer, &dwBufferLength);
	RegCloseKey (hKey);
	if (ERROR_SUCCESS != lStatus)
		return false;

	m_strUserTools = szBuffer;
	TRACE (_T("User tools found at %s\n"), m_strUserTools);
	return ! m_strUserTools.IsEmpty ();
}
