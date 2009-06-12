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
// CG: This file was added by the Splash Screen component.
// Splash.cpp : implementation file
//
//
//===========================================================================
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s): 	sdf
// Contact(s):	sdf
// Date:		1998/08/11
// Version:		0.01
// Purpose:	
// Description:	This is the implementation of the splash screen class.
// Requires:	
// Provides:	
// See also:    
// Known bugs:	
// Usage:	
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include "stdafx.h"
#include "resource.h"
#include "Splash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( CSplashWnd, CWnd )



/////////////////////////////////////////////////////////////////////////////
//   Splash Screen class

BOOL CSplashWnd::c_bShowSplashWnd;
CSplashWnd* CSplashWnd::c_pSplashWnd;
CSplashWnd::CSplashWnd()
{
}

CSplashWnd::~CSplashWnd()
{
	// Clear the static window pointer.
	ASSERT(c_pSplashWnd == this);
	c_pSplashWnd = NULL;
}

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
	//{{AFX_MSG_MAP(CSplashWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSplashWnd::EnableSplashScreen(BOOL bEnable /*= TRUE*/)
{
	if(c_pSplashWnd != NULL && !bEnable){
		c_pSplashWnd->HideSplashScreen();
	}
	c_bShowSplashWnd = bEnable;
}

void CSplashWnd::ShowSplashScreen(CWnd* pParentWnd /*= NULL*/)
{
	if (!c_bShowSplashWnd || c_pSplashWnd != NULL)
		return;

	// Allocate a new splash screen, and create the window.
	c_pSplashWnd = new CSplashWnd;
	if (!c_pSplashWnd->Create(pParentWnd))
	{
		deleteZ(c_pSplashWnd);
	}
	else
		c_pSplashWnd->UpdateWindow();
}

BOOL CSplashWnd::PreTranslateAppMessage(MSG* pMsg)
{
	if (c_pSplashWnd == NULL)
		return FALSE;

	// If we get a keyboard or mouse message, hide the splash screen.
	if (pMsg->message == WM_KEYDOWN ||
	    pMsg->message == WM_SYSKEYDOWN ||
	    pMsg->message == WM_LBUTTONDOWN ||
	    pMsg->message == WM_RBUTTONDOWN ||
	    pMsg->message == WM_MBUTTONDOWN ||
	    pMsg->message == WM_NCLBUTTONDOWN ||
	    pMsg->message == WM_NCRBUTTONDOWN ||
	    pMsg->message == WM_NCMBUTTONDOWN)
	{
		c_pSplashWnd->HideSplashScreen();
		return TRUE;	// message handled here
	}

	return FALSE;	// message not handled
}

BOOL CSplashWnd::Create(CWnd* pParentWnd /*= NULL*/)
{
	UINT image = IDB_SPLASH;
	if (!m_bitmap.LoadBitmap(image))
		return FALSE;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

	BOOL rc=CreateEx(0,
		AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		NULL, WS_POPUP | WS_VISIBLE, 0, 0, bm.bmWidth, bm.bmHeight, pParentWnd->GetSafeHwnd(), NULL);
	m_hBitmap = LoadResourceBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(image));
	m_pBitmap = CBitmap::FromHandle(m_hBitmap);
	return rc;
}

HBITMAP CSplashWnd::LoadResourceBitmap(HINSTANCE hInstance, LPCTSTR lpString)
                         
{
    HGLOBAL hGlobal;
    HBITMAP hBitmapFinal = NULL;
    LPBITMAPINFOHEADER  lpbi;
    HDC hdc;
	CDC* dc;
    int iNumColors;
    HRSRC  hRsrc = FindResource(hInstance, lpString, RT_BITMAP);
 
    if (hRsrc)
       {
       hGlobal = LoadResource(hInstance, hRsrc);
       lpbi = (LPBITMAPINFOHEADER)LockResource(hGlobal);
   	   CreateDIBPalette ((LPBITMAPINFO)lpbi, &iNumColors);

	   dc = GetDC();
       hdc = dc->GetSafeHdc();
       CPalette* oldpal = dc->SelectPalette(&Palette,FALSE);
	   dc->RealizePalette();
       hBitmapFinal = CreateDIBitmap(hdc,
                   (LPBITMAPINFOHEADER)lpbi,
                   (LONG)CBM_INIT,
                   (LPSTR)lpbi + lpbi->biSize + iNumColors *
					sizeof(RGBQUAD),
                    (LPBITMAPINFO)lpbi,
                   DIB_RGB_COLORS );
       dc->SelectPalette(oldpal,FALSE);
	   dc->RealizePalette();

       ReleaseDC( dc );
       UnlockResource(hGlobal);
       FreeResource(hGlobal);
       }
    return (hBitmapFinal);
}

void CSplashWnd::HideSplashScreen()
{
	// Destroy the window, and update the mainframe.
	DestroyWindow();
	CWnd *pWnd=AfxGetMainWnd();
	if(pWnd)
	{
		pWnd->UpdateWindow();
	}
}

void CSplashWnd::PostNcDestroy()
{
	// Free the C++ class.
	delete this;
}

int CSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Center the window.
	CenterWindow();

	// Set a timer to destroy the splash screen.
	SetTimer(1, 3000, NULL);

	return 0;
}

void CSplashWnd::OnPaint()
{
	CPaintDC dc(this);

	CDC dcImage;
	if (!dcImage.CreateCompatibleDC(&dc))
		return;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);
	// Paint the image.
	CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);
	dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
	dcImage.SelectObject(pOldBitmap);

	for(int i=0;i<bm.bmHeight;i++){
		COLORREF col=PALETTERGB(0,0,(255*i)/bm.bmHeight);
		CRect rect(0,i,bm.bmWidth,i+2);
		dcImage.FillSolidRect(&rect, col);
	}
	dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, MERGEPAINT);

}

void CSplashWnd::OnTimer(UINT nIDEvent)
{
	// Destroy the splash screen window.
	HideSplashScreen();
	UNUSED_ALWAYS(nIDEvent);
}

void CSplashWnd::CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpiNumColors)
{
   LPBITMAPINFOHEADER	lpbi;
   LPLOGPALETTE			lpPal;
   HANDLE				hLogPal;
   int					i;
 
   lpbi = (LPBITMAPINFOHEADER)lpbmi;
   if (lpbi->biBitCount <= 8)
       *lpiNumColors = (1 << lpbi->biBitCount);
   else
       *lpiNumColors = 0;  // No palette needed for 24 BPP DIB
 
   if (*lpiNumColors)
      {
      hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) +
                             sizeof (PALETTEENTRY) * (*lpiNumColors));
      lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
      lpPal->palVersion    = 0x300;
      lpPal->palNumEntries = (unsigned short)*lpiNumColors;
 
      for (i = 0;  i < *lpiNumColors;  i++)
         {
         lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
         lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
         lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
         lpPal->palPalEntry[i].peFlags = 0;
         }
      Palette.CreatePalette(lpPal);
	  GlobalUnlock (hLogPal);
      GlobalFree   (hLogPal);
   }
   return ;
}
