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
//        eCosTest.cpp
//
//        Test class
//
//=================================================================
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     sdf
// Contributors:  sdf
// Date:          1999-04-01
// Description:   This class abstracts a test for use in the testing infrastructure
// Usage:
//
//####DESCRIPTIONEND####
///////////////////////////////////////////////////////////////////////////////
#include "eCosStd.h"
#include "eCosTest.h"
#include "eCosTestPlatform.h"
#include "eCosTrace.h"
#include "TestResource.h"
#include "eCosTestUtils.h"
#include "eCosSocket.h"
#include "eCosSerial.h"
#include "eCosTestSerialFilter.h"
#include "eCosTestDownloadFilter.h"
#include "Properties.h"
#include "Subprocess.h"

#define WF(n) (n+50)/1000,((n+50)%1000)/100     // Present n as whole and fractional part.  Round to nearest least significant digit
#define WFS   _T("%u.%u")                           // The format string to output the above

LPCTSTR  const CeCosTest::arResultImage[1+CeCosTest::StatusTypeMax]=
{_T("NotStarted"), _T("NoResult"), _T("Inapplicable"), _T("Pass"), _T("DTimeout"), _T("Timeout"), _T("Cancelled"), _T("Fail"), _T("AssertFail"), _T("Unknown")};

CeCosTest *CeCosTest::pFirstInstance=0;
int CeCosTest::InstanceCount=0;

LPCTSTR  const CeCosTest::arServerStatusImage[1+CeCosTest::ServerStatusMax]={
  _T("Busy"), _T("Ready"), _T("Can't run"), _T("Connection failed"), _T("Locked"), _T("Bad server status")};
LPCTSTR  CeCosTest::ExecutionParameters::arRequestImage [1+ExecutionParameters::RequestTypeMax]={
  _T("Run"), _T("Query"), _T("Lock"), _T("Unlock"), _T("Stop"), _T("Bad request") };
  
static bool CALLBACK IsCancelled(void *pThis)
{
  return CeCosTest::Cancelled==((CeCosTest *)pThis)->Status();
}

// Ctors and dtors:
CeCosTest::CeCosTest(const ExecutionParameters &e, LPCTSTR pszExecutable,LPCTSTR pszTitle):
  m_pspPipe(0),
  m_nStrippedSize(0),
  m_nFileSize(0),
  m_bDownloading(false),
  m_pSock(0),
  m_ep(e),
  m_strTitle(pszTitle),
  m_Status(NotStarted),
  m_nDownloadTime(0),
  m_nTotalTime(0),
  m_nMaxInactiveTime(0),
  m_pResource(0),
  m_psp(0)
{
  
  assert(e.Platform());
  
  SetExecutable (pszExecutable);
  
  TRACE(_T("%%%% Create test instance %08x count:=%d\n"),this,InstanceCount+1);  

  // By recording the path now, we ensure processes are always run in the context in which the test instance
  // is created (important for the ConfigTool to be able to call PrepareEnvironment).

#ifdef _WIN32
  // for some reason _tgetenv() doesn't return the PATH set
  // by PrepareEnvironment() so use GetEnvironmentVariable() instead
  // JLD - 2000-06-09
  String strPath;
  int nSize=GetEnvironmentVariable(_T("PATH"), NULL, 0);
  GetEnvironmentVariable(_T("PATH"), strPath.GetBuffer(nSize), nSize);
  strPath.ReleaseBuffer();
  m_strPath=strPath;
#else
  LPCTSTR pszPath=_tgetenv(_T("PATH"));
  if(pszPath){
    m_strPath=pszPath;
  }
#endif
  
  ENTERCRITICAL;
  InstanceCount++;
  m_pNextInstance=pFirstInstance;
  if(m_pNextInstance){
    m_pNextInstance->m_pPrevInstance=this;
  } 
  m_pPrevInstance=0;
  pFirstInstance=this;
  LEAVECRITICAL;
  
}

CeCosTest::~CeCosTest()
{
  for(int i=0;i<(signed)m_arpExecsp.size();i++){
    delete (CSubprocess *)m_arpExecsp[i];
  }
  delete m_pspPipe;

  TRACE(_T("%%%% Delete test instance %08x\n"),this);
  Cancel();
  CloseSocket();
  if(m_pResource){
    m_pResource->Release();
    //delete m_pResource;
    //m_pResource=0;
  }
  
  VTRACE(_T("~CeCosTest(): EnterCritical and decrease instance count\n"));
  ENTERCRITICAL;
  InstanceCount--;
  TRACE(_T("%%%% Destroy instance.  Instance count:=%d\n"),InstanceCount);
  if(pFirstInstance==this){
    pFirstInstance=m_pNextInstance;
  }
  if(m_pPrevInstance){
    m_pPrevInstance->m_pNextInstance=m_pNextInstance;
  }
  if(m_pNextInstance){
    m_pNextInstance->m_pPrevInstance=m_pPrevInstance;
  }
  LEAVECRITICAL;
}

bool CeCosTest::RunRemote (LPCTSTR pszRemoteHostPort)
{
  bool rc=false;
  TRACE(_T("RunRemote\n"));
  m_strExecutionHostPort=pszRemoteHostPort;
  m_Status=NotStarted;
  
  VTRACE(_T("RemoteThreadFunc()\n"));
  
  // Find a server.
  ConnectForExecution();
  if(Cancelled!=Status()){       
    if(m_ep.Platform()->ServerSideGdb()){
      // The executable is transmitted to the server for execution.
      // Send file size
      if(m_pSock->sendInteger(m_nFileSize,_T("file size"))&&m_nFileSize>0){
        int nBufSize=MIN(10000,m_nFileSize);
        Buffer b(nBufSize);
        TRACE(_T("Sending [%d bytes]\n"), m_nFileSize);
        int nToSend=m_nFileSize;
        FILE *f1=_tfopen(m_strExecutable,_T("rb"));
        if(0==f1){
          Log(_T("Failed to open %s - %s\n"),(LPCTSTR)m_strExecutable,strerror(errno));
        } else {
          while (nToSend>0){
            int nRead=fread( b.Data(), 1, nBufSize, f1);
            if(nRead<=0){
              Log(_T("Failure reading %s - %s\n"),(LPCTSTR)m_strExecutable,strerror(errno));
              break;
            }
            if(!send( b.Data(), nRead, _T("executable"))){
              Log(_T("Failure sending %s - %s\n"),(LPCTSTR)m_strExecutable,(LPCTSTR)m_pSock->SocketErrString());
              break;
            }
            nToSend-=nRead;
          }
          fclose(f1);
          f1=0;
          if(nToSend>0){
            TRACE(_T("done [%d bytes sent]\n"),m_nFileSize-nToSend);
            Log(_T("Failed to transmit %s - %d/%d bytes sent\n"),(LPCTSTR)m_strExecutable,m_nFileSize-nToSend,m_nFileSize);
          } else {
            TRACE(_T("done\n"));
            rc=true;
          }
        }
        if(!recvResult(9*1000*60)){ // nine minutes
          Log(_T("Failed to receive result from remote server\n"));
          rc=false;
        }
        m_pSock->sendInteger(456); // send an ack [n'importe quoi]
        CloseSocket();
      }
    } else {
      // The server sets up a connection between port and tcp/ip socket, and gdb is run locally
      // Big timeout here because we have to wait for the target to be reset
      // We do this:
      // do {
      //     target ready indicator (0==fail, 1==ready, 2==fail and will retry)
      //     any output so far
      // } while (2==target ready indicator)
      // read host:port
      String strHostPort;
      if(GetTargetReady(strHostPort)){
        // Fix up a resource to represent permission to use the host:port we have been told about
        CTestResource resource;
        resource.SetTarget(m_ep.PlatformName());
        resource.SetDownload(strHostPort,0);
        m_pResource=&resource;
        RunLocal();
        m_pResource=0;
        m_pSock->sendInteger(Status(),_T("Terminating ack"));
        m_pSock->Close();
        rc=true;
      }
    }
  }
  TRACE(_T("RemoteThreadFunc - exiting\n"));
  return rc;
}

