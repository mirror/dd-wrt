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
//=================================================================
//
//        X10.h
//
//        X10 class
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   Class to drive the X10 controller. This header file is host-independent.
// Usage:
//
//####DESCRIPTIONEND####
#ifndef _CX10_H
#define _CX10_H
#include "eCosSerial.h"

class CX10 {
public:

	CX10(LPCTSTR pszPort);
	virtual ~CX10();
	bool Power(bool bOn,int cControl1,int cControl2,int &nErr);	// Perform a reset on this port, with these control chars
	bool Ok(){return m_Serial.Ok();}
protected:
  CeCosSerial m_Serial;
	bool PutChar (unsigned char c);
	bool GetChar (unsigned char &c);
	bool ReceiveIPS();
  bool SetClock();
	bool Open();
	bool Close();
	LPCTSTR m_pszPort;
  bool Send( char c1, char c2,int &nErr);	// Send it these bytes and verify the returned checksum
};
#endif
