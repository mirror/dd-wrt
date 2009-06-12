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
//        eCosSocket.cpp
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

#include "eCosStd.h"
#include "eCosSocket.h"
#include "eCosSerial.h"
#include "eCosThreadUtils.h"
#include "eCosTrace.h"
#include <map>

enum {ERR_TIMEOUT=20000, ERR_READ_AFTER_CLOSE=20001};

// Blocking read on one or other of the data sources:
// Result:  -1 - socket error occurred
//           1 - data read from socket
//          -2 - serial error occurred
//           2 - data read from serial

CeCosSocket::SSReadResult CeCosSocket::SSRead (CeCosSerial &serial,CeCosSocket &socket,void *pBuf,unsigned int nSize,unsigned int &nRead,bool *pbStop)
{
  SSReadResult rc=SS_STOPPED;
  bool bBlocking=serial.GetBlockingReads();
  bool bBlockingModified=false;
  while(0==pbStop || !(*pbStop)){
    if(!socket.Peek(nRead)){
      rc=SS_SOCKET_ERROR;
      break;
    } else if(nRead){
      nRead=MIN(nRead,nSize);
      rc=socket.recv(pBuf,nRead)?SS_SOCKET_READ:SS_SOCKET_ERROR;
      break;
    } else {
      if(bBlocking){
        serial.SetBlockingReads(false);
        bBlockingModified=true;
        bBlocking=false;
      }
      if(serial.Read(pBuf,nSize,nRead)){
        if(nRead>0){
          rc=SS_SERIAL_READ;
          break;
        }
      } else {
        rc=SS_SERIAL_ERROR;
        break;
      }
    }
    CeCosThreadUtils::Sleep(10);
  }
  if(bBlockingModified){
    serial.SetBlockingReads(true);
  }
  return rc;
}

// ctors and dtors

CeCosSocket::CeCosSocket ():
m_nDefaultTimeout(10*1000),
m_nSock(-1),
m_nClient(0)
{
  VTRACE(_T("Create socket instance %08x\n"),(unsigned int)this);
}

CeCosSocket::CeCosSocket (int sock /*result of previous call of Listen*/, bool *pbStop):
m_nDefaultTimeout(10*1000),
m_nSock(-1),
m_nClient(0)
{
  VTRACE(_T("Create socket instance %08x\n"),(unsigned int)this);
  Accept(sock,pbStop);
}

CeCosSocket::CeCosSocket (LPCTSTR pszHostPort,Duration dTimeout):
m_nDefaultTimeout(10*1000),
m_nSock(-1),
m_nClient(0)
{
  VTRACE(_T("Create socket instance %08x\n"),(unsigned int)this);
  Connect(pszHostPort,dTimeout);
}

bool CeCosSocket::Accept(int sock /*result of previous call of Listen*/, bool *pbStop)
{
  m_nSock=-1;
  while(0==pbStop||!*pbStop){
    struct sockaddr cli_addr;
#ifndef _WIN32
    unsigned 
#endif
      int clilen=sizeof(struct sockaddr);
    m_nSock=::accept(sock, (struct sockaddr *) &cli_addr, &clilen);
    SaveError();
    if(-1==m_nSock){ 
      if(WOULDBLOCK==SocketError()){
        CeCosThreadUtils::Sleep(100);
        continue;
      }
    } else {
      memcpy(&m_nClient,cli_addr.sa_data+2,4);
      TRACE(_T("Connection accepted from %s - socket %d\n"),(LPCTSTR )ClientName(m_nClient),m_nSock);
      SetSocketOptions();
      break;
    }
  } 
  return -1!=m_nSock;
}