// Run the test locally
bool CeCosTest::RunLocal()
{
  bool rc=false;

  TRACE(_T("RunLocal %s\n"),(LPCTSTR)Executable());

  if(!CeCosTestUtils::IsFile(Executable())){
    Log(_T("Cannot run - %s is not a file\n"),(LPCTSTR)Executable());
  } else if(0==m_pResource && 0==CTestResource::Count(m_ep)){
    Log(_T("Cannot run a %s test\n"),(LPCTSTR)m_ep.PlatformName());
  } else {
    
    m_Status=NotStarted;

    TRACE(_T("LocalThreadFunc - target=%s\n"),(LPCTSTR)m_ep.PlatformName());
    // Acquire a port (our caller may have done this for us)
    VTRACE(_T("LocalThreadFunc():Trying to acquire a port\n"));
    if(0==m_pResource){
      for(;;){
        m_pResource=CTestResource::GetResource(m_ep);
        if(m_pResource||Cancelled==Status()){
          break;
        }
        CeCosThreadUtils::Sleep(2000);
        TRACE(_T("Waiting for a port\n"));
      }
    }
    VTRACE(_T("\nPort acquired!\n"));
    
    if(Cancelled!=Status()){
      // This means we have acquired a local port 
      bool bTargetReady=false;
      if(!m_pResource->HasReset()){
        bTargetReady=true;
      } else {
        bTargetReady=(CResetAttributes::RESET_OK==m_pResource->Reset(0,this));
      }
      // we may proceed to execute the test
      if(bTargetReady){
        SetStatus(NotStarted);

        if(NOTIMEOUT==m_ep.DownloadTimeout()){
          // No elapsed timeout given - calculate from knowledge of executable size and baud rate
          // 10 baud ~= 1 byte/sec, but we halve this to account for download in hex :-(
          // We use a minimum of 30 seconds and double the calculated result for safety
          // Note that the baud rate is generally unknown on the client side.
          int nBaud=m_pResource->Baud();
          if(0==nBaud){
            CTestResource *pExecutionResource=CTestResource::Lookup(m_strExecutionHostPort);
            if(pExecutionResource){
              nBaud=pExecutionResource->Baud();
            } 
          }
          if(0==nBaud){
            nBaud=38400;
          }

          int nBytesPerSec=(nBaud/10)/2; // division by 2 assumes download in "ascii" (2 bytes/char)
          m_ep.SetDownloadTimeout (1000*MAX(30,2*(m_nStrippedSize/nBytesPerSec)));
          TRACE(_T("Estimated download time %d sec (%d bytes @ %d bytes/sec [%d baud])\n"),m_nStrippedSize/nBytesPerSec,m_nStrippedSize,nBytesPerSec,nBaud);
        }

        TRACE(_T("Active timeout=%d download timeout=%d\n"),m_ep.ActiveTimeout(), m_ep.DownloadTimeout());

        GetInferiorCommands(m_arstrInferiorCmds);
        String strInferior(m_ep.Platform()->Inferior());
        strInferior.Replace(_T("%e"),CygPath(m_strExecutable),true);
        RunInferior(strInferior);
        rc=true;          
      }
    }
    if(m_pResource){
      m_pResource->Release();
      m_pResource=0;
    }
    TRACE(_T("RunLocal - exiting\n"));
  }

  return rc;
}

void CeCosTest::Cancel ()
{
  SetStatus(Cancelled);
}

CeCosTest::ServerStatus CeCosTest::Connect (LPCTSTR pszHostPort, CeCosSocket *&pSock, const ExecutionParameters &e,String &strInfo,Duration dTimeout)
{
  // Find out whether this host is receptive
  ServerStatus s=CONNECTION_FAILED;
  pSock=new CeCosSocket(pszHostPort,dTimeout);
  int nStatus;    
  if(pSock->Ok() &&
    pSock->sendString(e.Image(), _T("execution parameters")) &&
    pSock->recvInteger(nStatus,_T("ready status")) &&
    pSock->recvString(strInfo)){
    s=(ServerStatus)MIN(nStatus,ServerStatusMax);
  }
  if(SERVER_READY!=s || ExecutionParameters::RUN!=e.Request()){
    delete pSock;
    pSock=0;
  }
  return s;
}

// Initiate a connection to hostName:nPort and acquire the ready status [retry until this is achieved]
// The socket (m_pSock) is left open.
// This function is either called with m_strExecutionHostPort already set to a desired server
// or else m_strExecutionHostPort empty (in which case the server is / dynamically)

void CeCosTest::ConnectForExecution ()
{
  bool bSchedule=(0==m_strExecutionHostPort.size());
  Duration nDelay=2000;
  
  m_pSock=0;
  
  bool *arbHostTried=0;
  
  while(Cancelled!=Status()){
    StringArray arstrHostPort,arstrTries;
    int nChoices;
    
    if(bSchedule){
      if(!CTestResource::GetMatches(m_ep,arstrHostPort)){
        Log(_T("Could not establish matches\n"));
        continue;
      }
      nChoices=arstrHostPort.size();
      if(nChoices>0){
        TRACE(_T("ConnectForExecution: choices are:\n"));
        for(int i=0;i<nChoices;i++){
          TRACE(_T("\t%s\n"),(LPCTSTR)arstrHostPort[i]);
        }
      }
    } else {
      // Server has already been picked by caller
      nChoices=1;
      String str;
      arstrHostPort.push_back(m_strExecutionHostPort);
    }
    
    if(nChoices>0){
      delete [] arbHostTried;
      arbHostTried=new bool[nChoices];
      for(int i=0;i<nChoices;i++){
        arbHostTried[i]=false;
      }
      
      // Loop around the choices
      for(int nUntried=nChoices;nUntried>0;nUntried--) {
        // Select one we haven't tried already:
        int nChoice;        
        do {
          nChoice=rand() % nChoices;
        } while (arbHostTried[nChoice]);
        
        m_strExecutionHostPort=arstrHostPort[nChoice];
        
        TRACE(_T("ConnectForExecution: chosen %s\n"),(LPCTSTR)m_strExecutionHostPort);
        if(CeCosSocket::IsLegalHostPort(m_strExecutionHostPort)){
          // If we're using the resource server we had better check that the host
          // we are about to lock has not been resource-locked (the other match checks
          // will of course always succeed)
          String strInfo; 
          ServerStatus s=bSchedule && !CTestResource::Matches(m_strExecutionHostPort,m_ep)?SERVER_LOCKED:  
            Connect(m_strExecutionHostPort,m_pSock,m_ep,strInfo);
          arbHostTried[nChoice]=true;        
          TRACE(_T("Connect: %s says %s %s\n"),(LPCTSTR)m_strExecutionHostPort,(LPCTSTR)Image(s),(LPCTSTR)strInfo);
          CTestResource *pResource=CTestResource::Lookup(m_strExecutionHostPort);
          if(pResource){
            String str;
            str.Format(_T("%s %s %s"),(LPCTSTR)pResource->Image(),(LPCTSTR)strInfo,(LPCTSTR)Image(s));
            arstrTries.push_back(str);
          }
          if(SERVER_READY==s){
            // So that's ok then.  We're outta here.
            INTERACTIVE(_T("Connected to %s\n"),(LPCTSTR)m_strExecutionHostPort);
            goto Done;
          } else {
            delete m_pSock;
            m_pSock=0;
          }
        }
      } 
    }
    
    INTERACTIVE(_T("Warning - could not connect to any test servers:\n"));
    if(arstrTries.size()>0){
      for(unsigned int i=0;i<arstrTries.size();i++){
        INTERACTIVE(_T("    %s\n"),(LPCTSTR)arstrTries[i]);
      }
    } else {
      INTERACTIVE(_T("No servers available to execute %s test:\n"),(LPCTSTR)m_ep.PlatformName());
      ENTERCRITICAL;
      for(CTestResource *pResource=CTestResource::First();pResource;pResource=pResource->Next()){
        INTERACTIVE(_T("    %s\n"),(LPCTSTR)pResource->Image());
      }
      LEAVECRITICAL;
    }
    INTERACTIVE(_T("Retry in %d seconds...\n"),nDelay/1000);
    
    // We have tried all possibilities - sleep before retrying
    CeCosThreadUtils::Sleep(nDelay);
    
    if(Cancelled==m_Status){
      TRACE(_T("ConnectForExecution : cancelled\n"));
      goto Done;
    }
    if(nDelay<20*1000){
      nDelay+=rand() % 500;
    }
  }
Done:    
  delete [] arbHostTried;
}

