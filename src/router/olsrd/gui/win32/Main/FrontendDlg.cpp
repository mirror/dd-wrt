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

#include "Ipc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif /* _DEBUG */

CFrontendDlg::CFrontendDlg(CWnd* pParent)
	: CDialog(CFrontendDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFrontendDlg)
	//}}AFX_DATA_INIT

	Event = CreateEvent(NULL, FALSE, FALSE, "TheOlsrdShimEvent");

	LogThread = NULL;
	NetThread = NULL;
}

void CFrontendDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFrontendDlg)
	DDX_Control(pDX, IDC_BUTTON2, m_StopButton);
	DDX_Control(pDX, IDC_BUTTON1, m_StartButton);
	DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFrontendDlg, CDialog)
	//{{AFX_MSG_MAP(CFrontendDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnStartButton)
	ON_BN_CLICKED(IDC_BUTTON2, OnStopButton)
	ON_BN_CLICKED(IDC_BUTTON3, OnExitButton)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#if 0
static void HexDump(unsigned char *Mem, int Len)
{
	char Buff[10000];
	int i, k;
	char *Walker = Buff;

	for (i = 0; i < Len; i += k)
	{
		Walker += sprintf(Walker, "%08x:", i);

		for (k = 0; i + k < Len && k < 16; k++)
			Walker += sprintf(Walker, " %02x", Mem[i + k]);

		while (k < 16)
		{
			Walker += sprintf(Walker, "   ");
			k++;
		}

		Walker += sprintf(Walker, " ");

		for (k = 0; i + k < Len && k < 16; k++)
			if (Mem[i + k] < 32 || Mem[i + k] > 126)
				Walker += sprintf(Walker, ".");

			else
				Walker += sprintf(Walker, "%c", Mem[i + k]);

		Walker += sprintf(Walker, "\r\n");
	}

	::MessageBox(NULL, Buff, "HEXDUMP", MB_OK);
}
#endif

// XXX - pretty inefficient

void CFrontendDlg::Timeout(void)
{
	POSITION Pos, Pos2;
	class NodeEntry Entry;
	class MprEntry MprEntry;
	class MidEntry MidEntry;
	class HnaEntry HnaEntry;

Restart0:
	Pos = NodeList.GetHeadPosition();

	while (Pos != NULL)
	{
		Entry = NodeList.GetAt(Pos);

		if (Entry.Timeout < Now)
		{
			NodeList.RemoveAt(Pos);
			goto Restart0;
		}

Restart1:
		Pos2 = Entry.MprList.GetHeadPosition();

		while (Pos2 != NULL)
		{
			MprEntry = Entry.MprList.GetAt(Pos2);

			if (MprEntry.Timeout < Now)
			{
				Entry.MprList.RemoveAt(Pos2);
				goto Restart1;
			}

			Entry.MprList.GetNext(Pos2);
		}

Restart2:
		Pos2 = Entry.MidList.GetHeadPosition();

		while (Pos2 != NULL)
		{
			MidEntry = Entry.MidList.GetAt(Pos2);

			if (MidEntry.Timeout < Now)
			{
				Entry.MidList.RemoveAt(Pos2);
				goto Restart2;
			}

			Entry.MidList.GetNext(Pos2);
		}

Restart3:
		Pos2 = Entry.HnaList.GetHeadPosition();

		while (Pos2 != NULL)
		{
			HnaEntry = Entry.HnaList.GetAt(Pos2);

			if (HnaEntry.Timeout < Now)
			{
				Entry.HnaList.RemoveAt(Pos2);
				goto Restart3;
			}

			Entry.HnaList.GetNext(Pos2);
		}

		NodeList.GetNext(Pos);
	}

	if( NodeList.IsEmpty() )
		TrayIcon::getInstance()->setStatus( TrayIcon::ON, "No nodes found" );
	else
		TrayIcon::getInstance()->setStatus( TrayIcon::CONNECTED, "Nodes available" );

	m_TabCtrl.m_Dialog3.UpdateNodeInfo(NodeList);
}

unsigned int VTimeToInt(unsigned int VTime)
{
	return ((0x10 | ((VTime & 0xf0) >> 4)) << (VTime & 0x0f)) >> 8;
}

