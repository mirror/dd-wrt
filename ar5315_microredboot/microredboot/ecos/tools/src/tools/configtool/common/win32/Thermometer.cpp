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
// Thermometer.cpp: implementation of the CThermometer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Thermometer.h"
#include "ConfigTool.h"

#ifdef PLUGIN
  #define INCLUDEFILE "ide.common.h"
  #include "IncludeSTL.h"
  #include "common/StatusBar.h"
#else
  #include "MainFrm.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CThermometer::CThermometer(int nMax,LPCTSTR psz)
{
    TRACE(_T("Create thermometer nMax=%d string='%s'\n"),nMax,psz?psz:_T("<null>"));
    #ifdef PLUGIN
#ifndef _AFXEXT
  AFX_MANAGE_STATE(CConfigTool::CallerModuleState);
#endif
    AppInstance::getAppManager()->getStatusBar()->showProgressBar(psz?psz:_T(""), 0, nMax);
    #else
	if(CConfigTool::GetMain()){
		CConfigTool::GetMain()->SetThermometerMax(nMax);
        if(psz){
    		CConfigTool::GetMain()->SetIdleMessage(psz);
        }
    }
    #endif
}

CThermometer::~CThermometer()
{
    TRACE(_T("Destroy thermometer\n"));
    #ifdef PLUGIN
#ifndef _AFXEXT
  AFX_MANAGE_STATE(CConfigTool::CallerModuleState);
#endif
    AppInstance::getAppManager()->getStatusBar()->hideProgressBar();
    #else
	if(CConfigTool::GetMain()){
		CConfigTool::GetMain()->SetThermometerMax(0);
   		CConfigTool::GetMain()->SetIdleMessage(NULL);
    }
    #endif
}

void CThermometer::Set(int n)
{
    #ifdef PLUGIN
#ifndef _AFXEXT
  AFX_MANAGE_STATE(CConfigTool::CallerModuleState);
#endif
    AppInstance::getAppManager()->getStatusBar()->updateProgressBar(n);
    #else
	if(CConfigTool::GetMain()){
		CConfigTool::GetMain()->UpdateThermometer(n);
	}
    #endif
}