void CeCosTest::SetStatus (StatusType status)
{
  ENTERCRITICAL;
  if((int)status>(int)m_Status){
    TRACE(_T("Status <- %s\n"),(LPCTSTR)Image(status));
    m_Status=status;
  }
  LEAVECRITICAL;
}

bool CeCosTest::WaitForAllInstances(int nPoll,Duration nTimeout)
{
  Time t0=Now();
  while(InstanceCount>0){
    CeCosThreadUtils::Sleep(nPoll);
    if(NOTIMEOUT!=nTimeout && Now()-t0>nTimeout){
      return false;
    }
  }
  return true;
}

void CeCosTest::DeleteAllInstances()
{
  while(pFirstInstance){
    delete pFirstInstance;
  }
}

void CeCosTest::CancelAllInstances()
{
  ENTERCRITICAL;
  for(CeCosTest *pTest=pFirstInstance;pTest;pTest=pTest->m_pNextInstance){
    pTest->Cancel();
  }
  LEAVECRITICAL;
}

// The same format is used for _stscanf as for Format (which is like printf), so restrict to the format specifiers
// the former is happy with.  In particular, do not use %-3s etc...

LPCTSTR CeCosTest::pszFormat=
// 1999-01-15 17:24:36 Fireblade:5002 MN10300 sin.exe 219k/134k Pass sin download=106.3/117.0 Total=107.6 Max inactive=1.0/300.0    
_T("%04d-%02d-%02d %02d:%02d:%02d ")                   // Time
_T("%15s ")                                            // Execution host:port
_T("%16s ")                                             // Target
_T("%30s ")                                            // Executable tail
_T("%11s ")                                            // Result
_T("%dk/%dk ")                                         // Sizes
_T("D=") WFS _T("/") WFS _T(" Total=") WFS _T(" ")     // Times
_T("E=") WFS _T("/") WFS _T(" ")
_T("\"%s\"");

bool CeCosTest::Value (
  LPCTSTR pszStr, 
  struct tm &t,
  StatusType &status,
  String &target,
  String &strExecutionHostPort,
  String &strExecutableTail,
  String &strTitle,

  int &nFileSize,
  Duration &nTotalTime,
  Duration &nMaxInactiveTime,
  Duration &nDownloadTime,
  Duration &nDownloadTimeout,
  Duration &nActiveTimeout,

  int &nDownloadedSize)
{
  int nLen=_tcslen(pszStr);
  String strStatus;

  nFileSize=nTotalTime=nMaxInactiveTime=nDownloadTime=nDownloadTimeout=nActiveTimeout=nDownloadedSize=0;
  
  int nTotalTimeFrac=0;
  int nMaxInactiveTimeFrac=0;
  int nActiveTimeoutFrac=0;
  int nDownloadTimeFrac=0;
  int nDownloadTimeoutFrac=0;
  
  static String strFormat;
  if(0==strFormat.size()){
    // Construct a version of the format string sans length attributes for %s items
    LPCTSTR c=pszFormat;
    TCHAR *d=strFormat.GetBuffer(_tcslen(pszFormat));
    while(_TCHAR('\0')!=*c){
      if(_TCHAR('%')==c[0] && _istdigit(c[1])){
        *d++=_TCHAR('%');
        do {
          c++;
        } while (_istdigit(*c));
      }
      *d++=*c++;
    }
    *d=_TCHAR('\0');
    strFormat.ReleaseBuffer();
  }
  
  _stscanf(pszStr,
    strFormat,
    &t.tm_year,&t.tm_mon,&t.tm_mday,
    &t.tm_hour,&t.tm_min,&t.tm_sec,         // Time of day
    strExecutionHostPort.GetBuffer(1+nLen),       // Execution host:port
    target.GetBuffer(1+nLen),                  // Target
    strExecutableTail.GetBuffer(1+nLen),          // Executable
    strStatus.GetBuffer(1+nLen),                  // Result
    &nDownloadedSize,&nFileSize,            // Sizes
    &nDownloadTime,&nDownloadTimeFrac,      // Times
    &nDownloadTimeout,&nDownloadTimeoutFrac,
    &nTotalTime,&nTotalTimeFrac,
    &nMaxInactiveTime,&nMaxInactiveTimeFrac,
    &nActiveTimeout,&nActiveTimeoutFrac,
    strTitle.GetBuffer(1+nLen)                    // Title
    );
  
  strExecutionHostPort.ReleaseBuffer();
  target.ReleaseBuffer();
  strExecutableTail.ReleaseBuffer();
  strStatus.ReleaseBuffer();
  strTitle.ReleaseBuffer();
  status=StatusTypeValue(strStatus);

  LPCTSTR c1=_tcschr(pszStr,_TCHAR('"'));
  if(c1){
    c1++;
    LPCTSTR c2=_tcschr(c1+1,_TCHAR('"'));
    if(c2){
      strTitle=String(c1,c2-c1);
    }
  }
  
  nTotalTime=nTotalTime*1000+nTotalTimeFrac*100;
  nMaxInactiveTime=nMaxInactiveTime*1000+nMaxInactiveTimeFrac*100;
  nActiveTimeout=nActiveTimeout*1000+nActiveTimeoutFrac*100;
  nDownloadTime=nDownloadTime*1000+nDownloadTimeFrac*100;
  nDownloadTimeout=nDownloadTimeout*1000+nDownloadTimeoutFrac*100;
  
  nFileSize*=1024;
  nDownloadedSize*=1024;
  t.tm_year-=1900;
  t.tm_mon--;
  return t.tm_year>=0 && t.tm_year<=200 && t.tm_mon>=0 && t.tm_mon<=11 && t.tm_mday>=1 && t.tm_mday<=31 && t.tm_hour>=0 && t.tm_hour<=23 && t.tm_min>=0 && t.tm_min<=59 && t.tm_sec>=0 && t.tm_sec<=59 &&
    status!=StatusTypeMax 
    //&& exetype!=ExecutionParameters::ExecutableTypeMax
    ;
}
  
const String CeCosTest::ResultString(bool bIncludeOutput) const
{
  String strResultString;
  String strTitle(m_strTitle);
  String strExecutionHostPort(m_strExecutionHostPort);
  
  if(0==strTitle.size()){
    strTitle=CeCosSocket::MySimpleHostName();
    strTitle+=_TCHAR(':');
    strTitle+=m_strExecutable; 
  }
  
  if(0==strExecutionHostPort.size()){
    strExecutionHostPort=CeCosSocket::MySimpleHostName();
    strExecutionHostPort+=_T(":0");
  }
  
  ENTERCRITICAL;
  time_t ltime;
  time(&ltime);
  struct tm *now=localtime( &ltime );
  
  strResultString.Format(
    pszFormat,
    1900+now->tm_year,1+now->tm_mon,now->tm_mday,
    now->tm_hour,now->tm_min,now->tm_sec,               // Time of day
    (LPCTSTR)strExecutionHostPort,                      // Execution host:port
    (LPCTSTR)m_ep.PlatformName(),                       // Target
    (LPCTSTR)CeCosTestUtils::Tail(m_strExecutable),     // Executable
    (LPCTSTR)Image(Status()),                           // Result
    m_nStrippedSize/1024,m_nFileSize/1024,              // Sizes
    WF(m_nDownloadTime),WF(m_ep.DownloadTimeout()),WF(m_nTotalTime),// Times
    WF(m_nMaxInactiveTime),WF(m_ep.ActiveTimeout()),
    (LPCTSTR)strTitle                                   // Title
    );
  if(bIncludeOutput && m_strOutput.size()>0){                            
    strResultString+=_TCHAR('\n');
    strResultString+=m_strOutput;
  }
  LEAVECRITICAL;
  return strResultString;
}

