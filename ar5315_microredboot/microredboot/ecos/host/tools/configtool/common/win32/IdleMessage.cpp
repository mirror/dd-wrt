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
// IdleMessage.cpp: implementation of the CIdleMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IdleMessage.h"
#ifdef PLUGIN
  #define INCLUDEFILE "ide.common.h"
  #include "IncludeSTL.h"
  #include "common/StatusBar.h"
#else
  #include "MainFrm.h"
#endif
#include "ConfigTool.h"

#ifdef _DEBUG
  #undef THIS_FILE
  static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIdleMessage::CIdleMessage(LPCTSTR pszMsg)
{
#ifdef PLUGIN
  AppInstance::getAppManager()->getStatusBar()->getIdleText().c_str();
#endif
  Set(pszMsg);
}

CIdleMessage::~CIdleMessage()
{
  Reset();
}

void CIdleMessage::Set(LPCTSTR pszMsg)
{
#ifdef PLUGIN
  AppInstance::getAppManager()->getStatusBar()->setIdleText(pszMsg);
#else
  if(CConfigTool::GetMain()){
    CConfigTool::GetMain()->SetIdleMessage(pszMsg);
  }
#endif
  
}

void CIdleMessage::Reset()
{
#ifdef PLUGIN
  AppInstance::getAppManager()->getStatusBar()->setIdleText((LPCTSTR)m_strPrevIdle);
#else
  if(CConfigTool::GetMain()){
    CConfigTool::GetMain()->SetIdleMessage();
  }
#endif
}
