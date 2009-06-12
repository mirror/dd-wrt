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
//        eCosTestSerialFilter.cpp
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
// Description:   This filter sits between GDB and the test running on
//                the target, allowing testing of the serial driver 
//                without confusing GDB.
// To Do:
//  o Add timeout setup and handling for recovery, can rely on testing
//    agent to control global timeout.
//  o Saving chunks that caused transfer failure?
//     - In SEND with echo, do CRC on 32-byte sub-packets
//  o Additional To Do items under each sub-protocol function.
//  o Option to get all serial IO written (in hex, > and < prepends 
//    input/output lines) to a file.
//  o Clean up the mess in this file....
//####DESCRIPTIONEND####

#include "eCosStd.h"

#include "eCosTestSerialFilter.h"
#include "eCosThreadUtils.h"

char msg_ok[] = "OK";
char msg_er[] = "ER";



CeCosTestSerialFilter::CeCosTestSerialFilter():
  m_bOptConsoleOutput(false), 
  m_bOptSerDebug(false), 
  m_bOptFilterTrace(false),
  m_xUnreadBuffer(NULL),
  m_nUnreadBufferIndex(0), 
  m_nUnreadBufferSize(0), 
  m_xStoredTraceBuffer(NULL),
  m_nStoredTraceBufferSize(0),
  m_bNullFilter(false), 
  m_nCmdIndex(0), 
  m_bCmdFlag(false), 
  m_bFirstCommandSeen(false),
  m_cGDBSocket(NULL)
{
}

CeCosTestSerialFilter::~CeCosTestSerialFilter()
{
}

//------------------------
// Output helpers.

// Encode string in an O-packet and send it to GDB.
void 
CeCosTestSerialFilter::GDBWrite(const char* pszStr)
{
    if (m_cGDBSocket) {
        static const char hexchars[] = "0123456789abcdef";
        char* packet = new char[strlen(pszStr)*2+6];
        char* p = packet;

        *p++ = '$';
        *p++ = 'O';
        unsigned char crc = 'O';
        char c;
        for (;;) {
            c = *pszStr++;
            if (0 == c)
                break;

            char h = hexchars[(c >> 4) & 0x0f];
            char l = hexchars[c & 0x0f];
            *p++ = h;
            *p++ = l;
            crc = (unsigned char) (crc + h + l);
        };

        *p++ = '#';
        *p++ = hexchars[(crc >> 4) & 0x0f];
        *p++ = hexchars[crc & 0x0f];
        
        // Only try to send once. If it fails, it's probably because
        // GDB has disconnected.
        m_cGDBSocket->send(packet, p - packet);
        m_cGDBSocket->recv(&c, 1);

        delete [] packet;
    }
}

void
CeCosTestSerialFilter::ConsoleWrite(const char* pszStr)
{
    fputs(pszStr, stderr);
    fflush(stderr);
}

void
CeCosTestSerialFilter::Trace(const char* pszFormat, ...)
{
  
  va_list marker;
  va_start (marker, pszFormat);
  
  for(int nLength=100;nLength;) {
    char *buf=new char[1+nLength];
    int n=vsnprintf(buf+4, nLength-4, pszFormat, marker ); 
    if(-1==n){
      nLength*=2;  // NT behavior
    } else if (n<nLength){
      memcpy(buf,"[f] ",4);
      if (m_bOptConsoleOutput) {
        ConsoleWrite(buf);
      } else {
        GDBWrite(buf);
      }
      nLength=0;   // trigger exit from loop
    } else {
      nLength=n+1; // UNIX behavior generally, or NT behavior when buffer size exactly matches required length
    }
    delete [] buf;
  }
  
  va_end (marker);
  
}

void
CeCosTestSerialFilter::Log(const char* pszFormat, ...)
{
  va_list marker;
  va_start (marker, pszFormat);
  
  for(int nLength=100;nLength;) {
    char *buf=new char[1+nLength];
    int n=vsnprintf(buf, nLength, pszFormat, marker ); 
    if(-1==n){
      nLength*=2;  // NT behavior
    } else if (n<nLength){
      if (m_bOptConsoleOutput) {
        ConsoleWrite(buf);
      } else {
        GDBWrite(buf);
      }
      nLength=0;   // trigger exit from loop
    } else {
      nLength=n+1; // UNIX behavior generally, or NT behavior when buffer size exactly matches required length
    }
    delete [] buf;
  }
  
  va_end (marker);
}