int CeCosSocket::Listen(int nTcpPort)
{
  // Create socket
  int sock=::socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    ERROR(_T("Couldn't create socket\n"));
  } else {
    VTRACE(_T("Created socket %d listening on port %d\n"),sock,nTcpPort);
    // Bind socket to address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof serv_addr);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port=htons((short)nTcpPort);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (::bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
      TRACE(_T("Couldn't bind socket on port %d\n"),nTcpPort);
      CloseSocket(sock);
    } else if (-1==::listen(sock, SOMAXCONN)){
      CloseSocket(sock);
      TRACE(_T("socket error on listen - port %d\n"),nTcpPort);
    } else {
#ifdef _WIN32
      int nTrue=1;
      bool rc=(0==::ioctlsocket(sock, FIONBIO, (unsigned long *)&nTrue));
#else //UNIX
      int flags=::fcntl(sock,F_GETFL);
      flags|=O_NONBLOCK;
      bool rc=(0==::fcntl (sock, F_SETFL, flags));
#endif
      if(!rc){
        TRACE(_T("Failed to set socket options on socket %d\n"),sock);
      }
    }
  }
  return sock;
}

bool CeCosSocket::Connect(LPCTSTR pszHostPort,Duration dTimeout)
{
  dTimeout=TimeoutDuration(dTimeout);
  struct sockaddr_in serv_addr;
  
  VTRACE(_T("Connect: %s timeout=%d\n"),pszHostPort,dTimeout);
  
  // Get the target host address
  String strHost;
  int nPort;
  CeCosSocket::ParseHostPort(pszHostPort,strHost,nPort);
  String strErr;

  char *ip=GetHostByName(strHost).GetCString();
  memset(&serv_addr, 0, sizeof serv_addr);
  // Create socket
  m_nSock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == m_nSock) {
    TRACE(_T("Could not create socket [%s]\n"),pszHostPort);
  } else {
#ifdef _WIN32
    SetSocketOptions();
#endif
    VTRACE(_T("Created socket %d connected to %s\n"),m_nSock,pszHostPort);
    // Bind socket to address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port=htons((short)nPort);
    SaveError();
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    
    // Connect to server
    VTRACE(_T("Connect() : connecting to server\n"));
    int cc=::connect(m_nSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    SaveError();
    String strMsg;
    if(-1==cc){
      if(
#ifdef _WIN32
        WOULDBLOCK==SocketError()
#else // UNIX
        EINPROGRESS==SocketError()
#endif
        ){
        // Allow dTimeout milliseconds for connect to complete
        fd_set set;
        FD_ZERO(&set);
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4127 ) // conditional expression is constant
#endif
        FD_SET((unsigned)m_nSock, &set);
#ifdef _WIN32
#pragma warning( pop )
#endif
        struct timeval tv;
        tv.tv_sec = dTimeout/1000;  
        tv.tv_usec = 1000*(dTimeout % 1000);
        switch(::select(m_nSock, NULL, &set , NULL, &tv)){
        case 0:
          m_nErr=ERR_TIMEOUT;
          strMsg.Format(_T("attempt timed out after %d seconds"),dTimeout/1000);
          break;
        case -1:
          SaveError();
          strMsg=SocketErrString();
          break;
        default:
          cc=0;
        }
      } else {
        strMsg=SocketErrString();
      }
    }
    
    if(-1==cc){
      TRACE(_T("Could not connect to %s - %s\n"),pszHostPort,(LPCTSTR)strMsg);
      CloseSocket(m_nSock);
    }  else {
#ifndef _WIN32
      SetSocketOptions();
#endif
    }
  }
  delete [] ip;
  return -1!=m_nSock;
}

bool CeCosSocket::sendrecv(bool bSend,const void *pData,unsigned int nLength,
                           LPCTSTR pszMsg,Duration dTimeout,CeCosSocket::StopFunc pFnStop,void *pParam)
{
  
  dTimeout=TimeoutDuration(dTimeout);
  
  LPCTSTR pszSR=(bSend?_T("sending"):_T("receiving"));
  LPTSTR c=(LPTSTR )pData;
  Time ft0=Now();
  int nTodo=nLength;
  while((nTodo>0) && ((0==pFnStop) || (!pFnStop(pParam)))){
    int s=bSend?::send(m_nSock, (const char *)c, nTodo, 0): ::recv(m_nSock, (char *)c, nTodo, 0);
    if(0==s && !bSend){
      m_nErr=ERR_READ_AFTER_CLOSE;
    } else {
      SaveError();
    }
    if(-1==s && WOULDBLOCK==SocketError()){
      Duration d=Duration(Now()-ft0);
      if(d>dTimeout){
        TRACE(_T("%d/%d mSec timeout on socket %d %s %s - processed %d/%d bytes\n") ,
          d,dTimeout,m_nSock,pszSR,pszMsg, 
          nLength-nTodo,nLength);
        m_nErr=ERR_TIMEOUT;
        break;
      }
      CeCosThreadUtils::Sleep(100);
    } else if (s>0) {
      c+=s;
      nTodo-=s;
      ft0=Now();
    } else {
      TRACE(_T("Error on socket %d %s %s - %s\n") ,m_nSock, pszSR, pszMsg, (LPCTSTR )SocketErrString());
      break;
    }
  }
  return 0==nTodo;
}