void CFrontendDlg::AddMpr(unsigned int MprAddr, unsigned int NodeAddr,
						  unsigned int VTime)
{
	class NodeEntry NewEntry;
	POSITION Pos;
	unsigned __int64 Timeout;

	Timeout = Now +
		(unsigned __int64)VTimeToInt(VTime) * (unsigned __int64)10000000;

	AddNode(NodeAddr, VTime);
	AddNode(MprAddr, VTime);

	NewEntry.Addr = NodeAddr;

	Pos = NodeList.Find(NewEntry);

	if (Pos == NULL)
		return;

	class NodeEntry &OldEntry = NodeList.GetAt(Pos);

	OldEntry.AddMpr(MprAddr, Timeout);

	m_TabCtrl.m_Dialog3.UpdateNodeInfo(NodeList);
}

void CFrontendDlg::AddMid(unsigned int IntAddr, unsigned int NodeAddr,
						  unsigned int VTime)
{
	class NodeEntry NewEntry;
	POSITION Pos;
	unsigned __int64 Timeout;

	Timeout = Now +
		(unsigned __int64)VTimeToInt(VTime) * (unsigned __int64)10000000;

	AddNode(NodeAddr, VTime);

	NewEntry.Addr = NodeAddr;

	Pos = NodeList.Find(NewEntry);

	if (Pos == NULL)
		return;

	class NodeEntry &OldEntry = NodeList.GetAt(Pos);

	OldEntry.AddMid(IntAddr, Timeout);

	m_TabCtrl.m_Dialog3.UpdateNodeInfo(NodeList);
}

void CFrontendDlg::AddHna(unsigned int NetAddr, unsigned int NetMask,
						  unsigned int NodeAddr, unsigned int VTime)
{
	class NodeEntry NewEntry;
	POSITION Pos;
	unsigned __int64 Timeout;

	Timeout = Now +
		(unsigned __int64)VTimeToInt(VTime) * (unsigned __int64)10000000;

	AddNode(NodeAddr, VTime);

	NewEntry.Addr = NodeAddr;

	Pos = NodeList.Find(NewEntry);

	if (Pos == NULL)
		return;

	class NodeEntry &OldEntry = NodeList.GetAt(Pos);

	OldEntry.AddHna(NetAddr, NetMask, Timeout);

	m_TabCtrl.m_Dialog3.UpdateNodeInfo(NodeList);
}

void CFrontendDlg::AddNode(unsigned int NodeAddr, unsigned int VTime)
{
	class NodeEntry NewEntry;
	POSITION Pos;
	unsigned __int64 Timeout;

	if (NodeAddr == LocalHost)
		return;

	Timeout = Now +
		(unsigned __int64)VTimeToInt(VTime) * (unsigned __int64)10000000;

	NewEntry.Addr = NodeAddr;

	Pos = NodeList.Find(NewEntry);

	if (Pos != NULL)
	{
		class NodeEntry &OldEntry = NodeList.GetAt(Pos);
		OldEntry.Timeout = Timeout;
	}

	else
	{
		NewEntry.Timeout = Timeout;
		NodeList.AddTail(NewEntry);
	}

	m_TabCtrl.m_Dialog3.UpdateNodeInfo(NodeList);
}

void CFrontendDlg::HandleOlsrTc(struct OlsrTc *Msg, int UseLq)
{
	int Size;
	unsigned int *Addr;

	Msg->Header.SeqNo = ::ntohs(Msg->Header.SeqNo);
	Msg->Ansn = ::ntohs(Msg->Ansn);

	AddNode(Msg->Header.Orig, Msg->Header.VTime);

	Size = Msg->Header.Size;

	Size -= sizeof (struct OlsrTc);

	Addr = (unsigned int *)(Msg + 1);

	while (Size > 0)
	{
		Size -= 4;

		AddMpr(*Addr, Msg->Header.Orig, Msg->Header.VTime);

		Addr++;

		if (UseLq != 0)
		{
			Size -= 4;
			Addr++;
		}

	}
}

