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
//        eCosTestDownloadFilter.h
//
//        Socket/serial download filter class
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jskov
// Contributors:  jskov
// Date:          1999-09-20
//####DESCRIPTIONEND####
#ifndef _CECOSDOWNLOADFILTER_H
#define _CECOSDOWNLOADFILTER_H

#include "eCosStd.h"
#include "eCosTest.h"
#include "eCosSocket.h"
#include "eCosSerial.h"

//----------------------------------------------------------------------------
// Macros to help extract values from the argument string.
// Note: This is probably not an ideal solution, but it was easy to make :)

#define INIT_VALUE(__args)                      unsigned int v; char *__ptr1, *__ptr2 = (__args)

#define SET_VALUE(__type, __slot) {          \
    __ptr1 = strchr(__ptr2, (int) ':');      \
    if (*__ptr2 == '\0')                     \
           (__slot) = (__type)-1;            \
    else {                                   \
        if (__ptr1)                          \
            *__ptr1 = 0;                     \
        else                                 \
            __ptr1 = strchr( __ptr2, 0) - 1; \
        v = atoi(__ptr2);                    \
        __ptr2 = __ptr1+1;                   \
        (__slot) = (__type) v;               \
    }                                        \
}

#define SET_STRING(__slot)               {    __ptr1 = strchr(__ptr2, (int) ':'); if (__ptr1) *__ptr1 = 0; __slot = __ptr2; __ptr2 = __ptr1+1; }


//----------------------------------------------------------------------------
// The filter class
class CeCosTestDownloadFilter;

class CeCosTestDownloadFilter {
public:
    // Constructor
    CeCosTestDownloadFilter();
    ~CeCosTestDownloadFilter();

    // Configuration methods
    void SetSerialDebug(bool bSerialDebug) 
        { m_bOptSerDebug = bSerialDebug; }
    void SetFilterTrace(bool bFilterTrace) 
        { m_bOptFilterTrace = bFilterTrace; }


    bool ContinueSession() 
        { bool r = m_bContinueSession; m_bContinueSession = false; return r; }

    bool FilterFunctionProper(void*& pBuf,
                              unsigned int& nRead,
                              CeCosSerial& serial,
                              CeCosSocket& socket);

private:
    // If we can guarantee a minimum buffer size in the stub, we can
    // increase PBUFSIZE.
    enum {MAX_CMD_LEN=128, PBUFSIZ=400};
    enum data_origin_t {SF_TARGET=0, SF_FILTER} ;

    // Output methods
    void ConsoleWrite(const char* pszStr);
    void Trace(const char* pszFormat, ...);

    void PrintHex(const unsigned char* d1, int len, 
                  data_origin_t origin=SF_TARGET);

    // Target read/write methods
    void TargetWrite(CeCosSerial &pSer, 
                     const unsigned char* buffer, int len);
        
    // GDB stuff
    int tohex(int nib);
    int hexnumstr(unsigned char* buf, unsigned long num);
    int hexnumlen(unsigned long num);
    int put_binary (unsigned char* buf, int cnt, unsigned long dl_address,
                    int packet_size, CeCosSerial& serial);

    // Options used for configuring behavior.
    bool m_bNullFilter;
    bool m_bOptSerDebug;
    bool m_bOptFilterTrace;

    // Filter state
    int  m_nCmdIndex;
    bool m_bCmdFlag;
    char m_aCmd[MAX_CMD_LEN];
    bool m_bContinueSession;            // set true only after a successfull dl
};

extern bool CALLBACK DownloadFilterFunction(void*& pBuf,
                                            unsigned int& nRead,
                                            CeCosSerial& serial,
                                            CeCosSocket& socket,
                                            void* pParem);
    
#endif // _CECOSDOWNLOADFILTER_H
