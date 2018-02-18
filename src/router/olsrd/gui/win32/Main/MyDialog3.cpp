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
#include "MyDialog3.h"

#include "MprEntry.h"
#include "MidEntry.h"
#include "HnaEntry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif /* _DEBUG */

MyDialog3::MyDialog3(CWnd* pParent)
	: CDialog(MyDialog3::IDD, pParent)
{
	LastUpdate = 0;
	Info = NULL;

	//{{AFX_DATA_INIT(MyDialog3)
	//}}AFX_DATA_INIT
}

BOOL MyDialog3::Create(CWnd *Parent)
{
	return CDialog::Create(MyDialog3::IDD, Parent);
}

void MyDialog3::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MyDialog3)
	DDX_Control(pDX, IDC_LIST6, m_HnaList);
	DDX_Control(pDX, IDC_LIST5, m_MidList);
	DDX_Control(pDX, IDC_LIST4, m_MprList);
	DDX_Control(pDX, IDC_LIST1, m_NodeList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(MyDialog3, CDialog)
	//{{AFX_MSG_MAP(MyDialog3)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedNodeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void MyDialog3::OnOK()
{
}

void MyDialog3::OnCancel()
{
}

BOOL MyDialog3::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_NodeList.InsertColumn(0, "Address", LVCFMT_LEFT, 110, 0);
	m_NodeList.InsertColumn(1, "Timeout", LVCFMT_LEFT, 110, 1);
	m_NodeList.InsertColumn(2, "MID", LVCFMT_LEFT, 68, 2);
	m_NodeList.InsertColumn(3, "HNA", LVCFMT_LEFT, 67, 3);

	m_MprList.InsertColumn(0, "MPR", LVCFMT_LEFT, 169, 0);

	m_MidList.InsertColumn(0, "MID", LVCFMT_LEFT, 169, 0);

	m_HnaList.InsertColumn(0, "HNA", LVCFMT_LEFT, 169, 0);

	return TRUE;
}

void MyDialog3::ClearNodeInfo(void)
{
	m_NodeList.DeleteAllItems();

	m_MprList.DeleteAllItems();
	m_MidList.DeleteAllItems();
	m_HnaList.DeleteAllItems();

	if (Info != NULL)
	{
		delete[] Info;
		Info = NULL;
	}
}

