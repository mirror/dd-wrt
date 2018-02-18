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
#include "TrayIcon.h"

#include <windows.h>
#include <shellapi.h>

#include "Frontend.h"
#include "FrontendDlg.h"
#include "Resource.h"

#define TRAYICONID 987435

HWND s_hWnd;
TrayIcon* TrayIcon::instance = NULL;

LRESULT CALLBACK TrayIconProc( HWND, UINT, WPARAM, LPARAM lParam )
{
	switch (lParam)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		TrayIcon::instance->displayPopup();
		break;
	}
	
	return 1;
}

void TrayIcon::displayPopup()
{
	HMENU hMenu = LoadMenu(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_TRAYMENU));
	HMENU hSubMenu = GetSubMenu(hMenu, 0);

	EnableMenuItem(hSubMenu, IDM_STOP,
		MF_BYCOMMAND | (main_dlg.m_StopButton.IsWindowEnabled() ? MF_ENABLED : MF_GRAYED));
	
	EnableMenuItem(hSubMenu, IDM_START,
		MF_BYCOMMAND | (main_dlg.m_StartButton.IsWindowEnabled() ? MF_ENABLED : MF_GRAYED));

	POINT pt;
	
	GetCursorPos(&pt);
	
	SetForegroundWindow( s_hWnd );
	
	int cmd = TrackPopupMenu(hSubMenu,
		TPM_RIGHTBUTTON | TPM_RETURNCMD, 
		pt.x, 
		pt.y, 
		0, 
		s_hWnd, 
		NULL);
	
	PostMessage(s_hWnd, WM_NULL, 0, 0); 
	
	switch(cmd)
	{
	case IDM_EXIT:
		main_dlg.OnExitButton();
		break;
		
	case IDM_WINDOW:
		main_dlg.OpenIcon();
		main_dlg.SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		break;

	case IDM_SETTINGS:
		main_dlg.m_TabCtrl.SetCurSel(0);
		main_dlg.m_TabCtrl.DisplayTabDialog();
		main_dlg.OpenIcon();
		main_dlg.SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		break;
		
	case IDM_OUTPUT:
		main_dlg.m_TabCtrl.SetCurSel(1);
		main_dlg.m_TabCtrl.DisplayTabDialog();
		main_dlg.OpenIcon();
		main_dlg.SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		break;
		
	case IDM_STOP:
		main_dlg.OnStopButton();
		break;
		
	case IDM_START:
		main_dlg.OnStartButton();
		break;
	}
	
	DestroyMenu(hMenu);
}

void TrayIcon::setStatus( status con_stat, const char* message )
{
	switch( con_stat )
	{
	case CONNECTED:
		setTrayAppearance( false, IDI_ICON2, message );
		break;
		
	case OFF:
		setTrayAppearance( false, IDI_ICON3, message );
		break;
		
	case ON:
		setTrayAppearance( false, IDI_ICON1, message );
		break;
	}
}

void TrayIcon::setTrayAppearance( bool start, unsigned int res_id, const char* message )
{
	NOTIFYICONDATA notifyIconData;
	
	notifyIconData.cbSize = sizeof(notifyIconData);
	notifyIconData.hWnd = s_hWnd;
	
	notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIconData.uCallbackMessage = 123456;
	notifyIconData.uID = TRAYICONID; 
	notifyIconData.hIcon = (HICON)LoadIcon( hInst, MAKEINTRESOURCE( res_id ) );  
	strncpy( notifyIconData.szTip, message, sizeof (notifyIconData.szTip) ); 
	
	if( start )
		Shell_NotifyIcon( NIM_ADD, &notifyIconData );
	else
		Shell_NotifyIcon( NIM_MODIFY, &notifyIconData );
}

TrayIcon::TrayIcon( CFrontendDlg& dlg, HINSTANCE hInst ) : main_dlg( dlg ), hInst( hInst )
{
	instance = this;
	WNDCLASS wndClass;
	
	wndClass.style = 0; 
	wndClass.lpfnWndProc = TrayIconProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInst;
	wndClass.hIcon = NULL;
	wndClass.hCursor = NULL;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "OLSRdTrayIcon";
	
	if (0 != RegisterClass(&wndClass))
	{
		s_hWnd = CreateWindow( 
			"OLSRdTrayIcon", 
			"Invisible Message Window", 
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			hInst,
			NULL);
		
		if (s_hWnd)
		{
			ShowWindow(s_hWnd, SW_HIDE);
			
			UpdateWindow(s_hWnd);
		}
	}
	
	setTrayAppearance( true, IDI_ICON3, "Starting..." );
}

TrayIcon::~TrayIcon()
{
	NOTIFYICONDATA notifyIconData;
	
	notifyIconData.cbSize = sizeof(notifyIconData);
	notifyIconData.hWnd = s_hWnd;
	
	notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIconData.uCallbackMessage = 123456;
	notifyIconData.uID = TRAYICONID; 
	notifyIconData.hIcon = NULL;
	
	Shell_NotifyIcon( NIM_DELETE, &notifyIconData );
}

#endif /* _WIN32 */
