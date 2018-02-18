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
#include "MyDialog2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif /* _DEBUG */

#define MAXIF 100

MyDialog2::MyDialog2(CWnd* pParent)
	: CDialog(MyDialog2::IDD, pParent)
{
	Conf = NULL;

	//{{AFX_DATA_INIT(MyDialog2)
	//}}AFX_DATA_INIT
}

void MyDialog2::SetDebugLevel(int Level)
{
	char LevelText[2];

	LevelText[0] = (char)(Level + '0');
	LevelText[1] = 0;

	DebugLevel = Level;
	m_DebugLevel.SetPos(Level);
	m_DebugLevelText.SetWindowText(LevelText);
}

BOOL MyDialog2::Create(CWnd *Parent)
{
	return CDialog::Create(MyDialog2::IDD, Parent);
}

void MyDialog2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MyDialog2)
	DDX_Control(pDX, IDC_COMBO3, m_LqAlgo);
	DDX_Control(pDX, IDC_COMBO1, m_TcRed);
	DDX_Control(pDX, IDC_EDIT15, m_MprCov);
	DDX_Control(pDX, IDC_RADIO2, m_EtxRadio2);
	DDX_Control(pDX, IDC_RADIO1, m_EtxRadio1);
	DDX_Control(pDX, IDC_CHECK4, m_EtxCheck);
	DDX_Control(pDX, IDC_CHECK3, m_Ipv6Check);
	DDX_Control(pDX, IDC_CHECK2, m_InternetCheck);
	DDX_Control(pDX, IDC_CHECK1, m_HystCheck);
	DDX_Control(pDX, IDC_CHECK5, m_FishEyeCheck);
	DDX_Control(pDX, IDC_EDIT13, m_HystThresholdHigh);
	DDX_Control(pDX, IDC_EDIT12, m_HystThresholdLow);
	DDX_Control(pDX, IDC_EDIT11, m_HystScaling);
	DDX_Control(pDX, IDC_EDIT10, m_HnaHold);
	DDX_Control(pDX, IDC_EDIT9, m_MidHold);
	DDX_Control(pDX, IDC_EDIT7, m_PollInt);
	DDX_Control(pDX, IDC_EDIT6, m_TcHold);
	DDX_Control(pDX, IDC_EDIT5, m_TcInt);
	DDX_Control(pDX, IDC_EDIT4, m_HnaInt);
	DDX_Control(pDX, IDC_EDIT3, m_MidInt);
	DDX_Control(pDX, IDC_EDIT2, m_HelloHold);
	DDX_Control(pDX, IDC_EDIT1, m_HelloInt);
	DDX_Control(pDX, IDC_LIST1, m_InterfaceList);
	DDX_Control(pDX, IDC_TEXT1, m_DebugLevelText);
	DDX_Control(pDX, IDC_SLIDER2, m_DebugLevel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(MyDialog2, CDialog)
	//{{AFX_MSG_MAP(MyDialog2)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK1, OnHystCheck)
	ON_BN_CLICKED(IDC_BUTTON4, OnOpenButton)
	ON_BN_CLICKED(IDC_BUTTON5, OnSaveButton)
	ON_BN_CLICKED(IDC_BUTTON1, OnResetButton)
	ON_BN_CLICKED(IDC_CHECK4, OnEtxCheck)
	ON_BN_CLICKED(IDC_RADIO1, OnEtxRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnEtxRadio2)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void MyDialog2::OnOK()
{
}

void MyDialog2::OnCancel()
{
}

void MyDialog2::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (pScrollBar == (CScrollBar *)&m_DebugLevel)
		SetDebugLevel(m_DebugLevel.GetPos());
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void MyDialog2::Reset(void)
{
	char PathName[MAX_PATH + 50];
	char *Walker;
	int i;

	::GetModuleFileName(NULL, PathName, MAX_PATH);

	for (Walker = PathName; *Walker != 0; Walker++);
	
	while (*Walker != '\\')
		Walker--;

	lstrcpy(Walker + 1, "Default.olsr");

	if (OpenConfigFile(PathName) < 0)
		return;

	m_Ipv6Check.SetCheck(FALSE);

	if (Conf->interfaces == NULL)
	{
		for (i = 0; i < Interfaces->GetSize(); i++)
		{
			if ((*IsWlan)[i] == "-")
				m_InterfaceList.SetCheck(i, FALSE);

			else
				m_InterfaceList.SetCheck(i, TRUE);
		}
	}
}

BOOL MyDialog2::OnInitDialog() 
{
	int i;

	CDialog::OnInitDialog();
	
	m_DebugLevel.SetRange(0, 9, TRUE);

	m_InterfaceList.SetExtendedStyle(m_InterfaceList.GetExtendedStyle() |
			LVS_EX_CHECKBOXES);

	for (i = 0; i < Interfaces->GetSize(); i++)
	{
		m_InterfaceList.InsertItem(i,
		(*Interfaces)[i] + " - " + (*Addresses)[i]);
	}

	MIB_IPFORWARDROW IpFwdRow;

	if (::GetBestRoute(0, 0, &IpFwdRow) != NO_ERROR)
		m_InternetCheck.EnableWindow(FALSE);

	m_MprCov.LimitText(1);

	Reset();

	return TRUE;
}

void MyDialog2::OnHystCheck() 
{
	BOOL EnaDis = m_HystCheck.GetCheck();

	m_HystThresholdLow.EnableWindow(EnaDis);
	m_HystThresholdHigh.EnableWindow(EnaDis);
	m_HystScaling.EnableWindow(EnaDis);
}

void MyDialog2::OnEtxCheckWorker() 
{
	BOOL EnaDis = m_EtxCheck.GetCheck();

	m_EtxRadio1.EnableWindow(EnaDis);
	m_EtxRadio2.EnableWindow(EnaDis);
	m_FishEyeCheck.EnableWindow(EnaDis);
}

void MyDialog2::OnEtxCheck()
{
	OnEtxCheckWorker();

	AfxMessageBox("WARNING - This parameter breaks compliance with the OLSR standard.\n\n"
		"Make sure that either all nodes in your network use ETX or all nodes in your network don't use ETX.\n\n"
		"DO NOT MIX NODES WITH DIFFERENT ETX SETTINGS!");
}

int MyDialog2::OpenConfigFile(CString PathName)
{
	struct ip_prefix_list *Hna;
	struct olsr_if *Int, *PrevInt;
	struct olsr_msg_params *MsgPar;
	int NumInt = m_InterfaceList.GetItemCount();
	int i;
	CString IntName;
	CString Conv;

	if (Conf != NULL)
		olsrd_free_cnf(Conf);

	if (olsrd_parse_cnf(PathName) < 0)
		return -1;

	Conf = olsr_cnf;

	for (i = 0; i < NumInt; i++)
		m_InterfaceList.SetCheck(i, FALSE);

	for (Int = Conf->interfaces; Int != NULL; Int = Int->next)
	{
		IntName = Int->name;
		IntName.MakeUpper();

		for (i = 0; i < NumInt; i++)
		{
			if (m_InterfaceList.GetItemText(i, 0).Mid(0, 4) == IntName)
				m_InterfaceList.SetCheck(i, TRUE);
		}
	}

	Int = Conf->interfaces;

	MsgPar = &Int->cnf->hello_params;

	Conv.Format("%.2f", MsgPar->emission_interval);
	m_HelloInt.SetWindowText(Conv);

	Conv.Format("%.2f", MsgPar->validity_time);
	m_HelloHold.SetWindowText(Conv);

	MsgPar = &Int->cnf->tc_params;
	
	Conv.Format("%.2f", MsgPar->emission_interval);
	m_TcInt.SetWindowText(Conv);

	Conv.Format("%.2f", MsgPar->validity_time);
	m_TcHold.SetWindowText(Conv);

	MsgPar = &Int->cnf->mid_params;
	
	Conv.Format("%.2f", MsgPar->emission_interval);
	m_MidInt.SetWindowText(Conv);

	Conv.Format("%.2f", MsgPar->validity_time);
	m_MidHold.SetWindowText(Conv);

	MsgPar = &Int->cnf->hna_params;
	
	Conv.Format("%.2f", MsgPar->emission_interval);
	m_HnaInt.SetWindowText(Conv);

	Conv.Format("%.2f", MsgPar->validity_time);
	m_HnaHold.SetWindowText(Conv);

	SetDebugLevel(Conf->debug_level);

	Conv.Format("%.2f", Conf->pollrate);
	m_PollInt.SetWindowText(Conv);

	Conv.Format("%d", Conf->mpr_coverage);
	m_MprCov.SetWindowText(Conv);

	m_TcRed.SetCurSel(Conf->tc_redundancy);

	m_LqAlgo.SetCurSel(m_LqAlgo.FindStringExact(-1, Conf->lq_algorithm));

	m_HystCheck.SetCheck(Conf->use_hysteresis);

	Conv.Format("%.2f", Conf->hysteresis_param.scaling);
	m_HystScaling.SetWindowText(Conv);

	Conv.Format("%.2f", Conf->hysteresis_param.thr_high);
	m_HystThresholdHigh.SetWindowText(Conv);

	Conv.Format("%.2f", Conf->hysteresis_param.thr_low);
	m_HystThresholdLow.SetWindowText(Conv);

	OnHystCheck();

	m_FishEyeCheck.SetCheck(Conf->lq_fish > 0);

	m_EtxCheck.SetCheck(Conf->lq_level > 0);

#if 0
	Conv.Format("%d", Conf->lq_wsize);
	m_EtxWindowSize.SetWindowText(Conv);
#endif

	m_EtxRadio1.SetCheck(Conf->lq_level == 1);
	m_EtxRadio2.SetCheck(Conf->lq_level == 0 || Conf->lq_level == 2);

	OnEtxCheckWorker();

	m_InternetCheck.SetCheck(FALSE);

	for (Hna = Conf->hna_entries; Hna != NULL; Hna = Hna->next)
		if (0 == Hna->net.prefix_len &&
			m_InternetCheck.IsWindowEnabled())
		m_InternetCheck.SetCheck(TRUE);

	PrevInt = NULL;

	for (Int = Conf->interfaces; Int != NULL; Int = Int->next)
	{
		IntName = Int->name;

		if (IntName.CompareNoCase("GUI") == 0)
			break;

		PrevInt = Int;
	}

	if (Int != NULL)
	{
		if (PrevInt == NULL)
			Conf->interfaces = Int->next;

		else
			PrevInt->next = Int->next;

		win32_olsrd_free(Int);
	}

	return 0;
}

static struct olsr_if *AddInterface(struct olsrd_config **Conf, CString Name)
{
	struct olsr_if *Int;

	Int = (struct olsr_if *)win32_olsrd_malloc(sizeof (struct olsr_if));

	if (Int == NULL)
	{
		AfxMessageBox("Cannot allocate memory.");
		return NULL;
	}

	Int->name = (char *)win32_olsrd_malloc(Name.GetLength() + 1);

	if (Int->name == NULL)
	{
		win32_olsrd_free(Int);

		AfxMessageBox("Cannot allocate memory.");
		return NULL;
	}

	::lstrcpy(Int->name, Name);

	Int->config = NULL;
	Int->configured = false;
	Int->interf = NULL;

	Int->cnf = get_default_if_config();

	Int->next = (*Conf)->interfaces;
	(*Conf)->interfaces = Int;

	return Int;
}

int MyDialog2::SaveConfigFile(CString PathName, int Real)
{
	struct olsr_if *Int, *PrevInt;
	struct olsr_msg_params *MsgPar;
	CString Conv;
	struct ip_prefix_list *Hna, *NewHna, *PrevHna;
	int NumInt = m_InterfaceList.GetItemCount();
	int i;
	CString IntName, IntName2;
	struct ip_prefix_list *IpcHost;
	union olsr_ip_addr Local;

	PrevInt = NULL;

	// remove interfaces that we do not want
	
	for (Int = Conf->interfaces; Int != NULL; Int = Int->next)
	{
		IntName = Int->name;
		IntName.MakeUpper();

		for (i = 0; i < NumInt; i++)
			if (m_InterfaceList.GetItemText(i, 0).Mid(0, 4) == IntName)
				break;

		if (i == NumInt || !m_InterfaceList.GetCheck(i))
		{
			if (PrevInt != NULL)
				PrevInt->next = Int->next;

			else
				Conf->interfaces = Int->next;
		}
	}
	
	// add missing interfaces
	
	for (i = 0; i < NumInt; i++)
	{
		if (!m_InterfaceList.GetCheck(i))
			continue;

		IntName2 = m_InterfaceList.GetItemText(i, 0).Mid(0, 4);

		for (Int = Conf->interfaces; Int != NULL; Int = Int->next)
		{
			IntName = Int->name;
			IntName.MakeUpper();

			if (IntName2 == IntName)
				break;
		}

		if (Int == NULL)
			AddInterface(&Conf, m_InterfaceList.GetItemText(i, 0).Mid(0, 4));
	}

	// add dummy interface, if there aren't any real interfaces

	if (Conf->interfaces == NULL && Real != 0)
		AddInterface(&Conf, "GUI");

	// per-interface settings

	for (Int = Conf->interfaces; Int != NULL; Int = Int->next)
	{
		MsgPar = &Int->cnf->hello_params;

		m_HelloInt.GetWindowText(Conv);
		MsgPar->emission_interval = (float)atof(Conv);

		m_HelloHold.GetWindowText(Conv);
		MsgPar->validity_time = (float)atof(Conv);

		MsgPar = &Int->cnf->tc_params;

		m_TcInt.GetWindowText(Conv);
		MsgPar->emission_interval = (float)atof(Conv);

		m_TcHold.GetWindowText(Conv);
		MsgPar->validity_time = (float)atof(Conv);

		MsgPar = &Int->cnf->mid_params;

		m_MidInt.GetWindowText(Conv);
		MsgPar->emission_interval = (float)atof(Conv);

		m_MidHold.GetWindowText(Conv);
		MsgPar->validity_time = (float)atof(Conv);

		MsgPar = &Int->cnf->hna_params;

		m_HnaInt.GetWindowText(Conv);
		MsgPar->emission_interval = (float)atof(Conv);

		m_HnaHold.GetWindowText(Conv);
		MsgPar->validity_time = (float)atof(Conv);
	}

	// global settings

	Conf->debug_level = DebugLevel;

	m_PollInt.GetWindowText(Conv);
	Conf->pollrate = (float)atof(Conv);

	Conf->tc_redundancy = (unsigned char)m_TcRed.GetCurSel();

	i = m_LqAlgo.GetCurSel();
	Conf->lq_algorithm = NULL;
	if (0 <= i)
	{
		CString str;
		m_LqAlgo.GetLBText(i, str);
		Conf->lq_algorithm = strdup((LPCTSTR)str);
	}

	m_MprCov.GetWindowText(Conv);
	Conf->mpr_coverage = (unsigned char)atoi(Conv);

	Conf->use_hysteresis = m_HystCheck.GetCheck() ? true : false;

	m_HystScaling.GetWindowText(Conv);
	Conf->hysteresis_param.scaling = (float)atof(Conv);

	m_HystThresholdHigh.GetWindowText(Conv);
	Conf->hysteresis_param.thr_high = (float)atof(Conv);

	m_HystThresholdLow.GetWindowText(Conv);
	Conf->hysteresis_param.thr_low = (float)atof(Conv);

	if (!m_EtxCheck.GetCheck())
		Conf->lq_level = 0;

	else if (m_EtxRadio1.GetCheck())
		Conf->lq_level = 1;

	else
		Conf->lq_level = 2;

	if (!m_FishEyeCheck.GetCheck())
		Conf->lq_fish = 0;

	else
		Conf->lq_fish = 1;

	PrevHna = NULL;

	// check for a default gateway HNA4 entry

	for (Hna = Conf->hna_entries; Hna != NULL; Hna = Hna->next)
	{
		if (0 == Hna->net.prefix_len)
			break;

		PrevHna = Hna;
	}

	// add default gateway HNA entry

	if (m_InternetCheck.GetCheck() && Hna == NULL)
	{
		NewHna = (struct ip_prefix_list * )
			win32_olsrd_malloc(sizeof (struct ip_prefix_list));

		if (NewHna == NULL)
		{
			AfxMessageBox("Cannot allocate memory.");
			return -1;
		}

		memset(NewHna, 0, sizeof (struct ip_prefix_list));

		NewHna->next = Conf->hna_entries;
		Conf->hna_entries = NewHna;
	}

	// remove default gateway HNA4 entry

	if (!m_InternetCheck.GetCheck() && Hna != NULL)
	{
		if (PrevHna == NULL)
			Conf->hna_entries = Hna->next;

		else
			PrevHna->next = Hna->next;

		win32_olsrd_free(Hna);
	}

	if (AF_INET == Conf->ip_version)
	{
		Local.v4.s_addr = ::inet_addr("127.0.0.1");
	}
	else
	{
		memset(&Local, 0, sizeof(Local));
		Local.v6.u.Byte[15] = 1;
	}

	for (IpcHost = Conf->ipc_nets; IpcHost != NULL; IpcHost = IpcHost->next)
		if (0 == memcmp(&IpcHost->net.prefix, &Local, Conf->ipsize))
			break;

	if (IpcHost == NULL && Real == 0)
	{
		IpcHost = (struct ip_prefix_list *)
			win32_olsrd_malloc(sizeof (struct ip_prefix_list));

		if (IpcHost == NULL)
		{
			AfxMessageBox("Cannot allocate memory.");
			return -1;
		}

		IpcHost->net.prefix = Local;
		IpcHost->net.prefix_len = (uint8_t)Conf->ipsize;

		IpcHost->next = Conf->ipc_nets;
		Conf->ipc_nets = IpcHost;

		Conf->ipc_connections++;
	}

	// write configuration file

	if (olsrd_write_cnf(Conf, PathName) < 0)
		return -1;

	return 0;
}

void MyDialog2::OnSaveButton()
{
	CFileDialog FileDialog(FALSE);
	CString FileName = "Default.olsr";
	CString PathName;

	FileDialog.m_ofn.lpstrFilter = "Configuration file (*.olsr)\0*.olsr\0";
	FileDialog.m_ofn.nFilterIndex = 1;

	FileDialog.m_ofn.lpstrFile = FileName.GetBuffer(500);
	FileDialog.m_ofn.nMaxFile = 500;

	if (FileDialog.DoModal() == IDOK)
	{
		PathName = FileDialog.GetPathName();

		if (SaveConfigFile(PathName, 1) < 0)
			AfxMessageBox("Cannot save configuration file '" + PathName + "'.");
	}

	FileName.ReleaseBuffer();
}

void MyDialog2::OnOpenButton()
{
	CFileDialog FileDialog(TRUE);
	CString FileName = "Default.olsr";
	CString PathName;

	FileDialog.m_ofn.lpstrFilter = "Configuration file (*.olsr)\0*.olsr\0";
	FileDialog.m_ofn.nFilterIndex = 1;

	FileDialog.m_ofn.lpstrFile = FileName.GetBuffer(500);
	FileDialog.m_ofn.nMaxFile = 500;

	if (FileDialog.DoModal() == IDOK)
	{
		PathName = FileDialog.GetPathName();

		if (OpenConfigFile(PathName) < 0)
			AfxMessageBox("Cannot open configuration file '" + PathName + "'.");
	}

	FileName.ReleaseBuffer();
}

void MyDialog2::OnResetButton() 
{
	Reset();
}

void MyDialog2::OnEtxRadio1() 
{
	m_EtxRadio2.SetCheck(FALSE);
}

void MyDialog2::OnEtxRadio2() 
{
	m_EtxRadio1.SetCheck(FALSE);
}

#endif /* _WIN32 */