// Run as a server, listening on the port given as parameter
bool CeCosTest::RunAgent(int nTcpPort)
{
  bool bLocked=false;
  
  // Create socket
  int nSock = CeCosSocket::Listen(nTcpPort);
  int nLastClient=0;
  if (-1!=nSock) {
    for (;;) {
      try {
        CeCosSocket *pSock=new CeCosSocket(nSock); // AcceptThreadFunc deletes if not deleted below
        String str;
        // Read the execution parameters
        if(!pSock->recvString(str)){
          // Socket error on the recv - nothing much we can do
          TRACE(_T("RunAgent : could not read execution parameters\n"));
          delete pSock;
          pSock=0;
        } else {
          ExecutionParameters e;
          e.FromStr(str);
          TRACE(_T("Execution parameters: %s\n"),(LPCTSTR)e.Image());
          ServerStatus s;
          CTestResource *pPort=0;
          String strInfo;

          switch(e.Request()) {
            case ExecutionParameters::LOCK:
              if(bLocked){
                s=SERVER_BUSY;
              } else {
                WaitForAllInstances(1000,NOTIMEOUT);
                bLocked=true;
                s=SERVER_LOCKED;
              }
              break;
            case ExecutionParameters::UNLOCK:
              if(bLocked){
                bLocked=false;
                s=SERVER_READY;
              } else {
                s=SERVER_BUSY;
              }
              break;
            case ExecutionParameters::QUERY:
              if (bLocked) {
                s=SERVER_LOCKED;
              } else {
                s=SERVER_BUSY;
                ENTERCRITICAL;
                for(CTestResource *pResource=CTestResource::First();pResource;pResource=pResource->Next()){
                  if(!pResource->InUse()){
                    s=SERVER_READY;
                    break;
                  }
                }
                LEAVECRITICAL;
                if(SERVER_READY!=s){
                  strInfo.Format(_T("serving %s"),(LPCTSTR)CeCosSocket::ClientName(nLastClient));
                }
              }
              break;
            case ExecutionParameters::RUN:
              if(NULL==e.Platform()){
                // Looks like a confused client ...
                strInfo.Format(_T("Bad target value %s read from client\n"),(LPCTSTR)str);
                s=SERVER_CANT_RUN;
              } else if(0==CTestResource::Count(e)){
                // No chance of running this test
                strInfo.Format(_T("Cannot run a %s test from this server\n"),(LPCTSTR)e.PlatformName());
                s=SERVER_CANT_RUN;
              } else if (bLocked) {
                s=SERVER_LOCKED;
              } else {
                pPort=CTestResource::GetResource(e);
                if(0==pPort){
                  // We must disappoint our client
                  strInfo.Format(_T("serving %s"),(LPCTSTR)CeCosSocket::ClientName(nLastClient));
                  s=SERVER_BUSY;
                } else {
                  s=SERVER_READY;
                  nLastClient=pSock->Client();
                }
              }
              break;
            case ExecutionParameters::STOP:
              s=SERVER_READY;
              break;
            default:
              s=SERVER_CANT_RUN;
          }
          
#ifndef VERBOSE
          if(ExecutionParameters::QUERY!=e.Request())
#endif
            TRACE(_T("RunAgent : %s request tActive=%d tDownload=%d Target=%s Reply status=%s %s\n"),
              (LPCTSTR)e.Image(e.Request()),e.ActiveTimeout(),e.DownloadTimeout(),
              (LPCTSTR)e.PlatformName(),
              (LPCTSTR)Image(s),(LPCTSTR)strInfo);
          
          bool bSendok=pSock->sendInteger(s) && pSock->sendString(strInfo);
          
          if(SERVER_READY==s && bSendok && ExecutionParameters::RUN==e.Request()){
            
            // Create a new class instance
            // AcceptThreadFunc deletes the instance and closes new_sock
            // RunLocal, called by AcceptThreadFunc, releases the port
            // No need for meaningful callback, but must run asynchronously
            
            int nAuxPort=30000;
            int nAuxListenSock=-1;
      
            do {
              nAuxListenSock=CeCosSocket::Listen(nAuxPort);
            } while (-1==nAuxListenSock && nAuxPort++<=0xffff);
            
            if(-1==nAuxListenSock){
              ERROR(_T("Couldn't find a socket to bind to for RDI\n"));
            } else {
              
              CeCosTest *pTest=new CeCosTest(e,NULL);
              pTest->m_nAuxPort=nAuxPort;
              pTest->m_nAuxListenSock=nAuxListenSock;
              pTest->m_pSock=pSock;
              pTest->m_strExecutionHostPort=CeCosSocket::HostPort(CeCosSocket::MyHostName(),nTcpPort);
              pTest->m_pResource=pPort;
              CeCosThreadUtils::RunThread(SAcceptThreadFunc,pTest,_T("SAcceptThreadFunc"));
              // AcceptThreadFunc deletes pSock
            }
            
          } else {
            delete pSock;
            pSock=0;
            if(pPort){
              pPort->Release();
              pPort=0;
            }
            if(CeCosTest::ExecutionParameters::STOP==e.Request()){
              CancelAllInstances();
              WaitForAllInstances(1000,20*1000);
              break;
            }
          }
        }
      }
      catch(...){
        TRACE(_T("!!! Exception caught in RunAgent()\n"));
      }
    }
    CeCosSocket::CloseSocket (nSock);
  }
    
  return false;
}

CeCosTest::StatusType CeCosTest::StatusTypeValue(LPCTSTR  pszStr)
{
  for(int i=0;i<StatusTypeMax;i++){
    StatusType t=(StatusType)i;
    if(0==_tcsicmp(Image(t),pszStr)){
      return t;
    }
  }
  return StatusTypeMax;
}

// Thread to run ConnectSocketToSerial
void CeCosTest::ConnectSocketToSerialThreadFunc()
{
  TRACE(_T("ConnectSocketToSerialThreadFunc sock=%d\n"),m_nAuxListenSock);
    
  CeCosTestSerialFilter serial_filter;
  CeCosTestDownloadFilter download_filter;
  
  CeCosSerial serial;
  serial.SetBlockingReads(false);
  bool rc=false;
  // Open serial device.
  if (!serial.Open(m_pResource->Serial(),m_pResource->Baud())){
    ERROR(_T("Couldn't open port %s\n"),m_pResource->Serial());
  } else {
    for(;;){
      // Flush the serial buffer.
      serial.Flush();
      TRACE(_T("ConnectSocketToSerial: waiting for connection...\n"));
      CeCosSocket socket;
      if(!socket.Accept(m_nAuxListenSock,&m_bStopConnectSocketToSerial)){
        ERROR(_T("ConnectSocketToSerial - couldn't accept: %s\n"),(LPCTSTR)socket.SocketErrString());
        break;
      } else if (m_pSock->Client() != socket.Client()){    
        // Make sure the client is who we think it is...
        TRACE(_T("ConnectSocketToSerialThread - illegal connection attempted from %s\n"),(LPCTSTR)socket.ClientName(socket.Client()));
      } else {
        try {
            rc=CeCosSocket::ConnectSocketToSerial(socket,serial,m_ep.m_bUseFilter?SerialFilterFunction:NULL, (void*)&serial_filter, m_ep.m_bUseFilter?DownloadFilterFunction:NULL, (void*)&download_filter, &m_bStopConnectSocketToSerial);
          
          // If the download filter was just active, it may
          // allow the session to continue.
          if(!download_filter.ContinueSession()){
            break;
          }
          
        }
        catch (LPCTSTR pszMsg){
          Log(_T("!!! ConnectSocketToSerial exception caught: %s!!!\n"),pszMsg);
          rc=false;
          break;
        }
        catch (...){
          Log(_T("!!! ConnectSocketToSerial exception caught!!!\n"));
          rc=false;
          break;
        }
      }
    }
  }
  TRACE(_T("ConnectSocketToSerial : done\n"));
  CeCosSocket::CloseSocket(m_nAuxListenSock);
}

static bool CALLBACK DerefBool(void *pParam)
{
  return *(bool *)pParam;
}

