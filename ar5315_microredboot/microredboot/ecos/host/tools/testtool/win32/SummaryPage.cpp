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
// SummaryPage.cpp : implementation file
//

#include "stdafx.h"
#include "SummaryPage.h"
#include "eCosTest.h"
#include "RunTestsSheet.h"
#include <windowsx.h> // for GET_X_LPARAM, GET_Y_LPARAM

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LPCTSTR CSummaryPage::arpszCols[]={_T("Time"), _T("Host"), _T( "Platform"), _T("Executable"), _T("Status"), _T("Size"), _T("Download"), _T("Elapsed"), _T("Execution")};
const int CSummaryPage::nCols=sizeof CSummaryPage::arpszCols/sizeof CSummaryPage::arpszCols[0];

/////////////////////////////////////////////////////////////////////////////
// CSummaryPage property page

IMPLEMENT_DYNCREATE(CSummaryPage, CeCosPropertyPage)

CSummaryPage::CSummaryPage() : CeCosPropertyPage(IDD_TT_SUMMARY_PAGE)
{
	//{{AFX_DATA_INIT(CSummaryPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CSummaryPage::~CSummaryPage()
{
}

void CSummaryPage::DoDataExchange(CDataExchange* pDX)
{
	CeCosPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSummaryPage)
	DDX_Control(pDX, IDC_TT_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSummaryPage, CeCosPropertyPage)
	//{{AFX_MSG_MAP(CSummaryPage)
	ON_NOTIFY(NM_RCLICK, IDC_TT_LIST, OnRclickList)
	ON_COMMAND(IDC_TT_CLEAR, OnClear)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_TT_LIST, OnColumnclickList)
	ON_WM_SIZE()
	ON_COMMAND(ID_TT_EDIT_SAVE, OnEditSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSummaryPage message handlers

void CSummaryPage::AddResult(CeCosTest *pTest)
{
  const CString strResult((LPCTSTR)pTest->ResultString(false));
  int nLength=strResult.GetLength();
  CString arstr[8];
  CString strTime,strDate;
  TRACE(_T("%s\n"),strResult);
  // 1999-05-28 10:29:28 nan:0 TX39-jmr3904-sim tx39-jmr3904sim-libc10-signal2.exe Fail 0k/1108k D=0.0/0.0 Total=9.3 E=0.6/300.0 
  _stscanf(strResult,_T("%s %s %s %s %s %s %s %s %s %s"),strDate.GetBuffer(1+nLength),strTime.GetBuffer(1+nLength),arstr[0].GetBuffer(1+nLength),arstr[1].GetBuffer(1+nLength),arstr[2].GetBuffer(1+nLength),arstr[3].GetBuffer(1+nLength),arstr[4].GetBuffer(1+nLength),arstr[5].GetBuffer(1+nLength),arstr[6].GetBuffer(1+nLength),arstr[7].GetBuffer(1+nLength));
  // Remove before '=' in time fields
  for(int i=5;i<8;i++){
      TCHAR *pch=_tcschr(arstr[i],_TCHAR('='));
      if(pch){
          arstr[i]=pch+1;
      }
  }
  
  strDate.ReleaseBuffer();
  strTime.ReleaseBuffer();
  strDate+=_TCHAR(' ');
  strDate+=strTime;
  int nItem=m_List.GetItemCount();
  m_List.InsertItem(nItem,strDate);
  m_List.SetItemData(nItem,nItem);// to support sorting
  for(i=0;i<8;i++){
      m_List.SetItemText(nItem,1+i,arstr[i]);
      arstr[i].ReleaseBuffer();
  }
}

BOOL CSummaryPage::OnInitDialog() 
{
    m_nLastCol=0x7fffffff;
	CeCosPropertyPage::OnInitDialog();

    CRect rect;
    m_List.GetClientRect(rect);
    for(int i=0;i<nCols;i++){
        m_List.InsertColumn(i,arpszCols[i]);
    }
	ListView_SetExtendedListViewStyle(m_List.GetSafeHwnd(),LVS_EX_FULLROWSELECT);
    for(i=0;i<nCols;i++){
	    m_List.SetColumnWidth(i,rect.Width()/nCols);
    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSummaryPage::OnRclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    UNUSED_ALWAYS(pNMHDR);
    if(m_List.GetItemCount()>0){
	    CMenu menu;
	    menu.LoadMenu(IDR_TT_CONTEXTMENU1);
        DWORD dwPos=GetMessagePos();
    
	    CMenu *pPopup=menu.GetSubMenu(0);
	    pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, MAKEPOINTS(dwPos).x,MAKEPOINTS(dwPos).y,this);
    }	    
	*pResult = 0;
}

void CSummaryPage::OnClear() 
{
    m_List.DeleteAllItems();
}

int CSummaryPage::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CSummaryPage *pPage=(CSummaryPage *)lParamSort;
	CString str1,str2;
    int nCol=pPage->m_nLastCol&0x7fffffff;
    str1=pPage->m_List.GetItemText(lParam1,nCol);
    str2=pPage->m_List.GetItemText(lParam2,nCol);
    int rc=str1.Compare(str2);
    if(rc && (pPage->m_nLastCol&0x80000000)){
        rc^=0x80000000;
    }
    return rc;
}