void CFrontendDlg::HandleOlsrMid(struct OlsrHeader *Msg)
{
	int Size;
	unsigned int *Addr;

	Msg->SeqNo = ::ntohs(Msg->SeqNo);

	AddNode(Msg->Orig, Msg->VTime);

	Size = Msg->Size;

	Size -= sizeof (struct OlsrHeader);

	Addr = (unsigned int *)(Msg + 1);

	while (Size > 0)
	{
		Size -= 4;

		AddMid(*Addr, Msg->Orig, Msg->VTime);

		Addr++;
	}
}

void CFrontendDlg::HandleOlsrHna(struct OlsrHeader *Msg)
{
	int Size;
	unsigned int *Addr;

	Msg->SeqNo = ::ntohs(Msg->SeqNo);

	AddNode(Msg->Orig, Msg->VTime);

	Size = Msg->Size;

	Size -= sizeof (struct OlsrHeader);

	Addr = (unsigned int *)(Msg + 1);

	while (Size > 0)
	{
		Size -= 8;

		AddHna(Addr[0], Addr[1], Msg->Orig, Msg->VTime);

		Addr += 2;
	}
}

void CFrontendDlg::HandleOlsrHello(struct OlsrHello *Msg, int UseLq)
{
	int Size, LinkSize;
	struct OlsrHelloLink *Link;
	unsigned int *Addr;

	Msg->Header.SeqNo = ::ntohs(Msg->Header.SeqNo);

	AddNode(Msg->Header.Orig, Msg->Header.VTime);

	Size = Msg->Header.Size;

	Size -= sizeof (struct OlsrHello);

	Link = (struct OlsrHelloLink *)(Msg + 1);

	while (Size > 0)
	{
		Link->Size = ::ntohs(Link->Size);

		LinkSize = Link->Size;

		Size -= LinkSize;

		LinkSize -= sizeof (struct OlsrHelloLink);

		Addr = (unsigned int *)(Link + 1);

		while (LinkSize > 0)
		{
			LinkSize -= 4;

			AddNode(*Addr, Msg->Header.VTime);

			if ((Link->LinkCode & 0x0c) == 0x08)
				AddMpr(*Addr, Msg->Header.Orig, Msg->Header.VTime);

			Addr++;

			if (UseLq != 0)
			{
				LinkSize -= 4;
				Addr++;
			}
		}

		Link = (struct OlsrHelloLink *)Addr;
	}
}

void CFrontendDlg::HandleIpcRoute(struct IpcRoute *Msg)
{
	if (Msg->Header.Size != sizeof (struct IpcRoute))
		return;

	if (Msg->Add == 0)
		m_TabCtrl.m_Dialog4.DeleteRoute(Msg->Dest.v4);

	else
		m_TabCtrl.m_Dialog4.AddRoute(Msg->Dest.v4, Msg->Gate.v4, Msg->Metric,
		Msg->Int);
}

void CFrontendDlg::HandleIpcConfig(struct IpcConfig *Msg)
{
	if (Msg->Header.Size != sizeof (struct IpcConfig))
		return;

	Msg->HelloInt = ::ntohs(Msg->HelloInt);
	Msg->WiredHelloInt = ::ntohs(Msg->WiredHelloInt);
	Msg->TcInt = ::ntohs(Msg->TcInt);

	Msg->HelloHold = ::ntohs(Msg->HelloHold);
	Msg->TcHold = ::ntohs(Msg->TcHold);

	LocalHost = Msg->MainAddr.v4;
}

static int FullRead(SOCKET SockHand, char *Buff, int Len)
{
	int Res;

	do
	{
		Res = ::recv(SockHand, Buff, Len, 0);

		if (Res <= 0)
			return -1;

		Len -= Res;
		Buff += Res;
	}
	while (Len > 0);

	return 0;
}

// XXX - we should have access to olsrd's internal data structures instead