// Function called (on a separate thread) to process a successful connection to the RunAgent loop
// In the case of a simulator server, we can have many of these active at the same time.
void CeCosTest::AcceptThreadFunc()
{
  if(m_ep.Platform()->ServerSideGdb()){
    // We dream up a temporary name for the executable
    ENTERCRITICAL;
    m_strExecutable.Format(_T("%s-%s-%d"),_ttmpnam(0),(LPCTSTR)m_ep.PlatformName(),m_nAuxPort);
    LEAVECRITICAL;

    int n;
    if(m_pSock->recvInteger(n,_T("file size"))){
      m_nFileSize=n;
      // Read file from the socket
      bool bCanRun=true;
      TRACE(_T("AcceptThreadFunc file size=%d reading...\n"),m_nFileSize);
      FILE *f2;
      f2=_tfopen(m_strExecutable,_T("wb"));
      if(0==f2){
        Log(_T("Could not create %s - %s\n"),(LPCTSTR)m_strExecutable,strerror(errno));
        bCanRun=false;
      }
      unsigned int nBufSize=MIN(100000,m_nFileSize);
      Buffer b(nBufSize);
      unsigned int nWritten=0;
      unsigned int nRead=0;
      while(nRead<m_nFileSize){
        int nToRead=MIN(nBufSize,m_nFileSize-nRead);
        if(!recv( b.Data(), nToRead, _T("executable"))){
          break;
        }
        nRead+=nToRead;
        if(0!=f2){
          char *c=(char *)b.Data();
          while(nToRead>0){
            int w=fwrite(c,1,nToRead,f2);
            if(-1==w){
              Log(_T("Write error on %s - %s\n"),(LPCTSTR)m_strExecutable,strerror(errno));
              bCanRun=false;
              break;
            }
            nWritten+=w;
            c+=w;
            nToRead-=w;
          }
        }
      }
      TRACE(_T("Accept - done reading [%d bytes read, %d bytes written]\n"),nRead,nWritten);
      if(0!=f2){
        fclose(f2);
        _tchmod(m_strExecutable,00700); // user read, write and execute
      } 
      if(0!=f2 && m_nFileSize!=nWritten){
        Log(_T("Failed to create %s correctly [%d/%d bytes written]\n"),(LPCTSTR)m_strExecutable, nWritten, m_nFileSize);
        bCanRun=false;
      }
      SetExecutable(m_strExecutable); // to set stripped length and title
      RunLocal();
      _tunlink(m_strExecutable);
    }
    sendResult();
    m_pSock->recvInteger(n); // receive an ack
  } else {
    // Client-side GDB
    bool bTargetReady;
    if(_TCHAR('\0')==*(m_pResource->ResetString())){
      bTargetReady=true;
      TRACE(_T("No reset possible\n"));
    } else {
      Log(_T("Resetting target using %s"),(LPCTSTR)m_pResource->ResetString());
      bTargetReady=(CResetAttributes::RESET_OK==m_pResource->Reset(ResetLogFunc,this));
    }
    TRACE(_T("Send Target Ready indicator=%d\n"),bTargetReady);
    m_pSock->sendInteger(bTargetReady,_T("target ready indicator"));
    
    int nAck=-1;
    
    if(bTargetReady){
      if(CeCosSocket::IsLegalHostPort(m_pResource->Serial())){
        TRACE(_T("Sending %s\n"),(LPCTSTR)m_pResource->Serial());
        if(m_pSock->sendString(m_pResource->Serial(),_T("Serial name")) && m_pSock->recvInteger(nAck,_T("Terminating ack"),CeCosSocket::NOTIMEOUT)){
          TRACE(_T("Terminating ack=%d\n"),nAck);
        }
      } else {
        String strHostPort(CeCosSocket::HostPort(CeCosSocket::MyHostName(),m_nAuxPort));
        
        TRACE(_T("Using %s\n"),(LPCTSTR)strHostPort);
        
        if(m_pSock->sendString(strHostPort,_T("host:port"))){
          
          // This Boolean signifies that the serial<-->tcp/ip conversation is done.  It may be set
          // on completion of the ConnectSocketToSerial thread (which is why we pass it to runthread)
          // and also set by us to *cause* the thread to complete.
          
          bool bConnectSocketToSerialThreadDone=false; // Indication of termination of ConnectSocketToSerial thread
          m_bStopConnectSocketToSerial=false; // Used to tap ConnectSocketToSerial thread on the shoulder
          
          CeCosThreadUtils::RunThread(SConnectSocketToSerialThreadFunc,this,&bConnectSocketToSerialThreadDone,_T("SConnectSocketToSerialThreadFunc")); 
          
          // Wait for either client or the ConnectSocketToSerial thread to finish.
          if(m_pSock->recv(&nAck,sizeof(int),_T("Terminating ack"),CeCosSocket::NOTIMEOUT,DerefBool,&bConnectSocketToSerialThreadDone)){
            TRACE(_T("Session terminated by request of client (%s)\n"),(LPCTSTR)Image((StatusType)nAck));
          } else if(0!=m_pSock->SocketError()){
            TRACE(_T("Session terminated by socket error - %s\n"),(LPCTSTR)m_pSock->SocketErrString());
          }
          if(!bConnectSocketToSerialThreadDone){
            // Tap ConnectSocketToSerial thread on the shoulder
            TRACE(_T("Waiting for ConnectSocketToSerial thread to terminate...\n"));
            m_bStopConnectSocketToSerial=true;
            CeCosThreadUtils::WaitFor(bConnectSocketToSerialThreadDone);
          }
        }
      }
    }
  }
  delete this;
}

bool CeCosTest::send(const void *pData,unsigned int nLength,LPCTSTR pszMsg,Duration dTimeout)
{
  return m_pSock->send(pData,nLength,pszMsg,dTimeout,IsCancelled,this);
}

bool CeCosTest::recv(const void *pData,unsigned int nLength,LPCTSTR pszMsg,Duration dTimeout)
{
  return m_pSock->recv(pData,nLength,pszMsg,dTimeout,IsCancelled,this);
}

void CeCosTest::Log(LPCTSTR  pszFormat, ...)
{
  va_list args;
  va_start(args, pszFormat);
  String str;
  str.vFormat(pszFormat,args);
  va_end(args);
  LogString(str);
}

void CeCosTest::LogString(LPCTSTR psz)
{
  if(*psz){
    ENTERCRITICAL;
    m_strOutput+=psz;
    LEAVECRITICAL;
    if(CeCosTrace::IsInteractive()){
      CeCosTrace::Out(psz);
    } else {
      TRACE(_T("%s"),psz);
    }
  }
}

bool CeCosTest::sendResult(Duration dTimeout)
{
  bool rc=
    m_pSock->sendInteger(m_Status,_T("result"),dTimeout) &&
    m_pSock->sendInteger(m_nDownloadTime,_T("result"),dTimeout) &&
    m_pSock->sendInteger(m_nTotalTime,_T("result"),dTimeout) &&
    m_pSock->sendInteger(m_nMaxInactiveTime,_T("result"),dTimeout) &&
    m_pSock->sendString (m_strOutput,_T("result"),dTimeout);
  return rc;
}

bool CeCosTest::recvResult(Duration dTimeout)
{
  String strOutput;
  int nStatus=StatusTypeMax;
  bool rc=
    m_pSock->recvInteger(nStatus,_T("result"),dTimeout) &&
    m_pSock->recvInteger(m_nDownloadTime,_T("result"),dTimeout) &&
    m_pSock->recvInteger(m_nTotalTime,_T("result"),dTimeout) &&
    m_pSock->recvInteger(m_nMaxInactiveTime,_T("result"),dTimeout) &&
    m_pSock->recvString (strOutput,_T("result"),dTimeout);
  m_Status=(StatusType)MIN(nStatus,StatusTypeMax);
  LogString(strOutput);
  return rc;
}

// Return time used by inferior gdb process - CPU for sim, wallclock otherwise
Time CeCosTest::InferiorTime() const
{
  if(*(m_pResource->Serial())){
    return Now();
  }
  if(!m_psp){
    return 0;
  }
  Time now=Now();
  if(now-m_tPrevSample>1000){
    m_tPrevSample=now;
    m_tInferiorCpuTime=m_psp->CpuTime();
  }
  return m_tInferiorCpuTime;
}

bool CeCosTest::CheckForTimeout()
{
  bool rc=(Cancelled!=Status());
  if(TimeOut!=m_Status && DownloadTimeOut!=m_Status){
    Time t=InferiorTime();
    if(t){
      // We have been able to measure the time
      if(m_bDownloading){
        m_nDownloadTime=MAX(m_nDownloadTime,Duration(InferiorTime()-m_tBase));
        if(m_nDownloadTime>m_ep.DownloadTimeout()){
          Log(_T("\n*** Timeout - download time ") WFS _T(" exceeds limit of ") WFS _T("\n"),WF(m_nDownloadTime),WF(m_ep.DownloadTimeout()));
          rc=false;
        }
      } else {
        m_nMaxInactiveTime=MAX(m_nMaxInactiveTime,Duration(InferiorTime()-m_tBase));
        if (m_nMaxInactiveTime>m_ep.ActiveTimeout()) {
          Log(_T("\n*** Timeout - inactive time ") WFS _T(" exceeds limit of ") WFS _T("\n"),WF(m_nMaxInactiveTime),WF(m_ep.ActiveTimeout()));
          rc=false;
        }
      }
    }
    m_nTotalTime=Duration(Now()-m_tWallClock0);
/*
    if(m_nTotalTime>m_ep.ElapsedTimeout()){
      Log(_T("\n*** Timeout - total time ") WFS _T(" exceeds limit of ") WFS _T("\n"),   WF(m_nTotalTime),WF(m_ep.ElapsedTimeout()));
      rc=false;
    }
*/
    if(!rc){
      SetStatus(m_bDownloading?DownloadTimeOut:TimeOut);
    }
  }
  return rc;
}

