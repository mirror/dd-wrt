/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifdef _WIN32

#include "stdafx.h"
#include "Frontend.h"
#include "MyDialog1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif /* _DEBUG */

MyDialog1::MyDialog1(CWnd* pParent)
	: CDialog(MyDialog1::IDD, pParent)
{
	NumLines = 0;

	//{{AFX_DATA_INIT(MyDialog1)
	//}}AFX_DATA_INIT
}

BOOL MyDialog1::Create(CWnd *Parent)
{
	return CDialog::Create(MyDialog1::IDD, Parent);
}

void MyDialog1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MyDialog1)
	DDX_Control(pDX, IDC_BUTTON4, m_SaveButton);
	DDX_Control(pDX, IDC_BUTTON3, m_FreezeButton);
	DDX_Control(pDX, IDC_BUTTON2, m_ContinueButton);
	DDX_Control(pDX, IDC_BUTTON1, m_ClearButton);
	DDX_Control(pDX, IDC_EDIT1, m_OutputWindow);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MyDialog1, CDialog)
	//{{AFX_MSG_MAP(MyDialog1)
	ON_BN_CLICKED(IDC_BUTTON1, OnClearButton)
	ON_BN_CLICKED(IDC_BUTTON2, OnContinueButton)
	ON_BN_CLICKED(IDC_BUTTON3, OnFreezeButton)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BUTTON4, OnSaveButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void MyDialog1::OnOK()
{
}

void MyDialog1::OnCancel()
{
}

void MyDialog1::AddOutputLine(CString Line)
{
	int Index;

	CritSect.Lock();

	Output += Line + "\r\n";

	if (NumLines == 1000)
	{
		Index = Output.Find("\r\n");
		Output.Delete(0, Index + 2);
	}

	else
		NumLines++;

	CritSect.Unlock();

	if (Frozen == 0)
	{
		m_OutputWindow.SetWindowText(Output);
		m_OutputWindow.SetSel(Output.GetLength(), Output.GetLength());
	}
}

void MyDialog1::SetFrozen(int NewFrozen)
{
	Frozen = NewFrozen;
}

BOOL MyDialog1::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CClientDC DevCont(&m_OutputWindow);

	EditFont.CreatePointFont(90, "Courier New", NULL);
	m_OutputWindow.SetFont(&EditFont);

	m_FreezeButton.EnableWindow(FALSE);
	m_ContinueButton.EnableWindow(FALSE);

	Frozen = 0;

	WhiteBrush.CreateSolidBrush(RGB(255, 255, 255));

	return TRUE;
}

void MyDialog1::OnClearButton() 
{
	CritSect.Lock();
	Output.Empty();
	NumLines = 0;
	CritSect.Unlock();

	m_OutputWindow.SetWindowText(Output);
	m_OutputWindow.SetSel(0, 0);
}

void MyDialog1::OnContinueButton()
{
	CString Copy;
	int Len;

	m_FreezeButton.EnableWindow(TRUE);
	m_ContinueButton.EnableWindow(FALSE);

	Frozen = 0;

	CritSect.Lock();
	Copy = Output;
	Len = Output.GetLength();
	CritSect.Unlock();

	m_OutputWindow.SetWindowText(Copy);
	m_OutputWindow.SetSel(Len, Len);
}

void MyDialog1::OnFreezeButton() 
{
	m_FreezeButton.EnableWindow(FALSE);
	m_ContinueButton.EnableWindow(TRUE);

	Frozen = 1;
}

void MyDialog1::HandleStart(void)
{
	CString Copy;
	int Len;

	m_FreezeButton.EnableWindow(TRUE);
	m_ContinueButton.EnableWindow(FALSE);

	Frozen = 0;

	CritSect.Lock();
	Copy = Output;
	Len = Output.GetLength();
	CritSect.Unlock();

	m_OutputWindow.SetWindowText(Copy);
	m_OutputWindow.SetSel(Len, Len);
}

void MyDialog1::HandleStop(void)
{
	CString Copy;
	int Len;

	m_FreezeButton.EnableWindow(FALSE);
	m_ContinueButton.EnableWindow(FALSE);

	Frozen = 0;

	CritSect.Lock();
	Copy = Output;
	Len = Output.GetLength();
	CritSect.Unlock();

	m_OutputWindow.SetWindowText(Copy);
	m_OutputWindow.SetSel(Len, Len);
}

HBRUSH MyDialog1::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd == &m_OutputWindow)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		hbr = WhiteBrush;
	}
	
	return hbr;
}

void MyDialog1::OnSaveButton()
{
	CString Copy;
	int Len;
	CFileDialog FileDialog(FALSE);
	CString FileName = "OLSR log.txt";
	CString PathName;
	CFile File;

	CritSect.Lock();
	Copy = Output;
	Len = Output.GetLength();
	CritSect.Unlock();

	FileDialog.m_ofn.lpstrFilter = "Text file (*.txt)\0*.txt\0";
	FileDialog.m_ofn.nFilterIndex = 1;

	FileDialog.m_ofn.lpstrFile = FileName.GetBuffer(500);
	FileDialog.m_ofn.nMaxFile = 500;

	if (FileDialog.DoModal() == IDOK)
	{
		PathName = FileDialog.GetPathName();

		if (File.Open(PathName, CFile::modeCreate | CFile::modeWrite |
			CFile::shareExclusive) == 0)
			AfxMessageBox("Cannot open logfile '" + PathName + "'.");

		else
		{
			File.Write((const char *)Copy, Len);
			File.Close();
		}
	}

	FileName.ReleaseBuffer();
}

#endif /* _WIN32 */