unsigned int CFrontendDlg::NetThreadFunc(void)
{
	struct IpcHeader Header;
	int Res;
	char *Msg;

	for (;;)
	{
		Res = FullRead(SockHand, (char *)&Header, sizeof (struct IpcHeader));

		if (Res < 0)
			break;

		Header.Size = ntohs(Header.Size);

		Msg = new char [Header.Size];

		::memcpy(Msg, &Header, sizeof (struct IpcHeader));

		Res = FullRead(SockHand, Msg + sizeof (struct IpcHeader),
			Header.Size - sizeof (struct IpcHeader));

		if (Res < 0)
			break;

		SYSTEMTIME SysTime;
		FILETIME FileTime;

		::GetSystemTime(&SysTime);
		::SystemTimeToFileTime(&SysTime, &FileTime);

		Now = *(unsigned __int64 *)&FileTime;

		switch (Header.Type)
		{
		case MSG_TYPE_IPC_ROUTE:
			HandleIpcRoute((struct IpcRoute *)Msg);
			break;

		case MSG_TYPE_IPC_CONFIG:
			HandleIpcConfig((struct IpcConfig *)Msg);
			break;

		case MSG_TYPE_OLSR_HELLO:
			HandleOlsrHello((struct OlsrHello *)Msg, 0);
			break;

		case MSG_TYPE_OLSR_TC:
			HandleOlsrTc((struct OlsrTc *)Msg, 0);
			break;

		case MSG_TYPE_OLSR_MID:
			HandleOlsrMid((struct OlsrHeader *)Msg);
			break;

		case MSG_TYPE_OLSR_HNA:
			HandleOlsrHna((struct OlsrHeader *)Msg);
			break;

		case MSG_TYPE_OLSR_LQ_HELLO:
			HandleOlsrHello((struct OlsrHello *)Msg, 1);
			break;

		case MSG_TYPE_OLSR_LQ_TC:
			HandleOlsrTc((struct OlsrTc *)Msg, 1);
			break;
		}

		delete[] Msg;

		// XXX - nodes are only timed out while messages keep coming in

		Timeout();
	}

	AfxEndThread(0);
	return 0;
}

unsigned int CFrontendDlg::LogThreadFunc(void)
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

				if (PipeMode == PIPE_MODE_RUN)
					m_TabCtrl.m_Dialog1.AddOutputLine(Line);

				else if (Line.GetLength() > 8 && Line[0] == 'i' && Line[1] == 'f')
				{
					Int = Line.Mid(0, 4);
					Int.MakeUpper();

					Interfaces.Add(Int);
					IsWlan.Add(Line.Mid(6, 1));
					Addresses.Add(Line.Mid(8));
				}

				Line.Empty();

				Left = Right + 1;
			}
		}

		Buff[Right] = 0;
		Line += (Buff + Left);
	}

	if (PipeMode == PIPE_MODE_RUN)
	{
		m_StopButton.EnableWindow(FALSE);
		m_StartButton.EnableWindow(TRUE);
	}

	AfxEndThread(0);
	return 0;
}

static unsigned int LogThreadStub(void *Arg)
{
	class CFrontendDlg *This;

	This = (class CFrontendDlg *)Arg;

	return This->LogThreadFunc();
}

static unsigned int NetThreadStub(void *Arg)
{
	class CFrontendDlg *This;

	This = (class CFrontendDlg *)Arg;

	return This->NetThreadFunc();
}