// Graceful socket closedown
CeCosSocket::~CeCosSocket()
{
  Close();
  VTRACE(_T("Delete socket instance %08x\n"),(unsigned int)this);
}

bool CeCosSocket::CloseSocket(int &sock)
{
  bool rc=false;
  if(-1!=sock){
    VTRACE(_T("Closing socket %d\n"),sock);
    try{
      shutdown(sock,0);// SD_BOTH
#ifdef _WIN32
      rc=(0==closesocket(sock));
#else // UNIX
      rc=(0==close(sock));
#endif
    }
    catch(...) {
      TRACE(_T("!!! Exception caught in CeCosSocket::CloseSocket!!!\n"));
    }
    sock=-1;
  }
  return rc;
}

bool CeCosSocket::SetSocketOptions()
{
  bool rc;
#ifdef _WIN32
  int nTrue=1;
  rc=(0==::ioctlsocket(m_nSock, FIONBIO, (unsigned long *)&nTrue));
  SaveError();
#else // UNIX
  int flags=::fcntl(m_nSock,F_GETFL);
  SaveError();
  flags|=O_NONBLOCK;
  rc=(0==::fcntl (m_nSock, F_SETFL, flags));
  SaveError();
#endif
  int bLinger=0;
  setsockopt(m_nSock,SOL_SOCKET,SO_LINGER,(const char *)&bLinger, sizeof(bLinger));
  if(!rc){
    TRACE(_T("Failed to set socket options socket %d - %s\n"),m_nSock,(LPCTSTR )SocketErrString());
  }
  return rc;
}

