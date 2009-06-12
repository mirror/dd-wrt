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
//        eCosTestMonitorFilter.cpp
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
// Description:   This filter sits between GDB and the test running on
//                the target, allowing all transmitted data to be output.
//
// *Take* the time to move some of the functions here (which are shared
// with eCosTestSerialFilter, and probably DownloadFilter) and move
// them to some shared file.
//####DESCRIPTIONEND####

#include "eCosStd.h"

#include "eCosTestMonitorFilter.h"

CeCosTestMonitorFilter::CeCosTestMonitorFilter():
  m_bOptVerbose(false)
{
}

CeCosTestMonitorFilter::~CeCosTestMonitorFilter()
{
}

//------------------------
// Output helpers.

void
CeCosTestMonitorFilter::ConsoleWrite(const char* pszStr)
{
    fputs(pszStr, stderr);
    fflush(stderr);
}

void
CeCosTestMonitorFilter::Trace(const char* pszFormat, ...)
{
  
  va_list marker;
  va_start (marker, pszFormat);
  
  for(int nLength=100;nLength;) {
    char *buf=new char[1+nLength];
    int n=vsnprintf(buf, nLength, pszFormat, marker ); 
    if(-1==n){
      nLength*=2;  // NT behavior
    } else if (n<nLength){
      ConsoleWrite(buf);
      nLength=0;   // trigger exit from loop
    } else {
      nLength=n+1; // UNIX behavior generally, or NT behavior when buffer size exactly matches required length
    }
    delete [] buf;
  }
  
  va_end (marker);
}

void
CeCosTestMonitorFilter::PrintHex(const unsigned char* d1, int len, data_origin_t origin/*=SF_TARGET*/)
{
    int offset = 0;
    int i;
    char buf[128];
    int width = 8;

    while (len) {
        int count = MIN(width, len);
        char* p = buf;
        switch (origin) {
        case MF_TARGET:
            p += sprintf(p, "T:");
            break;
        case MF_HOST:
            p += sprintf(p, "H:");
            break;
        }
        p += sprintf(p, "%04x > ", offset);
        // Print hex values.
        for (i = 0; i < count; i++)
            p += sprintf(p, "%02x ", d1[i]);
        for (     ; i < width   ; i++)
            p += sprintf(p, ".. ");

        // Print ASCII string
        p += sprintf(p, "'");
        for (i = 0; i < count; i++) {
            int c = d1[i];
            if (' ' >= c || 'z' <= c)
                c = '.';
            p += sprintf(p, "%c", c);
        }
        sprintf(p, "'\n");

        Trace("%s", buf);

        len -= count;
        offset += count;
        d1 += count;
    }
}

bool CALLBACK
SerialMonitorFunction(void*& pBuf,
                      unsigned int& nRead,
                      CeCosSerial& serial,
                      CeCosSocket& socket,
                      void* pParem)
{
    CeCosTestMonitorFilter* p = (CeCosTestMonitorFilter*) pParem;
    return p->FilterFunctionProper(pBuf, nRead);
}

bool
CeCosTestMonitorFilter::FilterFunctionProper(void*& pBuf, unsigned int& nRead)
{
    char* buffer = (char*) pBuf;

    if (m_bOptVerbose)
        PrintHex((unsigned char*) buffer, nRead, m_eOrigin);

    return true;
}