// Convert a path to something a cygwin tool will understand.  Used when invoking -size and -gdb
String CeCosTest::CygPath (LPCTSTR pszPath)
{
#ifdef _WIN32
  String str = "";
  HKEY hKey = 0;
  DWORD type;
  BYTE value[256];
  DWORD sz = sizeof(value);

  // look for the cygdrive prefix in the user's registry settings
  if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Cygnus Solutions\\Cygwin\\mounts v2", 0, KEY_READ, &hKey)) {
    if (ERROR_SUCCESS == RegQueryValueEx(hKey, "cygdrive prefix", NULL, & type, value, & sz)) {
      str = (const char*) value;
    }
    RegCloseKey(hKey);
  }

  // if not yet found, look for the cygdrive prefix in the system registry settings
  hKey = 0;
  sz = sizeof(value);
  if (str.empty() && (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Cygnus Solutions\\Cygwin\\mounts v2", 0, KEY_READ, &hKey))) {
    if (ERROR_SUCCESS == RegQueryValueEx(hKey, "cygdrive prefix", NULL, & type, value, & sz)) {
      str = (const char*) value;
    }
    RegCloseKey(hKey);
  }

  int prefixlen = str.length();
  TCHAR *buf=str.GetBuffer(prefixlen+1+MAX_PATH);
  TCHAR *pszFname;
  if(::GetFullPathName(pszPath,MAX_PATH,prefixlen+buf, &pszFname)){
    GetShortPathName(prefixlen+buf,prefixlen+buf,MAX_PATH); // ignore errors
    buf[prefixlen+1]=buf[prefixlen];
    buf[prefixlen]=_TCHAR('/');
    for(int i=prefixlen+2;buf[i];i++){
      if(_TCHAR('\\')==buf[i]){
        buf[i]=_TCHAR('/');
      }
    }
    str.ReleaseBuffer();
    return str;
  } else {
    str.ReleaseBuffer();
    return pszPath;
  }
#endif
  return pszPath;
}

void CeCosTest::SetExecutable(LPCTSTR pszExecutable)
{
  m_strOutput=_T("");
  if(pszExecutable){
    m_strExecutable=pszExecutable;
    if(m_ep.Platform()){
      GetSizes();
    } else {
      ERROR(_T("Don't know how to get sizes of this platform type\n"));
    }
  } else {
    m_strExecutable=_T("");
  }
}

// Calculate the sizes of the given file.  The target parameter is necessary in order to 
// determine which -size executable to use to do the job.
bool CeCosTest::GetSizes()
{
TRACE(_T("GetSizes %s\n"),(LPCTSTR)Executable());
  bool rc=false;
  m_nStrippedSize=m_nFileSize=0;
  LPCTSTR pszPrefix=m_ep.Platform()->Prefix();
  struct _stat buf;
  if(-1==_tstat(Executable(),&buf)){
    Log(_T("%s does not exist\n"),(LPCTSTR)Executable());
  } else if (_TCHAR('\0')==*pszPrefix){
    LogString(_T("No prefix to run a size program\n"));
  } else {
    m_nFileSize=buf.st_size;
    const String strSizeCmd(String::SFormat(_T("%s-size %s"),pszPrefix,(LPCTSTR)CygPath(Executable())));
    String strOut;
    CSubprocess sp;
    if(!sp.Run(strOut,strSizeCmd)){
      Log(_T("Failed to run \"%s\" - %s\n"),(LPCTSTR)strSizeCmd,(LPCTSTR)sp.ErrorString());
    } else {
      const TCHAR *c=_tcschr(strOut,_TCHAR('\n'));
      if(c){
        c++;
      }
      int s1=0;
      int s2=0;
      if(c && 2==_stscanf(c,_T(" %d %d"),&s1,&s2)){
        rc=true;
        m_nStrippedSize=s1+s2;
      }
      TRACE(_T("GetSizes %s rc=%d file size=%d stripped size=%d\n"),(LPCTSTR)Executable(),rc,m_nFileSize,m_nStrippedSize);
    }
  }
  return rc;
}

void CeCosTest::SetTimeouts (Duration dActive,Duration dDownload/*,Duration dElapsed*/)
{
  m_ep.SetActiveTimeout  (dActive);
  m_ep.SetDownloadTimeout(dDownload);
/*
  m_ep.SetElapsedTimeout (dElapsed);
*/
}

void CeCosTest::CloseSocket (){
  delete m_pSock;
  m_pSock=0;
}

bool CeCosTest::AtPrompt()
{
  const String strPrompt(m_ep.Platform()->Prompt());
  unsigned int nPromptLen=_tcslen(strPrompt);
  return
    nPromptLen>0 &&
    m_strOutput.size()>=nPromptLen && 
    0==_tcscmp((LPCTSTR)m_strOutput+m_strOutput.size()-nPromptLen,strPrompt);
}

#ifdef _WIN32
BOOL WINAPI HandlerRoutine(
                           DWORD dwCtrlType   //  control signal type
                           )
{
  dwCtrlType; // eliminate compiler warning
  return TRUE;
}
#endif


bool CeCosTest::InteractiveInferior(LPCTSTR pszHostPort,TCHAR **argv)
{
  bool rc=false;
  if(_TCHAR('\0')!=*pszHostPort){
    if(!CeCosSocket::IsLegalHostPort(pszHostPort)){
      ERROR(_T("Illegal host:port '%s'\n"),pszHostPort);
      return false;
    } else {
      m_strExecutionHostPort=pszHostPort;
      Log(_T("Waiting to connect to %s...\n"),(LPCTSTR)m_strExecutionHostPort);
    }
  } else {
    Log(_T("Waiting to connect to a server...\n"));
  }
  
  ConnectForExecution();
  
  Log(_T("Connected to %s - waiting for target reset\n"),(LPCTSTR)m_strExecutionHostPort);
  String strHostPort,strOutput;
  // We read:
  //     target ready indicator
  //     any output so far
  //     (if target ready) host:port
  if(GetTargetReady(strHostPort)){
    Log(_T("Use target remote %s\n"),(LPCTSTR)strHostPort);
    String strInferior(m_ep.Platform()->Prefix());
    strInferior+=_T("-gdb");
#ifdef _WIN32
    SetConsoleCtrlHandler(HandlerRoutine,TRUE);
    int n=_tspawnvp(_P_WAIT,strInferior,argv);
    if(-1==n){
      Log(_T("Failed to spawn %s\n"),(LPCTSTR)strInferior);
    } else {
      rc=(0==n);
    }   
    SetConsoleCtrlHandler(HandlerRoutine,FALSE);
#else // UNIX
    
    int pid=fork();
    switch(pid){
    case -1:
      _ftprintf(stderr,_T("fork failed\n"));
      pid=0;
      break;  
    case 0:
      // Process is created (we're the child)
      execvp(strInferior,argv);
      Log(_T("Error invoking %s - %s\n"),(LPCTSTR)strInferior,strerror(errno));
      exit(1);
      break;
    default:
      // Process is created (we're the parent)
      {
        signal(SIGINT,SIG_IGN);
        int stat;
        waitpid(pid,&stat,0);
        rc=(0==stat);
        signal(SIGINT,SIG_DFL);
      }
      break;
    }
#endif
    Log(_T("Inferior terminated\n"));
    // Tell the server we're through
    m_pSock->sendInteger(123,_T("Terminating ack"));
  }
  return rc;
}