String CeCosSocket::SocketErrString(int nErr)
{
  String str;
#ifdef _WIN32
  switch(nErr){
  case ERR_TIMEOUT: str=_T("Read operation timed out");break;
  case ERR_READ_AFTER_CLOSE: str=_T("Read operation after socket closed");break;
    
  case WSAEACCES: str=_T("Permission denied");break;
  case WSAEADDRINUSE: str=_T("Address already in use");break;
  case WSAEADDRNOTAVAIL: str=_T("Cannot assign requested address");break;
  case WSAEAFNOSUPPORT: str=_T("Address family not supported by protocol family");break;
  case WSAEALREADY: str=_T("Operation already in progress");break;
  case WSAECONNABORTED: str=_T("Software caused connection abort");break;
  case WSAECONNREFUSED: str=_T("Connection refused");break;
  case WSAECONNRESET: str=_T("Connection reset by peer");break;
  case WSAEDESTADDRREQ: str=_T("Destination address required");break;
  case WSAEFAULT: str=_T("Bad address");break;
  case WSAEHOSTDOWN: str=_T("Host is down");break;
  case WSAEHOSTUNREACH: str=_T("No route to host");break;
  case WSAEINPROGRESS: str=_T("Operation now in progress");break;
  case WSAEINTR: str=_T("Interrupted function call");break;
  case WSAEINVAL: str=_T("Invalid argument");break;
  case WSAEISCONN: str=_T("Socket is already connected");break;
  case WSAEMFILE: str=_T("Too many open files");break;
  case WSAEMSGSIZE: str=_T("Message too long");break;
  case WSAENETDOWN: str=_T("Network is down");break;
  case WSAENETRESET: str=_T("Network dropped connection on reset");break;
  case WSAENETUNREACH: str=_T("Network is unreachable");break;
  case WSAENOBUFS: str=_T("No buffer space available");break;
  case WSAENOPROTOOPT: str=_T("Bad protocol option");break;
  case WSAENOTCONN: str=_T("Socket is not connected");break;
  case WSAENOTSOCK: str=_T("Socket operation on non-socket");break;
  case WSAEOPNOTSUPP: str=_T("Operation not supported");break;
  case WSAEPFNOSUPPORT: str=_T("Protocol family not supported");break;
  case WSAEPROCLIM: str=_T("Too many processes");break;
  case WSAEPROTONOSUPPORT: str=_T("Protocol not supported");break;
  case WSAEPROTOTYPE: str=_T("Protocol wrong type for socket");break;
  case WSAESHUTDOWN: str=_T("Cannot send after socket shutdown");break;
  case WSAESOCKTNOSUPPORT: str=_T("Socket type not supported");break;
  case WSAETIMEDOUT: str=_T("Connection timed out");break;
  case WSATYPE_NOT_FOUND: str=_T("Class type not found");break;
  case WSAEWOULDBLOCK: str=_T("Resource temporarily unavailable");break;
  case WSAHOST_NOT_FOUND: str=_T("Host not found");break;
  case WSA_INVALID_HANDLE: str=_T("Specified event object handle is invalid");break;
  case WSA_INVALID_PARAMETER: str=_T("One or more parameters are invalid");break;
    //case WSAINVALIDPROCTABLE: str=_T("Invalid procedure table from service provider");break;
    //case WSAINVALIDPROVIDER: str=_T("Invalid service provider version number");break;
  case WSA_IO_INCOMPLETE: str=_T("Overlapped I/O event object not in signaled state");break;
  case WSA_IO_PENDING: str=_T("Overlapped operations will complete later");break;
  case WSA_NOT_ENOUGH_MEMORY: str=_T("Insufficient memory available");break;
  case WSANOTINITIALISED: str=_T("Successful case WSAStartup not yet:performed");break;
  case WSANO_DATA: str=_T("Valid name, no data record of requested type");break;
  case WSANO_RECOVERY: str=_T("This is a non-recoverable error");break;
    //case WSAPROVIDERFAILEDINIT: str=_T("Unable to initialize a service provider");break;
  case WSASYSCALLFAILURE: str=_T("System call failure");break;
  case WSASYSNOTREADY: str=_T("Network subsystem is unavailable");break;
  case WSATRY_AGAIN: str=_T("Non-authoritative host not found");break;
  case WSAVERNOTSUPPORTED: str=_T("WINSOCK.DLL version out of range");break;
  case WSAEDISCON: str=_T("Graceful shutdown in progress");break;
  case WSA_OPERATION_ABORTED: str=_T("Overlapped operation aborted");break;
  default:
    str.Format(_T("Unknown error %d (0x%08x)"),nErr,nErr);
  }
#else // UNIX
  switch(nErr){
  case ERR_TIMEOUT: str=_T("Read operation timed out");break;
  case ERR_READ_AFTER_CLOSE: str=_T("Read operation after socket closed");break;
  default:
    str=strerror(errno);
  }
#endif
  return str;
}

bool CeCosSocket::sendInteger(int n,LPCTSTR pszMsg,Duration dTimeout)
{
  // This has to support cross-architectural endianness
  unsigned char c[sizeof(int)];
  for(unsigned int i=0;i<sizeof(int);i++){
    c[i]=(unsigned char)(n&0xff);
    n>>=8;
  }
  return send (c, sizeof(int),pszMsg,dTimeout);
}

bool CeCosSocket::recvInteger(int & n,LPCTSTR pszMsg,Duration dTimeout)
{
  // This has to support cross-architectural endianness
  unsigned char c[sizeof(int)];
  bool rc=recv (c, sizeof(int),pszMsg,dTimeout);
  n=0;
  if(rc){
    for(int i=sizeof(int)-1;i>=0;--i){
      n<<=8;
      n|=c[i];
    }
  }
  return rc;
}