void
CeCosTestSerialFilter::PrintHex(const unsigned char* d1, int len, data_origin_t origin/*=SF_TARGET*/)
{
    int offset = 0;
    int i;
    char buf[128];
    int width = 8;

    while (len) {
        int count = MIN(width, len);
        char* p = buf;
        switch (origin) {
        case SF_TARGET:
            p += sprintf(p, "T");
            break;
        case SF_FILTER:
            p += sprintf(p, "F");
            break;
        }
        p += sprintf(p, ":%04x ", offset);
        // Print hex values.
        for (i = 0; i < count; i++)
            p += sprintf(p, "%02x ", d1[i]);
        for (     ; i < width   ; i++)
            p += sprintf(p, ".. ");

        // Print ASCII string
        p += sprintf(p, "'");
        for (i = 0; i < count; i++) {
            int c = d1[i];
            if (' ' > c || 'z' < c)
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

void
CeCosTestSerialFilter::TargetWrite(CeCosSerial &pSer, 
                                    const unsigned char* buffer, int len)
{
    unsigned int __written;

    if (m_bOptFilterTrace)
        PrintHex(buffer, len, SF_FILTER);

    do {
        if (!(pSer.Write((void*) buffer, len, __written))) {
            fprintf(stderr, "Writing %d bytes to serial failed\n", len);
            fprintf(stderr, "%s", (LPCTSTR)pSer.ErrString());
            throw "serial write failed";
        }
        buffer += __written;
        len -= __written;
    } while (len);

}


bool
CeCosTestSerialFilter::TargetRead(CeCosSerial &pSer, 
                                  unsigned char* buffer, int len)
{
    unsigned int __read;
    int __total_read = 0;
    unsigned char* __buffer_base = buffer;
    int __timeouts = 0;
    int __timeout_failure = 0;
    int __orig_len = len;

    do {
        // First check for unread data.
        if (m_nUnreadBufferSize) {
            int i = 0;
            __read = 0;
            while (i < len && m_nUnreadBufferIndex < m_nUnreadBufferSize) {
                buffer[i++] = m_xUnreadBuffer[m_nUnreadBufferIndex++];
                __read++;
            }
            
            if (m_nUnreadBufferIndex == m_nUnreadBufferSize) {
                free(m_xUnreadBuffer);
                m_nUnreadBufferSize = 0;
                m_nUnreadBufferIndex = 0;
            }
        } else { 
            // Then read directly from serial.
            if (!(pSer.Read((void*) buffer, len, __read))) {
                fprintf(stderr,"Reading %d bytes from serial failed (read %d).\n",
                        len, __read);
                char *pszErr=pSer.ErrString().GetCString();
                fprintf(stderr, "%s", pszErr);
                delete [] pszErr;
                throw "serial read failed";
            }
        }

        __total_read += __read;
        unsigned int i;
        for (i = 0; i < __read; i++) {
            if ('$' == buffer[i]) {

                Log("FAIL:<target crashed>\n"); 

                Trace("**** Detected $ -- resuming as null filter ****\n");

                Trace("Data received %d bytes (of %d) from target:\n", 
                      __total_read, __orig_len);
                PrintHex(__buffer_base, __total_read);
                Trace("<end>\n");

                filter_abort_t* msg = new filter_abort_t();
                msg->data_ptr = &buffer[i];
                msg->data_len = __read - i;

                throw msg;
            }
        }

        if (0 == __read) {
            CeCosThreadUtils::Sleep(20);
            __timeouts++;
            if (25 == __timeouts) {
                __timeouts = 0;
                if (5 == __timeout_failure++) {
                    Log("FAIL:<target timed out>\n"); 

                    Trace("**** Timed out while reading -- resuming as null filter\n");

                    Trace("Data received %d bytes (of %d) from target:\n", 
                          __total_read, __orig_len);
                    PrintHex(__buffer_base, __total_read);
                    Trace("<end>\n");

                    static const char kill_msg[] = "$X00#b8";

                    filter_abort_t* msg = new filter_abort_t();
                    msg->data_len = strlen(kill_msg);
                    msg->data_ptr = (const unsigned char *)kill_msg;

                    throw msg;
                }
            }
        } else {
            __timeouts = 0;
            __timeout_failure = 0;
        }

        buffer += __read;
        len -= __read;

    } while (len);

    return true;
}

// Send C ASCII string to target.
void 
CeCosTestSerialFilter::TargetASCIIWrite(CeCosSerial &pSer, const char* s) 
{ 
    TargetWrite(pSer, (const unsigned char*) s, strlen(s)); 
}

//------------------------
// Configuration Command.
// Set serial configuration.
bool
CeCosTestSerialFilter::SetConfig(CeCosSerial &pSer, 
                                 const ser_cfg_t* new_cfg, 
                                 ser_cfg_t* old_cfg)
{
    // Note that for flow control, we assume that *both* receive and transmit
    // flow control are set or not set
    if (old_cfg) {
        old_cfg->baud_rate = pSer.GetBaud();
        old_cfg->parity = (0 != pSer.GetParity()) ? true : false;
        old_cfg->data_bits = pSer.GetDataBits();
        old_cfg->stop_bits = pSer.GetStopBits();
        old_cfg->flags = pSer.GetXONXOFFFlowControl() ? FLOW_XONXOFF_RX : 0;
        old_cfg->flags |= pSer.GetRTSCTSFlowControl() ? FLOW_RTSCTS_RX : 0;
        old_cfg->flags |= pSer.GetDSRDTRFlowControl() ? FLOW_DSRDTR_RX : 0;
    }

    pSer.SetBaud(new_cfg->baud_rate, false);
    pSer.SetParity(new_cfg->parity, false);
    pSer.SetDataBits(new_cfg->data_bits, false);
    pSer.SetXONXOFFFlowControl((new_cfg->flags&FLOW_XONXOFF_RX) != 0, false);
    pSer.SetRTSCTSFlowControl((new_cfg->flags&FLOW_RTSCTS_RX) != 0, false);
    pSer.SetDSRDTRFlowControl((new_cfg->flags&FLOW_DSRDTR_RX) != 0, false);
    return pSer.SetStopBits(new_cfg->stop_bits, true); // apply settings
}

// Return false if the serial configuration is not valid for the host.
bool
CeCosTestSerialFilter::VerifyConfig(CeCosSerial &pSer, ser_cfg_t* new_cfg)
{
    ser_cfg_t old_cfg;
    bool rc;

    // Try changing to the new config, recording the result. Then restore
    // the original config.
    rc = SetConfig(pSer, new_cfg, &old_cfg);
    SetConfig(pSer, &old_cfg, NULL);

    return rc;
}

//-----------------------------------------------------------------------------
// Configuration changing function.
//
// First change to the new config and back again to determine if the driver
// can handle the config.
// If not, return error.
//
// Then query the host for its capability to use the config:
// Format out:
//  "@CONFIG:<baud rate code>:<#data bits>:<#stop bits>:<parity on/off>!"
// Format in:
//  OK/ER
//
// On ER, return error.
//
// On OK, change to the new configuration. Resynchronize with the host:
//  Target waits for host to send S(ync) 
//     [host will delay at least .1 secs after changing baud rate so the 
//      line has time to settle.]
//
//  When receiving S(ync), target replies OK to the host which then
//  acknowledges with D(one).
//
//  Host can also send R(esync) which means it didn't receieve the OK. If
//  so the target resends its S(ync) message.
//
// If the synchronization has not succeeded within 1 second
// (configurable in the protocol), both host and target will revert to
// the previous configuration and attempt to synchronize again. If
// this fails, this call will hang and the host will consider the test
// a failure.
//
// To Do:
//  Host&protocol currently only supports:
//   - no/even parity
void
CeCosTestSerialFilter::CMD_ChangeConfig(CeCosSerial &pSer, char* cfg_str)
{
    ser_cfg_t new_cfg, old_cfg;

    ParseConfig(cfg_str, &new_cfg);

    // Return without changing the config if it's not valid.
    if (!VerifyConfig(pSer, &new_cfg)) {
        TargetASCIIWrite(pSer, "ER");
        return;
    }

    // Tell target we're ready to go, wait 1/10 sec, and then change
    // the config.
    TargetASCIIWrite(pSer, "OK");
    CeCosThreadUtils::Sleep(100);
    SetConfig(pSer, &new_cfg, &old_cfg);

    int loops;
    for (loops = 0; loops < 3; loops++) {
        unsigned int len, read;
        unsigned char buffer[2];
        int delay_mticks = 0; // millisecond-ticks. 10 of these per target tick

        // Start by sending a Sync.
        TargetASCIIWrite(pSer, "S");
        for(;;) {
            // Did target reply?
            len = 2;
            read = 0;
            buffer[0] = 0;
            buffer[1] = 0;
            if (!pSer.Read((void*) buffer, len, read)) {
                throw "CMD_ChangeConfig: serial read failure";
            }
            
            if (read) {
                // If only one char read, try to get the next one.
                if (1 == read) {
                    unsigned int read2 = 0;
                    len = 1;
                    if (!pSer.Read((void*) &buffer[1], len, read2)) {
                        throw "CMD_ChangeConfig: serial read failure";
                    }
                    read += read2;
                }

                if (m_bOptSerDebug)
                    PrintHex(buffer, read);

                if ('O' == buffer[0] && 'K' == buffer[1]) {
                    // success!
                    TargetASCIIWrite(pSer, "D");
                    Trace("Config change succeeded.\n");
                    return;
                } else {
                    // Garbage, ask target to resend its OK message.
                    TargetASCIIWrite(pSer, "R");
                }
            } else {
                // Resend Sync message.
                TargetASCIIWrite(pSer, "S");
            }

            CeCosThreadUtils::Sleep(1);
            delay_mticks++;
            // Timeout.
            if (100 == delay_mticks/10)
                break;
        }

        SetConfig(pSer, &old_cfg, NULL);
    }

    // Abort the test.
    Log("FAIL:<target timed out>\n"); 
    Trace("**** Timed out while changing config\n");

    static const char kill_msg[] = "$X00#b8";

    filter_abort_t* msg = new filter_abort_t();
    msg->data_len = strlen(kill_msg);
    msg->data_ptr = (const unsigned char *)kill_msg;

    throw msg;
}

// Set default configuration.
void
CeCosTestSerialFilter::CMD_DefaultConfig(CeCosSerial &pSer)
{
    static const ser_cfg_t default_ser_cfg = { 9600, 
                                               8, 
                                               CeCosSerial::ONE_STOP_BIT,
                                               false };

    TargetASCIIWrite(pSer, "OK");
    SetConfig(pSer, &default_ser_cfg, NULL);
}

// Parse config string from target and set new_cfg accordingly.
// String from target is:
//  <baud rate>:<data bits>:<stop bits>:<parity>:....
void
CeCosTestSerialFilter::ParseConfig(char* args, ser_cfg_t* new_cfg)
{
    int ecos_parity, ecos_stop_bits, ecos_baud_rate, ecos_flags;

    CeCosSerial::StopBitsType t2h_stop_bits[3] = {
        CeCosSerial::ONE_STOP_BIT, 
        CeCosSerial::ONE_POINT_FIVE_STOP_BITS,
        CeCosSerial::TWO_STOP_BITS};
    INIT_VALUE(args);

    SET_VALUE(int, ecos_baud_rate);
    SET_VALUE(int, new_cfg->data_bits);
    SET_VALUE(int, ecos_stop_bits);
    SET_VALUE(int, ecos_parity);
    SET_VALUE(int, ecos_flags);

    new_cfg->parity = (ecos_parity != 0) ? true : false;
    new_cfg->stop_bits = t2h_stop_bits[ecos_stop_bits - 1];

    // flags is an optional field
    if ( -1 == ecos_flags )
        new_cfg->flags = FLOW_NONE;
    else
        new_cfg->flags = ecos_flags;

    // eCos->human translation of serial baud rate. This table must
    // match the one in io/serial/current/include/serialio.h
    static const int tt_baud_rate[] = {
        -1,                                 // 0 invalid
        50,                                 // 1 50
        75,                                 // 2 75
        110,                                // 3
        135,                                // 4 134_5
        150,                                // 5
        200,                                // 6 200
        300,                                // 7
        600,                                // 8
        1200,                               // 9
        1800,                               // 10 1800
        2400,                               // 11
        3600,                               // 12 3600
        4800,                               // 13
        7200,                               // 14 7200
        9600,                               // 15
        14400,                              // 16 14400
        19200,                              // 17
        38400,                              // 18
        57600,                              // 19
        115200,                             // 20
        234000                              // 21 234000
    };

    if (ecos_baud_rate > 0 && ecos_baud_rate < (int) sizeof(tt_baud_rate))
        ecos_baud_rate = tt_baud_rate[ecos_baud_rate];
    else
        ecos_baud_rate = -2;

    new_cfg->baud_rate = ecos_baud_rate;

    Trace("Parsed Config baud=%d, bParity=%d, stopbits=%d, databits=%d\n",
          new_cfg->baud_rate, (int) new_cfg->parity, new_cfg->stop_bits,
          new_cfg->data_bits);
    Trace("Parsed Config xonxoff_rx=%d,tx=%d, rtscts_rx=%d,tx=%d, "
          "dsrdtr_rx=%d,tx=%d\n",
          (new_cfg->flags & FLOW_XONXOFF_RX) != 0,
          (new_cfg->flags & FLOW_XONXOFF_TX) != 0,
          (new_cfg->flags & FLOW_RTSCTS_RX) != 0,
          (new_cfg->flags & FLOW_RTSCTS_TX) != 0,
          (new_cfg->flags & FLOW_DSRDTR_RX) != 0,
          (new_cfg->flags & FLOW_DSRDTR_TX) != 0);
}

// Always make sure CRC fits in 31 bits. Bit of a hack, but we want
// to send CRC as ASCII without too much hassle.
int
CeCosTestSerialFilter::DoCRC(unsigned char* data, int size)
{
    int i;
    unsigned long crc;

    for (i = 0, crc = 0; i < size; i++) {
        crc = (crc << 1) ^ data[i];     // FIXME: standard definition?
    }

    i = (int) crc;
    if (i < 0)
        i = -i;

    return i;
}

void
CeCosTestSerialFilter::SendChecksum(CeCosSerial &pSer, int crc)
{
    char buffer[128];
    int len;

    len = sprintf(buffer, "%d!", crc);

    TargetWrite(pSer, (const unsigned char*)buffer, len);
}

void
CeCosTestSerialFilter::SendStatus(CeCosSerial &pSer, int state)
{
    if (state)
        TargetWrite(pSer, (unsigned char*) &msg_ok, 2);
    else
        TargetWrite(pSer, (unsigned char*) &msg_er, 2);
}


// Receive test DONE message from target.
void
CeCosTestSerialFilter::ReceiveDone(CeCosSerial &pSer, 
                                   unsigned char* data_in, int size)
{
    static const char msg_done[] = "DONE";
    unsigned char data_reply[4];
    int first = 1;

    TargetRead(pSer, data_reply, 4);
    while (0 != strncmp((char*) data_reply, msg_done, 4)) {
        if (first) {
            if (data_in && size) {
                Trace("Data received from target:\n");
                PrintHex(data_in, size);
                Trace("<end>\n");
            }
            Trace("Receiving junk instead of DONE:\n");
            first = 0;
        }
        PrintHex(data_reply, 4);

        data_reply[0] = data_reply[1];
        data_reply[1] = data_reply[2];
        data_reply[2] = data_reply[3];

        // The TargetRead call will handle recovery in case of timeout...
        TargetRead(pSer, &data_reply[3], 1);
    }
}

//-----------------------------------------------------------------------------
// Test binary data transmission.
// Format in:
//  <byte size>:<mode>
// Format out:
//  <4 bytes binary checksum><#size bytes data>
// If echo mode, also:
//    Format in:
//     <#size bytes data>
//    Format out:
//     OK/ER - according to CRC match on incomin data
// Format in:
//  DONE
//
// To Do:
//  o Add mode/flag specifying 5-8 bit transfer.
//     Test that 0xff gets masked off accordingly when transfered.
//     (This should be an INFO result if failing)
//  o Clean up the DUPLEX_ECHO implementation. Currently it's an ugly hack
//    that doesn't match the arguments / behavior of the two other modes.
void
CeCosTestSerialFilter::CMD_TestBinary(CeCosSerial &pSer, char* args)
{
    int size;
    cyg_mode_t mode;
    unsigned char *data_out, *data_in;
    int i;
    int crc;

    int loop_count = 0;

    INIT_VALUE(args);

    SET_VALUE(int, size);
    SET_VALUE(cyg_mode_t, mode);

    // Change behavior for DUPLEX mode.
    if (MODE_DUPLEX_ECHO == mode) {
        loop_count = size;
        size = 1024;                    // must be at least 4*block_size
    }

    // Generate data.
    data_out = (unsigned char*) malloc(size);
    if (!data_out) {
        fprintf(stderr, "Could not allocate %d byte buffer for data!\n", size);
        throw "data_out malloc failed";
    }
    data_in = (unsigned char*) malloc(size);
    if (!data_in) {
        fprintf(stderr, "Could not allocate %d byte buffer for data!\n", size);
        throw "data_in malloc failed";
    }
    int count = 0;
    for (i = 0; i < size; i++) {
        // Output 255 chars, not 256 so that we aren't a multiple/factor of the
        // likely buffer sizes in the system, this can mask problems as I've
        // found to my cost!
        unsigned char c = (unsigned char) (count++ % 255);
        // don't allow $s and @s in the data, nor 0x03 (GDB C-c), nor flow
        // control chars
        if ('$' == c || '@' == c || 0x03 == c || 0x11 == c || 0x13 == c)
            c = (unsigned char) '*';
        data_out[i] = c;
    }

    // Do checksum.
    crc = DoCRC(data_out, size);

    // Send checksum to target.
    SendChecksum(pSer, crc);

    // Give the target 1/10th of a sec to digest it
    CeCosThreadUtils::Sleep(100);

    switch (mode) {
    case MODE_NO_ECHO:
    {
        // Simple transmit. Don't expect target to echo data back.
        TargetWrite(pSer, data_out, size);
        ReceiveDone(pSer, NULL, 0);
    }
    break;
    case MODE_EOP_ECHO:
    {
        int in_crc;

        TargetWrite(pSer, data_out, size);
        Trace("Finished write, waiting for target echo.\n");

        // Expect target to echo the data
        TargetRead(pSer, data_in, size);

        // Check echoed data, and reply OK/ER accordingly.
        in_crc = DoCRC(data_in, size);
        SendStatus(pSer, (in_crc == crc));


        // Dump seen/expected on console.
        if (in_crc != crc) {
            Trace("Data seen:\n");
            PrintHex(data_in, size);
            Trace("<end>\n");
            Trace("Data expected:\n");
            PrintHex(data_out, size);
            Trace("<end>\n");
        }

        ReceiveDone(pSer, data_in, size);

    }
    break;
    case MODE_DUPLEX_ECHO:
    {
        int block_size = 64;
        int fail, j;

        // This is a simple implementation (maybe too simple).
        // Host sends 4 packets with the same size (64 bytes atm).
        // Target echoes in this way:
        //  packet1 -> packet1
        //  packet2 -> packet2, packet2
        //  packet3 -> packet3
        //  packet4 -> /dev/null
        //
        // The reads/writes are interleaved in a way that should ensure
        // the target out buffer to be full before the target starts to read
        // packet3. That is, the target should be both receiving (packet3)
        // and sending (packet2) at the same time.

        // This code needs restructuring. It's not very obvious what's
        // happening: The same block of data is output several times,
        // the target echoes the data back (one of the blocks is
        // echoed twice). Then the echoed data is compared agains the
        // outgoing data block.

        fail = 0;
        while (loop_count--) {
            int i;
            for (i = 0; i < block_size*4; i++)
                data_in[i] = 0;

            // out1: block_size -> block_size
            TargetWrite(pSer, data_out, block_size);

            // out2: block_size -> 2 x block_size
            TargetWrite(pSer, data_out, block_size);

            // in1:
            TargetRead(pSer, data_in, block_size);

            // out3: block_size -> block_size
            TargetWrite(pSer, data_out, block_size);
        
            // in2:
            TargetRead(pSer, &data_in[block_size], 2*block_size);

            // out4: block_size -> 0
            TargetWrite(pSer, data_out, block_size);
        
            // in3:
            TargetRead(pSer, &data_in[block_size*3], block_size);

            if (0 == loop_count % 10)
                Trace("%d loops to go\n", loop_count);

            // Verify data.
            if (!fail) {
                for (j = 0; j < 4 && !fail; j++) {
                    for (i = 0; i < block_size && !fail; i++) {
                        if (data_out[i] != data_in[j*block_size + i]) {
                            fail = 1;
                            Trace("Failed at byte %d\n", j*block_size + i);
                            
                            Trace("Data seen:\n");
                            PrintHex(&data_in[j*block_size], 
                                           block_size);
                            Trace("<end>\n");
                            Trace("Data expected:\n");
                            PrintHex(data_out, block_size);
                            Trace("<end>\n");
                        }
                    }
                }
            }
        }
        // Check echoed data, and reply OK/ER accordingly.
        SendStatus(pSer, (!fail));
        ReceiveDone(pSer, data_in, block_size*4);
    }
    break;
    default:
        Trace("Unknown mode. Ignoring.\n");
    }

    // Free buffer.
    free(data_in);
    free(data_out);
}

//-----------------------------------------------------------------------------
// Test transformations on text transmissions
//
// This test transmits null-terminated C strings back and forth. Since
// the translation is under test and may fail, the length of the data is
// (potentially) unknown. Sending with a null-terminator allows proper
// recovery even if the translations do not work as intended.
//
// Format in:
//  <flags>!<4 bytes binary checksum><C string>
// Format out:
//  <C string>
//  OK/ER
//
// Mode:
//   MODE_EOP_ECHO:
//       Receive data, verify CRC, resend data.
//       Send OK/ER reply when done.
//   MODE_DUPLEX_ECHO:
//       Receive data, echo data, verify CRC.
//       Send OK/ER reply when done.
//
// To Do:
//  Implement.
void
CeCosTestSerialFilter::CMD_TestText(CeCosSerial &pSer, char* /*args*/)
{
    SendStatus(pSer, 1);
}

//-----------------------------------------------------------------------------
// Reply to PING packet from target.
// Format in:
//  "!"
// Format out:
//  OK
void
CeCosTestSerialFilter::CMD_TestPing(CeCosSerial &pSer, char* /*args*/)
{ 
    SendStatus(pSer, 1);
}

//-----------------------------------------------------------------------------
// Dispatch test command. 
void
CeCosTestSerialFilter::DispatchCommand(CeCosSerial &pSer, char* cmd)
{
    char* args;

    args = strchr(cmd, (int) ':');
    if (!args) {
        Trace("Bogus command (%s) Ignoring.\n", cmd);
        return;
    }
        
    *args++ = 0;

    Trace("Dispatching command %s.\n", cmd);

    if (0 == strcmp("CONFIG", cmd)) {
        CMD_ChangeConfig(pSer, args);
    } 
    else if (0 == strcmp("DEFCONFIG", cmd)) {
        // Note: Currently the arguments are ignored. 9600 8N1 is default.
        CMD_DefaultConfig(pSer);
    }
    else if (0 == strcmp("BINARY", cmd)) {
        CMD_TestBinary(pSer, args);
    }
    else if (0 == strcmp("TEXT", cmd)) {
        CMD_TestText(pSer, args);
    }
    else if (0 == strcmp("PING", cmd)) {
        CMD_TestPing(pSer, args);
    }
    else
        Trace("Unknown command '%s'.\n", cmd);

    Trace("Command %s completed.\n", cmd);
}

bool CALLBACK
SerialFilterFunction(void*& pBuf,
                     unsigned int& nRead,
                     CeCosSerial& serial,
                     CeCosSocket& socket,
                     void* pParem)
{
    CeCosTestSerialFilter* p = (CeCosTestSerialFilter*) pParem;
    return p->FilterFunctionProper(pBuf, nRead, serial, socket);
}

bool
CeCosTestSerialFilter::FilterFunctionProper(void*& pBuf,
                                            unsigned int& nRead,
                                            CeCosSerial& serial,
                                            CeCosSocket& socket)
{
    char* buffer = (char*) pBuf;

    // Don't do anything in the null filter mode.
    if (m_bNullFilter)
        return true;

    // Allows trace to be called without a reference to the socket...
    m_cGDBSocket = &socket;

    // Put in trace buffer in case we have to leave it because the packet
    // is incomplete
    m_xStoredTraceBuffer = (unsigned char *)
        realloc( m_xStoredTraceBuffer, m_nStoredTraceBufferSize + nRead );
    if ( NULL == m_xStoredTraceBuffer ) 
        throw "Could not allocate stored trace buffer";
    memcpy( m_xStoredTraceBuffer + m_nStoredTraceBufferSize, buffer, nRead );
    m_nStoredTraceBufferSize += nRead;

    // Now search for distinct packets, delimited by '@' (filter commands)
    // and '$' (GDB packets)
    unsigned int i, newStart=0;
    for (i=0; i<m_nStoredTraceBufferSize; i++) {
        if ( m_xStoredTraceBuffer[i] == '@' ||
             m_xStoredTraceBuffer[i] == '$' ) {
            if (m_bOptSerDebug &&
                (m_bOptConsoleOutput || m_bFirstCommandSeen)) {

                // Output the serial data if option enabled - but only if
                // dumping state to the console or after the first command
                // has been seen from the filter. GDB gets confused by
                // O-packets if they appear when it's trying to connect.

                PrintHex(&m_xStoredTraceBuffer[newStart], i - newStart);
            }
            newStart = i;
        }
    }

    // If we managed to print output, rejig the buffer size, and shunt
    // the new start of the data to the front of the trace buffer
    m_nStoredTraceBufferSize -= newStart;
    
    memmove( m_xStoredTraceBuffer, &m_xStoredTraceBuffer[newStart],
             m_nStoredTraceBufferSize );
        

    // Command handling.
    // If we are not presently reading a command, look for the
    // start marker.
    i = 0;
    if (!m_bCmdFlag)
        for (; i < nRead; i++) {
            if ('@' == buffer[i]) {
                m_bCmdFlag = true;
                // Send the data before the marker.
                if (i)
                    socket.send(buffer, i);
                break;
            }
        }

    // If reading a command, look for the end marker.
    if (m_bCmdFlag) {
        char c = 0;
        while (i < nRead && m_nCmdIndex < MAX_CMD_LEN) {
            c = buffer[i++];
            m_aCmd[m_nCmdIndex++] = c;
            if ('!' == c) {
                if (i != nRead) {
                    m_nUnreadBufferIndex = 0;
                    m_nUnreadBufferSize = nRead - i;
                    m_xUnreadBuffer = 
                        (unsigned char*) malloc(m_nUnreadBufferSize);
                    if (!m_xUnreadBuffer) {
                        m_nUnreadBufferSize = 0;
                        throw "Could not allocate unread buffer!";
                    }
                    
                    int ix = 0;
                    while (i < nRead)
                        m_xUnreadBuffer[ix++] = buffer[i++];
                }
                break;
            }
        }

        if (MAX_CMD_LEN == m_nCmdIndex) {
            Trace("Received too long command. Ignoring it!\n");
            m_nCmdIndex = 0;
            m_bCmdFlag = false;
        } else if ('!' == c) {
            // Was the command completed?
            m_aCmd[m_nCmdIndex - 1] = 0;// terminate cmd
            m_nCmdIndex = 0;
            m_bCmdFlag = false;

            // First command dispatched. Initialize serial to nonblocking.
            if (!m_bFirstCommandSeen) {
                m_bFirstCommandSeen = true;
                serial.SetBlockingReads(false);
            }
  
            try {
                // skip @ when passing ptr
                DispatchCommand(serial, &m_aCmd[1]);
            } 
            catch (filter_abort_t* msg) {
                // This allows the filter to unwind, wherever in the
                // protocol it may be, when a $ is detected from the
                // target side.  When this happens, we may have a
                // trap/exception on the target and we want the user
                // to access the target via GDB without intervention.

                // Do nothing from next call.
                m_bNullFilter = true;

                // Copy the start of the $-packet to the inbuffer.
                unsigned char *d = (unsigned char*) pBuf;
                const unsigned char *s = msg->data_ptr;
                unsigned int len = msg->data_len;
                
                // It should be possible to re-allocate buffer. Didn't seem
                // to work properly though. Probably won't be a problem
                // since we would normally only see 1-2 bytes of the
                // $-packet anyway.
                if (len > nRead)
                    throw "Not enough room for $-message";

                while (len--)
                    *d++ = *s++;

                nRead = msg->data_len;

                delete msg;

                return true;
            }
        }

        nRead = 0;                      // Never leave anything for caller
                                        // This is a violation of the intended
                                        // filter function behavior.
    }
    return true;
}