void CALLBACK CeCosTest::ResetLogFunc(void *pParam, LPCTSTR psz) 
{
  CeCosTest *pTest=(CeCosTest *)pParam;
  TRACE(_T("Send Target Ready indicator=2\n"));
  pTest->m_pSock->sendInteger(2,_T("target ready indicator"));
  TRACE(_T("Send %s\n"),psz);
  pTest->m_pSock->sendString(psz,_T("output so far"));
}

CeCosTest::ExecutionParameters::RequestType CeCosTest::ExecutionParameters::RequestTypeValue(LPCTSTR psz)
{
  int r;
  for(r=0;r<RequestTypeMax;r++){
    if(0==_tcsicmp(psz,arRequestImage[r])){
      break;
    }
  }
  return (RequestType)r;
}

void CeCosTest::InferiorOutputFunc(LPCTSTR pszMsg)
{
  LogString(pszMsg);

  m_nOutputLen+=_tcslen(pszMsg);

  if(m_pspPipe){
    m_pspPipe->Send(pszMsg);
  }

  if(m_nOutputLen>20000){
    LogString(_T("\n>>>> Infra FAIL\n*** too much output ***\n>>>>\n"));
    SetStatus(Fail);
    m_psp->Kill();
  }
  
  m_tBase=InferiorTime(); // We are seeing life, so reset the clock for timeouts
  
  if(AtPrompt()){
    
    // gdb's output included one or more prompts
    // Send another command along
    if(m_nCmdIndex>=m_arstrInferiorCmds.size()){
      // Nothing further to say to gdb - exit
      
      m_psp->Kill(); // case 3
    } else {

      if(m_nCmdIndex>0 && 0==_tcscmp(_T("load"),m_arstrInferiorCmds[m_nCmdIndex-1])){
        // load command was previous command - we are no longer downloading
        m_bDownloading=false;
      }

      String strCmd(m_arstrInferiorCmds[m_nCmdIndex++]);

      // If we can there is a GDB instruction to send to gdb, do it
      String str;
      if(GetDirective(_T("GDB:"),str,m_nLastGdbInst)){
        strCmd=str;
        m_nCmdIndex--; // undo increment above
      }

      if(0==_tcscmp(_T("load"),strCmd)){
        // load command issued - we are now "downloading"
        m_bDownloading=true;
      } else if(0==_tcscmp(_T("run"),strCmd) || 0==_tcscmp(_T("cont"),strCmd)){
        SetStatus(NoResult);
      } 
      
      strCmd+=_TCHAR('\n');
      LogString(strCmd);
      m_psp->Send(strCmd);          

    }
  }

  // If there is a EXEC instruction to process, obey it
  String strCmd;
  while(GetDirective(_T("EXEC:"),strCmd,m_nLastExecInst)){
    CSubprocess *pExecsp=new CSubprocess;
    pExecsp->SetPath(m_strPath);
    if(!pExecsp->Run(AppendFunc,this,(LPCTSTR)strCmd,false)){
      Log(_T("%%%% Failed to create process '%s'\n"),(LPCTSTR)strCmd);
      delete pExecsp;
    } else {
      m_arpExecsp.push_back(pExecsp);
    }
  }

  // If there is a PIPE instruction to process, obey it
  while(GetDirective(_T("PIPE:"),strCmd,m_nLastPipeInst)){
    if(m_pspPipe){
      Log(_T("%%%% Two PIPE commands are a no-no\n"));
    } else {
      m_pspPipe=new CSubprocess;
      m_pspPipe->SetPath(m_strPath);

      if(!m_pspPipe->Run(AppendFunc,this,(LPCTSTR)strCmd,false)){
        Log(_T("%%%% Failed to create process '%s'\n"),(LPCTSTR)strCmd);
        delete m_pspPipe;
        m_pspPipe=0;
      } else {
        // Send what we read have so far
        m_pspPipe->Send(m_strOutput);
      }
    }
  }

  while(GetDirective(_T("TIMEOUT:"),strCmd,m_nLastTimeoutInst)){
    int n=_ttoi(strCmd);
    if(n){
      SetTimeouts(n); // second parameter is download timeout, which is now irrelevant
    } else {
      Log(_T("%%%% Illegal timeout specified: %s\n"),(LPCTSTR)strCmd);
    }
  }
}

void CeCosTest::RunInferior(LPCTSTR pszCmdline)
{
  m_psp=new CSubprocess;
  m_psp->SetContinuationFunc(SCheckForTimeout,this);
  try {
    m_nMaxInactiveTime=0;
    m_nTotalTime=0;
    m_nDownloadTime=0;
    m_nOutputLen=0;
    m_bDownloading=false;
    
    // Decide on the baseline status - NotStarted if there is a download element, NoResult otherwise.
    m_Status=NoResult;
    for(unsigned int i=0;i<m_arstrInferiorCmds.size();i++){
      if(0==_tcscmp(_T("run"),m_arstrInferiorCmds[i]) || 0==_tcscmp(_T("cont"),m_arstrInferiorCmds[i])){
        m_Status=NotStarted;
        break;
      }
    }
    TRACE(_T("Status <- %s\n"),(LPCTSTR)Image(m_Status));

    m_tPrevSample=0; // force an initial reading
    m_tInferiorCpuTime=0;

    m_tBase=m_tBase0=InferiorTime(); // Returns either Now() or nothing
    m_tWallClock0=Now();

    m_nCmdIndex=0;
  
    TRACE(_T("RunGDB()\n"));
  
    m_nLastGdbInst=m_nLastExecInst=m_nLastTimeoutInst=m_nLastPipeInst=0;
    m_psp->SetPath(m_strPath);
    if(m_psp->Run(SInferiorOutputFunc,this,pszCmdline,true)){

      if(m_pspPipe){
        m_pspPipe->Send(_T("\n"));
        m_pspPipe->CloseInput();
        if(m_pspPipe->Wait(5000)){
          // OK the pipe process terminated.
          int rc=m_pspPipe->GetExitCode();
          if(0!=rc){
            Log(_T("%%%% Pipe process returned rc=%d\n"),rc);
            SetStatus(Fail);
          }
        } else {
          LogString(_T("%%%% Pipe process would not complete\n"));
        }
      }

      AnalyzeOutput();
    
    } else {
      Log(_T("Failed to run \"%s\" - %s\n"),pszCmdline,(LPCTSTR)m_psp->ErrorString());
    }
  } 
  catch(...){
    ERROR(_T("!!! Exception caught in RunInferior()\n"));
  }
  delete m_psp; // will cause process to be killed as necessary and completion to be waited for
  m_psp=NULL;
  for(int i=0;i<(signed)m_arpExecsp.size();i++){
    delete (CSubprocess *)m_arpExecsp[i]; // ditto
  }
  m_arpExecsp.clear();
  TRACE(_T("Exiting RunInferior()\n"));
}

void CeCosTest::AnalyzeOutput()
{
  // This test is pulled out to allow ser_filter to simulate a test failure
  if(OutputContains(_T("FAIL:"))){
    SetStatus(Fail);
  }
  
  if(OutputContains(_T("EXIT:"))||OutputContains(_T("NOTAPPLICABLE:"))){
    static LPCTSTR arpszKeepAlive[]={_T("FAIL:"),_T("NOTAPPLICABLE:"), _T("PASS:")}; 
    static const StatusType arStatus[] ={Fail, Inapplicable, Pass};
    for(unsigned int i=0;i<sizeof arpszKeepAlive/sizeof arpszKeepAlive[0];i++){
      if(OutputContains(arpszKeepAlive[i])){
        TRACE(_T("DriveInferior: saw '%s'\n"),arpszKeepAlive[i]);
        SetStatus(arStatus[i]); // Do not break!
      }
    }
  }
  
  // Certain output spells failure...
  if(OutputContains(_T("cyg_assert_fail ("))){
    SetStatus(AssertFail);
  } else {
    static LPCTSTR arpszSignals[]={_T("SIGBUS"), _T("SIGSEGV"), _T("SIGILL"), _T("SIGFPE"), _T("SIGSYS"), _T("SIGTRAP")};
    for(unsigned int i=0;i<sizeof arpszSignals/sizeof arpszSignals[0];i++){
      String str1,str2;
      str1.Format(_T("signal %s"),arpszSignals[i]);
      str2.Format(_T("handle %s nostop"),arpszSignals[i]);
      if(OutputContains(str1)&&!OutputContains(str2)){
        SetStatus(Fail);
        break;
      }
    }
  }
  
  int nIndex=0;
  String str;
  while(GetDirective(_T("EXPECT:"),str,nIndex)){
    // s1 is the pointer to the text following the expect - that to be tested
    LPCTSTR s1=(LPCTSTR)m_strOutput+nIndex;
    while (_istspace(*s1)){
      s1++;
    }
    // whereas s2 is the pointer to the text in the expect string (what we are expecting)
    LPCTSTR s2=(LPCTSTR)str;
    while(*s2){
      if(*s2!=*s1){
        Log(_T("EXPECT:<> failure - expected '%s' saw '%s'\n"),(LPCTSTR)str,(LPCTSTR)m_strOutput+nIndex);
        SetStatus(Fail);
        break;
      }
      s1++;
      s2++;
    }
  }
}

