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
#include "FrontendDlg.h"
#include "TrayIcon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif /* _DEBUG */

BEGIN_MESSAGE_MAP(CFrontendApp, CWinApp)
	//{{AFX_MSG_MAP(CFrontendApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

CFrontendApp::CFrontendApp()
{
}

CFrontendApp theApp;

static int SetEnableRedirKey(unsigned long New)
{
	HKEY Key;
	unsigned long Type;
	unsigned long Len;
	unsigned long Old;
	
	if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
		0, KEY_READ | KEY_WRITE, &Key) != ERROR_SUCCESS)
		return -1;
	
	Len = sizeof (Old);
	
	if (::RegQueryValueEx(Key, "EnableICMPRedirect", NULL, &Type,
		(unsigned char *)&Old, &Len) != ERROR_SUCCESS ||
		Type != REG_DWORD)
		Old = 1;
	
	if (::RegSetValueEx(Key, "EnableICMPRedirect", 0, REG_DWORD,
		(unsigned char *)&New, sizeof (New)))
	{
		::RegCloseKey(Key);
		return -1;
	}
	
	::RegCloseKey(Key);
	return Old;
}

BOOL CFrontendApp::InitInstance()
{
	int Res;

	CCommandLineInfo CmdLineInfo;
	ParseCommandLine(CmdLineInfo);

	CFrontendDlg dlg;

	tray_icon = new TrayIcon( dlg, AfxGetInstanceHandle() );

	dlg.ConfigFile = CmdLineInfo.m_strFileName;

	m_pMainWnd = &dlg;

	Res = SetEnableRedirKey(0);

	if (Res == 1)
	{
		Res = AfxMessageBox("- WARNING -\n\n"
			"The OLSR software has just switched off the processing of incoming ICMP "
			" redirect messages in your registry.\n\n"
			" Please REBOOT your computer for this change to take effect.\n\n"
			" Do you want to allow the OLSR software to reboot your computer now?\n\n"
			" (Please say \"Yes\".)", MB_YESNO | MB_ICONEXCLAMATION);

		if (Res == IDYES)
		{
			HANDLE Proc;
			HMODULE Lib;
			BOOL (*Open)(HANDLE, DWORD, HANDLE *);
			BOOL (*Lookup)(char *, char *, LUID *);
			BOOL (*Adjust)(HANDLE, BOOL, TOKEN_PRIVILEGES *, DWORD,
				TOKEN_PRIVILEGES *, DWORD *);
			HANDLE Token;

			Proc = ::GetCurrentProcess();

			Lib = ::LoadLibrary("advapi32.dll");

			if (Lib != NULL)
			{
				Open = (BOOL (*)(HANDLE, DWORD, HANDLE *))
					::GetProcAddress(Lib, "OpenProcessToken");

				Lookup = (BOOL (*)(char *, char *, LUID *))
					::GetProcAddress(Lib, "LookupPrivilegeValueA");

				Adjust = (BOOL (*)(HANDLE, BOOL, TOKEN_PRIVILEGES *, DWORD,
					TOKEN_PRIVILEGES *, DWORD *))
					::GetProcAddress(Lib, "AdjustTokenPrivileges");

				if (Open != NULL && Lookup != NULL && Adjust != NULL)
				{
					struct
					{
						DWORD Count;
						LUID_AND_ATTRIBUTES Priv;
					}
					TokPriv;

					Proc = ::GetCurrentProcess();

					if (!Open(Proc, TOKEN_ALL_ACCESS, &Token))
						AfxMessageBox("OpenProcessToken() failed.");

					else if (!Lookup("", "SeShutdownPrivilege", &TokPriv.Priv.Luid))
						AfxMessageBox("LookupPrivilegeValue() failed.");

					else
					{
						TokPriv.Count = 1;
						TokPriv.Priv.Attributes = SE_PRIVILEGE_ENABLED;

						if (!Adjust(Token, FALSE, (TOKEN_PRIVILEGES *)&TokPriv,
							0, NULL, NULL))
							AfxMessageBox("AdjustTokenPrivilege() failed.");
					}
				}

				::FreeLibrary(Lib);
			}

			::ExitWindowsEx(EWX_REBOOT, 0);
			::TerminateProcess(Proc, 0);
		}
	}

	RedirectStdHandles();

	dlg.DoModal();

	return FALSE;
}

unsigned int CFrontendApp::RedirectThreadFunc(void)
{
	char Buff[1000];
	int Len;
	int Left, Right;
	CString Line;
	CString Int;

	while (::ReadFile(OutRead, Buff, sizeof (Buff) - 1, (unsigned long *)&Len, NULL))
	{
		if (Len == 0)
			break;

		Left = 0;

		for (Right = 0; Right < Len; Right++)
		{
			if (Buff[Right] != 13)
				Buff[Left++] = Buff[Right];
		}

		Len = Left;

		Left = 0;

		for (Right = 0; Right < Len; Right++)
		{
			if (Buff[Right] == 10)
			{
				Buff[Right] = 0;
				Line += (Buff + Left);

				AfxMessageBox(Line);

				Line.Empty();

				Left = Right + 1;
			}
		}

		Buff[Right] = 0;
		Line += (Buff + Left);
	}

	AfxEndThread(0);
	return 0;
}

static unsigned int RedirectThreadStub(void *Arg)
{
	class CFrontendApp *This;

	This = (class CFrontendApp *)Arg;

	return This->RedirectThreadFunc();
}

struct IoInfo
{
	HANDLE Hand;
	unsigned char Attr;
	char Buff;
#if defined _MT
	int Flag;
	CRITICAL_SECTION Lock;
#endif
};

extern "C" struct IoInfo *__pioinfo[];

int CFrontendApp::RedirectStdHandles(void)
{
	SECURITY_ATTRIBUTES SecAttr;
	HANDLE OutWrite;
	struct IoInfo *Info;

	SecAttr.nLength = sizeof (SECURITY_ATTRIBUTES);
	SecAttr.lpSecurityDescriptor = NULL;
	SecAttr.bInheritHandle = TRUE;

	if (!::CreatePipe(&OutRead, &OutWrite, &SecAttr, 0))
	{
		AfxMessageBox("Cannot create stdout pipe.");
		return -1;
	}

	AfxBeginThread(RedirectThreadStub, (void *)this);

	Info = __pioinfo[0];

	// Info[1].Hand = OutWrite;
	// Info[1].Attr = 0x89; // FOPEN | FTEXT | FPIPE;

	Info[2].Hand = OutWrite;
	Info[2].Attr = 0x89;

	// stdout->_file = 1;
	stderr->_file = 2;

	win32_stdio_hack((unsigned int)OutWrite);

	return 0;
}

#endif /* _WIN32 */
