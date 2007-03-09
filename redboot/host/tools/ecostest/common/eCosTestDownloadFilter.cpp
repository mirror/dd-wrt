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
//        eCosTestDownloadFilter.cpp
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

// Description:   This filter sits between the (farm) host's socket and

//                the serial connection to the target board. The
//                filter listens for special download packets from the
//                client using the board. The download packet allows
//                the relative slow GDB download protocol to be restricted
//                to the connection between the farm host and the board.
//                This prevents slow downloads due to long round-trip times
//                and TCP packets with only a few bytes in them.
// To Do:
//  o Move parts shared between eCosTestDownloadFilter and eCosTestSerialFilter
//    into a shared class.
//####DESCRIPTIONEND####

#include "eCosStd.h"
#include "eCosTrace.h"

#include "eCosTestDownloadFilter.h"

CeCosTestDownloadFilter::CeCosTestDownloadFilter():
    m_bNullFilter(false), m_bOptSerDebug(false), m_bOptFilterTrace(false),
    m_nCmdIndex(0), m_bCmdFlag(false), m_bContinueSession(false)
{
}

CeCosTestDownloadFilter::~CeCosTestDownloadFilter()
{
}

//------------------------
// Output helpers.

void
CeCosTestDownloadFilter::ConsoleWrite(const char* pszStr)
{
    fputs(pszStr, stderr);
    fflush(stderr);
}