bool CeCosTest::ExecutionParameters::FromStr(LPCTSTR psz)
{
  String str1,str2,str3,str4,str5;
  int nUseFilter,nUnused2,nUnused3;
  int nLen=_tcslen(psz);
  _stscanf(psz,_T("%s %s %d %d %d %d %d %d %d %d %s %s %s"),
    str1.GetBuffer(1+nLen),
    str2.GetBuffer(1+nLen),
    &m_nActiveTimeout,
    &m_nDownloadTimeout,
    &m_nUnused1,
    &m_nUnused2,
    &m_nUnused3,
    &nUseFilter,
    &nUnused2,
    &nUnused3,
    str3.GetBuffer(1+nLen),
    str4.GetBuffer(1+nLen),
    str5.GetBuffer(1+nLen)
    );
  m_bUseFilter=(0!=nUseFilter);            
  m_bUnused2=(0!=nUnused2);            
  m_bUnused3=(0!=nUnused3);            
  str1.ReleaseBuffer();
  str2.ReleaseBuffer();
  str3.ReleaseBuffer();
  str4.ReleaseBuffer();
  str5.ReleaseBuffer();
  m_Target=str1;
  int r;
  for(r=0;r<RequestTypeMax;r++){
    if(0==_tcscmp(arRequestImage[r],str2)){
      break;
    }
  }
  m_Request=(RequestType)r;
  return CeCosTestPlatform::IsValid(m_Target);
}

CeCosTest::ExecutionParameters::ExecutionParameters (RequestType r,
                                                     LPCTSTR  Target,
                                                     Duration    nActiveTimeout/*=NOTIMEOUT*/,
                                                     Duration    nDownloadTimeout/*=NOTIMEOUT*/):
  m_bUseFilter(true),
  m_Target(Target),
  m_nActiveTimeout(nActiveTimeout),
  m_nDownloadTimeout(nDownloadTimeout),
  m_Request(r),
  m_nUnused1(0),
  m_nUnused2(0),
  m_nUnused3(0),
  m_bUnused2(false),
  m_bUnused3(false)
{
}

String CeCosTest::ExecutionParameters::Image() const
{
  String str;
  str.Format(_T("%s %s %d %d %d %d %d %d %d %d"),(LPCTSTR)PlatformName(),(LPCTSTR)Image(Request()),
    ActiveTimeout(),DownloadTimeout(),
    m_nUnused1,
    m_nUnused2,
    m_nUnused3,
    m_bUseFilter,
    m_bUnused2,
    m_bUnused3);
  return str;
}

bool CeCosTest::GetTargetReady(String &strHostPort)
{
  bool rc=false;
  int nTargetReady;
  do{
    if(!m_pSock->recvInteger(nTargetReady,_T("Target ready"),120*1000)){
      Log(_T("Failed to read target ready indicator from server - %s\n"),(LPCTSTR)m_pSock->SocketErrString());
      break;
    }
    switch(nTargetReady){
    case 0:
      LogString(_T("Failed to reset target"));
      break;
    case 1:
      if(m_pSock->recvString(strHostPort, _T("host:port"))){
        TRACE(_T("Instructed to use %s\n"),(LPCTSTR)strHostPort);
        rc=true;
      } else {
        Log(_T("Failed to read host:port - %s\n"),(LPCTSTR)m_pSock->SocketErrString());
      }
      break;
    case 2:
      {
        String strOutput;
        if(m_pSock->recvString(strOutput, _T("output"))){
          LogString(strOutput);               
        } else {
          Log(_T("Failed to read output\n"),(LPCTSTR)m_pSock->SocketErrString());
          return false;
        }
      }
      break;
    }
  } while(2==nTargetReady);
  return rc;
}


CeCosTest::ServerStatus CeCosTest::ServerStatusValue(LPCTSTR psz)
{
  int s;
  for(s=0;s<ServerStatusMax;s++){
    if(0==_tcsicmp(psz,arServerStatusImage[s])){
      break;
    }
  }
  return (ServerStatus)s;

}

// Gets a directive from the test output (like EXEC:)
bool CeCosTest::GetDirective(LPCTSTR pszDirective, String &str, int &nIndex)
{
  bool rc=false;
  ENTERCRITICAL;
  LPCTSTR pszOutput=(LPCTSTR)m_strOutput;
  LPCTSTR pc=_tcsstr(pszOutput+nIndex,pszDirective);
  if(pc){
    
    pc+=_tcslen(pszDirective); // Now after the final character (':') of the directive
    if(_TCHAR('<')==*pc){

      pc++;

      // Extract the argument
      str=_T("");
      while(*pc){
        // Process escapes: FIXME more escapes?
        TCHAR c=*pc;
        if(_TCHAR('\\')==c){
          switch(pc[1]){
            case _TCHAR('t'):
              c=_TCHAR('\t');
              break;
            case _TCHAR('n'):
              c=_TCHAR('\n');
              break;
            case _TCHAR('\0'):
              pc--; // avoid grief
              break;
            default:
              c=pc[1];
              break;
          }
          pc++;
        } else if (_TCHAR('>')==c) {
          nIndex=pc+1-pszOutput;
          rc=true;
          break;
        } else if (_TCHAR('\n')==c) {
          nIndex=pc+1-pszOutput;
          Log(_T("%%%% Unterminated directive: %s"),(LPCTSTR)str);
          break;
        }
        str+=c;
        pc++;
      }
    }
  }
  LEAVECRITICAL;
  return rc;
}

void CeCosTest::GetInferiorCommands(StringArray &arstrInferiorCmds)
{
  arstrInferiorCmds.clear();

  // Construct commands for gdb.  The commands may be found (semicolon-separated) in the target info:
  const String strInferiorCmds(m_ep.Platform()->GdbCmds());
  StringArray ar;
  int nCmds=strInferiorCmds.Chop(ar,_TCHAR(';'),false);
  for(int i=0;i<nCmds;i++){
    // Into each command must be substituted:
    // Baud rate (%b)  
    // Port      (%p)  This will be a serial port (e.g. COM1) or a socket connection (e.g.aloo:8000) depending on circumstances.
    // and escapes must be dealt with.
    String strCmd;
    for(const TCHAR *pc=ar[i];*pc;pc++){
      switch(*pc){
        // Process escapes: FIXME more escapes?
        case _TCHAR('\\'):
          switch(pc[1]){
            case _TCHAR('t'):
              strCmd+=_TCHAR('\t');
              pc++;
              continue;
            case _TCHAR('n'):
              strCmd+=_TCHAR('\n');
              pc++;
              continue;
            case _TCHAR('\0'):
              continue;
            default:
              break;
          }
          break;
        case _TCHAR('%'):
          switch(pc[1]){
            case _TCHAR('%'):
              strCmd+=_TCHAR('%');
              pc++;
              break;
            case _TCHAR('b'):
              if(0==m_pResource->Baud()){
                goto NextCmd; // Suppress output of this command if there is no baud rate to output
              }
              strCmd+=String::SFormat(_T("%d"),m_pResource->Baud());
              pc++;
              continue;
            case _TCHAR('p'):
              if(_TCHAR('\0')==*(m_pResource->Serial())){
                goto NextCmd; // Suppress output of this command if there is no serial port
              }
              strCmd+=m_pResource->Serial();
              pc++;
              continue;
            case _TCHAR('\0'):
              continue;
            default:
              break;
          }
          break;
        default:
          break;
      }
      strCmd+=*pc;
    }
    arstrInferiorCmds.push_back(strCmd);
NextCmd:
    ;
  }
  return;
}

