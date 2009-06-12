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
//        eCosTestMonitorFilter.h
//
//        Simple filter for monitoring data flowing through the client
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jskov
// Contributors:  jskov
// Date:          2000-03-16
//####DESCRIPTIONEND####
#ifndef _CECOSMONITORFILTER_H
#define _CECOSMONITORFILTER_H

#include "eCosStd.h"
#include "eCosTest.h"
#include "eCosSocket.h"
#include "eCosSerial.h"

//----------------------------------------------------------------------------
// The filter class
class CeCosTestMonitorFilter;

class CeCosTestMonitorFilter {
public:
    enum data_origin_t {MF_TARGET=0, MF_HOST} ;

    // Constructor
    CeCosTestMonitorFilter();
    ~CeCosTestMonitorFilter();

    // Configuration methods
    void SetVerbose(bool bVerbose) 
        { m_bOptVerbose = bVerbose; }
    void SetOrigin(data_origin_t eOrigin)
        { m_eOrigin = eOrigin; }

    bool FilterFunctionProper(void*& pBuf, unsigned int& nRead);

private:

    // Output methods
    void ConsoleWrite(const char* pszStr);
    void Trace(const char* pszFormat, ...);
    void PrintHex(const unsigned char* d1, int len, 
                  data_origin_t origin=MF_TARGET);

        
    // Options used for configuring behavior.
    bool m_bOptVerbose;

    data_origin_t m_eOrigin;

};

extern bool CALLBACK SerialMonitorFunction(void*& pBuf,
                                           unsigned int& nRead,
                                           CeCosSerial& serial,
                                           CeCosSocket& socket,
                                           void* pParem);
    
#endif // _CECOSMONITORFILTER_H