int CFrontendDlg::ExecutePipe(const char *CmdLine, HANDLE *InWrite,
							  HANDLE *OutRead, HANDLE *ShimProc)
{
	SECURITY_ATTRIBUTES SecAttr;
	HANDLE OutWrite, OutReadTmp;
	HANDLE ErrWrite;
	HANDLE InRead, InWriteTmp;
	HANDLE Proc;
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;

	SecAttr.nLength = sizeof (SECURITY_ATTRIBUTES);
	SecAttr.lpSecurityDescriptor = NULL;
	SecAttr.bInheritHandle = TRUE;

	Proc = ::GetCurrentProcess();

	if (!::CreatePipe(&OutReadTmp, &OutWrite, &SecAttr, 0))
	{
		AfxMessageBox("Cannot create stdout pipe.");
		return -1;
	}

	if (!::DuplicateHandle(Proc, OutReadTmp, Proc, OutRead,
		0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		AfxMessageBox("Cannot duplicate temporary stdout read handle.");
		return -1;
	}

	if (!::CloseHandle(OutReadTmp))
	{
		AfxMessageBox("Cannot close temporary stdout read handle.");
		return -1;
	}

	if (!::CreatePipe(&InRead, &InWriteTmp, &SecAttr, 0))
	{
		AfxMessageBox("Cannot create stdin pipe.");
		return -1;
	}

	if (!::DuplicateHandle(Proc, InWriteTmp, Proc, InWrite,
		0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		AfxMessageBox("Cannot duplicate temporary stdin write handle.");
		return -1;
	}

	if (!::CloseHandle(InWriteTmp))
	{
		AfxMessageBox("Cannot close temporary stdin write handle.");
		return -1;
	}

	if (!::DuplicateHandle(Proc, OutWrite, Proc, &ErrWrite,
		0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		AfxMessageBox("Cannot duplicate stdout write handle for stderr.");
		return -1;
	}

	::memset(&StartupInfo, 0, sizeof (STARTUPINFO));

	StartupInfo.cb = sizeof (STARTUPINFO);

	StartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

	StartupInfo.hStdInput = InRead;
	StartupInfo.hStdOutput = OutWrite;
	StartupInfo.hStdError = ErrWrite;

	StartupInfo.wShowWindow = SW_HIDE;

	if (!::CreateProcess(NULL, (char *)CmdLine, NULL, NULL, TRUE,
		0, NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		AfxMessageBox("Cannot create OLSR server process.");
		return -1;
	}

	if (!::CloseHandle(InRead))
	{
		AfxMessageBox("Cannot close stdin read handle.");
		return -1;
	}

	if (!::CloseHandle(OutWrite))
	{
		AfxMessageBox("Cannot close stdout write handle.");
		return -1;
	}

	if (!::CloseHandle(ErrWrite))
	{
		AfxMessageBox("Cannot close stderr write handle.");
		return -1;
	}

	*ShimProc = ProcessInfo.hProcess;

	return 0;
}

int CFrontendDlg::GetInterfaces()
{
	char GuiPath[MAX_PATH];
	CString CmdLine;
	CWinThread *IntThread;

	::GetModuleFileName(NULL, GuiPath, MAX_PATH);

	CmdLine = GuiPath;
	CmdLine = CmdLine.Mid(0, CmdLine.ReverseFind('\\')) + "\\olsrd.exe -int";

	if (ExecutePipe((const char *)CmdLine, &InWrite, &OutRead, &ShimProc) < 0)
	{
		AfxMessageBox("Cannot execute '" + CmdLine + "'.");
		return -1;
	}

	PipeMode = PIPE_MODE_INT;

	IntThread = AfxBeginThread(LogThreadStub, (void *)this);

	::WaitForSingleObject((HANDLE)(*IntThread), INFINITE);

	return 0;
}

int CFrontendDlg::StartOlsrd()
{
	WSADATA WsaData;
	CString CmdLine;
	char Path[MAX_PATH];
	char TempPath[MAX_PATH];
	int Try;

	m_TabCtrl.m_Dialog3.ClearNodeInfo();
	m_TabCtrl.m_Dialog4.ClearRoutes();

	if (WSAStartup(0x0202, &WsaData))
	{
		AfxMessageBox("Cannot initialize WinSock library.");
		return -1;
	}

	::GetModuleFileName(NULL, Path, MAX_PATH);

	CmdLine = Path;
	CmdLine = CmdLine.Mid(0, CmdLine.ReverseFind('\\')) + "\\Shim.exe";

	::GetTempPath(MAX_PATH - 16, Path);
	::GetTempFileName(Path, "GNU", 0, TempPath);

	StoredTempFile = TempPath;

	if (m_TabCtrl.m_Dialog2.SaveConfigFile(StoredTempFile, 0) < 0)
	{
		AfxMessageBox("Cannot save temporary configuration file '" + 
			StoredTempFile + "'.");
		return -1;
	}

	CmdLine += " -f " + StoredTempFile;

	if (ExecutePipe((const char *)CmdLine, &InWrite, &OutRead, &ShimProc) < 0)
	{
		AfxMessageBox("Cannot execute '" + CmdLine + "'.");
		return -1;
	}

	PipeMode = PIPE_MODE_RUN;

	LogThread = AfxBeginThread(LogThreadStub, (void *)this);

	struct sockaddr_in Addr;

	Addr.sin_family = AF_INET;
	Addr.sin_port = ::htons(1212);
	Addr.sin_addr.s_addr = ::inet_addr("127.0.0.1");

	SockHand = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (SockHand < 0)
	{
		AfxMessageBox("Cannot create IPC socket.");

		::SetEvent(Event);
		::WaitForSingleObject((HANDLE)LogThread, INFINITE);

		LogThread = NULL;

		return -1;
	}

	for (Try = 0; Try < 5; Try++)
	{
		if (::connect(SockHand, (struct sockaddr *)&Addr,
			sizeof (struct sockaddr_in)) >= 0)
			break;

		::Sleep(500);
	}

	if (Try == 10)
	{
		AfxMessageBox("Cannot connect to IPC port.");

		::SetEvent(Event);
		::WaitForSingleObject((HANDLE)LogThread, INFINITE);

		::closesocket(SockHand);

		LogThread = NULL;

		return -1;
	}

	NetThread = AfxBeginThread(NetThreadStub, (void *)this);

	return 0;
}

int CFrontendDlg::StopOlsrd()
{
	if (LogThread == NULL && NetThread == NULL)
		return 0;

	TrayIcon::getInstance()->setStatus( TrayIcon::OFF, "Off" );

	::SetEvent(Event);

	::WaitForSingleObject((HANDLE)LogThread, INFINITE);
	::WaitForSingleObject((HANDLE)NetThread, INFINITE);

	LogThread = NULL;
	NetThread = NULL;

	::DeleteFile(StoredTempFile);

	return 0;
}

BOOL CFrontendDlg::OnInitDialog()
{
	HICON Small, Large;

	CDialog::OnInitDialog();

	Small = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON), 0);

	Large = (HICON)::LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_ICON1),
		IMAGE_ICON, GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON), 0);

	SetIcon(Small, FALSE);
	SetIcon(Large, TRUE);

	GetInterfaces();

	m_TabCtrl.InsertItem(0, "Settings");
	m_TabCtrl.InsertItem(1, "Output");
	m_TabCtrl.InsertItem(2, "Nodes");
	m_TabCtrl.InsertItem(3, "Routes");

	m_TabCtrl.InitTabDialogs(&Interfaces, &Addresses, &IsWlan);

	m_StopButton.EnableWindow(FALSE);

	if (!ConfigFile.IsEmpty())
	{
		if (m_TabCtrl.m_Dialog2.OpenConfigFile(ConfigFile) < 0)
			AfxMessageBox("Cannot open configuration file '" + ConfigFile + "'.");

		else
		{
			OnStartButton();

			m_TabCtrl.SetCurSel(1);
			m_TabCtrl.DisplayTabDialog();
		}
	}

	return TRUE;
}

