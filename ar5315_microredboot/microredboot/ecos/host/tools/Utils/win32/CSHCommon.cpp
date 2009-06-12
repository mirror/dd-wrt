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
// CSHCommon.cpp : implementation file
//

#include "stdafx.h"
#include "CSHCommon.h"
#include <HTMLHelp.h>
#include <windowsx.h> // for GET_X_LPARAM, GET_Y_LPARAM

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCSHCommon

CString CCSHCommon::m_strCSHFilePath;

CCSHCommon::CCSHCommon()
{
  m_bSupressContextMenu=false;
}

CCSHCommon::~CCSHCommon()
{
}

CWnd *CCSHCommon::WndFromPoint(CWnd *pDialog,CWnd* pWnd,CPoint pt) 
{
  CWnd *rc=NULL;
  CPoint ptClient(pt);
  pDialog->ScreenToClient(&ptClient);
  if(pWnd==pDialog){
    // The pWnd argument is the dialog itself for disabled controls
    // Can't use ChildWindowFromPoint() - may return an enclosing group box
    for(CWnd *p=pDialog->GetWindow(GW_CHILD);p;p=p->GetWindow(GW_HWNDNEXT)){
      TCHAR buf[256];
      if(::GetClassName(p->m_hWnd,buf,sizeof buf)){
        if(0==_tcscmp(buf,_T("STATIC"))){
          continue;
        } else if(0==_tcscmp(buf,_T("Button")) && p->GetStyle()&BS_GROUPBOX) {
          continue;
        } else {
          CRect rect;
          p->GetWindowRect(rect);
          if(rect.PtInRect(pt)){
            rc=p;
            break;
          }
        }
      }
    }
  } else {
    rc=pWnd;
  }
  return rc;
}

void CCSHCommon::DisplayHelp(HWND hCtrl,UINT ids,HINSTANCE hInst)
{
  DWORD dwPos=GetMessagePos();
  HH_POPUP hhp;
  if (HIWORD(ids) == 0) {
    hhp.idString=ids;
    hhp.pszText=_T("No help is available for this item");
  } else {
    hhp.idString=0;
    hhp.pszText=(LPCTSTR)ids;
  }

  hhp.cbStruct=sizeof(hhp);
  hhp.hinst=hInst;
  hhp.pt.x=GET_X_LPARAM(dwPos);
  hhp.pt.y=GET_Y_LPARAM(dwPos);
  hhp.clrForeground=(COLORREF)-1; //default 
  hhp.clrBackground=GetSysColor(COLOR_INFOBK); 
  hhp.rcMargins=CRect(-1,-1,-1,-1);
  hhp.pszFont=NULL;

  HtmlHelp(hCtrl,NULL,HH_DISPLAY_TEXT_POPUP,(DWORD)&hhp); 
}

// FilterMessage has the same semantics as OnWndMsg
bool CCSHCommon::FilterMessage(UINT &message, WPARAM &wParam,LPARAM &lParam,LRESULT *&)
{
  switch(message){
    case WM_ACTIVATE:
      // This fixes a bug in HTMLHelp v1.3 whereby a click on the dialog to dismiss a helpbox causes a crash
      if(WA_CLICKACTIVE==wParam && !::IsWindow((HWND)lParam)) {
        lParam=0;
      }
      break;
      /*
    case WM_NOTIFY:
      // This deals with the case that a control is sending us the notification message.  We set the flag to ignore
      // the next WM_CONTEXTMENU message (for else we would prevent 
      if(NM_RCLICK==((LPNMHDR)lParam)->code) {
        m_bSupressContextMenu=true;
      }
      break;
      */
    case WM_CONTEXTMENU:
      if(m_bSupressContextMenu){
        m_bSupressContextMenu=false;
          return true; // processed
      }
      break;
    default:
      break;
  }
  return false;
}


/////////////////////////////////////////////////////////////////////////////
// CCSHCommon message handlers

bool CCSHCommon::OnContextMenu(CWnd *pDialog, CPoint pt, UINT idHelp)
{
  bool rc=false;
  if(NULL!=m_pwndContext&& 0!=idHelp){
    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu(MF_STRING,ID_WHATS_THIS,_T("&What's This?"));
    menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x,pt.y,pDialog);
    rc=true;
  }
  return rc;
}