void MyDialog3::UpdateNodeInfo(CList<class NodeEntry, class NodeEntry &> &NodeList)
{
	SYSTEMTIME SysTime;
	FILETIME FileTime;
	unsigned __int64 Now;
	class NodeEntry Entry;
	POSITION Pos;
	CString AddrStr;
	CString TimeoutStr;
	int Idx, Num;
	POSITION Pos2;
	class MprEntry MprEntry;
	class MidEntry MidEntry;
	class HnaEntry HnaEntry;
	CString CurrNode;

	::GetSystemTime(&SysTime);
	::SystemTimeToFileTime(&SysTime, &FileTime);

	Now = *(unsigned __int64 *)&FileTime;

	if (Now < LastUpdate + (__int64)3 * (__int64)10000000)
		return;

	LastUpdate = Now;

	int CurrItem = m_NodeList.GetNextItem(-1, LVNI_SELECTED);

	if (CurrItem >= 0)
		CurrNode = m_NodeList.GetItemText(CurrItem, 0);

	m_NodeList.DeleteAllItems();

	if (Info != NULL)
	{
		delete[] Info;
		Info = NULL;
	}

	Num = NodeList.GetCount();

	if (Num == 0)
		return;

	Info = new class NodeInfo[Num];

	Pos = NodeList.GetHeadPosition();

	Idx = 0;

	while (Pos != NULL)
	{
		Entry = NodeList.GetNext(Pos);

		Pos2 = Entry.MprList.GetHeadPosition();

		while (Pos2 != NULL)
		{
			MprEntry = Entry.MprList.GetNext(Pos2);

			AddrStr.Format("%d.%d.%d.%d",
				((unsigned char *)&MprEntry.Addr)[0], ((unsigned char *)&MprEntry.Addr)[1],
				((unsigned char *)&MprEntry.Addr)[2], ((unsigned char *)&MprEntry.Addr)[3]);

			Info[Idx].MprList.Add(AddrStr);
		}

		Pos2 = Entry.MidList.GetHeadPosition();

		while (Pos2 != NULL)
		{
			MidEntry = Entry.MidList.GetNext(Pos2);

			AddrStr.Format("%d.%d.%d.%d",
				((unsigned char *)&MidEntry.Addr)[0], ((unsigned char *)&MidEntry.Addr)[1],
				((unsigned char *)&MidEntry.Addr)[2], ((unsigned char *)&MidEntry.Addr)[3]);

			Info[Idx].MidList.Add(AddrStr);
		}

		Pos2 = Entry.HnaList.GetHeadPosition();

		while (Pos2 != NULL)
		{
			HnaEntry = Entry.HnaList.GetNext(Pos2);

			unsigned int Mask = 0x80000000;
			unsigned int Val = ::ntohl(HnaEntry.Mask);
			int Bits;

			for (Bits = 0; Bits < 32; Bits++)
			{
				if ((Val & Mask) == 0)
					break;

				Mask >>= 1;
			}

			AddrStr.Format("%d.%d.%d.%d/%d",
				((unsigned char *)&HnaEntry.Addr)[0], ((unsigned char *)&HnaEntry.Addr)[1],
				((unsigned char *)&HnaEntry.Addr)[2], ((unsigned char *)&HnaEntry.Addr)[3],
				Bits);

			Info[Idx].HnaList.Add(AddrStr);
		}

		AddrStr.Format("%d.%d.%d.%d",
			((unsigned char *)&Entry.Addr)[0], ((unsigned char *)&Entry.Addr)[1],
			((unsigned char *)&Entry.Addr)[2], ((unsigned char *)&Entry.Addr)[3]);

		m_NodeList.InsertItem(Idx, AddrStr);

		if (AddrStr == CurrNode)
			m_NodeList.SetItemState(Idx, LVIS_SELECTED, LVIS_SELECTED);

		SYSTEMTIME SysTime;
		FILETIME LocalFileTime;

		::FileTimeToLocalFileTime((FILETIME *)&Entry.Timeout, &LocalFileTime);
		::FileTimeToSystemTime(&LocalFileTime, &SysTime);

		TimeoutStr.Format("%02d:%02d:%02d", SysTime.wHour, SysTime.wMinute,
			SysTime.wSecond);

		m_NodeList.SetItemText(Idx, 1, TimeoutStr);

		if (Entry.MidList.IsEmpty())
			m_NodeList.SetItemText(Idx, 2, "no");

		else
			m_NodeList.SetItemText(Idx, 2, "yes");

		if (Entry.HnaList.IsEmpty())
			m_NodeList.SetItemText(Idx, 3, "no");

		else
			m_NodeList.SetItemText(Idx, 3, "yes");

		Idx++;
	}
}

void MyDialog3::OnItemchangedNodeList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int Item;
	int Idx;
	int Num;

	pNMHDR = pNMHDR;

	*pResult = 0;

	Item = m_NodeList.GetNextItem(-1, LVNI_SELECTED);

	if (Item < 0)
		return;

	m_MprList.DeleteAllItems();

	Num = Info[Item].MprList.GetSize();
	
	for (Idx = 0; Idx < Num; Idx++)
		m_MprList.InsertItem(Idx, Info[Item].MprList.GetAt(Idx));

	m_MidList.DeleteAllItems();

	Num = Info[Item].MidList.GetSize();
	
	for (Idx = 0; Idx < Num; Idx++)
		m_MidList.InsertItem(Idx, Info[Item].MidList.GetAt(Idx));

	m_HnaList.DeleteAllItems();

	Num = Info[Item].HnaList.GetSize();
	
	for (Idx = 0; Idx < Num; Idx++)
		m_HnaList.InsertItem(Idx, Info[Item].HnaList.GetAt(Idx));
}

#endif /* _WIN32 */