// Socket communications for strings are always non-UNICODE:
bool CeCosSocket::recvString  (String &str,LPCTSTR pszMsg,Duration dTimeout)
{
  int nLength;
  bool rc=false;
  if(recvInteger(nLength,pszMsg,dTimeout)){
    if(0==nLength){
      rc=true;
    } else {
      Buffer b(1+nLength);
      char *c=(char *)b.Data();
      if(c){
        rc=recv(c,nLength,pszMsg,dTimeout);
        c[nLength]='\0';
        str=String::CStrToUnicodeStr(c);
      }
    }
  }
  return rc;
}

// Socket communications for strings are always non-UNICODE:
bool CeCosSocket::sendString  (const String &str,LPCTSTR pszMsg,Duration dTimeout)
{
  char *psz=str.GetCString();
  int nLength=strlen(psz); 
  bool rc=sendInteger(nLength,pszMsg,dTimeout) && (0==nLength || send(psz,nLength,pszMsg,dTimeout));
  delete [] psz;
  return rc;
}


// Give indication of bytes available to be read (but don't read them)
bool CeCosSocket::Peek (unsigned int &nAvail)
{
  char buf[8192];
  int n=::recv(m_nSock, buf, sizeof buf, MSG_PEEK);
  nAvail=0;
  bool rc=false;
  switch(n) {
  case -1:
    SaveError();
    if(WOULDBLOCK==SocketError()){
      rc=true; // nAvail stays==0
    } else {
      ERROR(_T("Peek: err=%d %s\n"),SocketError(),(LPCTSTR)SocketErrString());
    }
    break;
  case 0:
    m_nErr=ERR_READ_AFTER_CLOSE;
    break;
  default:
    rc=true;
    nAvail=n;
  }
  return rc;
}

// Connect tcp/ip port and serial port together.
// Traffic is passed through pFunc, passed parameter pParam.
// The pFunc function:
//    may reallocate pBuf (using malloc/realloc etc...)
//    must leave pBuf allocated on exit
//    should not close either serial or socket
//    should leave writing to its caller
//    should return false if it wishes to terminate the connection (after caller has written output)
bool CeCosSocket::ConnectSocketToSerial (CeCosSocket &socket,CeCosSerial &serial,FilterFunc *pSerialToSocketFilterFunc/*=0*/,void *pSerialParam/*=0*/,FilterFunc *pSocketToSerialFilterFunc/*=0*/,void *pSocketParam/*=0*/,bool *pbStop/*=0*/)
{
  serial.ClearError();
  enum {BUFSIZE=8192};
  void *pBuf=malloc(BUFSIZE);
  TRACE(_T("ConnectSocketToSerial: connected\n"));
  bool rc=true;
  try {
  /*
  { //hack
  unsigned int nWritten;//hack
  serial.Write(_T("+"),1,nWritten);//hack
  }//hack
    */
    while(rc && (0==pbStop || !(*pbStop))){
      unsigned int nRead=0;
      switch(SSRead (serial,socket,pBuf,BUFSIZE,nRead,pbStop)){
      case SS_SERIAL_READ: 
        VTRACE(_T("Serial:%d\n"),nRead);
        if(pSerialToSocketFilterFunc){
          rc=pSerialToSocketFilterFunc(pBuf,nRead,serial,socket,pSerialParam);
        }
        if(nRead && !socket.send(pBuf,nRead)){
          TRACE(_T("Failed to write to socket\n"));
          rc=false;
        }
        break;
      case SS_SOCKET_READ:
        unsigned int nWritten;
        VTRACE(_T("Socket:%d\n"),nRead);
        if(pSocketToSerialFilterFunc){
          rc=pSocketToSerialFilterFunc(pBuf,nRead,serial,socket,pSocketParam);
        }
        {
          LPTSTR c=(LPTSTR )pBuf;
          int nToWrite=nRead;
          while(nToWrite>0){
            if(!serial.Write(pBuf,nRead,nWritten)){
              TRACE(_T("Failed to write to serial\n"));
              rc=false;
              break;
            }
            nToWrite-=nWritten;
            c+=nWritten;
          }
        }
        break;
      // Error conditions:
      case SS_SERIAL_ERROR:
        TRACE(_T("SSRead serial error - %s\n"),(LPCTSTR)serial.ErrString());
        rc=false;
        break;
      case SS_SOCKET_ERROR:
        TRACE(_T("SSRead socket error - %s\n"),(LPCTSTR)socket.SocketErrString());
        rc=false;
        break;
      case SS_STOPPED:
        TRACE(_T("SSRead stopped\n"));
        rc=false;
        break;
      }
    }
  }
  catch (...){
    ERROR(_T("!!! ConnectSocketToSerial exception caught!!!\n"));
    free(pBuf);
    throw;
  }
  free(pBuf);
  return rc;
}

