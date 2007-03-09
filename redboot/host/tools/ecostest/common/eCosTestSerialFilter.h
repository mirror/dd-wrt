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
//        eCosTestSerialFilter.h
//
//        Serial test filter class
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jskov
// Contributors:  jskov
// Date:          1999-03-01
//####DESCRIPTIONEND####
#ifndef _CECOSSERIALFILTER_H
#define _CECOSSERIALFILTER_H

#include "eCosStd.h"
#include "eCosTest.h"
#include "eCosSocket.h"
#include "eCosSerial.h"

//----------------------------------------------------------------------------
// Macros to help extract values from the argument string.
// Note: This is probably not an ideal solution, but it was easy to make :)

#define INIT_VALUE(__args)                \
    unsigned int v;                       \
    char *__ptr1, *__ptr2 = (__args)      \

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


//----------------------------------------------------------------------------
// Structures used by the filter.
struct filter_abort_t {
    const unsigned char* data_ptr;
    int data_len;
    
    filter_abort_t():
        data_ptr(NULL),
        data_len(0)
        {}
};

typedef enum {
    FLOW_NONE=0,
    FLOW_XONXOFF_RX=1,
    FLOW_XONXOFF_TX=2,
    FLOW_RTSCTS_RX=4,
    FLOW_RTSCTS_TX=8,
    FLOW_DSRDTR_RX=16,
    FLOW_DSRDTR_TX=32
} flow_cfg_t;

typedef struct ser_cfg {
    int baud_rate;
    int data_bits;
    CeCosSerial::StopBitsType stop_bits;
    bool parity;
    
    unsigned int flags;
    // etc...
} ser_cfg_t;

typedef enum {
    MODE_NO_ECHO = 0,
    MODE_EOP_ECHO,
    MODE_DUPLEX_ECHO
} cyg_mode_t;


//----------------------------------------------------------------------------
// The filter class
class CeCosTestSerialFilter;

class CeCosTestSerialFilter {
public:
    // Constructor
    CeCosTestSerialFilter();
    ~CeCosTestSerialFilter();

    // Configuration methods
    void SetConsoleOutput(bool bConsoleOutput) 
        { m_bOptConsoleOutput = bConsoleOutput; }
    void SetSerialDebug(bool bSerialDebug) 
        { m_bOptSerDebug = bSerialDebug; }
    void SetFilterTrace(bool bFilterTrace) 
        { m_bOptFilterTrace = bFilterTrace; }


    bool FilterFunctionProper(void*& pBuf,
                              unsigned int& nRead,
                              CeCosSerial& serial,
                              CeCosSocket& socket);

private:
    enum {MAX_CMD_LEN=128};
    enum data_origin_t {SF_TARGET=0, SF_FILTER} ;
    
    // Output methods
    void GDBWrite(const char* pszStr);
    void ConsoleWrite(const char* pszStr);
    void Trace(const char* pszFormat, ...);
    void Log(const char* pszFormat, ...);

    void PrintHex(const unsigned char* d1, int len, 
                  data_origin_t origin=SF_TARGET);

    // Target read/write methods
    void TargetWrite(CeCosSerial &pSer, 
                     const unsigned char* buffer, int len);
    void TargetASCIIWrite(CeCosSerial &pSer, const char* s);
    bool TargetRead(CeCosSerial &pSer, 
                    unsigned char* buffer, int len);
        
    // Configuration CMD and helper methods
    void ParseConfig(char* args, ser_cfg_t* new_cfg);
    bool SetConfig(CeCosSerial &pSer, const ser_cfg_t* new_cfg, 
                   ser_cfg_t* old_cfg);
    bool VerifyConfig(CeCosSerial &pSer, ser_cfg_t* new_cfg);
    void CMD_ChangeConfig(CeCosSerial &pSer, char* cfg_str);
    void CMD_DefaultConfig(CeCosSerial &pSer);

    // Other CMD methods.
    void CMD_TestBinary(CeCosSerial &pSer, char* args);
    void CMD_TestText(CeCosSerial &pSer, char* args);
    void CMD_TestPing(CeCosSerial &pSer, char* args);


    // Misc helper methods
    int DoCRC(unsigned char* data, int size);
    void SendChecksum(CeCosSerial &pSer, int crc);
    void SendStatus(CeCosSerial &pSer, int state);
    void ReceiveDone(CeCosSerial &pSer, unsigned char* data_in, int size);
    void DispatchCommand(CeCosSerial &pSer, char* cmd);

    // Options used for configuring behavior.
    bool m_bOptConsoleOutput;
    bool m_bOptSerDebug;
    bool m_bOptFilterTrace;

    // Buffer holding unread bytes.
    unsigned char* m_xUnreadBuffer;     // unread_buffer;
    int m_nUnreadBufferIndex;           // unread_buffer_ix;
    int m_nUnreadBufferSize;            // unread_buffer_size = 0;

    unsigned char* m_xStoredTraceBuffer;// We need this to avoid outputting
                                        // serial tracing when the target
                                        // last sent an incomplete packet, so
                                        // we store it here temporarily until
                                        // the entire packet arrives
    unsigned int m_nStoredTraceBufferSize; // size of above

    // Filter state
    bool m_bNullFilter;
    int  m_nCmdIndex;
    bool m_bCmdFlag;
    char m_aCmd[MAX_CMD_LEN];
    bool m_bFirstCommandSeen;           // We need this to avoid outputting
                                        // serial tracing while GDB is trying
                                        // to connect, or it will get confused.

    CeCosSocket* m_cGDBSocket;      // gdb_socket
};

extern bool CALLBACK SerialFilterFunction(void*& pBuf,
                                          unsigned int& nRead,
                                          CeCosSerial& serial,
                                          CeCosSocket& socket,
                                          void* pParem);
    
#endif // _CECOSSERIALFILTER_H
