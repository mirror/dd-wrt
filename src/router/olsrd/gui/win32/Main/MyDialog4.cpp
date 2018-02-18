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
#include "MyDialog4.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif /* _DEBUG */

MyDialog4::MyDialog4(CWnd* pParent)
	: CDialog(MyDialog4::IDD, pParent)
{
	//{{AFX_DATA_INIT(MyDialog4)
	//}}AFX_DATA_INIT
}

BOOL MyDialog4::Create(CWnd *Parent)
{
	return CDialog::Create(MyDialog4::IDD, Parent);
}

void MyDialog4::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MyDialog4)
	DDX_Control(pDX, IDC_LIST1, m_RoutingTable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MyDialog4, CDialog)
	//{{AFX_MSG_MAP(MyDialog4)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void MyDialog4::OnOK()
{
}

void MyDialog4::OnCancel()
{
}

BOOL MyDialog4::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_RoutingTable.InsertColumn(0, "Destination", LVCFMT_LEFT, 110, 0);
	m_RoutingTable.InsertColumn(1, "Gateway", LVCFMT_LEFT, 110, 1);
	m_RoutingTable.InsertColumn(2, "Metric", LVCFMT_LEFT, 68, 2);
	m_RoutingTable.InsertColumn(3, "Interface", LVCFMT_LEFT, 67, 3);

	return TRUE;
}

void MyDialog4::AddRoute(unsigned int Dest, unsigned int Gate, int Metric,
						 char *Int)
{
	CString DestStr;
	CString GateStr;
	CString MetricStr;
	CString IntStr;
	int Idx;

	DestStr.Format("%d.%d.%d.%d",
		((unsigned char *)&Dest)[0], ((unsigned char *)&Dest)[1],
		((unsigned char *)&Dest)[2], ((unsigned char *)&Dest)[3]);

	GateStr.Format("%d.%d.%d.%d",
		((unsigned char *)&Gate)[0], ((unsigned char *)&Gate)[1],
		((unsigned char *)&Gate)[2], ((unsigned char *)&Gate)[3]);

	MetricStr.Format("%d", Metric);

	IntStr.Format("%c%c%c%c", Int[0], Int[1], Int[2], Int[3]);
	IntStr.MakeUpper();

	Idx = m_RoutingTable.GetItemCount();

	m_RoutingTable.InsertItem(Idx, DestStr);

	m_RoutingTable.SetItemText(Idx, 1, GateStr);
	m_RoutingTable.SetItemText(Idx, 2, MetricStr);
	m_RoutingTable.SetItemText(Idx, 3, IntStr);
}

void MyDialog4::DeleteRoute(unsigned int Dest)
{
	CString DestStr;
	int Idx, Num;

	DestStr.Format("%d.%d.%d.%d",
		((unsigned char *)&Dest)[0], ((unsigned char *)&Dest)[1],
		((unsigned char *)&Dest)[2], ((unsigned char *)&Dest)[3]);

	Num = m_RoutingTable.GetItemCount();

	for (Idx = 0; Idx < Num; Idx++)
	{
		if (m_RoutingTable.GetItemText(Idx, 0) == DestStr)
		{
			m_RoutingTable.DeleteItem(Idx);
			break;
		}
	}
}

void MyDialog4::ClearRoutes(void)
{
	m_RoutingTable.DeleteAllItems();
}

#endif /* _WIN32 */