void CFrontendDlg::OnOK()
{
}

void CFrontendDlg::OnCancel()
{
	OnExitButton();
}

void CFrontendDlg::OnStartButton() 
{
	m_StartButton.EnableWindow(FALSE);

	m_TabCtrl.m_Dialog1.SetFrozen(1);

	if (StartOlsrd() < 0)
	{
		m_TabCtrl.m_Dialog1.SetFrozen(0);
		m_TabCtrl.m_Dialog1.AddOutputLine("");

		AfxMessageBox("Cannot start OLSR server.");

		m_StartButton.EnableWindow(TRUE);

		return;
	}

	m_TabCtrl.m_Dialog1.HandleStart();

	m_StopButton.EnableWindow(TRUE);
}

void CFrontendDlg::OnStopButton() 
{
	if (StopOlsrd() < 0)
		return;

	m_TabCtrl.m_Dialog1.HandleStop();

	m_StopButton.EnableWindow(FALSE);
	m_StartButton.EnableWindow(TRUE);
}

void CFrontendDlg::OnExitButton()
{
	delete TrayIcon::getInstance();

	if (StopOlsrd() < 0)
		return;

	m_TabCtrl.m_Dialog3.ClearNodeInfo();
	m_TabCtrl.m_Dialog4.ClearRoutes();

	DestroyWindow();
}

#endif /* _WIN32 */