// Connect two tcp/ip ports together.
// Traffic is passed through pFunc, passed parameter pParam.
// The pFunc function:
//    may reallocate pBuf (using malloc/realloc etc...)
//    must leave pBuf allocated on exit
//    should not close either serial or socket
//    should leave writing to its caller
//    should return false if it wishes to terminate the connection (after caller has written output)
bool CeCosSocket::ConnectSocketToSocket (CeCosSocket &o,FilterFunc *pSocketToSocketFilterFunc1,FilterFunc *pSocketToSocketFilterFunc2,void *pParam,bool *pbStop)
{
  enum {BUFSIZE=8192};
  void *pBuf=malloc(BUFSIZE);
  TRACE(_T("ConnectSocketToSocket: connected\n"));
  bool rc=true;
  try {
    while(rc && (0==pbStop || !(*pbStop))){
      fd_set set;
      FD_ZERO(&set);
      FD_SET((unsigned)m_nSock, &set);
      FD_SET((unsigned)o.m_nSock, &set);
      struct timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      switch(::select(m_nSock,&set,0,0,&tv)){
      case -1:
        rc=false;
        break;
      case 1:
      case 2:
        {
          unsigned int nAvail=0;
          if(FD_ISSET((unsigned)m_nSock, &set) && Peek(nAvail) && recv(pBuf,nAvail)){
            //rc=pSocketToSocketFilterFunc1(pBuf,nAvail,socket,this,o);
            o.send(pBuf,nAvail);
          }
          if(FD_ISSET((unsigned)o.m_nSock, &set) && o.Peek(nAvail) && o.recv(pBuf,nAvail)){
            //rc=pSocketToSocketFilterFunc2(pBuf,nAvail,socket,o,this);
            send(pBuf,nAvail);
          }
        }
      case 0:
        break;
      }
    }
  }
  catch (...){
    TRACE(_T("!!! ConnectSocketToSocket exception caught!!!\n"));
    rc=false;
  }
  free(pBuf);
  return rc;
}

bool CeCosSocket::ConnectSocketToSerial (
                                         int nListenSock,LPCTSTR pszPort, int nBaud,
                                         FilterFunc *pSerialToSocketFilterFunc/*=0*/,void *pSerialParam/*=0*/,FilterFunc *pSocketToSerialFilterFunc/*=0*/,void *pSocketParam/*=0*/,
                                         bool *pbStop)
{
  bool rc=false;
  try{
    TRACE(_T("ConnectSocketToSerial : socket %d <--> %s\n"),nListenSock,pszPort);
    
    CeCosSerial serial;
    serial.SetBlockingReads(false);
    // Open serial device.
    if (!serial.Open(pszPort,nBaud)){
      ERROR(_T("Couldn't open port %s\n"),pszPort);
    } else {
      // Flush the serial buffer.
      serial.Flush();
      
      TRACE(_T("ConnectSocketToSerial: waiting for connection...\n"));
      CeCosSocket socket;
      if(!socket.Accept(nListenSock,pbStop)){
        ERROR(_T("ConnectSocketToSerial - couldn't accept\n"));
      } else {    
        rc=ConnectSocketToSerial (socket,serial,pSerialToSocketFilterFunc,pSerialParam,pSocketToSerialFilterFunc,pSocketParam,pbStop);
      }
    }
    TRACE(_T("ConnectSocketToSerial : done\n"));
  }
  catch(...){
    TRACE(_T("ConnectSocketToSerial !!!exception handled!!!\n"));
  }
  return rc;
}

