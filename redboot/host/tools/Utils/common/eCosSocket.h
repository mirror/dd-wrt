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
//        eCosSocket.h
//
//        Socket test class
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   This class abstracts tcp/ip sockets for use in the testing infrastructure
// Usage:
//
//####DESCRIPTIONEND####
//=================================================================
// This class is a host-independent interface to a TCP/IP socket
// There are two flavours of socket - server and client.
// Server sockets listen (accept) on a socket number.  Client sockets connect to a host:port.
// The class can be used thus:
//   Server:
//      CeCosSocket sock; // no-argument ctor
//      if(-1!=sock.Listen(6000)){
//        ...      
//      }
//   Client:
//      CeCosSocket sock;
//      if(sock.Connect(_T("ginga"),5000)){
//        ...      
//      }
// In each of the above cases the socket is closed automatically by the dtor.
//
// Alternatively, the ctor can be used directly:
//   Server:
//      CeCosSocket sock(6000));
//      ...      
//   Client:
//      CeCosSocket sock(_T("ginga"),5000));
//      ...      
//
//=================================================================
#include "eCosStd.h"
#include "Collections.h"

#ifndef _SOCKETUTILS_H
#define _SOCKETUTILS_H

class CeCosSerial;

class CeCosSocket {
public:	
	static const String GetHostByName(LPCTSTR pszHost);

  // These functions must be called before any other operation is carried out:
	static bool Init();
	static void Term();

  typedef bool (CALLBACK FilterFunc)(void *&,unsigned int &,CeCosSerial&,CeCosSocket &,void *);

  // A function that causes an operation to stop - i.e. it when it returns true the operation is aborted.
  typedef bool (CALLBACK StopFunc)(void *);
  
  enum {NOTIMEOUT=0x7fffffff-1,DEFAULTTIMEOUT=-2}; // No explicit timeout specified
  
  // Listen and this form of constructor used to act as server
  static int Listen(int nTcpPort);
  CeCosSocket (); // Caller promises to call Accept() or Connect() later
  
  // Accept-like ctor (act as server)
  CeCosSocket (int sock /*result of previous call of Listen*/, bool *pbStop=0);
  // Connect-like ctor (act as client)
  CeCosSocket (LPCTSTR pszHostPort,Duration dTimeout=NOTIMEOUT);
  
  bool Accept(int sock /*result of previous call of Listen*/, bool *pbStop=0);
  // This form of constructor used to act as client
  bool Connect(LPCTSTR pszHostPort,Duration dTimeout=NOTIMEOUT);
  ~CeCosSocket();
  
  int Client() const { return m_nClient; }
  static String ClientName(int nClient);
  
  int Sock() const { return m_nSock; }
  
  // Set the default timeout for all operations
  void SetTimeout (Duration dTimeout) { m_nDefaultTimeout=dTimeout; }
  
  // Use to test success after opening with the ctor:
  bool Ok() { return -1!=m_nSock; }

  // Close the given socket
  bool Close () { return CloseSocket(m_nSock); }
  
  // Return last error on this socket
  int SocketError() { return m_nErr; }

  // Return last socket error, translated to a string
  String SocketErrString();
  static String SocketErrString(int nErr);

  // Read and write functions

  // Untyped: these versions allow the operation to be aborted either by timeout or by the "stop func" returning true.
  bool send(const void *pData,unsigned int nLength,LPCTSTR pszMsg=_T(""),int dTimeout=DEFAULTTIMEOUT,StopFunc *pFunc=0,void *pParam=0){
    return sendrecv(true,pData,nLength,pszMsg,dTimeout,pFunc,pParam);
  }
  bool recv(const void *pData,unsigned int nLength,LPCTSTR pszMsg=_T(""),int dTimeout=DEFAULTTIMEOUT,StopFunc *pFunc=0,void *pParam=0){
    return sendrecv(false,pData,nLength,pszMsg,dTimeout,pFunc,pParam);
  }

  // Read/write an integer (this can be used between machines of different endianness)
  bool recvInteger (int &n,LPCTSTR pszMsg=_T(""),Duration dTimeout=DEFAULTTIMEOUT);
  bool sendInteger (int n,LPCTSTR pszMsg=_T(""),Duration dTimeout=DEFAULTTIMEOUT);

  // Read/write a string
  bool recvString  (String &str,LPCTSTR pszMsg=_T(""),Duration dTimeout=DEFAULTTIMEOUT);
  bool sendString  (const String &str,LPCTSTR pszMsg=_T(""),Duration dTimeout=DEFAULTTIMEOUT);
  
  static bool CloseSocket (int &sock);
  bool Peek (unsigned int &nAvail);

  // Miscellaneous helper functions:

  // Combine string and integer to the form host:port:
  static String HostPort(LPCTSTR pszHost,int nPort);
  // Decompose (opposite of the above):
  static bool ParseHostPort (LPCTSTR pszHostPort, String &pszHost, int &nPort);
  // Just check for legality:
  static bool IsLegalHostPort (LPCTSTR pszHostPort);
  // Are these two hosts really the same?
  static bool SameHost (LPCTSTR host1,LPCTSTR host2);
  // Set up a connection between a serial port and a socket.  Traffic is simply passed between them.
  static bool ConnectSocketToSerial (int nListenSock,LPCTSTR pszPort, int nBaud,FilterFunc *pSerialToSocketFilterFunc=0,void *pSerialParam=0,FilterFunc *pSocketToSerialFilterFunc=0,void *pSocketParam=0,bool *pbStop=0);
  static bool ConnectSocketToSerial (CeCosSocket &socket,CeCosSerial &serial,FilterFunc *pSerialToSocketFilterFunc=0,void *pSerialParam=0, FilterFunc *pSocketToSerialFilterFunc=0,void *pSocketParam=0,bool *pbStop=0);

  static LPCTSTR MyHostName();
  static LPCTSTR MySimpleHostName();

  // Set up a connection between two sockets.  Traffic is simply passed between them.
  bool ConnectSocketToSocket (CeCosSocket &o,FilterFunc *pSocketToSocketFilterFunc1,FilterFunc *pSocketToSocketFilterFunc2,void *pParam,bool *pbStop);
  
  enum   SSReadResult {SS_SOCKET_ERROR=-1,SS_SOCKET_READ=1,SS_SERIAL_ERROR=-2,SS_SERIAL_READ=2,SS_STOPPED=0};

protected:

  // Blocking read on one or other of the data sources:
  // Result:  -1 - socket error occurred
  //           1 - data read from socket
  //          -2 - serial error occurred
  //           2 - data read from serial

  static SSReadResult SSRead (CeCosSerial &serial,CeCosSocket &socket,void *pBuf,unsigned int nSize,unsigned int &nRead,bool *pbStop);
  
  Duration m_nDefaultTimeout;
  Duration TimeoutDuration (Duration dTimeout);
  // Set appropriate socket options (most importantly, non-blocking mode)
  bool SetSocketOptions ();
  int m_nSock;
  int m_nClient;
  int m_nErr;
  void SaveError() { 
#ifdef _WIN32
  m_nErr=WSAGetLastError();
#else // UNIX
  m_nErr=errno;
#endif
  }
  bool sendrecv(bool bSend,const void *pData,unsigned int nLength,LPCTSTR pszMsg=_T(""),int dTimeout=DEFAULTTIMEOUT,StopFunc *pFunc=0,void *pParam=0);
  
};
#endif
