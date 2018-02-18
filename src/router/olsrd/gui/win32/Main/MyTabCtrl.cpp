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
#include "MyTabCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif /* _DEBUG */

MyTabCtrl::MyTabCtrl()
{
}

MyTabCtrl::~MyTabCtrl()
{
}

BEGIN_MESSAGE_MAP(MyTabCtrl, CTabCtrl)
	//{{AFX_MSG_MAP(MyTabCtrl)
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelchange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void MyTabCtrl::InitTabDialogs(CStringArray *Interfaces,
							   CStringArray *Addresses,
							   CStringArray *IsWlan)
{
	int i;
	CRect Client;
	CRect Win;

	m_Dialog2.Interfaces = Interfaces;
	m_Dialog2.Addresses = Addresses;
	m_Dialog2.IsWlan = IsWlan;

	m_Dialog1.Create(GetParent());
	m_Dialog2.Create(GetParent());
	m_Dialog3.Create(GetParent());
	m_Dialog4.Create(GetParent());

	Dialogs[0] = &m_Dialog2;
	Dialogs[1] = &m_Dialog1;
	Dialogs[2] = &m_Dialog3;
	Dialogs[3] = &m_Dialog4;

	Sel = -1;

	for (i = 0; i < 4; i++)
	{
		GetClientRect(Client);
		AdjustRect(FALSE, Client);

		GetWindowRect(Win);
		GetParent()->ScreenToClient(Win);

		Client.OffsetRect(Win.left, Win.top);

		Dialogs[i]->SetWindowPos(&wndTop, Client.left, Client.top,
			Client.Width(), Client.Height(), SWP_HIDEWINDOW);
	}

	DisplayTabDialog();
}

void MyTabCtrl::DisplayTabDialog()
{
	if (Sel != -1)
		Dialogs[Sel]->ShowWindow(SW_HIDE);

	Sel = GetCurSel();

	Dialogs[Sel]->ShowWindow(SW_SHOW);
}

void MyTabCtrl::OnSelchange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	pNMHDR = pNMHDR;

	DisplayTabDialog();

	*pResult = 0;
}

#endif /* _WIN32 */