void CSummaryPage::OnColumnclickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	LPARAM nCol=pNMListView->iSubItem;
	if((nCol&0x7fffffff)==(0x7fffffff&m_nLastCol)){
		m_nLastCol=m_nLastCol^0x80000000;
    } else {
        m_nLastCol=nCol;
    }
    m_List.SortItems(CompareFunc,(LPARAM)this);
    for(int i=m_List.GetItemCount()-1;i>=0;--i){
        m_List.SetItemData(i,i);
    }
	*pResult = 0;
}

void CSummaryPage::OnSize(UINT nType, int cx, int cy) 
{
	CeCosPropertyPage::OnSize(nType, cx, cy);
    float arProp[nCols]; // to preserve column propertions
    if(m_List){
        CRect rect;
        m_List.GetClientRect(rect);
        float fWidth=(float)rect.Width();
        for(int i=0;i<nCols;i++){
            arProp[i]=float(m_List.GetColumnWidth(i))/fWidth;
        }
        ((CRunTestsSheet*)GetParent())->MoveWindow(GetDlgItem(IDC_TT_LIST),CRunTestsSheet::Stretch);
        m_List.GetClientRect(rect);
        fWidth=(float)rect.Width();
        for(i=0;i<nCols;i++){
	        m_List.SetColumnWidth(i,(int)(0.5+fWidth*arProp[i]));
        }
    }
}

void CSummaryPage::OnEditSave() 
{
	CFileDialog dlg( FALSE, _T("log"), _T("Output"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 		
		_T("Log Files (*.log)|*.log|All Files (*.*)|*.*||"));
	if(IDOK==dlg.DoModal()){
	    TRY
	    {    
		    CStdioFile f( dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite );
            CString str;
            int arColWidth[nCols];
            for(int nCol=0;nCol<nCols;nCol++){
    		    int &w=arColWidth[nCol];
                w=_tcslen(arpszCols[nCol]);
                for(int nItem=0;nItem<m_List.GetItemCount();nItem++){
                    w=max(w,m_List.GetItemText(nItem,nCol).GetLength());
                }
                TRACE(_T("max[%d]=%d\n"),nCol,w);
            }

            for(nCol=0;nCol<nCols;nCol++){
                const CString &strItem=arpszCols[nCol];
                str+=strItem;
                if(nCol<nCols-1){
                    CString strTab(_TCHAR(' '),1+arColWidth[nCol]-strItem.GetLength());
                    str+=strTab;
                }
            }
            str+=_TCHAR('\n');
            f.WriteString(str);
            for(int nItem=0;nItem<m_List.GetItemCount();nItem++){
		        CString str;
                for(int nCol=0;nCol<nCols;nCol++){
                    const CString strItem=m_List.GetItemText(nItem,nCol);
                    str+=strItem;
                    if(nCol<nCols-1){
                        CString strTab(_TCHAR(' '),1+arColWidth[nCol]-strItem.GetLength());
                        str+=strTab;
                    }
                }
                str+=_TCHAR('\n');
		        f.WriteString(str);
            }
		    f.Close();
	    }
	    CATCH( CFileException, e )
	    {
		    MessageBox(_T("Failed to write file"));
	    }
	    END_CATCH
	}
	
}
