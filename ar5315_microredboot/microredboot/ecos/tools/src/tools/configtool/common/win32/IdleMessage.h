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
// IdleMessage.h: interface for the CIdleMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IDLEMESSAGE_H__D2C84B41_378B_11D3_8567_A711BD1AC4A3__INCLUDED_)
#define AFX_IDLEMESSAGE_H__D2C84B41_378B_11D3_8567_A711BD1AC4A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIdleMessage  
{
public:
	void Reset();
	void Set(LPCTSTR psz);
	CIdleMessage(LPCTSTR pszMsg);
	virtual ~CIdleMessage();
protected:
    CString m_strPrevIdle;
};

#endif // !defined(AFX_IDLEMESSAGE_H__D2C84B41_378B_11D3_8567_A711BD1AC4A3__INCLUDED_)