void
CeCosTestDownloadFilter::Trace(const char* pszFormat, ...)
{
  va_list marker;
  va_start (marker, pszFormat);
  
  for(int nLength=100;nLength;) {
    char *buf=new char[1+nLength];
    int n=vsnprintf(buf+4, nLength-4, pszFormat, marker ); 
    if(-1==n){
      nLength*=2;  // NT behavior
    } else if (n<nLength){
      memcpy(buf,"[d] ",4);
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
CeCosTestDownloadFilter::PrintHex(const unsigned char* d1, int len, data_origin_t origin/*=SF_TARGET*/)
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

void
CeCosTestDownloadFilter::TargetWrite(CeCosSerial &pSer, 
                                    const unsigned char* buffer, int len)
{
    unsigned int __written;

    if (m_bOptSerDebug)
        PrintHex(buffer, len, SF_FILTER);

    do {
        if (!(pSer.Write((void*) buffer, len, __written))) {
            fprintf(stderr, "Writing %d bytes to serial failed\n", len);
            fprintf(stderr, "%s", (LPCTSTR)pSer.ErrString());
            throw _T("serial write failed");
        }
        buffer += __written;
        len -= __written;
    } while (len);
}

// Snuffed from gdb/remote.c
int
CeCosTestDownloadFilter::tohex (int nib)
{
    if (nib < 10)
        return '0'+nib;
    else
        return 'a'+nib-10;
}

int
CeCosTestDownloadFilter::hexnumstr (unsigned char* buf, unsigned long num)
{
  int i;
  unsigned long num2 = num;

  for (i = 0; num2 != 0; i++)
    num2 >>= 4;

  int len = MAX (i, 1);

  buf[len] = '\0';

  for (i = len - 1; i >= 0; i--)
    {
      buf[i] = "0123456789abcdef" [(num & 0xf)];
      num >>= 4;
    }

  return len;
}

int
CeCosTestDownloadFilter::hexnumlen (unsigned long num)
{
  int i;
  unsigned long num2 = num;

  for (i = 0; num2 != 0; i++)
    num2 >>= 4;

  return MAX (i, 1);
}

// Based on routines in gdb/remote.c
int
CeCosTestDownloadFilter::put_binary (unsigned char* buf, int len,
                                     unsigned long dl_address,
                                     int packet_size,
                                     CeCosSerial& serial)
{
    int i;
    unsigned char csum;
    Buffer buf2(packet_size);
    unsigned char ch;
    int tcount = 0;
    unsigned char *p, *p2, *plen;

    while (len > 0) {
        /* Subtract header overhead from MAX payload size: 
           $M<memaddr>,<len>:#nn */
        int max_buf_size = 
            buf2.Size() 
            - ( 2 + hexnumlen(dl_address) + 1 + hexnumlen(buf2.Size()) + 4);

        /* Copy the packet into buffer BUF2, encapsulating it
           and giving it a checksum.  */
        int todo = MIN (len, max_buf_size);

        p = (unsigned char*) buf2.Data();
        *p++ = '$';

        // Add X header.
        *p++ = 'X';
        p += hexnumstr(p, dl_address);
        *p++ = ',';
        plen = p;			/* remember where len field goes */
        p += hexnumstr(p, todo);
        *p++ = ':';

        int escaped = 0;
        for (i = 0;
             (i < todo) && (i + escaped) < (max_buf_size - 2);
             i++)
        {
            switch (buf[i] & 0xff)
            {
            case '$':
            case '#':
            case 0x7d:
                /* These must be escaped */
                escaped++;
                *p++ = 0x7d;
                *p++ = (unsigned char) ((buf[i] & 0xff) ^ 0x20);
                break;
            default:
                *p++ = (unsigned char) (buf[i] & 0xff);
                break;
            }
        }

        if (i < todo)
        {
            /* Escape chars have filled up the buffer prematurely, 
               and we have actually sent fewer bytes than planned.
               Fix-up the length field of the packet.  */
        
            /* FIXME: will fail if new len is a shorter string than 
               old len.  */
            
            plen += hexnumstr (plen, i);
            *plen++ = ':';
        }

        // Calculate checksum
        p2 = (unsigned char*)buf2.Data();
        p2++; // skip $
        csum = 0;
        while (p2 < p)
            csum = (unsigned char)(csum + *p2++);
        *p++ = '#';
        *p++ = (unsigned char) tohex ((csum >> 4) & 0xf);
        *p++ = (unsigned char) tohex (csum & 0xf);

        /* Send it over and over until we get a positive ack.  */

        int resend = 1;
        const unsigned char* write_ptr = (const unsigned char*) buf2.Data();
        int write_len = (int)p-(int)buf2.Data();
        while (resend)
        {
            unsigned int __written;

            Trace("Sending bytes for %p-%p\n", dl_address, dl_address+i);
            TargetWrite(serial, write_ptr, write_len);

            /* read until either a timeout occurs (-2) or '+' is read */
            for(;;)
            {
                unsigned int __read;
                serial.Read(&ch, 1, __read);
                
                if (0 == __read) {
                    tcount ++;
                    if (tcount > 3) {
                        Trace("Timeout in putpkt_binary\n");
                        return 0;
                    }
                    break;		/* Retransmit buffer */
                }
                
                switch (ch)
                {
                case '+':
                    // Now expect OK packet from target
                    unsigned char ok_msg[6];// $OK#9a
                    serial.Read(ok_msg, 6, __read);
                    
                    // Reply with ACK
                    serial.Write((void*)"+", 1, __written);
                    
                    // And process next packet.
                    resend = 0;
                    break;

                case '-':
                    // Bad packet CRC. Retransmit.
                    Trace ("Bad CRC\n");
                    break;
                    
                default:
                    Trace("Got junk..%02x\n", ch);
                    continue;           // keep reading
                }
                break;		/* Here to retransmit */
            }
        }

        len -= i;
        dl_address += i;
        buf += i;
    }

    return 1;
}    

bool CALLBACK
DownloadFilterFunction(void*& pBuf,
                       unsigned int& nRead,
                       CeCosSerial& serial,
                       CeCosSocket& socket,
                       void* pParem)
{
    CeCosTestDownloadFilter* p = (CeCosTestDownloadFilter*) pParem;
    bool res = false;

    try {
        res = p->FilterFunctionProper(pBuf, nRead, serial, socket);
    } 
    catch (LPCTSTR s) {
        TRACE(_T("Download filter caught string: %s\n"), s);
    }
    catch (...) {
        TRACE(_T("Download filter caught unknown exception\n"));
    }

    return res;
}

bool
CeCosTestDownloadFilter::FilterFunctionProper(void*& pBuf,
                                              unsigned int& nRead,
                                              CeCosSerial& serial,
                                              CeCosSocket& socket)
{
    char* buffer = (char*) pBuf;

    // Assume the worst - don't allow session to continue until a successful
    // download.
    m_bContinueSession = false;

    // Output the serial data if option enabled
    if (m_bOptSerDebug)
        PrintHex((unsigned char*) buffer, nRead);

    // Stop here if in NULL-filter mode
    if (m_bNullFilter)
        return true;

    // Command handling.
    // Be strict here; very first byte we see must be the start marker,
    // else go into NULL-filter mode
    unsigned int i = 0;
    if (!m_bCmdFlag) {
        if ('@' != buffer[i]) {
            m_bNullFilter = true;
            return true;
        }
        m_bCmdFlag = true;
    }

    // If reading a command, look for the end marker.
    if (m_bCmdFlag) {
        char c = 0;
        while (i < nRead && m_nCmdIndex < MAX_CMD_LEN) {
            c = buffer[i++];
            m_aCmd[m_nCmdIndex++] = c;
            if ('!' == c) {
                if (i != nRead) {
                    throw _T("Extra bytes after command packet!?!");
                }
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
            // command now in m_aCmd[]

            Trace("Got command %s\n", m_aCmd);

            // After this, never interfere.
            m_bNullFilter = true;

            // Get arguments: @<length>:<dl_address>:<start_address>!
            int length, packet_size;
            unsigned long dl_addr, start_addr;
            INIT_VALUE(&m_aCmd[1]);
            SET_VALUE(int, length);
            SET_VALUE(unsigned long, dl_addr);
            SET_VALUE(unsigned long, start_addr);
            SET_VALUE(int, packet_size);

            Trace("len %d, dl %08x, start %08x, packet_size %d\n", 
                  length, dl_addr, start_addr, packet_size);

            // Reply so host will send file.
            socket.send("@", 1);

            // Read file from host
            Buffer buf(length);
            if (socket.recv(buf.Data(), length)) {
            
                // Remember old blocking state, and set serial to
                // blocking reads.
                bool __blocking = serial.GetBlockingReads();
                serial.SetBlockingReads(true);
                serial.Flush();

                // Send + to target, acking whatever packet was pending
                unsigned int __written = 0;
                serial.Write((void*)"+", 1, __written);
                
                // Convert to packets and transfer to target.
                if (put_binary((unsigned char*) buf.Data(), 
                               length, dl_addr, packet_size, serial)) {
                    // Send detach signal to target
                    unsigned char ch;
                    unsigned int __read;
                    serial.Write((void*)"$D#44", 5, __written);
                    serial.Read(&ch, 1, __read);
                    
                    // Reply to host marking end of download
                    socket.send("+", 1);

                    // Let server know it's OK to accept another connection
                    // in this session.
                    m_bContinueSession = true;
                } else {
                    // Reply to host marking failed download
                    socket.send("-", 1);
                }

                // Reset previous blocking mode
                serial.SetBlockingReads(__blocking);

            } else {
                // Reply to host marking failed file transfer
                socket.send("?", 1);
            }
        }
        nRead = 0;                      // Never leave anything for caller
                                        // This is a violation of the intended
                                        // filter function behavior.
    }
    return true;
}