String CeCosSocket::ClientName(int nClient) 
{
  char ip[4];
  memcpy(ip,&nClient,4);
  struct hostent *he=::gethostbyaddr((const char *)ip,4,AF_INET);
  String str;
  if(he){
    str=String::CStrToUnicodeStr(he->h_name);
  } else {
    str.Format(_T("%u.%u.%u.%u"),ip[0],ip[1],ip[2],ip[3]);
  }
  return str;
}

String CeCosSocket::HostPort(LPCTSTR pszHost,int nPort) 
{
  String str;
  str.Format(_T("%s:%d"),pszHost,nPort);
  return str;
}

// Split the string into host:port parts.  Result tells us whether it was successful.
bool CeCosSocket::ParseHostPort (LPCTSTR pszHostPort, String &strHost, int &nPort)
{
  int n=_stscanf(pszHostPort,_T("%[^:]:%d"),strHost.GetBuffer(_tcslen(pszHostPort)),&nPort);
  strHost.ReleaseBuffer();
  return 2==n && nPort>0 && nPort<=0xffff;
}

// Is the string in the form host:port?
bool CeCosSocket::IsLegalHostPort (LPCTSTR pszHostPort)
{
  int nPort=0;
  String strHost;
  return ParseHostPort(pszHostPort,strHost,nPort);
}

// Translate a timeout that may be one of the special values DEFAULTTIMEOUT or NOTIMEOUT to a value in milliseconds.
Duration CeCosSocket::TimeoutDuration(Duration dTimeout)
{
  switch(dTimeout){
  case DEFAULTTIMEOUT:
    dTimeout=m_nDefaultTimeout;
    break;
  case NOTIMEOUT:
    dTimeout=0x7fffffff;
    break;
  default:
    break;
  }
  return dTimeout;
}

String CeCosSocket::SocketErrString() { 
  return SocketErrString(m_nErr); 
}


bool CeCosSocket::SameHost(LPCTSTR host1, LPCTSTR host2)
{
  return 0==_tcscmp(host1,host2) || (GetHostByName(host1)==GetHostByName(host2));
}

bool CeCosSocket::Init()
{
#ifdef _WIN32
  WSADATA wsaData;
  WORD wVersionRequested = MAKEWORD( 2, 0 ); 
  WSAStartup( wVersionRequested, &wsaData );
#endif
  return true;
}

void CeCosSocket::Term()
{
#ifdef _WIN32
  WSACleanup();
#endif
}

LPCTSTR CeCosSocket::MyHostName()
{
  static String str;
  if(str.empty()){
    char szMyname[256];
    if(0==gethostname(szMyname,sizeof szMyname)){
      str=String::CStrToUnicodeStr(szMyname);
    }
  }
  return str;
}

LPCTSTR CeCosSocket::MySimpleHostName()
{
  static String str;
  if(str.empty()){
    str=MyHostName();
    // Remove all after a '.'
    LPCTSTR c=_tcschr(str,_TCHAR('.'));
    if(c){
      str.resize(c-(LPCTSTR)str);
    }
  }
  return str;
}

const String CeCosSocket::GetHostByName(LPCTSTR pszHost)
{
  typedef std::map<String,String> MapStringToString;
  static MapStringToString hostmap;
  MapStringToString::iterator it=hostmap.find(pszHost);
  if(hostmap.end()==it){
    char *h=0; // avoid erroneous gcc warning message
    h=String(pszHost).GetCString();
    char ip[16];
    struct hostent* host_dat;
    if (0!=(host_dat=::gethostbyname(h))){
      char *c=inet_ntoa( *( (struct in_addr *)host_dat->h_addr_list[0] )  );
      if(c){
        strcpy(ip,c);
        hostmap[pszHost]=String::CStrToUnicodeStr(ip);
      }
    }
    delete [] h;
    return String::CStrToUnicodeStr(ip);
  } else {
    return it->second;
  }
}
